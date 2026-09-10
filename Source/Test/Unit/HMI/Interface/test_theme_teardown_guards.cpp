// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_theme_teardown_guards.cpp
 * @brief Garde-fous contre le plantage **à la fermeture**, régression récurrente du projet
 *        (`LOT-56`, `LOT-68`).
 *
 * L'IHM a planté deux fois à la fermeture, pour deux causes différentes, et les deux fois de façon
 * **intermittente** — donc invisible en essai rapide et coûteuse à diagnostiquer. Les deux causes
 * connues étaient jusqu'ici consignées en prose dans les guides et les commentaires ; rien ne les
 * empêchait de revenir. Ce fichier les transforme en tests.
 *
 * Tests **purs** : ils lisent la feuille de style livrée, sans instancier Qt. C'est ce qui les rend
 * exécutables dans `UnitTests`, qui ne lie pas Qt — un test qui exigerait une `QApplication` ne
 * tournerait ni ici ni en intégration continue.
 */

#include <fstream>
#include <regex>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

namespace {

[[nodiscard]] std::string readThemeTemplate() {
    // La feuille de l'EDITEUR, desormais la seule : celle de l'identite a disparu au LOT-86 avec
    // les ecrans du jeu, passes en QML. Ces garde-fous ne retrecissent pas pour autant -- ils
    // portaient sur des proprietes de widgets Qt, qui n'existent plus que dans l'editeur.
    std::ostringstream buffer;
    for (const char* const path : {JADG_THEME_EDITOR_PATH}) {
        std::ifstream file(path);
        buffer << file.rdbuf();
    }
    return buffer.str();
}

/// Retire les commentaires `/* … */` : ils citent volontairement les propriétés interdites pour
/// expliquer pourquoi elles le sont, et un test qui s'y déclencherait interdirait de documenter la
/// raison de l'interdiction.
[[nodiscard]] std::string withoutComments(const std::string& text) {
    static const std::regex commentPattern(R"(/\*[\s\S]*?\*/)");
    return std::regex_replace(text, commentPattern, "");
}

}  // namespace

/**
 * @brief Aucune propriété de **barre de titre de dock** dans la feuille de style. `titlebar-close-
 *        icon` et `titlebar-normal-icon` ont provoqué un plantage intermittent à la fermeture
 *        depuis l'éditeur pendant le `LOT-56` : Qt y détruit les boutons de barre de titre du dock
 *        au moment où la feuille de style prétend encore les habiller.
 *
 * Masquer un bouton de barre de titre se fait par les `features` du `QDockWidget`, jamais par la
 * feuille de style. L'invariant était écrit dans le guide de design ; il est désormais vérifié.
 * \castest{<b>La feuille de style ne porte aucune propriete de barre de titre de dock.</b><br/>
 * \tcat Unitaire · Theme de l'IHM<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Lire les feuilles de theme et en retirer les commentaires.<br/>2. Chercher les
 * proprietes de barre de titre de dock.<br/>
 * \tattendu Aucune n'apparait hors commentaire.
 * }
 */
TEST(ThemeTeardownGuardsTest, AucunePropieteDeBarreDeTitreDeDock) {
    const std::string theme = withoutComments(readThemeTemplate());
    ASSERT_FALSE(theme.empty()) << "feuilles de theme introuvables (JADG_THEME_*_PATH)";

    for (const char* forbidden : {"titlebar-close-icon", "titlebar-normal-icon"}) {
        EXPECT_EQ(theme.find(forbidden), std::string::npos)
            << forbidden
            << " : provoque un plantage intermittent a la fermeture depuis l'editeur (LOT-56). "
               "Masquer un bouton de barre de titre passe par les features du QDockWidget.";
    }
}

/**
 * @brief La feuille de style ne référence **aucun fichier externe**. Une `url(...)` pointant hors
 *        des ressources embarquées serait résolue à chaque repolissage — y compris pendant le
 *        démontage — et un chemin absent produit un comportement dépendant de la machine, donc une
 *        panne qui ne se reproduit pas chez celui qui la corrige.
 * \castest{<b>La feuille de style ne reference aucun fichier externe.</b><br/>
 * \tcat Unitaire · Theme de l'IHM<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Lire les feuilles de theme et en retirer les commentaires.<br/>2. Chercher un appel
 * url(...).<br/>
 * \tattendu Aucun, ou uniquement vers une ressource embarquee (prefixe deux-points).
 * }
 */
TEST(ThemeTeardownGuardsTest, AucuneReferenceDeFichierExterne) {
    const std::string theme = withoutComments(readThemeTemplate());
    ASSERT_FALSE(theme.empty());

    static const std::regex urlPattern(R"(url\(\s*([^)]*)\))");
    for (std::sregex_iterator it(theme.begin(), theme.end(), urlPattern), end; it != end; ++it) {
        const std::string target = (*it)[1].str();
        EXPECT_FALSE(target.empty());
        EXPECT_EQ(target.front(), ':')
            << "ressource externe referencee par la feuille de style : " << target;
    }
}
