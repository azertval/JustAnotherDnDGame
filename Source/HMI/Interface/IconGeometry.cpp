// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

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
        case IconId::ToolLink:
            return IconGeometry{{
                IconStroke{.points = circlePoints(0.27F, 0.5F, 0.14F),
                           .closed = true,
                           .filled = false,
                           .color = IconColorRole::Foreground},
                IconStroke{
                    .points = {IconPoint{.x = 0.41F, .y = 0.5F}, IconPoint{.x = 0.59F, .y = 0.5F}},
                    .closed = false,
                    .filled = false,
                    .color = IconColorRole::Foreground},
                IconStroke{.points = circlePoints(0.73F, 0.5F, 0.14F),
                           .closed = true,
                           .filled = true,
                           .color = IconColorRole::Accent},
            }};
        case IconId::ToolTextureAssign:
            return IconGeometry{{
                rectStroke(0.15F, 0.15F, 0.70F, 0.70F, false, IconColorRole::Foreground),
                rectStroke(0.35F, 0.35F, 0.30F, 0.30F, true, IconColorRole::Accent),
            }};
        case IconId::ToolCameraZone:
            // Cadre de visee (memes coins en L que ToolSelection) avec un objectif au centre
            // (disque plein, accent) : distingue visuellement "dessiner une zone de camera" de
            // "selectionner une zone de contenu".
            return IconGeometry{{
                IconStroke{
                    .points = {IconPoint{.x = 0.15F, .y = 0.32F}, IconPoint{.x = 0.15F, .y = 0.15F},
                               IconPoint{.x = 0.32F, .y = 0.15F}},
                    .closed = false,
                    .filled = false,
                    .color = IconColorRole::Foreground},
                IconStroke{
                    .points = {IconPoint{.x = 0.68F, .y = 0.15F}, IconPoint{.x = 0.85F, .y = 0.15F},
                               IconPoint{.x = 0.85F, .y = 0.32F}},
                    .closed = false,
                    .filled = false,
                    .color = IconColorRole::Foreground},
                IconStroke{
                    .points = {IconPoint{.x = 0.85F, .y = 0.68F}, IconPoint{.x = 0.85F, .y = 0.85F},
                               IconPoint{.x = 0.68F, .y = 0.85F}},
                    .closed = false,
                    .filled = false,
                    .color = IconColorRole::Foreground},
                IconStroke{
                    .points = {IconPoint{.x = 0.32F, .y = 0.85F}, IconPoint{.x = 0.15F, .y = 0.85F},
                               IconPoint{.x = 0.15F, .y = 0.68F}},
                    .closed = false,
                    .filled = false,
                    .color = IconColorRole::Foreground},
                IconStroke{.points = circlePoints(0.5F, 0.5F, 0.12F),
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
        case IconId::ToggleRenderMode:
            return IconGeometry{{
                rectStroke(0.15F, 0.2F, 0.35F, 0.6F, true, IconColorRole::Accent),
                rectStroke(0.50F, 0.2F, 0.35F, 0.6F, false, IconColorRole::Foreground),
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
        case IconId::PixelBrush:
            // Manche vertical + touffe (cercle plein) : distinct de ToolPaint (manche diagonal,
            // cercle en haut a droite) pour rester reconnaissable meme cote a cote dans un menu.
            return IconGeometry{{
                IconStroke{
                    .points = {IconPoint{.x = 0.5F, .y = 0.15F}, IconPoint{.x = 0.5F, .y = 0.55F}},
                    .closed = false,
                    .filled = false,
                    .color = IconColorRole::Foreground},
                IconStroke{.points = circlePoints(0.5F, 0.72F, 0.16F),
                           .closed = true,
                           .filled = true,
                           .color = IconColorRole::Accent},
            }};
        case IconId::PixelEraser: {
            const std::vector<IconPoint> body{
                IconPoint{.x = 0.20F, .y = 0.55F}, IconPoint{.x = 0.45F, .y = 0.25F},
                IconPoint{.x = 0.82F, .y = 0.50F}, IconPoint{.x = 0.57F, .y = 0.80F}};
            return IconGeometry{{
                IconStroke{
                    .points = body, .closed = true, .filled = true, .color = IconColorRole::Accent},
                IconStroke{.points = body,
                           .closed = true,
                           .filled = false,
                           .color = IconColorRole::Foreground},
            }};
        }
        case IconId::PixelFill:
            // Bec verseur (triangle) + goutte (cercle plein) : pot de peinture qui se renverse.
            return IconGeometry{{
                IconStroke{
                    .points = {IconPoint{.x = 0.20F, .y = 0.30F}, IconPoint{.x = 0.60F, .y = 0.30F},
                               IconPoint{.x = 0.40F, .y = 0.65F}},
                    .closed = true,
                    .filled = false,
                    .color = IconColorRole::Foreground},
                IconStroke{.points = circlePoints(0.72F, 0.72F, 0.14F),
                           .closed = true,
                           .filled = true,
                           .color = IconColorRole::Accent},
            }};
        case IconId::PixelEyedropper:
            // Diagonale inverse de ToolPaint (cercle en bas a gauche plutot qu'en haut a droite) :
            // silhouette distincte tout en restant reconnaissable comme une pipette.
            return IconGeometry{{
                IconStroke{.points = {IconPoint{.x = 0.75F, .y = 0.25F},
                                      IconPoint{.x = 0.35F, .y = 0.65F}},
                           .closed = false,
                           .filled = false,
                           .color = IconColorRole::Foreground},
                IconStroke{.points = circlePoints(0.25F, 0.75F, 0.12F),
                           .closed = true,
                           .filled = true,
                           .color = IconColorRole::Accent},
            }};
        case IconId::PixelOpen:
            // Dossier ouvert : base + rabat superieur.
            return IconGeometry{{
                rectStroke(0.15F, 0.35F, 0.70F, 0.45F, false, IconColorRole::Foreground),
                IconStroke{
                    .points = {IconPoint{.x = 0.15F, .y = 0.35F}, IconPoint{.x = 0.25F, .y = 0.20F},
                               IconPoint{.x = 0.55F, .y = 0.20F},
                               IconPoint{.x = 0.62F, .y = 0.35F}},
                    .closed = true,
                    .filled = true,
                    .color = IconColorRole::Accent},
            }};
        case IconId::PixelCreate:
            // Feuille vierge + signe plus : "nouveau".
            return IconGeometry{{
                rectStroke(0.20F, 0.20F, 0.60F, 0.60F, false, IconColorRole::Foreground),
                IconStroke{
                    .points = {IconPoint{.x = 0.5F, .y = 0.32F}, IconPoint{.x = 0.5F, .y = 0.68F}},
                    .closed = false,
                    .filled = false,
                    .color = IconColorRole::Accent},
                IconStroke{
                    .points = {IconPoint{.x = 0.32F, .y = 0.5F}, IconPoint{.x = 0.68F, .y = 0.5F}},
                    .closed = false,
                    .filled = false,
                    .color = IconColorRole::Accent},
            }};
        case IconId::PixelSave:
            // Meme silhouette de disquette que Save (niveau) : le concept est identique, seule la
            // cible (fichier d'asset plutot que de niveau) differe -- deux barres d'outils
            // distinctes evitent toute confusion malgre l'icone partagee.
            return IconGeometry{{
                rectStroke(0.20F, 0.15F, 0.60F, 0.70F, false, IconColorRole::Foreground),
                rectStroke(0.35F, 0.15F, 0.30F, 0.20F, true, IconColorRole::Foreground),
                rectStroke(0.30F, 0.55F, 0.40F, 0.25F, false, IconColorRole::Accent),
            }};
        case IconId::PixelSaveAs:
            // Disquette reduite + pastille : "enregistrer sous" (une copie).
            return IconGeometry{{
                rectStroke(0.14F, 0.15F, 0.52F, 0.70F, false, IconColorRole::Foreground),
                rectStroke(0.24F, 0.15F, 0.26F, 0.18F, true, IconColorRole::Foreground),
                rectStroke(0.20F, 0.52F, 0.34F, 0.24F, false, IconColorRole::Foreground),
                IconStroke{.points = circlePoints(0.78F, 0.72F, 0.15F),
                           .closed = true,
                           .filled = true,
                           .color = IconColorRole::Accent},
            }};
        case IconId::PixelSelectionTool:
            // Meme motif que ToolSelection (quatre coins en L) : le concept de selection est
            // identique, seule la cible (canevas plutot que niveau) differe.
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
        case IconId::PixelFlipHorizontal:
            // Axe vertical + deux blocs miroir (un plein, un vide) de part et d'autre.
            return IconGeometry{{
                IconStroke{
                    .points = {IconPoint{.x = 0.5F, .y = 0.15F}, IconPoint{.x = 0.5F, .y = 0.85F}},
                    .closed = false,
                    .filled = false,
                    .color = IconColorRole::Foreground},
                rectStroke(0.15F, 0.30F, 0.25F, 0.40F, true, IconColorRole::Accent),
                rectStroke(0.60F, 0.30F, 0.25F, 0.40F, false, IconColorRole::Foreground),
            }};
        case IconId::PixelFlipVertical:
            // Axe horizontal + deux blocs miroir de part et d'autre.
            return IconGeometry{{
                IconStroke{
                    .points = {IconPoint{.x = 0.15F, .y = 0.5F}, IconPoint{.x = 0.85F, .y = 0.5F}},
                    .closed = false,
                    .filled = false,
                    .color = IconColorRole::Foreground},
                rectStroke(0.30F, 0.15F, 0.40F, 0.25F, true, IconColorRole::Accent),
                rectStroke(0.30F, 0.60F, 0.40F, 0.25F, false, IconColorRole::Foreground),
            }};
        case IconId::PixelRotateClockwise:
            // Anneau ouvert (cercle a douze points, trace non ferme) + fleche a l'ouverture.
            return IconGeometry{{
                IconStroke{.points = circlePoints(0.5F, 0.5F, 0.28F, 12),
                           .closed = false,
                           .filled = false,
                           .color = IconColorRole::Foreground},
                IconStroke{
                    .points = {IconPoint{.x = 0.76F, .y = 0.30F}, IconPoint{.x = 0.90F, .y = 0.40F},
                               IconPoint{.x = 0.74F, .y = 0.46F}},
                    .closed = true,
                    .filled = true,
                    .color = IconColorRole::Accent},
            }};
        case IconId::PixelRotateCounterClockwise:
            // Meme anneau, fleche du cote oppose : sens inverse.
            return IconGeometry{{
                IconStroke{.points = circlePoints(0.5F, 0.5F, 0.28F, 12),
                           .closed = false,
                           .filled = false,
                           .color = IconColorRole::Foreground},
                IconStroke{
                    .points = {IconPoint{.x = 0.24F, .y = 0.30F}, IconPoint{.x = 0.10F, .y = 0.40F},
                               IconPoint{.x = 0.26F, .y = 0.46F}},
                    .closed = true,
                    .filled = true,
                    .color = IconColorRole::Accent},
            }};
    }
    return IconGeometry{};
}

}  // namespace hmi
