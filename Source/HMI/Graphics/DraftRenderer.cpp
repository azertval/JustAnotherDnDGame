// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Graphics/DraftRenderer.h"

#include <algorithm>

#include "Core/Ecs/Components/Sprite.h"  // core::AtlasRegion, core::Color
#include "Core/Ecs/Components/Transform.h"
#include "Core/Levels/CameraFraming.h"
#include "Core/Levels/LevelDraft.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Rect.h"
#include "Core/Math/Vector2.h"
#include "HMI/Editor/LinkGeometry.h"
#include "HMI/Graphics/AssetContract.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/FollowCamera.h"
#include "HMI/Graphics/MissingTexture.h"
#include "HMI/Graphics/Parallax.h"
#include "HMI/Graphics/PlaneVisuals.h"
#include "HMI/Graphics/RoomGrid.h"
#include "HMI/Graphics/ShadowRenderer.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TextureCache.h"
#include "HMI/Graphics/TileAutotile.h"
#include "HMI/Graphics/TileSkinTag.h"
#include "HMI/Graphics/TileVisuals.h"

namespace hmi {

namespace {
// Ordre de dessin des aides d'edition **a l'interieur** du calque EditorOverlay : la grille sous
// les liens, les liens sous l'apercu de selection. Ces valeurs jouent le role de
// `core::Sprite::layer` pour des primitives qui ne viennent d'aucune entite.
constexpr std::int32_t OVERLAY_ORDER_GRID = 0;
// Previsualisation du cadrage de camera (LOT-64) : juste au-dessus de la grille de repere, sous
// tout le reste -- un grand rectangle qui ne doit jamais masquer un lien ou une poignee.
constexpr std::int32_t OVERLAY_ORDER_CAMERA_FRAMING = 1;
constexpr std::int32_t OVERLAY_ORDER_LINKS = 2;
constexpr std::int32_t OVERLAY_ORDER_HIGHLIGHT = 3;
constexpr std::int32_t OVERLAY_ORDER_TEXTURE_OVERRIDES = 4;
// Poignees de manipulation (LOT-67) : contour double couleur (sombre puis clair, dessine
// par-dessus) pour rester lisible sur tout fond (EX-EDIT-030). Valeurs heritees des poignees de
// poignees de manipulation du LOT-50, conservees telles quelles apres le retrait de celui-ci
// (LOT-69 TACHE-04) : seul leur rang relatif compte.
constexpr std::int32_t OVERLAY_ORDER_HANDLE_DARK = 7;
constexpr std::int32_t OVERLAY_ORDER_HANDLE_BRIGHT = 8;
// Parcours de plateforme mobile (LOT-63) : dernier calque d'édition -- un repère de placement,
// jamais masqué par une manipulation en cours.
constexpr std::int32_t OVERLAY_ORDER_PLATFORM_PATH = 9;
// Course d'un danger mobile (LOT-67) : juste au-dessus du parcours des plateformes, teinte
// distincte -- les deux peuvent se croiser sans qu'on confonde le sur quoi on marche et le qui
// tue.
constexpr std::int32_t OVERLAY_ORDER_MOVER_PATH = 10;
}  // namespace

DraftRenderer::DraftRenderer(SpriteBatch& batch, const TextureAtlas& atlas, TextureCache& cache)
    : _batch(batch), _atlas(atlas), _cache(cache) {}

void DraftRenderer::render(
    const core::LevelDraft& draft, const Camera2D& camera, bool showGrid,
    const std::optional<std::pair<core::GridPosition, core::GridPosition>>& highlight,
    const LinkOverlayState& linkOverlay, RenderMode mode, bool showTextureOverrides,
    float deltaSeconds, const LayerVisibility& visibility,
    const PlaneVisibility& planeVisibility) {
    if (_dirty) {
        rebuild(draft);
        _dirty = false;
    }

    // Apercu des tuiles animees (LOT-46 TACHE-05), en temps reel : meme mecanisme que
    // GameSession, mais sans exigence de determinisme (EX-NFR-002 ne s'applique qu'en jeu).
    advanceTileAnimations(_skins, _skinSet, _cache, deltaSeconds, _tileAnimations,
                          _warnedExcludedAnimations);

    // Une seule scene pour toute l'image : l'ordre visuel est porte par les calques, plus par
    // l'ordre des appels de dessin (LOT-40). L'ordre de composition ci-dessous reste celui d'avant
    // le lot -- a calque et texture egaux, le tri stable le preserve tel quel.
    _scene.clear();
    _scene.setVisibleBounds(camera.visibleBounds());
    // Calque Fond (LOT-51) : hors de composeWorldSprites (le fond ne vient pas de l'ECS), gate donc
    // ici l'appel entier plutot que de filtrer une primitive deja composee.
    if (visibility.visible(RenderLayer::Background)) {
        composeBackground(_scene, resolveBackgroundTexture(draft.background(), _cache),
                          draft.tileMap().width(), draft.tileMap().height(), mode);
    }
    // Plans picturaux (LOT-69 TACHE-05) : juste apres le fond et avant les tuiles, dans l'ordre
    // declare. Le mode d'inspection peut en masquer ou en isoler un (EX-DEC-045).
    composePlanes(_scene, draft.planes(),
                  resolvePlaneTextures(_cache, _planesDirectory, draft.planes()),
                  draft.tileMap().width(), draft.tileMap().height(), mode, planeVisibility);
    const SceneTextures textures =
        sceneTextures(_atlas, _cache, _skins, _skinSet, draft.textureOverrides(), _tileAnimations);
    // Calque Ombre (LOT-55) : meme raison de gate a l'appel que Fond ci-dessus -- aucune simulation
    // de mecanisme dans l'editeur (jamais de porte fermee/ouverte a suivre), donc pas de grille de
    // collision a transmettre.
    if (visibility.visible(RenderLayer::Shadow)) {
        composeShadows(_scene, _world, mode, textures, 1.0f);
    }
    composeWorldSprites(_scene, _world, mode, textures, 1.0f, visibility);
    if (showGrid) {
        composeGrid(draft);
    }
    composeCameraFraming(draft);
    composeLinks(draft, linkOverlay);
    if (showTextureOverrides) {
        composeTextureOverrideMarkers(draft);
    }
    if (highlight) {
        composeHighlight(highlight->first, highlight->second);
    }
    _scene.sort();
    submitComposedScene(_batch, camera.projectionMatrix(), _scene);
}

// Compose le voile d'apercu d'une zone (outil Rectangle/Selection) sur le calque d'edition.
void DraftRenderer::composeHighlight(const core::GridPosition& minimum,
                                     const core::GridPosition& maximum) {
    const float atlasWidth = static_cast<float>(_atlas.width());
    const float atlasHeight = static_cast<float>(_atlas.height());
    const core::AtlasRegion solid = _atlas.tile(0, 0);
    SpriteQuad quad;
    quad.x = static_cast<float>(minimum.column);
    quad.y = static_cast<float>(minimum.row);
    quad.width = static_cast<float>(maximum.column - minimum.column + 1);
    quad.height = static_cast<float>(maximum.row - minimum.row + 1);
    quad.u0 = static_cast<float>(solid.x) / atlasWidth;
    quad.v0 = static_cast<float>(solid.y) / atlasHeight;
    quad.u1 = static_cast<float>(solid.x + solid.width) / atlasWidth;
    quad.v1 = static_cast<float>(solid.y + solid.height) / atlasHeight;
    quad.r = 0.3f;
    quad.g = 0.7f;
    quad.b = 1.0f;
    quad.a = 0.28f;  // voile bleu semi-transparent (apercu rectangle/selection)
    _scene.addSprite(RenderLayer::EditorOverlay, _atlas.textureHandle(), OVERLAY_ORDER_HIGHLIGHT,
                     quad);
}

// Signale les cases portant une surcharge de texture par instance (EX-EDIT-043, LOT-45) : un petit
// marqueur au coin haut-droit de chaque case habillee, visible seulement quand l'outil dedie est
// actif (showTextureOverrides du render()).
void DraftRenderer::composeTextureOverrideMarkers(const core::LevelDraft& draft) {
    const float atlasWidth = static_cast<float>(_atlas.width());
    const float atlasHeight = static_cast<float>(_atlas.height());
    const core::AtlasRegion solid = _atlas.tile(0, 0);
    constexpr float MARKER_SIZE = 0.28f;  // fraction d'une case
    for (const core::TileTextureOverride& override : draft.textureOverrides()) {
        SpriteQuad quad;
        quad.x = static_cast<float>(override.position.column) + 1.0f - MARKER_SIZE;
        quad.y = static_cast<float>(override.position.row);
        quad.width = MARKER_SIZE;
        quad.height = MARKER_SIZE;
        quad.u0 = static_cast<float>(solid.x) / atlasWidth;
        quad.v0 = static_cast<float>(solid.y) / atlasHeight;
        quad.u1 = static_cast<float>(solid.x + solid.width) / atlasWidth;
        quad.v1 = static_cast<float>(solid.y + solid.height) / atlasHeight;
        quad.r = 1.0f;
        quad.g = 0.85f;
        quad.b = 0.1f;
        quad.a = 0.9f;  // jaune dore, oppose au bleu du voile de selection/rectangle
        _scene.addSprite(RenderLayer::EditorOverlay, _atlas.textureHandle(),
                         OVERLAY_ORDER_TEXTURE_OVERRIDES, quad);
    }
}

// Compose la grille de repere (frontieres de cases + de salles) sur le calque d'edition.
void DraftRenderer::composeGrid(const core::LevelDraft& draft) {
    const int width = draft.tileMap().width();
    const int height = draft.tileMap().height();
    const core::AtlasRegion solid =
        _atlas.tile(0, 0);  // region opaque unie (teintee pour la ligne)
    const float atlasWidth = static_cast<float>(_atlas.width());
    const float atlasHeight = static_cast<float>(_atlas.height());

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
    constexpr float LINE = 0.035f;  // epaisseur en unites monde (fraction de case)
    constexpr float lineAlpha = 0.18f;
    const float w = static_cast<float>(width);
    const float h = static_cast<float>(height);
    for (int column = 0; column <= width; ++column) {
        add(lineQuad(static_cast<float>(column) - LINE * 0.5f, 0.0f, LINE, h, 1.0f, 1.0f, 1.0f,
                     lineAlpha));
    }
    for (int row = 0; row <= height; ++row) {
        add(lineQuad(0.0f, static_cast<float>(row) - LINE * 0.5f, w, LINE, 1.0f, 1.0f, 1.0f,
                     lineAlpha));
    }

    // Frontieres de salles (RoomGrid, LOT-32), a la taille RESOLUE du niveau (LOT-64 : reglable,
    // valeurs par defaut sinon) : plus epaisses, teinte ambre. Meme remarque que
    // composeCameraFraming pour le mode "par salle".
    const core::CameraFramingConfig& framing = draft.cameraFraming();
    const int roomWidthTiles = framing.roomWidthTiles.value_or(core::DEFAULT_ROOM_WIDTH_TILES);
    const int roomHeightTiles = framing.roomHeightTiles.value_or(core::DEFAULT_ROOM_HEIGHT_TILES);
    constexpr float ROOM_LINE = 0.09f;
    constexpr float roomLineAlpha = 0.5f;
    for (int column = 0; column * roomWidthTiles <= width; ++column) {
        const float x = static_cast<float>(std::min(column * roomWidthTiles, width));
        add(lineQuad(x - ROOM_LINE * 0.5f, 0.0f, ROOM_LINE, h, 1.0f, 0.85f, 0.3f, roomLineAlpha));
    }
    for (int row = 0; row * roomHeightTiles <= height; ++row) {
        const float y = static_cast<float>(std::min(row * roomHeightTiles, height));
        add(lineQuad(0.0f, y - ROOM_LINE * 0.5f, w, ROOM_LINE, 1.0f, 0.85f, 0.3f, roomLineAlpha));
    }
}

// Compose la previsualisation du cadrage de camera du niveau (EX-EDIT-028, LOT-64, voir en-tete) :
// composee INCONDITIONNELLEMENT (comme composeLinks), pas derriere le
// bascule F10 -- ce n'est pas une aide de placement mais une information sur ce que montrera la
// camera en jeu.
void DraftRenderer::composeCameraFraming(const core::LevelDraft& draft) {
    const int width = draft.tileMap().width();
    const int height = draft.tileMap().height();
    const core::AtlasRegion solid = _atlas.tile(0, 0);
    const float atlasWidth = static_cast<float>(_atlas.width());
    const float atlasHeight = static_cast<float>(_atlas.height());

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
        _scene.addSprite(RenderLayer::EditorOverlay, _atlas.textureHandle(),
                         OVERLAY_ORDER_CAMERA_FRAMING, quad);
    };
    // Rectangle CREUX (quatre bords) : plus lisible qu'un voile plein sur une grande zone, et
    // distinguable de l'ambre de la grille de salles (teinte cyan).
    const auto strokeRect = [&](float x, float y, float w, float h, float thickness, float r,
                                float g, float b, float a) {
        add(lineQuad(x - thickness * 0.5f, y - thickness * 0.5f, w + thickness, thickness, r, g, b,
                     a));
        add(lineQuad(x - thickness * 0.5f, y + h - thickness * 0.5f, w + thickness, thickness, r, g,
                     b, a));
        add(lineQuad(x - thickness * 0.5f, y - thickness * 0.5f, thickness, h + thickness, r, g, b,
                     a));
        add(lineQuad(x + w - thickness * 0.5f, y - thickness * 0.5f, thickness, h + thickness, r, g,
                     b, a));
    };

    constexpr float FRAME_THICKNESS = 0.12f;
    constexpr float CYAN_R = 0.35f;
    constexpr float CYAN_G = 0.85f;
    constexpr float CYAN_B = 1.0f;

    const core::CameraFramingConfig& framing = draft.cameraFraming();
    switch (framing.mode) {
        case core::CameraFramingMode::WholeLevel:
            strokeRect(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height),
                       FRAME_THICKNESS, CYAN_R, CYAN_G, CYAN_B, 0.6f);
            break;
        case core::CameraFramingMode::PerRoom: {
            if (!framing.zones.empty()) {
                // Zones dessinees a la main (EX-LVL-007) : remplacent entierement la grille
                // automatique -- un rectangle par zone, dans l'ordre de la liste (la premiere
                // zone qui contient une position gagne en cas de chevauchement, hmi::
                // activeCameraZoneIndex, non visible ici : la previsualisation montre simplement
                // les rectangles tels que dessines).
                for (const core::CameraZone& zone : framing.zones) {
                    strokeRect(static_cast<float>(zone.x), static_cast<float>(zone.y),
                               static_cast<float>(zone.width), static_cast<float>(zone.height),
                               FRAME_THICKNESS, CYAN_R, CYAN_G, CYAN_B, 0.5f);
                }
                break;
            }
            // Reutilise hmi::RoomGrid (LOT-32) a la taille resolue -- jamais une seconde
            // implementation du decoupage en salles (tache-03).
            const int roomWidthTiles =
                framing.roomWidthTiles.value_or(core::DEFAULT_ROOM_WIDTH_TILES);
            const int roomHeightTiles =
                framing.roomHeightTiles.value_or(core::DEFAULT_ROOM_HEIGHT_TILES);
            const RoomGrid rooms(width, height, roomWidthTiles, roomHeightTiles);
            for (int row = 0; row < rooms.rows(); ++row) {
                for (int column = 0; column < rooms.columns(); ++column) {
                    const RoomBounds bounds = rooms.roomBounds(core::GridPosition{column, row});
                    strokeRect(static_cast<float>(bounds.column), static_cast<float>(bounds.row),
                               static_cast<float>(bounds.width), static_cast<float>(bounds.height),
                               FRAME_THICKNESS, CYAN_R, CYAN_G, CYAN_B, 0.5f);
                }
            }
            break;
        }
        case core::CameraFramingMode::Follow: {
            const std::optional<core::GridPosition> entry = draft.entry();
            if (!entry) {
                break;  // pas d'entree posee : rien de significatif a previsualiser (tache-03).
            }
            const float viewWidth = static_cast<float>(core::DEFAULT_ROOM_WIDTH_TILES);
            const float viewHeight = static_cast<float>(core::DEFAULT_ROOM_HEIGHT_TILES);
            // Meme regle de bornage/centrage qu'hmi::advanceFollowCamera (FollowCamera.cpp) : un
            // axe plus etroit que le cadrage centre plutot que borne -- previsualisation fidele au
            // comportement reel, pas une approximation.
            const auto clampCenter = [](float center, float levelSize, float viewSize) {
                if (levelSize <= viewSize) {
                    return levelSize * 0.5f;
                }
                return std::clamp(center, viewSize * 0.5f, levelSize - viewSize * 0.5f);
            };
            const float centerX = clampCenter(static_cast<float>(entry->column) + 0.5f,
                                              static_cast<float>(width), viewWidth);
            const float centerY = clampCenter(static_cast<float>(entry->row) + 0.5f,
                                              static_cast<float>(height), viewHeight);
            strokeRect(centerX - viewWidth * 0.5f, centerY - viewHeight * 0.5f, viewWidth,
                       viewHeight, FRAME_THICKNESS, CYAN_R, CYAN_G, CYAN_B, 0.6f);
            // Zone morte materialisee (tache-03) : plus fine, meme teinte, plus opaque (repere
            // secondaire a l'interieur du rectangle visible).
            strokeRect(centerX - FOLLOW_DEAD_ZONE_HALF_WIDTH_UNITS,
                       centerY - FOLLOW_DEAD_ZONE_HALF_HEIGHT_UNITS,
                       FOLLOW_DEAD_ZONE_HALF_WIDTH_UNITS * 2.0f,
                       FOLLOW_DEAD_ZONE_HALF_HEIGHT_UNITS * 2.0f, FRAME_THICKNESS * 0.6f, CYAN_R,
                       CYAN_G, CYAN_B, 0.85f);
            break;
        }
    }
}

// Compose les liens de mecanismes (fleches declencheur -> cible) sur le calque d'edition.
void DraftRenderer::composeLinks(const core::LevelDraft& draft, const LinkOverlayState& overlay) {
    const std::vector<LinkRow> rows = buildLinkRows(draft);
    if (rows.empty() && !overlay.pendingLink) {
        return;  // rien a dessiner (ni liaison, ni geste de creation en cours).
    }

    // Regroupe les liens par declencheur (fan-out anti-superposition, LinkGeometry::linkSegment) :
    // index et nombre total de liens partageant le meme declencheur, pour chaque ligne.
    std::vector<int> fanIndex(rows.size(), 0);
    std::vector<int> fanCount(rows.size(), 1);
    for (std::size_t i = 0; i < rows.size(); ++i) {
        int index = 0;
        int count = 0;
        for (std::size_t j = 0; j < rows.size(); ++j) {
            if (rows[j].trigger == rows[i].trigger) {
                if (j < i) {
                    ++index;
                }
                ++count;
            }
        }
        fanIndex[i] = index;
        fanCount[i] = count;
    }

    const core::AtlasRegion solid = _atlas.tile(0, 0);  // region opaque unie (teintee).
    const float atlasWidth = static_cast<float>(_atlas.width());
    const float atlasHeight = static_cast<float>(_atlas.height());
    const float u0 = static_cast<float>(solid.x) / atlasWidth;
    const float v0 = static_cast<float>(solid.y) / atlasHeight;
    const float u1 = static_cast<float>(solid.x + solid.width) / atlasWidth;
    const float v1 = static_cast<float>(solid.y + solid.height) / atlasHeight;

    // Trait epais entre deux points monde, meme region de texture (unie) que le reste de l'overlay.
    const auto segment = [&](core::Vector2 a, core::Vector2 b, float thickness, float r, float g,
                             float bl, float alpha) {
        LineQuad quad;
        quad.ax = a.x;
        quad.ay = a.y;
        quad.bx = b.x;
        quad.by = b.y;
        quad.thickness = thickness;
        quad.u0 = u0;
        quad.v0 = v0;
        quad.u1 = u1;
        quad.v1 = v1;
        quad.r = r;
        quad.g = g;
        quad.b = bl;
        quad.a = alpha;
        return quad;
    };
    const auto addLine = [&](const LineQuad& quad) {
        _scene.addLine(RenderLayer::EditorOverlay, _atlas.textureHandle(), OVERLAY_ORDER_LINKS,
                       quad);
    };

    constexpr float LINE_THICKNESS = 0.045f;
    constexpr float HIGHLIGHT_THICKNESS = 0.08f;
    constexpr float PENDING_THICKNESS = 0.03f;

    // Case en attente (premier clic de l'outil Lien) : voile plein pour la signaler.
    if (overlay.pendingLink) {
        SpriteQuad pendingQuad;
        pendingQuad.x = static_cast<float>(overlay.pendingLink->column);
        pendingQuad.y = static_cast<float>(overlay.pendingLink->row);
        pendingQuad.width = 1.0f;
        pendingQuad.height = 1.0f;
        pendingQuad.u0 = u0;
        pendingQuad.v0 = v0;
        pendingQuad.u1 = u1;
        pendingQuad.v1 = v1;
        pendingQuad.r = 1.0f;
        pendingQuad.g = 0.9f;
        pendingQuad.b = 0.2f;
        pendingQuad.a = 0.35f;
        _scene.addSprite(RenderLayer::EditorOverlay, _atlas.textureHandle(), OVERLAY_ORDER_LINKS,
                         pendingQuad);

        // Trait provisoire vers la case survolee, si distincte (retour visuel du geste en cours).
        if (overlay.hoveredCell && *overlay.hoveredCell != *overlay.pendingLink) {
            const core::Vector2 from{static_cast<float>(overlay.pendingLink->column) + 0.5f,
                                     static_cast<float>(overlay.pendingLink->row) + 0.5f};
            const core::Vector2 to{static_cast<float>(overlay.hoveredCell->column) + 0.5f,
                                   static_cast<float>(overlay.hoveredCell->row) + 0.5f};
            addLine(segment(from, to, PENDING_THICKNESS, 1.0f, 0.9f, 0.2f, 0.7f));
        }
    }

    for (std::size_t i = 0; i < rows.size(); ++i) {
        const LinkRow& row = rows[i];
        const bool selected = overlay.selectedLink.has_value() &&
                              overlay.selectedLink->first == row.trigger &&
                              overlay.selectedLink->second == row.target;
        const bool hovered =
            overlay.hoveredCell.has_value() &&
            (*overlay.hoveredCell == row.trigger || *overlay.hoveredCell == row.target);
        const bool highlighted = selected || hovered;

        const LinkSegment line = linkSegment(row.trigger, row.target, fanIndex[i], fanCount[i]);
        const bool mechanism = row.kind == LinkKind::Mechanism;
        const float r = mechanism ? 0.35f : 1.0f;
        const float g = mechanism ? 0.85f : 0.45f;
        const float b = mechanism ? 1.0f : 0.3f;
        const float alpha = highlighted ? 0.95f : 0.65f;
        const float thickness = highlighted ? HIGHLIGHT_THICKNESS : LINE_THICKNESS;

        addLine(segment(line.a, line.b, thickness, r, g, b, alpha));
        const ArrowHead head = arrowHead(line.a, line.b);
        addLine(segment(line.b, head.left, thickness, r, g, b, alpha));
        addLine(segment(line.b, head.right, thickness, r, g, b, alpha));
    }
}

void DraftRenderer::rebuild(const core::LevelDraft& draft) {
    _world = core::World{};  // repart d'une scène vierge
    const core::TileMap& map = draft.tileMap();
    for (int row = 0; row < map.height(); ++row) {
        for (int column = 0; column < map.width(); ++column) {
            const core::TileType type = map.tile(column, row);
            if (type == core::TileType::Empty) {
                continue;  // case vide : aucune entité (grille éparse, comme en jeu).
            }
            const core::Entity entity = _world.createEntity();
            _world.addComponent(entity,
                                core::Transform{core::Vector2{static_cast<float>(column),
                                                              static_cast<float>(row)},
                                                core::Vector2{1.0f, 1.0f}, 0.0f});
            core::Sprite sprite;
            sprite.region = regionForTile(type);
            sprite.tint = core::Color{1.0f, 1.0f, 1.0f, 1.0f};
            // Aucun calque a fixer : une entite sans `RenderLayerTag` est dessinee sur
            // RenderLayer::Tile (hmi::DEFAULT_RENDER_LAYER), et `core::Sprite::layer` garde sa
            // valeur par defaut -- le tri fin entre tuiles n'a pas lieu d'etre.
            _world.addComponent(entity, sprite);
            // Marque d'habillage (LOT-42), identique a celle posee en jeu : c'est ce qui fait que
            // le canevas de l'editeur montre exactement ce que le joueur verra. La surcharge de
            // texture par instance (EX-EDIT-043, LOT-45) y est resolue une fois, ici, comme le
            // voisinage solide.
            _world.addComponent(entity,
                                TileSkinTag{type, solidNeighborMask(map, column, row),
                                            textureOverrideAt(draft.textureOverrides(),
                                                              core::GridPosition{column, row})});
        }
    }
}

}  // namespace hmi
