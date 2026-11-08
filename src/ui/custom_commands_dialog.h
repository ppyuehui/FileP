// custom_commands_dialog.h — 自定义命令 editor (name, the three places and the
// ordered steps). A plain Win32 dialog on purpose: the rest of Pulse draws with
// Direct2D, but this sheet is edited rarely and the native controls keep the
// text fields, list boxes and combo box working without new furniture.
#pragma once
#include <windows.h>

namespace pulse::app { class CustomCommandStore; }

namespace pulse::ui {

// Modal. Returns true when the commands changed (the caller saves the store).
bool ShowCustomCommandsDialog(HWND owner, app::CustomCommandStore& store);

} // namespace pulse::ui
