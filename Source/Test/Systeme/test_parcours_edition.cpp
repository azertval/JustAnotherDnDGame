// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_parcours_edition.cpp
 * @brief Parcours système : créer un niveau dans l'éditeur, l'enregistrer, le recharger, et
 *        vérifier qu'il est immédiatement jouable — sans la couche GPU (LOT-14).
 */

#include <filesystem>

#include <gtest/gtest.h>

#include "Core/Levels/CameraFraming.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelDraft.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Levels/LevelWriter.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"


/**
 * @brief Parcours complet d'édition du cadrage de caméra : choisir un mode, l'annuler, en choisir
 * un autre avec une taille de salle personnalisée, enregistrer, recharger -- exactement le geste
 * du sélecteur de la section « Cadrage » (`EX-EDIT-028`, LOT-64).
 * \castest{<b>Parcours complet d'édition du cadrage de caméra : choisir, annuler, enregistrer,
 * recharger.</b><br/>
 * \tcat Système · Éditeur de niveaux<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Choisir le mode *suivi* sur un brouillon.<br/>2. Annuler (`undo`) : retour au
 * cadrage par défaut.<br/>3. Choisir le mode *par salle* avec une taille personnalisée.<br/>4.
 * Enregistrer sur disque, recharger.<br/>
 * \tattendu Le niveau rechargé restitue exactement le mode et la taille de salle choisis en
 * dernier ; l'annulation intermédiaire n'y figure pas (historique linéaire normal).
 * }
 */
TEST(ParcoursEditionSysteme, EditeAnnuleEnregistreEtRechargeLeCadrageDeCamera) {
    // 1. Le level designer choisit le mode "suivi" dans la section "Cadrage".
    core::LevelDraft draft = core::LevelDraft::empty("Cadrage (edition)", 30, 20);
    draft.setEntry(0, 0);
    draft.setExit(29, 19);
    draft.setCameraFraming(core::CameraFramingConfig{.mode = core::CameraFramingMode::Follow});
    EXPECT_EQ(draft.cameraFraming().mode, core::CameraFramingMode::Follow);

    // 2. Changement d'avis, annulé (EX-EDIT-005) : restitue le cadrage par défaut d'un brouillon
    // vierge (LevelDraft::empty, niveau entier) -- valeur de départ du champ, pas la règle de
    // repli d'EX-LVL-006 (qui ne s'applique qu'au CHARGEMENT d'un fichier sans champ déclaré).
    ASSERT_TRUE(draft.undo());
    EXPECT_EQ(draft.cameraFraming().mode, core::CameraFramingMode::WholeLevel);

    // 3. Choix definitif : mode "par salle" avec une taille de salle personnalisee.
    draft.setCameraFraming(core::CameraFramingConfig{
        .mode = core::CameraFramingMode::PerRoom, .roomWidthTiles = 10, .roomHeightTiles = 8});

    // 4. Enregistrement puis rechargement (round-trip disque, EX-EDIT-011).
    const core::LevelLoadResult validated = draft.toLevel();
    ASSERT_TRUE(validated.ok()) << validated.error;

    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "jadg_systeme_edition_cadrage.json";
    ASSERT_TRUE(core::LevelWriter::saveToFile(*validated.level, path));

    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromFile(path);
    std::filesystem::remove(path);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    const core::Level& level = *reloaded.level;
    EXPECT_EQ(level.cameraFraming().mode, core::CameraFramingMode::PerRoom);
    ASSERT_TRUE(level.cameraFraming().roomWidthTiles.has_value());
    EXPECT_EQ(*level.cameraFraming().roomWidthTiles, 10);
    ASSERT_TRUE(level.cameraFraming().roomHeightTiles.has_value());
    EXPECT_EQ(*level.cameraFraming().roomHeightTiles, 8);
}

/**
 * @brief Parcours complet d'édition des zones de caméra dessinées à la main : passer en mode *par
 * salle*, dessiner deux zones (outil « Zone de caméra »), retirer l'une d'elles par erreur puis
 * annuler ce retrait, enregistrer, recharger (`EX-LVL-007`, `EX-EDIT-029`).
 * \castest{<b>Parcours complet d'édition des zones de caméra dessinées à la main.</b><br/>
 * \tcat Système · Éditeur de niveaux<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Passer en mode *par salle*, dessiner deux zones.<br/>2. Retirer la première par
 * erreur, annuler le retrait.<br/>3. Enregistrer sur disque, recharger.<br/>
 * \tattendu Le niveau rechargé restitue les deux zones, dans le même ordre, avec leurs bornes
 * exactes.
 * }
 */
TEST(ParcoursEditionSysteme, EditeDessineDesZonesDeCameraEnregistreEtRecharge) {
    // 1. Le level designer choisit le mode "par salle" puis dessine deux zones sur le canevas.
    core::LevelDraft draft = core::LevelDraft::empty("Zones (edition)", 30, 20);
    draft.setEntry(0, 0);
    draft.setExit(29, 19);
    draft.setCameraFraming(core::CameraFramingConfig{.mode = core::CameraFramingMode::PerRoom});
    draft.addCameraZone(core::CameraZone{.x = 0, .y = 0, .width = 20, .height = 20});
    draft.addCameraZone(core::CameraZone{.x = 20, .y = 0, .width = 10, .height = 20});
    ASSERT_EQ(draft.cameraFraming().zones.size(), 2u);

    // 2. Retrait par erreur de la premiere zone (bouton "Retirer" du tableau), puis annulation
    // (EX-EDIT-005).
    draft.removeCameraZone(0);
    ASSERT_EQ(draft.cameraFraming().zones.size(), 1u);
    ASSERT_TRUE(draft.undo());
    ASSERT_EQ(draft.cameraFraming().zones.size(), 2u);

    // 3. Enregistrement puis rechargement (round-trip disque, EX-EDIT-011).
    const core::LevelLoadResult validated = draft.toLevel();
    ASSERT_TRUE(validated.ok()) << validated.error;

    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "jadg_systeme_edition_zones.json";
    ASSERT_TRUE(core::LevelWriter::saveToFile(*validated.level, path));

    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromFile(path);
    std::filesystem::remove(path);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    const core::Level& level = *reloaded.level;
    ASSERT_EQ(level.cameraFraming().zones.size(), 2u);
    EXPECT_EQ(level.cameraFraming().zones[0],
              (core::CameraZone{.x = 0, .y = 0, .width = 20, .height = 20}));
    EXPECT_EQ(level.cameraFraming().zones[1],
              (core::CameraZone{.x = 20, .y = 0, .width = 10, .height = 20}));
}
