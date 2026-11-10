// fork_input.cpp — 见 fork_input.h。
#include "fork_input.h"

#include <utility>

#include "../ui/ui_renderer.h"
#include "app_navigation.h"
#include "app_runtime.h"
#include "app_state.h"

namespace pulse {

bool ForkArmTabClose(AppState& s, const ui::HitTestResult& hit, POINT at) {
    if (hit.region != ui::HitTestResult::TabClose || hit.index < 0) return false;
    // 松开鼠标才关：按下后划走可以反悔。
    s.tabClosePending = hit.index;
    s.tabClosePoint = at;
    return true;
}

bool ForkFinishTabClose(AppState& s, HWND hwnd, POINT at) {
    if (s.tabClosePending < 0) return false;
    // 标签 ✕: pressing armed it; releasing on the same ✕ closes the tab.
    const int pending = std::exchange(s.tabClosePending, -1);
    const ui::WindowViewModel vm = BuildVm(s, false);
    const D2D1_RECT_F rect = D2D1::RectF(0, 0, static_cast<float>(s.compositor.Width()),
                                         static_cast<float>(s.compositor.Height()));
    const ui::HitTestResult hit = s.renderer.HitTest(vm, rect, static_cast<float>(at.x),
                                                     static_cast<float>(at.y));
    if (hit.region == ui::HitTestResult::TabClose && hit.index == pending) {
        s.renderer.FreezeTabWidths(vm, static_cast<float>(s.compositor.Width()));
        CloseLayoutTab(s, static_cast<size_t>(pending));
    }
    InvalidateRect(hwnd, nullptr, FALSE);
    return true;
}

bool ForkHandleHomeClick(AppState& s) {
    NavigateTo(s, L"");   // 此电脑
    return true;
}

} // namespace pulse
