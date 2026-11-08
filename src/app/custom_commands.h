// custom_commands.h — 自定义命令 (ported from File+ M90): a named list of steps
// the user runs in the folder they invoked it from, without writing code.
#pragma once
#include <string>
#include <vector>

namespace pulse::app {

enum class CommandStepKind : int {
    NewFolder = 0,   // 新建文件夹: an existing folder is kept, not an error
    NewFile = 1,     // 新建文件: same, an existing file is kept
    RunProgram = 2,  // 运行程序: "C:\Windows\notepad.exe" {dir}
    Script = 3,      // 批处理脚本: cmd text, run with chcp 65001 for UTF-8
};

struct CommandStep {
    CommandStepKind kind = CommandStepKind::NewFolder;
    std::wstring value;
};

struct CustomCommand {
    std::wstring name;
    bool toolbar = false;          // 工具栏
    bool folder_menu = false;      // 右键文件夹
    bool background_menu = false;  // 右键空白处
    std::vector<CommandStep> steps;

    bool ListedFor(bool on_folder) const {
        return on_folder ? folder_menu : background_menu;
    }
};

struct CommandContext {
    std::wstring dir;   // the folder the command runs in
    std::wstring name;  // the selected item's name, empty when nothing is selected
};

// {dir} {name} {date} {time}; anything else, including a stray brace, is left
// exactly as typed.
std::wstring ExpandCommandText(const std::wstring& text, const CommandContext& context);

// Runs the steps in order and stops at the first failure; `error` then says
// which step failed and why. Returns true when every step ran.
bool RunCustomCommand(const CustomCommand& command, const CommandContext& context,
                      std::wstring& error);

// commands.json in the app data folder. A missing file is an empty list; a
// malformed one keeps load_failed set so the UI can warn before overwriting.
class CustomCommandStore {
public:
    bool persist = true;
    bool load_failed = false;
    std::vector<CustomCommand> commands;

    bool Load();
    bool Save() const;
};

} // namespace pulse::app
