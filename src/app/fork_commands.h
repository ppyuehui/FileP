// fork_commands.h — FileP 自己的命令实现。
//
// 目前只有「左右双栏」一键开关：官方只有分屏菜单，没有一键切换。
#pragma once

namespace pulse {

struct AppState;

// 处理 FileP 自己的命令；返回 true 表示已处理（调用方直接 return）。
bool ForkDispatchCommand(AppState& s, int cmd);

} // namespace pulse
