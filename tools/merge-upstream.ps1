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
$hotspots = @(
    @{ Path = 'tools/l10n/catalog.tsv';   Hint = '两边都追加即可；合并后必须跑 py -3.12 tools/l10n/gen_strings.py' },
    @{ Path = 'src/app/app_commands.cpp'; Hint = '工具栏目录 / 命令派发 / 自定义命令：保留自己的块，合入官方的块' },
    @{ Path = 'src/app/context_menu.h';   Hint = '命令 id 区间：5200-5299 是自定义命令，5400+ 是新建按钮，别被官方新 id 撞上' },
    @{ Path = 'src/ui/ui_renderer.cpp';   Hint = '标题栏 / 状态栏品牌：冲突时保留自己的布局' },
    @{ Path = 'src/ui/ui_sidebar.cpp';    Hint = '侧边栏缩进：保留 item.indent 那段' },
    @{ Path = 'src/ui/ui_toolbar.cpp';    Hint = '地址栏复制路径按钮：保留自己的' },
    @{ Path = 'src/app/app_prefs.cpp';    Hint = '新增的偏好（show_ai_entry 等）：两边字段都留' },
    @{ Path = 'README.md';                Hint = '保持本仓库版本：git checkout --ours README.md' }
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