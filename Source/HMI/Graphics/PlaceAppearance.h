// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/TileType.h"

/**
 * @file HMI/Graphics/PlaceAppearance.h
 * @brief Ce qu'un **lieu** met sur une case : la table qui traduit un type de tuile en pièce de la
 *        planche de l'atelier des textures (`LOT-92`, `LOT-09`).
 *
 * ## La règle, décidée le 17 septembre 2026 (*décision de l'auteur*)
 *
 * - **Le sol** vient du **type de tuile** de la couche « sol », traduit par cette table : un sable
 *   donne `sand`, `sand-2` ou `sand-3`, la variante choisie par la case elle-même, toujours la
 *   même pour la même case. Le sol est dense — sept mille cases pour le Colisée —, et le nommer à
 *   la case rendrait la carte illisible et son tracé interminable.
 * - **Le relief** (murs, torches, bancs, arches, gradins) nomme sa pièce **à la case**, par
 *   l'assignation de texture que l'éditeur du `LOT-11` sait déjà poser
 *   (`core::TileTextureOverride`). Le relief est rare et voulu : c'est là que l'auteur décide.
 *   À défaut d'assignation, le type de la case de décor donne la pièce par cette même table.
 *
 * Aucune évolution du format de niveau : ni champ nouveau, ni `version: 4`.
 */

namespace core {
struct JsonDocument;
}

namespace hmi {

/// @brief Catégorie d'échec de lecture (même esprit que `hmi::SkinCatalogError`).
enum class PlaceAppearanceError : std::uint8_t {
    None,
    FileNotFound,
    ParseError,
    UnsupportedVersion,
    MalformedStructure,
};

struct PlaceAppearanceResult;

/**
 * @brief La table d'un lieu : type de tuile → pièces, pour le sol et pour le relief.
 *
 * Logique **pure** (aucun Qt, aucun GPU, `EX-NFR-010`). Aucune lecture ne lève (`EX-NFR-040`) : un
 * fichier absent donne une table vide, et une case sans pièce ne dessine rien plutôt que de tomber
 * sur un damier sur sept mille cases.
 */
class PlaceAppearance {
public:
    static constexpr int FORMAT_VERSION = 1;

    [[nodiscard]] static PlaceAppearanceResult loadFromString(std::string_view json);
    [[nodiscard]] static PlaceAppearanceResult loadFromFile(const std::filesystem::path& path);

    /// @return L'identifiant du lieu (`coliseum`), vide pour une table vide.
    [[nodiscard]] const std::string& place() const noexcept {
        return _place;
    }

    /**
     * @brief La pièce de sol d'une case.
     *
     * La variante est choisie par la case : `(colonne × 7 + ligne × 13) % nombre de variantes`.
     * Un tirage aléatoire ferait scintiller le sol d'une image à l'autre ; un compteur ferait
     * dépendre le sol de l'ordre de parcours.
     * @return Le nom de la pièce (`sand-2`), vide si le type n'a aucune pièce dans ce lieu.
     */
    [[nodiscard]] std::string_view floorPiece(core::TileType type, core::GridPosition cell) const;

    /// @brief La pièce de relief d'une case, même règle de variante que le sol.
    [[nodiscard]] std::string_view reliefPiece(core::TileType type, core::GridPosition cell) const;

    /// @return Tous les noms de pièce que la table peut rendre, triés, sans doublon — ce que le
    ///         rendu doit charger.
    [[nodiscard]] std::vector<std::string> pieces() const;

    /// @return Vrai si la table ne traduit aucun type.
    [[nodiscard]] bool empty() const noexcept {
        return _floors.empty() && _relief.empty();
    }

private:
    using Table = std::map<core::TileType, std::vector<std::string>>;

    [[nodiscard]] static PlaceAppearanceResult fromDocument(const core::JsonDocument& document);

    std::string _place;
    Table _floors;
    Table _relief;
};

/// @brief Résultat d'une lecture : la table, et ce qui a échoué.
struct PlaceAppearanceResult {
    PlaceAppearance appearance;
    PlaceAppearanceError error = PlaceAppearanceError::None;
    /// Message technique, vide en cas de succès. Pour les journaux et les tests.
    std::string message;

    [[nodiscard]] bool ok() const noexcept {
        return error == PlaceAppearanceError::None;
    }
};

}  // namespace hmi
