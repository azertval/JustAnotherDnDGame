// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <string>

/**
 * @file HMI/Graphics/AssetContract.h
 * @brief Contrat de dimensions par famille d'assets graphiques et validation pure (`EX-REN-007`).
 */

namespace hmi {

/**
 * @brief Famille d'un asset graphique, qui détermine les dimensions attendues du fichier image.
 *
 * Chaque famille correspond à un usage du programme d'habillage (`LOT-40` → `LOT-55`) : ce n'est
 * pas une classification esthétique mais le **contrat** que le fichier doit respecter pour être
 * découpable par le rendu.
 */
enum class AssetFamily {
    /// Atlas historique du jeu (`Assets/atlas.png`) : grille de tuiles carrées + images du
    /// personnage sous la grille (`LOT-39`).
    Atlas,
    /// Skin d'une tuile isolée : exactement une case (`LOT-42`).
    TileSkin,
    /// Planche à raccords automatiques : grille de cases entières, au moins 4×4 (`LOT-42`).
    AutotileSheet,
    /// Image de fond de niveau : dimensions libres, l'étirement est fait au rendu (`LOT-44`).
    Background,
    /// Texture d'un objet interactif (interrupteur, porte, danger) : grille de cases (`LOT-45`).
    Object,
    /// Spritesheet du personnage : grille d'images carrées de la taille d'une case (`LOT-48`).
    CharacterSheet,
    /// Atlas de glyphes d'une police bitmap : dimensions libres, découpées par ses métriques
    /// (`LOT-52`), pas par une grille de cases.
    Font,
};

/**
 * @brief Dimensions attendues d'une famille d'assets.
 *
 * Donnée **pure**, sans dépendance fichier ni GPU : c'est la description dont `validateAsset`
 * dérive à la fois le verdict et le message d'erreur, pour qu'ils ne puissent pas diverger.
 */
struct AssetDimensionContract {
    /// Largeur exacte exigée, en pixels ; `0` si la largeur n'est pas contrainte.
    int exactWidth = 0;
    /// Hauteur exacte exigée, en pixels ; `0` si la hauteur n'est pas contrainte.
    int exactHeight = 0;
    /// Si `true`, largeur **et** hauteur doivent être des multiples entiers de la taille de tuile.
    bool multipleOfTileSize = false;
    /// Nombre minimal de cases sur chaque axe (n'a de sens qu'avec `multipleOfTileSize`).
    int minimumTiles = 1;
};

/**
 * @brief Verdict de validation d'un asset, avec son message exploitable.
 *
 * Le message est vide quand l'asset est conforme. Sinon il nomme le **fichier**, la dimension
 * **trouvée** et la dimension **attendue** : c'est ce que l'auteur de l'asset lit pour savoir quoi
 * corriger (`EX-REN-007`).
 */
struct AssetValidation {
    /// `true` si l'asset respecte le contrat de sa famille.
    bool valid = false;
    /// Message d'erreur en français, vide si `valid`.
    std::string message;
};

/**
 * @brief Nom lisible d'une famille d'assets, utilisé dans les messages de journalisation.
 * @param family Famille à nommer.
 * @return Le nom de la famille (chaîne statique, jamais nulle).
 */
[[nodiscard]] const char* assetFamilyName(AssetFamily family) noexcept;

/**
 * @brief Contrat de dimensions d'une famille d'assets.
 * @param family Famille dont on veut le contrat.
 * @return Les dimensions attendues pour cette famille.
 */
[[nodiscard]] AssetDimensionContract assetDimensionContract(AssetFamily family) noexcept;

/**
 * @brief Description en français des dimensions attendues (« 16×16 px », « multiple de 16 px… »).
 * @param contract Contrat à décrire.
 * @return La description, telle qu'elle apparaît dans le message d'un asset refusé.
 */
[[nodiscard]] std::string describeContract(const AssetDimensionContract& contract);

/**
 * @brief Valide les dimensions d'un asset décodé contre le contrat de sa famille (`EX-REN-007`).
 *
 * Fonction **pure** : elle ne lit aucun fichier et ne touche pas au GPU — c'est ce qui la rend
 * testable en isolation (`EX-NFR-010`), et ce qui impose de l'appeler entre le **décodage** et
 * l'**upload**. Sans elle, un PNG aux mauvaises dimensions ne produit aucune erreur mais des
 * artefacts de filtrage silencieux (échantillonnage *nearest*, sans mipmap, `EX-ARCH-022`).
 * @param family   Famille de l'asset, qui fixe les dimensions attendues.
 * @param fileName Nom logique du fichier, repris tel quel dans le message d'erreur.
 * @param width    Largeur décodée, en pixels.
 * @param height   Hauteur décodée, en pixels.
 * @return Le verdict, accompagné d'un message nommant le fichier, le trouvé et l'attendu si
 *         l'asset est refusé.
 */
[[nodiscard]] AssetValidation validateAsset(AssetFamily family, const std::string& fileName,
                                            int width, int height);

}  // namespace hmi
