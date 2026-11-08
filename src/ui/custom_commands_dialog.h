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

// The same editor, living inside the settings page instead of a window of its
// own: a caption-less panel parked over the page's content area. Edits save as
// they happen, so the menus pick them up immediately. Show parks/creates it,
// Move follows the window, Hide is for leaving the page.
HWND ShowCustomCommandsPanel(HWND owner, app::CustomCommandStore& store, const RECT& rc);
void MoveCustomCommandsPanel(const RECT& rc);
void HideCustomCommandsPanel();
void CloseCustomCommandsPanel();

} // namespace pulse::ui
