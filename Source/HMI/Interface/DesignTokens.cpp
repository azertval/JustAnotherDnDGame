// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/DesignTokens.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>

namespace hmi {

namespace {

// Échelles partagées par les deux portées (TACHE-01) : seules les couleurs diffèrent entre elles.
constexpr SpacingTokens SHARED_SPACING{};

// Echelle typographique partagee par les deux portees. La FAMILLE, elle, ne l'est pas : chaque
// portee la pose apres appel (LOT-68) -- c'est le seul champ de TypographyTokens qui les separe.
[[nodiscard]] TypographyTokens sharedTypography() noexcept {
    TypographyTokens typography;
    typography.screenTitle = TypographyLevel{
        .pointSize = 32, .weight = 700};  // QFont::Bold -- titre du menu/des Options.
    typography.sectionTitle = TypographyLevel{
        .pointSize = 16, .weight = 700};  // Boutons de navigation, titres de panneau.
    typography.body = TypographyLevel{.pointSize = 10, .weight = 400};  // QFont::Normal
    typography.caption = TypographyLevel{.pointSize = 9, .weight = 400};
    typography.monospaceBody = TypographyLevel{.pointSize = 10, .weight = 400};
    return typography;
}

constexpr SizeTokens SHARED_SIZE{};

// Identité du jeu (menu principal, écran Options, jeu) : reprend les couleurs du thème historique
// (fond sombre, accent ambre), désormais nommées par rôle plutôt qu'éparpillées en littéraux.
[[nodiscard]] DesignTokens buildIdentityTokens() noexcept {
    DesignTokens tokens;
    // Palette du PARCHEMIN DE TANARES (LOT-66, EX-IHM-070). Chaque teinte est RELEVEE sur
    // `Documentation/SourceBook/Character_Sheets_Tanares.pdf` -- histogramme quantifie par paliers
    // de 16 des pages rendues -- et non choisie a vue. La raison est la meme que pour les regles
    // chiffrees (EX-CNT-020) : une couleur inventee ressemble a la source sans en venir, et rien
    // ne le dit jamais. Les valeurs ci-dessous sont les teintes DOMINANTES de ces pages.
    tokens.color.background = DesignColor{.r = 0xd0, .g = 0xc0, .b = 0xa0};  // parchemin vieilli
    tokens.color.surface = DesignColor{.r = 0xe0, .g = 0xd0, .b = 0xb0};     // champ de la feuille
    tokens.color.surfaceAlt = DesignColor{.r = 0xf0, .g = 0xe0, .b = 0xd0};  // encadre clair
    tokens.color.border = DesignColor{.r = 0x90, .g = 0x70, .b = 0x30};      // brun dore
    tokens.color.text = DesignColor{.r = 0x30, .g = 0x20, .b = 0x00};        // encre sepia
    tokens.color.textMuted = DesignColor{.r = 0x70, .g = 0x50, .b = 0x20};   // encre delavee
    tokens.color.accent = DesignColor{.r = 0xc0, .g = 0xa0, .b = 0x60};      // or des filets
    tokens.color.accentHover = DesignColor{.r = 0xc0, .g = 0xb0, .b = 0x80};
    // `error` est le SEUL role qui ne vienne pas des feuilles : une feuille de personnage n'a pas
    // d'etat d'erreur a montrer. Rouge de garance assombri, choisi pour tenir le contraste sur le
    // parchemin -- et signale ici comme non atteste plutot que passe sous silence.
    tokens.color.error = DesignColor{.r = 0x8a, .g = 0x2f, .b = 0x20};
    // Cadre de parchemin : un trait exterieur d'encre, un filet ornemental dore en retrait, et
    // l'ombre que le cadre porte sur le fond. Ce ne sont plus des biseaux : la lumiere ne vient
    // plus d'en haut a gauche, il n'y a plus de relief a simuler. Ce qui doit rester LISIBLE,
    // c'est l'ecart entre le trait et le filet -- deux traits de meme valeur ne composent pas un
    // encadrement, ils composent une bordure epaisse.
    tokens.color.frameEdge = DesignColor{.r = 0x30, .g = 0x20, .b = 0x00};
    tokens.color.frameOrnament = DesignColor{.r = 0x90, .g = 0x70, .b = 0x30};
    tokens.color.frameShadow = DesignColor{.r = 0x70, .g = 0x50, .b = 0x20};
    tokens.spacing = SHARED_SPACING;
    tokens.typography = sharedTypography();
    tokens.typography.family = FontRole::Identity;  // titrage a empattements (LOT-68).
    tokens.size = SHARED_SIZE;
    return tokens;
}

// Châssis d'édition, thème sombre (variable, TACHE-06 lui ajoute un jeu clair) : palette neutre
// proche de l'identité, même accent ambre pour la cohérence visuelle de l'ensemble (critère
// d'acceptation 1 du lot : une seule apparence, aucune couture entre les deux portées).
[[nodiscard]] DesignTokens buildEditorDarkTokens() noexcept {
    DesignTokens tokens;
    tokens.color.background = DesignColor{.r = 0x1e, .g = 0x22, .b = 0x2b};
    tokens.color.surface = DesignColor{.r = 0x26, .g = 0x2b, .b = 0x36};
    tokens.color.surfaceAlt = DesignColor{.r = 0x2d, .g = 0x33, .b = 0x40};
    tokens.color.border = DesignColor{.r = 0x3a, .g = 0x41, .b = 0x50};
    tokens.color.text = DesignColor{.r = 0xe6, .g = 0xe8, .b = 0xee};
    tokens.color.textMuted = DesignColor{.r = 0x9a, .g = 0xa1, .b = 0xb0};
    tokens.color.accent = DesignColor{.r = 0xff, .g = 0xd1, .b = 0x33};
    tokens.color.accentHover = DesignColor{.r = 0xff, .g = 0xdb, .b = 0x5c};
    tokens.color.error = DesignColor{.r = 0xff, .g = 0x6b, .b = 0x6b};
    // Le chassis ne dessine aucun cadre : ces trois roles y servent de bordure eclaircie /
    // assombrie, deja necessaires aux separateurs et aux barres de titre de dock.
    tokens.color.frameEdge = DesignColor{.r = 0x14, .g = 0x17, .b = 0x1e};
    tokens.color.frameOrnament = DesignColor{.r = 0x4a, .g = 0x52, .b = 0x63};
    tokens.color.frameShadow = DesignColor{.r = 0x1a, .g = 0x1e, .b = 0x26};
    tokens.spacing = SHARED_SPACING;
    tokens.typography = sharedTypography();
    tokens.size = SHARED_SIZE;
    return tokens;
}

// Chassis d'edition, theme CLAIR (LOT-56 TACHE-06) : pas le sombre invers -- ecarts de luminosite
// entre fond/surface/bordure/lignes alternees choisis independamment, accent assombri (l'ambre vif
// du theme sombre manque de contraste utilise comme texte sur un fond clair).
[[nodiscard]] DesignTokens buildEditorLightTokens() noexcept {
    DesignTokens tokens;
    tokens.color.background = DesignColor{.r = 0xf5, .g = 0xf6, .b = 0xf8};
    tokens.color.surface = DesignColor{.r = 0xff, .g = 0xff, .b = 0xff};
    tokens.color.surfaceAlt = DesignColor{.r = 0xec, .g = 0xee, .b = 0xf2};
    tokens.color.border = DesignColor{.r = 0xc9, .g = 0xce, .b = 0xd6};
    tokens.color.text = DesignColor{.r = 0x1c, .g = 0x21, .b = 0x28};
    tokens.color.textMuted = DesignColor{.r = 0x5b, .g = 0x64, .b = 0x72};
    tokens.color.accent = DesignColor{.r = 0xb3, .g = 0x6b, .b = 0x00};
    tokens.color.accentHover = DesignColor{.r = 0xcc, .g = 0x7a, .b = 0x00};
    tokens.color.error = DesignColor{.r = 0xc0, .g = 0x26, .b = 0x26};
    // Theme clair : le biseau "clair" est plus SOMBRE que la surface (une surface blanche n'a pas
    // de plus clair qu'elle) -- inverser mecaniquement le theme sombre produirait deux biseaux
    // invisibles.
    tokens.color.frameEdge = DesignColor{.r = 0x9d, .g = 0xa4, .b = 0xb0};
    tokens.color.frameOrnament = DesignColor{.r = 0xe1, .g = 0xe5, .b = 0xeb};
    tokens.color.frameShadow = DesignColor{.r = 0xb5, .g = 0xbc, .b = 0xc6};
    tokens.spacing = SHARED_SPACING;
    tokens.typography = sharedTypography();
    tokens.size = SHARED_SIZE;
    return tokens;
}

// Ajoute les douze roles de couleur d'une portee au tableau de substitution, sous le prefixe donne
// (ex. "identity.color.background" -> "#1a1f29").
void addColorValues(std::unordered_map<std::string, std::string>& values, const std::string& prefix,
                    const ColorTokens& color) {
    values[prefix + ".background"] = toCssColor(color.background);
    values[prefix + ".surface"] = toCssColor(color.surface);
    values[prefix + ".surfaceAlt"] = toCssColor(color.surfaceAlt);
    values[prefix + ".border"] = toCssColor(color.border);
    values[prefix + ".text"] = toCssColor(color.text);
    values[prefix + ".textMuted"] = toCssColor(color.textMuted);
    values[prefix + ".accent"] = toCssColor(color.accent);
    values[prefix + ".accentHover"] = toCssColor(color.accentHover);
    values[prefix + ".error"] = toCssColor(color.error);
    values[prefix + ".frameEdge"] = toCssColor(color.frameEdge);
    values[prefix + ".frameOrnament"] = toCssColor(color.frameOrnament);
    values[prefix + ".frameShadow"] = toCssColor(color.frameShadow);
}

}  // namespace

DesignColor mixColor(DesignColor from, DesignColor to, float ratio) noexcept {
    const float t = std::clamp(ratio, 0.0f, 1.0f);
    const auto blend = [t](std::uint8_t a, std::uint8_t b) {
        const float mixed = (static_cast<float>(a) * (1.0f - t)) + (static_cast<float>(b) * t);
        // std::lround plutot qu'un + 0.5 tronque : ce dernier arrondit mal les valeurs negatives
        // et depend du mode d'arrondi courant. Les composantes sont positives ici, mais la forme
        // fautive se recopie.
        return static_cast<std::uint8_t>(std::lround(mixed));
    };
    return DesignColor{.r = blend(from.r, to.r),
                       .g = blend(from.g, to.g),
                       .b = blend(from.b, to.b),
                       .a = blend(from.a, to.a)};
}

const IdentityBaseScale& identityBaseScale() noexcept {
    static const IdentityBaseScale scale;
    return scale;
}

const char* genericCssFamily(FontRole role) noexcept {
    switch (role) {
        case FontRole::Identity:
            // Le titrage a empattements se rapproche davantage d'une chasse fixe que d'une
            // lineale, pour le repli generique : si la
            // police embarquee manque, autant que le repli garde des colonnes alignees.
            return "monospace";
        case FontRole::Ui:
            return "sans-serif";
    }
    return "sans-serif";
}

const DesignTokens& identityTokens() noexcept {
    static const DesignTokens tokens = buildIdentityTokens();
    return tokens;
}

const DesignTokens& editorDarkTokens() noexcept {
    static const DesignTokens tokens = buildEditorDarkTokens();
    return tokens;
}

const DesignTokens& editorLightTokens() noexcept {
    static const DesignTokens tokens = buildEditorLightTokens();
    return tokens;
}

DesignColor viewportClearColor(bool editorMode, const DesignTokens& activeEditorTokens) noexcept {
    return editorMode ? activeEditorTokens.color.background : identityTokens().color.background;
}

std::string toCssColor(DesignColor color) {
    std::array<char, 8> buffer{};
    std::snprintf(buffer.data(), buffer.size(), "#%02x%02x%02x", color.r, color.g, color.b);
    return std::string(buffer.data());
}

std::string toCssRgba(DesignColor color) {
    return "rgba(" + std::to_string(color.r) + ", " + std::to_string(color.g) + ", " +
           std::to_string(color.b) + ", " + std::to_string(color.a) + ")";
}

namespace {

// Composante -> canal lineaire WCAG (gamma sRGB inverse).
[[nodiscard]] double linearChannel(std::uint8_t component) noexcept {
    const double c = static_cast<double>(component) / 255.0;
    return c <= 0.04045 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);
}

}  // namespace

double relativeLuminance(DesignColor color) noexcept {
    return (0.2126 * linearChannel(color.r)) + (0.7152 * linearChannel(color.g)) +
           (0.0722 * linearChannel(color.b));
}

double contrastRatio(DesignColor a, DesignColor b) noexcept {
    const double la = relativeLuminance(a);
    const double lb = relativeLuminance(b);
    const double lighter = std::max(la, lb);
    const double darker = std::min(la, lb);
    return (lighter + 0.05) / (darker + 0.05);
}

std::unordered_map<std::string, std::string> buildStyleSheetValues(const DesignTokens& editorTokens,
                                                                   int identityScale) {
    std::unordered_map<std::string, std::string> values;
    addColorValues(values, "identity.color", identityTokens().color);
    addColorValues(values, "editor.color", editorTokens.color);
    // Echelle d'espacement : partagee entre les deux portees (DesignTokensTest.
    // LesDeuxPorteesPartagentLesMemesEchelles), la source choisie ici est arbitraire.
    const SpacingTokens& spacing = identityTokens().spacing;
    values["tokens.spacing.extraSmall"] = std::to_string(spacing.extraSmall);
    values["tokens.spacing.small"] = std::to_string(spacing.small);
    values["tokens.spacing.medium"] = std::to_string(spacing.medium);
    values["tokens.spacing.large"] = std::to_string(spacing.large);
    values["tokens.spacing.extraLarge"] = std::to_string(spacing.extraLarge);
    // Echelle typographique (LOT-56 TACHE-03) : elle aussi partagee entre les deux portees.
    const TypographyTokens& typography = identityTokens().typography;
    // Familles de police : valeur par defaut PURE (mot-cle generique). hmi::applyStyleSheet
    // l'ecrase par le nom REELLEMENT enregistre quand il y en a un. Les declarer ici, et non
    // seulement cote Qt, garantit que tout marqueur du modele a une valeur meme hors application
    // -- et qu'une police manquante degrade au lieu de laisser un marqueur non resolu.
    // Grandeurs de la portee identite : multipliees par le facteur ENTIER (LOT-68). Le facteur
    // est borne a 1 au minimum -- une valeur nulle ou negative reduirait les ecrans a rien.
    const int scale = identityScale < 1 ? 1 : identityScale;
    const IdentityBaseScale& base = identityBaseScale();
    values["identity.size.screenTitle"] = std::to_string(base.screenTitle * scale);
    values["identity.size.sectionTitle"] = std::to_string(base.sectionTitle * scale);
    values["identity.size.body"] = std::to_string(base.body * scale);
    values["identity.size.caption"] = std::to_string(base.caption * scale);
    values["identity.space.small"] = std::to_string(base.spaceSmall * scale);
    values["identity.space.medium"] = std::to_string(base.spaceMedium * scale);
    values["identity.space.large"] = std::to_string(base.spaceLarge * scale);
    values["identity.space.extraLarge"] = std::to_string(base.spaceExtraLarge * scale);
    values["identity.frame.thickness"] = std::to_string(base.frameThickness * scale);
    values["identity.font.body"] = genericCssFamily(identityTokens().typography.family);
    values["identity.font.title"] = genericCssFamily(identityTokens().typography.family);
    values["tokens.typography.screenTitle.pointSize"] =
        std::to_string(typography.screenTitle.pointSize);
    values["tokens.typography.sectionTitle.pointSize"] =
        std::to_string(typography.sectionTitle.pointSize);
    return values;
}

}  // namespace hmi
