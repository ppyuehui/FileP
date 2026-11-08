#include "app_window_title.h"
#include "app_runtime.h"
#include "window_identity_ui.h"
#include <string>

namespace pulse {
namespace {

// The taskbar title counts characters (a Chinese name costs one per character);
// a name longer than its limit keeps the limit and ends with an ellipsis.
std::wstring LimitName(const std::wstring& name, int limit) {
    if (limit <= 0) return name;
    size_t characters = 0;
    for (size_t i = 0; i < name.size();) {
        const wchar_t c = name[i];
        i += (c >= 0xD800 && c <= 0xDBFF && i + 1 < name.size()) ? 2u : 1u;
        if (++characters > static_cast<size_t>(limit)) break;
    }
    if (characters <= static_cast<size_t>(limit)) return name;
    std::wstring cut;
    size_t kept = 0;
    for (size_t i = 0; i < name.size() && kept < static_cast<size_t>(limit);) {
        const wchar_t c = name[i];
        const size_t step = (c >= 0xD800 && c <= 0xDBFF && i + 1 < name.size()) ? 2u : 1u;
        cut.append(name, i, step);
        i += step;
        ++kept;
    }
    return cut + L"\u2026";
}

// The folder above `path`: empty for a drive root, a UNC share root or a
// virtual tab (This PC, search results), which have nothing to name above them.
std::wstring ParentTitle(const std::wstring& path) {
    std::wstring trimmed = path;
    while (trimmed.size() > 1 && (trimmed.back() == L'\\' || trimmed.back() == L'/'))
        trimmed.pop_back();
    const size_t slash = trimmed.find_last_of(L"\\/");
    if (slash == std::wstring::npos || slash == 0) return {};
    std::wstring parent = trimmed.substr(0, slash);
    if (parent.size() == 2 && parent[1] == L':') return {};  // C: has nothing above it
    // \\server\share is a share root too: only the server sits above it, and
    // naming that would read as a folder.
    if (parent.size() > 2 && parent.compare(0, 2, L"\\\\") == 0 &&
        parent.find_first_of(L"\\/", 2) == std::wstring::npos)
        return {};
    return app::TabTitle(parent);
}

} // namespace

std::wstring TaskbarWindowTitle(const app::Tab* tab, int parent_chars, int current_chars) {
    if (!tab) return L"FileP";
    const std::wstring current = tab->virtual_title.empty()
        ? app::TabTitle(tab->current_path) : tab->virtual_title;
    if (current.empty()) return L"FileP";
    std::wstring title = LimitName(current, current_chars);
    if (tab->virtual_title.empty()) {
        const std::wstring parent = ParentTitle(tab->current_path);
        if (!parent.empty()) title = LimitName(parent, parent_chars) + L"\\" + title;
    }
    return title;
}

bool SyncTaskbarWindowTitle(AppState& s) {
    if (!s.hwnd) return false;
    // M7c: with two or more windows each is called by its name, so Alt+Tab
    // and the taskbar tell them apart.
    const auto title = WindowIdentityShown(s) ? WindowDisplayName(s)
        : TaskbarWindowTitle(ActiveTab(s), s.appPrefs.taskbar_parent_chars, s.appPrefs.taskbar_current_chars);
    const int length = GetWindowTextLengthW(s.hwnd);
    if (length == static_cast<int>(title.size())) {
        std::wstring current(static_cast<size_t>(length) + 1, L'\0');
        const int copied = GetWindowTextW(s.hwnd, current.data(), length + 1);
        current.resize(static_cast<size_t>(copied));
        if (current == title) return false;
    }
    return SetWindowTextW(s.hwnd, title.c_str()) != FALSE;
}
} // namespace pulse
