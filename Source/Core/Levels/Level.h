// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/MapEntity.h"
#include "Core/Levels/TileLayer.h"
#include "Core/Levels/TileMap.h"

/**
 * @file Core/Levels/Level.h
 * @brief Carte chargée : grille de collision, couches, entités, point d'entrée.
 */

namespace core {

/**
 * @brief Pièce de la planche du lieu assignée explicitement à **une case précise**, prioritaire sur
 *        la table d'apparence de son type (`EX-EDIT-043`, `EX-VIS-008`).
 *
 * Vecteur annexe de `Level`, keyé par position, `TileMap` ne portant qu'un `TileType` par case. Le
 * nom de pièce est une simple chaîne : `Core` ne vérifie pas son existence (`EX-NFR-011`), une
 * assignation pointant une pièce absente reste une carte valide.
 */
struct TileTextureOverride {
    GridPosition position;
    std::string assetName;
};

/**
 * @brief Composantes d'une carte, nommées — agrégat de construction de `core::Level` (`LOT-03`).
 *
 * S'écrit avec les *designated initializers* de C++20, qui rendent chaque site de construction
 * lisible sans commentaire :
 * @code
 * core::Level level(core::LevelData{.name = "village",
 *                                   .tileMap = std::move(map),
 *                                   .entry = entryPosition});
 * @endcode
 *
 * @note `tileMap` n'a **volontairement** pas de valeur par défaut : `core::TileMap` n'est pas
 *       constructible par défaut, si bien que l'omettre est une **erreur de compilation** et non
 *       une grille vide silencieuse. Tous les autres champs ont un défaut utile.
 */
struct LevelData {
    /// Nom de la carte.
    std::string name;
    /// Grille de tuiles typées. Sans défaut : voir la note ci-dessus.
    ///
    /// C'est la grille de **collision** de la carte — celle que consomment l'exploration et la
    /// grille de combat tactique — et elle porte aussi l'entrée. Le format ne connaît qu'elle sur
    /// ce point : une couche `collision` déclarée à côté est refusée au chargement, précisément
    /// pour qu'il n'existe jamais deux grilles à tenir d'accord (`EX-LVL-016`).
    TileMap tileMap;
    /// Couches de tuiles de la carte (`LOT-04`), dans leur ordre de superposition.
    ///
    /// La **première** est `tileMap` ci-dessus, promue par le chargeur : de rôle `Collision` quand
    /// la carte déclare des couches visibles, `Legacy` quand elle n'en déclare aucune (une grille
    /// plate `version: 2`, qui vaut alors décor **et** collision). Un consommateur boucle donc sur
    /// `layers` sans cas particulier. Vide seulement pour un `Level` construit **directement**,
    /// sans passer par le chargeur — le rendu retombe alors sur `tileMap`.
    std::vector<TileLayer> layers;
    /// Entités placées sur la carte (`LOT-04`) : PNJ, coffres, panneaux, portails, déclencheurs.
    std::vector<MapEntity> entities;
    /// Point d'arrivée par défaut (case `Entry`).
    GridPosition entry{};
    /// Pièces assignées par case (`EX-EDIT-043`), prioritaires sur la table d'apparence du lieu.
    std::vector<TileTextureOverride> textureOverrides;
};

/**
 * @brief Carte complète en mémoire.
 *
 * Assemblée par le chargeur (après parsing et validation) puis lue par l'exploration, le combat et
 * le rendu. Donnée pure (`EX-ARCH-011`, `EX-LVL-002`) : aucune dépendance rendu ni fichier.
 */
class Level {
public:
    /**
     * @brief Construit une carte à partir de ses composantes nommées.
     * @param data Composantes de la carte (déplacées).
     */
    explicit Level(LevelData data)
        : _name(std::move(data.name)),
          _tileMap(std::move(data.tileMap)),
          _layers(std::move(data.layers)),
          _entities(std::move(data.entities)),
          _entry(data.entry),
          _textureOverrides(std::move(data.textureOverrides)) {}

    /// @return Le nom de la carte.
    [[nodiscard]] const std::string& name() const noexcept {
        return _name;
    }

    /// @return La grille de tuiles de la carte.
    [[nodiscard]] const TileMap& tileMap() const noexcept {
        return _tileMap;
    }

    /// @return Les couches de tuiles de la carte (`LOT-04`), dans leur ordre de superposition,
    /// la grille de collision en tête (voir `LevelData::layers`).
    [[nodiscard]] const std::vector<TileLayer>& layers() const noexcept {
        return _layers;
    }

    /// @return Les entités placées sur la carte (`LOT-04`).
    [[nodiscard]] const std::vector<MapEntity>& entities() const noexcept {
        return _entities;
    }

    /// @return Le point d'arrivée par défaut.
    [[nodiscard]] GridPosition entry() const noexcept {
        return _entry;
    }

    /// @return Les pièces assignées par case (`EX-EDIT-043`).
    [[nodiscard]] const std::vector<TileTextureOverride>& textureOverrides() const noexcept {
        return _textureOverrides;
    }

private:
    std::string _name;
    TileMap _tileMap;
    std::vector<TileLayer> _layers;
    std::vector<MapEntity> _entities;
    GridPosition _entry;
    std::vector<TileTextureOverride> _textureOverrides;
};

}  // namespace core
