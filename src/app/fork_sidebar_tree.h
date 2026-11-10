// fork_sidebar_tree.h — 侧栏文件夹树（就地展开）。
//
// 官方的侧栏是"位置列表"：驱动器、收藏夹都只是一行，点了就导航，不能就地展开。
// 这里给每个文件夹行加一个行末箭头，展开后把子文件夹插进同一组里（缩进一级），
// 像资源管理器导航窗格。子项在 I/O 线程枚举，回来再插进视图模型。
#pragma once
#include <windows.h>

#include <string>
#include <vector>

namespace pulse::ui {
struct WindowViewModel;
}

namespace pulse {

struct AppState;

// 后台枚举回来了（lparam 是 new std::pair<std::wstring, std::vector<std::wstring>>）。
inline constexpr UINT WM_FORK_SIDEBAR_TREE = WM_APP + 201;

// 侧栏文件夹树的展开状态与子项缓存。
struct ForkSidebarTreeState {
    std::vector<std::wstring> expanded;                       // 展开中的文件夹（有序，便于调试）
    std::vector<std::pair<std::wstring, std::vector<std::wstring>>> children;  // 已枚举的子文件夹
};

// 行末箭头：展开/收起。返回 true 表示这是文件夹树的行（调用方只管重绘）。
bool ForkSidebarTreeToggle(AppState& s, const std::wstring& path);

// 点行时只展开（已展开就什么都不做），避免一边导航一边收起。
bool ForkSidebarTreeExpand(AppState& s, const std::wstring& path);

// 构建视图模型后调用：给文件夹行加箭头，并把已展开文件夹的子项插进去。
void ForkSidebarTreeApply(AppState& s, ui::WindowViewModel& vm);

// WM_FORK_SIDEBAR_TREE：子文件夹枚举完成。
void ForkSidebarTreeLoaded(AppState& s, LPARAM lparam);

} // namespace pulse
