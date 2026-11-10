<#
.SYNOPSIS
  把我们相对官方的改动导出成一组补丁 + 一份清单，供下次合并参考。

.DESCRIPTION
  以「本仓库与 upstream 的共同祖先」为基线，把之后所有非合并提交按时间顺序导出成
  fork-patches\NNNN-<提交>.patch，并生成 fork-patches\MANIFEST.md：
  每个提交改了哪些文件、一句话说明。

  用途：合并官方新版时某个功能被整侧覆盖掉了，可以按 MANIFEST 找到对应的补丁，
  用 git apply --3way fork-patches\NNNN-xxx.patch 只把那一个功能贴回来。

  注意：补丁是在「当时的上游代码」上生成的，官方大改过的文件直接 patch 会冲突，
  这时仍然要手工接回（参考 docs/UPSTREAM-MERGE.md），补丁只用来抄代码。
#>
[CmdletBinding()]
param(
    [string]$Base = ''
)

$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
Set-Location $repo

if (-not $Base) {
    $Base = (git merge-base HEAD upstream/main).Trim()
    if (-not $Base) { Write-Host '找不到与 upstream/main 的共同祖先，用 -Base 指定。' -ForegroundColor Red; exit 1 }
}
Write-Host "基线: $Base" -ForegroundColor Cyan

$outDir = Join-Path $repo 'fork-patches'
if (-not (Test-Path $outDir)) { New-Item -ItemType Directory -Path $outDir | Out-Null }
Get-ChildItem $outDir -Filter '*.patch' | Remove-Item -Force

$tab = [char]9
$log = git log --no-merges --reverse --date=short --format='%H%x09%ad%x09%s' "$Base..HEAD"
if (-not $log) { Write-Host '基线之后没有我们的提交。' -ForegroundColor Yellow; exit 0 }

$rows = @()
$n = 0
foreach ($line in $log) {
    $parts = $line -split $tab
    if ($parts.Count -lt 3) { continue }
    $sha = $parts[0]
    $date = $parts[1]
    $subject = $parts[2]
    $n++
    $slug = ($subject -replace '[\\/:*?"<>|]', '_') -replace '\s+', '-'
    if ($slug.Length -gt 60) { $slug = $slug.Substring(0, 60) }
    $name = '{0:D4}-{1}.patch' -f $n, $slug
    git diff --binary "$sha^" "$sha" | Out-File -Encoding utf8 (Join-Path $outDir $name)
    $files = (git diff --name-only "$sha^" "$sha") -join '<br>'
    $rows += [pscustomobject]@{ N = $n; Patch = $name; Date = $date; Sha = $sha.Substring(0, 8); Subject = $subject; Files = $files }
}

$m = @()
$m += '# 本仓库相对官方的改动清单（自动生成）'
$m += ''
$m += '基线提交：' + $Base + '（与 upstream 的共同祖先）'
$m += ''
$m += '共 ' + $rows.Count + ' 个非合并提交，每个对应 fork-patches 里的一个补丁。'
$m += ''
$m += '| # | 补丁 | 日期 | 提交 | 说明 | 涉及文件 |'
$m += '| --- | --- | --- | --- | --- | --- |'
foreach ($r in $rows) {
    $files = $r.Files
    if ($files.Length -gt 300) { $files = $files.Substring(0, 300) + ' …' }
    $m += '| ' + $r.N + ' | ' + $r.Patch + ' | ' + $r.Date + ' | ' + $r.Sha + ' | ' + $r.Subject + ' | ' + $files + ' |'
}
$m += ''
$m += '## 怎么用'
$m += ''
$m += '1. 合并官方新版后先跑 tools/check-fork-features.ps1，看哪几项没了。'
$m += '2. 在本表里搜对应的功能关键词，找到补丁编号。'
$m += '3. 只想抄代码：git apply --3way fork-patches\NNNN-xxx.patch（冲突就手工挑）；'
$m += '   只想看当时怎么改的：git show <提交>。'
$m += '4. 不要整包 git apply 全部补丁 —— 它们是在旧的上游代码上生成的，会大面积冲突。'
$m | Out-File -Encoding utf8 (Join-Path $outDir 'MANIFEST.md')

Write-Host ("导出 {0} 个补丁 -> {1}" -f $rows.Count, $outDir) -ForegroundColor Green
Write-Host ("清单: {0}" -f (Join-Path $outDir 'MANIFEST.md')) -ForegroundColor Green
