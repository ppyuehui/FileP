// fork_branding.cpp — 见 fork_branding.h。所有和品牌有关的绘制都收在这里，
// 官方的 ui_renderer.cpp / ui_renderer_internal.h 只负责调用。
#include "fork_branding.h"

#include "ui_renderer.h"
#include "ui_renderer_internal.h"

namespace pulse::ui {

bool ForkStatusBrandRect(const D2D1_RECT_F& bar, float pad, const D2D1_RECT_F& full, float scale,
                         IDWriteFactory2* factory, IDWriteTextFormat* small_format,
                         D2D1_RECT_F* brand, float* content_left) {
    // 左下角的品牌：状态栏最左边是「FileP」，右边才是状态文字。
    if (!factory || !small_format || !brand || full.right - full.left <= 720.0f * scale) return false;
    const float name_w = MeasureTextWidth(factory, small_format, L"FileP") * 1.06f + 1.0f;
    const float desc_w = MeasureTextWidth(factory, small_format,
                                          l10n::Get(l10n::StringId::AppDescription));
    const float left = full.left + pad;
    *brand = D2D1::RectF(left, bar.top, left + name_w + 6.0f * scale + desc_w, bar.bottom);
    if (content_left) *content_left = brand->right + 14.0f * scale;
    return true;
}

void MainRenderer::DrawForkStatusBrand(ID2D1DeviceContext* dc, const Theme& theme,
                                       const D2D1_RECT_F& bar, const D2D1_RECT_F& brand,
                                       IDWriteTextFormat* small_format) {
    if (brand.right <= brand.left || !dc || !small_format) return;
    const float mark = 14.0f * scale_;
    const float mark_y = bar.top + (status_height_ - mark) * 0.5f;
    const float mark_x = brand.left;
    if (ID2D1Bitmap* logo = LogoBitmap()) {
        const D2D1_SIZE_U ls = logo->GetPixelSize();
        const float lx = std::round(mark_x + (mark - static_cast<float>(ls.width)) * 0.5f);
        const float ly = std::round(mark_y + (mark - static_cast<float>(ls.height)) * 0.5f);
        dc->DrawBitmap(logo, D2D1::RectF(lx, ly, lx + static_cast<float>(ls.width),
                                         ly + static_cast<float>(ls.height)),
                       1.0f, D2D1_INTERPOLATION_MODE_LINEAR, nullptr, nullptr);
    } else {
        MakeBrush(dc, theme.accent, brAccent_);
        dc->FillEllipse(D2D1::Ellipse(D2D1::Point2F(mark_x + mark * 0.5f, mark_y + mark * 0.5f),
                                      mark * 0.5f, mark * 0.5f), brAccent_.get());
    }
    MakeBrush(dc, theme.text, brText_);
    DrawTextRect(dc, small_format, brText_.get(), L"FileP", mark_x + mark + 6.0f * scale_,
                 bar.top, brand.right - (mark_x + mark + 6.0f * scale_), status_height_);
}

} // namespace pulse::ui
