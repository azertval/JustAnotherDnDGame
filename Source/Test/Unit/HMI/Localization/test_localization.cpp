// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_localization.cpp
 * @brief Tests unitaires du catalogue de traduction : analyse, repli, changement de langue.
 */

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

#include <gtest/gtest.h>

#include "HMI/Localization/Localization.h"

namespace {

// Contenu brut d'un catalogue livre. Les chemins viennent de Test/CMakeLists.txt : le test
// verifie les fichiers REELLEMENT livres, jamais une copie de test qui pourrait en diverger.
[[nodiscard]] std::string readFile(const char* path) {
    std::ifstream file(path);
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

}  // namespace

/**
 * @brief L'analyse ignore les lignes vides et les commentaires, et retire les espaces autour de
 * '='.
 * \castest{<b>L'analyse ignore les lignes vides et les commentaires, et retire les espaces autour
 * de '='.</b><br/>
 * \tcat Unitaire · Localization<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu L'analyse ignore les lignes vides et les commentaires, et retire les espaces autour de
 * '='.
 * }
 */
TEST(LocalizationTest, AnalyseIgnoreCommentairesEtEspaces) {
    const auto strings = hmi::Localization::parseCatalog(
        "# commentaire\n"
        "\n"
        "menu.quitter = Quitter\n"
        "  menu.titre   =   JustAnotherDnDGame  \n"
        "# autre commentaire\n");

    ASSERT_EQ(strings.size(), 2u);
    EXPECT_EQ(strings.at("menu.quitter"), "Quitter");
    EXPECT_EQ(strings.at("menu.titre"), "JustAnotherDnDGame");
}

/**
 * @brief Seul le premier '=' sépare ; un '=' dans la valeur est conservé.
 * \castest{<b>Seul le premier '=' sépare ; un '=' dans la valeur est conservé.</b><br/>
 * \tcat Unitaire · Localization<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Seul le premier '=' sépare ; un '=' dans la valeur est conservé.
 * }
 */
TEST(LocalizationTest, AnalyseConserveEgalDansLaValeur) {
    const auto strings = hmi::Localization::parseCatalog("expression = a = b + c\n");

    ASSERT_EQ(strings.size(), 1u);
    EXPECT_EQ(strings.at("expression"), "a = b + c");
}

/**
 * @brief Une clé existante est résolue dans la langue active.
 * \castest{<b>Une clé existante est résolue dans la langue active.</b><br/>
 * \tcat Unitaire · Localization<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une clé existante est résolue dans la langue active.
 * }
 */
TEST(LocalizationTest, CleExistanteResolue) {
    hmi::Localization localization;
    localization.setDefaultCatalog("fr", {{"menu.quitter", "Quitter"}});

    EXPECT_EQ(localization.text("menu.quitter"), "Quitter");
    EXPECT_EQ(localization.activeLanguage(), "fr");
}

/**
 * @brief Une clé inconnue partout est renvoyée telle quelle (repli déterministe, pas de plantage).
 * \castest{<b>Une clé inconnue partout est renvoyée telle quelle (repli déterministe, pas de
 * plantage).</b><br/>
 * \tcat Unitaire · Localization<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une clé inconnue partout est renvoyée telle quelle (repli déterministe, pas de
 * plantage).
 * }
 */
TEST(LocalizationTest, CleInconnueRenvoieLaCle) {
    hmi::Localization localization;
    localization.setDefaultCatalog("fr", {{"menu.quitter", "Quitter"}});

    EXPECT_EQ(localization.text("menu.inconnue"), "menu.inconnue");
}

/**
 * @brief Changer de langue résout les valeurs de la nouvelle langue.
 * \castest{<b>Changer de langue résout les valeurs de la nouvelle langue.</b><br/>
 * \tcat Unitaire · Localization<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Changer de langue résout les valeurs de la nouvelle langue.
 * }
 */
TEST(LocalizationTest, ChangementDeLangue) {
    hmi::Localization localization;
    localization.setDefaultCatalog("fr", {{"menu.quitter", "Quitter"}});
    localization.setActiveCatalog("en", {{"menu.quitter", "Quit"}});

    EXPECT_EQ(localization.activeLanguage(), "en");
    EXPECT_EQ(localization.text("menu.quitter"), "Quit");
}

/**
 * @brief Une clé manquante dans la langue active retombe sur la langue par défaut.
 * \castest{<b>Une clé manquante dans la langue active retombe sur la langue par défaut.</b><br/>
 * \tcat Unitaire · Localization<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une clé manquante dans la langue active retombe sur la langue par défaut.
 * }
 */
TEST(LocalizationTest, RepliSurLangueParDefaut) {
    hmi::Localization localization;
    localization.setDefaultCatalog("fr", {{"menu.quitter", "Quitter"}, {"menu.titre", "Jeu"}});
    localization.setActiveCatalog("en", {{"menu.quitter", "Quit"}});  // pas de menu.titre

    EXPECT_EQ(localization.text("menu.quitter"), "Quit");  // langue active
    EXPECT_EQ(localization.text("menu.titre"), "Jeu");     // repli langue par défaut
}

/**
 * @brief Charger une langue absente échoue proprement et conserve la langue active (récupérable).
 * \castest{<b>Charger une langue absente échoue proprement et conserve la langue active
 * (récupérable).</b><br/>
 * \tcat Unitaire · Localization<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Charger une langue absente échoue proprement et conserve la langue active
 * (récupérable).
 * }
 */
TEST(LocalizationTest, LangueAbsenteEstRecuperable) {
    hmi::Localization localization("dossier/inexistant");
    localization.setDefaultCatalog("fr", {{"menu.quitter", "Quitter"}});

    EXPECT_FALSE(localization.loadLanguage("xx"));
    EXPECT_EQ(localization.activeLanguage(), "fr");
    EXPECT_EQ(localization.text("menu.quitter"), "Quitter");
}

/**
 * @brief Le catalogue français livré (Source/Elements/Localization) se charge et résout ses clés.
 * \castest{<b>Le catalogue français livré (Source/Elements/Localization) se charge et résout ses
 * clés.</b><br/>
 * \tcat Unitaire · Localization<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le catalogue français livré (Source/Elements/Localization) se charge et résout ses
 * clés.
 * }
 */
TEST(LocalizationTest, CatalogueFrancaisLivreSeCharge) {
    hmi::Localization localization(std::filesystem::path(JADG_LOCALIZATION_DIR));

    ASSERT_TRUE(localization.loadDefaultLanguage("fr"));
    EXPECT_EQ(localization.activeLanguage(), "fr");
    EXPECT_EQ(localization.text("menu.quit"), "Quitter");
    EXPECT_EQ(localization.text("menu.new_game"), "Nouvelle partie");
}

/**
 * @brief Les deux catalogues livrés déclarent **exactement** les mêmes clés (`LOT-73`,
 *        `EX-REN-033`).
 *
 * Une clé ajoutée d'un seul côté ne casse rien de visible : `Localization::text` replie sur la
 * langue par défaut, et l'interface s'affiche — en français au milieu d'un écran anglais. Le
 * défaut ne se voit donc qu'en changeant de langue, écran par écran, ce que personne ne fait
 * après avoir ajouté un libellé. Ce test le voit pour nous, dans les deux sens : une clé
 * anglaise oubliée en français compte autant.
 * \castest{<b>Les catalogues francais et anglais declarent les memes cles.</b><br/>
 * \tcat Unitaire · Localization<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger fr.lang et en.lang.<br/>2. Comparer les ensembles de cles dans les
 * deux sens.<br/>
 * \tattendu Aucune cle ne manque a l'un ni a l'autre.
 * }
 */
TEST(LocalizationTest, LesDeuxCataloguesDeclarentLesMemesCles) {
    const std::unordered_map<std::string, std::string> french =
        hmi::Localization::parseCatalog(readFile(JADG_FR_LANG_PATH));
    const std::unordered_map<std::string, std::string> english =
        hmi::Localization::parseCatalog(readFile(JADG_EN_LANG_PATH));
    ASSERT_FALSE(french.empty());
    ASSERT_FALSE(english.empty());

    for (const auto& [key, value] : french) {
        EXPECT_TRUE(english.count(key) > 0) << "cle absente de en.lang : " << key;
    }
    for (const auto& [key, value] : english) {
        EXPECT_TRUE(french.count(key) > 0) << "cle absente de fr.lang : " << key;
    }
    EXPECT_EQ(french.size(), english.size());
}
