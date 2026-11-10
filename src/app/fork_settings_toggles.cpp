// fork_settings_toggles.cpp — 见 fork_settings_toggles.h。
#include "fork_settings_toggles.h"

#include "app_prefs.h"
#include "settings_controller.h"

namespace pulse::app {
namespace {

// 82 显示复制路径 · 83 自动收起侧栏 · 84 地址栏主页按钮 ·
// 85 最近变化悬停详情 · 86 自定义命令执行前确认
struct ForkToggle {
    int index;
    bool AppPrefs::*field;
};

constexpr ForkToggle kForkToggles[] = {
    {82, &AppPrefs::show_path_copy},
    {83, &AppPrefs::auto_collapse_sidebar},
    {84, &AppPrefs::show_home_button},
    {85, &AppPrefs::show_change_hover},
    {86, &AppPrefs::confirm_custom_commands},
};

} // namespace

void SettingsController::ForkSaveToggle() { SaveAndApply(SettingsEffect::None); }

bool ForkToggleSetting(SettingsController& settings, int index) {
    for (const ForkToggle& toggle : kForkToggles) {
        if (toggle.index != index) continue;
        AppPrefs& prefs = settings.ForkPrefs();
        prefs.*(toggle.field) = !(prefs.*(toggle.field));
        settings.ForkSaveToggle();
        return true;
    }
    return false;
}

} // namespace pulse::app
