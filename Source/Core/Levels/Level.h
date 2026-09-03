// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "Core/Levels/CameraFraming.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Plane.h"
#include "Core/Levels/TileMap.h"

/**
 * @file Core/Levels/Level.h
 * @brief Niveau chargé : grille de tuiles, entrée/sortie et mécanismes.
 */

namespace core {

/**
 * @brief Liaison d'un **interrupteur** à une **porte**, par positions résolues.
 *
 * Dans le fichier, la liaison est exprimée par identifiant (`switch.id` ↔ `door.opensWith`) ;
 * le chargeur la résout en positions de grille. Le **comportement** (l'interrupteur ou la plaque
 * de pression ouvre la porte) est résolu chaque pas fixe par `core::MechanismController`.
 */
struct Mechanism {
    GridPosition switchPosition;
    GridPosition doorPosition;
};

/**
 * @brief Texture assignée explicitement à **une case précise**, prioritaire sur le skin de son
 *        type (`EX-EDIT-043`, LOT-42).
 *
 * Même patron que `Mechanism` : vecteur
 * annexe de `Level`, keyé par position, `TileMap` ne portant qu'un `TileType` par case. Le nom
 * d'asset est une simple chaîne : `Core` ne vérifie pas son existence (`EX-NFR-011`), un override
 * pointant un fichier absent reste un niveau valide.
 */
struct TileTextureOverride {
    GridPosition position;
    std::string assetName;
};

/**
 * @brief Niveau complet en mémoire : nom, grille de tuiles, entrée/sortie et mécanismes.
 *
 * Assemblé par le chargeur (après parsing et validation) puis lu par le rendu et, à terme, le
 * gameplay. Donnée pure (`EX-ARCH-011`, `EX-LVL-002`) : aucune dépendance rendu ni fichier.
 */
/**
 * @brief Composantes d'un niveau, nommées — agrégat de construction de `core::Level` (`LOT-03`).
 *
 * Le constructeur de `Level` a compté jusqu'à **19 paramètres positionnels**. Le `LOT-01` en a
 * retiré huit avec le gameplay de plateforme, mais la dette de fond restait : deux
 * `std::optional<std::string>` voisins (`background`, `skinSet`) s'intervertissent sans que le
 * compilateur bronche, et le RPG s'apprête à rajouter des champs (couches de tuiles, entités,
 * connexions de carte, zones de rencontre, points d'apparition). Solder maintenant, alors que la
 * liste est au plus court, coûte le minimum.
 *
 * S'écrit avec les *designated initializers* de C++20, qui rendent chaque site de construction
 * lisible sans commentaire :
 * @code
 * core::Level level(core::LevelData{.name = "village",
 *                                   .tileMap = std::move(map),
 *                                   .entry = entryPosition,
 *                                   .exit = exitPosition});
 * @endcode
 *
 * @note `tileMap` n'a **volontairement** pas de valeur par défaut : `core::TileMap` n'est pas
 *       constructible par défaut, si bien que l'omettre est une **erreur de compilation** et non
 *       une grille vide silencieuse. Tous les autres champs ont un défaut utile.
 */
struct LevelData {
    /// Nom du niveau.
    std::string name;
    /// Grille de tuiles typées. Sans défaut : voir la note ci-dessus.
    TileMap tileMap;
    /// Position d'apparition (case `Entry`).
    GridPosition entry{};
    /// Position de sortie (case `Exit`).
    GridPosition exit{};
    /// Liaisons interrupteur↔porte résolues.
    std::vector<Mechanism> mechanisms;
    /// Nom de l'asset de fond (`EX-REN-044`), absent si aucun. Une chaîne, jamais un handle de
    /// texture : `Core` ignore tout du rendu.
    std::optional<std::string> background;
    /// Nom du jeu de skins (`EX-EDIT-024`), absent pour le jeu par défaut.
    std::optional<std::string> skinSet;
    /// Textures assignées par instance (`EX-EDIT-043`), prioritaires sur le skin de leur type.
    std::vector<TileTextureOverride> textureOverrides;
    /// Cadrage de caméra **résolu** (`EX-LVL-006`) : déjà passé par `resolveCameraFraming` côté
    /// chargeur, jamais un champ brut « peut-être absent ». Le défaut (`WholeLevel`) est légitime
    /// pour un niveau construit directement, cohérent avec un petit niveau tenant dans une salle.
    CameraFramingConfig cameraFraming;
    /// Plans picturaux (`EX-DEC-040`), dans leur ordre de superposition.
    std::vector<Plane> planes;
    /// `true` si la parallaxe des plans s'applique (`EX-DEC-043`) ; le mode de cadrage peut la
    /// neutraliser par-dessus ce drapeau.
    bool parallaxEnabled = true;
};

class Level {
public:
    /**
     * @brief Construit un niveau à partir de ses composantes nommées.
     * @param data Composantes du niveau (déplacées).
     */
    explicit Level(LevelData data)
        : _name(std::move(data.name)),
          _tileMap(std::move(data.tileMap)),
          _entry(data.entry),
          _exit(data.exit),
          _mechanisms(std::move(data.mechanisms)),
          _background(std::move(data.background)),
          _skinSet(std::move(data.skinSet)),
          _textureOverrides(std::move(data.textureOverrides)),
          _cameraFraming(std::move(data.cameraFraming)),
          _planes(std::move(data.planes)),
          _parallaxEnabled(data.parallaxEnabled) {}

    /// @return Le nom du niveau.
    [[nodiscard]] const std::string& name() const noexcept {
        return _name;
    }

    /// @return La grille de tuiles du niveau.
    [[nodiscard]] const TileMap& tileMap() const noexcept {
        return _tileMap;
    }

    /// @return La position d'apparition.
    [[nodiscard]] GridPosition entry() const noexcept {
        return _entry;
    }

    /// @return La position de sortie.
    [[nodiscard]] GridPosition exit() const noexcept {
        return _exit;
    }

    /// @return Les liaisons de mécanismes du niveau.
    [[nodiscard]] const std::vector<Mechanism>& mechanisms() const noexcept {
        return _mechanisms;
    }

    /// @return Le nom de l'asset de fond du niveau (`EX-REN-044`), absent si aucun n'est
    /// configuré. Une chaîne, jamais un handle : `Core` n'a pas accès au dossier d'assets.
    [[nodiscard]] const std::optional<std::string>& background() const noexcept {
        return _background;
    }

    /// @return Le nom du jeu de skins du niveau (`EX-EDIT-024`), absent si le niveau utilise le
    /// jeu par défaut.
    [[nodiscard]] const std::optional<std::string>& skinSet() const noexcept {
        return _skinSet;
    }

    /// @return Les textures assignées par instance du niveau (`EX-EDIT-043`).
    [[nodiscard]] const std::vector<TileTextureOverride>& textureOverrides() const noexcept {
        return _textureOverrides;
    }

    /// @return Les **plans picturaux** du niveau (`EX-DEC-040`), dans leur ordre de superposition.
    [[nodiscard]] const std::vector<Plane>& planes() const noexcept {
        return _planes;
    }

    /// @return `true` si la **parallaxe** des plans s'applique dans ce niveau (`EX-DEC-043`).
    /// Le mode de cadrage peut la neutraliser malgré ce drapeau : c'est une règle du moteur, pas
    /// une propriété du niveau (`hmi::planeParallaxActive`).
    [[nodiscard]] bool parallaxEnabled() const noexcept {
        return _parallaxEnabled;
    }

    /// @return Le cadrage de caméra **résolu** du niveau (`EX-LVL-006`), jamais un champ optionnel
    /// "peut-être absent" : la règle de repli (`resolveCameraFraming`) a déjà été appliquée par le
    /// chargeur avant de construire ce `Level`.
    [[nodiscard]] const CameraFramingConfig& cameraFraming() const noexcept {
        return _cameraFraming;
    }

private:
    std::string _name;
    TileMap _tileMap;
    GridPosition _entry;
    GridPosition _exit;
    std::vector<Mechanism> _mechanisms;
    std::optional<std::string> _background;
    std::optional<std::string> _skinSet;
    std::vector<TileTextureOverride> _textureOverrides;
    CameraFramingConfig _cameraFraming;
    std::vector<Plane> _planes;
    bool _parallaxEnabled = true;
};

}  // namespace core
