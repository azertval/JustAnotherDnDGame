// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_skin_assignments.cpp
 * @brief Tests unitaires de la logique du panneau « Textures » (LOT-42 TACHE-04).
 */

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Levels/TileType.h"
#include "HMI/Editor/SkinAssignments.h"
#include "HMI/Editor/TaxonomyLabels.h"
#include "HMI/Editor/TileTaxonomy.h"
#include "HMI/Graphics/SkinCatalog.h"
#include "HMI/Localization/Localization.h"

namespace {

/// Catalogue de reference : un seul type assigne, pour distinguer skinne et non skinne.
hmi::SkinCatalog referenceCatalog() {
    hmi::SkinCatalog catalog;
    catalog.assign("foret", core::TileType::Solid,
                   hmi::SkinEntry{"stone.png", hmi::SkinMode::Bitmask16});
    return catalog;
}

/// Toutes les lignes de toutes les sections, a plat.
std::vector<hmi::SkinRow> flatten(const std::vector<hmi::SkinSection>& sections) {
    std::vector<hmi::SkinRow> rows;
    for (const hmi::SkinSection& section : sections) {
        rows.insert(rows.end(), section.rows.begin(), section.rows.end());
    }
    return rows;
}

/// Dossier temporaire vide, cree pour un test de balayage.
std::filesystem::path makeTempSkinsDir(const std::string& suffix) {
    const std::filesystem::path directory =
        std::filesystem::temp_directory_path() / ("jadg_skins_dir_" + suffix);
    std::filesystem::remove_all(directory);
    std::filesystem::create_directories(directory);
    return directory;
}

/// Cree un fichier vide au chemin donne.
void touchFile(const std::filesystem::path& path) {
    std::ofstream file(path);
}

}  // namespace

/**
 * @brief Chaque type peignable apparait exactement une fois dans le panneau.
 * \castest{<b>Chaque type de tuile peignable apparait exactement une fois dans le panneau.</b><br/>
 * \tcat Unitaire · Panneau Textures<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire les lignes du panneau pour un catalogue quelconque.<br/>
 * 2. Collecter les types de toutes les lignes.<br/>
 * \tattendu Chaque type de la taxonomie y figure une fois, sauf la case vide qui en est exclue.
 * }
 */
TEST(SkinAssignmentsTest, ChaqueTypeApparaitUneFois) {
    const hmi::SkinCatalog catalog = referenceCatalog();
    const std::vector<hmi::SkinRow> rows = flatten(hmi::buildSkinRows(catalog, "foret"));

    std::set<core::TileType> seen;
    for (const hmi::SkinRow& row : rows) {
        EXPECT_TRUE(seen.insert(row.type).second) << "type en double dans le panneau";
        // Une case vide n'affiche rien : lui assigner une apparence n'aurait pas de sens.
        EXPECT_NE(row.type, core::TileType::Empty);
    }

    // Tous les types de la taxonomie (hors case vide) doivent etre habillables.
    std::set<core::TileType> expected;
    for (const hmi::TileCategory& category : hmi::tileTaxonomy()) {
        for (const hmi::TileEntry& entry : category.tiles) {
            expected.insert(entry.type);
        }
        for (const hmi::TileSubgroup& subgroup : category.subgroups) {
            for (const hmi::TileEntry& entry : subgroup.tiles) {
                expected.insert(entry.type);
            }
        }
    }
    expected.erase(core::TileType::Empty);
    EXPECT_EQ(seen, expected);
}

/**
 * @brief Les lignes refletent l'assignation courante du jeu consulte.
 * \castest{<b>Les lignes du panneau refletent l'assignation courante du jeu consulte.</b><br/>
 * \tcat Unitaire · Panneau Textures<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire les lignes pour un catalogue ou seul Solid est assigne.<br/>
 * \tattendu La ligne de Solid porte son fichier et son mode ; les autres ont un fichier vide.
 * }
 */
TEST(SkinAssignmentsTest, LignesRefletentLAssignation) {
    const hmi::SkinCatalog catalog = referenceCatalog();
    const std::vector<hmi::SkinRow> rows = flatten(hmi::buildSkinRows(catalog, "foret"));

    for (const hmi::SkinRow& row : rows) {
        if (row.type == core::TileType::Solid) {
            EXPECT_EQ(row.asset, "stone.png");
            EXPECT_EQ(row.mode, hmi::SkinMode::Bitmask16);
        } else {
            // Fichier vide = type non skinne : il s'affichera en damier, et le panneau doit le
            // montrer comme tel plutot que de le masquer.
            EXPECT_TRUE(row.asset.empty()) << "type non assigne avec un fichier";
        }
    }
}

/**
 * @brief Les sections suivent l'organisation de la palette.
 * \castest{<b>Les sections du panneau suivent les categories de la palette.</b><br/>
 * \tcat Unitaire · Panneau Textures<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Comparer les libelles de sections aux categories de la taxonomie.<br/>
 * \tattendu Les sections reprennent les categories, dans le meme ordre.
 * }
 */
TEST(SkinAssignmentsTest, SectionsSuiventLaPalette) {
    const hmi::SkinCatalog catalog = referenceCatalog();
    const std::vector<hmi::SkinSection> sections = hmi::buildSkinRows(catalog, "foret");

    // Deux organisations differentes entre la palette et ce panneau obligeraient le level designer
    // a reapprendre ou trouver chaque tuile.
    std::vector<std::string> sectionLabels;
    for (const hmi::SkinSection& section : sections) {
        sectionLabels.push_back(section.label);
    }
    std::vector<std::string> categoryLabels;
    for (const hmi::TileCategory& category : hmi::tileTaxonomy()) {
        categoryLabels.push_back(category.label);
    }

    EXPECT_EQ(sectionLabels, categoryLabels);
}

/**
 * @brief Assigner puis retirer un skin depuis le panneau modifie le catalogue.
 * \castest{<b>Assigner puis retirer un skin depuis le panneau modifie le catalogue.</b><br/>
 * \tcat Unitaire · Panneau Textures<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Appliquer une assignation a un type non skinne, puis relire les lignes.<br/>
 * 2. Appliquer une assignation vide au meme type, puis relire.<br/>
 * \tattendu La premiere renseigne le fichier ; la seconde le retire entierement.
 * }
 */
TEST(SkinAssignmentsTest, AssignationPuisRetraitDepuisLePanneau) {
    hmi::SkinCatalog catalog = referenceCatalog();

    hmi::applySkinAssignment(catalog, "foret", core::TileType::Danger, "spikes.png",
                             hmi::SkinMode::Single);
    const std::optional<hmi::SkinEntry> assigned = catalog.resolve("foret", core::TileType::Danger);
    ASSERT_TRUE(assigned.has_value());
    EXPECT_EQ(assigned->asset, "spikes.png");

    // « Aucun skin » doit RETIRER l'assignation : associer un fichier de nom vide laisserait une
    // entree que la resolution tenterait de charger a chaque image.
    hmi::applySkinAssignment(catalog, "foret", core::TileType::Danger, "", hmi::SkinMode::Single);
    EXPECT_FALSE(catalog.resolve("foret", core::TileType::Danger).has_value());
}

/**
 * @brief Un jeu non precise vise le jeu par defaut.
 * \castest{<b>Une assignation sans jeu precise vise le jeu par defaut.</b><br/>
 * \tcat Unitaire · Panneau Textures<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Appliquer une assignation en passant un nom de jeu vide.<br/>
 * \tattendu Elle est enregistree dans le jeu par defaut du catalogue.
 * }
 */
TEST(SkinAssignmentsTest, JeuNonPreciseViseLeDefaut) {
    hmi::SkinCatalog catalog = referenceCatalog();
    ASSERT_EQ(catalog.defaultSetName(), "foret");

    hmi::applySkinAssignment(catalog, "", core::TileType::Door, "door.png", hmi::SkinMode::Single);

    const std::map<core::TileType, hmi::SkinEntry> assignments = catalog.assignments("foret");
    EXPECT_NE(assignments.find(core::TileType::Door), assignments.end());
}

/**
 * @brief Le balayage ne retient que les images, triees.
 * \castest{<b>Le balayage du dossier de skins ne retient que les images, triees.</b><br/>
 * \tcat Unitaire · Panneau Textures<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Creer un dossier contenant deux PNG, un fichier texte et un README.<br/>
 * \tattendu Seuls les deux PNG sont listes, par ordre alphabetique.
 * }
 */
TEST(SkinAssignmentsTest, BalayageNeRetientQueLesImages) {
    const std::filesystem::path directory = makeTempSkinsDir("balayage");
    touchFile(directory / "stone.png");
    touchFile(directory / "crate.png");
    touchFile(directory / "notes.txt");
    touchFile(directory / "README.md");

    const std::vector<std::string> assets = hmi::listSkinAssets(directory);

    const std::vector<std::string> expected{"crate.png", "stone.png"};
    EXPECT_EQ(assets, expected);

    std::filesystem::remove_all(directory);
}

/**
 * @brief Un dossier de skins absent donne une liste vide.
 * \castest{<b>Un dossier de skins absent donne une liste vide, sans erreur.</b><br/>
 * \tcat Unitaire · Panneau Textures<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Balayer un chemin qui n'existe pas.<br/>
 * \tattendu La liste est vide, sans exception.
 * }
 */
TEST(SkinAssignmentsTest, DossierAbsentListeVide) {
    // Etat de depart legitime : aucun skin n'a encore ete depose.
    EXPECT_TRUE(
        hmi::listSkinAssets(std::filesystem::temp_directory_path() / "jadg_dossier_inexistant")
            .empty());
}

/**
 * @brief Chaque libelle de la taxonomie est traduit dans les deux catalogues livres.
 * \castest{<b>Chaque libelle de la taxonomie a une traduction dans les deux catalogues.</b><br/>
 * \tcat Unitaire · Panneau Textures<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Pour chaque libelle de la taxonomie, obtenir sa cle de traduction.<br/>
 * 2. Verifier que la cle est resolue en francais puis en anglais.<br/>
 * \tattendu Aucune cle ne reste non traduite dans l'une des deux langues.
 * }
 */
TEST(SkinAssignmentsTest, LibellesTaxonomieTraduitsDansLesDeuxLangues) {
    std::vector<std::string> labels;
    for (const hmi::TileCategory& category : hmi::tileTaxonomy()) {
        labels.push_back(category.label);
        for (const hmi::TileEntry& entry : category.tiles) {
            labels.push_back(entry.label);
        }
        for (const hmi::TileSubgroup& subgroup : category.subgroups) {
            labels.push_back(subgroup.label);
            for (const hmi::TileEntry& entry : subgroup.tiles) {
                labels.push_back(entry.label);
            }
        }
    }

    for (const std::string& language : {"fr", "en"}) {
        hmi::Localization loc{JADG_LOCALIZATION_DIR};
        ASSERT_TRUE(loc.loadLanguage(language)) << "catalogue " << language << " illisible";

        for (const std::string& label : labels) {
            const std::string key = hmi::taxonomyLabelKey(label);
            ASSERT_FALSE(key.empty()) << "libelle sans cle de traduction : " << label;
            // Localization::text rend la CLE quand elle est absente du catalogue : c'est le
            // symptome a detecter ici, sans quoi l'interface afficherait « palette.tile.solid ».
            EXPECT_NE(loc.text(key), key)
                << "cle absente du catalogue " << language << " : " << key;
        }
    }
}

/**
 * @brief Les libelles du panneau Textures sont traduits dans les deux catalogues.
 * \castest{<b>Les libelles propres au panneau Textures existent dans les deux catalogues.</b><br/>
 * \tcat Unitaire · Panneau Textures<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Pour chaque cle utilisee par le panneau, la resoudre en francais puis en
 * anglais.<br/> \tattendu Chaque cle est traduite dans les deux langues.
 * }
 */
TEST(SkinAssignmentsTest, LibellesDuPanneauTraduits) {
    // Aucun libelle en dur (EX-REN-033) : regle bloquante du projet, pas une preference.
    const std::vector<std::string> keys{
        "dock.textures",
        "textures.skin_set",
        "textures.section_skins",
        "textures.column_type",
        "textures.column_asset",
        "textures.column_mode",
        "textures.none",
        "textures.section_animations",
        "textures.animations_column_diagnostic",
        "textures.animations_preview",
        "textures.animations_complete",
        "textures.animations_missing",
    };

    for (const std::string& language : {"fr", "en"}) {
        hmi::Localization loc{JADG_LOCALIZATION_DIR};
        ASSERT_TRUE(loc.loadLanguage(language));
        for (const std::string& key : keys) {
            EXPECT_NE(loc.text(key), key)
                << "cle absente du catalogue " << language << " : " << key;
        }
    }
}
