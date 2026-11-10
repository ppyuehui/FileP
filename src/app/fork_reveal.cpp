// fork_reveal.cpp — 见 fork_reveal.h。
#include "fork_reveal.h"

#include <algorithm>
#include <unordered_map>

#include "app_input.h"
#include "app_runtime.h"
#include "app_state.h"

namespace pulse {
namespace {

struct RevealRequest {
    std::wstring folder;
    std::vector<std::wstring> names;
    ULONGLONG deadline = 0;
};

// 每个窗口一份；窗口关掉后条目不回收也不碍事（很小，且带超时）。
std::unordered_map<const AppState*, RevealRequest> g_requests;

// 列表里的 current_path 是给人看的文本（C:\dev），而 SelectedFullPath 带 \\?\ 前缀
// （\\?\C:\dev\x.png）。两边先剥成同一个样子再比，否则永远不相等。
std::wstring PlainPath(std::wstring value) {
    if (value.rfind(L"\\\\?\\UNC\\", 0) == 0) value = L"\\\\" + value.substr(8);
    else if (value.rfind(L"\\\\?\\", 0) == 0) value.erase(0, 4);
    while (value.size() > 1 && (value.back() == L'\\' || value.back() == L'/') &&
           value[value.size() - 2] != L':')
        value.pop_back();
    return value;
}

bool SamePath(const std::wstring& a, const std::wstring& b) {
    if (a.empty() || b.empty()) return false;
    return _wcsicmp(PlainPath(a).c_str(), PlainPath(b).c_str()) == 0;
}

} // namespace

void ForkRequestRevealMany(AppState& s, const std::wstring& folder,
                           const std::vector<std::wstring>& names) {
    if (folder.empty() || names.empty()) return;
    RevealRequest& request = g_requests[&s];
    request.folder = folder;
    request.names = names;
    request.deadline = GetTickCount64() + 15000;   // 15 秒内没走到就作废
}

void ForkRequestReveal(AppState& s, const std::wstring& full_path) {
    const size_t slash = full_path.find_last_of(L"\\/");
    if (slash == std::wstring::npos) return;
    const std::wstring folder = full_path.substr(0, slash);
    const std::wstring name = full_path.substr(slash + 1);
    if (name.empty()) return;
    ForkRequestRevealMany(s, folder, {name});
}

void ForkApplyReveal(AppState& s) {
    const auto found = g_requests.find(&s);
    if (found == g_requests.end()) return;
    RevealRequest& request = found->second;
    if (GetTickCount64() > request.deadline) {
        g_requests.erase(found);
        return;
    }
    app::Tab* tab = ActiveTab(s);
    if (!tab || !tab->snapshot || !SamePath(tab->current_path, request.folder)) return;
    std::vector<int> rows;
    int focus = -1;
    for (size_t i = 0; i < tab->snapshot->size(); ++i) {
        const auto& entry = (*tab->snapshot)[i];
        for (const auto& name : request.names) {
            if (_wcsicmp(entry.name.c_str(), name.c_str()) != 0) continue;
            rows.push_back(static_cast<int>(i));
            if (focus < 0) focus = static_cast<int>(i);
            break;
        }
    }
    g_requests.erase(found);   // 列表到了：无论有没有这个名字，这次请求都结束
    if (rows.empty()) return;
    tab->SelectIndices(rows);
    if (focus >= 0) {
        tab->selected_index = tab->selection_anchor = focus;
        EnsureRowVisible(s, *tab, focus);
    }
}

} // namespace pulse
