// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_playtest_interaction.cpp
 * @brief Tests unitaires du compte rendu d'interaction de l'essai immédiat de l'éditeur (LOT-11).
 *        Fonction pure, sans GPU.
 */

#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include "Core/Levels/MapEntity.h"
#include "Core/Rpg/Dialogue.h"
#include "Core/World/EntityKinds.h"
#include "HMI/Game/PlaytestInteraction.h"
#include "HMI/Localization/Localization.h"

namespace {

core::MapEntity entity(std::string type) {
    core::MapEntity result;
    result.type = std::move(type);
    result.position = core::GridPosition{3, 4};
    return result;
}

hmi::PlaytestMessage message(std::string_view key, std::vector<std::string> args = {}) {
    return hmi::PlaytestMessage{std::string{key}, std::move(args)};
}

}  // namespace

/**
 * @brief Un coffre s'ouvre une fois, puis se dit déjà ouvert.
 * \castest{<b>Le coffre se dit ouvert, puis deja ouvert.</b><br/>
 * \tcat Unitaire · Essai immediat<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Demander le compte rendu d'un coffre non consomme, puis consomme.<br/>
 * \tattendu chest_opened puis chest_already_opened.
 * }
 */
TEST(PlaytestInteractionTest, CoffreOuvertPuisDejaOuvert) {
    EXPECT_EQ(hmi::playtestInteractionMessage(entity("chest"), false),
              message(hmi::PLAYTEST_CHEST_OPENED_KEY));
    EXPECT_EQ(hmi::playtestInteractionMessage(entity("chest"), true),
              message(hmi::PLAYTEST_CHEST_ALREADY_OPENED_KEY));
}

/**
 * @brief Un panneau se lit, consommé ou non.
 * \castest{<b>Le panneau se lit.</b><br/>
 * \tcat Unitaire · Essai immediat<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Demander le compte rendu d'un panneau.<br/>
 * \tattendu sign_read.
 * }
 */
TEST(PlaytestInteractionTest, PanneauLu) {
    EXPECT_EQ(hmi::playtestInteractionMessage(entity("sign"), false),
              message(hmi::PLAYTEST_SIGN_READ_KEY));
}

/**
 * @brief Un PNJ nomme son dialogue ; sans dialogue (absent, vide ou mal typé), il se dit muet.
 * \castest{<b>Le PNJ nomme son dialogue, ou se dit sans dialogue.</b><br/>
 * \tcat Unitaire · Essai immediat<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. PNJ avec dialogue "herault".<br/>2. PNJ sans propriete, avec texte vide, avec
 * entier.<br/>
 * \tattendu npc_dialogue("herault") puis npc_silent trois fois.
 * }
 */
TEST(PlaytestInteractionTest, PnjDialogueOuSansDialogue) {
    core::MapEntity pnj = entity(std::string{core::NPC_ENTITY_TYPE});
    pnj.properties[std::string{core::NPC_DIALOGUE_PROPERTY}] = std::string{"herault"};
    EXPECT_EQ(hmi::playtestInteractionMessage(pnj, false),
              message(hmi::PLAYTEST_NPC_DIALOGUE_KEY, {"herault"}));

    core::MapEntity muet = entity(std::string{core::NPC_ENTITY_TYPE});
    EXPECT_EQ(hmi::playtestInteractionMessage(muet, false), message(hmi::PLAYTEST_NPC_SILENT_KEY));
    muet.properties[std::string{core::NPC_DIALOGUE_PROPERTY}] = std::string{};
    EXPECT_EQ(hmi::playtestInteractionMessage(muet, false), message(hmi::PLAYTEST_NPC_SILENT_KEY));
    muet.properties[std::string{core::NPC_DIALOGUE_PROPERTY}] = std::int64_t{7};
    EXPECT_EQ(hmi::playtestInteractionMessage(muet, false), message(hmi::PLAYTEST_NPC_SILENT_KEY));
}

/**
 * @brief Un portail dit sa carte cible et son arrivée ; incomplet, il ne promet rien.
 * \castest{<b>Le portail nomme sa destination, ou se dit incomplet.</b><br/>
 * \tcat Unitaire · Essai immediat<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Portail complet.<br/>2. Portail sans arrivee, puis sans carte cible.<br/>
 * \tattendu portal(foret, lisiere) puis portal_incomplete deux fois.
 * }
 */
TEST(PlaytestInteractionTest, PortailDestinationOuIncomplet) {
    core::MapEntity portail = entity(std::string{core::PORTAL_ENTITY_TYPE});
    portail.properties[std::string{core::PORTAL_TARGET_MAP_PROPERTY}] = std::string{"foret"};
    EXPECT_EQ(hmi::playtestInteractionMessage(portail, false),
              message(hmi::PLAYTEST_PORTAL_INCOMPLETE_KEY));

    portail.properties[std::string{core::PORTAL_ARRIVAL_PROPERTY}] = std::string{"lisiere"};
    EXPECT_EQ(hmi::playtestInteractionMessage(portail, false),
              message(hmi::PLAYTEST_PORTAL_KEY, {"foret", "lisiere"}));

    portail.properties.erase(std::string{core::PORTAL_TARGET_MAP_PROPERTY});
    EXPECT_EQ(hmi::playtestInteractionMessage(portail, false),
              message(hmi::PLAYTEST_PORTAL_INCOMPLETE_KEY));
}

/**
 * @brief Un autre type se nomme, sans rien promettre.
 * \castest{<b>Un type sans interaction se nomme.</b><br/>
 * \tcat Unitaire · Essai immediat<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Demander le compte rendu d'une entite "altar".<br/>
 * \tattendu other("altar").
 * }
 */
TEST(PlaytestInteractionTest, AutreTypeSeNomme) {
    EXPECT_EQ(hmi::playtestInteractionMessage(entity("altar"), false),
              message(hmi::PLAYTEST_OTHER_KEY, {"altar"}));
}

/**
 * @brief La traduction remplace `%1` et `%2` par les valeurs, dans l'ordre.
 * \castest{<b>Le compte rendu se traduit avec ses valeurs.</b><br/>
 * \tcat Unitaire · Essai immediat<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Traduire un compte rendu a deux valeurs.<br/>
 * \tattendu Le texte, %1 et %2 remplaces.
 * }
 */
TEST(PlaytestInteractionTest, TraductionRemplaceLesValeurs) {
    hmi::Localization localization;
    localization.setDefaultCatalog(
        "fr", {{std::string{hmi::PLAYTEST_PORTAL_KEY}, "Vers %1, arrivee %2 (%1)"}});
    EXPECT_EQ(hmi::formatPlaytestMessage(message(hmi::PLAYTEST_PORTAL_KEY, {"foret", "lisiere"}),
                                         localization),
              "Vers foret, arrivee lisiere (%1)");
}

/**
 * @brief La durée d'affichage se compte en pas : trois secondes à 60 pas par seconde font 180 pas.
 * \castest{<b>La duree d'un compte rendu se compte en pas de simulation.</b><br/>
 * \tcat Unitaire · Essai immediat<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Convertir pour un pas de 1/60 s, de 10 s, et nul.<br/>
 * \tattendu 180, 1 et 1.
 * }
 */
TEST(PlaytestInteractionTest, DureeEnPasDeSimulation) {
    EXPECT_EQ(hmi::playtestMessageSteps(1.0f / 60.0f), 180);
    EXPECT_EQ(hmi::playtestMessageSteps(10.0f), 1);
    EXPECT_EQ(hmi::playtestMessageSteps(0.0f), 1);
}

/**
 * @brief Chaque clé de compte rendu existe, traduite, dans les deux catalogues livrés.
 * \castest{<b>Les cles de l'essai immediat existent dans les deux catalogues livres.</b><br/>
 * \tcat Unitaire · Essai immediat<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger fr.lang puis en.lang.<br/>2. Resoudre chaque cle.<br/>
 * \tattendu Chaque cle resout vers un texte distinct de la cle.
 * }
 */
TEST(PlaytestInteractionTest, ClesDeTraductionExistentDansLesDeuxCatalogues) {
    const std::filesystem::path directory(JADG_LOCALIZATION_DIR);
    for (const std::string& language : {"fr", "en"}) {
        hmi::Localization localization(directory);
        ASSERT_TRUE(localization.loadDefaultLanguage(language)) << language;
        for (const std::string_view key :
             {hmi::PLAYTEST_CHEST_OPENED_KEY, hmi::PLAYTEST_CHEST_ALREADY_OPENED_KEY,
              hmi::PLAYTEST_SIGN_READ_KEY, hmi::PLAYTEST_NPC_DIALOGUE_KEY,
              hmi::PLAYTEST_NPC_SILENT_KEY, hmi::PLAYTEST_PORTAL_KEY,
              hmi::PLAYTEST_PORTAL_INCOMPLETE_KEY, hmi::PLAYTEST_OTHER_KEY}) {
            EXPECT_NE(localization.text(key), key) << language << " " << key;
        }
    }
}
