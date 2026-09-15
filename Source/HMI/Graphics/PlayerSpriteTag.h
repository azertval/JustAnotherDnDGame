// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include "Core/Ecs/Components/Sprite.h"  // core::AtlasRegion
#include "Core/Math/Vector2.h"

/**
 * @file HMI/Graphics/PlayerSpriteTag.h
 * @brief Marque de présentation portée par le personnage, pour son habillage (`LOT-48`).
 */

namespace hmi {

/**
 * @brief Apparence du personnage en `RenderMode::Texture`, résolue chaque image
 *        (`GameSession::refreshPlayerSprite`) — même patron que `hmi::TileSkinTag` pour les
 *        tuiles : le composant restitue côté présentation ce que `core::Sprite`/`core::Transform`
 *        (partagés avec la physique) ne peuvent pas porter sans coupler simulation et rendu.
 *
 * `core::Sprite::region` continue de porter la région **procédurale** (utilisée telle quelle en
 * `RenderMode::Physique`, comportement inchangé depuis avant `LOT-48`) ; ce composant porte la
 * région **résolue pour l'habillage** (spritesheet externe si chargée, sinon le même repli
 * procédural), le quad décorrélé de la hitbox (`hmi::computePlayerSpriteQuad`, TACHE-01) et
 * l'orientation (TACHE-03). Une entité **sans** ce composant (tout ce qui n'est pas le
 * personnage) n'est pas concernée par cette résolution — `hmi::composeWorldSprites` continue de
 * dessiner ces entités par le chemin `Sprite::region`/`Transform::scale` existant.
 */
struct PlayerSpriteTag {
    /// Région à échantillonner dans la texture d'habillage résolue (spritesheet externe si
    /// `usesCharacterSheet`, sinon l'atlas — même région que `core::Sprite::region` dans ce cas).
    core::AtlasRegion textureRegion{};
    /// `true` si `textureRegion` doit être échantillonnée dans la spritesheet externe
    /// (`hmi::SceneTextures::characterSheet`) plutôt que dans l'atlas (repli procédural).
    bool usesCharacterSheet = false;
    /// Décalage du coin haut-gauche du quad depuis `Transform::position` (coin haut-gauche de la
    /// hitbox), en unités monde — ancrage centre-bas (`hmi::computePlayerSpriteQuad`, TACHE-01).
    core::Vector2 quadOffset{};
    /// Taille du quad à l'écran, en unités monde — indépendante de la hitbox (TACHE-01).
    core::Vector2 quadSize{};
    /// `true` si le personnage regarde vers la **gauche** (`core::Player::facing < 0`) : les
    /// coordonnées de texture horizontales sont échangées à la composition (TACHE-03), sans
    /// affecter `quadOffset`/`quadSize` (ancrage symétrique).
    bool flipHorizontal = false;
};

}  // namespace hmi
