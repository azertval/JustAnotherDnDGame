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
    for (const char* const path : {JADG_THEME_EDITOR_PATH}) {
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

// Les deux tests d'ETANCHEITE DES PORTEES ont ete retires au LOT-86, et leur garantie n'est pas
// perdue : elle est devenue STRUCTURELLE. Ils verifiaient que le theme de l'editeur ne faisait pas
// bouger les regles d'identite du menu principal, les deux portees vivant dans deux feuilles de
// style du meme binaire. Depuis, l'identite du jeu vit dans Source/Ui/Theme/Tokens.qml, en QML,
// dans un AUTRE BINAIRE : aucun chemin ne relie plus les deux, et il n'y a plus rien a verifier.
//
// Le troisieme, « les deux portees sont dans deux fichiers disjoints », tombe pour la meme
// raison : il n'y a plus deux fichiers, il y a deux langages.
//
// C'est le meilleur sort qu'on puisse reserver a un test : que ce qu'il surveillait devienne
// impossible.

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

// Le test « aucune taille de police residuelle dans les fichiers .ui » a ete RETIRE au LOT-86 : les
// .ui des ecrans du jeu n'existent plus, ces ecrans etant passes en QML. Sa garantie n'est pas
// perdue pour autant -- `scripts/check_ui_layers.py` (regle 6, EX-IHM-105) verifie desormais
// qu'aucune couleur, police ni taille n'est ecrite en dur hors de `Source/Ui/Theme`. Le controle
// couvre donc plus qu'avant : tous les ecrans, et pas seulement deux.

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
