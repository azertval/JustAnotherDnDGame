// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

#include "Core/Levels/Level.h"

/**
 * @file Core/Levels/LevelLoader.h
 * @brief Chargement d'un niveau depuis le format JSON (liste de tuiles-objets).
 */

namespace core {

/**
 * @brief Catégorie d'échec de chargement/validation d'un niveau (LOT-15, `EX-EDIT-012`).
 *
 * Complète le message technique (`LevelLoadResult::error`) d'un code **programmatique**, pour
 * qu'un appelant (l'éditeur) puisse traduire une erreur en message non-codeur sans dépendre du
 * texte exact du message — une correspondance de sous-chaînes s'y casserait silencieusement au
 * moindre changement de formulation.
 */
enum class LevelValidationError {
    None,                      ///< Pas d'erreur (chargement réussi).
    ParseError,                ///< JSON malformé, champ obligatoire manquant, ou de mauvais type.
    UnknownTileType,           ///< Type de tuile non reconnu.
    OutOfBounds,               ///< Tuile positionnée hors des dimensions déclarées.
    DuplicatePosition,         ///< Deux tuiles à la même position.
    MissingSwitchId,           ///< Interrupteur sans identifiant.
    DuplicateSwitchId,         ///< Deux interrupteurs partagent le même identifiant.
    InvalidEntryCount,         ///< Zéro ou plusieurs tuiles d'entrée (une seule attendue).
    InvalidExitCount,          ///< Zéro ou plusieurs tuiles de sortie (une seule attendue).
    UnresolvedMechanism,       ///< Porte ou danger commuté lié à un identifiant d'interrupteur
                               ///< inexistant.
    FileNotFound,              ///< Fichier de niveau introuvable sur disque.
    UnsupportedFormatVersion,  ///< `"version"` du fichier supérieure à celle gérée (`EX-LVL-005`).
    InvalidCameraFraming,      ///< Cadrage de caméra invalide (`EX-LVL-006`) : mode inconnu, taille
                               ///< de salle nulle/supérieure au niveau, ou paramètre étranger au
                               ///< mode retenu.
};

/**
 * @brief Version courante du format de niveau JSON (`EX-LVL-005`).
 *
 * Écrite par `LevelWriter` dans le champ racine `"version"`. Un fichier sans ce champ est lu
 * comme la version initiale (0), sans erreur ni avertissement — rétrocompatibilité des niveaux
 * antérieurs à ce champ (`LOT-44`). Une version supérieure à celle-ci est une erreur exploitable
 * (`LevelValidationError::UnsupportedFormatVersion`), pas une lecture au mieux.
 *
 * Version 2 (`LOT-64`) : ajout du champ optionnel `"cameraFraming"` (`EX-LVL-006`). Un fichier de
 * version antérieure, sans ce champ, se charge sans erreur -- la règle de repli
 * (`core::resolveCameraFraming`) reproduit exactement le comportement historique.
 *
 * Version 3 (`LOT-04`) : **couches de tuiles** (`"layers"`, `core::TileLayer`) et **entités**
 * (`"entities"`, `core::MapEntity`), les deux optionnels. `"layers"` ne porte que les couches
 * **visibles** (sol, décor) : la grille de collision d'une carte est son tableau racine
 * `"tiles"`, celui qui porte déjà l'entrée, la sortie et les mécanismes — une couche
 * `"collision"` déclarée est refusée (`EX-LVL-016`). Cette grille racine est **promue** en couche
 * de tête, de rôle `LayerKind::Collision`, ou `LayerKind::Legacy` quand la carte ne déclare aucune
 * couche : aucun fichier existant n'a besoin d'être touché, et rien de son comportement ne change.
 *
 * Toute clé non reconnue dans une couche ou une entité est rangée dans ses propriétés libres
 * (`core::PropertyMap`) et **réémise** à l'écriture : un fichier produit par une version
 * ultérieure de l'éditeur traverse une version antérieure sans rien perdre.
 */
inline constexpr int LEVEL_FORMAT_VERSION = 3;

/**
 * @brief Résultat d'un chargement de niveau : soit un `Level`, soit une **erreur** décrite.
 *
 * En cas de succès, `level` contient le niveau, `error` est vide et `errorCode` vaut `None`. En
 * cas d'échec (récupérable, `EX-NFR-040`), `level` est vide, `error` décrit le problème de façon
 * exploitable pour les journaux/tests, et `errorCode` catégorise l'échec pour un traitement
 * programmatique (traduction non-codeur, LOT-15).
 */
struct LevelLoadResult {
    std::optional<Level> level;
    std::string error;
    LevelValidationError errorCode = LevelValidationError::None;

    /// @return true si le chargement a réussi.
    [[nodiscard]] bool ok() const noexcept {
        return level.has_value();
    }
};

/**
 * @brief Charge un niveau au format **JSON** (objet `{name, width, height, tiles}` où `tiles`
 *        est une liste d'objets `{x, y, type, …}`), `EX-LVL-001`/`EX-LVL-003`.
 *
 * Le chargement ne lève **aucune exception** vers l'appelant : toute erreur (JSON malformé,
 * champ manquant, type de tuile inconnu, tuile hors bornes, liaison de mécanisme non résolue…)
 * est renvoyée dans le `LevelLoadResult`. La **validation métier** additionnelle (unicité de
 * l'entrée/sortie, positions en double) relève de la validation du niveau.
 */
class LevelLoader {
public:
    /// @brief Charge un niveau depuis une chaîne JSON. @param json Contenu JSON. @return Résultat.
    [[nodiscard]] static LevelLoadResult loadFromString(std::string_view json);

    /// @brief Charge un niveau depuis un fichier. @param path Chemin du fichier. @return Résultat.
    [[nodiscard]] static LevelLoadResult loadFromFile(const std::filesystem::path& path);
};

}  // namespace core
