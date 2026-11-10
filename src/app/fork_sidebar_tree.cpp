// fork_sidebar_tree.cpp — 见 fork_sidebar_tree.h。
#include "fork_sidebar_tree.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "../common/localization.h"
#include "../ui/ui_view_model.h"
#include "app_state.h"

namespace pulse {
namespace {

constexpr size_t kMaxChildren = 300;   // 一个文件夹最多列这么多子文件夹
constexpr int kMaxDepth = 12;

bool IsRealFolderPath(const std::wstring& path) {
    // 只处理真实文件系统路径：虚拟位置（pulse:...）交给官方逻辑。
    return path.size() > 3 && (path.rfind(L"\\\\?\\", 0) == 0 || path.find(L":\\") != std::wstring::npos) &&
           path.rfind(L"pulse:", 0) != 0;
}

std::wstring LeafName(const std::wstring& path) {
    const size_t slash = path.find_last_of(L"\\/");
    return slash == std::wstring::npos ? path : path.substr(slash + 1);
}

std::vector<std::wstring> ListChildFolders(const std::wstring& path) {
    std::vector<std::wstring> out;
    WIN32_FIND_DATAW data{};
    std::wstring pattern = path;
    if (pattern.empty() || pattern.back() != L'\\') pattern.push_back(L'\\');
    pattern.push_back(L'*');
    HANDLE find = FindFirstFileExW(pattern.c_str(), FindExInfoBasic, &data, FindExSearchLimitToDirectories,
                                   nullptr, FIND_FIRST_EX_LARGE_FETCH);
    if (find == INVALID_HANDLE_VALUE) return out;
    do {
        if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
        if (data.cFileName[0] == L'.' &&
            (data.cFileName[1] == L'\0' || (data.cFileName[1] == L'.' && data.cFileName[2] == L'\0')))
            continue;
        std::wstring child = path;
        if (child.back() != L'\\') child.push_back(L'\\');
        child += data.cFileName;
        out.push_back(std::move(child));
        if (out.size() >= kMaxChildren) break;
    } while (FindNextFileW(find, &data));
    FindClose(find);
    std::sort(out.begin(), out.end(), [](const std::wstring& a, const std::wstring& b) {
        return _wcsicmp(LeafName(a).c_str(), LeafName(b).c_str()) < 0;
    });
    return out;
}

ForkSidebarTreeState& State(AppState& s) {
    if (!s.forkSidebarTree) s.forkSidebarTree = std::make_shared<ForkSidebarTreeState>();
    return *s.forkSidebarTree;
}

bool IsExpanded(const ForkSidebarTreeState& st, const std::wstring& path) {
    return std::find(st.expanded.begin(), st.expanded.end(), path) != st.expanded.end();
}

const std::vector<std::wstring>* ChildrenOf(const ForkSidebarTreeState& st, const std::wstring& path) {
    for (const auto& entry : st.children)
        if (entry.first == path) return &entry.second;
    return nullptr;
}

} // namespace

bool ForkSidebarTreeToggle(AppState& s, const std::wstring& path) {
    if (!IsRealFolderPath(path)) return false;
    ForkSidebarTreeState& st = State(s);
    const auto at = std::find(st.expanded.begin(), st.expanded.end(), path);
    if (at != st.expanded.end()) {
        st.expanded.erase(at);
        return true;
    }
    st.expanded.push_back(path);
    if (ChildrenOf(st, path)) return true;   // 已经枚举过
    // 第一次展开：后台列出子文件夹，回来后插进视图模型。
    const HWND hwnd = s.hwnd;
    s.worker.EnqueueIo([path, hwnd] {
        auto payload = std::make_unique<std::pair<std::wstring, std::vector<std::wstring>>>(
            path, ListChildFolders(path));
        if (!PostMessageW(hwnd, WM_FORK_SIDEBAR_TREE, 0, reinterpret_cast<LPARAM>(payload.get())))
            return;
        payload.release();
    });
    return true;
}

bool ForkSidebarTreeExpand(AppState& s, const std::wstring& path) {
    if (!IsRealFolderPath(path)) return false;
    ForkSidebarTreeState& st = State(s);
    if (IsExpanded(st, path)) return true;
    return ForkSidebarTreeToggle(s, path);
}

void ForkSidebarTreeApply(AppState& s, ui::WindowViewModel& vm) {
    ForkSidebarTreeState* st = s.forkSidebarTree.get();
    for (ui::SidebarGroup& group : vm.sidebar) {
        if (group.items.empty()) continue;
        std::vector<ui::SidebarItem> out;
        out.reserve(group.items.size() + 16);
        // 子项插在当前行之后；递归处理展开的子项，层数有上限。
        const auto emit = [&](const ui::SidebarItem& item, int depth, const auto& self) -> void {
            const bool folder = IsRealFolderPath(item.path) && !item.starred_child;
            out.push_back(item);
            if (!folder) return;
            out.back().expandable = true;   // 行末箭头
            if (depth >= kMaxDepth || !st) return;
            if (!IsExpanded(*st, item.path)) return;
            out.back().expanded = true;
            const std::vector<std::wstring>* kids = ChildrenOf(*st, item.path);
            if (!kids) return;              // 还在枚举
            for (const std::wstring& child : *kids) {
                ui::SidebarItem row;
                row.label = LeafName(child);
                row.path = child;
                row.icon_glyph = L"\xE8B7";
                row.fallback_text = L"Dir";
                row.icon_color = ui::HexColor(0xF5C451);
                row.indent = item.indent + 1;
                row.expandable = true;
                row.expanded = IsExpanded(*st, child);
                self(row, depth + 1, self);
            }
        };
        for (const ui::SidebarItem& item : group.items) emit(item, 0, emit);
        group.items = std::move(out);
    }
}

void ForkSidebarTreeLoaded(AppState& s, LPARAM lparam) {
    std::unique_ptr<std::pair<std::wstring, std::vector<std::wstring>>> payload(
        reinterpret_cast<std::pair<std::wstring, std::vector<std::wstring>>*>(lparam));
    if (!payload) return;
    ForkSidebarTreeState& st = State(s);
    for (auto& entry : st.children) {
        if (entry.first != payload->first) continue;
        entry.second = std::move(payload->second);
        return;
    }
    st.children.push_back(std::move(*payload));
}

} // namespace pulse
