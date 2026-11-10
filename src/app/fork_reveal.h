// fork_reveal.h — "打开所在位置"的迟到选中。
//
// 官方的手法是"跳转前把文件名记在标签页上"（pending_selected_name），等列表到了再选。
// 实测这条链路会丢：导航过程中选中被清空、revision 变化，列表到达时 pending 被当成
// 过期旧选中覆盖掉，结果只跳了目录、没选中文件。
//
// 这里改成一个独立请求：记下"目标文件夹 + 文件名"，每帧构建视图模型时检查，
// 一旦当前标签页真的在那个文件夹、列表里也有这个名字，就当场选中并滚动到可见。
#pragma once
#include <string>
#include <vector>

namespace pulse {

struct AppState;

// 到 folder/name 之后选中它（names 可以是多选）。folder 为空时按 name 的父目录算。
void ForkRequestReveal(AppState& s, const std::wstring& full_path);
void ForkRequestRevealMany(AppState& s, const std::wstring& folder,
                          const std::vector<std::wstring>& names);

// BuildVm 开头调用：列表到位就选中，然后清掉请求。
void ForkApplyReveal(AppState& s);

} // namespace pulse
