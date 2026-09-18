// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Interface/IconGeometry.h"

#include <cmath>
#include <numbers>

namespace hmi {

namespace {

constexpr float PI = std::numbers::pi_v<float>;

[[nodiscard]] std::vector<IconPoint> circlePoints(float cx, float cy, float radius,
                                                  int segments = 16) {
    std::vector<IconPoint> points;
    points.reserve(static_cast<std::size_t>(segments));
    for (int i = 0; i < segments; ++i) {
        const float angle = 2.0F * PI * static_cast<float>(i) / static_cast<float>(segments);
        points.push_back(
            IconPoint{.x = cx + (radius * std::cos(angle)), .y = cy + (radius * std::sin(angle))});
    }
    return points;
}

[[nodiscard]] IconStroke rectStroke(float x, float y, float width, float height, bool filled,
                                    IconColorRole color) {
    return IconStroke{
        .points = {IconPoint{.x = x, .y = y}, IconPoint{.x = x + width, .y = y},
                   IconPoint{.x = x + width, .y = y + height}, IconPoint{.x = x, .y = y + height}},
        .closed = true,
        .filled = filled,
        .color = color};
}

}  // namespace

IconGeometry iconGeometry(IconId id) {
    switch (id) {
        case IconId::ToolPaint:
            return IconGeometry{{
                IconStroke{.points = {IconPoint{.x = 0.20F, .y = 0.80F},
                                      IconPoint{.x = 0.58F, .y = 0.42F}},
                           .closed = false,
                           .filled = false,
                           .color = IconColorRole::Foreground},
                IconStroke{.points = circlePoints(0.70F, 0.30F, 0.15F),
                           .closed = true,
                           .filled = true,
                           .color = IconColorRole::Accent},
            }};
        case IconId::ToolRectangle:
            return IconGeometry{
                {rectStroke(0.20F, 0.20F, 0.60F, 0.60F, false, IconColorRole::Foreground)}};
        case IconId::ToolSelection:
            return IconGeometry{{
                IconStroke{
                    .points = {IconPoint{.x = 0.20F, .y = 0.35F}, IconPoint{.x = 0.20F, .y = 0.20F},
                               IconPoint{.x = 0.35F, .y = 0.20F}},
                    .closed = false,
                    .filled = false,
                    .color = IconColorRole::Foreground},
                IconStroke{
                    .points = {IconPoint{.x = 0.65F, .y = 0.20F}, IconPoint{.x = 0.80F, .y = 0.20F},
                               IconPoint{.x = 0.80F, .y = 0.35F}},
                    .closed = false,
                    .filled = false,
                    .color = IconColorRole::Foreground},
                IconStroke{
                    .points = {IconPoint{.x = 0.80F, .y = 0.65F}, IconPoint{.x = 0.80F, .y = 0.80F},
                               IconPoint{.x = 0.65F, .y = 0.80F}},
                    .closed = false,
                    .filled = false,
                    .color = IconColorRole::Foreground},
                IconStroke{
                    .points = {IconPoint{.x = 0.35F, .y = 0.80F}, IconPoint{.x = 0.20F, .y = 0.80F},
                               IconPoint{.x = 0.20F, .y = 0.65F}},
                    .closed = false,
                    .filled = false,
                    .color = IconColorRole::Foreground},
            }};
        case IconId::ToolEntity:
            // Une epingle plantee sur une case (LOT-11) : tete pleine (accent), tige, et la case au
            // sol -- « poser quelque chose ici », distinct du pinceau qui couvre la case.
            return IconGeometry{{
                IconStroke{.points = {IconPoint{.x = 0.20F, .y = 0.85F},
                                      IconPoint{.x = 0.80F, .y = 0.85F}},
                           .closed = false,
                           .filled = false,
                           .color = IconColorRole::Foreground},
                IconStroke{.points = {IconPoint{.x = 0.50F, .y = 0.45F},
                                      IconPoint{.x = 0.50F, .y = 0.85F}},
                           .closed = false,
                           .filled = false,
                           .color = IconColorRole::Foreground},
                IconStroke{.points = circlePoints(0.50F, 0.30F, 0.16F),
                           .closed = true,
                           .filled = true,
                           .color = IconColorRole::Accent},
            }};
        case IconId::Save:
            return IconGeometry{{
                rectStroke(0.20F, 0.15F, 0.60F, 0.70F, false, IconColorRole::Foreground),
                rectStroke(0.35F, 0.15F, 0.30F, 0.20F, true, IconColorRole::Foreground),
                rectStroke(0.30F, 0.55F, 0.40F, 0.25F, false, IconColorRole::Foreground),
            }};
        case IconId::Playtest:
            return IconGeometry{{IconStroke{
                .points = {IconPoint{.x = 0.30F, .y = 0.20F}, IconPoint{.x = 0.30F, .y = 0.80F},
                           IconPoint{.x = 0.80F, .y = 0.50F}},
                .closed = true,
                .filled = true,
                .color = IconColorRole::Accent}}};
        case IconId::Undo:
            return IconGeometry{{
                IconStroke{
                    .points = {IconPoint{.x = 0.75F, .y = 0.5F}, IconPoint{.x = 0.32F, .y = 0.5F}},
                    .closed = false,
                    .filled = false,
                    .color = IconColorRole::Foreground},
                IconStroke{
                    .points = {IconPoint{.x = 0.32F, .y = 0.5F}, IconPoint{.x = 0.47F, .y = 0.35F},
                               IconPoint{.x = 0.47F, .y = 0.65F}},
                    .closed = true,
                    .filled = true,
                    .color = IconColorRole::Foreground},
            }};
        case IconId::Redo:
            return IconGeometry{{
                IconStroke{
                    .points = {IconPoint{.x = 0.25F, .y = 0.5F}, IconPoint{.x = 0.68F, .y = 0.5F}},
                    .closed = false,
                    .filled = false,
                    .color = IconColorRole::Foreground},
                IconStroke{
                    .points = {IconPoint{.x = 0.68F, .y = 0.5F}, IconPoint{.x = 0.53F, .y = 0.35F},
                               IconPoint{.x = 0.53F, .y = 0.65F}},
                    .closed = true,
                    .filled = true,
                    .color = IconColorRole::Foreground},
            }};
        case IconId::ToggleGrid:
            return IconGeometry{{
                IconStroke{
                    .points = {IconPoint{.x = 0.37F, .y = 0.2F}, IconPoint{.x = 0.37F, .y = 0.8F}},
                    .closed = false,
                    .filled = false,
                    .color = IconColorRole::Foreground},
                IconStroke{
                    .points = {IconPoint{.x = 0.63F, .y = 0.2F}, IconPoint{.x = 0.63F, .y = 0.8F}},
                    .closed = false,
                    .filled = false,
                    .color = IconColorRole::Foreground},
                IconStroke{
                    .points = {IconPoint{.x = 0.2F, .y = 0.37F}, IconPoint{.x = 0.8F, .y = 0.37F}},
                    .closed = false,
                    .filled = false,
                    .color = IconColorRole::Foreground},
                IconStroke{
                    .points = {IconPoint{.x = 0.2F, .y = 0.63F}, IconPoint{.x = 0.8F, .y = 0.63F}},
                    .closed = false,
                    .filled = false,
                    .color = IconColorRole::Foreground},
            }};
        case IconId::ResetCamera:
            return IconGeometry{{
                IconStroke{.points = circlePoints(0.5F, 0.5F, 0.30F),
                           .closed = true,
                           .filled = false,
                           .color = IconColorRole::Foreground},
                IconStroke{.points = circlePoints(0.5F, 0.5F, 0.07F),
                           .closed = true,
                           .filled = true,
                           .color = IconColorRole::Accent},
            }};
        case IconId::Copy:
            return IconGeometry{{
                rectStroke(0.18F, 0.30F, 0.44F, 0.50F, false, IconColorRole::Foreground),
                rectStroke(0.38F, 0.18F, 0.44F, 0.50F, true, IconColorRole::Accent),
            }};
        case IconId::Paste:
            return IconGeometry{{
                rectStroke(0.22F, 0.26F, 0.56F, 0.58F, false, IconColorRole::Foreground),
                rectStroke(0.38F, 0.15F, 0.24F, 0.14F, true, IconColorRole::Accent),
            }};
        case IconId::Rename:
            return IconGeometry{{
                IconStroke{.points = {IconPoint{.x = 0.20F, .y = 0.80F},
                                      IconPoint{.x = 0.60F, .y = 0.40F}},
                           .closed = false,
                           .filled = false,
                           .color = IconColorRole::Foreground},
                IconStroke{
                    .points = {IconPoint{.x = 0.60F, .y = 0.40F}, IconPoint{.x = 0.76F, .y = 0.24F},
                               IconPoint{.x = 0.84F, .y = 0.32F},
                               IconPoint{.x = 0.68F, .y = 0.48F}},
                    .closed = true,
                    .filled = true,
                    .color = IconColorRole::Accent},
                IconStroke{.points = {IconPoint{.x = 0.18F, .y = 0.82F},
                                      IconPoint{.x = 0.30F, .y = 0.82F}},
                           .closed = false,
                           .filled = false,
                           .color = IconColorRole::Foreground},
            }};
        case IconId::ShortcutsOverview:
            return IconGeometry{{
                rectStroke(0.16F, 0.34F, 0.16F, 0.16F, true, IconColorRole::Accent),
                rectStroke(0.42F, 0.34F, 0.16F, 0.16F, true, IconColorRole::Accent),
                rectStroke(0.68F, 0.34F, 0.16F, 0.16F, true, IconColorRole::Accent),
                rectStroke(0.16F, 0.56F, 0.68F, 0.14F, false, IconColorRole::Foreground),
            }};
    }
    return IconGeometry{};
}

}  // namespace hmi
