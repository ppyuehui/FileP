// custom_commands_page.cpp — 自定义命令 page (settings 14): the model the page
// draws, the two dark text fields parked over it, and the row actions.
#include "custom_commands_page.h"

#include "../common/windows_compat.h"
#include "../common/localization.h"
#include "../ui/ui_renderer.h"
#include "../ui/ui_view_model.h"
#include "../ui/settings_page_ids.h"
#include "../ui/edit_host.h"
#include "../ui/typography.h"
#include "../ui/FluentTokens.h"
#include "app_hosted_edit.h"
#include "app_commands.h"
#include "app_navigation.h"

#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>
#pragma comment(lib, "uxtheme.lib")

namespace pulse::app {
namespace {

constexpr int kNameEditId = 0x4D01;
constexpr int kValueEditId = 0x4D02;

HBRUSH g_edit_brush = nullptr;

HBRUSH EditBrush() {
    if (!g_edit_brush) g_edit_brush = CreateSolidBrush(RGB(43, 43, 43));
    return g_edit_brush;
}

std::wstring KindName(app::CommandStepKind kind) {
    using I = l10n::StringId;
    switch (kind) {
    case app::CommandStepKind::NewFile: return l10n::Get(I::CmdKindFile);
    case app::CommandStepKind::RunProgram: return l10n::Get(I::CmdKindProgram);
    case app::CommandStepKind::Script: return l10n::Get(I::CmdKindScript);
    default: return l10n::Get(I::CmdKindFolder);
    }
}

std::wstring StepText(const app::CommandStep& step, int index) {
    return std::to_wstring(index + 1) + L". " + KindName(step.kind) + L"   " + step.value;
}

app::CustomCommand* Selected(AppState& s) {
    if (s.command_sel < 0 || s.command_sel >= static_cast<int>(s.customCommands.commands.size()))
        return nullptr;
    return &s.customCommands.commands[static_cast<size_t>(s.command_sel)];
}

void Save(AppState& s) { s.customCommands.Save(); }

void ClampSelection(AppState& s) {
    const int count = static_cast<int>(s.customCommands.commands.size());
    if (count == 0) { s.command_sel = -1; s.command_step_sel = -1; return; }
    if (s.command_sel >= count) s.command_sel = count - 1;
    if (s.command_sel < 0) s.command_sel = 0;
    const int steps = static_cast<int>(s.customCommands.commands[static_cast<size_t>(s.command_sel)].steps.size());
    if (s.command_step_sel >= steps) s.command_step_sel = steps - 1;
    if (s.command_step_sel < 0 && steps > 0) s.command_step_sel = -1;
}

// Tab moves between the two fields; everything else is drawn by the compositor
// (LumaText), so the field's text is the same face, size and weight as the
// DirectWrite text around it instead of GDI text that never quite matches.
LRESULT CALLBACK FieldProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam,
                           UINT_PTR, DWORD_PTR ref) {
    AppState* s = reinterpret_cast<AppState*>(ref);
    if (!s) return DefSubclassProc(hwnd, msg, wparam, lparam);
    if (msg == WM_KEYDOWN && wparam == VK_TAB) {
        HWND other = hwnd == s->command_name_edit ? s->command_value_edit : s->command_name_edit;
        if (other && IsWindow(other) && IsWindowVisible(other)) SetFocus(other);
        return 0;
    }
    IDWriteTextFormat* format = HostedEditFormat(*s, hwnd);
    const D2D1_COLOR_F ink = HostedEditForeground(*s);
    const D2D1_COLOR_F paper = HostedEditBackground(*s);
    LRESULT handled = 0;
    if (ui::HandleChildEditMessage(s->compositor, format, ink, paper,
                                   ui::EditBackBrush(s->editBrush), hwnd, msg, wparam, lparam, handled))
        return handled;
    return ui::DefPresentedChildEditProc(s->compositor, format, ink, paper, hwnd, msg, wparam, lparam);
}

HWND MakeEdit(HWND parent, int id, AppState& s) {
    // A redirected child surface: plain child windows sit behind the
    // DirectComposition surface, so the fields use the same host as the
    // rename / address editors (ui::edit_host.h).
    HWND edit = ui::CreateRedirectedChildEdit(parent);
    if (!edit) return nullptr;
    SetWindowLongPtrW(edit, GWLP_ID, id);
    SetWindowSubclass(edit, FieldProc, 1, reinterpret_cast<DWORD_PTR>(&s));
    // A hair of side margin: the caret should not touch the frame's border.
    SendMessageW(edit, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(2, 2));
    SetWindowTheme(edit, L"", L"");
    // The app's own edit font, not the stock GUI font: the fields must match
    // the text around them (size and family alike).
    if (!s.editFont) s.editFont = ui::typography::CreateEditFont(s.scale);
    SendMessageW(edit, WM_SETFONT, reinterpret_cast<WPARAM>(s.editFont), TRUE);
    return edit;
}

} // namespace

bool CommandsPageOwnsEdit(const AppState& s, HWND edit) {
    return edit && (edit == s.command_name_edit || edit == s.command_value_edit);
}

HBRUSH CommandsPageEditBrush() { return EditBrush(); }

void FillCommandsPage(AppState& s, ui::WindowViewModel& vm) {
    using I = l10n::StringId;
    ClampSelection(s);
    vm.command_names.clear();
    vm.command_places.clear();
    vm.command_step_counts.clear();
    vm.command_step_texts.clear();
    for (const auto& command : s.customCommands.commands) {
        vm.command_names.push_back(command.name.empty() ? l10n::Get(I::SettingsPageCommands) : command.name);
        std::wstring places;
        if (command.toolbar) places = l10n::Get(I::CmdPlaceToolbar);
        if (command.folder_menu) places += (places.empty() ? L"" : L" · ") + l10n::Get(I::CmdPlaceFolder);
        if (command.background_menu) places += (places.empty() ? L"" : L" · ") + l10n::Get(I::CmdPlaceBackground);
        vm.command_places.push_back(places);
        vm.command_step_counts.push_back(static_cast<int>(command.steps.size()));
    }
    vm.command_selected = s.command_sel;
    vm.command_step_selected = s.command_step_sel;
    vm.command_kind = s.command_kind;
    if (const app::CustomCommand* command = Selected(s)) {
        vm.command_toolbar = command->toolbar;
        vm.command_folder = command->folder_menu;
        vm.command_background = command->background_menu;
        for (size_t i = 0; i < command->steps.size(); ++i)
            vm.command_step_texts.push_back(StepText(command->steps[i], static_cast<int>(i)));
    }
}

void SyncCommandsPage(AppState& s, const ui::WindowViewModel& vm) {
    if (s.shot.active) return;   // the snapshot needs no text fields
    // The page is drawn only while the settings tab shows it, so the page
    // number is enough: the active pane can belong to another layout.
    const bool on = s.hwnd && vm.settings_page == ui::kSettingsPageCommands;
    if (!on) {
        if (s.command_name_edit) ShowWindow(s.command_name_edit, SW_HIDE);
        if (s.command_value_edit) ShowWindow(s.command_value_edit, SW_HIDE);
        return;
    }
    if (!s.command_name_edit) {
        s.command_name_edit = MakeEdit(s.hwnd, kNameEditId, s);
        // Grey hint inside the empty field (static string: the control keeps the
        // pointer, so it must outlive the page).
        SendMessageW(s.command_name_edit, EM_SETCUEBANNER, TRUE,
                     reinterpret_cast<LPARAM>(L"输入命令名称"));
    }
    if (!s.command_value_edit) {
        s.command_value_edit = MakeEdit(s.hwnd, kValueEditId, s);
        SendMessageW(s.command_value_edit, EM_SETCUEBANNER, TRUE,
                     reinterpret_cast<LPARAM>(L"文件夹名 / 文件名 / 命令行"));
    }
    RECT client{};
    GetClientRect(s.hwnd, &client);
    D2D1_RECT_F name_rc{}, value_rc{};
    s.renderer.SettingsCommandFields(vm, D2D1::RectF(0.0f, 0.0f, static_cast<float>(client.right),
                                                     static_cast<float>(client.bottom)),
                                     &name_rc, &value_rc);
    // The column the page scrolls in: a field whose row left it must go, or it
    // would float over the header / status bar.
    const D2D1_RECT_F column = s.renderer.SettingsContentRect(
        vm, D2D1::RectF(0.0f, 0.0f, static_cast<float>(client.right), static_cast<float>(client.bottom)));
    // The drawn frame is the field; the native edit sits inside its border.
    const float pad_x = 10.0f * s.scale;
    const float line = 28.0f * s.scale;
    const auto place = [&](HWND edit, const D2D1_RECT_F& rc) {
        if (!edit) return;
        if (rc.right - rc.left <= 2 * pad_x || rc.bottom <= rc.top ||
            rc.top < column.top || rc.bottom > column.bottom) {
            ShowWindow(edit, SW_HIDE);
            return;
        }
        const float mid = (rc.top + rc.bottom) * 0.5f;
        const int x = static_cast<int>(rc.left + pad_x);
        const int y = static_cast<int>(mid - line * 0.5f);
        const int w = static_cast<int>(rc.right - rc.left - 2 * pad_x);
        SetWindowPos(edit, HWND_TOP, x, y, w, static_cast<int>(line),
                     SWP_SHOWWINDOW | SWP_NOACTIVATE);
    };
    place(s.command_name_edit, name_rc);
    place(s.command_value_edit, value_rc);
    // The page draws the frame; it lights up while its field has the caret.
    const HWND focus = GetFocus();
    const_cast<ui::WindowViewModel&>(vm).command_name_focused = focus && focus == s.command_name_edit;
    const_cast<ui::WindowViewModel&>(vm).command_value_focused = focus && focus == s.command_value_edit;
    // Text: the selected command's name and the step value being typed.
    if (s.command_name_edit && GetFocus() != s.command_name_edit) {
        const app::CustomCommand* command = Selected(s);
        const std::wstring wanted = command ? command->name : std::wstring();
        wchar_t now[512]{};
        GetWindowTextW(s.command_name_edit, now, ARRAYSIZE(now));
        if (wanted != now) SetWindowTextW(s.command_name_edit, wanted.c_str());
    }
    if (s.command_value_edit && GetFocus() != s.command_value_edit) {
        wchar_t now[1024]{};
        GetWindowTextW(s.command_value_edit, now, ARRAYSIZE(now));
        if (s.command_value != now) SetWindowTextW(s.command_value_edit, s.command_value.c_str());
    }
}

bool CommandsPageTextChanged(AppState& s, HWND edit) {
    if (!CommandsPageOwnsEdit(s, edit)) return false;
    wchar_t buffer[1024]{};
    GetWindowTextW(edit, buffer, ARRAYSIZE(buffer));
    if (edit == s.command_name_edit) {
        if (app::CustomCommand* command = Selected(s)) command->name = buffer;
    } else {
        s.command_value = buffer;
    }
    s.customCommands.Save();
    InvalidateRect(s.hwnd, nullptr, FALSE);
    return true;
}

void CommandsPageAction(AppState& s, ui::HitTestResult::Region region, int index) {
    using R = ui::HitTestResult;
    app::CustomCommand* command = Selected(s);
    const R::Region region_id = region;
    switch (region_id) {
    case R::SettingsCommandRow:
        s.command_sel = index;
        s.command_step_sel = -1;
        s.command_value.clear();
        break;
    case R::SettingsCommandStep:
        s.command_step_sel = index;
        if (command && index >= 0 && index < static_cast<int>(command->steps.size())) {
            s.command_kind = static_cast<int>(command->steps[static_cast<size_t>(index)].kind);
            s.command_value = command->steps[static_cast<size_t>(index)].value;
        }
        break;
    case R::SettingsCommandNew: {
        app::CustomCommand created;
        created.name = l10n::Get(l10n::StringId::CmdNew);
        created.background_menu = true;
        s.customCommands.commands.push_back(std::move(created));
        s.command_sel = static_cast<int>(s.customCommands.commands.size()) - 1;
        s.command_step_sel = -1;
        s.command_value.clear();
        s.customCommands.Save();
        break;
    }
    case R::SettingsCommandDelete:
        if (s.command_sel >= 0 && s.command_sel < static_cast<int>(s.customCommands.commands.size())) {
            s.customCommands.commands.erase(s.customCommands.commands.begin() + s.command_sel);
            s.command_step_sel = -1;
            s.command_value.clear();
            s.customCommands.Save();
        }
        break;
    case R::SettingsCommandPlaces:
        if (command) {
            if (index == 0) command->toolbar = !command->toolbar;
            else if (index == 1) command->folder_menu = !command->folder_menu;
            else command->background_menu = !command->background_menu;
            s.customCommands.Save();
        }
        break;
    case R::SettingsCommandKind:
        s.command_kind = index;
        break;
    case R::SettingsCommandAddStep:
        if (command) {
            app::CommandStep step;
            step.kind = static_cast<app::CommandStepKind>(std::clamp(s.command_kind, 0, 3));
            step.value = s.command_value;
            command->steps.push_back(std::move(step));
            s.command_step_sel = static_cast<int>(command->steps.size()) - 1;
            s.customCommands.Save();
        }
        break;
    case R::SettingsCommandUpdateStep:
        if (command && s.command_step_sel >= 0 && s.command_step_sel < static_cast<int>(command->steps.size())) {
            auto& step = command->steps[static_cast<size_t>(s.command_step_sel)];
            step.kind = static_cast<app::CommandStepKind>(std::clamp(s.command_kind, 0, 3));
            step.value = s.command_value;
            s.customCommands.Save();
        }
        break;
    case R::SettingsCommandUp:
    case R::SettingsCommandDown:
        if (command && s.command_step_sel >= 0 && s.command_step_sel < static_cast<int>(command->steps.size())) {
            const int target = s.command_step_sel + (region_id == R::SettingsCommandUp ? -1 : 1);
            if (target >= 0 && target < static_cast<int>(command->steps.size())) {
                std::swap(command->steps[static_cast<size_t>(s.command_step_sel)],
                          command->steps[static_cast<size_t>(target)]);
                s.command_step_sel = target;
                s.customCommands.Save();
            }
        }
        break;
    case R::SettingsCommandDeleteStep:
        if (command && s.command_step_sel >= 0 && s.command_step_sel < static_cast<int>(command->steps.size())) {
            command->steps.erase(command->steps.begin() + s.command_step_sel);
            s.command_step_sel = std::min(s.command_step_sel, static_cast<int>(command->steps.size()) - 1);
            s.customCommands.Save();
        }
        break;
    default:
        return;
    }
    ClampSelection(s);
    InvalidateRect(s.hwnd, nullptr, FALSE);
}

} // namespace pulse::app
