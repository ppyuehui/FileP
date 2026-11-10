// fork_commands.cpp — 见 fork_commands.h。
#include "fork_commands.h"

#include <algorithm>

#include "../../src/common/runtime_log.h"
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
