<#
.SYNOPSIS
  合并官方新版后跑「号段 + catalog 自有行」校验，一条命令。

.DESCRIPTION
  git 合并不会报警的两类问题，这里补上：

    1. 我们改过官方已有行的文案（例如 AboutPulse = 「关于 FileP」）会被官方版本
       顶回去 —— 由 tools/l10n/merge_fork_rows.py 按 merge-base 三方比对自动补回；
    2. 官方新占的号段（字符串 id、菜单命令 id、设置切换号、WM_APP 消息）压到我
       们的号段上 —— 逐项查出来报错。

  然后重新生成 l10n、跑 tools/check-fork-features.ps1；-Build / -Smoke 可以顺带
  编译和冒烟（编排用，单独调试时不要带）。

  合并前（还没有 merge）也会工作：base 取 merge-base，只检查号段。

.EXAMPLE
  pwsh -File tools/merge-verify.ps1

.EXAMPLE
  pwsh -File tools/merge-verify.ps1 -Build -Smoke
#>
[CmdletBinding()]
param(
    # 我们的合并起点；默认自动取：合并中 = HEAD，已提交 = 最后一个 merge 的第一父
    [string]$BaseRef = '',
    [string]$UpstreamRef = 'upstream/main',
    [string]$Catalog = 'tools/l10n/catalog.tsv',
    [switch]$SkipL10n,
    [switch]$Build,
    [switch]$Smoke,
    [string]$BuildDir = 'C:\dev\FileP'
)

$ErrorActionPreference = 'Stop'
$NL = [char]10

function Info([string]$m) { Write-Host $m -ForegroundColor Cyan }
function Good([string]$m) { Write-Host $m -ForegroundColor Green }
function Bad([string]$m) { Write-Host $m -ForegroundColor Red }

$repo = Split-Path -Parent $PSScriptRoot
Set-Location $repo

if (-not (git rev-parse --verify --quiet $UpstreamRef)) {
    Bad "找不到 $UpstreamRef，先 git fetch upstream。"
    exit 1
}

if (-not $BaseRef) {
    if (Test-Path (Join-Path $repo '.git/MERGE_HEAD')) {
        $BaseRef = 'HEAD'   # 合并进行中：HEAD 仍是合并前的我们
    } else {
        $merge = (git log --merges --format=%H -1) -join ''
        if ($merge) { $BaseRef = $merge + '^1' } else { $BaseRef = (git merge-base HEAD $UpstreamRef) -join '' }
    }
}

function Short([string]$ref) { (git rev-parse --short $ref) -join '' }

function RefText([string]$ref, [string]$path) {
    $text = @(git show ("{0}:{1}" -f $ref, $path) 2>$null)
    if ($LASTEXITCODE -ne 0 -or $text.Count -eq 0) { return $null }
    return ($text -join $NL)
}

function CatalogIds([string]$text) {
    $map = @{}
    foreach ($line in ($text -split "\r?\n")) {
        $t = $line.Trim()
        if (-not $t -or $t.StartsWith('#') -or $t.StartsWith('//')) { continue }
        $p = $line -split "\t"
        if ($p.Count -lt 3) { continue }
        $map[$p[0]] = [int]$p[2]
    }
    return $map
}

function EnumValues([string]$text, [string]$enum) {
    $map = @{}
    $m = [regex]::Match($text, "enum\s+$enum\s*:\s*(?:int|long)\s*\{(?<body>.*?)\};", 'Singleline')
    if (-not $m.Success) { return $map }
    foreach ($line in ($m.Groups['body'].Value -split "\r?\n")) {
        $code = (($line -split '//')[0]).Trim().TrimEnd(',')
        if ($code -match '^(\w+)\s*=\s*(0x[0-9A-Fa-f]+|\d+)$') {
            $name = $Matches[1]
            $raw = $Matches[2]
            $value = if ($raw -match '^0x') { [Convert]::ToInt64($raw, 16) } else { [int64]$raw }
            $map[$name] = $value
        }
    }
    return $map
}

$rows = New-Object System.Collections.Generic.List[object]
$fail = 0
function Record([string]$check, [bool]$ok, [string]$detail) {
    $script:rows.Add([pscustomobject]@{ 检查 = $check; 结果 = $(if ($ok) { 'OK' } else { '冲突' }); 说明 = $detail })
    if (-not $ok) { $script:fail++ }
}

Info ("合并起点 base = {0} ({1})，官方 upstream = {2} ({3})" -f $BaseRef, (Short $BaseRef), $UpstreamRef, (Short $UpstreamRef))
Write-Host ''

# --- 1. catalog：先把我们改过的官方行补回来，再查字符串号段 -------------------
Info '1/5 catalog 自有行 + 字符串号段'
$helper = Join-Path $PSScriptRoot 'l10n/merge_fork_rows.py'
$helperOut = & py -3.12 $helper --repo $repo --base $BaseRef --upstream $UpstreamRef --catalog $Catalog 2>&1
$helperCode = $LASTEXITCODE
$helperOut | ForEach-Object { Write-Host "  $_" -ForegroundColor DarkGray }
Record 'catalog 自有行（我们改过的官方文案）' ($helperCode -eq 0) $(if ($helperCode -eq 0) { '已按 base 回填' } else { '有行丢失，见上方 PROBLEM' })

$baseCat = RefText $BaseRef $Catalog
$upCat = RefText $UpstreamRef $Catalog
$baseIds = CatalogIds $baseCat
$upIds = CatalogIds $upCat
$curIds = CatalogIds (Get-Content $Catalog -Raw)
$ourNames = @($baseIds.Keys | Where-Object { -not $upIds.ContainsKey($_) })
$ourIds = @($ourNames | Where-Object { $curIds.ContainsKey($_) } | ForEach-Object { $curIds[$_] })
if ($ourIds.Count -gt 0) {
    $lo = ($ourIds | Measure-Object -Minimum).Minimum
    $hi = ($ourIds | Measure-Object -Maximum).Maximum
    $upMax = ($upIds.Values | Measure-Object -Maximum).Maximum
    $clash = @($upIds.GetEnumerator() | Where-Object { $_.Value -ge $lo -and $_.Value -le $hi } | ForEach-Object { "$($_.Key)($($_.Value))" })
    $detail = "我们 {0} 行 = {1}..{2}；官方最大 {3}" -f $ourNames.Count, $lo, $hi, $upMax
    if ($clash.Count) { $detail = $detail + "；撞号：" + ($clash -join ', ') }
    Record '字符串号段（7000 段）' ($clash.Count -eq 0) $detail
} else {
    Record '字符串号段（7000 段）' $false '找不到我们的字符串行'
}

# --- 2. 菜单命令号 -------------------------------------------------------------
Info '2/5 菜单命令号（context_menu.h MenuCmd）'
$curCmds = EnumValues (Get-Content 'src/app/context_menu.h' -Raw) 'MenuCmd'
$upCmds = EnumValues (RefText $UpstreamRef 'src/app/context_menu.h') 'MenuCmd'
$cmdClash = @()
foreach ($pair in $curCmds.GetEnumerator()) {
    $sameValue = @($upCmds.GetEnumerator() | Where-Object { $_.Value -eq $pair.Value -and $_.Key -ne $pair.Key })
    foreach ($other in $sameValue) { $cmdClash += ("{0}={1} 撞 {2}" -f $pair.Key, $pair.Value, $other.Key) }
}
Record '菜单命令号' ($cmdClash.Count -eq 0) $(if ($cmdClash.Count) { $cmdClash -join '；' } else { "当前 $($curCmds.Count) 个命令号，与官方无重名重号" })

# --- 3. 设置切换号 82-86 ------------------------------------------------------
Info '3/5 设置切换号（ForkToggleSetting）'
$ourToggles = @([regex]::Matches((Get-Content 'src/app/fork_settings_toggles.cpp' -Raw), '\{\s*(\d+)\s*,\s*&AppPrefs::') | ForEach-Object { [int]$_.Groups[1].Value })
$upSettings = RefText $UpstreamRef 'src/app/settings_controller.cpp'
$intervals = New-Object System.Collections.Generic.List[object]
foreach ($m in [regex]::Matches($upSettings, 'index\s*==\s*(\d+)')) {
    $v = [int]$m.Groups[1].Value
    $intervals.Add(@($v, $v))
}
foreach ($m in [regex]::Matches($upSettings, 'index\s*>=\s*(\d+)\s*&&\s*index\s*(?:<=|<)\s*(\d+)')) {
    $intervals.Add(@([int]$m.Groups[1].Value, [int]$m.Groups[2].Value))
}
$toggleClash = @($ourToggles | Where-Object {
    $t = $_
    @($intervals | Where-Object { $t -ge $_[0] -and $t -le $_[1] }).Count -gt 0
})
Record '设置切换号 82-86' ($toggleClash.Count -eq 0) $(if ($toggleClash.Count) { "官方也在用：" + ($toggleClash -join ', ') } else { "我们的 " + ($ourToggles -join '/') + " 没被官方占用" })

# --- 4. 私有消息 WM_APP 号 ----------------------------------------------------
Info '4/5 私有消息号（app_messages.h）'
$baseMsg = RefText $BaseRef 'src/app/app_messages.h'
$upMsg = RefText $UpstreamRef 'src/app/app_messages.h'
function Messages([string]$text) {
    $map = @{}
    foreach ($m in [regex]::Matches($text, 'constexpr\s+UINT\s+(\w+)\s*=\s*WM_APP\s*\+\s*(\d+)')) {
        $map[$m.Groups[1].Value] = [int]$m.Groups[2].Value
    }
    return $map
}
$baseWm = Messages $baseMsg
$upWm = Messages $upMsg
$ourWm = @($baseWm.Keys | Where-Object { -not $upWm.ContainsKey($_) })
$wmClash = @()
foreach ($name in $ourWm) {
    $value = $baseWm[$name]
    $usedBy = @($upWm.GetEnumerator() | Where-Object { $_.Value -eq $value } | ForEach-Object { "$($_.Key)(+$value)" })
    if ($usedBy.Count) { $wmClash += ("{0}(+{1}) 撞 {2}" -f $name, $value, ($usedBy -join ',')) }
}
$upWmMax = ($upWm.Values | Measure-Object -Maximum).Maximum
Record '私有消息 WM_APP 号' ($wmClash.Count -eq 0) $(if ($wmClash.Count) { $wmClash -join '；' } else { "我们用 " + (($ourWm | ForEach-Object { "WM_APP+$($baseWm[$_])" }) -join ' / ') + "，官方消息最大 +$upWmMax" })

# --- 5. 版本号分工（产品版本=日期，预览包地址=官方基线） -----------------------
Info '5/5 版本号分工'
$cmake = Get-Content 'CMakeLists.txt' -Raw
$baseVersion = (Get-Content 'version.txt' -Raw).Trim()
$versionOk = $cmake -match 'PULSE_UPSTREAM_VERSION' -and $cmake -match 'PULSE_VERSION_OVERRIDE' -and $baseVersion -match '^\d+\.\d+\.\d+$'
Record '版本号分工' $versionOk $(if ($versionOk) { "version.txt = $baseVersion（预览包地址用），产品版本由 build_filep.ps1 打当天日期" } else { 'CMake 的 PULSE_UPSTREAM_VERSION / version.txt 被改坏了，预览包会 404' })

Write-Host ''

# --- l10n + 功能守卫 ----------------------------------------------------------
if (-not $SkipL10n) {
    Info '重新生成 l10n 字符串表'
    & py -3.12 tools/l10n/gen_strings.py | Out-Null
    if ($LASTEXITCODE -ne 0) { Bad 'gen_strings.py 失败（多半是重复 id）'; $fail++ }
    & py -3.12 tools/l10n/gen_strings.py --check | Out-Null
    if ($LASTEXITCODE -ne 0) { Bad '生成结果与 catalog 不一致'; $fail++ }
}

Info '自有功能守卫'
& pwsh -NoProfile -File (Join-Path $PSScriptRoot 'check-fork-features.ps1')
if ($LASTEXITCODE -ne 0) { $fail++ }

if ($Build) {
    $builder = Join-Path $BuildDir 'build_filep.ps1'
    if (Test-Path $builder) {
        Info "编译：$builder"
        & pwsh -NoProfile -File $builder
        if ($LASTEXITCODE -ne 0) { Bad '编译失败'; $fail++ }
    } else {
        Bad "找不到 $builder（用 -BuildDir 指定）"
        $fail++
    }
}

if ($Smoke) {
    Info '冒烟：5 个设置页 + 2 个回归测试'
    & pwsh -NoProfile -File (Join-Path $PSScriptRoot 'smoke-fork.ps1')
    if ($LASTEXITCODE -ne 0) { Bad '冒烟不合格'; $fail++ }
}

Write-Host ''
$rows | Format-Table -AutoSize | Out-String -Width 220 | Write-Host
if ($fail -gt 0) {
    Bad "有 $fail 项没通过，先处理再提交/推送。"
    exit 1
}
Good '号段与 catalog 校验通过。'
exit 0
