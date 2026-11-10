# 本仓库相对官方的改动清单（自动生成）

基线提交：0856a6556d5c8fc060aeee024074b6b18c0778c6（与 upstream 的共同祖先）

共 70 个非合并提交，每个对应 fork-patches 里的一个补丁。

| # | 补丁 | 日期 | 提交 | 说明 | 涉及文件 |
| --- | --- | --- | --- | --- | --- |
| 1 | 0001-新功能_-改名-FileP-并新增任务栏标题字符数设置.patch | 2026-11-08 | 5eb40530 | 新功能: 改名 FileP 并新增任务栏标题字符数设置 | CMakeLists.txt<br>build_filep.ps1<br>src/app/about_info.cpp<br>src/app/actions/action_settings.cpp<br>src/app/actions/action_settings.h<br>src/app/app_input.cpp<br>src/app/app_prefs.cpp<br>src/app/app_prefs.h<br>src/app/app_runtime.cpp<br>src/app/app_window_title.cpp<br>src/app/app_window_title.h<br … |
| 2 | 0002-重构_-界面品牌统一改为-FileP.patch | 2026-11-08 | 67ea7944 | 重构: 界面品牌统一改为 FileP | src/common/strings.rcinc<br>src/common/strings_zh_tw.rcinc<br>src/common/zh_hant_table.inc<br>src/ui/ui_renderer.cpp<br>tools/l10n/catalog.tsv |
| 3 | 0003-新功能_-任务栏标题字符数改为-1-40-加减步进并移到外观页.patch | 2026-11-08 | 32c56d34 | 新功能: 任务栏标题字符数改为 1-40 加减步进并移到外观页 | src/app/actions/action_settings.cpp<br>src/app/app_input.cpp<br>src/app/app_prefs.h<br>src/app/app_window_title.cpp<br>src/app/scroll_hover_taskbar_test.cpp<br>src/app/settings_controller.cpp<br>src/app/settings_controller.h<br>src/common/localization.h<br>src/common/string_ids.h<br>src/common/strin … |
| 4 | 0004-新功能_-自定义命令核心（模型-_-commands.json-_-执行器）.patch | 2026-11-08 | ce4d02fc | 新功能: 自定义命令核心（模型 / commands.json / 执行器） | src/app/custom_commands.cpp<br>src/app/custom_commands.h |
| 5 | 0005-新功能_-自定义命令接入右键菜单并补核心回归测试.patch | 2026-11-08 | 5e260499 | 新功能: 自定义命令接入右键菜单并补核心回归测试 | CMakeLists.txt<br>src/app/app_commands.cpp<br>src/app/app_core.h<br>src/app/app_main.cpp<br>src/app/app_state.h<br>src/app/context_menu.h<br>src/app/custom_commands.cpp<br>src/bench/custom_commands_test.cpp |
| 6 | 0006-新功能_-任务栏标题字符数改成滑块，1-12-档，最后一档不限.patch | 2026-11-08 | b6e6a7ba | 新功能: 任务栏标题字符数改成滑块，1-12 档，最后一档不限 | src/app/app_input.cpp<br>src/app/app_prefs.h<br>src/app/settings_controller.cpp<br>src/app/settings_controller.h<br>src/common/localization.h<br>src/common/string_ids.h<br>src/common/strings.rcinc<br>src/common/strings_zh_tw.rcinc<br>src/common/zh_hant_table.inc<br>src/ui/settings_layout_sections.h< … |
| 7 | 0007-修复_-任务栏标题截断只在末尾加省略号.patch | 2026-11-08 | 6045eac4 | 修复: 任务栏标题截断只在末尾加省略号 | src/app/app_window_title.cpp<br>src/app/scroll_hover_taskbar_test.cpp<br>src/common/strings.rcinc<br>src/common/strings_zh_tw.rcinc<br>src/common/zh_hant_table.inc<br>tools/l10n/catalog.tsv |
| 8 | 0008-新功能_-自定义命令编辑器（名称-_-出现位置-_-步骤增删改移）.patch | 2026-11-08 | ebf16f6d | 新功能: 自定义命令编辑器（名称 / 出现位置 / 步骤增删改移） | src/app/app_commands.cpp<br>src/app/app_main.cpp<br>src/app/app_state.h<br>src/app/context_menu.h<br>src/ui/custom_commands_dialog.cpp<br>src/ui/custom_commands_dialog.h |
| 9 | 0009-新功能_-自定义命令纳入设置备份与迁移.patch | 2026-11-08 | 81500b00 | 新功能: 自定义命令纳入设置备份与迁移 | src/app/settings_transfer.cpp<br>src/bench/settings_transfer_test.cpp |
| 10 | 0010-重构_-设置文件搬到-%APPDATA%_FileP，旧位置首次启动自动迁移.patch | 2026-11-08 | 19767a93 | 重构: 设置文件搬到 %APPDATA%\FileP，旧位置首次启动自动迁移 | src/app/app_main.cpp<br>src/app/app_prefs.cpp<br>src/app/context_menu_prefs.cpp<br>src/app/custom_commands.cpp<br>src/app/places.cpp<br>src/app/saved_search.cpp<br>src/app/session.cpp<br>src/app/session.h<br>src/app/settings_transfer_ui.cpp<br>src/app/tag_rules.cpp<br>src/common/app_dirs.h |
| 11 | 0011-界面_-标题移到状态栏左下角，左上角只留侧栏开关.patch | 2026-11-08 | 9f75fdb2 | 界面: 标题移到状态栏左下角，左上角只留侧栏开关 | src/app/app_prefs.cpp<br>src/app/app_prefs.h<br>src/app/app_runtime.cpp<br>src/app/favorites_bar.cpp<br>src/app/settings_controller.cpp<br>src/common/localization.h<br>src/common/string_ids.h<br>src/common/strings.rcinc<br>src/common/strings_zh_tw.rcinc<br>src/common/zh_hant_table.inc<br>src/ui/sett … |
| 12 | 0012-新功能_-设置里新增「自定义命令」页.patch | 2026-11-08 | 9519075d | 新功能: 设置里新增「自定义命令」页 | src/app/actions/action_settings.cpp<br>src/app/app_input.cpp<br>src/common/localization.h<br>src/common/string_ids.h<br>src/common/strings.rcinc<br>src/common/strings_zh_tw.rcinc<br>src/common/zh_hant_table.inc<br>src/ui/settings_layout_sections.h<br>src/ui/ui_hit_test.cpp<br>src/ui/ui_renderer_inte … |
| 13 | 0013-界面_-侧边栏各分组的条目缩进一级，形成层级.patch | 2026-11-08 | 6c1ed784 | 界面: 侧边栏各分组的条目缩进一级，形成层级 | src/ui/ui_renderer_internal.h |
| 14 | 0014-修复_-自定义命令设置页不显示；新增设置页截图开关；复制路径给提示.patch | 2026-11-08 | 69ee05a4 | 修复: 自定义命令设置页不显示；新增设置页截图开关；复制路径给提示 | src/app/app_commands.cpp<br>src/app/app_main.cpp<br>src/app/app_state.h<br>src/app/settings_controller.cpp<br>src/common/localization.h<br>src/common/string_ids.h<br>src/common/strings.rcinc<br>src/common/strings_zh_tw.rcinc<br>src/common/zh_hant_table.inc<br>src/ui/settings_page_ids.h<br>src/ui/ui_ … |
| 15 | 0015-新功能_-工具栏新增「左右双栏」一键按钮.patch | 2026-11-08 | e1406404 | 新功能: 工具栏新增「左右双栏」一键按钮 | src/app/app_commands.cpp<br>src/app/toolbar_tools.h |
| 16 | 0016-修复_-自定义命令点下去变成「新建-Excel-工作表」.patch | 2026-11-08 | 46ee5c92 | 修复: 自定义命令点下去变成「新建 Excel 工作表」 | src/app/context_menu.h |
| 17 | 0017-重构_-自定义命令编辑器并入设置页，不再是独立窗口.patch | 2026-11-08 | f5e37687 | 重构: 自定义命令编辑器并入设置页，不再是独立窗口 | src/app/app_main.cpp<br>src/app/app_runtime.cpp<br>src/app/app_runtime.h<br>src/app/context_menu.h<br>src/ui/custom_commands_dialog.cpp<br>src/ui/custom_commands_dialog.h<br>src/ui/settings_layout_sections.h<br>src/ui/ui_hit_test.cpp<br>src/ui/ui_renderer.cpp<br>src/ui/ui_renderer.h<br>src/ui/ui_set … |
| 18 | 0018-新功能_-自定义命令可以加到工具栏.patch | 2026-11-08 | d2254b2c | 新功能: 自定义命令可以加到工具栏 | src/app/app_commands.cpp<br>src/app/app_main.cpp<br>src/app/toolbar_tools.h |
| 19 | 0019-界面_-「左右分屏」一键按钮换成两栏图标.patch | 2026-11-08 | 78163308 | 界面: 「左右分屏」一键按钮换成两栏图标 | src/app/app_commands.cpp<br>src/ui/command_icons.h |
| 20 | 0020-重构_-自定义命令页改为设置样式自绘界面（进行中）.patch | 2026-11-08 | 00f0348f | 重构: 自定义命令页改为设置样式自绘界面（进行中） | src/app/app_input.cpp<br>src/app/app_main.cpp<br>src/app/app_runtime.cpp<br>src/app/app_state.h<br>src/app/custom_commands_page.cpp<br>src/app/custom_commands_page.h<br>src/common/localization.h<br>src/common/string_ids.h<br>src/common/strings.rcinc<br>src/common/strings_zh_tw.rcinc<br>src/common/zh … |
| 21 | 0021-修复_-「左右分屏」按钮点了没反应.patch | 2026-11-08 | eaefce89 | 修复: 「左右分屏」按钮点了没反应 | src/app/app_commands.cpp<br>src/app/app_main.cpp<br>src/app/app_state.h<br>src/app/context_menu.h |
| 22 | 0022-修复(进行中)_-自定义命令页的两个输入框没有创建出来.patch | 2026-11-08 | d3ad7275 | 修复(进行中): 自定义命令页的两个输入框没有创建出来 | src/app/custom_commands_page.cpp |
| 23 | 0023-修复_-自定义命令页的名称_值输入框显示不出来.patch | 2026-11-08 | ef454ecc | 修复: 自定义命令页的名称/值输入框显示不出来 | src/app/custom_commands_page.cpp |
| 24 | 0024-功能_-左右分屏按钮变成开关，分屏时图标变蓝.patch | 2026-11-08 | d6b2c0d7 | 功能: 左右分屏按钮变成开关，分屏时图标变蓝 | src/app/app_commands.cpp |
| 25 | 0025-修复_-自定义命令页的输入框随页面滚动隐藏.patch | 2026-11-08 | 8cf80d89 | 修复: 自定义命令页的输入框随页面滚动隐藏 | src/app/custom_commands_page.cpp |
| 26 | 0026-功能_-自定义命令页-Tab-在名称与值之间切换.patch | 2026-11-08 | b88b46aa | 功能: 自定义命令页 Tab 在名称与值之间切换 | src/app/custom_commands_page.cpp |
| 27 | 0027-文档_-README-改为-FileP-说明，标注上游来源.patch | 2026-11-08 | 966259d0 | 文档: README 改为 FileP 说明，标注上游来源 | README.en.md<br>README.md |
| 28 | 0028-构建_-新增-tools_merge-upstream.ps1，一键跟随上游.patch | 2026-11-08 | fa1193a2 | 构建: 新增 tools/merge-upstream.ps1，一键跟随上游 | README.md<br>tools/merge-upstream.ps1 |
| 29 | 0029-界面_-自定义命令页的名称_值改成表单式输入框.patch | 2026-11-08 | c23acb73 | 界面: 自定义命令页的名称/值改成表单式输入框 | src/app/app_main.cpp<br>src/app/custom_commands_page.cpp<br>src/ui/settings_layout_sections.h<br>src/ui/ui_renderer_internal.h<br>src/ui/ui_settings_core.cpp<br>src/ui/ui_view_model.h |
| 30 | 0030-界面_-自定义命令输入框换成系统字体；去掉右键入口与独立编辑窗口.patch | 2026-11-08 | 6dbcbf26 | 界面: 自定义命令输入框换成系统字体；去掉右键入口与独立编辑窗口 | .github/workflows/l10n.yml<br>.github/workflows/preview-pack-archive.yml<br>.github/workflows/preview-pack-ffmpeg.yml<br>.github/workflows/preview-pack-images.yml<br>.github/workflows/preview-pack-raw.yml<br>.github/workflows/release.yml<br>src/app/app_commands.cpp<br>src/app/app_input.cpp<br>src/ap … |
| 31 | 0031-界面_-自定义命令的两个输入框改用合成器绘制文字.patch | 2026-11-08 | c13d92c0 | 界面: 自定义命令的两个输入框改用合成器绘制文字 | src/app/custom_commands_page.cpp |
| 32 | 0032-修复_-重新生成-zh_hant_table.inc（l10n-检查失败的原因）.patch | 2026-11-08 | 7f56af25 | 修复: 重新生成 zh_hant_table.inc（l10n 检查失败的原因） | src/common/zh_hant_table.inc |
| 33 | 0033-修复_-右键菜单恢复自定义命令，按每条命令的「出现位置」显隐.patch | 2026-11-08 | 5d43938b | 修复: 右键菜单恢复自定义命令，按每条命令的「出现位置」显隐 | src/app/app_commands.cpp |
| 34 | 0034-界面_-自定义命令页改成主从双栏（方案-A-第一步）.patch | 2026-11-08 | 47b3c382 | 界面: 自定义命令页改成主从双栏（方案 A 第一步） | src/ui/settings_layout_sections.h<br>src/ui/ui_hit_test.cpp<br>src/ui/ui_renderer_internal.h<br>src/ui/ui_settings_core.cpp |
| 35 | 0035-界面_-自定义命令页第二步——步骤行内编辑、将执行摘要.patch | 2026-11-08 | bdf9767f | 界面: 自定义命令页第二步——步骤行内编辑、将执行摘要 | src/app/custom_commands_page.cpp<br>src/ui/settings_layout_sections.h<br>src/ui/ui_renderer.cpp<br>src/ui/ui_renderer.h<br>src/ui/ui_renderer_internal.h<br>src/ui/ui_settings_core.cpp<br>src/ui/ui_view_model.h |
| 36 | 0036-界面_-步骤行内编辑时保留「序号-+-类型」，输入框收窄成字段宽度.patch | 2026-11-08 | 278d1f3b | 界面: 步骤行内编辑时保留「序号 + 类型」，输入框收窄成字段宽度 | src/app/app_main.cpp<br>src/app/custom_commands_page.cpp<br>src/ui/settings_layout_sections.h<br>src/ui/ui_settings_core.cpp<br>src/ui/ui_view_model.h |
| 37 | 0037-界面_-自定义命令页收尾——行内删除、行内排序、去掉底部按钮条.patch | 2026-11-08 | 4aa42109 | 界面: 自定义命令页收尾——行内删除、行内排序、去掉底部按钮条 | src/app/custom_commands_page.cpp<br>src/ui/settings_layout_sections.h<br>src/ui/ui_hit_test.cpp<br>src/ui/ui_renderer_internal.h<br>src/ui/ui_settings_core.cpp<br>src/ui/ui_view_model.h |
| 38 | 0038-新功能_-地址栏复制按钮可关闭（设置-→-外观）.patch | 2026-11-08 | 1016af0f | 新功能: 地址栏复制按钮可关闭（设置 → 外观） | src/app/app_prefs.cpp<br>src/app/app_prefs.h<br>src/app/app_runtime.cpp<br>src/app/settings_controller.cpp<br>src/common/localization.h<br>src/common/string_ids.h<br>src/common/strings.rcinc<br>src/common/strings_zh_tw.rcinc<br>src/common/zh_hant_table.inc<br>src/ui/settings_layout_sections.h<br>src … |
| 39 | 0039-新功能_-设置里增加「自动收起侧栏」，并修好窄窗口下展开按钮消失.patch | 2026-11-08 | 6dfc9fc7 | 新功能: 设置里增加「自动收起侧栏」，并修好窄窗口下展开按钮消失 | src/app/app_prefs.cpp<br>src/app/app_prefs.h<br>src/app/app_runtime.cpp<br>src/app/settings_controller.cpp<br>src/common/localization.h<br>src/common/string_ids.h<br>src/common/strings.rcinc<br>src/common/strings_zh_tw.rcinc<br>src/common/zh_hant_table.inc<br>src/ui/settings_layout_sections.h<br>src … |
| 40 | 0040-修复_-手动展开侧栏在窄窗口下无效；复制路径按钮被收藏栏挡掉.patch | 2026-11-08 | f263d402 | 修复: 手动展开侧栏在窄窗口下无效；复制路径按钮被收藏栏挡掉 | src/ui/ui_renderer.cpp<br>src/ui/ui_renderer.h |
| 41 | 0041-修复_-收藏夹栏过早退化成纯图标.patch | 2026-11-08 | 0234a315 | 修复: 收藏夹栏过早退化成纯图标 | src/ui/favorites_bar_layout.h |
| 42 | 0042-新功能_-地址栏左侧加「此电脑」按钮（可设置关闭）.patch | 2026-11-08 | 9bd70db7 | 新功能: 地址栏左侧加「此电脑」按钮（可设置关闭） | src/app/app_input.cpp<br>src/app/app_prefs.cpp<br>src/app/app_prefs.h<br>src/app/app_runtime.cpp<br>src/app/settings_controller.cpp<br>src/common/localization.h<br>src/common/string_ids.h<br>src/common/strings.rcinc<br>src/common/strings_zh_tw.rcinc<br>src/common/zh_hant_table.inc<br>src/ui/settings … |
| 43 | 0043-修复_-全面代码审查后的缺陷修复.patch | 2026-11-09 | 6de130de | 修复: 全面代码审查后的缺陷修复 | src/app/actions/action_inventory.h<br>src/app/actions/action_state.cpp<br>src/app/ai_agent_settings.cpp<br>src/app/app_commands.cpp<br>src/app/app_commands.h<br>src/app/app_input.cpp<br>src/app/app_main.cpp<br>src/app/app_messages.h<br>src/app/app_state.h<br>src/app/custom_commands.cpp<br>src/app/cu … |
| 44 | 0044-修复_-第二轮审查（AI-工具层、更新_安装、标签托盘胶水层）.patch | 2026-11-09 | daa1e723 | 修复: 第二轮审查（AI 工具层、更新/安装、标签托盘胶水层） | src/app/ai_agent_rename.cpp<br>src/app/ai_broker.cpp<br>src/app/ai_cleanup_scan.cpp<br>src/app/ai_model_list.cpp<br>src/app/app_main.cpp<br>src/app/app_runtime.cpp<br>src/app/app_runtime.h<br>src/app/app_updates.cpp<br>src/app/git_ui.cpp<br>src/app/shell_window_sync.cpp<br>src/setup/setup_common.cpp … |
| 45 | 0045-修复_-任务栏标题的两个滑块不能拖动.patch | 2026-11-09 | 0a571715 | 修复: 任务栏标题的两个滑块不能拖动 | src/app/settings_controller.cpp |
| 46 | 0046-修复_-局域网设备打开时多出-.local-后缀导致进不去.patch | 2026-11-09 | 4d76d835 | 修复: 局域网设备打开时多出 .local 后缀导致进不去 | src/app/lan_device.cpp |
| 47 | 0047-修复_-右键菜单增加安全三角，斜向移动到二级菜单不再被抢选中.patch | 2026-11-09 | ba3aaf67 | 修复: 右键菜单增加安全三角，斜向移动到二级菜单不再被抢选中 | src/ui/fluent_menu.cpp<br>src/ui/fluent_menu.h |
| 48 | 0048-界面_-变化记录并入文件列表页的修改时间提示区.patch | 2026-11-09 | c1f3ec38 | 界面: 变化记录并入文件列表页的修改时间提示区 | src/common/strings.rcinc<br>src/common/strings_zh_tw.rcinc<br>src/common/zh_hant_table.inc<br>src/ui/settings_layout_sections.h<br>src/ui/ui_settings_core.cpp<br>tools/l10n/catalog.tsv |
| 49 | 0049-新功能_-悬停「最近变化」的详情弹窗可关闭（设置-→-文件列表）.patch | 2026-11-09 | 3e7469b2 | 新功能: 悬停「最近变化」的详情弹窗可关闭（设置 → 文件列表） | src/app/app_change_tracking.cpp<br>src/app/app_prefs.cpp<br>src/app/app_prefs.h<br>src/app/app_runtime.cpp<br>src/app/settings_controller.cpp<br>src/common/localization.h<br>src/common/string_ids.h<br>src/common/strings.rcinc<br>src/common/strings_zh_tw.rcinc<br>src/common/zh_hant_table.inc<br>src/u … |
| 50 | 0050-交互_-最小化_最大化_关闭改为松开鼠标才生效.patch | 2026-11-09 | 2b1e57bf | 交互: 最小化/最大化/关闭改为松开鼠标才生效 | src/app/app_main.cpp |
| 51 | 0051-交互_-标签上的关闭按钮改为松开鼠标才生效.patch | 2026-11-09 | 54a27fa6 | 交互: 标签上的关闭按钮改为松开鼠标才生效 | src/app/app_input.cpp<br>src/app/app_state.h |
| 52 | 0052-修复_-窄窗口下左上角按钮变成弹出「改名_换颜色」菜单.patch | 2026-11-09 | 29c14f6e | 修复: 窄窗口下左上角按钮变成弹出「改名/换颜色」菜单 | src/ui/ui_hit_test.cpp<br>src/ui/ui_renderer.cpp |
| 53 | 0053-修复_-最小化_最大化_关闭单击无反应.patch | 2026-11-09 | da434dda | 修复: 最小化/最大化/关闭单击无反应 | src/app/app_main.cpp |
| 54 | 0054-回退_-标题栏三个按钮恢复按下即执行.patch | 2026-11-09 | 7ef43509 | 回退: 标题栏三个按钮恢复按下即执行 | src/app/app_main.cpp<br>src/app/app_state.h |
| 55 | 0055-界面_-关于页区分本项目与原项目，左下角改显示编译日期.patch | 2026-11-09 | bebef93a | 界面: 关于页区分本项目与原项目，左下角改显示编译日期 | src/app/about_info.h<br>src/app/app_runtime.cpp<br>src/app/settings_ui.cpp<br>src/common/localization.h<br>src/common/string_ids.h<br>src/common/strings.rcinc<br>src/common/strings_zh_tw.rcinc<br>src/common/zh_hant_table.inc<br>src/ui/ui_hit_test.cpp<br>src/ui/ui_renderer_internal.h<br>src/ui/ui_set … |
| 56 | 0056-文案_-托盘提示、窗口标题、错误框标题从-Pulse-改为-FileP.patch | 2026-11-09 | fdd6a224 | 文案: 托盘提示、窗口标题、错误框标题从 Pulse 改为 FileP | src/app/app_main.cpp<br>src/app/tray_controller.cpp<br>src/common/strings.rcinc<br>tools/l10n/catalog.tsv |
| 57 | 0057-修复_-网络盘文件标签不再每次启动都重试并弹提示.patch | 2026-11-09 | 20ff1c31 | 修复: 网络盘文件标签不再每次启动都重试并弹提示 | src/app/tag_ads_sync.cpp |
| 58 | 0058-修复_-隐藏已知扩展名时，重命名框只编辑主名.patch | 2026-11-09 | d888591e | 修复: 隐藏已知扩展名时，重命名框只编辑主名 | src/app/app_hosted_edit.cpp<br>src/app/app_state.h |
| 59 | 0059-构建_-自有字符串-ID-迁到-6400+，避开官方-1.0.59-占用的号段.patch | 2026-11-09 | 68df84b1 | 构建: 自有字符串 ID 迁到 6400+，避开官方 1.0.59 占用的号段 | src/common/string_ids.h<br>tools/l10n/catalog.tsv |
| 60 | 0060-修复_-合并后标题栏品牌文字仍显示-Pulse.patch | 2026-11-09 | a5a8a526 | 修复: 合并后标题栏品牌文字仍显示 Pulse | src/ui/ui_renderer.cpp |
| 61 | 0061-文档_-新增-docs_FORK-CHANGES.md，逐条列出与官方-Pulse-的差异.patch | 2026-11-09 | 86382128 | 文档: 新增 docs/FORK-CHANGES.md，逐条列出与官方 Pulse 的差异 | docs/FORK-CHANGES.md<br>src/app/app_main.cpp |
| 62 | 0062-文档_-更正设置项所在页面（四个开关都在外观页）.patch | 2026-11-09 | 2a5eb7ad | 文档: 更正设置项所在页面（四个开关都在外观页） | docs/FORK-CHANGES.md |
| 63 | 0063-修复_-合并后设置又写回了-%LOCALAPPDATA%_FileP.patch | 2026-11-09 | dd323496 | 修复: 合并后设置又写回了 %LOCALAPPDATA%\FileP | src/app/ai_settings.cpp<br>src/app/app_main.cpp |
| 64 | 0064-清理_-删掉官方已提供的-AI-入口开关，并补回合并时丢失的六处改动.patch | 2026-11-10 | 73b34de1 | 清理: 删掉官方已提供的 AI 入口开关，并补回合并时丢失的六处改动 | docs/FORK-CHANGES.md<br>src/app/app_commands.cpp<br>src/app/app_input.cpp<br>src/app/app_prefs.cpp<br>src/app/app_prefs.h<br>src/app/app_runtime.cpp<br>src/app/settings_controller.cpp<br>src/app/toolbar_tools.h<br>src/common/strings.rcinc<br>src/common/strings_zh_tw.rcinc<br>src/common/zh_hant_table … |
| 65 | 0065-文档_-新增跟随官方更新的操作手册和自有功能守卫脚本.patch | 2026-11-10 | 51a7a706 | 文档: 新增跟随官方更新的操作手册和自有功能守卫脚本 | docs/FORK-CHANGES.md<br>docs/UPSTREAM-MERGE.md<br>tools/check-fork-features.ps1<br>tools/merge-upstream.ps1 |
| 66 | 0066-工具_-新增一键冒烟-smoke-fork.ps1，并接进-merge-upstream.ps1-的--Smoke.patch | 2026-11-10 | 2a9ee742 | 工具: 新增一键冒烟 smoke-fork.ps1，并接进 merge-upstream.ps1 的 -Smoke | docs/UPSTREAM-MERGE.md<br>tools/merge-upstream.ps1<br>tools/smoke-fork.ps1 |
| 67 | 0067-修复_-去掉热键提权弹窗、此电脑按钮错位、品牌移到左下角.patch | 2026-11-10 | 1261bd69 | 修复: 去掉热键提权弹窗、此电脑按钮错位、品牌移到左下角 | docs/FORK-CHANGES.md<br>src/app/shell_window_sync.cpp<br>src/ui/toolbar_layout.h<br>src/ui/ui_renderer.cpp |
| 68 | 0068-修复_-任务栏标题设置项空白、标签页与窗口多余空行、自定义命令输入框.patch | 2026-11-10 | a0ca61e1 | 修复: 任务栏标题设置项空白、标签页与窗口多余空行、自定义命令输入框 | src/app/app_main.cpp<br>src/ui/settings_layout_sections.h<br>tools/check-fork-features.ps1 |
| 69 | 0069-修复_-自定义命令勾了「工具栏」却找不到按钮.patch | 2026-11-10 | 778217b7 | 修复: 自定义命令勾了「工具栏」却找不到按钮 | src/app/app_commands.cpp<br>src/app/custom_commands_page.cpp<br>tools/check-fork-features.ps1 |
| 70 | 0070-修复_-设置写入前自动滚存备份（app.json.bak1…bak5）.patch | 2026-11-10 | 6ea69fb7 | 修复: 设置写入前自动滚存备份（app.json.bak1…bak5） | docs/FORK-CHANGES.md<br>docs/UPSTREAM-MERGE.md<br>src/app/app_prefs.cpp |

## 怎么用

1. 合并官方新版后先跑 tools/check-fork-features.ps1，看哪几项没了。
2. 在本表里搜对应的功能关键词，找到补丁编号。
3. 只想抄代码：git apply --3way fork-patches\NNNN-xxx.patch（冲突就手工挑）；
   只想看当时怎么改的：git show <提交>。
4. 不要整包 git apply 全部补丁 —— 它们是在旧的上游代码上生成的，会大面积冲突。
