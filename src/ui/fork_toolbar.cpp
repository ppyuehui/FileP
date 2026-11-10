// fork_toolbar.cpp — 见 fork_toolbar.h。
#include "fork_toolbar.h"

#include "toolbar_layout.h"

namespace pulse::ui {

void ForkHomeButtonLayout(ToolbarLayout& layout, float available_dip, float scale, float margin) {
    // 上一级按钮（navigation[2]）之后紧跟一个同样宽度的按钮；地址栏让出这段。
    if (layout.navigation[2].right <= layout.navigation[2].left) return;
    const float step = available_dip < 500 ? 26.0f * scale : 34.0f * scale;
    const float left = layout.navigation[2].right + 4 * scale;
    layout.home = D2D1::RectF(left, layout.navigation[2].top, left + step - 4 * scale,
                              layout.navigation[2].bottom);
    const float need = (layout.home.right + margin) - layout.address.left;
    if (need <= 0) return;
    layout.address.left += need;
    layout.address_text.left = (std::max)(layout.address_text.left + need, layout.address.left);
}

} // namespace pulse::ui
