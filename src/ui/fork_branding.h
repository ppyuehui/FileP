// fork_branding.h — FileP 自己的品牌处理。
//
// 和官方不同的地方只有两处：
//   1. 标题栏左上角只留侧栏开关，产品图标和名字搬到状态栏左下角；
//   2. 状态栏左下角的「图标 + FileP」由这里量宽度、这里画。
//
// 官方文件里只留常量判断和一次调用，合并官方新版时基本不会冲突。
// 注意：参数只用基本类型/矩形，不引用 ui_renderer_internal.h 里的内部结构
// （那个结构在匿名命名空间里，跨编译单元类型不同）。
#pragma once
#include <d2d1.h>
#include <windows.h>

struct IDWriteFactory2;
struct IDWriteTextFormat;

namespace pulse::ui {

// 标题栏品牌区只留侧栏开关时需要的宽度（DIP）。
inline constexpr float kForkTitleBrandDip = 52.0f;
// true：标题栏不画产品图标/名字（品牌在状态栏左下角）。
inline constexpr bool kForkHidesTitleBrand = true;

// 状态栏左下角品牌块的宽度。
// bar/pad/full 来自状态栏布局；成功时写回 brand 与 content_left 并返回 true。
bool ForkStatusBrandRect(const D2D1_RECT_F& bar, float pad, const D2D1_RECT_F& full, float scale,
                         IDWriteFactory2* factory, IDWriteTextFormat* small_format,
                         D2D1_RECT_F* brand, float* content_left);

} // namespace pulse::ui
