// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstddef>
#include <string>

/**
 * @file Core/Levels/Plane.h
 * @brief Plan pictural d'un niveau : image couvrant le niveau entier (`EX-DEC-040`, LOT-69).
 */

namespace core {

/**
 * @brief Position d'un plan dans l'ordonnancement de rendu, relativement au personnage
 *        (`EX-DEC-042`).
 *
 * **Deux valeurs, pas trois.** Le système de décors qu'il remplace en déclarait trois
 * (`Background`, `Decor`, `Foreground`) mais la projection vers les calques de rendu en écrasait
 * déjà deux sur la même : les trois couches de conception n'exprimaient que **deux** intentions de
 * superposition. On acte la simplification plutôt que de la reproduire.
 *
 * `Core` ne connaît pas `hmi::RenderLayer` (`EX-NFR-011`) : la projection vers les calques de rendu
 * est le travail de `HMI`.
 */
enum class PlaneDepth {
    /// Derrière les tuiles physiques.
    Behind,
    /// Devant le personnage (`EX-DEC-042`) : c'est le moyen de lecture qui distingue d'un coup
    /// d'œil le décor traversable du décor physique.
    Front,
};

/**
 * @brief Plan pictural : image couvrant **tout** le niveau, avec sa densité, sa parallaxe et sa
 *        profondeur (`EX-DEC-040` à `EX-DEC-043`).
 *
 * Donnée pure (`EX-ARCH-011`), sur le patron de `Mechanism` / `DangerLink` / `TileTextureOverride`
 * (`Source/Core/Levels/Level.h`) : vecteur annexe de `Level` / `LevelDraft`, keyé par **rang** dans
 * le vecteur. **L'ordre du vecteur est significatif** : il détermine la superposition à l'intérieur
 * d'une même profondeur.
 *
 * Un plan n'a **ni position ni rotation**, contrairement au décor-sprite qu'il remplace : il couvre
 * le niveau entier par construction. Ce qui le distingue d'un autre plan, c'est sa **densité**, sa
 * **parallaxe** et sa **profondeur**.
 *
 * Le nom de fichier est une simple chaîne : `Core` ne vérifie pas son existence (`EX-NFR-011`), un
 * plan pointant un fichier absent reste un niveau valide (`EX-LVL-004` ne le rejette pas) — c'est
 * `HMI` qui replie sur un damier visible (`EX-NFR-040`).
 */
struct Plane {
    /// Nom du fichier PNG, relatif au dossier des plans (résolu côté `HMI`).
    std::string fileName;
    /// Densité en pixels par unité monde (`EX-DEC-041`) : 16 (natif), 8 ou 4.
    int pixelsPerUnit = 16;
    /// Facteur de défilement horizontal (`EX-DEC-043`) ; 1 = solidaire du niveau.
    float parallaxX = 1.0f;
    /// Facteur de défilement vertical (`EX-DEC-043`) ; 1 = solidaire du niveau.
    float parallaxY = 1.0f;
    /// Opacité de composition, dans `[0, 1]` ; 1 = opaque.
    float opacity = 1.0f;
    /// Côté de l'ordonnancement de rendu (`EX-DEC-042`).
    PlaneDepth depth = PlaneDepth::Behind;
};

/// Densité **native** d'un plan, en pixels par unité monde — même valeur que l'échelle de rendu
/// (`hmi::Camera2D::PIXELS_PER_UNIT`), que `Core` ne peut pas inclure (`EX-NFR-011`).
inline constexpr int PLANE_NATIVE_PIXELS_PER_UNIT = 16;

/**
 * @brief Densité acceptée pour un plan (`EX-DEC-041`).
 *
 * `pixelsPerUnit` est un **entier** et non une énumération : il sert directement au calcul des
 * dimensions attendues (`width * pixelsPerUnit`), et une énumération imposerait une table de
 * conversion à chaque site d'appel. La contrainte est donc validée au **chargement**, pas portée
 * par le type.
 * @param pixelsPerUnit Densité à vérifier.
 * @return `true` si la densité vaut 4, 8 ou 16.
 */
[[nodiscard]] constexpr bool isValidPlaneDensity(int pixelsPerUnit) noexcept {
    return pixelsPerUnit == 4 || pixelsPerUnit == 8 ||
           pixelsPerUnit == PLANE_NATIVE_PIXELS_PER_UNIT;
}

/// Nombre maximal de plans par niveau (`EX-DEC-044`) : borne le coût dans le **format**, plutôt que
/// de le laisser à l'usage.
inline constexpr std::size_t MAX_PLANES_PER_LEVEL = 16;

/// Dimension maximale, en pixels, d'une texture de plan sur un axe (`EX-DEC-044`) — limite de
/// Direct3D 11 au *feature level* 11.
inline constexpr int MAX_PLANE_TEXTURE_EXTENT = 16384;

}  // namespace core
