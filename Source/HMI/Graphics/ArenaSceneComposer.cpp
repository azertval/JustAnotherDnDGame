// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

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
                 const ArenaSceneSnapshot& snapshot, core::GridPosition cell, std::string& path) {
    const ArenaTileAppearance appearance =
        catalog.tileAppearance(cell, snapshot.columns, snapshot.rows, snapshot.isObstructed(cell));
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
                   const ArenaAnimationState& animation, const ArenaFigureSnapshot& combatant,
                   std::string& path) {
    const core::CombatSide side = combatant.side;
    const FigureAppearance figure = catalog.figureFor(combatant.name, side);
    if (figure.sheet.empty() || figure.frameCount <= 0) {
        return;
    }
    const bool down = combatant.down;
    const bool ally = side == core::CombatSide::Allies;

    path.assign(figure.directory);
    path.append(down && ally ? "/death.png" : "/idle.png");
    const ArenaTexture& texture = composer.textures.resolve(path);
    if (texture.texture == nullptr) {
        return;
    }

    // La bande dit sa propre decoupe : sa largeur d'image (48 par defaut, 96 pour les bandes
    // larges des PNJ de l'atelier) et, avec sa largeur totale, son nombre d'images. Le compte du
    // manifeste ne sert que si la texture est inconnue.
    const int frameWidthPixels =
        texture.frameWidth > 0 ? texture.frameWidth : ARENA_FIGURE_FRAME_WIDTH_PIXELS;
    const int frameCount = texture.width > 0 ? std::max(1, texture.width / frameWidthPixels)
                                             : std::max(1, figure.frameCount);

    // A terre, la figurine s'arrete sur la derniere image ; debout, elle suit l'animation, ramenee
    // dans sa bande.
    const int frame =
        down ? frameCount - 1 : std::clamp(animation.frameOf(combatant.id), 0, frameCount - 1);

    // Une emprise de n cases : une figurine centree dessus, n fois plus grande, au pied de
    // l'emprise.
    const core::GridPosition anchor = combatant.anchor;
    const int footprint = std::max(1, combatant.footprint);
    const float extent = static_cast<float>(footprint);
    const core::Vector2 center =
        composer.projection.gridToWorld({static_cast<float>(anchor.column) + extent / 2.0f,
                                         static_cast<float>(anchor.row) + extent / 2.0f});
    const float footY =
        composer.projection
            .gridToWorld(gridPoint(anchor.column + footprint, anchor.row + footprint))
            .y;
    const float tileHeight = composer.projection.tileHeight();

    // Une image plus large que 48 px garde son pied au centre de sa cellule : le quad s'elargit
    // autour du meme centre.
    const float scale = composer.unitsPerPixel * ARENA_FIGURE_SCALE * extent;
    SpriteQuad quad;
    quad.width = static_cast<float>(frameWidthPixels) * scale;
    quad.height = static_cast<float>(ARENA_FIGURE_FRAME_HEIGHT_PIXELS) * scale;
    quad.x = center.x - quad.width / 2.0f;
    quad.y = footY - tileHeight * FIGURE_BOTTOM_MARGIN * extent - quad.height;
    if (texture.width > 0 && texture.height > 0) {
        const float frameWidth = static_cast<float>(frameWidthPixels);
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

std::vector<std::string> arenaTexturePaths(const ArenaAppearanceCatalog& catalog) {
    std::vector<std::string> paths{std::string(SAND),   std::string(STONE),  std::string(WALL),
                                   std::string(COLUMN), std::string(BANNER), std::string(TORCH),
                                   std::string(ARCH)};
    for (const std::string& slab : catalog.paleSlabs()) {
        paths.push_back("coliseum/" + slab + ".png");
    }
    // Un allie a terre montre sa bande de mort ; un ennemi, sa bande de repos estompee.
    for (const std::string& hero : catalog.heroes()) {
        const std::string directory = catalog.sheetDirectory(hero, core::CombatSide::Allies);
        paths.push_back(directory + "/idle.png");
        paths.push_back(directory + "/death.png");
    }
    for (const std::string& gladiator : catalog.gladiators()) {
        paths.push_back(catalog.sheetDirectory(gladiator, core::CombatSide::Enemies) + "/idle.png");
    }
    return paths;
}

bool ArenaSceneSnapshot::isObstructed(core::GridPosition cell) const noexcept {
    if (cell.column < 0 || cell.row < 0 || cell.column >= columns || cell.row >= rows) {
        return false;
    }
    const std::size_t index =
        static_cast<std::size_t>(cell.row) * static_cast<std::size_t>(columns) +
        static_cast<std::size_t>(cell.column);
    return index < obstructed.size() && obstructed[index];
}

ArenaSceneSnapshot snapshotArenaScene(const core::ArenaSession& session) {
    const core::CombatState& combat = session.combat();
    const core::BattleGrid& grid = combat.grid();

    ArenaSceneSnapshot snapshot;
    snapshot.columns = std::max(0, grid.width());
    snapshot.rows = std::max(0, grid.height());
    snapshot.obstructed.reserve(static_cast<std::size_t>(snapshot.columns) *
                                static_cast<std::size_t>(snapshot.rows));
    for (int row = 0; row < snapshot.rows; ++row) {
        for (int column = 0; column < snapshot.columns; ++column) {
            snapshot.obstructed.push_back(
                grid.isObstructed({.column = column, .row = row}, core::Locomotion::Walk));
        }
    }

    for (const core::CombatantId id : combat.combatants()) {
        const core::Combatant* const combatant = combat.find(id);
        // Sorti : il a quitte la grille et l'ordre, il n'a plus de figurine.
        if (combatant == nullptr || combatant->status == core::CombatantStatus::Withdrawn) {
            continue;
        }
        const std::optional<core::GridPosition> anchor = grid.positionOf(id);
        if (!anchor.has_value()) {
            continue;
        }
        snapshot.figures.push_back(
            ArenaFigureSnapshot{.id = id,
                                .name = combatant->profile.name,
                                .side = combatant->profile.side,
                                .down = combatant->status == core::CombatantStatus::Down,
                                .anchor = *anchor,
                                .footprint = std::max(1, grid.sideOf(id))});
    }
    return snapshot;
}

void composeArenaScene(ComposedScene& scene, const ArenaSceneSnapshot& snapshot,
                       const ArenaAppearanceCatalog& catalog, const ArenaAnimationState& animation,
                       const core::IsoProjection& projection, const ArenaSceneTextures& textures) {
    const Composer composer{
        .scene = scene,
        .projection = projection,
        .textures = textures,
        .unitsPerPixel = projection.tileWidth() / core::ARENA_SHEET_TILE_WIDTH_PIXELS};
    // Un seul tampon de chemin pour toute la scene : apres la premiere image, plus d'allocation.
    std::string path;

    for (int row = 0; row < snapshot.rows; ++row) {
        for (int column = 0; column < snapshot.columns; ++column) {
            composeTile(composer, catalog, snapshot, {.column = column, .row = row}, path);
        }
    }
    for (const ArenaFigureSnapshot& figure : snapshot.figures) {
        composeFigure(composer, catalog, animation, figure, path);
    }
}

void composeArenaScene(ComposedScene& scene, const core::ArenaSession& session,
                       const ArenaAppearanceCatalog& catalog, const ArenaAnimationState& animation,
                       const core::IsoProjection& projection, const ArenaSceneTextures& textures) {
    composeArenaScene(scene, snapshotArenaScene(session), catalog, animation, projection, textures);
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
