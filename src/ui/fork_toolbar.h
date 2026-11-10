// fork_toolbar.h — FileP 自己加的工具栏部件。
//
// 目前只有地址栏左边的「此电脑」按钮：官方没有它，逻辑放在这里，
// 官方的 toolbar_layout.h 只留一次调用，合并时冲突面很小。
#pragma once
#include <d2d1.h>

namespace pulse::ui {

struct ToolbarLayout;

// 把「此电脑」按钮排在导航按钮串（返回/前进/上一级）之后，并把地址栏
// 左边界让出这块宽度。不可用时（太窄、已合并搜索栏）什么都不做。
void ForkHomeButtonLayout(ToolbarLayout& layout, float available_dip, float scale, float margin);

} // namespace pulse::ui
