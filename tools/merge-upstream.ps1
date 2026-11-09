<#
.SYNOPSIS
  把官方 Pulse 的新提交合并进 FileP。

.DESCRIPTION
  默认只做「看看有什么新东西」：fetch 之后列出官方新增的提交、改了哪些文件、
  以及这次很可能冲突的位置。加 -Merge 才真的合并；-Build 合并后编译；
  -Push 合并后推 origin（远端 Gitea）、local（本机 Gitea）、github 三个远端。

.EXAMPLE
  pwsh -File tools/merge-upstream.ps1

.EXAMPLE
  pwsh -File tools/merge-upstream.ps1 -Merge -Build -Push
#>
[CmdletBinding()]
param(
    [switch]$Merge,
    [switch]$Build,
    [switch]$Push,
    [string]$Upstream = 'upstream',
    [string]$Branch = 'main',
    # 编译入口（必须是 ASCII 路径：rc.exe 不认中文路径）
    [string]$BuildDir = 'C:\dev\FileP'
)

$ErrorActionPreference = 'Stop'

function Fail([string]$message) { Write-Host $message -ForegroundColor Red; exit 1 }
function Info([string]$message) { Write-Host $message -ForegroundColor Cyan }

$repo = Split-Path -Parent $PSScriptRoot
Set-Location $repo

git rev-parse --is-inside-work-tree *> $null
if ($LASTEXITCODE -ne 0) { Fail "当前目录不是 git 仓库：$repo" }

if (-not (git remote | Select-String -Quiet ('^' + [regex]::Escape($Upstream) + '$'))) {
    Fail "没有名为 $Upstream 的远端。先加一个：git remote add $Upstream https://github.com/jimmgreen/pulse.git"
}

# 直连 github.com 会超时，走本地代理；没配就提醒一句。
$proxy = git config --get 'http.https://github.com/.proxy'
if (-not $proxy) {
    Write-Host '提示：没有为 github.com 配置代理（直连大概率超时）。可执行：' -ForegroundColor Yellow
    Write-Host '  git config --local http.https://github.com/.proxy http://127.0.0.1:7900' -ForegroundColor Yellow
}

Info "拉取 $Upstream ..."
git fetch $Upstream --prune
if ($LASTEXITCODE -ne 0) { Fail 'fetch 失败：检查网络 / 代理。' }

$upRef = "$Upstream/$Branch"
git rev-parse --verify --quiet $upRef *> $null
if ($LASTEXITCODE -ne 0) { Fail "远端没有 $upRef 分支。" }

$newCount = [int](git rev-list --count "$Branch..$upRef")
if ($newCount -eq 0) {
    Info "已是最新：$Branch 里没有缺 $upRef 的提交。"
    exit 0
}

Info "官方领先 $newCount 个提交："
git log --oneline --no-decorate "$Branch..$upRef" | Select-Object -First 40

Write-Host ''
Write-Host '改动最大的文件：' -ForegroundColor Cyan
git diff --stat "$Branch...$upRef" | Select-Object -Last 20

# 本分支改过的地方，官方一动就容易冲突。
# 「别整侧取官方」的文件：整侧 checkout --theirs 会静默丢掉我们的功能，
# 取完必须跑 tools/check-fork-features.ps1 确认没丢东西。
$hotspots = @(
    @{ Path = 'tools/l10n/catalog.tsv';    Hint = '两边都追加；我们的 id 在 6400+，官方 1.0.59 已用到 6390。合并后必须跑 py -3.12 tools/l10n/gen_strings.py' },
    @{ Path = 'src/app/app_commands.cpp';  Hint = '别整侧取官方！工具栏目录（split_two）、命令派发、自定义命令行都在这' },
    @{ Path = 'src/app/app_input.cpp';     Hint = '别整侧取官方！此电脑点击、设置页交互、任务栏滑块、标签 ✕ 松开都在这' },
    @{ Path = 'src/app/app_main.cpp';      Hint = '别整侧取官方！设置目录 Roaming 覆写、截图开关、FileP 标题都在这' },
    @{ Path = 'src/ui/ui_renderer.cpp';    Hint = '别整侧取官方！标题栏品牌、侧栏手动展开、自定义命令页辅助函数都在这' },
    @{ Path = 'src/app/context_menu.h';    Hint = '命令 id：我们的自定义命令在 5700-5799，别被官方新 id 撞上' },
    @{ Path = 'src/ui/settings_page_ids.h'; Hint = '设置页号：自定义命令固定 16（官方占了 13/14/15）' },
    @{ Path = 'src/app/app_messages.h';    Hint = '私有消息：WM_CUSTOM_COMMAND_DONE = WM_APP+200（官方占 122）' },
    @{ Path = 'src/app/app_prefs.cpp';     Hint = '新增的偏好：两边字段都留，JSON 读写也要成对' },
    @{ Path = 'README.md';                 Hint = '保持本仓库版本：git checkout --ours README.md' },
    @{ Path = '.github/workflows';         Hint = '本仓库有意删除了 CI：保持删除，git rm 掉上游恢复的' }
)
$changed = git diff --name-only "$Branch...$upRef"
$hit = $hotspots | Where-Object { $changed -contains $_.Path }
if ($hit) {
    Write-Host ''
    Write-Host '这些是本分支改过、官方也动了的文件，合并时大概率要手工处理：' -ForegroundColor Yellow
    foreach ($h in $hit) { Write-Host ('  {0}' -f $h.Path); Write-Host ('    -> {0}' -f $h.Hint) }
}

if (-not $Merge) {
    Write-Host ''
    Write-Host '只是看看（没有合并）。要真的合并加 -Merge：' -ForegroundColor Cyan
    Write-Host '  pwsh -File tools/merge-upstream.ps1 -Merge -Build -Push'
    exit 0
}

if (git status --porcelain) { Fail '工作区有未提交的改动，先 commit 或处理掉再合并。' }

Info "合并 $upRef ..."
git merge --no-edit $upRef
if ($LASTEXITCODE -ne 0) {
    Write-Host ''
    Write-Host '有冲突，需要手工处理：' -ForegroundColor Yellow
    git diff --name-only --diff-filter=U | ForEach-Object { Write-Host ('  ' + $_) }
    Write-Host ''
    Write-Host '处理办法：' -ForegroundColor Gray
    Write-Host '  git checkout --ours   -- <文件>    保留本仓库的版本（README、ui_*.cpp 常用）' -ForegroundColor Gray
    Write-Host '  git checkout --theirs -- <文件>    采用官方的版本（生成文件、bench 常用）' -ForegroundColor Gray
    Write-Host '  手工编辑后再：git add <文件> && git merge --continue' -ForegroundColor Gray
    Write-Host '想放弃这次合并：git merge --abort' -ForegroundColor Gray
    Write-Host ''
    Write-Host '注意：重新生成的文件不要手改，跑一次 py -3.12 tools/l10n/gen_strings.py' -ForegroundColor Gray
    exit 1
}

if ($changed -contains 'tools/l10n/catalog.tsv') {
    Info 'catalog.tsv 变了，重新生成字符串表 ...'
    py -3.12 tools/l10n/gen_strings.py
    if ($LASTEXITCODE -ne 0) { Fail '生成字符串表失败。' }
}

Info '检查自有功能是否被合并冲掉 ...'
pwsh -NoProfile -File (Join-Path $PSScriptRoot 'check-fork-features.ps1')
if ($LASTEXITCODE -ne 0) {
    Write-Host ''
    Write-Host '有自有功能被官方版本覆盖了，先补回来再编译/推送。' -ForegroundColor Red
    Write-Host '补法：看上面缺的那几项，回到 68df84b1（或任意更早的提交）把对应代码取回来。' -ForegroundColor Gray
    Write-Host '  git show <旧提交>:src/app/app_input.cpp | Select-String -Pattern <锚点> -Context 6,20' -ForegroundColor Gray
    exit 1
}

Info '合并完成。别忘了复验：自动更新是否仍是关闭的、界面是否正常。'

if ($Build) {
    $builder = Join-Path $BuildDir 'build_filep.ps1'
    if (-not (Test-Path $builder)) {
        Write-Host "跳过编译：找不到 $builder（用 -BuildDir 指定）" -ForegroundColor Yellow
    } else {
        Info "编译：$builder"
        pwsh -NoProfile -File $builder
        if ($LASTEXITCODE -ne 0) { Fail '编译失败，先修好再推送。' }
    }
}

if ($Push) {
    Info '推送到 origin / local / github ...'
    git push origin $Branch
    if ($LASTEXITCODE -ne 0) { Fail '推 origin 失败。' }
    git push local $Branch
    if ($LASTEXITCODE -ne 0) { Write-Host '推 local 失败（本机 Gitea 没起来？）' -ForegroundColor Yellow }
    git push github $Branch
    if ($LASTEXITCODE -ne 0) { Write-Host '推 github 失败（代理没开？）' -ForegroundColor Yellow }
}

Info '完成。'