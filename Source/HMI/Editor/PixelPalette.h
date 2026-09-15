// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "HMI/Graphics/TextureLoader.h"

/**
 * @file HMI/Editor/PixelPalette.h
 * @brief Palette de projet persistée, extraction depuis un asset et contrainte de couleur
 *        (`LOT-54` TACHE-07, `EX-REN-042`).
 *
 * **Donnée d'auteur**, distincte des jetons de design (`HMI/Interface/DesignTokens.h`) : cette
 * palette habille le **contenu du jeu**, les jetons habillent l'**application** — les deux ne se
 * mélangent jamais (epic.md, décision de cadrage).
 */

namespace hmi {

/// Une entrée nommée de la palette de projet.
struct PixelPaletteEntry {
    std::string name;
    std::uint32_t color = 0xFF000000u;  ///< `R8G8B8A8_UNORM` (LOT-54 TACHE-01).

    [[nodiscard]] friend bool operator==(const PixelPaletteEntry&,
                                         const PixelPaletteEntry&) noexcept = default;
};

/**
 * @brief Palette de couleurs nommées du projet, lue/écrite dans `Assets/palettes.json`.
 *
 * Logique **pure** (aucune dépendance Qt/GPU au-delà de `std::filesystem`/JSON), testable en
 * isolation (`EX-NFR-010`). Aucune lecture ni écriture ne lève d'exception (`EX-NFR-040`) : un
 * fichier **absent** produit une palette **vide**, traitée comme un état de départ légitime — pas
 * une erreur à signaler — et un fichier malformé ignore l'entrée fautive plutôt que d'abandonner
 * toute la palette.
 */
class PixelPalette {
public:
    /// Version du format écrite dans le fichier.
    static constexpr int FORMAT_VERSION = 1;

    /// @return La palette lue depuis @p json ; vide (et journalisée) si malformée. Ne lève jamais.
    [[nodiscard]] static PixelPalette loadFromString(std::string_view json);

    /// @return La palette lue depuis @p path ; vide si le fichier est absent ou illisible (état de
    ///         départ légitime, pas une erreur). Ne lève jamais.
    [[nodiscard]] static PixelPalette loadFromFile(const std::filesystem::path& path);

    /// @return La représentation JSON de la palette, relisible par `loadFromString`.
    [[nodiscard]] std::string toJsonString() const;

    /// Écrit la palette dans un fichier, en créant les dossiers parents au besoin.
    /// @return true si l'écriture a réussi. Ne lève jamais.
    [[nodiscard]] bool saveToFile(const std::filesystem::path& path) const;

    /// @return Les entrées de la palette, dans l'ordre d'affichage/écriture.
    [[nodiscard]] const std::vector<PixelPaletteEntry>& entries() const noexcept {
        return _entries;
    }

    /// Ajoute une entrée en fin de palette.
    void add(std::string name, std::uint32_t color);
    /// Retire l'entrée @p index. @return true si l'index était valide.
    bool removeAt(std::size_t index);
    /// Renomme l'entrée @p index. @return true si l'index était valide.
    bool renameAt(std::size_t index, std::string newName);
    /// Déplace l'entrée @p from à la position @p to (réordonnancement). @return true si les deux
    /// index étaient valides.
    bool moveEntry(std::size_t from, std::size_t to);

private:
    std::vector<PixelPaletteEntry> _entries;
};

/// Une couleur extraite, avec son nombre d'occurrences dans l'image.
struct PixelPaletteExtractionEntry {
    std::uint32_t color = 0;
    int count = 0;
};

/**
 * @brief Couleurs effectivement présentes dans une image, dans un ordre **déterministe**.
 *
 * Fonction **pure**. Un pixel totalement transparent (alpha nul) n'est **jamais** une couleur —
 * c'est la gomme, pas une entrée de palette (même règle que `hmi::constrainToPalette` ci-dessous) :
 * une image entièrement transparente produit donc une liste vide. L'ordre (première apparition,
 * balayage ligne par ligne) est stable d'un appel à l'autre sur la même image.
 * @param image Image consultée.
 * @return Les couleurs présentes, chacune avec son nombre d'occurrences.
 */
[[nodiscard]] std::vector<PixelPaletteExtractionEntry> extractPalette(const DecodedImage& image);

/**
 * @brief Couleur de @p palette la plus proche de @p color (mode « contraindre à la palette »).
 *
 * Fonction **pure**, distance euclidienne sur les seuls canaux RVB (l'alpha d'origine est toujours
 * préservé, voir ci-dessous). Le départage entre couleurs équidistantes retient la **première**
 * rencontrée dans @p palette — déterministe et stable, pour qu'un même geste sur une même image
 * produise toujours le même résultat.
 *
 * Un pixel **totalement transparent** (alpha nul) est renvoyé **inchangé** : ce n'est pas une
 * couleur mais l'état « gomme », qui doit rester accessible même en mode contraint. Une **palette
 * vide** laisse également la couleur inchangée plutôt que de produire une couleur indéfinie.
 * @param color   Couleur posée par le geste courant.
 * @param palette Couleurs de la palette (généralement `PixelPalette::entries()`, projetées sur
 *                leur seul champ `color`).
 * @return La couleur contrainte, alpha d'origine préservé.
 */
[[nodiscard]] std::uint32_t nearestPaletteColor(std::uint32_t color,
                                                const std::vector<std::uint32_t>& palette) noexcept;

}  // namespace hmi
