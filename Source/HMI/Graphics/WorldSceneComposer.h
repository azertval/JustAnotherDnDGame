// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "Core/Levels/GridPosition.h"
#include "Core/Math/Vector2.h"
#include "HMI/Graphics/ComposedScene.h"
#include "HMI/Graphics/ScenePieces.h"

/**
 * @file HMI/Graphics/WorldSceneComposer.h
 * @brief Un **lieu qu'on parcourt** composé en primitives, sans GPU : le jumeau
 *        d'`hmi::ArenaSceneComposer` pour la carte du jeu (`LOT-09`).
 *
 * Le lieu et l'arène se dessinent par le **même** code : même projection isométrique
 * (`core::IsoProjection`), mêmes planches de l'atelier des textures (`LOT-92`, `ScenePieces.h`),
 * même tri par profondeur. Ce qui change est la **source** : l'arène lit une grille de combat, le
 * lieu lit une carte (`core::Level`) — ses couches, ses assignations de texture, ses entités.
 *
 * ## Ce qui va où
 *
 * | Pièce | Calque | Tri |
 * |---|---|---|
 * | sol (type de tuile de la couche « sol » → table du lieu) | `RenderLayer::Tile` | profondeur de case |
 * | relief (pièce nommée à la case, ou type de la couche « décor ») | `RenderLayer::Object` | pied de la case |
 * | figurine (héros, PNJ) | `RenderLayer::Player` | pied de sa case |
 *
 * ## Par instantané, comme l'arène
 *
 * La composition ne lit qu'un `hmi::WorldSceneSnapshot` — des **valeurs**, aucun pointeur vers la
 * carte ni vers la session. C'est ce qui lui permet de tourner sur le fil de rendu de Qt Quick,
 * l'instantané étant pris dans `synchronize()` pendant que le fil graphique est bloqué.
 */

namespace core {
class IsoProjection;
class Level;
}  // namespace core

namespace hmi {

class PlaceAppearance;

/// @brief Rang d'une pièce dans une même profondeur : le relief, puis la figurine posée dessus.
enum class WorldDepthSlot : std::int32_t {
    Relief = 0,
    Figure,
};

/// Nombre de rangs par profondeur.
inline constexpr std::int32_t WORLD_DEPTH_SLOTS = 2;

/// Marge basse d'une figurine, en hauteurs de losange — la même que dans l'arène.
inline constexpr float WORLD_FIGURE_BOTTOM_MARGIN = 0.42F;

/// @brief Propriété de couche qui nomme le **lieu** dont la carte porte les planches.
inline constexpr std::string_view SCENE_PLACE_PROPERTY = "scene";

/// @brief Ordre de tri d'une pièce du lieu.
[[nodiscard]] std::int32_t worldDepthSortOrder(float footWorldY, WorldDepthSlot slot) noexcept;

/// @brief Une figurine à dessiner sur la carte : sa planche, son image, où elle est.
struct WorldFigureSnapshot {
    /// Identifiant de la figurine de l'atelier (`anariel`, `jade`…), dossier `Assets/Npc/<slug>`.
    std::string figure;
    /// Bande d'animation : `idle`, `walk`.
    std::string clip = "idle";
    /// Position **continue**, en cases : `{1.5, 2.5}` est le centre de la case (1, 2).
    core::Vector2 point{};
    /// Image de la bande, ramenée dans la bande par la composition.
    int frame = 0;

    [[nodiscard]] bool operator==(const WorldFigureSnapshot&) const = default;
};

/**
 * @brief Le lieu **en valeurs** : ce que la composition lit, et rien d'autre.
 *
 * `floors` et `relief` portent une entrée par case, ligne par ligne : le **nom** de la pièce de la
 * planche du lieu, vide si la case ne dessine rien.
 */
struct WorldSceneSnapshot {
    int columns = 0;
    int rows = 0;
    /// Le lieu, qui nomme le dossier de planches : `Assets/Scene/<place>/`.
    std::string place;
    std::vector<std::string> floors;
    std::vector<std::string> relief;
    std::vector<WorldFigureSnapshot> figures;

    /// @return Le nom de la pièce de sol de @p cell, vide hors grille ou sans pièce.
    [[nodiscard]] std::string_view floorAt(core::GridPosition cell) const;
    /// @return Le nom de la pièce de relief de @p cell, vide hors grille ou sans relief.
    [[nodiscard]] std::string_view reliefAt(core::GridPosition cell) const;

    [[nodiscard]] bool operator==(const WorldSceneSnapshot&) const = default;
};

/// @return Le lieu que déclare @p level (propriété de couche `scene`), vide s'il n'en déclare pas.
[[nodiscard]] std::string scenePlaceOf(const core::Level& level);

/**
 * @brief Tire de @p level l'instantané que la composition dessine.
 *
 * Le sol vient de la couche **visuelle de sol** (`core::LayerKind::Ground`, à défaut la grille
 * racine), traduit par @p appearance ; le relief de la couche **décor**, où une assignation de
 * texture à la case (`core::TileTextureOverride`) l'emporte sur la table du lieu.
 *
 * @param level      La carte, lue seulement.
 * @param appearance La table du lieu.
 * @param figures    Les figurines à poser, dans l'ordre où l'appelant les veut.
 */
[[nodiscard]] WorldSceneSnapshot snapshotWorldScene(const core::Level& level,
                                                    const PlaceAppearance& appearance,
                                                    std::vector<WorldFigureSnapshot> figures);

/// @return Tous les chemins de texture que @p snapshot demandera, sans doublon, triés.
[[nodiscard]] std::vector<std::string> worldTexturePaths(const WorldSceneSnapshot& snapshot);

/**
 * @brief Compose le lieu dans un tampon réutilisé.
 *
 * Le tampon n'est **ni vidé ni trié** : même contrat qu'`hmi::composeArenaScene`, l'appelant
 * enchaîne `clear()`, les compositions, puis `sort()`.
 */
void composeWorldScene(ComposedScene& scene, const WorldSceneSnapshot& snapshot,
                       const core::IsoProjection& projection, const ScenePieceTextures& textures);

/// @brief Compose le lieu dans une scène neuve, **triée** — commodité des tests et des captures.
[[nodiscard]] ComposedScene composeWorldScene(const WorldSceneSnapshot& snapshot,
                                              const core::IsoProjection& projection,
                                              const ScenePieceTextures& textures);

}  // namespace hmi
