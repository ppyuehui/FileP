<#
.SYNOPSIS
  合并官方新版之后的一键冒烟：截 5 个设置页 + 跑 2 个回归测试。

.DESCRIPTION
  截图只能证明「页面画出来了、没崩」；hover、拖拽、右键二级菜单、输入框打字仍需手点。
  任何一页没出图、图太小、进程非零退出，或测试有失败项，都会返回 1。

.EXAMPLE
  pwsh -File tools/smoke-fork.ps1
#>
[CmdletBinding()]
param(
    [string]$Exe = 'C:\dev\FileP\build\FileP.exe',
    [string]$OutDir = (Join-Path $env:TEMP 'filep-smoke'),
    [switch]$KeepOpen,
    [int]$TimeoutSeconds = 90
)

$ErrorActionPreference = 'Stop'

function Info([string]$m) { Write-Host $m -ForegroundColor Cyan }
function Good([string]$m) { Write-Host $m -ForegroundColor Green }
function Bad([string]$m) { Write-Host $m -ForegroundColor Red }

$pages = @(
    [pscustomobject]@{ Id = 0;  Name = '外观（四个自有开关）' }
    [pscustomobject]@{ Id = 3;  Name = '关于与诊断（三按钮 + FileP）' }
    [pscustomobject]@{ Id = 9;  Name = '文件列表（变化摘要）' }
    [pscustomobject]@{ Id = 10; Name = '标签页与窗口' }
    [pscustomobject]@{ Id = 16; Name = '自定义命令（主从双栏）' }
)

$tests = @(
    [pscustomobject]@{ Exe = 'pulse_custom_commands_test.exe';   Name = '自定义命令序列化' }
    [pscustomobject]@{ Exe = 'pulse_settings_transfer_test.exe'; Name = '设置备份迁移' }
)

if (-not (Test-Path $Exe)) {
    Bad "找不到 $Exe —— 先编译（pwsh -File C:\dev\FileP\build_filep.ps1）。"
    exit 1
}
$exeDir = Split-Path -Parent $Exe

# DSH 是 Low 令牌，编出的 exe 会带 Low：提回 Medium，否则双击/拖放异常。
try { & icacls $Exe /setintegritylevel Medium *> $null } catch { }
$level = (& icacls $Exe) -join ' '
if ($level -match 'Mandatory Label\\Low') { Bad "exe 仍带 Low 完整性标签：$Exe"; exit 1 }

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
$failures = @()
$rows = @()

Info "截图 $($pages.Count) 个页面 -> $OutDir"
Get-Process FileP -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Milliseconds 800

foreach ($page in $pages) {
    $png = Join-Path $OutDir ('page{0}.png' -f $page.Id)
    Remove-Item $png -ErrorAction SilentlyContinue
    $proc = Start-Process -FilePath $Exe -ArgumentList '--shot', $png, '--shot-settings', "$($page.Id)" -PassThru
    if (-not $proc.WaitForExit($TimeoutSeconds * 1000)) {
        try { $proc.Kill() } catch { }
        $failures += "第 $($page.Id) 页（$($page.Name)）截图超时"
        $rows += [pscustomobject]@{ 项目 = "页面 $($page.Id)"; 说明 = $page.Name; 结果 = '超时' }
        continue
    }
    if ((Test-Path $png) -and ((Get-Item $png).Length -gt 20000)) {
        $kb = [math]::Round((Get-Item $png).Length / 1KB)
        $rows += [pscustomobject]@{ 项目 = "页面 $($page.Id)"; 说明 = $page.Name; 结果 = "$kb KB" }
    } else {
        $failures += "第 $($page.Id) 页（$($page.Name)）没出图或图太小"
        $rows += [pscustomobject]@{ 项目 = "页面 $($page.Id)"; 说明 = $page.Name; 结果 = '失败' }
    }
    if (-not $KeepOpen) { Get-Process FileP -ErrorAction SilentlyContinue | Stop-Process -Force }
    Start-Sleep -Milliseconds 400
}

Info '跑回归测试'
foreach ($t in $tests) {
    $path = Join-Path $exeDir $t.Exe
    if (-not (Test-Path $path)) {
        $rows += [pscustomobject]@{ 项目 = $t.Name; 说明 = $t.Exe; 结果 = '未编译（跳过）' }
        continue
    }
    $out = & $path 2>&1 | Out-String
    $code = $LASTEXITCODE
    $hasFail = $out -match '\[FAIL\]'
    $hasSummary = $out -match '(\d+) passed, (\d+) failed'
    if (($code -eq 0) -and (-not $hasFail) -and $hasSummary -and ($matches[2] -eq '0')) {
        $rows += [pscustomobject]@{ 项目 = $t.Name; 说明 = $t.Exe; 结果 = "$($matches[1]) passed" }
    } else {
        $failures += "$($t.Name)（$($t.Exe)）失败，退出码 $code"
        $rows += [pscustomobject]@{ 项目 = $t.Name; 说明 = $t.Exe; 结果 = '失败' }
        ($out.Trim() -split [Environment]::NewLine | Select-Object -Last 8) | ForEach-Object { Write-Host $_ -ForegroundColor DarkGray }
    }
}

Write-Host ''
$rows | Format-Table -AutoSize | Out-String -Width 200 | Write-Host

if ($failures.Count -eq 0) {
    Good '冒烟通过。'
    Write-Host '仍需手点：地址栏「此电脑」、标签 ✕ 按下后划走、自定义命令打字/Tab、' -ForegroundColor DarkGray
    Write-Host '工具栏「左右双栏」开关、右键二级菜单安全三角、任务栏标题滑块。' -ForegroundColor DarkGray
    exit 0
}

Bad ("冒烟有 {0} 项不合格：" -f $failures.Count)
$failures | ForEach-Object { Bad ('  - ' + $_) }
exit 1
