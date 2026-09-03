// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_mechanism_animation_assignments.cpp
 * @brief Tests unitaires de la section « Animations » du panneau « Textures » (LOT-47 TACHE-04).
 */

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Levels/TileType.h"
#include "HMI/Editor/MechanismAnimationAssignments.h"
#include "HMI/Graphics/MechanismVisuals.h"
#include "HMI/Graphics/SkinCatalog.h"

namespace {

std::filesystem::path makeTempSkinsDir(const std::string& suffix) {
    const std::filesystem::path directory =
        std::filesystem::temp_directory_path() / ("jadg_mech_anim_dir_" + suffix);
    std::filesystem::remove_all(directory);
    std::filesystem::create_directories(directory);
    return directory;
}

void writeFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream file(path);
    file << content;
}

// Descripteur d'animation minimal (une seule case, non exploite par le decodage PNG ici) portant
// exactement les clips donnes, une seule image chacun.
std::string descriptorWithClips(const std::vector<std::string>& clipNames) {
    std::string clips;
    for (std::size_t index = 0; index < clipNames.size(); ++index) {
        if (index > 0) {
            clips += ",";
        }
        clips += "\"" + clipNames[index] + "\":{\"frames\":[0]}";
    }
    return "{\"version\":1,\"frameWidth\":16,\"frameHeight\":16,\"clips\":{" + clips + "}}";
}

const hmi::MechanismAnimationRow* findRow(const std::vector<hmi::MechanismAnimationRow>& rows,
                                          core::TileType type) {
    for (const hmi::MechanismAnimationRow& row : rows) {
        if (row.type == type) {
            return &row;
        }
    }
    return nullptr;
}

}  // namespace

/**
 * @brief Une ligne par famille de mecanisme, dans l'ordre attendu.
 * \castest{<b>La section Animations propose exactement les familles de mecanismes a
 * etat.</b><br/> \tcat Unitaire · Panneau Textures (Animations)<br/> \tcrit Critique<br/>
 * \tetapes 1. Construire les lignes pour un catalogue vide.<br/>
 * \tattendu Une ligne par famille de MECHANISM_ANIMATION_TYPES, sans asset assigne.
 * }
 */
TEST(MechanismAnimationAssignmentsTest, UneLigneParFamille) {
    const hmi::SkinCatalog catalog;
    const std::filesystem::path directory = makeTempSkinsDir("vide");

    const std::vector<hmi::MechanismAnimationRow> rows =
        hmi::buildMechanismAnimationRows(catalog, "", directory);

    ASSERT_EQ(rows.size(), std::size(hmi::MECHANISM_ANIMATION_TYPES));
    for (std::size_t index = 0; index < rows.size(); ++index) {
        EXPECT_EQ(rows[index].type, hmi::MECHANISM_ANIMATION_TYPES[index]);
        EXPECT_TRUE(rows[index].asset.empty());
        EXPECT_TRUE(rows[index].missingClips.empty()) << "rien a diagnostiquer sans asset assigne";
    }

    std::filesystem::remove_all(directory);
}

/**
 * @brief Un asset assigne sans fichier d'animation liste tous les clips comme manquants.
 * \castest{<b>Un asset assigne sans .anim.json liste tous les clips attendus comme
 * manquants.</b><br/> \tcat Unitaire · Panneau Textures (Animations)<br/> \tcrit Critique<br/>
 * \tetapes 1. Assigner un asset a Door, sans creer de fichier d'animation.<br/>
 * \tattendu missingClips contient exactement les clips attendus de Door.
 * }
 */
TEST(MechanismAnimationAssignmentsTest, AssetSansAnimationListeToutManquant) {
    hmi::SkinCatalog catalog;
    catalog.assign("defaut", core::TileType::Door,
                   hmi::SkinEntry{"door.png", hmi::SkinMode::Single});
    const std::filesystem::path directory = makeTempSkinsDir("sans_anim");

    const std::vector<hmi::MechanismAnimationRow> rows =
        hmi::buildMechanismAnimationRows(catalog, "defaut", directory);
    const hmi::MechanismAnimationRow* door = findRow(rows, core::TileType::Door);
    ASSERT_NE(door, nullptr);
    EXPECT_EQ(door->asset, "door.png");
    EXPECT_EQ(door->missingClips, hmi::mechanismExpectedClips(core::TileType::Door));

    std::filesystem::remove_all(directory);
}

/**
 * @brief Un asset fournissant tous les clips attendus ne signale rien de manquant.
 * \castest{<b>Un asset fournissant tous les clips attendus ne signale rien de manquant.</b><br/>
 * \tcat Unitaire · Panneau Textures (Animations)<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Assigner un asset a Switch et deposer un .anim.json avec ses deux clips
 * attendus.<br/> \tattendu missingClips est vide.
 * }
 */
TEST(MechanismAnimationAssignmentsTest, AssetCompletNeManqueRien) {
    hmi::SkinCatalog catalog;
    catalog.assign("defaut", core::TileType::Switch,
                   hmi::SkinEntry{"switch.png", hmi::SkinMode::Single});
    const std::filesystem::path directory = makeTempSkinsDir("complet");
    writeFile(directory / "switch.anim.json",
              descriptorWithClips(hmi::mechanismExpectedClips(core::TileType::Switch)));

    const std::vector<hmi::MechanismAnimationRow> rows =
        hmi::buildMechanismAnimationRows(catalog, "defaut", directory);
    const hmi::MechanismAnimationRow* row = findRow(rows, core::TileType::Switch);
    ASSERT_NE(row, nullptr);
    EXPECT_TRUE(row->missingClips.empty());

    std::filesystem::remove_all(directory);
}

/**
 * @brief Un asset ne fournissant qu'une partie des clips attendus signale le reste.
 * \castest{<b>Un asset partiel signale uniquement les clips absents.</b><br/>
 * \tcat Unitaire · Panneau Textures (Animations)<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Assigner un asset a Door dont le .anim.json ne fournit que « closed » et « open
 * ».<br/> \tattendu missingClips contient exactement « opening » et « closing ».
 * }
 */
TEST(MechanismAnimationAssignmentsTest, AssetPartielSignaleLeReste) {
    hmi::SkinCatalog catalog;
    catalog.assign("defaut", core::TileType::Door,
                   hmi::SkinEntry{"door.png", hmi::SkinMode::Single});
    const std::filesystem::path directory = makeTempSkinsDir("partiel");
    writeFile(directory / "door.anim.json", descriptorWithClips({hmi::MECHANISM_CLIP_DOOR_CLOSED,
                                                                 hmi::MECHANISM_CLIP_DOOR_OPEN}));

    const std::vector<hmi::MechanismAnimationRow> rows =
        hmi::buildMechanismAnimationRows(catalog, "defaut", directory);
    const hmi::MechanismAnimationRow* door = findRow(rows, core::TileType::Door);
    ASSERT_NE(door, nullptr);
    EXPECT_EQ(door->missingClips, (std::vector<std::string>{hmi::MECHANISM_CLIP_DOOR_OPENING,
                                                            hmi::MECHANISM_CLIP_DOOR_CLOSING}));

    std::filesystem::remove_all(directory);
}
