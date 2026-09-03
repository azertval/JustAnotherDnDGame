// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_mechanism_controller.cpp
 * @brief Tests unitaires des mécanismes interrupteur↔porte (`MechanismController`).
 */

#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Gameplay/MechanismController.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"

namespace {

// Niveau minimal 6×3 : un interrupteur en (2,1) lié à une porte en (4,1).
core::Level makeLevelWithMechanism() {
    core::TileMap map(6, 3);
    map.setTile(2, 1, core::TileType::Switch);
    map.setTile(4, 1, core::TileType::Door);
    std::vector<core::Mechanism> mechanisms{
        core::Mechanism{core::GridPosition{2, 1}, core::GridPosition{4, 1}}};
    return core::Level("puzzle", std::move(map), core::GridPosition{0, 0}, core::GridPosition{5, 2},
                       std::move(mechanisms));
}

// Boîte 1×1 posée sur la case (column, row).
core::Aabb boxAt(int column, int row) {
    return core::Aabb::fromTopLeftSize(
        core::Vector2{static_cast<float>(column), static_cast<float>(row)},
        core::Vector2{1.0f, 1.0f});
}

// Niveau minimal 6×3 : une plaque de pression en (2,1) liée à une porte en (4,1).
core::Level makeLevelWithPressurePlate() {
    core::TileMap map(6, 3);
    map.setTile(2, 1, core::TileType::PressurePlate);
    map.setTile(4, 1, core::TileType::Door);
    std::vector<core::Mechanism> mechanisms{
        core::Mechanism{core::GridPosition{2, 1}, core::GridPosition{4, 1}}};
    return core::Level("puzzle-poids", std::move(map), core::GridPosition{0, 0},
                       core::GridPosition{5, 2}, std::move(mechanisms));
}

// Niveau minimal 6×3 : une clé en (2,1) liée à une porte verrouillée en (4,1) (EX-GP-023).
core::Level makeLevelWithKeyAndLockedDoor() {
    core::TileMap map(6, 3);
    map.setTile(2, 1, core::TileType::Key);
    map.setTile(4, 1, core::TileType::LockedDoor);
    std::vector<core::Mechanism> mechanisms{
        core::Mechanism{core::GridPosition{2, 1}, core::GridPosition{4, 1}}};
    return core::Level("puzzle-cle", std::move(map), core::GridPosition{0, 0},
                       core::GridPosition{5, 2}, std::move(mechanisms));
}

// Niveau minimal 8×3 : deux paires clé/porte verrouillée independantes.
core::Level makeLevelWithTwoKeyDoorPairs() {
    core::TileMap map(8, 3);
    map.setTile(1, 1, core::TileType::Key);
    map.setTile(2, 1, core::TileType::LockedDoor);
    map.setTile(5, 1, core::TileType::Key);
    map.setTile(6, 1, core::TileType::LockedDoor);
    std::vector<core::Mechanism> mechanisms{
        core::Mechanism{core::GridPosition{1, 1}, core::GridPosition{2, 1}},
        core::Mechanism{core::GridPosition{5, 1}, core::GridPosition{6, 1}}};
    return core::Level("puzzle-deux-cles", std::move(map), core::GridPosition{0, 0},
                       core::GridPosition{7, 2}, std::move(mechanisms));
}

}  // namespace

/**
 * @brief Au départ, la porte est **fermée** (solide) ; toucher l'interrupteur l'**ouvre**.
 * \castest{<b>Au départ, la porte est **fermée** (solide) ; toucher l'interrupteur
 * l'**ouvre**.</b><br/>
 * \tcat Unitaire · Mechanism Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Au départ, la porte est **fermée** (solide) ; toucher l'interrupteur l'**ouvre**.
 * }
 */
TEST(MechanismControllerTest, ContactOuvreLaPorte) {
    core::MechanismController controller(makeLevelWithMechanism());
    EXPECT_TRUE(controller.collisionMap().isSolid(4, 1));  // porte fermée = solide
    EXPECT_FALSE(controller.isDoorOpen(0));

    controller.update(boxAt(2, 1));                         // le personnage touche l'interrupteur
    EXPECT_FALSE(controller.collisionMap().isSolid(4, 1));  // porte ouverte = franchissable
    EXPECT_TRUE(controller.isDoorOpen(0));
}

/**
 * @brief La bascule est **sur front** : rester sur l'interrupteur ne re-bascule pas ; revenir
 * bascule.
 * \castest{<b>La bascule est **sur front** : rester sur l'interrupteur ne re-bascule pas ; revenir
 * bascule.</b><br/>
 * \tcat Unitaire · Mechanism Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La bascule est **sur front** : rester sur l'interrupteur ne re-bascule pas ; revenir
 * bascule.
 * }
 */
TEST(MechanismControllerTest, BasculeSurFront) {
    core::MechanismController controller(makeLevelWithMechanism());

    controller.update(boxAt(2, 1));  // 1er contact : ouvre
    EXPECT_TRUE(controller.isDoorOpen(0));
    controller.update(boxAt(2, 1));  // toujours dessus : pas de re-bascule
    EXPECT_TRUE(controller.isDoorOpen(0));

    controller.update(boxAt(0, 1));  // quitte l'interrupteur : rien
    EXPECT_TRUE(controller.isDoorOpen(0));
    controller.update(boxAt(2, 1));  // revient : re-bascule (ferme)
    EXPECT_FALSE(controller.isDoorOpen(0));
    EXPECT_TRUE(controller.collisionMap().isSolid(4, 1));
}

/**
 * @brief Loin de l'interrupteur, rien ne change (la porte reste fermée).
 * \castest{<b>Loin de l'interrupteur, rien ne change (la porte reste fermée).</b><br/>
 * \tcat Unitaire · Mechanism Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Loin de l'interrupteur, rien ne change (la porte reste fermée).
 * }
 */
TEST(MechanismControllerTest, SansContactRienNeChange) {
    core::MechanismController controller(makeLevelWithMechanism());
    controller.update(boxAt(0, 0));
    EXPECT_TRUE(controller.collisionMap().isSolid(4, 1));
    EXPECT_FALSE(controller.isDoorOpen(0));
}

/**
 * @brief Une plaque de pression ouvre la porte tant que le poids y repose, et la referme dès
 * qu'il en part — activation continue, sans effet de front (`EX-GP-025`).
 * \castest{<b>Une plaque de pression ouvre la porte tant que le poids y repose, et la referme dès
 * qu'il en part.</b><br/>
 * \tcat Unitaire · Mechanism Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une plaque de pression ouvre la porte tant que le poids y repose, et la referme dès
 * qu'il en part.
 * }
 */
TEST(MechanismControllerTest, PlaqueDePressionActivationContinue) {
    core::MechanismController controller(makeLevelWithPressurePlate());
    EXPECT_TRUE(controller.collisionMap().isSolid(4, 1));  // porte fermee au depart

    controller.update(boxAt(2, 1), 1.0f);  // poids suffisant sur la plaque
    EXPECT_TRUE(controller.isDoorOpen(0));
    EXPECT_FALSE(controller.collisionMap().isSolid(4, 1));

    controller.update(boxAt(2, 1), 1.0f);  // reste dessus : reste ouverte (pas de bascule)
    EXPECT_TRUE(controller.isDoorOpen(0));

    controller.update(boxAt(0, 1), 1.0f);  // quitte la plaque : se referme immediatement
    EXPECT_FALSE(controller.isDoorOpen(0));
    EXPECT_TRUE(controller.collisionMap().isSolid(4, 1));

    controller.update(boxAt(2, 1), 1.0f);  // revient : se rouvre (pas de bascule inversee)
    EXPECT_TRUE(controller.isDoorOpen(0));
}

/**
 * @brief Un poids insuffisant sur la plaque de pression n'ouvre pas la porte.
 * \castest{<b>Un poids insuffisant sur la plaque de pression n'ouvre pas la porte.</b><br/>
 * \tcat Unitaire · Mechanism Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un poids insuffisant sur la plaque de pression n'ouvre pas la porte.
 * }
 */
TEST(MechanismControllerTest, PlaqueDePressionPoidsInsuffisant) {
    core::MechanismController controller(makeLevelWithPressurePlate());
    controller.update(boxAt(2, 1), 0.1f);  // bien en dessous du seuil
    EXPECT_FALSE(controller.isDoorOpen(0));
    EXPECT_TRUE(controller.collisionMap().isSolid(4, 1));
}


/**
 * @brief Le simple contact sur la clé, sans « Interagir », n'ouvre pas la porte verrouillée —
 * contrairement à l'interrupteur (`EX-GP-023`, `EX-CTRL-022`).
 * \castest{<b>Le simple contact sur la clé n'ouvre pas la porte verrouillée.</b><br/>
 * \tcat Unitaire · Mechanism Controller<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le contact seul (sans Interagir) n'ouvre pas la porte verrouillée.
 * }
 */
TEST(MechanismControllerTest, ContactSeulNOuvrePasLaPorteVerrouillee) {
    core::MechanismController controller(makeLevelWithKeyAndLockedDoor());
    controller.update(boxAt(2, 1), 1.0f, /*interactPressed=*/false);
    EXPECT_FALSE(controller.isDoorOpen(0));
    EXPECT_TRUE(controller.collisionMap().isSolid(4, 1));
}

/**
 * @brief Contact + « Interagir » ramasse la clé et ouvre définitivement la porte verrouillée.
 * \castest{<b>Contact + Interagir ramasse la clé et ouvre définitivement la porte
 * verrouillée.</b><br/>
 * \tcat Unitaire · Mechanism Controller<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La porte s'ouvre au contact + Interagir, et reste ouverte quoi qu'il arrive ensuite.
 * }
 */
TEST(MechanismControllerTest, ContactEtInteragirRamasseLaCleEtOuvreDefinitivement) {
    core::MechanismController controller(makeLevelWithKeyAndLockedDoor());
    EXPECT_TRUE(controller.collisionMap().isSolid(4, 1));  // porte fermee au depart

    controller.update(boxAt(2, 1), 1.0f, /*interactPressed=*/true);
    EXPECT_TRUE(controller.isDoorOpen(0));
    EXPECT_FALSE(controller.collisionMap().isSolid(4, 1));

    // Contrairement a l'interrupteur : quitter la case, ou rappuyer sur Interagir, ne referme
    // jamais la porte -- la cle est CONSOMMEE, pas basculee.
    controller.update(boxAt(0, 1), 1.0f, /*interactPressed=*/false);
    EXPECT_TRUE(controller.isDoorOpen(0));
    EXPECT_FALSE(controller.collisionMap().isSolid(4, 1));

    controller.update(boxAt(2, 1), 1.0f, /*interactPressed=*/true);  // revient, rappuie
    EXPECT_TRUE(controller.isDoorOpen(0));
    EXPECT_FALSE(controller.collisionMap().isSolid(4, 1));
}

/**
 * @brief « Interagir » sans être au contact de la clé ne ramasse rien.
 * \castest{<b>Interagir sans contact ne ramasse pas la clé.</b><br/>
 * \tcat Unitaire · Mechanism Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Interagir loin de la clé n'a aucun effet.
 * }
 */
TEST(MechanismControllerTest, InteragirSansContactNeRamassePasLaCle) {
    core::MechanismController controller(makeLevelWithKeyAndLockedDoor());
    controller.update(boxAt(0, 0), 1.0f, /*interactPressed=*/true);
    EXPECT_FALSE(controller.isDoorOpen(0));
    EXPECT_TRUE(controller.collisionMap().isSolid(4, 1));
}

/**
 * @brief Deux paires clé/porte verrouillée indépendantes ne s'influencent pas : ramasser l'une
 * n'ouvre pas l'autre.
 * \castest{<b>Deux paires clé/porte verrouillée indépendantes ne s'influencent pas.</b><br/>
 * \tcat Unitaire · Mechanism Controller<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Ramasser la première clé ouvre uniquement sa porte, l'autre paire reste fermée.
 * }
 */
TEST(MechanismControllerTest, DeuxPairesCleEtPorteIndependantes) {
    core::MechanismController controller(makeLevelWithTwoKeyDoorPairs());

    controller.update(boxAt(1, 1), 1.0f, /*interactPressed=*/true);  // ramasse la premiere cle
    EXPECT_TRUE(controller.isDoorOpen(0));
    EXPECT_FALSE(controller.isDoorOpen(1));
    EXPECT_FALSE(controller.collisionMap().isSolid(2, 1));
    EXPECT_TRUE(controller.collisionMap().isSolid(6, 1));

    controller.update(boxAt(5, 1), 1.0f, /*interactPressed=*/true);  // ramasse la seconde cle
    EXPECT_TRUE(controller.isDoorOpen(0));
    EXPECT_TRUE(controller.isDoorOpen(1));
    EXPECT_FALSE(controller.collisionMap().isSolid(6, 1));
}

/**
 * @brief Reconstruire le contrôleur (rechargement du niveau) remet la clé et la porte dans leur
 * état initial (`EX-GP-024`, même principe que le budget de mouvements).
 * \castest{<b>Reconstruire le contrôleur remet la clé et la porte dans leur état initial.</b><br/>
 * \tcat Unitaire · Mechanism Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un nouveau contrôleur construit sur le même niveau redémarre porte fermée, clé non
 * ramassée.
 * }
 */
TEST(MechanismControllerTest, RechargementReinitialiseCleEtPorte) {
    const core::Level level = makeLevelWithKeyAndLockedDoor();
    core::MechanismController first(level);
    first.update(boxAt(2, 1), 1.0f, /*interactPressed=*/true);
    ASSERT_TRUE(first.isDoorOpen(0));

    core::MechanismController reloaded(level);  // simule le rechargement du niveau
    EXPECT_FALSE(reloaded.isDoorOpen(0));
    EXPECT_TRUE(reloaded.collisionMap().isSolid(4, 1));
}
