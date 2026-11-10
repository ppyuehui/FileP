// fork_commands.h — FileP 自己的命令实现。
//
// 目前只有「左右双栏」一键开关：官方只有分屏菜单，没有一键切换。
#pragma once
#include <string>
#include <vector>

namespace pulse {

struct AppState;

// 处理 FileP 自己的命令；返回 true 表示已处理（调用方直接 return）。
bool ForkDispatchCommand(AppState& s, int cmd);

// 工具栏钉住列表的防线：自定义工具栏提交新列表前调用。
// 一次丢掉 3 个以上（每次点击只会改一个）认为是异常，拒绝写入并记日志。
bool ForkToolbarPinsSane(const std::vector<std::wstring>& before,
                         const std::vector<std::wstring>& after);

} // namespace pulse
