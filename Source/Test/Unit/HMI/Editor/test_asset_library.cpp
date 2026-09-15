// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_asset_library.cpp
 * @brief Tests unitaires du balayage/filtrage partagé par le panneau « Textures » (LOT-43
 * TACHE-01).
 */

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "HMI/Editor/AssetLibrary.h"

namespace {

// Dossier temporaire vierge par test (créé/supprimé automatiquement).
class AssetLibraryTest : public ::testing::Test {
protected:
    std::filesystem::path dir;

    void SetUp() override {
        dir = std::filesystem::temp_directory_path() /
              ("pg_asset_library_" + std::to_string(reinterpret_cast<std::uintptr_t>(this)));
        std::filesystem::create_directories(dir);
    }
    void TearDown() override {
        std::error_code error;
        std::filesystem::remove_all(dir, error);
    }

    void touch(const std::string& name) {
        std::ofstream file(dir / name);
    }
};

}  // namespace

/**
 * @brief Le balayage ne retient que les images et les trie par ordre alphabétique : les fichiers
 * voisins non graphiques (notes, README) n'ont rien à faire dans une bibliothèque d'assets, et un
 * ordre stable évite que la grille se réorganise d'un balayage à l'autre.
 * \castest{<b>Le balayage ne retient que les images, triées par ordre alphabétique.</b><br/>
 * \tcat Unitaire · Bibliothèque d'assets<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST_F(AssetLibraryTest, NeRetientQueLesImagesTrieesParOrdreAlphabetique) {
    touch("stone.png");
    touch("crate.png");
    touch("notes.txt");
    touch("README.md");

    const std::vector<std::string> assets = hmi::listAssetFiles(dir);

    EXPECT_EQ(assets, (std::vector<std::string>{"crate.png", "stone.png"}));
}

/**
 * @brief L'extension est reconnue quelle que soit sa casse : un fichier déposé par un outil
 * externe en `.PNG` ou `.Png` est un asset comme un autre, et le système de fichiers Windows ne
 * distingue de toute façon pas les deux.
 * \castest{<b>L'extension d'image est reconnue quelle que soit sa casse.</b><br/>
 * \tcat Unitaire · Bibliothèque d'assets<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST_F(AssetLibraryTest, ExtensionInsensibleALaCasse) {
    touch("stone.PNG");
    touch("crate.Png");

    const std::vector<std::string> assets = hmi::listAssetFiles(dir);

    EXPECT_EQ(assets, (std::vector<std::string>{"crate.Png", "stone.PNG"}));
}

/**
 * @brief La recherche filtre par sous-chaîne, sans tenir compte de la casse, et renvoie une liste
 * vide plutôt qu'une erreur quand rien ne correspond — c'est une recherche incrémentale, tapée
 * caractère par caractère, où l'absence de résultat est un état normal.
 * \castest{<b>La recherche filtre par sous-chaîne insensible à la casse, liste vide si rien ne
 * correspond.</b><br/>
 * \tcat Unitaire · Bibliothèque d'assets<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST_F(AssetLibraryTest, FiltreParSousChaineInsensibleALaCasse) {
    touch("stone.png");
    touch("crate.png");
    touch("stone_cracked.png");

    EXPECT_EQ(hmi::listAssetFiles(dir, "STONE"),
              (std::vector<std::string>{"stone.png", "stone_cracked.png"}));
    EXPECT_EQ(hmi::listAssetFiles(dir, "crate"), (std::vector<std::string>{"crate.png"}));
    EXPECT_TRUE(hmi::listAssetFiles(dir, "inexistant").empty());
}

/**
 * @brief Un dossier d'assets absent donne une liste vide, jamais une exception : le dossier peut
 * légitimement ne pas exister (installation neuve, asset jamais créé), et le panneau doit alors
 * s'afficher vide plutôt que faire échouer son ouverture.
 * \castest{<b>Un dossier d'assets absent donne une liste vide, sans exception.</b><br/>
 * \tcat Unitaire · Bibliothèque d'assets<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST_F(AssetLibraryTest, DossierAbsentListeVide) {
    EXPECT_TRUE(
        hmi::listAssetFiles(std::filesystem::temp_directory_path() / "pg_asset_library_absent")
            .empty());
}
