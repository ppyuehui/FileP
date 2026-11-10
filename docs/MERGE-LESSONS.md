# 合并官方版本踩坑记录

> 一次 1.0.59 合并的完整复盘。**流程**见 [UPSTREAM-MERGE.md](UPSTREAM-MERGE.md)，
> **功能清单**见 [FORK-CHANGES.md](FORK-CHANGES.md)，**自动检查**见 `tools/check-fork-features.ps1`。

## 一句话结论

> 合并本身不难，难的是**「编译能过、功能静默消失」**。
> 官方大改的文件一旦整侧取官方版本，我们的改动会安静地不见 —— 直到用户点下去才发现。

---

## 一、坑清单（现象 → 根因 → 现在的护栏）

### 1. 整侧取官方版本 → 功能消失（最大的一条）

| 现象 | 根因 |
| --- | --- |
| 设置写回 `%LOCALAPPDATA%` | `app_main.cpp` 里「设置目录覆写 + 旧文件迁移」被官方版本顶掉 |
| 地址栏「此电脑」点了没反应 | `app_input.cpp` 里的 `NavHome` 点击分支没了 |
| 自定义命令页点什么都没反应 | `app_input.cpp` 里的 `CommandsPageAction` 派发没了 |
| 任务栏标题两个滑块拖不动 | 同上，`SettingsTaskbarChars` 分支没了 |
| 标签页 ✕ 变回按下即关 | `app_input.cpp` 的 `tabClosePending` 没了 |
| 工具栏「左右双栏」按钮消失 | `app_commands.cpp` 的 `split_two` 目录项 / `ToolbarPinActions` 分支没了 |
| 自定义命令全部消失 | `app_main.cpp` 的 `customCommands.Load()` 没了（运行时命令表是空的） |
| 自定义命令输入框打不了字 | `app_main.cpp` 每帧调用的 `SyncCommandsPage` 没了（原生输入框从未创建） |
| 自定义工具栏里找不到命令 | `app_commands.cpp` 的 `kToolbarCustomBase` 磁贴没了 |

**护栏**：`tools/check-fork-features.ps1`（40 项锚点，合并后自动跑，缺了就停）。
**规则**：`app_main.cpp`、`app_input.cpp`、`app_commands.cpp`、`ui_renderer.cpp` 四个文件**永远逐块合并**。

### 2. 号段撞车

| 类型 | 官方 1.0.59 占了 | 我们迁到 | 撞了会怎样 |
| --- | --- | --- | --- |
| 字符串 id | 5670–5690（已到 6390） | 6400–6445 | 文案串行、`gen_strings.py --check` 失败 |
| 设置页号 | 13 / 14 / 15 | 自定义命令 = 16 | 点「自定义命令」进 AI 助手页 |
| 命令 id | 5199 / 5200 | 5700–5799 | 点自定义命令变成「找重复文件」 |
| 私有消息 | `WM_APP+122` | `WM_APP+200` | 收到别的模块的消息 |
| 工具栏切换号 | 81（排列窗口） | 82–85 | **点 A 开关改了 B 设置**（真发生过） |

**护栏**：合并后 grep 一遍 `index=` / `\t64xx\t` / `WM_APP +`；`static_assert` 区间检查保留。

### 3. 布局块重复 → 页面多出空白

`settings_layout_sections.h` 里「标签页」分组被分配两次（官方一块 + 我们遗留一块），
多出四行什么都没画的空白，用户看到的就是一大块空盒子。

**规则**：合并后如果某页出现无名的空白，先搜该页的 `section(` 是否重复。

### 4. 只删了一半的布局代码 → 设置项变空白

任务栏标题那两行，我只保留了 `row()`，删掉了 `segments(...)` 的单元格分配。
结果标签画到屏幕外（x = -12），滑块没轨道 —— 看起来就是"设置项没了"。

**规则**：`row()` 只给行矩形，控件要自己调 `segments/fitted_segments` 才有位置。

### 5. 设置目录的覆写逻辑被吞

官方 1.0.59 的 `app_main` 里没有我们那段「`PulseConfigDirOverride() = RoamingConfigDir()`」，
合并后设置读写在 Local，Roaming 的旧设置被当成"另一个程序"忽略。

**护栏**：`check-fork-features.ps1` 里两条锚点（覆写 + 迁移）。

### 6. 用「文件时间」判断设置新旧 → 覆盖真设置

合并后程序在 Local 里写过一份**默认设置**（时间更新、体积更小）。
我按 mtime 把它拷回 Roaming，真设置就被这份默认值盖掉了。

**规则**：比内容（`toolbar_tools` 条数、`places` 条数、`tags` 条数），不比时间。
**护栏**：`app.json` 每次保存前滚存 `bak1…bak5`。

### 7. 生成文件

`strings.rcinc` / `string_ids.h` / `zh_hant_table.inc` 这类文件合并时直接取官方，
最后**统一重新生成**；手改必被 `--check` 拦下。

### 8. 「编译通过」不等于「功能在」

这是这次最贵的教训：`cmake --build` 全绿、程序也能起来，但上面 9 条功能全没了。
**必须有跑得动的检查**，否则只能等用户点。

### 9. 环境类

| 坑 | 处理 |
| --- | --- |
| 中文路径下 `rc.exe` 失败 | 统一从 `C:devFileP`（junction）编译 |
| `FileP.exe` / `pulse_shell.exe` 被占用 → LNK1104 | 先 `Stop-Process`，再编译 |
| DSH 是 Low 令牌，编出的 exe 带 Low 标签 | 每次 `icacls /setintegritylevel Medium`（冒烟脚本自动做） |
| GitHub 直连 TLS 失败 | `git -c http.sslBackend=openssl push github`；仍失败就等网络，不要 force |

---

## 二、现在的护栏

| 工具 | 作用 | 什么时候跑 |
| --- | --- | --- |
| `tools/merge-upstream.ps1` | 看官方改了什么、冲突热点、合并 | 每次跟版本 |
| `tools/check-fork-features.ps1` | 40 项自有功能锚点 | 合并后（脚本自动调用） |
| `tools/smoke-fork.ps1` | 提标签 + 5 页截图 + 2 个测试 | 编译后 |
| `tools/export-fork-patches.ps1` | 把 70 个自有提交导出成补丁 + 清单 | 需要"抄回某个功能"时 |
| `docs/FORK-CHANGES.md` | 功能差异清单 | 改功能时 |
| `docs/UPSTREAM-MERGE.md` | 九步流程 + 手点清单 | 每次跟版本 |

一条命令走完：`pwsh -File tools/merge-upstream.ps1 -Merge -Build -Smoke -Push`

---

## 三、「把改动全部统计起来、下次直接替换」可行吗？

**统计可以，直接替换不行。** 分三种情况：

| 我们的改动 | 能不能整包替换 | 原因 |
| --- | --- | --- |
| **新增文件**（`custom_commands*.cpp`、`tools/*.ps1`、文档） | ✅ 可以整包拷 | 官方没有同名文件，不会冲突 |
| **独立小改动**（`favorites_bar_layout.h` 阈值、`lan_device.cpp` 去 .local、`command_icons.h` 加图标） | ⚠️ 大部分可以，但要重编译验证 | 官方可能改过同一个函数 |
| **深度交织**（`app_main.cpp`、`app_input.cpp`、`app_commands.cpp`、`ui_renderer.cpp`、设置页布局/绘制） | ❌ 绝对不能 | 官方每次大版本都动这几处，整包替换 = 丢掉官方所有新功能 + 我们的改动也可能错位 |

**推荐做法（已落地）**：

1. **清单**：`docs/FORK-CHANGES.md`（功能级）+ `fork-patches/MANIFEST.md`（提交级，70 条，含涉及文件）
2. **补丁**：`pwsh -File tools/export-fork-patches.ps1` 随时生成 70 个补丁，哪个功能丢了就 `git apply --3way` 那个补丁抄回来
3. **护栏**：`check-fork-features.ps1` 是"哪些功能必须存在"的唯一权威
4. **未来方向**：把还没隔离的功能也往独立文件挪（像 `custom_commands` 那样），
   让官方文件里的改动越少越好 —— 这是降低合并成本的唯一根治办法

> 补丁是在**当时的上游代码**上生成的，不能整包 `git apply` 到今天；
> 它的价值是"这个功能当年是怎么写的"，用来抄，不是用来贴。
