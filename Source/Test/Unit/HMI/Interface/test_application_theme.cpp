// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_application_theme.cpp
 * @brief Tests unitaires du thème de l'IHM : modèle de feuille de style (`LOT-56` TACHE-02,
 *        `EX-IHM-050`, `EX-IHM-051`) et police/typographie (TACHE-03, `EX-IHM-052`).
 */

#include <cstdint>
#include <fstream>
#include <regex>
#include <sstream>
#include <unordered_map>

#include <gtest/gtest.h>

#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/FontResolution.h"
#include "HMI/Interface/StyleSheetTemplate.h"
#include "HMI/Interface/ThemeResolution.h"

namespace {

// Chemin (source) du modele reel, pour verifier le fichier livre plutot qu'une chaine de test
// isolee -- coherent avec JADG_ASSETS_DIR etc. (Test/CMakeLists.txt).
[[nodiscard]] std::string readThemeTemplate() {
    // Les DEUX portees, concatenees (LOT-73, EX-IHM-082) : separer les feuilles ne doit pas
    // retrecir ce que ces garde-fous couvrent. Une regle interdite le reste dans l'une comme dans
    // l'autre.
    std::ostringstream buffer;
    for (const char* const path : {JADG_THEME_IDENTITY_PATH, JADG_THEME_EDITOR_PATH}) {
        std::ifstream file(path);
        buffer << file.rdbuf();
    }
    return buffer.str();
}

// Une seule portee. Depuis le LOT-73 (EX-IHM-082) les deux portees sont deux FICHIERS distincts :
// l'etancheite se verifie donc sur le fichier entier, sans avoir a reperer une frontiere de
// section dans un texte concatene -- un reperage qu'un simple deplacement de commentaire cassait.
[[nodiscard]] std::string readScopeTemplate(const char* path) {
    std::ifstream file(path);
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

}  // namespace

/**
 * @brief Chaque marqueur `${nom}` présent dans le modèle est remplacé par sa valeur ; le résultat
 *        ne contient plus aucun marqueur.
 * \castest{<b>La substitution remplace tous les marqueurs du modele.</b><br/>
 * \tcat Unitaire · Theme de l'IHM<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Substituer un modele avec deux marqueurs connus.<br/>
 * \tattendu Le texte produit contient les valeurs substituees et plus aucun marqueur.
 * }
 */
TEST(ApplicationThemeTest, SubstitutionRemplaceTousLesMarqueurs) {
    const std::unordered_map<std::string, std::string> values{{"a", "1"}, {"b", "2"}};
    const hmi::StyleSheetSubstitutionResult result =
        hmi::substituteStyleSheetTemplate("x: ${a}; y: ${b};", values);
    ASSERT_TRUE(result.ok);
    EXPECT_EQ(result.text, "x: 1; y: 2;");
}

/**
 * @brief Un marqueur absent de la table de substitution est **signalé**, jamais produit
 *        silencieusement avec un trou.
 * \castest{<b>Un marqueur inconnu est signale plutot que produit avec un trou.</b><br/>
 * \tcat Unitaire · Theme de l'IHM<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Substituer un modele dont un marqueur n'a pas de valeur.<br/>
 * \tattendu Le resultat est en echec et nomme le marqueur manquant.
 * }
 */
TEST(ApplicationThemeTest, MarqueurInconnuEstSignale) {
    const hmi::StyleSheetSubstitutionResult result =
        hmi::substituteStyleSheetTemplate("x: ${inconnu};", {});
    ASSERT_FALSE(result.ok);
    EXPECT_NE(result.error.find("inconnu"), std::string::npos);
}

/**
 * @brief Les modèles réels livrés (`Source/Elements/Themes/theme-*.qss`) ne contiennent aucune
 * couleur écrite en dur : toutes passent par un marqueur `${...}`.
 * \castest{<b>Le modele de theme livre ne contient aucune couleur litterale.</b><br/>
 * \tcat Unitaire · Theme de l'IHM<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Lire les feuilles de theme livrees.<br/>2. Chercher un motif de couleur
 * hexadecimale.<br/> \tattendu Aucune occurrence en dehors des commentaires n'est trouvee.
 * }
 */
TEST(ApplicationThemeTest, AucuneCouleurLitteraleDansLeModeleReel) {
    const std::string themeText = readThemeTemplate();
    ASSERT_FALSE(themeText.empty()) << "feuilles de theme introuvables (JADG_THEME_*_PATH)";

    // Retire les commentaires /* ... */ (l'en-tete documente l'historique en exemples de couleurs)
    // avant de chercher un motif de couleur hexadecimale dans les regles elles-memes.
    const std::regex commentPattern(R"(/\*[\s\S]*?\*/)");
    const std::string withoutComments = std::regex_replace(themeText, commentPattern, "");

    const std::regex hexColorPattern(R"(#[0-9a-fA-F]{6}\b)");
    EXPECT_FALSE(std::regex_search(withoutComments, hexColorPattern))
        << "couleur hexadecimale litterale trouvee hors commentaire";
}

/**
 * @brief Produire la feuille de style avec deux jeux de valeurs **variables** différents donne
 *        deux résultats dont les règles d'**identité** (`#MainMenu`, `#OptionsPage`) sont
 *        identiques au caractère près : le thème de l'éditeur ne doit jamais faire bouger le menu
 *        principal.
 * \castest{<b>Les regles d'identite sont etanches au theme de l'editeur.</b><br/>
 * \tcat Unitaire · Theme de l'IHM<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Substituer le modele reel avec deux jeux de jetons d'editeur de couleurs
 * differentes, les valeurs derivant de buildStyleSheetValues.<br/>2. Extraire les blocs
 * `#MainMenu`/`#OptionsPage` des deux resultats.<br/> \tattendu Les deux extraits sont identiques
 * au caractere pres.
 * }
 */
TEST(ApplicationThemeTest, EtancheiteDesPortees) {
    const std::string themeText = readScopeTemplate(JADG_THEME_IDENTITY_PATH);
    ASSERT_FALSE(themeText.empty()) << "feuille d'identite introuvable (JADG_THEME_IDENTITY_PATH)";

    // Les valeurs sont DERIVEES de buildStyleSheetValues, jamais recopiees : une liste ecrite a la
    // main ici devrait etre etendue a chaque marqueur ajoute au modele, et ne le serait pas -- le
    // test echouerait alors sur un "marqueur inconnu" qui n'a rien a voir avec l'etancheite qu'il
    // verifie. Seules les couleurs de l'EDITEUR sont forcees, puisque c'est la variable du test.
    auto valuesWithEditor = [&](std::uint8_t level) {
        hmi::DesignTokens editorTokens = hmi::editorDarkTokens();
        const hmi::DesignColor uniform{.r = level, .g = level, .b = level};
        editorTokens.color.background = uniform;
        editorTokens.color.surface = uniform;
        editorTokens.color.surfaceAlt = uniform;
        editorTokens.color.border = uniform;
        editorTokens.color.text = uniform;
        editorTokens.color.textMuted = uniform;
        editorTokens.color.accent = uniform;
        editorTokens.color.accentHover = uniform;
        editorTokens.color.error = uniform;
        editorTokens.color.frameEdge = uniform;
        editorTokens.color.frameOrnament = uniform;
        editorTokens.color.frameShadow = uniform;
        return hmi::buildStyleSheetValues(editorTokens);
    };

    const hmi::StyleSheetSubstitutionResult dark =
        hmi::substituteStyleSheetTemplate(themeText, valuesWithEditor(0x11));
    const hmi::StyleSheetSubstitutionResult light =
        hmi::substituteStyleSheetTemplate(themeText, valuesWithEditor(0xee));
    ASSERT_TRUE(dark.ok) << dark.error;
    ASSERT_TRUE(light.ok) << light.error;

    // La feuille d'identite ENTIERE, au caractere pres : elle ne contient plus que cette portee.
    EXPECT_EQ(dark.text, light.text);
}

/**
 * @brief La police embarquée est retenue quand elle a pu être enregistrée ; sinon, aucun nom de
 *        famille n'est renvoyé (TACHE-03) -- l'appelant Qt doit alors demander une famille
 *        générique, jamais un second nom codé en dur.
 * \castest{<b>La resolution de police retombe sur une famille generique sans nom code en
 * dur.</b><br/> \tcat Unitaire · Theme de l'IHM<br/> \tcrit Critique<br/> \tetapes 1. Resoudre la
 * police embarquee enregistree, puis non enregistree.<br/> \tattendu Le premier cas rend la famille
 * embarquee ; le second ne rend aucun nom de famille.
 * }
 */
TEST(ApplicationThemeTest, ResolutionDePoliceSansNomDeRepliCodeEnDur) {
    const hmi::FontFamilyResolution registered = hmi::resolveFontFamily(true, "Inter");
    EXPECT_TRUE(registered.useEmbeddedFamily);
    EXPECT_EQ(registered.embeddedFamily, "Inter");

    const hmi::FontFamilyResolution missing = hmi::resolveFontFamily(false, "Inter");
    EXPECT_FALSE(missing.useEmbeddedFamily);
    EXPECT_TRUE(missing.embeddedFamily.empty());
}

/**
 * @brief L'échelle typographique produit des tailles strictement positives, et les rôles de titre
 *        et de corps sont ordonnés du plus grand au plus petit : titre d'écran > titre de
 *        section > corps > libellé secondaire.
 * \castest{<b>L'echelle typographique est positive et ordonnee du plus grand au plus
 * petit.</b><br/> \tcat Unitaire · Theme de l'IHM<br/> \tcrit Majeur<br/> \tetapes 1. Lire les
 * tailles de l'echelle typographique des jetons.<br/> \tattendu Toutes sont strictement positives ;
 * titre d'ecran > titre de section > corps > libelle secondaire.
 * }
 */
TEST(ApplicationThemeTest, EchelleTypographiquePositiveEtOrdonnee) {
    const hmi::TypographyTokens& typography = hmi::identityTokens().typography;
    EXPECT_GT(typography.screenTitle.pointSize, 0);
    EXPECT_GT(typography.sectionTitle.pointSize, 0);
    EXPECT_GT(typography.body.pointSize, 0);
    EXPECT_GT(typography.caption.pointSize, 0);
    EXPECT_GT(typography.monospaceBody.pointSize, 0);

    EXPECT_GT(typography.screenTitle.pointSize, typography.sectionTitle.pointSize);
    EXPECT_GT(typography.sectionTitle.pointSize, typography.body.pointSize);
    EXPECT_GT(typography.body.pointSize, typography.caption.pointSize);
}

/**
 * @brief Aucune propriété de police ni de marge figée ne subsiste dans `MainMenu.ui` ou
 *        `OptionsPage.ui` : la typographie et l'espacement viennent des jetons, pas du fichier
 *        `.ui`.
 * \castest{<b>Aucune taille de police ni marge figee ne subsiste dans les fichiers .ui.</b><br/>
 * \tcat Unitaire · Theme de l'IHM<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Lire MainMenu.ui et OptionsPage.ui.<br/>2. Chercher une propriete font/margin figee
 * au niveau du widget racine.<br/>
 * \tattendu Aucune des deux proprietes n'apparait dans l'un ou l'autre fichier.
 * }
 */
TEST(ApplicationThemeTest, AucuneTailleDePoliceResiduelleDansLesFichiersUi) {
    for (const char* path : {JADG_MAIN_MENU_UI_PATH, JADG_OPTIONS_PAGE_UI_PATH}) {
        std::ifstream file(path);
        std::ostringstream buffer;
        buffer << file.rdbuf();
        const std::string text = buffer.str();
        ASSERT_FALSE(text.empty()) << "fichier .ui introuvable : " << path;
        EXPECT_EQ(text.find("<property name=\"font\">"), std::string::npos)
            << "propriete de police figee trouvee dans " << path;
        EXPECT_EQ(text.find("Margin"), std::string::npos)
            << "propriete de marge figee trouvee dans " << path;
    }
}

/**
 * @brief Résolution pure du thème effectif (`LOT-56` TACHE-06) : `Système` suit le système
 *        d'exploitation, `Clair`/`Sombre` forcé l'ignore.
 * \castest{<b>La resolution du theme effectif suit le reglage et, si Systeme, le systeme.</b><br/>
 * \tcat Unitaire · Theme de l'IHM<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Resoudre les quatre combinaisons reglage/systeme.<br/>
 * \tattendu Systeme+sombre -> Sombre ; Systeme+clair -> Clair ; Clair/Sombre force ignorent le
 * systeme dans les deux etats.
 * }
 */
TEST(ApplicationThemeTest, ResolutionDuThemeEffectifSuitLeReglageEtLeSysteme) {
    using hmi::EditorThemeMode;
    using hmi::EditorThemeSetting;
    EXPECT_EQ(
        hmi::resolveEffectiveEditorTheme(EditorThemeSetting::System, /*systemPrefersDark=*/true),
        EditorThemeMode::Dark);
    EXPECT_EQ(
        hmi::resolveEffectiveEditorTheme(EditorThemeSetting::System, /*systemPrefersDark=*/false),
        EditorThemeMode::Light);
    EXPECT_EQ(
        hmi::resolveEffectiveEditorTheme(EditorThemeSetting::Light, /*systemPrefersDark=*/true),
        EditorThemeMode::Light);
    EXPECT_EQ(
        hmi::resolveEffectiveEditorTheme(EditorThemeSetting::Dark, /*systemPrefersDark=*/false),
        EditorThemeMode::Dark);
}

/**
 * @brief Avec les **vrais** jetons sombre et clair du châssis d'édition, les règles d'identité
 *        (`#MainMenu`, `#OptionsPage`) de la feuille de style produite restent identiques au
 *        caractère près : complète `ApplicationThemeTest.EtancheiteDesPortees` (jetons de test
 *        arbitraires) en couvrant la bascule réelle que `LOT-56` TACHE-06 introduit.
 * \castest{<b>L'etancheite des portees tient avec les vrais themes sombre et clair.</b><br/>
 * \tcat Unitaire · Theme de l'IHM<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Produire la feuille de style avec editorDarkTokens() puis editorLightTokens().<br/>
 * 2. Comparer les blocs `#MainMenu`/`#OptionsPage` des deux resultats.<br/>
 * \tattendu Les deux extraits sont identiques au caractere pres.
 * }
 */
TEST(ApplicationThemeTest, EtancheiteDesPorteesAvecLesVraisThemes) {
    const std::string identityText = readScopeTemplate(JADG_THEME_IDENTITY_PATH);
    const std::string editorText = readScopeTemplate(JADG_THEME_EDITOR_PATH);
    ASSERT_FALSE(identityText.empty()) << "feuille d'identite introuvable";
    ASSERT_FALSE(editorText.empty()) << "feuille du chassis introuvable";

    const auto substituted = [](const std::string& text, const hmi::DesignTokens& tokens) {
        return hmi::substituteStyleSheetTemplate(text, hmi::buildStyleSheetValues(tokens));
    };
    const hmi::StyleSheetSubstitutionResult identityDark =
        substituted(identityText, hmi::editorDarkTokens());
    const hmi::StyleSheetSubstitutionResult identityLight =
        substituted(identityText, hmi::editorLightTokens());
    const hmi::StyleSheetSubstitutionResult editorDark =
        substituted(editorText, hmi::editorDarkTokens());
    const hmi::StyleSheetSubstitutionResult editorLight =
        substituted(editorText, hmi::editorLightTokens());
    ASSERT_TRUE(identityDark.ok) << identityDark.error;
    ASSERT_TRUE(identityLight.ok) << identityLight.error;
    ASSERT_TRUE(editorDark.ok) << editorDark.error;
    ASSERT_TRUE(editorLight.ok) << editorLight.error;

    EXPECT_EQ(identityDark.text, identityLight.text);
    // Le chassis, lui, doit bel et bien changer : sinon TACHE-06 n'aurait aucun effet visible.
    EXPECT_NE(editorDark.text, editorLight.text);
}

/**
 * @brief Les deux portées vivent dans deux fichiers **disjoints** : la feuille d'identité ne nomme
 *        aucun jeton du châssis, et celle du châssis aucun jeton d'identité (`LOT-73`,
 *        `EX-IHM-082`).
 *
 * C'est la condition qui rend la séparation utile plutôt que cosmétique. Les grandeurs
 * `identity.size.*` sont multipliées par le facteur d'agrandissement, qui change avec la hauteur de
 * la fenêtre ; tant qu'elles cohabitaient avec le châssis dans la feuille **applicative**, en
 * changer repolissait les 862 widgets de l'application — cinq secondes par redimensionnement en
 * Debug. Une seule règle d'identité qui reviendrait dans la feuille du châssis ramènerait ce coût.
 * \castest{<b>Les deux portees de theme sont disjointes, marqueur par marqueur.</b><br/>
 * \tcat Unitaire · Theme de l'IHM<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Lire les deux feuilles livrees, commentaires retires.<br/>2. Chercher un marqueur
 * du prefixe de l'autre portee dans chacune.<br/>
 * \tattendu Aucune feuille ne reference les jetons de l'autre portee.
 * }
 */
TEST(ApplicationThemeTest, LesDeuxPorteesSontDansDeuxFichiersDisjoints) {
    // Commentaires retires : chaque feuille EXPLIQUE en tete pourquoi elle ignore l'autre portee,
    // et un test qui s'y declencherait interdirait d'en documenter la raison.
    static const std::regex commentPattern(R"(/\*[\s\S]*?\*/)");
    const auto ruleTextOf = [](const char* path) {
        return std::regex_replace(readScopeTemplate(path), commentPattern, "");
    };
    const std::string identityRules = ruleTextOf(JADG_THEME_IDENTITY_PATH);
    const std::string editorRules = ruleTextOf(JADG_THEME_EDITOR_PATH);
    ASSERT_FALSE(identityRules.empty());
    ASSERT_FALSE(editorRules.empty());

    EXPECT_EQ(identityRules.find("editor.color."), std::string::npos)
        << "la feuille d'identite reference un jeton du chassis d'edition";
    EXPECT_EQ(editorRules.find("identity."), std::string::npos)
        << "la feuille du chassis reference un jeton d'identite : un changement de facteur "
           "d'agrandissement redeviendrait un rejeu applicatif complet";
}
