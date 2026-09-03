// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

/**
 * @file HMI/Interface/DesignTokens.h
 * @brief Jetons de design de l'IHM Qt (`LOT-56`, `EX-IHM-050`, `EX-IHM-051`) : source unique des
 *        couleurs, espacements, typographie et tailles de l'application.
 *
 * Logique **pure** (aucune dépendance Qt/GPU), testable hors instance d'application
 * (`EX-NFR-010`) — compilée à la fois dans `JustAnotherDnDGame` et directement dans `UnitTests`, comme
 * `hmi::ProceduralFont` ou `hmi::TileVisuals`.
 *
 * Deux portées d'habillage partagent la structure `DesignTokens` (mêmes rôles, mêmes échelles,
 * mêmes tailles ; seules les couleurs diffèrent) :
 * - `identityTokens()` — l'**identité du jeu** (menu principal, écran Options, jeu) : invariante,
 *   ne suit jamais aucun réglage d'affichage.
 * - `editorDarkTokens()` — le **châssis d'édition** (panneaux, barre d'outils, barre d'état, barre
 *   de menus de l'éditeur, boîtes de dialogue ouvertes depuis lui) : variable, thème sombre par
 *   défaut ; `editorLightTokens()` (`LOT-56` TACHE-06) lui ajoute un second jeu de valeurs.
 *
 * Réutiliser la **même** structure pour les deux portées rend leur symétrie de rôles garantie par
 * le système de types plutôt que par convention : un rôle ajouté à l'une existe nécessairement
 * dans l'autre.
 */

namespace hmi {

/// Couleur RVBA pure (indépendante de Qt), convertible en chaîne CSS pour la feuille de style et
/// directement décomposable pour construire une `QColor`/`QPalette`.
struct DesignColor {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;

    [[nodiscard]] friend bool operator==(const DesignColor& lhs, const DesignColor& rhs) noexcept {
        return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
    }
};

/// Rôles de couleur, communs aux deux portées d'habillage. Nommés par **rôle** et non par teinte :
/// un jeton `accent` survit à un changement de couleur, un jeton `ambre` non.
struct ColorTokens {
    DesignColor background;   ///< Fond des fenêtres/vues.
    DesignColor surface;      ///< Fond des panneaux, boîtes de dialogue, champs.
    DesignColor surfaceAlt;   ///< Fond alterné (lignes de tableau, onglets non sélectionnés).
    DesignColor border;       ///< Bordures et séparateurs.
    DesignColor text;         ///< Texte principal.
    DesignColor textMuted;    ///< Texte secondaire/désactivé.
    DesignColor accent;       ///< Couleur d'accent (sélection, focus, contrôle actif).
    DesignColor accentHover;  ///< Couleur d'accent au survol.
    DesignColor error;        ///< Signalement d'erreur/d'échec.
    // Rôles du cadre pixel art (LOT-68, EX-IHM-070). Ajoutés aux DEUX portées : la structure est
    // commune par construction, ce qui garantit qu'un rôle ajouté à l'une existe dans l'autre.
    // Le châssis d'édition les définit sans les dessiner en cadre — il s'en sert pour ses bordures
    // et ses séparateurs, où ils tenaient déjà lieu de `border` éclairci/assombri.
    DesignColor outline;     ///< Contour extérieur d'un cadre : la valeur la plus sombre.
    DesignColor bevelLight;  ///< Biseau haut/gauche (la lumière vient d'en haut à gauche).
    DesignColor bevelDark;   ///< Biseau bas/droite.
};

// Neutralise la macro Windows `small` (`rpcndr.h`, incluse via <Windows.h> dans toute unite de
// traduction qui tire un en-tete Windows avant ce fichier) : `#define small char` casserait sinon
// silencieusement `SpacingTokens::small` ci-dessous. Garde conservee apres le portage QRhi : le
// projet n'inclut plus <d3d11.h>, mais rien ne garantit qu'aucun en-tete Windows ne remonte.
#ifdef small
#undef small
#endif

/// Échelle d'espacement, en pixels logiques.
struct SpacingTokens {
    int extraSmall = 4;
    int small = 8;
    int medium = 12;
    int large = 16;
    int extraLarge = 24;

    [[nodiscard]] friend bool operator==(const SpacingTokens&,
                                         const SpacingTokens&) noexcept = default;
};

/// Un niveau de l'échelle typographique : taille en points et graisse (`QFont::Weight`, 1-1000).
struct TypographyLevel {
    int pointSize = 10;
    int weight = 400;  ///< QFont::Normal

    [[nodiscard]] friend bool operator==(const TypographyLevel&,
                                         const TypographyLevel&) noexcept = default;
};

/// Famille de police d'une portée (`LOT-68`, `EX-IHM-070`). La distinction est un **rôle**, pas un
/// nom de police : `Identity` reste `Identity` le jour où la police pixel embarquée change.
enum class FontRole {
    Ui,        ///< Châssis d'édition : police de travail, lisible en petit et en tableau dense.
    Identity,  ///< Écrans du jeu : police bitmap, rendue sans lissage à échelle entière.
};

/// Échelle typographique, par **rôle** — jamais de taille ponctuelle en dehors de cette échelle.
struct TypographyTokens {
    FontRole family = FontRole::Ui;  ///< Famille employée par la portée.
    TypographyLevel screenTitle;     ///< Titre d'écran (menu principal, Options).
    TypographyLevel sectionTitle;    ///< Titre de section/panneau.
    TypographyLevel body;            ///< Corps de texte, contrôles.
    TypographyLevel caption;         ///< Libellé secondaire, infobulle.
    TypographyLevel monospaceBody;   ///< Texte à chasse fixe (identité du menu principal).

    [[nodiscard]] friend bool operator==(const TypographyTokens&,
                                         const TypographyTokens&) noexcept = default;
};

/// Tailles d'icônes, de vignettes et de contrôles usuels, en pixels logiques (avant mise à
/// l'échelle d'affichage, `LOT-56` TACHE-05).
struct SizeTokens {
    int iconSmall = 16;
    int iconMedium = 20;  ///< Icônes de ligne (panneau Textures) et d'action de barre d'outils.
    int iconLarge = 24;
    int paletteThumbnail = 32;  ///< Vignettes de la palette (multiple de la taille d'une case).
    int assetThumbnail = 48;    ///< Vignettes des grilles d'assets (`AssetThumbnailView`).
    int controlMinWidth =
        160;  ///< Largeur minimale usuelle (boutons de remappage clavier/manette).

    [[nodiscard]] friend bool operator==(const SizeTokens&, const SizeTokens&) noexcept = default;
};

/// Jeu complet de jetons de design d'une portée (identité invariante ou châssis variable).
struct DesignTokens {
    ColorTokens color;
    SpacingTokens spacing;
    TypographyTokens typography;
    SizeTokens size;
};

/// Grandeurs de la portée **identité**, exprimées en pixels **à l'échelle 1** (`LOT-68`,
/// `EX-IHM-070`). Distinctes de `TypographyTokens`/`SpacingTokens`, qui restent partagées et
/// **jamais multipliées** : le châssis d'édition est un outil de travail dont les tailles suivent
/// les réglages du système, les écrans du jeu sont une image agrandie d'un facteur entier.
///
/// En **pixels** et non en points : le pixel art se dimensionne en pixels, et un point vaut une
/// fraction variable de pixel selon la définition de l'écran — le facteur entier n'aurait alors
/// plus rien d'entier.
struct IdentityBaseScale {
    int screenTitle = 23;   ///< Titre d'écran (police de titre).
    int sectionTitle = 14;  ///< Entrée de menu, titre de section.
    int body = 10;          ///< Corps de texte, contrôles.
    int caption = 8;        ///< Libellé secondaire, aide de touche.

    int spaceSmall = 4;
    int spaceMedium = 6;
    int spaceLarge = 8;
    int spaceExtraLarge = 12;

    int frameThickness = 1;  ///< Contour et biseau d'un cadre : un pixel de maquette chacun.

    [[nodiscard]] friend bool operator==(const IdentityBaseScale&,
                                         const IdentityBaseScale&) noexcept = default;
};

/// @return L'échelle de base de la portée identité (`LOT-68`).
[[nodiscard]] const IdentityBaseScale& identityBaseScale() noexcept;

/// @return Le mélange de @p from et @p to, à la proportion @p ratio (0 = @p from, 1 = @p to).
/// Sert à **dériver** une nuance d'un jeton plutôt qu'à en écrire une nouvelle en dur : une teinte
/// littérale ne suivrait pas un changement de palette (`EX-IHM-051`). @p ratio est borné à [0, 1].
[[nodiscard]] DesignColor mixColor(DesignColor from, DesignColor to, float ratio) noexcept;

/// @return Le mot-clé de famille **générique** CSS correspondant à @p role (`monospace` pour
/// l'identité pixel art, `sans-serif` pour le châssis d'édition). C'est le repli de la feuille de
/// style quand aucune police embarquée n'a pu être enregistrée : un mot-clé générique, jamais un
/// second nom de police codé en dur (`EX-IHM-052`).
[[nodiscard]] const char* genericCssFamily(FontRole role) noexcept;

/// Jetons de l'identité du jeu (menu principal, écran Options, jeu) : **invariants**, ne suivent
/// jamais aucun réglage d'affichage (`EX-IHM-050`).
[[nodiscard]] const DesignTokens& identityTokens() noexcept;

/// Jetons du châssis d'édition en thème **sombre** (défaut) : **variables**, `editorLightTokens()`
/// (`LOT-56` TACHE-06) en fournit un second jeu.
[[nodiscard]] const DesignTokens& editorDarkTokens() noexcept;

/// Jetons du châssis d'édition en thème **clair** (`LOT-56` TACHE-06) : mêmes rôles, mêmes
/// échelles que `editorDarkTokens()` — seules les couleurs diffèrent, et pas par simple inversion
/// (les écarts de luminosité entre bordures/lignes alternées/fond diffèrent en clair et en sombre).
[[nodiscard]] const DesignTokens& editorLightTokens() noexcept;

/// Couleur d'effacement du viewport (`GameViewport`), dérivée des jetons plutôt que d'une valeur
/// littérale locale : fond de la portée **variable** en édition (suivant le thème actif de
/// l'éditeur, `LOT-56` TACHE-06), fond de la portée **invariante** en jeu et en essai — la seule
/// surface qui appartient tour à tour aux deux portées.
/// @param editorMode         `true` en édition, `false` en jeu/essai.
/// @param activeEditorTokens Jetons du châssis d'édition **actuellement effectifs** (thème clair
///                           ou sombre) ; ignorés si `editorMode` est faux.
[[nodiscard]] DesignColor viewportClearColor(bool editorMode,
                                             const DesignTokens& activeEditorTokens) noexcept;

/// Conversion pure : couleur -> chaîne CSS hexadécimale `"#rrggbb"` (minuscules, sans alpha — les
/// feuilles de style Qt n'acceptent l'alpha que via `rgba()`).
[[nodiscard]] std::string toCssColor(DesignColor color);

/// Conversion pure : couleur -> chaîne CSS `"rgba(r, g, b, a)"`, composantes entières 0-255 (les
/// feuilles de style Qt expriment l'alpha sur la même échelle que les autres composantes).
[[nodiscard]] std::string toCssRgba(DesignColor color);

/// Luminance relative WCAG d'une couleur (0 = noir, 1 = blanc), pour le calcul de contraste.
[[nodiscard]] double relativeLuminance(DesignColor color) noexcept;

/// Rapport de contraste WCAG entre deux couleurs (1 = aucun contraste, 21 = noir sur blanc).
[[nodiscard]] double contrastRatio(DesignColor a, DesignColor b) noexcept;

/// Table de substitution marqueur -> valeur pour le modèle de feuille de style (`LOT-56`
/// TACHE-02), à partir d'un jeu de jetons du châssis d'édition (portée variable) ; les marqueurs de
/// préfixe "identity." sont toujours résolus depuis `identityTokens()` (portée invariante).
/// Fonction **pure** : ne dépend que des jetons, jamais de l'état de l'application.
[[nodiscard]] std::unordered_map<std::string, std::string> buildStyleSheetValues(
    const DesignTokens& editorTokens, int identityScale = 1);

}  // namespace hmi
