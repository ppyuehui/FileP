// fork_input.h — FileP 自己加的鼠标处理。
//
// 这两段原来写在官方 app_input.cpp 的点击链里：链长、分支多，合并官方新版时
// 很容易整段丢掉。现在官方文件只留一行调用。
#pragma once
#include <windows.h>

#include "../ui/ui_view_model.h"

namespace pulse {

struct AppState;

// 标签页 ✕：按下时只记下是哪个标签，返回 true 表示已接管。
bool ForkArmTabClose(AppState& s, const ui::HitTestResult& hit, POINT at);

// 松开鼠标：仍在同一个 ✕ 上才真的关，返回 true 表示已处理（调用方直接 return）。
bool ForkFinishTabClose(AppState& s, HWND hwnd, POINT at);

// 地址栏「此电脑」：点一下回到驱动器视图。返回 true 表示已处理。
bool ForkHandleHomeClick(AppState& s);

} // namespace pulse
