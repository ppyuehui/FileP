// fork_commands.cpp — 见 fork_commands.h。
#include "fork_commands.h"

#include <algorithm>

#include "../../src/common/runtime_log.h"
#include "../common/localization.h"
#include "../ui/notification_toast.h"
#include "app_commands.h"
#include "app_navigation.h"
#include "app_runtime.h"
#include "app_state.h"
#include "toolbar_tools.h"

namespace pulse {

bool ForkDispatchCommand(AppState& s, int cmd) {
    if (cmd != app::CmdLayoutTwoVertical) return false;
    // 左右分屏 as a switch (the toolbar button): side by side, and the next
    // call folds back to one pane. The pane count is what the window actually
    // shows, so the switch cannot disagree with the screen.
    ApplyLayoutPreset(s, LiveLayout(s).VisiblePanes().size() > 1
        ? app::LayoutPreset::Single : app::LayoutPreset::TwoVertical);
    return true;
}

void ForkNotifyPathCopied(AppState& s, const std::wstring& text, size_t count) {
    if (!s.hwnd) return;
    std::wstring title;
    if (count > 1) {
        title = l10n::Get(l10n::StringId::CopyPathDoneMany);
        if (const size_t slot = title.find(L"{n}"); slot != std::wstring::npos)
            title.replace(slot, 3, std::to_wstring(count));
    } else {
        title = l10n::Get(l10n::StringId::CopyPathDoneTitle);
    }
    // 一条路径就把它显示出来，太长时截断——截图和日志里都不会铺满整个窗口。
    std::wstring detail;
    if (count <= 1 && !text.empty()) {
        detail = text;
        for (wchar_t& ch : detail)
            if (ch == L'\r' || ch == L'\n') ch = L' ';
        if (detail.size() > 120) detail = detail.substr(0, 117) + L"...";
    }
    s.notification_toast.ShowSnack(s.hwnd, ui::SnackIcon::Copied, std::move(title),
                                   std::move(detail), {}, {}, 2500);
}

bool ForkToolbarPinsSane(const std::vector<std::wstring>& before,
                         const std::vector<std::wstring>& after) {
    size_t kept = 0, dropped = 0;
    for (const auto& id : before) {
        if (!app::IsToolbarSlot(id)) continue;   // 分隔符不算
        if (std::find(after.begin(), after.end(), id) == after.end()) ++dropped;
        else ++kept;
    }
    if (dropped <= 2) return true;
    // 一次点击只会取消一个钉住项；掉这么多说明列表被算错了。
    // 拒绝写入并留下证据（日志里能看到前后的条目数），免得用户的工具栏被清空。
    diagnostics::runtime::Event("toolbar_pins_rejected",
                                {{"kept", kept}, {"dropped", dropped}, {"before", before.size()},
                                 {"after", after.size()}},
                                diagnostics::runtime::Level::Warning);
    return false;
}

} // namespace pulse
