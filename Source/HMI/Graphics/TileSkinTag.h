// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "Core/Ecs/Components/Sprite.h"  // core::AtlasRegion
#include "Core/Levels/TileType.h"

/**
 * @file HMI/Graphics/TileSkinTag.h
 * @brief Marque de présentation portée par une entité tuile, pour résoudre son skin
 *        (`EX-EDIT-042`, `EX-EDIT-025`).
 */

namespace hmi {

/**
 * @brief Type de tuile et voisinage solide d'une entité affichée, pour la résolution de son skin.
 *
 * `core::Sprite` ne porte qu'une **région d'atlas** : à la composition, plus rien n'indique quel
 * type de tuile est affiché, alors que c'est exactement ce dont dépend le choix d'un skin. Ce
 * composant rétablit l'information du **côté présentation**, sans faire entrer une notion
 * d'habillage dans `Core` — même patron que `hmi::RenderLayerTag` et `hmi::PreviousPosition`, déjà
 * des composants HMI attachés à des entités construites par `Core`.
 *
 * Le masque de voisinage est calculé **une fois**, à la construction de la scène : il ne dépend
 * que de la grille du niveau, jamais du mode de rendu ni du jeu de skins courant. Basculer de mode
 * (`F8`) ou réassigner un skin ne reconstruit donc pas la scène ECS — la règle posée par le
 * `LOT-41` reste tenue.
 *
 * Une entité **sans** ce composant (le personnage, les aides d'édition) n'est jamais habillée :
 * elle garde l'apparence résolue en mode Physique.
 */
struct TileSkinTag {
    /// Type de la tuile représentée par l'entité.
    core::TileType type{};
    /// Masque des quatre voisins solides (`hmi::solidNeighborMask`), pour le mode `bitmask16`.
    std::uint8_t neighborMask = 0;
    /// Nom de l'asset assigné **par instance** à cette case (`EX-EDIT-043`, `LOT-45`), prioritaire
    /// sur le skin du type ; vide si la case n'a pas de surcharge. Calculé une fois à la
    /// construction de la scène (`hmi::textureOverrideAt`), même principe que `neighborMask`.
    std::optional<std::string> overrideAsset;
    /// Image **courante**, par instance, d'un mécanisme animé (`LOT-47`) : recalculée chaque pas
    /// fixe par `hmi::GameSession` d'après l'état logique de **cette** tuile (porte, interrupteur,
    /// danger…), prioritaire sur l'horloge partagée par asset de `hmi::SkinTexture::animatedFrame`
    /// (`LOT-46` TACHE-05, décorative, en phase pour toutes les tuiles d'un même asset). Les deux
    /// mécanismes coexistent parce que leurs granularités diffèrent : une nappe d'eau anime tout un
    /// asset en phase, une porte anime **une case précise**, indépendamment de ses voisines de même
    /// asset. `std::nullopt` hors mécanisme (repli sur l'horloge partagée, ou sur l'image entière).
    std::optional<core::AtlasRegion> animatedFrame;
};

}  // namespace hmi
