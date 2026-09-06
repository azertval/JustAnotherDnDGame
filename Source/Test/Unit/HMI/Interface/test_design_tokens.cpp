// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_design_tokens.cpp
 * @brief Tests unitaires des jetons de design de l'IHM (`LOT-56` TACHE-01, `EX-IHM-050`,
 *        `EX-IHM-051`).
 */

#include <string>
#include <unordered_map>

#include <gtest/gtest.h>

#include "HMI/Interface/DesignTokens.h"

/**
 * @brief Chaque couleur produit une chaîne hexadécimale stable et valide (six chiffres, préfixe
 *        `#`), sans alpha.
 * \castest{<b>La conversion en couleur CSS hexadecimale est stable et bien formee.</b><br/>
 * \tcat Unitaire · Jetons de design<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Convertir une couleur connue en chaine hexadecimale.<br/>2. Reconvertir la meme
 * couleur.<br/>
 * \tattendu La chaine fait sept caracteres, commence par '#', et les deux conversions sont
 * identiques (fonction pure).
 * }
 */
TEST(DesignTokensTest, ConversionHexadecimaleStableEtBienFormee) {
    constexpr hmi::DesignColor color{0x1a, 0x2b, 0x3c, 255};
    const std::string first = hmi::toCssColor(color);
    const std::string second = hmi::toCssColor(color);
    EXPECT_EQ(first, second);
    ASSERT_EQ(first.size(), 7u);
    EXPECT_EQ(first, "#1a2b3c");
}

/**
 * @brief La conversion `rgba()` porte les quatre composantes sur 0-255, y compris une opacité
 *        partielle.
 * \castest{<b>La conversion en couleur CSS rgba() porte les quatre composantes.</b><br/>
 * \tcat Unitaire · Jetons de design<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Convertir une couleur avec alpha partiel.<br/>
 * \tattendu La chaine produite contient les quatre valeurs entieres attendues.
 * }
 */
TEST(DesignTokensTest, ConversionRgbaPorteLesQuatreComposantes) {
    constexpr hmi::DesignColor color{0, 128, 255, 64};
    EXPECT_EQ(hmi::toCssRgba(color), "rgba(0, 128, 255, 64)");
}

/**
 * @brief La couleur d'effacement du viewport dérive du **même** jeton que le fond de la portée
 *        correspondante, en mode édition comme en mode jeu : c'est le seul garde-fou contre la
 *        réapparition de la couture entre le canevas et les widgets.
 * \castest{<b>La couleur d'effacement du viewport suit le jeton de fond de la bonne
 * portee.</b><br/> \tcat Unitaire · Jetons de design<br/> \tcrit Critique<br/> \tetapes 1. Demander
 * la couleur d'effacement en mode edition, puis en mode jeu.<br/> \tattendu En edition elle vaut le
 * fond du chassis variable ; en jeu, le fond de l'identite invariante.
 * }
 */
TEST(DesignTokensTest, CouleurViewportSuitLaPorteeDuMode) {
    EXPECT_EQ(hmi::viewportClearColor(/*editorMode=*/true, hmi::editorDarkTokens()),
              hmi::editorDarkTokens().color.background);
    EXPECT_EQ(hmi::viewportClearColor(/*editorMode=*/true, hmi::editorLightTokens()),
              hmi::editorLightTokens().color.background);
    EXPECT_EQ(hmi::viewportClearColor(/*editorMode=*/false, hmi::editorDarkTokens()),
              hmi::identityTokens().color.background);
}

/**
 * @brief Les deux portées d'habillage (identité invariante, châssis variable) partagent
 *        exactement les mêmes échelles d'espacement, de typographie et de tailles — seules les
 *        couleurs diffèrent. Sans cette symétrie, TACHE-06 découvrirait une divergence trop tard.
 * \castest{<b>Les deux portees partagent les memes echelles et tailles.</b><br/>
 * \tcat Unitaire · Jetons de design<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Comparer espacement, niveaux typographiques et tailles des deux portees.<br/>
 * \tattendu Les echelles sont identiques entre les deux portees ; seule la famille de police
 * differe.
 * }
 */
TEST(DesignTokensTest, LesDeuxPorteesPartagentLesMemesEchelles) {
    EXPECT_EQ(hmi::identityTokens().spacing, hmi::editorDarkTokens().spacing);
    EXPECT_EQ(hmi::identityTokens().size, hmi::editorDarkTokens().size);
    // L'echelle typographique reste partagee CHAMP PAR CHAMP, mais plus la famille depuis le
    // LOT-68 : les ecrans du jeu emploient une police bitmap, le chassis d'edition non. Comparer
    // les structures entieres confondrait ces deux faits.
    EXPECT_EQ(hmi::identityTokens().typography.screenTitle,
              hmi::editorDarkTokens().typography.screenTitle);
    EXPECT_EQ(hmi::identityTokens().typography.sectionTitle,
              hmi::editorDarkTokens().typography.sectionTitle);
    EXPECT_EQ(hmi::identityTokens().typography.body, hmi::editorDarkTokens().typography.body);
    EXPECT_EQ(hmi::identityTokens().typography.caption, hmi::editorDarkTokens().typography.caption);
    EXPECT_EQ(hmi::identityTokens().typography.monospaceBody,
              hmi::editorDarkTokens().typography.monospaceBody);
    // Les deux themes d'editeur (LOT-56 TACHE-06) partagent aussi ces echelles entre eux, famille
    // comprise : ils habillent le meme chassis.
    EXPECT_EQ(hmi::editorDarkTokens().spacing, hmi::editorLightTokens().spacing);
    EXPECT_EQ(hmi::editorDarkTokens().typography, hmi::editorLightTokens().typography);
    EXPECT_EQ(hmi::editorDarkTokens().size, hmi::editorLightTokens().size);
}

/**
 * @brief Les deux portées emploient des familles de police **distinctes** (`LOT-68`,
 *        `EX-IHM-070`) : identité en police bitmap, châssis d'édition en police de travail.
 *        C'est ce qui empêche la police pixel de se répandre dans les tables et les arbres denses
 *        de l'éditeur, où elle serait illisible.
 * \castest{<b>Chaque portee designe sa propre famille de police.</b><br/>
 * \tcat Unitaire · Jetons de design<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Lire le role de famille des jetons d'identite et des deux themes d'editeur.<br/>
 * \tattendu L'identite designe Identity, les deux themes d'editeur designent Ui.
 * }
 */
TEST(DesignTokensTest, ChaquePorteeDesigneSaPropreFamille) {
    EXPECT_EQ(hmi::identityTokens().typography.family, hmi::FontRole::Identity);
    EXPECT_EQ(hmi::editorDarkTokens().typography.family, hmi::FontRole::Ui);
    EXPECT_EQ(hmi::editorLightTokens().typography.family, hmi::FontRole::Ui);
}

/**
 * @brief Les trois rôles du cadre pixel art se distinguent assez pour donner du relief à un trait
 *        d'un seul pixel (`LOT-68`, `EX-IHM-070`), et ce dans les trois jeux de jetons — y compris
 *        le thème clair, où un biseau « clair » plus clair que la surface serait invisible.
 * \castest{<b>Les trois roles du cadre pixel art sont distincts dans chaque portee.</b><br/>
 * \tcat Unitaire · Jetons de design<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Pour chaque jeu de jetons, comparer contour, biseau clair et biseau sombre entre
 * eux et a la surface.<br/>
 * \tattendu Les trois roles different deux a deux, le contour est plus sombre que la surface, et
 * le biseau clair est plus clair que le biseau sombre.
 * }
 */
TEST(DesignTokensTest, LesRolesDuCadreSontDistinctsDansChaquePortee) {
    for (const hmi::DesignTokens* tokens :
         {&hmi::identityTokens(), &hmi::editorDarkTokens(), &hmi::editorLightTokens()}) {
        const hmi::ColorTokens& color = tokens->color;
        EXPECT_FALSE(color.frameEdge == color.frameOrnament);
        EXPECT_FALSE(color.frameEdge == color.frameShadow);
        EXPECT_FALSE(color.frameOrnament == color.frameShadow);
        EXPECT_LT(hmi::relativeLuminance(color.frameEdge), hmi::relativeLuminance(color.surface))
            << "le contour doit ancrer le cadre : plus clair que sa surface, il disparait";
        EXPECT_GT(hmi::relativeLuminance(color.frameOrnament),
                  hmi::relativeLuminance(color.frameShadow))
            << "biseaux inverses : le cadre se lirait en creux";
    }
}

/**
 * @brief Les deux portées restent visuellement distinctes en couleur (sans quoi le découpage en
 *        deux jeux de jetons n'aurait pas de sens), et aucune n'utilise de couleurs dégénérées.
 * \castest{<b>Les couleurs des deux portees sont definies et distinctes.</b><br/>
 * \tcat Unitaire · Jetons de design<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Comparer le fond des deux portees.<br/>2. Verifier que le texte contraste avec le
 * fond dans chaque portee.<br/>
 * \tattendu Les fonds different ; dans chaque portee, le texte n'est pas egal au fond.
 * }
 */
TEST(DesignTokensTest, CouleursDesDeuxPorteesDistinctesEtCoherentes) {
    EXPECT_FALSE(hmi::identityTokens().color.background ==
                 hmi::editorDarkTokens().color.background);
    EXPECT_FALSE(hmi::identityTokens().color.text == hmi::identityTokens().color.background);
    EXPECT_FALSE(hmi::editorDarkTokens().color.text == hmi::editorDarkTokens().color.background);
}

/**
 * @brief Les deux widgets de remappage (clavier, manette) doivent exposer la même largeur
 *        minimale : c'est la grandeur que TACHE-01 unifie via `SizeTokens::controlMinWidth`.
 * \castest{<b>Une seule grandeur de largeur minimale existe pour les deux widgets de
 * remappage.</b><br/> \tcat Unitaire · Jetons de design<br/> \tcrit Mineur<br/> \tetapes 1. Lire
 * `controlMinWidth` dans les jetons du chassis d'edition.<br/> \tattendu La valeur est strictement
 * positive (utilisee telle quelle par les deux widgets).
 * }
 */
TEST(DesignTokensTest, AucunDoublonDeLargeurMinimale) {
    EXPECT_GT(hmi::editorDarkTokens().size.controlMinWidth, 0);
}

/**
 * @brief Pour chaque thème d'éditeur (sombre, clair), le rapport de contraste entre le texte et
 *        son fond, et entre le texte atténué et son fond, dépasse un seuil de lisibilité fixé —
 *        garde-fou contre un thème clair livré illisible (`LOT-56` TACHE-06), étendu à la portée
 *        identité au `LOT-68` (sa palette change).
 * \castest{<b>Chaque jeu de jetons satisfait un seuil de contraste texte/fond.</b><br/>
 * \tcat Unitaire · Jetons de design<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Calculer le contraste texte/fond et texte-attenue/fond pour les deux themes
 * d'editeur, puis pour l'identite.<br/>
 * \tattendu Le texte principal depasse 4.5:1 et le texte attenue depasse 3:1, dans les trois jeux
 * de jetons.
 * }
 */
TEST(DesignTokensTest, ChaqueThemeSatisfaitLeSeuilDeContraste) {
    constexpr double MIN_TEXT_CONTRAST = 4.5;
    constexpr double MIN_MUTED_TEXT_CONTRAST = 3.0;
    // La portee identite rejoint le garde-fou au LOT-68 : sa palette change, son contraste doit
    // etre verifie comme celui des themes d'editeur.
    for (const hmi::DesignTokens* tokens :
         {&hmi::editorDarkTokens(), &hmi::editorLightTokens(), &hmi::identityTokens()}) {
        EXPECT_GE(hmi::contrastRatio(tokens->color.text, tokens->color.background),
                  MIN_TEXT_CONTRAST);
        EXPECT_GE(hmi::contrastRatio(tokens->color.textMuted, tokens->color.background),
                  MIN_MUTED_TEXT_CONTRAST);
    }
}

/**
 * @brief Le rapport de contraste est symétrique et vaut 1 pour deux couleurs identiques (aucun
 *        contraste) : garde-fou de la formule elle-même.
 * \castest{<b>Le rapport de contraste est symetrique et vaut 1 sans difference de couleur.</b><br/>
 * \tcat Unitaire · Jetons de design<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Calculer le contraste d'une couleur avec elle-meme, puis dans les deux sens entre
 * deux couleurs distinctes.<br/>
 * \tattendu Le premier resultat vaut 1 ; les deux sens du second sont egaux.
 * }
 */
TEST(DesignTokensTest, ContrasteSymetriqueEtUnitairePourUneMemeCouleur) {
    constexpr hmi::DesignColor color{0x42, 0x88, 0xcc};
    EXPECT_NEAR(hmi::contrastRatio(color, color), 1.0, 1e-9);
    EXPECT_NEAR(hmi::contrastRatio(hmi::DesignColor{0, 0, 0}, hmi::DesignColor{255, 255, 255}),
                hmi::contrastRatio(hmi::DesignColor{255, 255, 255}, hmi::DesignColor{0, 0, 0}),
                1e-9);
}

/**
 * @brief Les grandeurs de la portee identite sont multipliees par le facteur entier, et celles du
 *        chassis d edition ne le sont JAMAIS (`LOT-68`, `EX-IHM-070`). C est toute la regle : les
 *        ecrans du jeu sont une image agrandie, l editeur est un outil dont les tailles suivent les
 *        reglages du systeme.
 * \castest{<b>Seules les grandeurs d identite suivent le facteur d agrandissement.</b><br/>
 * \tcat Unitaire · Jetons de design<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Produire les valeurs de feuille de style aux facteurs 1, 2 et 3.<br/>2. Comparer les
 * marqueurs identity.size.* et tokens.spacing.* entre eux.<br/>
 * \tattendu Les grandeurs d identite valent la base multipliee par le facteur ; celles du chassis
 * sont identiques aux trois facteurs.
 * }
 */
TEST(DesignTokensTest, SeulesLesGrandeursDIdentiteSuiventLeFacteur) {
    const hmi::IdentityBaseScale& base = hmi::identityBaseScale();
    for (const int scale : {1, 2, 3}) {
        const std::unordered_map<std::string, std::string> values =
            hmi::buildStyleSheetValues(hmi::editorDarkTokens(), scale);
        EXPECT_EQ(values.at("identity.size.screenTitle"), std::to_string(base.screenTitle * scale));
        EXPECT_EQ(values.at("identity.size.sectionTitle"),
                  std::to_string(base.sectionTitle * scale));
        EXPECT_EQ(values.at("identity.size.body"), std::to_string(base.body * scale));
        EXPECT_EQ(values.at("identity.frame.thickness"),
                  std::to_string(base.frameThickness * scale));
        // Le chassis d edition ne bouge pas d un pixel, quel que soit le facteur.
        EXPECT_EQ(values.at("tokens.spacing.small"),
                  std::to_string(hmi::editorDarkTokens().spacing.small));
        EXPECT_EQ(values.at("tokens.typography.screenTitle.pointSize"),
                  std::to_string(hmi::editorDarkTokens().typography.screenTitle.pointSize));
    }
    // Un facteur absurde est ramene a 1 plutot que de reduire les ecrans a rien.
    EXPECT_EQ(hmi::buildStyleSheetValues(hmi::editorDarkTokens(), 0).at("identity.size.body"),
              std::to_string(base.body));
    EXPECT_EQ(hmi::buildStyleSheetValues(hmi::editorDarkTokens(), -4).at("identity.size.body"),
              std::to_string(base.body));
}
