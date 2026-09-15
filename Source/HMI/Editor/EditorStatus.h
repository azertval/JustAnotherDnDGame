// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "Core/Levels/CameraFraming.h"
#include "Core/Levels/GridPosition.h"
#include "HMI/Editor/EditorTool.h"
#include "HMI/Editor/PixelTool.h"

/**
 * @file HMI/Editor/EditorStatus.h
 * @brief Choix du contenu de la barre d'état de l'éditeur : zones permanentes et aide
 *        contextuelle à l'outil actif (`LOT-57` TACHE-01, `EX-IHM-060`).
 */

namespace hmi {

class Localization;

/// État affiché pour un niveau en cours d'édition (barre d'état, `EX-EDIT-013`/`EX-EDIT-012`).
struct LevelStatusInfo {
    std::string name;                               ///< Nom du niveau ouvert.
    bool dirty = false;                             ///< Modifications non enregistrées.
    EditorTool tool = EditorTool::Paint;            ///< Outil d'édition actif.
    std::optional<core::GridPosition> hoveredCell;  ///< Case survolée, si le curseur est dessus.
    float zoom = 1.0f;                              ///< Facteur de zoom courant.
    /// Mode de cadrage courant du niveau (`EX-EDIT-028`, LOT-64) : ce qui informe est permanent.
    core::CameraFramingMode cameraFraming = core::CameraFramingMode::WholeLevel;
};

/**
 * @brief Ce qu'un **plan pictural** ajoute à la barre d'état, quand le canevas en peint un
 *        (`EX-EDIT-046`, `EX-NFR-043`, `LOT-69` TACHE-08).
 *
 * Renseigné en plus de `hmi::PixelEditStatusInfo`, jamais à sa place : peindre un plan, c'est
 * peindre sur le **même** canevas, avec les mêmes outils, la même couleur courante et le même
 * historique. Seules trois informations manquent — la résolution, la densité et le poids mémoire —
 * et elles n'ont de sens que pour un plan : un asset de pixel art n'a pas de densité, et son poids
 * est négligeable par construction.
 */
struct PlaneEditStatusInfo {
    int widthPixels = 0;           ///< Largeur de l'image du plan, en pixels.
    int heightPixels = 0;          ///< Hauteur de l'image du plan, en pixels.
    int pixelsPerUnit = 16;        ///< Densité déclarée (`EX-DEC-041`) : 4, 8 ou 16.
    std::size_t textureBytes = 0;  ///< Poids de la texture, en octets (`EX-NFR-043`).
};

/// État affiché pour un asset en cours d'édition dans l'atelier pixel art (barre d'état, `LOT-54`
/// TACHE-04, `EX-IHM-060`).
struct PixelEditStatusInfo {
    std::string assetName;              ///< Nom de l'asset ouvert, vide si aucun (TACHE-05).
    bool dirty = false;                 ///< Modifications non enregistrées.
    PixelTool tool = PixelTool::Brush;  ///< Outil de canevas actif.
    std::optional<std::pair<int, int>> hoveredPixel;  ///< Pixel survolé, si le curseur est dessus.
    int zoom = 1;                              ///< Facteur de zoom entier courant (TACHE-03).
    std::uint32_t currentColor = 0xFF000000u;  ///< Couleur courante (R8G8B8A8_UNORM).
    bool paletteConstrained = false;           ///< Mode « contraindre à la palette » (TACHE-07).
    /// Renseigné dans l'espace « Plans » seulement : le canevas peint alors un plan pictural.
    std::optional<PlaneEditStatusInfo> plane;
};

/**
 * @brief Contexte d'édition dont la barre d'état décide l'affichage.
 *
 * `level` et `pixelEdit` sont **mutuellement exclusifs en pratique** : au plus un contexte
 * d'édition est actif à la fois (`MainWindow` ne peuple que celui qui correspond au widget
 * focalisé). `pixelEdit` étend le modèle pour l'atelier pixel art (`LOT-54` TACHE-04) sans en créer
 * un second — même barre, même règle de restauration après message transitoire.
 */
struct EditorStatusContext {
    std::optional<LevelStatusInfo> level;
    std::optional<PixelEditStatusInfo> pixelEdit;
};

/// Lignes à afficher pour la barre d'état de l'éditeur, à un instant donné.
struct EditorStatusLines {
    /// Zones permanentes, dans l'ordre d'affichage : niveau/asset, modifications non enregistrées,
    /// outil actif, case/pixel survolé, zoom, couleur courante (atelier pixel art seulement), et
    /// une septième zone qui porte le cadrage de caméra en contexte niveau (`EX-EDIT-028`) et la
    /// **fiche du plan** — résolution, densité, poids mémoire — en contexte plan (`EX-NFR-043`).
    /// Les deux ne coexistent jamais, et une huitième zone resterait vide dans les deux cas. Une
    /// zone vide (chaîne vide) quand l'information n'a pas de sens pour le contexte courant —
    /// jamais de libellé de remplacement.
    std::vector<std::string> permanent;
    /// Aide contextuelle à l'outil actif ; vide hors contexte de niveau ou d'atelier.
    std::string help;
};

/**
 * @brief Décide le contenu de la barre d'état de l'éditeur pour @p context.
 *
 * Fonction **pure** (`EX-NFR-010`) : ne lit que ce qu'on lui passe, aucune dépendance Qt/GPU — même
 * patron que `hmi::gameHudLines` (`LOT-52`). Le rendu (`MainWindow`) n'a plus qu'à afficher ce que
 * cette fonction décide, et à la rappeler après l'expiration d'un message transitoire pour
 * restaurer l'aide affichée.
 * @param context      Contexte d'édition courant ; `context.level` absent (aucun niveau ouvert)
 *                      produit des zones et une aide vides.
 * @param localization Catalogue de traduction (`EX-REN-033`) — aucune chaîne en dur.
 * @return Les zones permanentes et l'aide contextuelle à afficher.
 */
[[nodiscard]] EditorStatusLines editorStatusLines(const EditorStatusContext& context,
                                                  const Localization& localization);

}  // namespace hmi
