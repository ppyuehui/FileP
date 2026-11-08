// custom_commands_test.cpp - 自定义命令: placeholders, step runner, store.
// A headless [PASS]/[FAIL] check: no window, a temp folder as the data dir.
#include "../app/custom_commands.h"

#include <windows.h>
#include <string>
#include <vector>

namespace pulse::app {
namespace {
std::wstring g_data_dir;
}
std::wstring GetPulseDataDir() { return g_data_dir; }
} // namespace pulse::app

namespace {

int g_passed = 0;
int g_failed = 0;

void Check(bool value, const char* label) {
    std::printf("[%s] %s\n", value ? "PASS" : "FAIL", label);
    if (value) ++g_passed; else ++g_failed;
}

bool Exists(const std::wstring& path) {
    return GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

std::wstring TempRoot() {
    wchar_t temp[MAX_PATH]{};
    GetTempPathW(MAX_PATH, temp);
    std::wstring root = std::wstring(temp) + L"filep-command-test-" +
        std::to_wstring(GetCurrentProcessId());
    CreateDirectoryW(root.c_str(), nullptr);
    return root;
}

} // namespace

int wmain() {
    using namespace pulse::app;

    // 1. Placeholders.
    CommandContext context;
    context.dir = L"C:\\work\\项目";
    context.name = L"报告.docx";
    const std::wstring expanded = ExpandCommandText(L"{dir}\\{name}-{date}-{time}", context);
    Check(expanded.rfind(L"C:\\work\\项目\\报告.docx-", 0) == 0,
          "placeholders expand dir, name, date and time");
    Check(ExpandCommandText(L"{unknown} stays", context) == L"{unknown} stays",
          "unknown placeholders are left as typed");

    // 2. Steps run in order and are idempotent, like File+.
    const std::wstring root = TempRoot();
    CustomCommand command;
    command.name = L"新建项目结构";
    command.background_menu = true;
    command.steps.push_back({CommandStepKind::NewFolder, L"01资料"});
    command.steps.push_back({CommandStepKind::NewFolder, L"02计算"});
    command.steps.push_back({CommandStepKind::NewFile, L"说明.txt"});

    CommandContext run;
    run.dir = root;
    std::wstring error;
    Check(RunCustomCommand(command, run, error), "steps create the folders and the file");
    Check(Exists(root + L"\\01资料") && Exists(root + L"\\02计算") && Exists(root + L"\\说明.txt"),
          "every step is on disk");
    error.clear();
    Check(RunCustomCommand(command, run, error), "a second run is not an error");
    Check(error.empty(), "the second run reports no error");

    // 3. {name} reaches a step.
    CustomCommand named;
    named.name = L"按名字建目录";
    named.steps.push_back({CommandStepKind::NewFolder, L"选中-{name}"});
    CommandContext with_name;
    with_name.dir = root;
    with_name.name = L"张三";
    Check(RunCustomCommand(named, with_name, error) && Exists(root + L"\\选中-张三"),
          "{name} is replaced with the selected item");

    // 4. A batch script runs through cmd with UTF-8.
    CustomCommand script;
    script.name = L"脚本";
    script.steps.push_back({CommandStepKind::Script, L"echo 中文内容> 脚本输出.txt"});
    Check(RunCustomCommand(script, run, error), "a batch script runs");
    Check(Exists(root + L"\\脚本输出.txt"), "the script wrote its file");

    // 5. Store round trip.
    g_data_dir = root;
    CustomCommandStore store;
    store.commands.push_back(command);
    store.commands.push_back(named);
    Check(store.Save(), "commands.json is written");
    CustomCommandStore loaded;
    Check(loaded.Load(), "commands.json is read back");
    Check(loaded.commands.size() == 2 && loaded.commands[0].name == L"新建项目结构" &&
          loaded.commands[0].steps.size() == 3 &&
          loaded.commands[0].steps[2].kind == CommandStepKind::NewFile &&
          loaded.commands[1].steps[0].value == L"选中-{name}",
          "every command and step survives the round trip");

    std::printf("\n%d passed, %d failed\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}
