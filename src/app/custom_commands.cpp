// custom_commands.cpp — 自定义命令: the commands.json store and the step runner
// (ported from File+ M90 so both apps behave the same).
#include "custom_commands.h"

#include "../common/app_dirs.h"
#include "../common/json_value.h"
#include "../common/utf8_file.h"
#include <windows.h>
#include <string>
#include <vector>

namespace pulse::app {
namespace {

std::wstring Pad(int value) {
    std::wstring text = std::to_wstring(value);
    return text.size() < 2 ? L"0" + text : text;
}

std::wstring Today() {
    SYSTEMTIME now{};
    GetLocalTime(&now);
    return std::to_wstring(now.wYear) + L"-" + Pad(now.wMonth) + L"-" + Pad(now.wDay);
}

std::wstring Clock() {
    SYSTEMTIME now{};
    GetLocalTime(&now);
    return Pad(now.wHour) + L":" + Pad(now.wMinute) + L":" + Pad(now.wSecond);
}

std::wstring JoinDir(const std::wstring& dir, const std::wstring& name) {
    if (dir.empty()) return name;
    if (dir.back() == L'\\' || dir.back() == L'/') return dir + name;
    return dir + L"\\" + name;
}

// "C:\Windows\notepad.exe" {dir} -> program + the rest of the line.
void SplitCommandLine(const std::wstring& line, std::wstring& program, std::wstring& args) {
    size_t i = 0;
    if (!line.empty() && line[0] == L'"') {
        const size_t close = line.find(L'"', 1);
        if (close == std::wstring::npos) {
            program = line.substr(1);
            i = line.size();
        } else {
            program = line.substr(1, close - 1);
            i = close + 1;
        }
    } else {
        const size_t space = line.find(L' ');
        program = space == std::wstring::npos ? line : line.substr(0, space);
        i = space == std::wstring::npos ? line.size() : space;
    }
    while (i < line.size() && line[i] == L' ') ++i;
    args = line.substr(i);
}

bool StartProcess(const std::wstring& command_line, const std::wstring& directory) {
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    std::wstring mutable_line = command_line;
    const BOOL started = CreateProcessW(nullptr, mutable_line.data(), nullptr, nullptr, FALSE,
                                        CREATE_NO_WINDOW, nullptr,
                                        directory.empty() ? nullptr : directory.c_str(), &si, &pi);
    if (!started) return false;
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}

// 批处理脚本: the text runs through cmd after chcp 65001 so Chinese names from
// {dir} / {name} survive. Returns the exit code, or -1 when it never started.
int RunScript(const std::wstring& script, const std::wstring& directory) {
    wchar_t temp[MAX_PATH]{};
    if (!GetTempPathW(MAX_PATH, temp)) return -1;
    const std::wstring path = std::wstring(temp) + L"filep-command-" +
        std::to_wstring(GetCurrentProcessId()) + L"-" +
        std::to_wstring(GetTickCount64()) + L".cmd";
    const std::wstring body = L"@echo off\r\nchcp 65001 >nul\r\n" + script + L"\r\n";
    if (!WriteUtf8FileAtomic(path, body)) return -1;
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    std::wstring line = L"cmd.exe /c \"" + path + L"\"";
    const BOOL started = CreateProcessW(nullptr, line.data(), nullptr, nullptr, FALSE,
                                        CREATE_NO_WINDOW, nullptr,
                                        directory.empty() ? nullptr : directory.c_str(), &si, &pi);
    if (!started) {
        DeleteFileW(path.c_str());
        return -1;
    }
    WaitForSingleObject(pi.hProcess, 120000);
    DWORD code = 1;
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    DeleteFileW(path.c_str());
    return static_cast<int>(code);
}

std::wstring StorePath() {
    const std::wstring dir = GetPulseConfigDir();
    return dir.empty() ? std::wstring() : dir + L"\\commands.json";
}

const wchar_t* KindKey(CommandStepKind kind) {
    switch (kind) {
    case CommandStepKind::NewFile: return L"file";
    case CommandStepKind::RunProgram: return L"program";
    case CommandStepKind::Script: return L"script";
    default: return L"folder";
    }
}

CommandStepKind KindFrom(const std::wstring& key) {
    if (key == L"file") return CommandStepKind::NewFile;
    if (key == L"program") return CommandStepKind::RunProgram;
    if (key == L"script") return CommandStepKind::Script;
    return CommandStepKind::NewFolder;
}

} // namespace

std::wstring ExpandCommandText(const std::wstring& text, const CommandContext& context) {
    struct Token { const wchar_t* name; std::wstring value; };
    const Token tokens[] = {
        {L"{dir}", context.dir},
        {L"{name}", context.name},
        {L"{date}", Today()},
        {L"{time}", Clock()},
    };
    std::wstring out = text;
    for (const auto& token : tokens) {
        const std::wstring key = token.name;
        size_t at = out.find(key);
        while (at != std::wstring::npos) {
            out.replace(at, key.size(), token.value);
            at = out.find(key, at + token.value.size());
        }
    }
    return out;
}

bool RunCustomCommand(const CustomCommand& command, const CommandContext& context,
                      std::wstring& error) {
    error.clear();
    for (size_t i = 0; i < command.steps.size(); ++i) {
        const CommandStep& step = command.steps[i];
        const std::wstring text = ExpandCommandText(step.value, context);
        if (text.empty()) continue;
        const std::wstring where = command.name + L" (#" + std::to_wstring(i + 1) + L"): ";
        switch (step.kind) {
        case CommandStepKind::NewFolder: {
            const std::wstring path = JoinDir(context.dir, text);
            if (CreateDirectoryW(path.c_str(), nullptr)) break;
            if (GetLastError() == ERROR_ALREADY_EXISTS) break;  // keeps it, like File+
            error = where + L"cannot create folder " + path;
            return false;
        }
        case CommandStepKind::NewFile: {
            const std::wstring path = JoinDir(context.dir, text);
            const HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW,
                                            FILE_ATTRIBUTE_NORMAL, nullptr);
            if (file != INVALID_HANDLE_VALUE) {
                CloseHandle(file);
                break;
            }
            if (GetLastError() == ERROR_FILE_EXISTS) break;
            error = where + L"cannot create file " + path;
            return false;
        }
        case CommandStepKind::RunProgram: {
            std::wstring program, args;
            SplitCommandLine(text, program, args);
            if (program.empty()) {
                error = where + L"no program to run";
                return false;
            }
            if (!StartProcess(text, context.dir)) {
                error = where + L"cannot start " + program;
                return false;
            }
            break;
        }
        case CommandStepKind::Script: {
            const int code = RunScript(text, context.dir);
            if (code != 0) {
                error = where + L"script failed with exit code " + std::to_wstring(code);
                return false;
            }
            break;
        }
        }
    }
    return true;
}

bool CustomCommandStore::Load() {
    commands.clear();
    load_failed = false;
    const std::wstring path = StorePath();
    if (path.empty()) return true;
    std::wstring text;
    if (!ReadUtf8File(path, text)) return true;  // no file yet: an empty list
    if (text.empty()) return true;
    json::Value root;
    std::string parse_error;
    if (!json::Parse(json::Utf8(text), root, &parse_error) || !root.is_object()) {
        load_failed = true;
        return false;
    }
    if (const json::Value* list = root.find("commands"); list && list->is_array()) {
        for (const json::Value& item : list->items()) {
            if (!item.is_object()) continue;
            CustomCommand command;
            command.name = json::Wide(item.get_string("name"));
            command.toolbar = item.get_bool("toolbar");
            command.folder_menu = item.get_bool("folder");
            command.background_menu = item.get_bool("background");
            if (const json::Value* steps = item.find("steps"); steps && steps->is_array()) {
                for (const json::Value& entry : steps->items()) {
                    if (!entry.is_object()) continue;
                    CommandStep step;
                    step.kind = KindFrom(json::Wide(entry.get_string("kind")));
                    step.value = json::Wide(entry.get_string("value"));
                    command.steps.push_back(std::move(step));
                }
            }
            if (!command.name.empty()) commands.push_back(std::move(command));
        }
    }
    return true;
}

bool CustomCommandStore::Save() const {
    if (!persist) return true;
    const std::wstring path = StorePath();
    if (path.empty()) return false;
    json::Value root = json::Value::Object();
    root.set("version", 1);
    json::Value list = json::Value::Array();
    for (const CustomCommand& command : commands) {
        json::Value item = json::Value::Object();
        item.set("name", json::Utf8(command.name));
        item.set("toolbar", command.toolbar);
        item.set("folder", command.folder_menu);
        item.set("background", command.background_menu);
        json::Value steps = json::Value::Array();
        for (const CommandStep& step : command.steps) {
            json::Value entry = json::Value::Object();
            entry.set("kind", json::Utf8(KindKey(step.kind)));
            entry.set("value", json::Utf8(step.value));
            steps.push(std::move(entry));
        }
        item.set("steps", std::move(steps));
        list.push(std::move(item));
    }
    root.set("commands", std::move(list));
    return WriteUtf8FileAtomic(path, json::Wide(json::Serialize(root)));
}

} // namespace pulse::app
