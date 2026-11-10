<#
.SYNOPSIS
  合并官方新版之后，检查「我们自己的功能」是否还在。

.DESCRIPTION
  每条检查 = 一个功能 + 一个必须出现的代码锚点（文件 + 文本）。
  合并官方大改（尤其 app_main.cpp / app_input.cpp / app_commands.cpp /
  ui_renderer.cpp）时，整侧取官方版本很方便，但会把我们的改动一起冲掉。
  这个脚本就是防止那种「编译能过、功能静默消失」的情况。

  锚点只证明代码还在，不能证明行为对；交互仍要手点一遍。

.EXAMPLE
  pwsh -File tools/check-fork-features.ps1
  pwsh -File tools/check-fork-features.ps1 -Fix
#>
[CmdletBinding()]
param(
    # 只列出缺失项，不改变退出码（给 merge-upstream.ps1 的预览用）
    [switch]$WarnOnly
)

$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
Set-Location $repo

# 功能名 / 文件 / 必须出现的文本 / 说明（缺了会怎样）
$checks = @(
    '品牌：窗口与标题栏|src/app/app_main.cpp|L"FileP"|窗口标题回到 Pulse'
    '品牌：标题栏文字|src/ui/ui_renderer.cpp|L"FileP"|标题栏又写 Pulse'
    '品牌：托盘|src/app/tray_controller.cpp|L"FileP"|托盘提示回到 Pulse'
    '品牌：关于页标题|tools/l10n/catalog.tsv|关于 FileP|关于页又写「关于 Pulse」'
    '关于页：本项目主页|src/app/about_info.h|kFilePHomepage|关于页少了「本项目主页」'

    '设置目录：Roaming 覆写|src/app/app_main.cpp|PulseConfigDirOverride() = roaming|设置又写回 %LOCALAPPDATA%\FileP'
    '设置目录：旧文件迁移|src/app/app_main.cpp|MigrateConfigToRoaming()|旧位置的设置不会搬过来'
    'AI 设置目录|src/app/ai_settings.cpp|FileP\\ai|AI 设置回到 Pulse 目录'

    '地址栏：此电脑按钮（命中）|src/ui/ui_hit_test.cpp|HitTestResult::NavHome|按钮在但点不到'
    '地址栏：此电脑按钮（点击）|src/app/app_input.cpp|HitTestResult::NavHome|点了没反应'
    '地址栏：复制按钮开关|src/app/app_prefs.h|show_path_copy|复制按钮无法关闭'
    '侧栏：自动收起开关|src/app/app_prefs.h|auto_collapse_sidebar|窄窗口总会收起'
    '侧栏：手动展开优先|src/ui/ui_renderer.cpp|user_forced_full_|手动展开在窄窗口无效'

    '任务栏标题：字符数限制|src/app/app_window_title.cpp|LimitName|任务栏标题不再截断'
    '任务栏标题：滑块派发|src/app/app_input.cpp|SettingsTaskbarChars|滑块拖不动'
    '变化摘要：悬停可关闭|src/ui/ui_settings_core.cpp|change_hover_row|设置项消失'

    '工具栏：一键左右双栏|src/app/toolbar_tools.h|split_two|一键分屏按钮没了'
    '工具栏：一键分屏派发|src/app/app_commands.cpp|split_two|点了变成弹菜单'
    '收藏夹栏：图标化阈值|src/ui/favorites_bar_layout.h|320.0f|半屏就退化成纯图标'
    '右键菜单：安全三角|src/ui/fluent_menu.cpp|InSafeTriangle|斜向移动二级菜单会被抢'
    '标签页：✕ 松开才关|src/app/app_input.cpp|tabClosePending|变回按下即关'

    '自定义命令：核心|src/app/custom_commands.cpp|RunCustomCommand|命令执行器丢失'
    '自定义命令：设置页|src/app/custom_commands_page.cpp|CommandsPageAction|页面交互丢失'
    '自定义命令：页面填充|src/app/app_runtime.cpp|FillCommandsPage|页面空白'
    '自定义命令：输入框同步|src/app/custom_commands_page.cpp|SyncCommandsPage|名称/步骤输入框不显示'
    '自定义命令：每帧同步调用|src/app/app_main.cpp|SyncCommandsPage|输入框不创建/不跟随滚动'
    '自定义命令：命令表加载|src/app/app_main.cpp|customCommands.Load()|自定义命令全部消失（读到空表）'
    '任务栏滑块：单元格分配|src/ui/settings_layout_sections.h|taskbar_parent_row|任务栏标题设置项变空白'
    '自定义命令：右键行|src/app/app_commands.cpp|AppendCustomCommandRows|右键菜单里没有自定义命令'
    '自定义命令：工具栏钉住|src/app/app_commands.cpp|CustomCommandToolSlot|不能钉到工具栏'
    '自定义命令：自定义工具栏磁贴|src/app/app_commands.cpp|kToolbarCustomBase|自定义工具栏里找不到命令'
    '自定义命令：勾选即钉住|src/app/custom_commands_page.cpp|CustomCommandToolId|勾了工具栏但按钮不出现'
    '自定义命令：执行前确认|src/app/app_commands.cpp|CmdRunConfirmTitle|点了就直接执行，没有确认框'
    '自定义命令：执行线程|src/app/app_commands.cpp|RunCustomCommandAt|命令点了没反应'
    '自定义命令：完成消息|src/app/app_messages.h|WM_CUSTOM_COMMAND_DONE|执行完不刷新'

    '设置页号：自定义命令 = 16|src/ui/settings_page_ids.h|kSettingsPageCommands = 16|页号被官方占用'
    '设置页：侧栏顺序含它|src/ui/ui_renderer_internal.h|kSettingsPageCommands|侧栏看不到入口'
    '号段：字符串 >= 6400|tools/l10n/catalog.tsv|IDS_SETTINGS_SHOW_PATH_COPY|字符串 id 与官方撞号'

    '网络盘：去掉 .local|src/app/lan_device.cpp|ends_with(L".local")|局域网设备进不去'
    '标签：永久失败不重试|src/app/tag_ads_sync.cpp|FILE_NAMED_STREAMS|每次启动都弹标签提示'
    '窗口：无 WS_SYSMENU|src/app/app_main.cpp|WS_SYSMENU|系统标题栏回来了'
)

$missing = @()
foreach ($c in $checks) {
    $parts = $c -split '\|'
    $name = $parts[0]; $file = $parts[1]; $pattern = $parts[2]; $symptom = $parts[3]
    if (-not (Test-Path $file)) { $missing += [pscustomobject]@{ 功能 = $name; 文件 = $file; 症状 = "$symptom（文件不存在）" }; continue }
    $count = (Select-String -Path $file -Pattern $pattern -SimpleMatch -ErrorAction SilentlyContinue | Measure-Object).Count
    if ($count -eq 0) { $missing += [pscustomobject]@{ 功能 = $name; 文件 = $file; 症状 = $symptom } }
}

Write-Host ("检查 {0} 项自有功能 ..." -f $checks.Count) -ForegroundColor Cyan
if ($missing.Count -eq 0) {
    Write-Host '全部锚点在位。' -ForegroundColor Green
    Write-Host '注意：这只证明代码还在，hover / 拖拽 / 右键二级菜单 / 输入框打字仍需手点验证。' -ForegroundColor DarkGray
    exit 0
}

Write-Host ''
Write-Host ("有 {0} 项不见了 —— 多半是合并时整侧取了官方版本：" -f $missing.Count) -ForegroundColor Red
$missing | Format-Table -AutoSize | Out-String -Width 200 | Write-Host
Write-Host '逐项补回后重新运行本脚本，再用 --shot-settings 截图复验页面。' -ForegroundColor Yellow
if ($WarnOnly) { exit 0 }
exit 1