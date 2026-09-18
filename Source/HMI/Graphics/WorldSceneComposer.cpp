// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/WorldSceneComposer.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <set>
#include <utility>
#include <variant>

#include "Core/Combat/IsoProjection.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/TileLayer.h"
#include "Core/Levels/TileMap.h"
#include "HMI/Graphics/PlaceAppearance.h"

namespace hmi {

namespace {

// Les planches de l'atelier, telles que le rendu les adresse : relatif au dossier des assets.
constexpr std::string_view SCENE_ROOT = "Scene/";
constexpr std::string_view FIGURE_ROOT = "Npc/";
// Les dossiers de figurines qu'un marqueur peut remplacer : les PNJ (LOT-91), les monstres
// (LOT-93).
constexpr std::array<std::string_view, 2> FIGURE_DIRECTORIES = {"Npc/", "Monsters/"};

[[nodiscard]] std::size_t indexOf(core::GridPosition cell, int columns) {
    return (static_cast<std::size_t>(cell.row) * static_cast<std::size_t>(columns)) +
           static_cast<std::size_t>(cell.column);
}

[[nodiscard]] bool inGrid(core::GridPosition cell, int columns, int rows) {
    return cell.column >= 0 && cell.row >= 0 && cell.column < columns && cell.row < rows;
}

[[nodiscard]] std::string piecePath(std::string_view place, std::string_view piece) {
    std::string path{SCENE_ROOT};
    path.append(place);
    path.push_back('/');
    path.append(piece);
    path.append(".png");
    return path;
}

// La couche dont le sol se lit : la premiere couche de sol, a defaut la grille racine (une carte
// sans couche visuelle reste jouable -- `EX-NFR-040`).
[[nodiscard]] const core::TileMap& groundOf(const core::Level& level) {
    for (const core::TileLayer& couche : level.layers()) {
        if (couche.kind == core::LayerKind::Ground) {
            return couche.tiles;
        }
    }
    return level.tileMap();
}

/// @return La couche de decor, ou `nullptr` si la carte n'en declare pas.
[[nodiscard]] const core::TileMap* decorOf(const core::Level& level) {
    for (const core::TileLayer& couche : level.layers()) {
        if (couche.kind == core::LayerKind::Decor) {
            return &couche.tiles;
        }
    }
    return nullptr;
}

[[nodiscard]] core::Vector2 gridPoint(float column, float row) {
    return {column, row};
}

void composeFloor(ComposedScene& scene, const WorldSceneSnapshot& snapshot,
                  const core::IsoProjection& projection, const ScenePieceTextures& textures,
                  core::GridPosition cell) {
    const std::string_view piece = snapshot.floorAt(cell);
    if (piece.empty()) {
        return;
    }
    const SceneTexture& texture = textures.resolve(piecePath(snapshot.place, piece));
    if (texture.texture == nullptr) {
        return;
    }
    // Le sol est etire sur la boite du losange : c'est la pièce du lieu, dessinee a sa taille.
    const core::Rect bounds = projection.tileBounds(cell);
    SpriteQuad quad;
    quad.x = bounds.position.x;
    quad.y = bounds.position.y;
    quad.width = bounds.size.x;
    quad.height = bounds.size.y;
    scene.addSprite(RenderLayer::Tile, texture.texture, core::IsoProjection::depth(cell), quad);
}

void composeRelief(ComposedScene& scene, const WorldSceneSnapshot& snapshot,
                   const core::IsoProjection& projection, const ScenePieceTextures& textures,
                   core::GridPosition cell, float unitsPerScenePixel) {
    const std::string_view piece = snapshot.reliefAt(cell);
    if (piece.empty()) {
        return;
    }
    const SceneTexture& texture = textures.resolve(piecePath(snapshot.place, piece));
    if (texture.texture == nullptr) {
        return;
    }
    // Posee par son ancre, le sommet haut du losange de sa case ; triee au pied de la case, pour
    // qu'une piece plus haute que sa case reste derriere ce qui est devant elle.
    const core::Vector2 topVertex = projection.gridToWorld(
        gridPoint(static_cast<float>(cell.column), static_cast<float>(cell.row)));
    const float footY = projection
                            .gridToWorld(gridPoint(static_cast<float>(cell.column) + 1.0F,
                                                   static_cast<float>(cell.row) + 1.0F))
                            .y;
    const SpriteQuad quad = standingPieceQuad(texture, topVertex, unitsPerScenePixel);
    scene.addSprite(RenderLayer::Object, texture.texture,
                    worldDepthSortOrder(footY, WorldDepthSlot::Relief), quad);
}

void composeFigure(ComposedScene& scene, const core::IsoProjection& projection,
                   const ScenePieceTextures& textures, const WorldFigureSnapshot& figure,
                   float unitsPerPixel) {
    if (figure.figure.empty()) {
        return;
    }
    const SceneTexture& texture = textures.resolve(figureStripPath(figure.figure, figure.clip));
    if (texture.texture == nullptr) {
        return;
    }
    // La bande dit sa propre decoupe : sa largeur d'image, et avec sa largeur totale son nombre
    // d'images. Une image hors bande est ramenee dedans plutot que de lire a cote de la texture.
    const int frameWidthPixels =
        texture.frameWidth > 0 ? texture.frameWidth : FIGURE_FRAME_WIDTH_PIXELS;
    const int frameCount = texture.width > 0 ? std::max(1, texture.width / frameWidthPixels) : 1;
    const int frame = ((figure.frame % frameCount) + frameCount) % frameCount;

    const core::Vector2 center = projection.gridToWorld(figure.point);
    const float footY =
        projection.gridToWorld(gridPoint(figure.point.x + 0.5F, figure.point.y + 0.5F)).y;
    const float scale = unitsPerPixel * FIGURE_SCALE;

    SpriteQuad quad;
    quad.width = static_cast<float>(frameWidthPixels) * scale;
    quad.height = static_cast<float>(FIGURE_FRAME_HEIGHT_PIXELS) * scale;
    quad.x = center.x - (quad.width / 2.0F);
    quad.y = footY - (projection.tileHeight() * WORLD_FIGURE_BOTTOM_MARGIN) - quad.height;
    if (texture.width > 0 && texture.height > 0) {
        const auto frameWidth = static_cast<float>(frameWidthPixels);
        quad.u0 = static_cast<float>(frame) * frameWidth / static_cast<float>(texture.width);
        quad.u1 = static_cast<float>(frame + 1) * frameWidth / static_cast<float>(texture.width);
        quad.v0 = 0.0F;
        quad.v1 = std::min(1.0F, static_cast<float>(FIGURE_FRAME_HEIGHT_PIXELS) /
                                     static_cast<float>(texture.height));
    }
    scene.addSprite(RenderLayer::Player, texture.texture,
                    worldDepthSortOrder(footY, WorldDepthSlot::Figure), quad);
}

}  // namespace

std::int32_t worldDepthSortOrder(float footWorldY, WorldDepthSlot slot) noexcept {
    return (depthSortOrder(footWorldY) * WORLD_DEPTH_SLOTS) + static_cast<std::int32_t>(slot);
}

std::string_view WorldSceneSnapshot::floorAt(core::GridPosition cell) const {
    if (!inGrid(cell, columns, rows)) {
        return {};
    }
    const std::size_t index = indexOf(cell, columns);
    return index < floors.size() ? std::string_view{floors[index]} : std::string_view{};
}

std::string_view WorldSceneSnapshot::reliefAt(core::GridPosition cell) const {
    if (!inGrid(cell, columns, rows)) {
        return {};
    }
    const std::size_t index = indexOf(cell, columns);
    return index < relief.size() ? std::string_view{relief[index]} : std::string_view{};
}

std::string scenePlaceOf(const core::Level& level) {
    for (const core::TileLayer& couche : level.layers()) {
        const auto trouvee = couche.properties.find(std::string{SCENE_PLACE_PROPERTY});
        if (trouvee == couche.properties.end()) {
            continue;
        }
        if (const std::string* nom = std::get_if<std::string>(&trouvee->second);
            nom != nullptr && !nom->empty()) {
            return *nom;
        }
    }
    return {};
}

WorldSceneSnapshot snapshotWorldScene(const core::Level& level, const PlaceAppearance& appearance,
                                      std::vector<WorldFigureSnapshot> figures) {
    const core::TileMap& sol = groundOf(level);
    const core::TileMap* decor = decorOf(level);

    WorldSceneSnapshot snapshot;
    snapshot.columns = std::max(0, sol.width());
    snapshot.rows = std::max(0, sol.height());
    snapshot.place = scenePlaceOf(level);
    if (snapshot.place.empty()) {
        snapshot.place = appearance.place();
    }
    snapshot.figures = std::move(figures);

    const auto cases =
        static_cast<std::size_t>(snapshot.columns) * static_cast<std::size_t>(snapshot.rows);
    snapshot.floors.assign(cases, std::string{});
    snapshot.relief.assign(cases, std::string{});

    for (int row = 0; row < snapshot.rows; ++row) {
        for (int column = 0; column < snapshot.columns; ++column) {
            const core::GridPosition cell{.column = column, .row = row};
            const std::size_t index = indexOf(cell, snapshot.columns);
            snapshot.floors[index] = appearance.floorPiece(sol.tile(column, row), cell);
            if (decor != nullptr && decor->inBounds(column, row)) {
                snapshot.relief[index] = appearance.reliefPiece(decor->tile(column, row), cell);
            }
        }
    }

    // L'assignation de texture a la case l'emporte sur la table du lieu : c'est la ou l'auteur a
    // decide (LOT-11, `EX-EDIT-043`). Elle nomme une piece de la planche, pas un fichier.
    for (const core::TileTextureOverride& assignee : level.textureOverrides()) {
        if (!inGrid(assignee.position, snapshot.columns, snapshot.rows)) {
            continue;
        }
        snapshot.relief[indexOf(assignee.position, snapshot.columns)] = assignee.assetName;
    }
    return snapshot;
}

std::string figureStripPath(std::string_view figure, std::string_view clip) {
    // Un nom sans barre est un PNJ de l'atelier ; avec, un dossier depuis la racine des assets.
    std::string path =
        figure.find('/') == std::string_view::npos ? std::string{FIGURE_ROOT} : std::string{};
    path.append(figure);
    path.push_back('/');
    path.append(clip.empty() ? std::string_view{"idle"} : clip);
    path.append(".png");
    return path;
}

std::string figureMarkerKey(std::string_view path) {
    for (const std::string_view dossier : FIGURE_DIRECTORIES) {
        if (!path.starts_with(dossier)) {
            continue;
        }
        const std::string_view reste = path.substr(dossier.size());
        const std::size_t barre = reste.find('/');
        if (barre == 0 || barre == std::string_view::npos) {
            return {};
        }
        std::string cle;
        for (const char lettre : dossier.substr(0, dossier.size() - 1)) {
            cle.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(lettre))));
        }
        cle.push_back('/');
        cle.append(reste.substr(0, barre));
        return cle;
    }
    return {};
}

std::vector<std::string> worldTexturePaths(const WorldSceneSnapshot& snapshot) {
    std::set<std::string> uniques;
    for (const std::vector<std::string>* couche : {&snapshot.floors, &snapshot.relief}) {
        for (const std::string& piece : *couche) {
            if (!piece.empty()) {
                uniques.insert(piecePath(snapshot.place, piece));
            }
        }
    }
    for (const WorldFigureSnapshot& figure : snapshot.figures) {
        if (figure.figure.empty()) {
            continue;
        }
        // Les deux bandes d'une figurine : elle marche et elle attend, et le rendu ne doit pas
        // charger une texture au milieu d'une image.
        uniques.insert(figureStripPath(figure.figure, "idle"));
        uniques.insert(figureStripPath(figure.figure, "walk"));
    }
    return {uniques.begin(), uniques.end()};
}

void composeWorldScene(ComposedScene& scene, const WorldSceneSnapshot& snapshot,
                       const core::IsoProjection& projection, const ScenePieceTextures& textures) {
    const float unitsPerPixel = projection.tileWidth() / core::ARENA_SHEET_TILE_WIDTH_PIXELS;
    const float unitsPerScenePixel =
        projection.tileWidth() / static_cast<float>(SCENE_TILE_WIDTH_PIXELS);

    for (int row = 0; row < snapshot.rows; ++row) {
        for (int column = 0; column < snapshot.columns; ++column) {
            const core::GridPosition cell{.column = column, .row = row};
            composeFloor(scene, snapshot, projection, textures, cell);
            composeRelief(scene, snapshot, projection, textures, cell, unitsPerScenePixel);
        }
    }
    for (const WorldFigureSnapshot& figure : snapshot.figures) {
        composeFigure(scene, projection, textures, figure, unitsPerPixel);
    }
}

ComposedScene composeWorldScene(const WorldSceneSnapshot& snapshot,
                                const core::IsoProjection& projection,
                                const ScenePieceTextures& textures) {
    ComposedScene scene;
    composeWorldScene(scene, snapshot, projection, textures);
    scene.sort();
    return scene;
}

}  // namespace hmi
