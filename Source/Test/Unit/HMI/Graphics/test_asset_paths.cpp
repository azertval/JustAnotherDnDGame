// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_asset_paths.cpp
 * @brief Tests unitaires de la résolution de chemins d'assets (LOT-39, EX-NFR-010/EX-NFR-040).
 */

#include <fstream>

#include <gtest/gtest.h>

#include "HMI/Graphics/AssetPaths.h"

namespace {
// Dossier temporaire unique (nettoyé à la fin du test), pour ne dépendre d'aucun fichier livré.
class TemporaryDirectory {
public:
    TemporaryDirectory()
        : _path(std::filesystem::temp_directory_path() /
                ("JustAnotherDnDGame_AssetPathsTest_" +
                 std::to_string(reinterpret_cast<std::uintptr_t>(this)))) {
        std::filesystem::create_directories(_path);
    }

    ~TemporaryDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(_path, ignored);
    }

    TemporaryDirectory(const TemporaryDirectory&) = delete;
    TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;

    [[nodiscard]] const std::filesystem::path& path() const {
        return _path;
    }

private:
    std::filesystem::path _path;
};
}  // namespace

/**
 * @brief Un fichier existant dans le dossier d'assets est résolu vers son chemin complet.
 * \castest{<b>Un fichier existant dans le dossier d'assets est résolu vers son chemin
 * complet.</b><br/>
 * \tcat Unitaire · Asset Paths<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un fichier existant dans le dossier d'assets est résolu vers son chemin complet.
 * }
 */
TEST(AssetPathsTest, FichierExistantResolu) {
    TemporaryDirectory directory;
    const std::filesystem::path filePath = directory.path() / "atlas.png";
    std::ofstream(filePath) << "contenu";

    const hmi::AssetPaths assetPaths(directory.path());
    const std::optional<std::filesystem::path> resolved = assetPaths.resolve("atlas.png");

    ASSERT_TRUE(resolved.has_value());
    EXPECT_EQ(*resolved, filePath);
}

/**
 * @brief Un asset absent est signalé par `std::nullopt`, sans exception (repli possible).
 * \castest{<b>Un asset absent est signalé par std::nullopt, sans exception.</b><br/>
 * \tcat Unitaire · Asset Paths<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un asset absent est signalé par std::nullopt, sans exception (repli possible).
 * }
 */
TEST(AssetPathsTest, AssetAbsentSignaleSansException) {
    TemporaryDirectory directory;
    const hmi::AssetPaths assetPaths(directory.path());

    EXPECT_EQ(assetPaths.resolve("introuvable.png"), std::nullopt);
}

/**
 * @brief Un dossier d'assets lui-même inexistant est traité comme « rien à résoudre », sans
 *        exception (robustesse, `EX-NFR-040`).
 * \castest{<b>Un dossier d'assets inexistant est traité comme rien à résoudre.</b><br/>
 * \tcat Unitaire · Asset Paths<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un dossier d'assets inexistant est traité comme rien à résoudre, sans exception.
 * }
 */
TEST(AssetPathsTest, DossierInexistantSansException) {
    const hmi::AssetPaths assetPaths(std::filesystem::temp_directory_path() /
                                     "JustAnotherDnDGame_DossierQuiNexistePas");

    EXPECT_EQ(assetPaths.resolve("atlas.png"), std::nullopt);
}

/**
 * @brief Un chemin désignant un dossier (et non un fichier) n'est pas résolu comme un asset.
 * \castest{<b>Un chemin désignant un dossier n'est pas résolu comme un asset.</b><br/>
 * \tcat Unitaire · Asset Paths<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un chemin désignant un dossier n'est pas résolu comme un asset.
 * }
 */
TEST(AssetPathsTest, DossierNestPasUnAssetValide) {
    TemporaryDirectory directory;
    std::filesystem::create_directories(directory.path() / "sous-dossier");

    const hmi::AssetPaths assetPaths(directory.path());
    EXPECT_EQ(assetPaths.resolve("sous-dossier"), std::nullopt);
}
