// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <string>

#include "Core/Ecs/Components/Sprite.h"  // core::AtlasRegion
#include "Core/Levels/TileType.h"
#include "HMI/Graphics/RenderMode.h"
#include "HMI/Graphics/SkinCatalog.h"

/**
 * @file HMI/Editor/PaletteAppearance.h
 * @brief Choix de la vignette d'un type de tuile dans la palette (`EX-EDIT-027`).
 */

namespace hmi {

/// D'où provient la vignette affichée par la palette pour un type de tuile.
enum class PaletteThumbnailSource {
    /// Couleur plate de l'atlas : mode Physique, ou repli si l'atlas fait foi.
    Atlas,
    /// Fichier de skin, dans le dossier `Assets/Skins/`.
    Skin,
    /// Damier magenta : le type n'est pas encore habillé (`EX-NFR-040`).
    MissingTexture,
};

/// Vignette à afficher pour un type de tuile : d'où la lire, et quelle région y prendre.
struct PaletteThumbnail {
    PaletteThumbnailSource source = PaletteThumbnailSource::Atlas;
    /// Région à échantillonner, en pixels, dans la source désignée.
    core::AtlasRegion region{};
    /// Fichier à décoder si `source` vaut `Skin` ; vide sinon.
    std::string asset;
    /// `true` si l'image doit être détourée à la silhouette du type avant affichage
    /// (pentes et arrondis, `LOT-42` TACHE-03).
    bool masked = false;

    [[nodiscard]] friend bool operator==(const PaletteThumbnail& lhs, const PaletteThumbnail& rhs) {
        return lhs.source == rhs.source && lhs.region.x == rhs.region.x &&
               lhs.region.y == rhs.region.y && lhs.region.width == rhs.region.width &&
               lhs.region.height == rhs.region.height && lhs.asset == rhs.asset &&
               lhs.masked == rhs.masked;
    }
};

/**
 * @brief Choisit la vignette à afficher pour un type de tuile dans la palette.
 *
 * INVARIANT : ce que la palette montre est ce que le canevas posera. Dès lors que le canevas
 * affiche des textures, une palette qui montrerait encore des pastilles de couleur ferait choisir
 * « la tuile violette » à l'aveugle.
 *
 * Applique **exactement** la même priorité que le canevas (`hmi::resolveTileAppearance`) — mode
 * Physique → couleur plate ; sinon skin assigné ; sinon damier. Les deux ne peuvent diverger que
 * si l'on duplique la décision, d'où cette fonction unique plutôt qu'un choix pris dans le widget.
 *
 * Un type en mode `bitmask16` rend la case **représentative** de la planche (l'intérieur plein) :
 * la planche entière serait illisible en vignette, et la case zéro — la tuile isolée — serait un
 * cas particulier peu représentatif du motif.
 *
 * Logique **pure** (aucune dépendance Qt/GPU), testable en isolation (`EX-NFR-010`).
 * @param mode           Mode de rendu courant (`EX-REN-046`).
 * @param type           Type de tuile dont on veut la vignette.
 * @param physicalRegion Région d'atlas du type en mode Physique (`hmi::regionForTile`).
 * @param catalog        Catalogue des skins, ou `nullptr` si aucun n'est chargé.
 * @param setName        Jeu de skins courant ; vide pour le jeu par défaut du catalogue.
 * @return La vignette à afficher.
 */
[[nodiscard]] PaletteThumbnail paletteThumbnail(RenderMode mode, core::TileType type,
                                                const core::AtlasRegion& physicalRegion,
                                                const SkinCatalog* catalog,
                                                std::string_view setName);

}  // namespace hmi
