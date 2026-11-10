// fork_custom_commands.h — 自定义命令的执行入口。
//
// 这两段原来写在官方 app_commands.cpp 里（约 90 行）。搬到自己的文件后，
// 官方文件只剩命令派发里的两行调用，合并官方新版时冲突面小。
#pragma once
#include <windows.h>

#include <cstddef>

namespace pulse {

struct AppState;

// 执行第 index 条自定义命令（会先弹确认框，取决于设置）。
void RunCustomCommandAt(AppState& s, size_t index);

// WM_CUSTOM_COMMAND_DONE：线程跑完回来了，刷新或报错。
void ReceiveCustomCommandResult(AppState& s, LPARAM lparam);

} // namespace pulse
