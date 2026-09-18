// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/DraftRenderer.h"

#include <algorithm>
#include <tuple>

#include "Core/Ecs/Components/Sprite.h"  // core::AtlasRegion
#include "Core/Levels/LevelDraft.h"
#include "Core/Levels/TileLayer.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/EntityMarkers.h"
#include "HMI/Graphics/RenderLayer.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/Graphics/SpriteRenderer.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TextureCache.h"
#include "HMI/Graphics/TileVisuals.h"

namespace hmi {

namespace {
// Ordre de dessin des aides d'edition **a l'interieur** du calque EditorOverlay : le masque de
// collision et la grille sous le reste, l'apercu de selection au-dessus, les entites et leur cadre
// de selection en tete.
constexpr std::int32_t OVERLAY_ORDER_COLLISION_MASK = 0;
constexpr std::int32_t OVERLAY_ORDER_GRID = 1;
constexpr std::int32_t OVERLAY_ORDER_HIGHLIGHT = 2;
constexpr std::int32_t OVERLAY_ORDER_TERRAIN = 3;
constexpr std::int32_t OVERLAY_ORDER_ENTITIES = 4;
// Cadre de selection double ton (sombre puis clair, dessine par-dessus) : lisible sur tout fond.
constexpr std::int32_t OVERLAY_ORDER_HANDLE_DARK = 5;
constexpr std::int32_t OVERLAY_ORDER_HANDLE_BRIGHT = 6;

/// @return Vrai si une couche de @p draft est visuelle : la collision n'est alors plus l'image.
[[nodiscard]] bool hasVisualLayers(const core::LevelDraft& draft) {
    return std::ranges::any_of(draft.layers(), [](const core::TileLayer& layer) {
        return core::isVisualLayerKind(layer.kind);
    });
}
}  // namespace

DraftRenderer::DraftRenderer(SpriteBatch& batch, const TextureAtlas& atlas, TextureCache& cache)
    : _batch(batch), _atlas(atlas), _cache(cache) {}

void DraftRenderer::render(
    const core::LevelDraft& draft, const Camera2D& camera, bool showGrid,
    const std::optional<std::pair<core::GridPosition, core::GridPosition>>& highlight,
    const DraftEntityOverlay& entityOverlay) {
    _scene.clear();
    _scene.setVisibleBounds(camera.visibleBounds());

    if (!hasVisualLayers(draft)) {
        // Grille unique : image et collision a la fois.
        const LayerDisplay display = _layerView.display(std::nullopt, /*hasVisualLayers=*/false);
        if (display.visible) {
            composeTiles(draft.tileMap(), RenderLayer::Tile, 0, display.opacity);
        }
    } else {
        // Carte a couches : les couches visuelles, dans leur ordre ; la collision est un masque.
        std::int32_t order = 0;
        const std::vector<core::TileLayer>& layers = draft.layers();
        for (std::size_t index = 0; index < layers.size(); ++index) {
            const core::TileLayer& layer = layers[index];
            if (!core::isVisualLayerKind(layer.kind)) {
                continue;
            }
            const LayerDisplay display = _layerView.display(index, /*hasVisualLayers=*/true);
            if (display.visible) {
                composeTiles(layer.tiles,
                             layer.kind == core::LayerKind::Decor ? RenderLayer::Object
                                                                  : RenderLayer::Tile,
                             order, display.opacity);
            }
            ++order;
        }
        composeCollisionMask(draft);
    }
    if (showGrid) {
        composeGrid(draft);
    }
    if (highlight) {
        composeHighlight(highlight->first, highlight->second);
    }
    composeEntities(draft, entityOverlay);
    _scene.sort();
    submitComposedScene(_batch, camera.projectionMatrix(), _scene);
}

void DraftRenderer::composeTiles(const core::TileMap& tiles, RenderLayer layer,
                                 std::int32_t order, float opacity) {
    const auto atlasWidth = static_cast<float>(_atlas.width());
    const auto atlasHeight = static_cast<float>(_atlas.height());
    for (int row = 0; row < tiles.height(); ++row) {
        for (int column = 0; column < tiles.width(); ++column) {
            const core::TileType type = tiles.tile(column, row);
            if (type == core::TileType::Empty) {
                continue;
            }
            const core::AtlasRegion region = regionForTile(type);
            SpriteQuad quad;
            quad.x = static_cast<float>(column);
            quad.y = static_cast<float>(row);
            quad.width = 1.0F;
            quad.height = 1.0F;
            quad.u0 = static_cast<float>(region.x) / atlasWidth;
            quad.v0 = static_cast<float>(region.y) / atlasHeight;
            quad.u1 = static_cast<float>(region.x + region.width) / atlasWidth;
            quad.v1 = static_cast<float>(region.y + region.height) / atlasHeight;
            quad.a = opacity;
            _scene.addSprite(layer, _atlas.textureHandle(), order, quad);
        }
    }
}

// Compose le voile d'apercu d'une zone (outil Rectangle/Selection) sur le calque d'edition.
void DraftRenderer::composeHighlight(const core::GridPosition& minimum,
                                     const core::GridPosition& maximum) {
    const auto atlasWidth = static_cast<float>(_atlas.width());
    const auto atlasHeight = static_cast<float>(_atlas.height());
    const core::AtlasRegion solid = hmi::TextureAtlas::tile(0, 0);
    SpriteQuad quad;
    quad.x = static_cast<float>(minimum.column);
    quad.y = static_cast<float>(minimum.row);
    quad.width = static_cast<float>(maximum.column - minimum.column + 1);
    quad.height = static_cast<float>(maximum.row - minimum.row + 1);
    quad.u0 = static_cast<float>(solid.x) / atlasWidth;
    quad.v0 = static_cast<float>(solid.y) / atlasHeight;
    quad.u1 = static_cast<float>(solid.x + solid.width) / atlasWidth;
    quad.v1 = static_cast<float>(solid.y + solid.height) / atlasHeight;
    quad.r = 0.3F;
    quad.g = 0.7F;
    quad.b = 1.0F;
    quad.a = 0.28F;  // voile bleu semi-transparent (apercu rectangle/selection)
    _scene.addSprite(RenderLayer::EditorOverlay, _atlas.textureHandle(), OVERLAY_ORDER_HIGHLIGHT,
                     quad);
}

// Compose la grille de repere sur le calque d'edition.
void DraftRenderer::composeGrid(const core::LevelDraft& draft) {
    const int width = draft.tileMap().width();
    const int height = draft.tileMap().height();
    const core::AtlasRegion solid =
        hmi::TextureAtlas::tile(0, 0);  // region opaque unie (teintee pour la ligne)
    const auto atlasWidth = static_cast<float>(_atlas.width());
    const auto atlasHeight = static_cast<float>(_atlas.height());

    // Fabrique un quad plein (UV de la region opaque) a une position/taille et teinte donnees.
    const auto lineQuad = [&](float x, float y, float w, float h, float r, float g, float b,
                              float a) {
        SpriteQuad quad;
        quad.x = x;
        quad.y = y;
        quad.width = w;
        quad.height = h;
        quad.u0 = static_cast<float>(solid.x) / atlasWidth;
        quad.v0 = static_cast<float>(solid.y) / atlasHeight;
        quad.u1 = static_cast<float>(solid.x + solid.width) / atlasWidth;
        quad.v1 = static_cast<float>(solid.y + solid.height) / atlasHeight;
        quad.r = r;
        quad.g = g;
        quad.b = b;
        quad.a = a;
        return quad;
    };
    const auto add = [&](const SpriteQuad& quad) {
        _scene.addSprite(RenderLayer::EditorOverlay, _atlas.textureHandle(), OVERLAY_ORDER_GRID,
                         quad);
    };

    // Grille de cases : lignes fines, faible alpha (repere de placement, EX-EDIT-023).
    constexpr float LINE = 0.035F;  // epaisseur en unites monde (fraction de case)
    constexpr float LINE_ALPHA = 0.18F;
    const auto w = static_cast<float>(width);
    const auto h = static_cast<float>(height);
    for (int column = 0; column <= width; ++column) {
        add(lineQuad(static_cast<float>(column) - (LINE * 0.5F), 0.0F, LINE, h, 1.0F, 1.0F, 1.0F,
                     LINE_ALPHA));
    }
    for (int row = 0; row <= height; ++row) {
        add(lineQuad(0.0F, static_cast<float>(row) - (LINE * 0.5F), w, LINE, 1.0F, 1.0F, 1.0F,
                     LINE_ALPHA));
    }
}

void DraftRenderer::setLayerView(const LayerViewState& view) {
    _layerView = view;
}

void DraftRenderer::addOverlayRect(float x, float y, float width, float height, float r, float g,
                                   float b, float a, std::int32_t order) {
    const auto atlasWidth = static_cast<float>(_atlas.width());
    const auto atlasHeight = static_cast<float>(_atlas.height());
    const core::AtlasRegion solid = hmi::TextureAtlas::tile(0, 0);
    SpriteQuad quad;
    quad.x = x;
    quad.y = y;
    quad.width = width;
    quad.height = height;
    quad.u0 = static_cast<float>(solid.x) / atlasWidth;
    quad.v0 = static_cast<float>(solid.y) / atlasHeight;
    quad.u1 = static_cast<float>(solid.x + solid.width) / atlasWidth;
    quad.v1 = static_cast<float>(solid.y + solid.height) / atlasHeight;
    quad.r = r;
    quad.g = g;
    quad.b = b;
    quad.a = a;
    _scene.addSprite(RenderLayer::EditorOverlay, _atlas.textureHandle(), order, quad);
}

void DraftRenderer::composeCollisionMask(const core::LevelDraft& draft) {
    const LayerDisplay display = _layerView.display(std::nullopt, /*hasVisualLayers=*/true);
    if (!display.visible || display.opacity <= 0.0F) {
        return;
    }
    const core::TileMap& map = draft.tileMap();
    for (int row = 0; row < map.height(); ++row) {
        for (int column = 0; column < map.width(); ++column) {
            const core::TileType type = map.tile(column, row);
            // Une teinte par CATEGORIE de regle, pas par type : l'auteur lit ou l'on bute, ou
            // l'on entre.
            float r = 0.0F;
            float g = 0.0F;
            float b = 0.0F;
            if (core::isSolid(type)) {
                r = 0.85F, g = 0.20F, b = 0.20F;  // obstacle
            } else if (type == core::TileType::Entry) {
                r = 0.20F, g = 0.85F, b = 0.30F;  // entree
            } else {
                continue;  // terrain franchissable : rien a masquer.
            }
            addOverlayRect(static_cast<float>(column), static_cast<float>(row), 1.0F, 1.0F, r, g, b,
                           display.opacity * 0.6F, OVERLAY_ORDER_COLLISION_MASK);
        }
    }
}

void DraftRenderer::composeEntities(const core::LevelDraft& draft,
                                    const DraftEntityOverlay& overlay) {
    const std::vector<core::MapEntity>& entities = draft.entities();

    // Terrain de la rencontre selectionnee : la zone ou l'on se bat, puis la case voulue de chaque
    // combattant -- verte si elle tient, rouge si le montage la refuserait.
    if (overlay.showTerrain && overlay.selectedEntity && overlay.terrains != nullptr) {
        for (const core::EncounterTerrain& terrain : *overlay.terrains) {
            if (terrain.entityIndex == *overlay.selectedEntity) {
                composeEncounterTerrain(terrain);
            }
        }
    }

    for (std::size_t index = 0; index < entities.size(); ++index) {
        composeEntityMarker(entities[index], overlay.selectedEntity == index);
    }
}

void DraftRenderer::composeEncounterTerrain(const core::EncounterTerrain& terrain) {
    const bool narrow = std::ranges::any_of(terrain.issues, [](const core::TacticalIssue& issue) {
        return issue.code == core::TacticalIssueCode::AreaTooNarrow;
    });
    for (const core::GridPosition& cell : terrain.area) {
        addOverlayRect(static_cast<float>(cell.column), static_cast<float>(cell.row), 1.0F, 1.0F,
                       narrow ? 1.0F : 0.30F, narrow ? 0.55F : 0.70F, narrow ? 0.10F : 1.00F, 0.18F,
                       OVERLAY_ORDER_TERRAIN);
    }
    for (const core::CombatantPlacement& placement : terrain.placements) {
        const bool refused =
            std::ranges::any_of(terrain.issues, [&placement](const core::TacticalIssue& issue) {
                return issue.code != core::TacticalIssueCode::AreaTooNarrow &&
                       issue.cell == placement.position;
            });
        constexpr float INSET = 0.2F;
        addOverlayRect(static_cast<float>(placement.position.column) + INSET,
                       static_cast<float>(placement.position.row) + INSET, 1.0F - (2 * INSET),
                       1.0F - (2 * INSET), refused ? 0.95F : 0.25F, refused ? 0.20F : 0.85F,
                       refused ? 0.20F : 0.35F, 0.55F, OVERLAY_ORDER_TERRAIN);
    }
}

void DraftRenderer::composeEntityMarker(const core::MapEntity& entity, bool selected) {
    constexpr float MARKER_INSET = 0.12F;
    const auto x = static_cast<float>(entity.position.column);
    const auto y = static_cast<float>(entity.position.row);
    // Marqueur genere de la famille (LOT-39) : aucune illustration n'est requise pour poser un
    // PNJ ou un portail, et deux familles ne se confondent pas.
    if (const LoadedTexture* const marker = _cache.markerTexture(entityMarkerKey(entity.type))) {
        SpriteQuad quad;
        quad.x = x + MARKER_INSET;
        quad.y = y + MARKER_INSET;
        quad.width = 1.0F - (2 * MARKER_INSET);
        quad.height = 1.0F - (2 * MARKER_INSET);
        quad.u0 = 0.0F;
        quad.v0 = 0.0F;
        quad.u1 = 1.0F;
        quad.v1 = 1.0F;
        quad.r = 1.0F;
        quad.g = 1.0F;
        quad.b = 1.0F;
        quad.a = 1.0F;
        _scene.addSprite(RenderLayer::EditorOverlay, marker->handle(), OVERLAY_ORDER_ENTITIES,
                         quad);
    } else {
        addOverlayRect(x + MARKER_INSET, y + MARKER_INSET, 1.0F - (2 * MARKER_INSET),
                       1.0F - (2 * MARKER_INSET), 1.0F, 0.0F, 1.0F, 0.8F, OVERLAY_ORDER_ENTITIES);
    }
    if (!selected) {
        return;
    }
    // Cadre double ton : sombre dessous, clair dessus, lisible sur tout fond.
    constexpr float THICK = 0.08F;
    for (const auto& [order, r, g, b, grow] :
         {std::tuple{OVERLAY_ORDER_HANDLE_DARK, 0.05F, 0.05F, 0.05F, THICK},
          std::tuple{OVERLAY_ORDER_HANDLE_BRIGHT, 1.0F, 0.95F, 0.35F, 0.0F}}) {
        const float left = x - grow;
        const float top = y - grow;
        const float size = 1.0F + (2 * grow);
        addOverlayRect(left, top, size, THICK, r, g, b, 1.0F, order);
        addOverlayRect(left, top + size - THICK, size, THICK, r, g, b, 1.0F, order);
        addOverlayRect(left, top, THICK, size, r, g, b, 1.0F, order);
        addOverlayRect(left + size - THICK, top, THICK, size, r, g, b, 1.0F, order);
    }
}

}  // namespace hmi
