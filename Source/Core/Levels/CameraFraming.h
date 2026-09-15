// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

/**
 * @file Core/Levels/CameraFraming.h
 * @brief Mode de cadrage de caméra porté par le niveau (`EX-LVL-006`, `EX-REN-016`, LOT-64).
 */

namespace core {

/**
 * @brief Mode de cadrage de caméra d'un niveau (`EX-REN-016`) : décision de **conception**, pas
 *        une règle déduite de ses dimensions.
 */
enum class CameraFramingMode {
    WholeLevel,  ///< Le niveau entier tient dans le cadre (`EX-REN-013`).
    PerRoom,     ///< Cadrage salle par salle, coupure nette au franchissement (`EX-REN-015`).
    Follow,      ///< Caméra de suivi du personnage (zone morte, anticipation, bornage).
};

/// Nom JSON du mode (symétrique à `parseCameraFramingMode`), pour la sérialisation.
[[nodiscard]] std::string_view cameraFramingModeName(CameraFramingMode mode) noexcept;

/// Reconnaît le nom JSON d'un mode ; `std::nullopt` si non reconnu (mode inconnu, `EX-LVL-004`).
[[nodiscard]] std::optional<CameraFramingMode> parseCameraFramingMode(
    std::string_view name) noexcept;

/**
 * @brief Taille de salle par défaut du mode *par salle*, en cases.
 *
 * Mêmes valeurs historiques que `hmi::RoomGrid` (`LOT-32`), désormais une **valeur par défaut**
 * plutôt que la seule vérité (`tache-01`) : un niveau peut la personnaliser. Dupliquées ici plutôt
 * que lues depuis `hmi::RoomGrid` : `Core` ne dépend jamais de `HMI` (`EX-ARCH-011`).
 */
inline constexpr int DEFAULT_ROOM_WIDTH_TILES = 24;
inline constexpr int DEFAULT_ROOM_HEIGHT_TILES = 14;

/**
 * @brief Rectangle de caméra dessiné à la main par le level designer (mode *par salle*), en cases.
 *
 * Quand `CameraFramingConfig::zones` en porte au moins un, il **remplace entièrement** le
 * découpage automatique en grille uniforme (`roomWidthTiles`/`roomHeightTiles`) : c'est ce qui
 * permet de mélanger plusieurs tailles de caméra dans un même niveau, chaque zone étant une
 * salle de la taille voulue par l'auteur plutôt qu'une cellule d'une grille régulière.
 */
struct CameraZone {
    int x = 0;
    int y = 0;
    int width = 1;
    int height = 1;

    [[nodiscard]] bool operator==(const CameraZone&) const noexcept = default;
};

/**
 * @brief Cadrage de caméra résolu d'un niveau : le mode retenu et ses paramètres.
 *
 * `roomWidthTiles`/`roomHeightTiles` ont deux usages selon le mode : en `PerRoom`, la taille de
 * chaque salle du découpage **automatique** en grille (ignorée si `zones` est non vide) ; en
 * `Follow`, la taille fixe de la zone visible par la caméra de suivi. `std::nullopt` signifie
 * « taille par défaut » (`DEFAULT_ROOM_WIDTH_TILES`/`DEFAULT_ROOM_HEIGHT_TILES`), jamais une taille
 * de zéro case. Sans objet dans les deux cas pour `WholeLevel`.
 */
struct CameraFramingConfig {
    CameraFramingMode mode = CameraFramingMode::WholeLevel;
    std::optional<int> roomWidthTiles;
    std::optional<int> roomHeightTiles;
    /**
     * Zones de caméra dessinées à la main (mode `PerRoom` uniquement) : si non vide, remplace
     * entièrement le découpage automatique en grille — la **première** zone de la liste qui
     * contient la position du personnage est active (l'ordre porte donc la priorité en cas de
     * chevauchement, même convention que la superposition des décors) ; si aucune zone ne la
     * contient, repli sur le cadrage niveau entier (jamais un état indéfini).
     */
    std::vector<CameraZone> zones;

    [[nodiscard]] bool operator==(const CameraFramingConfig&) const noexcept = default;
};

/**
 * @brief Résout le cadrage effectif d'un niveau à partir du champ **déclaré** (peut être absent).
 *
 * Unique endroit incarnant la règle de repli (`EX-LVL-006`) : un niveau **sans** mode déclaré est
 * cadré sur le niveau entier s'il tient dans une salle de taille par défaut, et par salle sinon.
 * Un mode déclaré explicitement est retourné tel quel, sans substitution.
 * @param declared    Cadrage déclaré dans le fichier, ou `std::nullopt` si le champ est absent.
 * @param levelWidth  Largeur du niveau, en cases (> 0).
 * @param levelHeight Hauteur du niveau, en cases (> 0).
 * @return Le cadrage effectif à appliquer.
 */
[[nodiscard]] CameraFramingConfig resolveCameraFraming(
    const std::optional<CameraFramingConfig>& declared, int levelWidth, int levelHeight) noexcept;

/**
 * @brief Valide un cadrage déclaré (`EX-LVL-004`) : taille de vue nulle/négative ou supérieure au
 *        niveau, zone hors bornes ou de taille nulle, ou paramètre étranger au mode retenu
 *        (`roomWidthTiles`/`roomHeightTiles` hors `PerRoom`/`Follow`, `zones` hors `PerRoom`).
 * @param config      Cadrage déclaré à valider.
 * @param levelWidth  Largeur du niveau, en cases.
 * @param levelHeight Hauteur du niveau, en cases.
 * @return Un message d'erreur exploitable nommant le champ fautif, ou `std::nullopt` si valide.
 */
[[nodiscard]] std::optional<std::string> validateCameraFramingConfig(
    const CameraFramingConfig& config, int levelWidth, int levelHeight);

}  // namespace core
