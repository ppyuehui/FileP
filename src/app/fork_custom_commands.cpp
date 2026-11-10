// fork_custom_commands.cpp — 见 fork_custom_commands.h。
#include "fork_custom_commands.h"

#include <memory>
#include <string>
#include <vector>

#include "app_commands.h"
#include "app_messages.h"
#include "app_prefs.h"
#include "app_navigation.h"
#include "app_state.h"
#include "custom_commands.h"
#include "../common/localization.h"
#include "../ui/confirm_dialog.h"

namespace pulse {
namespace {

struct CustomCommandOutcome {
    std::wstring dir;    // the folder the steps ran in
    std::wstring error;  // empty when every step ran
};

} // namespace

// 自定义命令: runs a command in the open folder, with the selected item's name
// as {name}. The steps create files, start programs and wait for scripts, so
// they run on the I/O worker; a batch script can take minutes and must never
// freeze the window. WM_CUSTOM_COMMAND_DONE reports the end.
void RunCustomCommandAt(AppState& s, size_t index) {
    if (index >= s.customCommands.commands.size()) return;
    const app::Tab* tab = ActiveTab(s);
    if (!tab || tab->current_path.empty()) return;
    app::CommandContext context;
    context.dir = tab->current_path;
    const std::vector<std::wstring> selection = SelectedFullPaths(*tab);
    if (selection.size() == 1) {
        const std::wstring& path = selection.front();
        const size_t slash = path.find_last_of(L"\\/");
        context.name = slash == std::wstring::npos ? path : path.substr(slash + 1);
    }
    const app::CustomCommand command = s.customCommands.commands[index];
    // 防误操作: 先把要执行的步骤摆出来问一句，确认了才真的动手。
    // 设置里可以关掉（设置 → 自定义命令 → 执行前先确认）。
    if (s.appPrefs.confirm_custom_commands && s.hwnd) {
        ui::ConfirmDialogSpec spec;
        spec.tone = ui::ConfirmTone::Question;
        spec.glyph = L"\xE756";
        spec.title = l10n::Get(l10n::StringId::CmdRunConfirmTitle);
        if (!command.name.empty()) spec.title += L" · " + command.name;
        spec.message = l10n::Get(l10n::StringId::CmdRunConfirmMessage);
        for (size_t step = 0; step < command.steps.size() && step < 12; ++step) {
            const app::CommandStep& it = command.steps[step];
            std::wstring line = std::to_wstring(step + 1) + L". ";
            switch (it.kind) {
            case app::CommandStepKind::NewFolder: line += l10n::Get(l10n::StringId::CmdKindFolder); break;
            case app::CommandStepKind::NewFile: line += l10n::Get(l10n::StringId::CmdKindFile); break;
            case app::CommandStepKind::RunProgram: line += l10n::Get(l10n::StringId::CmdKindProgram); break;
            default: line += l10n::Get(l10n::StringId::CmdKindScript); break;
            }
            if (!it.value.empty()) line += L"  " + app::ExpandCommandText(it.value, context);
            spec.items.push_back(std::move(line));
        }
        spec.items_total = command.steps.size();
        spec.note = context.dir;
        spec.confirm_text = l10n::Get(l10n::StringId::CmdRunConfirmButton);
        spec.cancel_text = l10n::Get(l10n::StringId::Cancel);
        spec.default_choice = ui::ConfirmChoice::Cancel;
        if (!ui::ShowConfirmDialog(s.hwnd, spec, s.darkMode, s.accentColor)) return;
    }
    const HWND hwnd = s.hwnd;
    s.worker.EnqueueIo([command, context, hwnd] {
        auto outcome = std::make_unique<CustomCommandOutcome>();
        outcome->dir = context.dir;
        app::RunCustomCommand(command, context, outcome->error);
        if (!PostMessageW(hwnd, WM_CUSTOM_COMMAND_DONE, 0, reinterpret_cast<LPARAM>(outcome.get())))
            return;  // the window is gone: the outcome dies with this scope
        outcome.release();
    });
}

void ReceiveCustomCommandResult(AppState& s, LPARAM lparam) {
    std::unique_ptr<CustomCommandOutcome> outcome(reinterpret_cast<CustomCommandOutcome*>(lparam));
    if (!outcome || !s.hwnd) return;
    if (!outcome->error.empty()) {
        MessageBoxW(s.hwnd, outcome->error.c_str(), L"FileP", MB_OK | MB_ICONERROR);
        return;
    }
    if (const app::Tab* tab = ActiveTab(s); tab && tab->current_path == outcome->dir)
        RefreshActiveTab(s);
    else
        s.store.MarkDirty(outcome->dir);
}

} // namespace pulse
