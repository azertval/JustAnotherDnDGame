// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Graphics/ComposedScene.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "Core/Ecs/Components/Sprite.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/Entity.h"
#include "Core/Ecs/World.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/PlayerSpriteTag.h"
#include "HMI/Graphics/PreviousPosition.h"
#include "HMI/Graphics/TileSkinTag.h"

namespace hmi {

// Vide la scene (capacite conservee) et remet les compteurs a zero. Le cadrage est conserve.
void ComposedScene::clear() noexcept {
    _quads.clear();
    _textureOrder.clear();
    _considered = 0;
    _culled = 0;
    _textureBytes = 0;
}

// Memoire de texture d'une primitive deja ajoutee (EX-NFR-043) : simple accumulation, l'emetteur
// decide seul de ce qui compte (voir l'en-tete -- aujourd'hui les seuls plans picturaux).
void ComposedScene::addTextureBytes(std::size_t bytes) noexcept {
    _textureBytes += bytes;
}

// Restreint la composition aux primitives visibles dans un cadrage donne (EX-NFR-005). Le
// rectangle elargi de la marge est calcule une fois ici, et non a chaque primitive testee.
void ComposedScene::setVisibleBounds(const core::Rect& worldBounds) noexcept {
    _visibleBounds = core::Rect{core::Vector2{worldBounds.position.x - CULLING_MARGIN_UNITS,
                                              worldBounds.position.y - CULLING_MARGIN_UNITS},
                                core::Vector2{worldBounds.size.x + 2.0f * CULLING_MARGIN_UNITS,
                                              worldBounds.size.y + 2.0f * CULLING_MARGIN_UNITS}};
}

// Desactive le culling : toutes les primitives ajoutees sont conservees.
void ComposedScene::clearVisibleBounds() noexcept {
    _visibleBounds.reset();
}

// Rectangle reellement utilise pour le culling : le cadrage elargi de la marge.
core::Rect ComposedScene::cullingBounds() const noexcept {
    return _visibleBounds.value_or(core::Rect{});
}

// Rang de premiere apparition d'une texture, ajoutee a la table si elle est nouvelle. Le nombre de
// textures distinctes par image se compte sur les doigts d'une main : une recherche lineaire est
// plus rapide (et bien plus simple) qu'une table de hachage.
int ComposedScene::textureRank(TextureHandle texture) {
    const auto found = std::find(_textureOrder.begin(), _textureOrder.end(), texture);
    if (found != _textureOrder.end()) {
        return static_cast<int>(std::distance(_textureOrder.begin(), found));
    }
    _textureOrder.push_back(texture);
    return static_cast<int>(_textureOrder.size()) - 1;
}

// true si la boite englobante est visible (ou si le culling est desactive).
bool ComposedScene::isVisible(const core::Rect& bounds) const {
    if (!_visibleBounds) {
        return true;
    }
    return _visibleBounds->intersects(bounds);
}

// Ajoute un rectangle texture a la scene, s'il est visible.
// true si la primitive a ete conservee, false si le culling l'a ecartee.
bool ComposedScene::addSprite(RenderLayer layer, TextureHandle texture, std::int32_t sortOrder,
                              const SpriteQuad& quad) {
    ++_considered;
    if (!isVisible(spriteQuadBounds(quad))) {
        ++_culled;
        return false;
    }
    ComposedQuad composed;
    composed.layer = layer;
    composed.texture = texture;
    composed.textureRank = textureRank(texture);
    composed.sortOrder = sortOrder;
    composed.kind = QuadKind::Sprite;
    composed.sprite = quad;
    _quads.push_back(composed);
    return true;
}

// Ajoute un segment epais a la scene, s'il est visible.
// true si la primitive a ete conservee, false si le culling l'a ecartee.
bool ComposedScene::addLine(RenderLayer layer, TextureHandle texture, std::int32_t sortOrder,
                            const LineQuad& quad) {
    ++_considered;
    if (!isVisible(lineQuadBounds(quad))) {
        ++_culled;
        return false;
    }
    ComposedQuad composed;
    composed.layer = layer;
    composed.texture = texture;
    composed.textureRank = textureRank(texture);
    composed.sortOrder = sortOrder;
    composed.kind = QuadKind::Line;
    composed.line = quad;
    _quads.push_back(composed);
    return true;
}

// Ordre de profondeur d'une primitive, a partir du pied de son quad (voir en-tete).
std::int32_t depthSortOrder(float footWorldY) noexcept {
    return static_cast<std::int32_t>(std::lround(footWorldY * DEPTH_SUBDIVISIONS_PER_UNIT));
}

// Ordonne la scene (bande, puis profondeur ou texture, puis le reste), de facon stable.
void ComposedScene::sort() {
    // La BANDE est prioritaire sur tout : regrouper par texture ne doit jamais faire passer une
    // primitive devant une primitive d'une bande inferieure (EX-REN-014). Le tri stable preserve
    // l'ordre de composition a cle egale.
    //
    // A l'interieur d'une bande, deux regimes (LOT-07) :
    //  - bande de PROFONDEUR (Object + Player) : le sortOrder -- le Y du pied -- tranche AVANT la
    //    texture. C'est la seule facon qu'un personnage passe derriere un arbre plus bas et devant
    //    un arbre plus haut, les deux n'ayant jamais la meme texture que lui (EX-REN-018) ;
    //  - partout ailleurs : la texture regroupe d'abord (une passe de dessin par groupe), le
    //    sortOrder ne departageant que l'interieur d'un groupe -- comportement d'avant le lot.
    std::stable_sort(_quads.begin(), _quads.end(),
                     [](const ComposedQuad& lhs, const ComposedQuad& rhs) {
                         const std::int32_t leftBand = renderBand(lhs.layer);
                         const std::int32_t rightBand = renderBand(rhs.layer);
                         if (leftBand != rightBand) {
                             return leftBand < rightBand;
                         }
                         if (sortsByDepth(lhs.layer)) {
                             if (lhs.sortOrder != rhs.sortOrder) {
                                 return lhs.sortOrder < rhs.sortOrder;
                             }
                             return lhs.textureRank < rhs.textureRank;
                         }
                         if (lhs.textureRank != rhs.textureRank) {
                             return lhs.textureRank < rhs.textureRank;
                         }
                         return lhs.sortOrder < rhs.sortOrder;
                     });
}

// Le nombre de passes begin/end : groupes contigus de meme texture.
int ComposedScene::batchCount() const noexcept {
    int batches = 0;
    TextureHandle current = nullptr;
    for (std::size_t i = 0; i < _quads.size(); ++i) {
        if (i == 0 || _quads[i].texture != current) {
            ++batches;
            current = _quads[i].texture;
        }
    }
    return batches;
}

// Les compteurs de l'image composee (EX-NFR-005).
SceneStatistics ComposedScene::statistics() const noexcept {
    SceneStatistics stats;
    stats.considered = _considered;
    stats.culled = _culled;
    stats.submitted = static_cast<int>(_quads.size());
    stats.batches = batchCount();
    stats.textureBytes = _textureBytes;
    return stats;
}

// Resume des compteurs d'une image, pour la journalisation de diagnostic (EX-NFR-005).
std::string formatSceneStatistics(const SceneStatistics& statistics) {
    return "Rendu : " + std::to_string(statistics.considered) + " primitive(s) composee(s), " +
           std::to_string(statistics.culled) + " ecartee(s) hors cadrage, " +
           std::to_string(statistics.submitted) + " soumise(s) en " +
           std::to_string(statistics.batches) + " passe(s), " +
           std::to_string(statistics.textureBytes / 1024) + " Kio de plans.";
}

// Boite englobante d'un rectangle texture, en unites monde -- tient compte de la rotation
// : un quad pivote occupe un rectangle englobant plus grand que sa taille propre, le culling
// doit donc le juger sur ce rectangle-la, jamais sur (x, y, width, height) brut (une entite pivotee
// pres du bord du cadrage disparaitrait sinon a tort).
core::Rect spriteQuadBounds(const SpriteQuad& quad) noexcept {
    if (quad.rotation == 0.0f) {
        return core::Rect{core::Vector2{quad.x, quad.y}, core::Vector2{quad.width, quad.height}};
    }
    const float halfWidth = quad.width * 0.5f;
    const float halfHeight = quad.height * 0.5f;
    const float centerX = quad.x + halfWidth;
    const float centerY = quad.y + halfHeight;
    const float cosR = std::fabs(std::cos(quad.rotation));
    const float sinR = std::fabs(std::sin(quad.rotation));
    const float extentX = halfWidth * cosR + halfHeight * sinR;
    const float extentY = halfWidth * sinR + halfHeight * cosR;
    return core::Rect{core::Vector2{centerX - extentX, centerY - extentY},
                      core::Vector2{extentX * 2.0f, extentY * 2.0f}};
}

// Boite englobante d'un segment epais, en unites monde (extremites elargies d'une demi-epaisseur).
core::Rect lineQuadBounds(const LineQuad& quad) noexcept {
    const float half = quad.thickness * 0.5f;
    const float left = (std::min)(quad.ax, quad.bx) - half;
    const float top = (std::min)(quad.ay, quad.by) - half;
    const float right = (std::max)(quad.ax, quad.bx) + half;
    const float bottom = (std::max)(quad.ay, quad.by) + half;
    return core::Rect{core::Vector2{left, top}, core::Vector2{right - left, bottom - top}};
}

namespace {

// Region/texture/dimensions monde d'une entite affichable, une fois son apparence resolue --
// resultat de resolveSpriteAppearance ci-dessous.
struct SpriteAppearance {
    core::AtlasRegion region;
    TextureHandle texture = nullptr;
    float atlasWidth = 0.0f;
    float atlasHeight = 0.0f;
    float worldWidth = 0.0f;
    float worldHeight = 0.0f;
    core::Vector2 quadOffset{};
};

// Resout l'apparence (texture, region, taille monde) d'une entite affichable selon sa nature --
// personnage, ou tuile generique (seul cas ou l'apparence peut etre absente : calque masque ou axe
// skin/surcharge sans rien a montrer, cf. hmi::resolveTileAppearance).
std::optional<SpriteAppearance> resolveSpriteAppearance(
    core::World& world, core::Entity entity, const core::Transform& transform,
    const core::Sprite& sprite, RenderMode mode, const SceneTextures& textures,
    const LayerVisibility& visibility, const PlayerSpriteTag* playerTag, bool usePlayerAppearance) {
    SpriteAppearance result;
    if (usePlayerAppearance) {
        result.region = playerTag->textureRegion;
        if (playerTag->usesCharacterSheet) {
            result.texture = textures.characterSheet;
            result.atlasWidth = static_cast<float>(textures.characterSheetWidth);
            result.atlasHeight = static_cast<float>(textures.characterSheetHeight);
        } else {
            result.texture = textures.atlas;
            result.atlasWidth = static_cast<float>(textures.atlasWidth);
            result.atlasHeight = static_cast<float>(textures.atlasHeight);
        }
        // Taille du quad independante de la hitbox (TACHE-01) : deja en unites monde, calculee
        // par hmi::computePlayerSpriteQuad -- pas de facteur Transform::scale ici.
        result.worldWidth = playerTag->quadSize.x;
        result.worldHeight = playerTag->quadSize.y;
        result.quadOffset = playerTag->quadOffset;
        return result;
    }

    // Type et voisinage de la tuile, quand l'entite en porte un (LOT-42) : c'est de la que vient
    // le choix du skin. Une entite sans ce composant n'est pas habillable.
    const TileSkinTag* skinTag = world.hasComponent<TileSkinTag>(entity)
                                     ? &world.getComponent<TileSkinTag>(entity)
                                     : nullptr;

    // Apparence resolue par le point d'appel unique (LOT-41) : c'est ici, a la composition, que
    // le mode de rendu agit -- la scene ECS, elle, ne bouge pas. Les axes skin/surcharge (LOT-51)
    // sont ceux des bits Tile/Object de `visibility` : absent (std::nullopt), rien n'est compose
    // pour cette entite (calque masque, ou axe isole sans rien a montrer).
    const std::optional<TileAppearance> appearance = resolveTileAppearance(
        mode, sprite.region, skinTag, textures, visibility.visible(RenderLayer::Tile),
        visibility.visible(RenderLayer::Object));
    if (!appearance.has_value()) {
        return std::nullopt;
    }
    result.region = appearance->region;
    result.texture = textures.textureFor(*appearance);
    result.atlasWidth = static_cast<float>(textures.widthFor(*appearance));
    result.atlasHeight = static_cast<float>(textures.heightFor(*appearance));

    // Taille du sprite en unites monde : la region (en pixels) ramenee a l'echelle du monde (16
    // px/unite), multipliee par l'echelle du Transform. Le zoom est applique plus tard par la
    // projection de la camera. Le damier de repli fait exactement une case (MISSING_TEXTURE_SIZE
    // == TILE_SIZE) : la geometrie composee est donc la meme dans les deux modes, seule la
    // texture echantillonnee change.
    result.worldWidth = static_cast<float>(appearance->region.width) / Camera2D::PIXELS_PER_UNIT *
                        transform.scale.x;
    result.worldHeight = static_cast<float>(appearance->region.height) / Camera2D::PIXELS_PER_UNIT *
                         transform.scale.y;
    return result;
}

}  // namespace

// Compose les entites affichables d'un monde ECS en primitives (lecture seule de l'ECS).
void composeWorldSprites(ComposedScene& scene, core::World& world, RenderMode mode,
                         const SceneTextures& textures, float interpolationAlpha,
                         const LayerVisibility& visibility) {
    // Lecture seule de l'ECS : les composants sont pris par reference constante.
    world.view<core::Transform, core::Sprite>().each(
        [&](core::Entity entity, const core::Transform& transform, const core::Sprite& sprite) {
            // Habillage du personnage (LOT-48), en RenderMode::Texture uniquement : resolu a part
            // de hmi::resolveTileAppearance (le personnage n'est pas une tuile, cf.
            // PlayerSpriteTag)
            // -- son quad est decorrelee de la hitbox, contrairement au chemin generique
            // ci-dessous.
            const PlayerSpriteTag* playerTag = world.hasComponent<PlayerSpriteTag>(entity)
                                                   ? &world.getComponent<PlayerSpriteTag>(entity)
                                                   : nullptr;
            const bool usePlayerAppearance = playerTag != nullptr && mode == RenderMode::Texture;

            // Calque de presentation : porte par l'entite si elle est taguee, sinon le defaut
            // (tuiles). Calcule ici, avant toute resolution d'apparence, pour pouvoir ecarter au
            // plus tot une entite de calque masque (LOT-51) -- personnage seulement : une entite
            // "tuile" (aucun RenderLayerTag) est filtree plus finement ci-dessous, par axe
            // skin/surcharge plutot que par ce seul bit de calque (hmi::resolveTileAppearance).
            const RenderLayer layer = world.hasComponent<RenderLayerTag>(entity)
                                          ? world.getComponent<RenderLayerTag>(entity).value
                                          : DEFAULT_RENDER_LAYER;
            const bool isTileEntity = !usePlayerAppearance;
            if (!isTileEntity && !visibility.visible(layer)) {
                return;
            }

            const std::optional<SpriteAppearance> resolved =
                resolveSpriteAppearance(world, entity, transform, sprite, mode, textures,
                                        visibility, playerTag, usePlayerAppearance);
            if (!resolved) {
                return;  // tuile generique sans apparence a montrer (calque masque, axe isole).
            }
            const core::AtlasRegion region = resolved->region;
            const TextureHandle texture = resolved->texture;
            const float atlasWidth = resolved->atlasWidth;
            const float atlasHeight = resolved->atlasHeight;
            const float worldWidth = resolved->worldWidth;
            const float worldHeight = resolved->worldHeight;
            const core::Vector2 quadOffset = resolved->quadOffset;

            // Position a dessiner : interpolee entre le pas precedent et le pas courant si
            // l'entite porte un `PreviousPosition` (personnage, dangers mobiles, blocs -- mouvement
            // continu), sinon la position courante telle quelle (tuiles fixes). Lire une seconde
            // pool pendant l'iteration de la vue Transform+Sprite est sur : on ne modifie aucune
            // pool, donc la vue reste valide (cf. @ref guide-ecs).
            core::Vector2 position = transform.position;
            if (world.hasComponent<PreviousPosition>(entity)) {
                const core::Vector2 previous = world.getComponent<PreviousPosition>(entity).value;
                position = previous + (transform.position - previous) * interpolationAlpha;
            }

            position = position + quadOffset;

            SpriteQuad quad;
            quad.x = position.x;
            quad.y = position.y;
            quad.width = worldWidth;
            quad.height = worldHeight;
            // Rotation portee par le Transform : nulle pour toute entite du jeu (tuiles,
            // personnage), l'orientation restant l'exception plutot que la regle.
            quad.rotation = transform.rotation;
            // Coordonnees de texture normalisees a partir de la region en pixels.
            quad.u0 = static_cast<float>(region.x) / atlasWidth;
            quad.v0 = static_cast<float>(region.y) / atlasHeight;
            quad.u1 = static_cast<float>(region.x + region.width) / atlasWidth;
            quad.v1 = static_cast<float>(region.y + region.height) / atlasHeight;
            if (usePlayerAppearance && playerTag->flipHorizontal) {
                // Retournement horizontal (TACHE-03) : u0/u1 echanges, ancrage deja symetrique
                // (quadOffset ci-dessus ne depend pas du sens) -- aucun decalage de position.
                std::swap(quad.u0, quad.u1);
            }
            quad.r = sprite.tint.r;
            quad.g = sprite.tint.g;
            quad.b = sprite.tint.b;
            quad.a = sprite.tint.a;

            // Tri fin (LOT-07). Sur la bande de PROFONDEUR, c'est le Y du PIED du quad qui
            // decide : le personnage passe devant ce qui est au-dessus de lui a l'ecran, derriere
            // ce qui est en dessous (EX-REN-018). Partout ailleurs -- tuiles, plans, interface --
            // `core::Sprite::layer` conserve son role de tri fin, et le rang de couche que la
            // projection y a ecrit (LOT-04) continue d'ordonner sol et decor.
            const std::int32_t fineOrder =
                sortsByDepth(layer) ? depthSortOrder(quad.y + quad.height) : sprite.layer;
            scene.addSprite(layer, texture, fineOrder, quad);
        });
}

}  // namespace hmi
