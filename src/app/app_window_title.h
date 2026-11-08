#pragma once
#include <string>

namespace pulse {
struct AppState;
namespace app { struct Tab; }

// The taskbar title: 上级\当前, each name cut to its own character limit
// (0 = no limit) with […] at the end, then " — FileP".
std::wstring TaskbarWindowTitle(const app::Tab* tab, int parent_chars = 5, int current_chars = 5);
// Changes the native caption only when the active folder's display name changes.
bool SyncTaskbarWindowTitle(AppState& s);
} // namespace pulse
