# 上游（Pulse）问题清单 —— 可直接提 issue

> 这些都是在 FileP（Pulse 私人分支）里实测遇到、并确认**在官方 1.0.59 原样存在**的问题。
> 每条都给了：复现步骤、期望 / 实际、代码位置、可能的原因与建议修法。
> 英文段落是给 GitHub issue 直接用的措辞；中文是给我们自己看的说明。
>
> 自查方法：`git show 0856a655:<文件>` 确认该代码来自上游，而不是我们的改动。
>
> **提交状态**
>
> | # | 问题 | 状态 |
> | --- | --- | --- |
> | 1 | 打开所在位置只跳目录不选中 | ✅ **已提交上游 issue [#578](https://github.com/jimmgreen/pulse/issues/578)**（含 `git am` 格式补丁） |
> | 2 | 免安装版误报"组件缺失或校验失败" | 未提交（备选） |
> | 3 | 关掉收藏栏会把用户隐藏的分组放出来 | 未提交（备选） |
> | 4 | 每次启动弹 UAC | 未提交（备选） |
> | 5 | `--shot` 带文件参数把文件当文件夹 | 未提交（开发工具，可不提） |

---

## 1. Search results: "Open location" navigates to the folder but does not select the file

> **已提交：[issue #578](https://github.com/jimmgreen/pulse/issues/578)**
> 正文里带了可直接 `git am` 的补丁（`src/app/reveal_request.{h,cpp}` + 三处调用）。
> 上游同类问题 [#190](https://github.com/jimmgreen/pulse/issues/190) 修的是"启动/命令行"那条路，
> 搜索结果的 reveal 这条路当时没覆盖到；`main` 上 `CmdOpenPath` 至今没变。

**Environment** Pulse 1.0.59, Windows 11 26H2 (x64).

**Steps**
1. Search for a file (content search / global search window).
2. Right-click a result → **Open location** (打开所在位置 / 在所在位置显示).

**Expected** The containing folder opens with that file selected and scrolled into view.

**Actual** Only the folder opens. The file is not selected, nothing is scrolled; the status bar shows no selection.

**Root cause (two independent problems)**

1. The reveal intent is stored in the tab's pending-selection fields:

   ```cpp
   // src/app/app_commands.cpp — CmdOpenPath
   NavigateTo(s, parent);
   tab->pending_selected_name = leaf;
   tab->pending_selection_revision = tab->selection_revision;
   ```

   Navigation clears the selection and bumps `selection_revision`. When the new listing
   arrives, `TakeListingSelection()` (`src/app/listing_selection_restore.h`) sees
   `pending_selection_revision != tab.selection_revision`, concludes the pending data is
   **stale**, calls `CapturePendingListingSelection(tab)` — which **overwrites** the
   pending names with the current (empty) selection — and then restores nothing.

2. The folder comparison uses two different path spellings: the pane's `current_path` is
   display text (`C:\dev`), while selection paths are extended (`\\?\C:\dev\x.png`).
   Any "is the target folder the current folder" check written naively never matches.

**Suggested fix** Carry the reveal intent in its own field (folder + names) that the
snapshot-apply step consumes once, instead of reusing `pending_selected_*`; normalize
`\\?\` before comparing paths.

**Status in our fork** Worked around in FileP: `src/app/fork_reveal.cpp` keeps a
`(folder, names, deadline)` request and applies it in `BuildVm` when the listing actually
matches. Verified with a screenshot: the row is selected and scrolled to.

---

## 提 issue 时的建议

- 一条 issue 一个现象，标题用 "component: what breaks" 的格式。
- 附上 Pulse 版本（设置 → 关于与诊断里有 build 串）、Windows 版本、是否安装版。
- 1、2、3 可以各附一小段日志：`%LOCALAPPDATA%\Pulse\Diagnostics\Runtime\app-*.jsonl`。
- 我们的修法可以只在 issue 里描述思路，**不要贴大段我们分支的代码**，方便他们自己取舍。
