// custom_commands_dialog.cpp — 自定义命令 editor. Plain Win32 controls in one
// modal sheet: the command list on the left, the selected command's name, the
// three places and its ordered steps on the right.
#include "custom_commands_dialog.h"

#include "../app/custom_commands.h"
#include "../common/windows_compat.h"

#include <windows.h>
#include <algorithm>
#include <string>
#include <vector>

namespace pulse::ui {
namespace {

constexpr wchar_t kClass[] = L"FilePCustomCommandsWindow";

enum ControlId : int {
    IDC_COMMANDS = 100,
    IDC_NEW,
    IDC_DELCMD,
    IDC_NAME,
    IDC_TOOLBAR,
    IDC_FOLDER,
    IDC_BACKGROUND,
    IDC_STEPS,
    IDC_KIND,
    IDC_VALUE,
    IDC_ADDSTEP,
    IDC_UPDATESTEP,
    IDC_UP,
    IDC_DOWN,
    IDC_DELSTEP,
    IDC_CLOSE,
};

const wchar_t* const kKinds[] = {L"新建文件夹", L"新建文件", L"运行程序", L"批处理脚本"};
constexpr int kKindCount = 4;

std::wstring StepText(const app::CommandStep& step, size_t index) {
    return std::to_wstring(index + 1) + L". " + kKinds[static_cast<int>(step.kind)] + L"    " + step.value;
}

class CommandDialog {
public:
    bool Show(HWND owner, app::CustomCommandStore& store) {
        store_ = &store;
        items_ = store.commands;
        owner_ = owner;
        dpi_ = pulse::compat::WindowDpi(owner ? owner : GetDesktopWindow());

        WNDCLASSEXW wc{sizeof(wc)};
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpfnWndProc = &CommandDialog::WndProc;
        wc.lpszClassName = kClass;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        if (!GetClassInfoExW(wc.hInstance, kClass, &wc)) RegisterClassExW(&wc);

        hwnd_ = CreateWindowExW(WS_EX_DLGMODALFRAME, kClass, L"自定义命令",
                                WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN,
                                0, 0, Dip(760), Dip(570), owner, nullptr, wc.hInstance, this);
        if (!hwnd_) return false;
        Center();
        ShowWindow(hwnd_, SW_SHOW);
        if (owner) EnableWindow(owner, FALSE);
        MSG msg{};
        while (IsWindow(hwnd_) && GetMessageW(&msg, nullptr, 0, 0) > 0) {
            if (!IsDialogMessageW(hwnd_, &msg)) {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }
        if (owner) EnableWindow(owner, TRUE);
        if (dirty_) {
            store_->commands = items_;
            store_->Save();
        }
        return dirty_;
    }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
        CommandDialog* self = reinterpret_cast<CommandDialog*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (message == WM_NCCREATE) {
            auto* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
            self = static_cast<CommandDialog*>(create->lpCreateParams);
            self->hwnd_ = hwnd;
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        }
        return self ? self->Handle(message, wparam, lparam)
                    : DefWindowProcW(hwnd, message, wparam, lparam);
    }

    int Dip(int value) const { return MulDiv(value, static_cast<int>(dpi_), 96); }

    HWND Make(const wchar_t* cls, const wchar_t* text, DWORD style, int id,
              int x, int y, int w, int h) {
        HWND child = CreateWindowExW(0, cls, text, WS_CHILD | WS_VISIBLE | style,
                                     Dip(x), Dip(y), Dip(w), Dip(h), hwnd_,
                                     reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
                                     GetModuleHandleW(nullptr), nullptr);
        if (child && font_) SendMessageW(child, WM_SETFONT, reinterpret_cast<WPARAM>(font_), TRUE);
        return child;
    }

    void Center() {
        RECT self{}, parent{};
        GetWindowRect(hwnd_, &self);
        if (owner_) GetWindowRect(owner_, &parent);
        else SystemParametersInfoW(SPI_GETWORKAREA, 0, &parent, 0);
        const int x = parent.left + ((parent.right - parent.left) - (self.right - self.left)) / 2;
        const int y = parent.top + ((parent.bottom - parent.top) - (self.bottom - self.top)) / 2;
        SetWindowPos(hwnd_, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }

    LRESULT Handle(UINT message, WPARAM wparam, LPARAM lparam) {
        switch (message) {
        case WM_CREATE:
            Build();
            return 0;
        case WM_COMMAND:
            OnCommand(LOWORD(wparam), HIWORD(wparam));
            return 0;
        case WM_CLOSE:
            DestroyWindow(hwnd_);
            return 0;
        case WM_DESTROY:
            if (font_) DeleteObject(font_);
            font_ = nullptr;
            return 0;
        default:
            return DefWindowProcW(hwnd_, message, wparam, lparam);
        }
    }

    void Build() {
        font_ = CreateFontW(-MulDiv(9, static_cast<int>(dpi_), 72), 0, 0, 0, FW_NORMAL, FALSE, FALSE,
                            FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");
        Make(L"STATIC", L"命令", 0, 0, 16, 12, 100, 20);
        Make(L"LISTBOX", L"", WS_BORDER | WS_VSCROLL | LBS_NOTIFY, IDC_COMMANDS, 16, 36, 250, 430);
        Make(L"BUTTON", L"新建命令", BS_PUSHBUTTON, IDC_NEW, 16, 476, 118, 30);
        Make(L"BUTTON", L"删除命令", BS_PUSHBUTTON, IDC_DELCMD, 144, 476, 118, 30);

        Make(L"STATIC", L"名称", 0, 0, 286, 12, 60, 20);
        Make(L"EDIT", L"", WS_BORDER | ES_AUTOHSCROLL, IDC_NAME, 346, 10, 396, 24);
        Make(L"BUTTON", L"工具栏", BS_AUTOCHECKBOX, IDC_TOOLBAR, 346, 42, 90, 22);
        Make(L"BUTTON", L"右键文件夹", BS_AUTOCHECKBOX, IDC_FOLDER, 446, 42, 110, 22);
        Make(L"BUTTON", L"右键空白处", BS_AUTOCHECKBOX, IDC_BACKGROUND, 566, 42, 120, 22);

        Make(L"STATIC", L"步骤（按顺序执行）", 0, 0, 286, 76, 200, 20);
        Make(L"LISTBOX", L"", WS_BORDER | WS_VSCROLL | LBS_NOTIFY, IDC_STEPS, 286, 100, 456, 280);
        Make(L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_VSCROLL, IDC_KIND, 286, 390, 150, 200);
        Make(L"EDIT", L"", WS_BORDER | ES_AUTOHSCROLL, IDC_VALUE, 444, 390, 298, 24);
        Make(L"BUTTON", L"添加步骤", BS_PUSHBUTTON, IDC_ADDSTEP, 286, 422, 84, 28);
        Make(L"BUTTON", L"更新", BS_PUSHBUTTON, IDC_UPDATESTEP, 378, 422, 68, 28);
        Make(L"BUTTON", L"上移", BS_PUSHBUTTON, IDC_UP, 454, 422, 68, 28);
        Make(L"BUTTON", L"下移", BS_PUSHBUTTON, IDC_DOWN, 530, 422, 68, 28);
        Make(L"BUTTON", L"删除步骤", BS_PUSHBUTTON, IDC_DELSTEP, 606, 422, 84, 28);
        Make(L"BUTTON", L"关闭", BS_DEFPUSHBUTTON, IDC_CLOSE, 652, 476, 90, 30);

        HWND kind = GetDlgItem(hwnd_, IDC_KIND);
        for (int i = 0; i < kKindCount; ++i)
            SendMessageW(kind, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(kKinds[i]));
        SendMessageW(kind, CB_SETCURSEL, 0, 0);

        RefreshCommands();
        if (!items_.empty()) {
            SendMessageW(GetDlgItem(hwnd_, IDC_COMMANDS), LB_SETCURSEL, 0, 0);
            LoadCommand(0);
        }
    }

    void RefreshCommands() {
        HWND list = GetDlgItem(hwnd_, IDC_COMMANDS);
        SendMessageW(list, LB_RESETCONTENT, 0, 0);
        for (const auto& item : items_)
            SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(item.name.c_str()));
        if (index_ >= 0 && index_ < static_cast<int>(items_.size()))
            SendMessageW(list, LB_SETCURSEL, index_, 0);
    }

    void RefreshSteps() {
        HWND list = GetDlgItem(hwnd_, IDC_STEPS);
        SendMessageW(list, LB_RESETCONTENT, 0, 0);
        if (index_ < 0 || index_ >= static_cast<int>(items_.size())) return;
        const auto& steps = items_[index_].steps;
        for (size_t i = 0; i < steps.size(); ++i) {
            const std::wstring text = StepText(steps[i], i);
            SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(text.c_str()));
        }
    }

    void LoadCommand(int index) {
        index_ = index;
        loading_ = true;
        if (index >= 0 && index < static_cast<int>(items_.size())) {
            const auto& command = items_[index];
            SetText(IDC_NAME, command.name);
            SetCheck(IDC_TOOLBAR, command.toolbar);
            SetCheck(IDC_FOLDER, command.folder_menu);
            SetCheck(IDC_BACKGROUND, command.background_menu);
        } else {
            SetText(IDC_NAME, L"");
            SetCheck(IDC_TOOLBAR, false);
            SetCheck(IDC_FOLDER, false);
            SetCheck(IDC_BACKGROUND, false);
        }
        loading_ = false;
        RefreshSteps();
    }

    void SetText(int id, const std::wstring& text) const {
        SetWindowTextW(GetDlgItem(hwnd_, id), text.c_str());
    }

    std::wstring GetText(int id) const {
        HWND control = GetDlgItem(hwnd_, id);
        const int length = GetWindowTextLengthW(control);
        std::wstring text(static_cast<size_t>(length) + 1, L'\0');
        GetWindowTextW(control, text.data(), length + 1);
        text.resize(static_cast<size_t>(length));
        return text;
    }

    void SetCheck(int id, bool on) const {
        SendMessageW(GetDlgItem(hwnd_, id), BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);
    }

    bool GetCheck(int id) const {
        return SendMessageW(GetDlgItem(hwnd_, id), BM_GETCHECK, 0, 0) == BST_CHECKED;
    }

    int SelectedStep() const {
        return static_cast<int>(SendMessageW(GetDlgItem(hwnd_, IDC_STEPS), LB_GETCURSEL, 0, 0));
    }

    app::CommandStepKind SelectedKind() const {
        const int index = static_cast<int>(SendMessageW(GetDlgItem(hwnd_, IDC_KIND), CB_GETCURSEL, 0, 0));
        return static_cast<app::CommandStepKind>(std::clamp(index, 0, kKindCount - 1));
    }

    void OnCommand(int id, int code) {
        switch (id) {
        case IDC_COMMANDS:
            if (code == LBN_SELCHANGE)
                LoadCommand(static_cast<int>(SendMessageW(GetDlgItem(hwnd_, IDC_COMMANDS), LB_GETCURSEL, 0, 0)));
            break;
        case IDC_NEW: {
            app::CustomCommand command;
            command.name = L"新命令";
            command.background_menu = true;
            items_.push_back(std::move(command));
            index_ = static_cast<int>(items_.size()) - 1;
            dirty_ = true;
            RefreshCommands();
            LoadCommand(index_);
            SetFocus(GetDlgItem(hwnd_, IDC_NAME));
            SendMessageW(GetDlgItem(hwnd_, IDC_NAME), EM_SETSEL, 0, -1);
            break;
        }
        case IDC_DELCMD:
            if (index_ >= 0 && index_ < static_cast<int>(items_.size())) {
                items_.erase(items_.begin() + index_);
                dirty_ = true;
                if (index_ >= static_cast<int>(items_.size())) index_ = static_cast<int>(items_.size()) - 1;
                RefreshCommands();
                LoadCommand(index_);
            }
            break;
        case IDC_NAME:
            if (code == EN_CHANGE && !loading_ && index_ >= 0 && index_ < static_cast<int>(items_.size())) {
                items_[index_].name = GetText(IDC_NAME);
                HWND list = GetDlgItem(hwnd_, IDC_COMMANDS);
                SendMessageW(list, LB_DELETESTRING, index_, 0);
                SendMessageW(list, LB_INSERTSTRING, index_,
                             reinterpret_cast<LPARAM>(items_[index_].name.c_str()));
                SendMessageW(list, LB_SETCURSEL, index_, 0);
                dirty_ = true;
            }
            break;
        case IDC_TOOLBAR:
        case IDC_FOLDER:
        case IDC_BACKGROUND:
            if (index_ >= 0 && index_ < static_cast<int>(items_.size())) {
                if (id == IDC_TOOLBAR) items_[index_].toolbar = GetCheck(id);
                if (id == IDC_FOLDER) items_[index_].folder_menu = GetCheck(id);
                if (id == IDC_BACKGROUND) items_[index_].background_menu = GetCheck(id);
                dirty_ = true;
            }
            break;
        case IDC_ADDSTEP:
            if (index_ >= 0 && index_ < static_cast<int>(items_.size())) {
                app::CommandStep step;
                step.kind = SelectedKind();
                step.value = GetText(IDC_VALUE);
                items_[index_].steps.push_back(std::move(step));
                dirty_ = true;
                RefreshSteps();
            }
            break;
        case IDC_UPDATESTEP: {
            const int step = SelectedStep();
            if (index_ >= 0 && step >= 0 && step < static_cast<int>(items_[index_].steps.size())) {
                items_[index_].steps[step].kind = SelectedKind();
                items_[index_].steps[step].value = GetText(IDC_VALUE);
                dirty_ = true;
                RefreshSteps();
                SendMessageW(GetDlgItem(hwnd_, IDC_STEPS), LB_SETCURSEL, step, 0);
            }
            break;
        }
        case IDC_UP:
        case IDC_DOWN: {
            const int step = SelectedStep();
            const int target = id == IDC_UP ? step - 1 : step + 1;
            if (index_ >= 0 && step >= 0 && target >= 0 &&
                target < static_cast<int>(items_[index_].steps.size())) {
                std::swap(items_[index_].steps[step], items_[index_].steps[target]);
                dirty_ = true;
                RefreshSteps();
                SendMessageW(GetDlgItem(hwnd_, IDC_STEPS), LB_SETCURSEL, target, 0);
            }
            break;
        }
        case IDC_DELSTEP: {
            const int step = SelectedStep();
            if (index_ >= 0 && step >= 0 && step < static_cast<int>(items_[index_].steps.size())) {
                items_[index_].steps.erase(items_[index_].steps.begin() + step);
                dirty_ = true;
                RefreshSteps();
            }
            break;
        }
        case IDC_CLOSE:
            DestroyWindow(hwnd_);
            break;
        default:
            break;
        }
    }

    HWND hwnd_ = nullptr;
    HWND owner_ = nullptr;
    HFONT font_ = nullptr;
    UINT dpi_ = 96;
    app::CustomCommandStore* store_ = nullptr;
    std::vector<app::CustomCommand> items_;
    int index_ = -1;
    bool loading_ = false;
    bool dirty_ = false;
};

} // namespace

bool ShowCustomCommandsDialog(HWND owner, app::CustomCommandStore& store) {
    CommandDialog dialog;
    return dialog.Show(owner, store);
}

} // namespace pulse::ui
