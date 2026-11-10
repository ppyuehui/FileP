// fork_settings_toggles.h — FileP 自己的几个设置开关。
//
// 官方的 ToggleSetting 是几百行的 if 链（settings_controller.cpp），我们再加
// 分支会越来越难合并。所以自己的开关都收在这里：官方文件调用一次
// ForkToggleSetting()，命中就返回 true。
#pragma once

namespace pulse::app {

struct AppState;
class SettingsController;

// 处理 index 属于 FileP 自己的开关时返回 true（调用方直接 return）。
bool ForkToggleSetting(SettingsController& settings, int index);

} // namespace pulse::app
