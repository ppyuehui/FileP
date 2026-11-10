# 以后怎么跟随官方更新（最保险的流程）

> 这份是操作手册。**踩过的坑**见 [MERGE-LESSONS.md](MERGE-LESSONS.md)，**功能清单**见 [FORK-CHANGES.md](FORK-CHANGES.md)，
> 自动检查见 `tools/check-fork-features.ps1`，合并脚本见 `tools/merge-upstream.ps1`。

## 一句话版本

> 开分支 → 合并 → **逐文件手工解冲突（绝不整侧取官方）** → 跑 `check-fork-features.ps1` → 编译 → 截图核对 → 提交 → 三端推送。

---

## 0. 先看清楚这次官方改了什么

```powershell
cd C:\Users\hui\工作\Agent\自编软件\FileP
pwsh -NoProfile -File tools/merge-upstream.ps1
```

脚本会列出：新提交、改动最大的文件、**我们和官方都改过的文件（冲突热点）**。
先读完列表再决定要不要跟。官方只是修几个 bug 的版本，风险小；像 1.0.59 这种动了
`app_main/app_input/app_commands/ui_renderer` 的大版本，就要按下面全程走。

## 1. 开分支（保命）

```powershell
git switch -c upstream-1.0.60      # 版本号自己填
```

**不要直接在 main 上合并。** 出问题随时 `git switch main && git branch -D upstream-1.0.60` 回到干净状态。

> 底线：不 `git push --force`、不 `git reset --hard`、不 `git stash`（会撤掉并发的改动）。

## 2. 合并

```powershell
git merge upstream/main --no-commit --no-ff
git diff --name-only --diff-filter=U      # 看冲突文件
```

## 3. 解冲突：按类处理

### 3.1 可以「整侧取官方」的

| 文件 | 做法 |
| --- | --- |
| `README.md` / `README.en.md` | `git checkout --ours`（保我们的） |
| `.github/workflows/*` | `git rm`（本来就删了） |
| `src/setup/*` | `--theirs`（安装包流程，我们不打包） |
| 生成文件：`strings.rcinc`、`strings_zh_tw.rcinc`、`string_ids.h`、`localization.h`、`zh_hant_table.inc` | `--theirs`，最后统一重新生成 |
| `src/bench/**` 里的官方测试 | `--theirs` |

### 3.2 **绝对不要整侧取**的（血泪教训）

> 2026-11-09 合并 1.0.59 时，这几个文件整侧取了官方版本，结果**编译一次就过、功能静默消失**：
> 设置写回 Local、此电脑按钮点了没反应、自定义命令页全废、任务栏滑块拖不动、
> 标签 ✕ 变回按下即关、工具栏「左右双栏」按钮消失。
> 全靠后来逐条对照才发现。

| 文件 | 里面藏着我们的 |
| --- | --- |
| `src/app/app_main.cpp` | FileP 标题/提示框、设置目录 Roaming 覆写、截图开关、多窗口关闭 |
| `src/app/app_input.cpp` | 此电脑点击、设置页全部交互、任务栏滑块、标签 ✕ 松开、自定义命令页派发 |
| `src/app/app_commands.cpp` | 菜单/派发、自定义命令行、split_two 工具栏项、分屏开关 |
| `src/ui/ui_renderer.cpp` | 标题栏品牌文字、侧栏手动展开、自定义命令页辅助函数、状态栏 |
| `src/ui/ui_settings_core.cpp` / `ui_settings_view.cpp` / `settings_layout_sections.h` | 四个自有开关的行、页号、绘制 |

这几个文件要**逐块合并**：官方的改动保留，我们的改动重新接上。

### 3.3 号段类（最容易埋雷）

| 类型 | 我们的号段 | 冲突表现 |
| --- | --- | --- |
| 字符串 id | 6400–6445（官方 1.0.59 已到 6390） | 重复 id → 文案串行；`gen_strings.py` 会报错 |
| 设置页 | 自定义命令 = **16** | 官方占了 13/14/15，页号一撞就进错页 |
| 命令 id | 自定义命令 = **5700–5799** | 撞上官方新命令 → 点了执行别的 |
| 私有消息 | `WM_CUSTOM_COMMAND_DONE = WM_APP+200` | 撞上官方消息 → 收到别人的消息 |
| 工具栏切换号 | 82 / 83 / 84 / 85 | 撞号 → 点一个开关变成改另一个（1.0.59 里 81 就撞过） |

**合并后必查**：在 `tools/l10n/catalog.tsv` 里搜 `\t6400\t`、`\t6445\t`，
确认我们的 46 条还在且没有和第二列重号。

### 3.4 生成文件最后统一做

```powershell
py -3.12 tools/l10n/gen_strings.py
py -3.12 tools/l10n/gen_strings.py --check     # 必须 "up to date"
```

## 4. 跑功能守卫（关键一步）

```powershell
pwsh -NoProfile -File tools/check-fork-features.ps1
```

35 项自有功能，逐条查代码锚点在不在；缺了会打印「功能 / 文件 / 症状」并返回 1。

> 它只证明**代码还在**，不证明行为对。所以还要走第 5 步。

## 5. 编译 + 冒烟（一条命令）

```powershell
pwsh -NoProfile -File C:\dev\FileP\build_filep.ps1
pwsh -NoProfile -File tools/smoke-fork.ps1     # 自动：提 Medium 标签 + 截 5 页 + 跑 2 个测试
```

`smoke-fork.ps1` 会做这些事，任何一项不合格就返回 1：

| 检查 | 判据 |
| --- | --- |
| exe 完整性标签 | 不是 Low（是就自动提 Medium） |
| 设置页 0 / 3 / 9 / 10 / 16 | 出图且大于 20 KB（页号见 `settings_page_ids.h`） |
| `pulse_custom_commands_test` | 退出码 0 且 `0 failed` |
| `pulse_settings_transfer_test` | 退出码 0 且 `0 failed` |

想留存图片：`-OutDir C:\dev\smoke -KeepOpen`。
整个合并流程也可以一条命令走完：

```powershell
pwsh -File tools/merge-upstream.ps1 -Merge -Build -Smoke -Push
```

**手点清单**（截图看不到交互）：
1. 地址栏「此电脑」按钮点一下
2. 标签页 ✕ 按下后划走，不应关闭
3. 设置 → 自定义命令：新建一个命令、打字、Tab 切换、加步骤、上移下移、删除
4. 工具栏加「左右双栏」按钮，点一次分屏（图标点亮），再点回到单栏
5. 右键菜单：斜向滑到二级菜单不应被抢；自定义命令按「出现位置」出现
6. 窗口拉窄到半屏：侧栏自动收成图标栏；点开后再拉窄，应保持展开
7. 任务栏标题两个滑块拖动

## 6. 提交与推送

```powershell
git add -A
git commit -m "合并: 官方 Pulse 1.0.60"
git switch main
git merge --ff-only upstream-1.0.60

git push origin main          # 远端 Gitea
git push local main           # 本机 Gitea（失败就重试一次，仍失败如实报告）
git -c http.sslBackend=openssl push github main   # GitHub（要代理，直连常 TLS 失败）
```

## 6.5 设置文件的坑（2026-11-10 踩过）

- **改设置前先确认 FileP 已退出**：程序把设置存在内存里，任何一次操作都会把内存状态写回 `app.json`；
  若你在它运行期间手工改文件，它一保存就覆盖掉了。
- **不要用「mtime 更新」判断哪份设置更好**：合并 1.0.59 后程序一度把默认值写到
  `%LOCALAPPDATA%\FileP`（更新、更小），按时间戳把它拷到 `%APPDATA%\FileP` 就把真设置盖掉了。
  要比内容（`toolbar_tools`、`places`、`tags` 的条数）。
- 现在每次保存前会自动滚存 `app.json.bak1…bak5`；出事就把 `bak1` 改名回 `app.json`。

## 7. 收尾

- 更新 `docs/FORK-CHANGES.md`：官方这次把哪些功能自己做了（我们的可以删）、
  新版本带来了什么、有没有新撞号。
- 如果官方占用了我们的号段，**先迁号再合并**（迁完单独提交一次，别和合并混在一起）。

---

## 附：这次合并踩过的坑，逐条记着

| 坑 | 后果 | 预防 |
| --- | --- | --- |
| 字符串 id 撞号 | 文案串行、生成失败 | 合并前先比对 catalog，我们的放 6400+ |
| 设置页号被占 | 点「自定义命令」进了 AI 助手页 | 页号只认常量，合并后核对 16 |
| 命令 id 撞号 | 点自定义命令弹出「新建 Excel」 | 5700+，`static_assert` 区间检查 |
| 消息 id 撞号 | 收到别的模块的消息 | `WM_APP+200` |
| 切换号撞号 | 点 A 开关改了 B 设置 | 合并后 grep 命中表里的 `index=` |
| 整侧取官方 | 功能静默消失 | 四个文件逐块合并 + 跑守卫脚本 |
| 生成文件手改 | CI 的 `--check` 拦下 | 只跑 `gen_strings.py` |
| GitHub 推不上去 | 只有两个远端有 | `-c http.sslBackend=openssl`，代理 7900，网络恢复后补推 |
