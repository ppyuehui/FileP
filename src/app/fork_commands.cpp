// fork_commands.cpp — 见 fork_commands.h。
#include "fork_commands.h"

#include "app_commands.h"
#include "app_navigation.h"
#include "app_runtime.h"
#include "app_state.h"

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

} // namespace pulse
