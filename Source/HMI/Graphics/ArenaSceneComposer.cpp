// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Graphics/ArenaSceneComposer.h"

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include "Core/Combat/Arena.h"
#include "Core/Combat/IsoProjection.h"
#include "HMI/Graphics/ArenaAnimationState.h"
#include "HMI/Graphics/ArenaAppearanceCatalog.h"

namespace hmi {

namespace {

// Chemins de la planche, repris de ArenaTile.ui.qml.
constexpr std::string_view SAND = "terrain/sand.png";
constexpr std::string_view STONE = "terrain/stone.png";
constexpr std::string_view WALL = "structures/wall.png";
constexpr std::string_view COLUMN = "structures/column_large.png";
constexpr std::string_view BANNER = "structures/banner_01.png";
constexpr std::string_view TORCH = "structures/torch_01.png";
constexpr std::string_view ARCH = "structures/arch.png";

// Marges basses des pieces, en hauteurs de losange (anchors.bottomMargin de ArenaTile.ui.qml).
constexpr float WALL_BOTTOM_MARGIN = 0.12f;
constexpr float DECORATION_BOTTOM_MARGIN = 0.3f;
constexpr float GATE_BOTTOM_MARGIN = 0.1f;
constexpr float FIGURE_BOTTOM_MARGIN = 0.42f;

/// Ce qui ne change pas d'une piece a l'autre : la projection, les textures et le tampon.
struct Composer {
    ComposedScene& scene;
    const core::IsoProjection& projection;
    const ArenaSceneTextures& textures;
    /// Unites monde par pixel de planche : une tuile de 86 px occupe la largeur du losange.
    float unitsPerPixel;

    /// Une piece de planche a sa taille native, centree sur @p centerX, son bord bas a @p bottomY.
    void addStanding(RenderLayer layer, std::string_view path, float centerX, float bottomY,
                     std::int32_t sortOrder) const {
        const ArenaTexture& texture = textures.resolve(path);
        if (texture.texture == nullptr) {
            return;
        }
        const float width = static_cast<float>(texture.width) * unitsPerPixel;
        const float height = static_cast<float>(texture.height) * unitsPerPixel;
        SpriteQuad quad;
        quad.x = centerX - width / 2.0f;
        quad.y = bottomY - height;
        quad.width = width;
        quad.height = height;
        scene.addSprite(layer, texture.texture, sortOrder, quad);
    }
};

[[nodiscard]] core::Vector2 gridPoint(int column, int row) {
    return {static_cast<float>(column), static_cast<float>(row)};
}

void composeTile(const Composer& composer, const ArenaAppearanceCatalog& catalog,
                 const core::BattleGrid& grid, core::GridPosition cell, std::string& path) {
    const ArenaTileAppearance appearance = catalog.tileAppearance(
        cell, grid.width(), grid.height(), grid.isObstructed(cell, core::Locomotion::Walk));
    const core::Rect bounds = composer.projection.tileBounds(cell);
    const float tileHeight = bounds.size.y;
    const float centerX = bounds.position.x + bounds.size.x / 2.0f;
    const float bottomY = bounds.position.y + tileHeight;

    // --- Le sol : etire sur la boite du losange, comme l'Image en anchors.fill ---------------
    if (appearance.slab) {
        path.assign("coliseum/");
        path.append(catalog.paleSlabs()[static_cast<std::size_t>(appearance.slabVariant)]);
        path.append(".png");
    } else {
        path.assign(appearance.wall ? STONE : SAND);
    }
    if (const ArenaTexture& floor = composer.textures.resolve(path); floor.texture != nullptr) {
        SpriteQuad quad;
        quad.x = bounds.position.x;
        quad.y = bounds.position.y;
        quad.width = bounds.size.x;
        quad.height = tileHeight;
        composer.scene.addSprite(RenderLayer::Tile, floor.texture, core::IsoProjection::depth(cell),
                                 quad);
    }

    // --- L'enceinte : des pieces debout, triees au pied de la case --------------------------
    switch (appearance.wallFeature) {
        case WallFeature::None:
            break;
        case WallFeature::Corner:
            composer.addStanding(RenderLayer::Object, COLUMN, centerX,
                                 bottomY - tileHeight * WALL_BOTTOM_MARGIN,
                                 arenaDepthSortOrder(bottomY, ArenaDepthSlot::Wall));
            break;
        case WallFeature::Plain:
        case WallFeature::BannerSpot:
        case WallFeature::TorchSpot:
            composer.addStanding(RenderLayer::Object, WALL, centerX,
                                 bottomY - tileHeight * WALL_BOTTOM_MARGIN,
                                 arenaDepthSortOrder(bottomY, ArenaDepthSlot::Wall));
            break;
    }
    if (appearance.wallFeature == WallFeature::BannerSpot ||
        appearance.wallFeature == WallFeature::TorchSpot) {
        composer.addStanding(RenderLayer::Object,
                             appearance.wallFeature == WallFeature::BannerSpot ? BANNER : TORCH,
                             centerX, bottomY - tileHeight * DECORATION_BOTTOM_MARGIN,
                             arenaDepthSortOrder(bottomY, ArenaDepthSlot::WallDecoration));
    }
    if (appearance.gateSpot) {
        composer.addStanding(RenderLayer::Object, ARCH, centerX,
                             bottomY - tileHeight * GATE_BOTTOM_MARGIN,
                             arenaDepthSortOrder(bottomY, ArenaDepthSlot::Gate));
    }
}

void composeFigure(const Composer& composer, const ArenaAppearanceCatalog& catalog,
                   const ArenaAnimationState& animation, const core::CombatState& combat,
                   const core::Combatant& combatant, std::string& path) {
    // Sorti : il a quitte la grille et l'ordre, il n'a plus de figurine.
    if (combatant.status == core::CombatantStatus::Withdrawn) {
        return;
    }
    const core::BattleGrid& grid = combat.grid();
    const std::optional<core::GridPosition> anchor = grid.positionOf(combatant.id);
    if (!anchor.has_value()) {
        return;
    }
    const core::CombatSide side = combatant.profile.side;
    const FigureAppearance figure = catalog.figureFor(combatant.profile.name, side);
    if (figure.sheet.empty() || figure.frameCount <= 0) {
        return;
    }
    const bool down = combatant.status == core::CombatantStatus::Down;
    const bool ally = side == core::CombatSide::Allies;

    path.assign(ally ? "characters/" : "enemies/");
    path.append(figure.sheet);
    path.append(down && ally ? "/death.png" : "/idle.png");
    const ArenaTexture& texture = composer.textures.resolve(path);
    if (texture.texture == nullptr) {
        return;
    }

    // A terre, la figurine s'arrete sur la derniere image ; debout, elle suit l'animation, ramenee
    // dans sa bande.
    const int frame = down ? figure.frameCount - 1
                           : std::clamp(animation.frameOf(combatant.id), 0, figure.frameCount - 1);

    // Une emprise de n cases : une figurine centree dessus, n fois plus grande, au pied de
    // l'emprise.
    const int footprint = std::max(1, grid.sideOf(combatant.id));
    const float extent = static_cast<float>(footprint);
    const core::Vector2 center =
        composer.projection.gridToWorld({static_cast<float>(anchor->column) + extent / 2.0f,
                                         static_cast<float>(anchor->row) + extent / 2.0f});
    const float footY =
        composer.projection
            .gridToWorld(gridPoint(anchor->column + footprint, anchor->row + footprint))
            .y;
    const float tileHeight = composer.projection.tileHeight();

    const float scale = composer.unitsPerPixel * ARENA_FIGURE_SCALE * extent;
    SpriteQuad quad;
    quad.width = static_cast<float>(ARENA_FIGURE_FRAME_WIDTH_PIXELS) * scale;
    quad.height = static_cast<float>(ARENA_FIGURE_FRAME_HEIGHT_PIXELS) * scale;
    quad.x = center.x - quad.width / 2.0f;
    quad.y = footY - tileHeight * FIGURE_BOTTOM_MARGIN * extent - quad.height;
    if (texture.width > 0 && texture.height > 0) {
        const float frameWidth = static_cast<float>(ARENA_FIGURE_FRAME_WIDTH_PIXELS);
        quad.u0 = static_cast<float>(frame) * frameWidth / static_cast<float>(texture.width);
        quad.u1 = static_cast<float>(frame + 1) * frameWidth / static_cast<float>(texture.width);
        quad.v0 = 0.0f;
        quad.v1 = std::min(1.0f, static_cast<float>(ARENA_FIGURE_FRAME_HEIGHT_PIXELS) /
                                     static_cast<float>(texture.height));
    }
    if (down && !ally) {
        quad.a = ARENA_DOWN_ENEMY_ALPHA;
    }
    composer.scene.addSprite(RenderLayer::Player, texture.texture,
                             arenaDepthSortOrder(footY, ArenaDepthSlot::Figure), quad);
}

}  // namespace

std::int32_t arenaDepthSortOrder(float footWorldY, ArenaDepthSlot slot) noexcept {
    return depthSortOrder(footWorldY) * ARENA_DEPTH_SLOTS + static_cast<std::int32_t>(slot);
}

void composeArenaScene(ComposedScene& scene, const core::ArenaSession& session,
                       const ArenaAppearanceCatalog& catalog, const ArenaAnimationState& animation,
                       const core::IsoProjection& projection, const ArenaSceneTextures& textures) {
    const Composer composer{
        .scene = scene,
        .projection = projection,
        .textures = textures,
        .unitsPerPixel = projection.tileWidth() / core::ARENA_SHEET_TILE_WIDTH_PIXELS};
    // Un seul tampon de chemin pour toute la scene : apres la premiere image, plus d'allocation.
    std::string path;

    const core::CombatState& combat = session.combat();
    const core::BattleGrid& grid = combat.grid();
    for (int row = 0; row < grid.height(); ++row) {
        for (int column = 0; column < grid.width(); ++column) {
            composeTile(composer, catalog, grid, {.column = column, .row = row}, path);
        }
    }
    for (const core::CombatantId id : combat.combatants()) {
        if (const core::Combatant* combatant = combat.find(id)) {
            composeFigure(composer, catalog, animation, combat, *combatant, path);
        }
    }
}

ComposedScene composeArenaScene(const core::ArenaSession& session,
                                const ArenaAppearanceCatalog& catalog,
                                const ArenaAnimationState& animation,
                                const core::IsoProjection& projection,
                                const ArenaSceneTextures& textures) {
    ComposedScene scene;
    composeArenaScene(scene, session, catalog, animation, projection, textures);
    scene.sort();
    return scene;
}

}  // namespace hmi
