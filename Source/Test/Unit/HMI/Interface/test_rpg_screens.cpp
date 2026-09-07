// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_rpg_screens.cpp
 * @brief Tests unitaires du catalogue des écrans du RPG et de leur ossature (`LOT-68`,
 *        `EX-IHM-090`, `EX-IHM-091`). Logique pure, sans Qt.
 */

#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>

#include <gtest/gtest.h>

#include "HMI/Interface/RpgScreens.h"
#include "HMI/Localization/Localization.h"

namespace {

using hmi::nextRpgScreen;
using hmi::pausesGame;
using hmi::previousRpgScreen;
using hmi::RpgBlockKind;
using hmi::RpgContentBlock;
using hmi::RpgScreenDescriptor;
using hmi::RpgScreenId;
using hmi::rpgScreens;

[[nodiscard]] std::unordered_map<std::string, std::string> readCatalog(const char* path) {
    std::ifstream file(path);
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return hmi::Localization::parseCatalog(buffer.str());
}

}  // namespace

/**
 * @brief Les huit écrans sont déclarés, et le cycle les traverse tous avant de revenir au premier.
 * \castest{<b>Le cycle de navigation traverse les huit ecrans et revient au premier.</b><br/>
 * \tcat Unitaire · Ecrans du RPG<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Partir du premier ecran.<br/>2. Appeler nextRpgScreen huit fois en notant chaque
 * ecran atteint.<br/>
 * \tattendu Les huit ecrans sont atteints une fois chacun, et le huitieme pas ramene au premier.
 * }
 */
TEST(RpgScreensTest, LeCycleTraverseLesHuitEcrans) {
    ASSERT_EQ(rpgScreens().size(), hmi::RPG_SCREEN_COUNT);

    const RpgScreenId first = rpgScreens().front().id;
    std::set<RpgScreenId> visited;
    RpgScreenId screen = first;
    for (std::size_t step = 0; step < hmi::RPG_SCREEN_COUNT; ++step) {
        visited.insert(screen);
        screen = nextRpgScreen(screen);
    }

    EXPECT_EQ(visited.size(), hmi::RPG_SCREEN_COUNT);
    EXPECT_EQ(screen, first);  // le cycle se referme : aucun cul-de-sac.
}

/**
 * @brief Le pas arrière est l'exact inverse du pas avant, aux extrémités comprises.
 * \castest{<b>Le pas arriere est l'inverse du pas avant.</b><br/>
 * \tcat Unitaire · Ecrans du RPG<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Pour chaque ecran, avancer puis reculer.<br/>2. Reculer depuis le premier
 * ecran.<br/>
 * \tattendu On retombe sur l'ecran de depart, et le premier ecran recule vers le dernier.
 * }
 */
TEST(RpgScreensTest, LePasArriereEstLInverseDuPasAvant) {
    for (const RpgScreenDescriptor& descriptor : rpgScreens()) {
        EXPECT_EQ(previousRpgScreen(nextRpgScreen(descriptor.id)), descriptor.id);
        EXPECT_EQ(nextRpgScreen(previousRpgScreen(descriptor.id)), descriptor.id);
    }
    EXPECT_EQ(previousRpgScreen(rpgScreens().front().id), rpgScreens().back().id);
}

/**
 * @brief La règle de superposition est celle décidée par le lot : la carte et l'ATH de combat se
 *        consultent en marchant, les six autres écrans suspendent la simulation (`EX-IHM-091`).
 * \castest{<b>La regle de superposition est celle attendue pour chacun des huit ecrans.</b><br/>
 * \tcat Unitaire · Ecrans du RPG<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Interroger pausesGame pour chacun des huit ecrans.<br/>
 * \tattendu La carte du monde et l'ATH de combat ne suspendent pas ; les six autres suspendent.
 * }
 */
TEST(RpgScreensTest, LaRegleDeSuperpositionEstCelleAttendue) {
    EXPECT_TRUE(pausesGame(RpgScreenId::CharacterSheet));
    EXPECT_TRUE(pausesGame(RpgScreenId::Inventory));
    EXPECT_TRUE(pausesGame(RpgScreenId::QuestJournal));
    EXPECT_TRUE(pausesGame(RpgScreenId::Dialogue));
    EXPECT_TRUE(pausesGame(RpgScreenId::Merchant));
    EXPECT_TRUE(pausesGame(RpgScreenId::GuildBoard));

    // On ouvre une carte pour savoir ou l'on va sans s'arreter ; et l'ATH de combat EST le jeu
    // pendant un combat, il ne se superpose a rien.
    EXPECT_FALSE(pausesGame(RpgScreenId::WorldMap));
    EXPECT_FALSE(pausesGame(RpgScreenId::CombatHud));
}

/**
 * @brief Chaque écran porte un nom d'objet et une clé de titre **uniques**, et une ossature non
 *        vide : sans quoi la feuille de style en habillerait deux pour un, ou l'écran s'ouvrirait
 *        sur du vide.
 * \castest{<b>Chaque ecran a un nom d'objet unique, un titre unique et une ossature non
 * vide.</b><br/>
 * \tcat Unitaire · Ecrans du RPG<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Collecter noms d'objets et cles de titre.<br/>2. Verifier leur unicite et la
 * presence d'au moins un bloc par ecran.<br/>
 * \tattendu Aucun doublon, aucune ossature vide.
 * }
 */
TEST(RpgScreensTest, ChaqueEcranEstIdentifiableEtNonVide) {
    std::set<std::string> objectNames;
    std::set<std::string> titleKeys;
    for (const RpgScreenDescriptor& descriptor : rpgScreens()) {
        EXPECT_TRUE(objectNames.insert(descriptor.objectName).second)
            << "nom d'objet duplique : " << descriptor.objectName;
        EXPECT_TRUE(titleKeys.insert(descriptor.titleKey).second)
            << "cle de titre dupliquee : " << descriptor.titleKey;
        EXPECT_FALSE(descriptor.layout.leftColumn.empty())
            << "ossature vide : " << descriptor.objectName;
        EXPECT_EQ(hmi::rpgScreenDescriptor(descriptor.id).id, descriptor.id);
    }
}

/**
 * @brief Chaque bloc est cohérent avec son genre : un bloc de champs porte des libellés, une
 *        grille des dimensions, une liste des lignes. Un genre renseigné à moitié ne se voit
 *        autrement qu'à l'écran, sous la forme d'un bloc vide qu'on prendrait pour une absence de
 *        contenu.
 * \castest{<b>Chaque bloc de l'ossature est coherent avec son genre.</b><br/>
 * \tcat Unitaire · Ecrans du RPG<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Parcourir les blocs des deux colonnes de chaque ecran.<br/>2. Verifier les
 * dimensions et libelles exiges par chaque genre.<br/>
 * \tattendu Aucun bloc n'est renseigne a moitie.
 * }
 */
TEST(RpgScreensTest, ChaqueBlocEstCoherentAvecSonGenre) {
    const auto check = [](const RpgContentBlock& block) {
        switch (block.kind) {
            case RpgBlockKind::Fields:
                EXPECT_FALSE(block.labelKeys.empty()) << "bloc de champs sans libelle";
                break;
            case RpgBlockKind::Grid:
                EXPECT_GT(block.columns, 0);
                EXPECT_GT(block.rows, 0);
                break;
            case RpgBlockKind::List:
                EXPECT_GT(block.rows, 0);
                break;
            case RpgBlockKind::Track:
            case RpgBlockKind::ActionBar:
                EXPECT_GT(block.columns, 0);
                break;
            case RpgBlockKind::Prose:
            case RpgBlockKind::Portrait:
                break;  // aucune dimension a declarer : ces deux blocs prennent la place offerte.
        }
    };

    for (const RpgScreenDescriptor& descriptor : rpgScreens()) {
        for (const RpgContentBlock& block : descriptor.layout.leftColumn) {
            check(block);
        }
        for (const RpgContentBlock& block : descriptor.layout.rightColumn) {
            check(block);
        }
    }
}

/**
 * @brief Toutes les clés de l'ossature — titres d'écrans, titres de blocs, libellés de champs —
 *        existent dans les **deux** catalogues livrés (`EX-REN-033`).
 *
 * L'ossature étant une table, une clé s'y ajoute sans qu'aucun `.ui` ni aucun appel n'en garde la
 * trace : rien, sinon ce test, ne dirait qu'elle n'a jamais été traduite. Le défaut s'afficherait
 * comme la clé elle-même au milieu d'une feuille de personnage.
 * \castest{<b>Chaque cle de l'ossature des ecrans du RPG existe en francais et en
 * anglais.</b><br/>
 * \tcat Unitaire · Ecrans du RPG<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Lire fr.lang et en.lang.<br/>2. Verifier chaque cle de titre, de bloc et de champ,
 * plus les cles du pied d'actions.<br/>
 * \tattendu Aucune cle n'est absente de l'un ou l'autre catalogue.
 * }
 */
TEST(RpgScreensTest, ChaqueCleDeLOssatureExisteDansLesDeuxLangues) {
    const std::unordered_map<std::string, std::string> fr = readCatalog(JADG_FR_LANG_PATH);
    const std::unordered_map<std::string, std::string> en = readCatalog(JADG_EN_LANG_PATH);
    ASSERT_FALSE(fr.empty());
    ASSERT_FALSE(en.empty());

    const auto expectKey = [&fr, &en](const std::string& key) {
        EXPECT_TRUE(fr.count(key) > 0) << "cle absente de fr.lang : " << key;
        EXPECT_TRUE(en.count(key) > 0) << "cle absente de en.lang : " << key;
    };

    const auto checkBlock = [&expectKey](const RpgContentBlock& block) {
        if (block.titleKey[0] != '\0') {
            expectKey(block.titleKey);
        }
        for (const char* const label : block.labelKeys) {
            expectKey(label);
        }
    };

    for (const RpgScreenDescriptor& descriptor : rpgScreens()) {
        expectKey(descriptor.titleKey);
        for (const RpgContentBlock& block : descriptor.layout.leftColumn) {
            checkBlock(block);
        }
        for (const RpgContentBlock& block : descriptor.layout.rightColumn) {
            checkBlock(block);
        }
    }

    // Le pied d'actions et ses rappels de touches : communs aux huit ecrans, donc absents de la
    // table, donc invisibles a la boucle ci-dessus.
    for (const char* const key :
         {"rpg.empty", "rpg.chassis.close", "rpg.chassis.previous_screen",
          "rpg.chassis.next_screen", "key.shoulders", "hint.change_screen"}) {
        expectKey(key);
    }
}
