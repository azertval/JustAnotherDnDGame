// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_asset_file_operations.cpp
 * @brief Tests unitaires des opérations fichiers sur les assets (LOT-43 TACHE-02).
 */

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

#include <gtest/gtest.h>

#include "HMI/Editor/AssetFileOperations.h"
#include "HMI/Graphics/AssetContract.h"
#include "HMI/Graphics/TextureAtlas.h"

namespace {

// Fournit deux dossiers temporaires vierges par test (source externe + dossier gere).
class AssetFileOps : public ::testing::Test {
protected:
    std::filesystem::path dir;
    std::filesystem::path externalDir;

    void SetUp() override {
        const std::string suffix = std::to_string(reinterpret_cast<std::uintptr_t>(this));
        dir = std::filesystem::temp_directory_path() / ("pg_assetops_" + suffix);
        externalDir = std::filesystem::temp_directory_path() / ("pg_assetops_ext_" + suffix);
        std::filesystem::create_directories(dir);
        std::filesystem::create_directories(externalDir);
    }
    void TearDown() override {
        std::error_code error;
        std::filesystem::remove_all(dir, error);
        std::filesystem::remove_all(externalDir, error);
    }

    // Cree un fichier externe (hors du dossier gere), pret a etre importe.
    std::filesystem::path makeExternalFile(const std::string& name) {
        const std::filesystem::path path = externalDir / name;
        std::ofstream file(path);
        file << "contenu";
        return path;
    }
};

}  // namespace

/**
 * @brief Importer un asset conforme le **copie** dans le dossier géré, en laissant l'original en
 * place : l'auteur importe depuis son dossier de travail, qui ne doit pas se vider au fur et à
 * mesure des imports.
 * \castest{<b>Importer un asset conforme le copie sans déplacer l'original.</b><br/>
 * \tcat Unitaire · Opérations sur fichiers d'assets<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST_F(AssetFileOps, ImporteUnAssetConforme) {
    const std::filesystem::path source = makeExternalFile("mur.png");
    const hmi::AssetFileOperations ops(dir);

    const hmi::FileOperationResult result =
        ops.import(source, hmi::AssetFamily::TileSkin, hmi::TextureAtlas::TILE_SIZE,
                   hmi::TextureAtlas::TILE_SIZE);

    ASSERT_TRUE(result.ok()) << result.error;
    EXPECT_TRUE(std::filesystem::exists(result.path));
    EXPECT_TRUE(std::filesystem::exists(source)) << "l'import doit COPIER, jamais deplacer";
}

/**
 * @brief Un asset non conforme au contrat de sa famille est refusé, le message **nomme le
 * fichier**, et rien n'est copié : un asset aux mauvaises dimensions accepté ne se manifesterait
 * qu'au rendu, longtemps après l'import, sous forme de tuiles décalées.
 * \castest{<b>Un asset non conforme est refusé en nommant le fichier, sans rien copier.</b><br/>
 * \tcat Unitaire · Opérations sur fichiers d'assets<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST_F(AssetFileOps, RefuseUnAssetNonConforme) {
    const std::filesystem::path source = makeExternalFile("mur.png");
    const hmi::AssetFileOperations ops(dir);

    const hmi::FileOperationResult result =
        ops.import(source, hmi::AssetFamily::TileSkin, hmi::TextureAtlas::TILE_SIZE + 1,
                   hmi::TextureAtlas::TILE_SIZE);

    EXPECT_FALSE(result.ok());
    EXPECT_NE(result.error.find("mur.png"), std::string::npos)
        << "le message doit nommer le fichier";
    EXPECT_TRUE(ops.list().empty()) << "un asset refuse ne doit pas etre copie";
}

/**
 * @brief Importer un second fichier de même nom est refusé plutôt qu'écrasant : deux fichiers
 * homonymes venus de dossiers différents sont un cas courant, et l'asset déjà référencé par des
 * niveaux ne doit pas être remplacé à l'insu de l'auteur.
 * \castest{<b>Importer un second fichier de même nom est refusé, sans écraser le
 * premier.</b><br/>
 * \tcat Unitaire · Opérations sur fichiers d'assets<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST_F(AssetFileOps, RefuseUneCollisionAlImport) {
    const std::filesystem::path first = makeExternalFile("mur.png");
    const hmi::AssetFileOperations ops(dir);
    ASSERT_TRUE(ops.import(first, hmi::AssetFamily::Background, 4, 4).ok());

    // Un second fichier externe, de meme nom, importe dans le meme dossier gere.
    const std::filesystem::path second = externalDir / "sous_dossier";
    std::filesystem::create_directories(second);
    std::ofstream(second / "mur.png") << "autre contenu";

    const hmi::FileOperationResult result =
        ops.import(second / "mur.png", hmi::AssetFamily::Background, 4, 4);
    EXPECT_FALSE(result.ok());
}

/**
 * @brief Renommer conserve l'extension d'origine et déplace le fichier : l'auteur saisit un nom,
 * pas un nom de fichier. Perdre l'extension rendrait l'asset invisible au balayage, qui ne retient
 * que les images.
 * \castest{<b>Renommer un asset conserve son extension et déplace le fichier.</b><br/>
 * \tcat Unitaire · Opérations sur fichiers d'assets<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST_F(AssetFileOps, RenommeEtConserveLExtension) {
    const std::filesystem::path source = makeExternalFile("mur.png");
    const hmi::AssetFileOperations ops(dir);
    const hmi::FileOperationResult imported =
        ops.import(source, hmi::AssetFamily::Background, 4, 4);
    ASSERT_TRUE(imported.ok()) << imported.error;

    const hmi::FileOperationResult renamed = ops.rename(imported.path, "pierre");
    ASSERT_TRUE(renamed.ok()) << renamed.error;
    EXPECT_EQ(renamed.path.filename().string(), "pierre.png");
    EXPECT_FALSE(std::filesystem::exists(imported.path));
}

/**
 * @brief Un nom invalide au renommage (barre oblique, qui sortirait du dossier géré) est refusé
 * **sans rien changer** : le fichier d'origine reste en place, l'opération est tout ou rien.
 * \castest{<b>Un nom invalide au renommage est refusé sans rien changer.</b><br/>
 * \tcat Unitaire · Opérations sur fichiers d'assets<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST_F(AssetFileOps, RefuseUnNomInvalideAuRenommage) {
    const std::filesystem::path source = makeExternalFile("mur.png");
    const hmi::AssetFileOperations ops(dir);
    const hmi::FileOperationResult imported =
        ops.import(source, hmi::AssetFamily::Background, 4, 4);
    ASSERT_TRUE(imported.ok()) << imported.error;

    EXPECT_FALSE(ops.rename(imported.path, "a/b").ok());  // barre oblique interdite
    EXPECT_TRUE(std::filesystem::exists(imported.path))
        << "un renommage refuse ne doit rien changer";
}

/**
 * @brief Dupliquer deux fois le même asset produit deux copies distinctes : le nom est rendu
 * unique à chaque appel, sinon la seconde duplication écraserait la première — le geste courant
 * pour décliner une variante de skin.
 * \castest{<b>Dupliquer deux fois le même asset produit deux copies distinctes.</b><br/>
 * \tcat Unitaire · Opérations sur fichiers d'assets<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST_F(AssetFileOps, DupliqueSousUnNomUnique) {
    const std::filesystem::path source = makeExternalFile("mur.png");
    const hmi::AssetFileOperations ops(dir);
    const hmi::FileOperationResult imported =
        ops.import(source, hmi::AssetFamily::Background, 4, 4);
    ASSERT_TRUE(imported.ok()) << imported.error;

    const hmi::FileOperationResult first = ops.duplicate(imported.path);
    const hmi::FileOperationResult second = ops.duplicate(imported.path);
    ASSERT_TRUE(first.ok()) << first.error;
    ASSERT_TRUE(second.ok()) << second.error;
    EXPECT_NE(first.path, second.path);
    EXPECT_EQ(ops.list().size(), 3U);
}

/**
 * @brief Supprimer retire effectivement l'asset du disque et le signale par un résultat
 * favorable — la grille du panneau se fie à ce résultat pour retirer la vignette.
 * \castest{<b>Supprimer retire effectivement l'asset du disque.</b><br/>
 * \tcat Unitaire · Opérations sur fichiers d'assets<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST_F(AssetFileOps, SupprimeLeFichier) {
    const std::filesystem::path source = makeExternalFile("mur.png");
    const hmi::AssetFileOperations ops(dir);
    const hmi::FileOperationResult imported =
        ops.import(source, hmi::AssetFamily::Background, 4, 4);
    ASSERT_TRUE(imported.ok()) << imported.error;

    EXPECT_TRUE(ops.remove(imported.path).ok());
    EXPECT_FALSE(std::filesystem::exists(imported.path));
}

/**
 * @brief Renommer, dupliquer ou supprimer un fichier absent échoue par un résultat récupérable,
 * jamais par une exception : le fichier a pu disparaître entre le balayage et le clic (suppression
 * externe, rechargement à chaud), et l'éditeur doit le signaler sans s'interrompre.
 * \castest{<b>Renommer, dupliquer ou supprimer un fichier absent échoue proprement.</b><br/>
 * \tcat Unitaire · Opérations sur fichiers d'assets<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST_F(AssetFileOps, OperationSurFichierAbsentEchoueProprement) {
    const hmi::AssetFileOperations ops(dir);
    const std::filesystem::path absent = dir / "inexistant.png";

    EXPECT_FALSE(ops.rename(absent, "autre").ok());
    EXPECT_FALSE(ops.duplicate(absent).ok());
    EXPECT_FALSE(ops.remove(absent).ok());
}
