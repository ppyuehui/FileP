// custom_commands_page.h — 自定义命令 as a settings page (page 14): the view
// model the page draws, the two text fields (native edits, dark) and the row
// actions. The commands themselves live in AppState::customCommands.
#pragma once
#include "app_state.h"
#include "../ui/ui_view_model.h"

namespace pulse::app {

// Fills vm.command_* from the store and the page's own selection.
void FillCommandsPage(AppState& s, ui::WindowViewModel& vm);
// Parks the two text fields over the page while it is open, hides them off it.
void SyncCommandsPage(AppState& s, const ui::WindowViewModel& vm);
// A row was clicked on page 14.
void CommandsPageAction(AppState& s, ui::HitTestResult::Region region, int index);
// WM_COMMAND from one of the fields (EN_CHANGE).
bool CommandsPageTextChanged(AppState& s, HWND edit);
// The dark background both fields paint on (WM_CTLCOLOREDIT).
HBRUSH CommandsPageEditBrush();
bool CommandsPageOwnsEdit(const AppState& s, HWND edit);

} // namespace pulse::app
