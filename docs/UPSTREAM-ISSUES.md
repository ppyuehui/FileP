# 上游（Pulse）问题清单 —— 可直接提 issue

> 这些都是在 FileP（Pulse 私人分支）里实测遇到、并确认**在官方 1.0.59 原样存在**的问题。
> 每条都给了：复现步骤、期望 / 实际、代码位置、可能的原因与建议修法。
> 英文段落是给 GitHub issue 直接用的措辞；中文是给我们自己看的说明。
>
> 自查方法：`git show 0856a655:<文件>` 确认该代码来自上游，而不是我们的改动。

---

## 1. Search results: "Open location" navigates to the folder but does not select the file

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

## 2. Portable copy: the dialog takeover card says "components missing or invalid — repair the installation"

**Steps**
1. Run Pulse from an ordinary folder (not installed).
2. 设置 → 启动与集成 → 打开和保存对话框 → **导出诊断**.

**Actual** A red "需要处理 / Needs attention" badge, the text
"组件缺失或校验失败。请修复安装后重试；尚未启用此功能。(50)", and the toggle is disabled.
The diagnostics file contains `bundle_verified=0`, `bundle_error=50`.

**Expected** "This feature requires an installed copy" — not "repair the installation".

**Root cause** `ResolveInstalledBundle()` (`src/common/dialog_bundle.cpp`) returns
`ERROR_NOT_SUPPORTED` (50) when `dialog-current.json` / `DialogBridge\<sha256>` are absent,
which is exactly the case for any uninstalled copy. `dialog_management_ui.cpp` maps **any**
non-zero bundle error to the "missing or invalid, repair the installation" string.

**Suggested fix** Special-case `ERROR_NOT_SUPPORTED` and say the feature needs an installed
build; keep the "repair" wording for genuine verification failures.

---

## 3. Turning the favorites bar off un-hides a sidebar section the user hid on purpose

**Steps**
1. In the sidebar section menu, hide **收藏夹 / Favorites**.
2. Turn the favorites bar on, then off (Ctrl+Shift+B twice).

**Expected** The section stays hidden, as the user chose.

**Actual** The section reappears: `ToggleFavoritesBar()` (
`src/app/favorites_bar.cpp`) clears the `Starred` bit in `sidebarHiddenMask`, which is the
same bit the user's "hide" choice uses.

**Suggested fix** `SidebarHiddenMaskWithBar()` already computes the bar-induced hide on the
fly; `ToggleFavoritesBar()` should not write it back into the user's mask.

---

## 4. UAC prompt on every launch when the Win + E takeover is enabled

**Steps** Start Pulse normally (default settings).

**Actual** Windows asks for administrator rights every launch; the elevated broker process
(`pulse_hotkey_broker.exe`) is started each time.

**Expected** No elevation unless the user turns on the Win + E takeover (a per-session
elevation is what actually needs admin).

**Root cause** `ShellWindowInit` passes `elevated = !s.isolatedTest`
(`src/app/shell_window_sync.cpp`), so the broker always runs elevated.

**Status in our fork** We pass `false`; folder opening (including Win + E) still works
through the non-elevated path.

---

## 5. (dev tool) `--shot <png> <file>` opens the file as a folder

**Steps** `Pulse.exe --shot out.png "C:\dev\photo.png"`.

**Actual** A tab opens **at the file path** and shows "This folder cannot be opened".

**Expected** The parent folder opens with the file selected (same rule as opening a file
from Explorer).

**Note** Only affects the screenshot harness; the normal launch path is fine.

---

## 提 issue 时的建议

- 一条 issue 一个现象，标题用 "component: what breaks" 的格式。
- 附上 Pulse 版本（设置 → 关于与诊断里有 build 串）、Windows 版本、是否安装版。
- 1、2、3 可以各附一小段日志：`%LOCALAPPDATA%\Pulse\Diagnostics\Runtime\app-*.jsonl`。
- 我们的修法可以只在 issue 里描述思路，**不要贴大段我们分支的代码**，方便他们自己取舍。
