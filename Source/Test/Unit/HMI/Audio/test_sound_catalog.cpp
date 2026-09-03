// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sound_catalog.cpp
 * @brief Tests unitaires du catalogue de sons (LOT-60, EX-REN-047, EX-REN-048).
 */

#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "HMI/Audio/SoundCatalog.h"

namespace {

constexpr const char* REFERENCE_JSON = R"({
  "version": 1,
  "sons": {
    "saut": "saut.wav",
    "atterrissage": "atterrissage.wav"
  }
})";

}  // namespace

/**
 * @brief Un catalogue valide se lit sans erreur et resout ses evenements connus.
 * \castest{<b>Un catalogue de sons valide se lit sans erreur.</b><br/>
 * \tcat Unitaire · Catalogue de sons<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Charger un JSON de catalogue avec deux evenements.<br/>
 * 2. Resoudre chacun des deux evenements.<br/>
 * \tattendu La lecture reussit et chaque evenement resout son fichier attendu.
 * }
 */
TEST(SoundCatalogTest, CatalogueValideSeCharge) {
    const hmi::SoundCatalogResult result = hmi::SoundCatalog::loadFromString(REFERENCE_JSON);
    ASSERT_TRUE(result.ok()) << result.error;

    EXPECT_EQ(result.catalog->resolve("saut"), "saut.wav");
    EXPECT_EQ(result.catalog->resolve("atterrissage"), "atterrissage.wav");
}

/**
 * @brief Un fichier de catalogue absent est un catalogue vide, pas une erreur bloquante.
 * \castest{<b>Un fichier de catalogue de sons absent produit un catalogue vide.</b><br/>
 * \tcat Unitaire · Catalogue de sons<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger un catalogue depuis un chemin inexistant.<br/>
 * \tattendu La lecture echoue avec le code FileNotFound (cas legitime : c'est a l'appelant de
 * retenir un catalogue vide comme etat de depart).
 * }
 */
TEST(SoundCatalogTest, FichierAbsentEstFileNotFound) {
    const std::filesystem::path missing =
        std::filesystem::temp_directory_path() / "jadg_sounds_inexistant.json";
    const hmi::SoundCatalogResult result = hmi::SoundCatalog::loadFromFile(missing);

    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, hmi::SoundCatalogError::FileNotFound);
}

/**
 * @brief Une entree malformee fait echouer le chargement entier, jamais un catalogue partiel.
 * \castest{<b>Une entree malformee du catalogue de sons fait echouer tout le chargement.</b><br/>
 * \tcat Unitaire · Catalogue de sons<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Charger un JSON dont une entree "sons" n'est pas une chaine.<br/>
 * \tattendu La lecture echoue avec MalformedStructure -- meme garantie que SkinCatalog et
 * AnimationCatalog : jamais un catalogue devine ou partiellement charge.
 * }
 */
TEST(SoundCatalogTest, EntreeMalformeeEchoueLeChargementEntier) {
    constexpr const char* malformed = R"({
      "version": 1,
      "sons": {
        "saut": "saut.wav",
        "atterrissage": 42
      }
    })";
    const hmi::SoundCatalogResult result = hmi::SoundCatalog::loadFromString(malformed);

    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, hmi::SoundCatalogError::MalformedStructure);
}

/**
 * @brief Un evenement inconnu resout silencieusement l'absence de son, jamais une exception.
 * \castest{<b>Un evenement inconnu du catalogue de sons ne produit aucune erreur.</b><br/>
 * \tcat Unitaire · Catalogue de sons<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger le catalogue de reference.<br/>
 * 2. Resoudre un identifiant d'evenement absent du catalogue.<br/>
 * \tattendu `resolve` renvoie `std::nullopt`, sans exception.
 * }
 */
TEST(SoundCatalogTest, EvenementInconnuResoutNullopt) {
    const hmi::SoundCatalogResult result = hmi::SoundCatalog::loadFromString(REFERENCE_JSON);
    ASSERT_TRUE(result.ok()) << result.error;

    EXPECT_EQ(result.catalog->resolve("evenement_qui_n_existe_pas"), std::nullopt);
}

/**
 * @brief Une version de format superieure a celle geree est refusee explicitement.
 * \castest{<b>Une version de format superieure au catalogue de sons connu est refusee.</b><br/>
 * \tcat Unitaire · Catalogue de sons<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Charger un JSON dont le champ "version" depasse SoundCatalog::FORMAT_VERSION.<br/>
 * \tattendu La lecture echoue avec UnsupportedVersion, jamais lue au mieux.
 * }
 */
TEST(SoundCatalogTest, VersionSuperieureRefusee) {
    constexpr const char* futureVersion = R"({
      "version": 99,
      "sons": { "saut": "saut.wav" }
    })";
    const hmi::SoundCatalogResult result = hmi::SoundCatalog::loadFromString(futureVersion);

    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, hmi::SoundCatalogError::UnsupportedVersion);
}

/**
 * @brief Le catalogue livre avec le jeu se lit sans erreur.
 * \castest{<b>Le fichier de sons livre avec le jeu se lit sans erreur.</b><br/>
 * \tcat Unitaire · Catalogue de sons<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Lire Source/Elements/Audio/sounds.json depuis les sources.<br/>
 * \tattendu La lecture reussit et le catalogue n'est pas vide.
 * }
 */
TEST(SoundCatalogTest, CatalogueLivreValide) {
    const std::filesystem::path path =
        std::filesystem::path(JADG_AUDIO_DIR) / "sounds.json";
    ASSERT_TRUE(std::filesystem::exists(path)) << path.string();

    const hmi::SoundCatalogResult result = hmi::SoundCatalog::loadFromFile(path);
    ASSERT_TRUE(result.ok()) << result.error;

    EXPECT_FALSE(result.catalog->eventIds().empty());
}

/**
 * @brief Chaque fichier reference par le catalogue livre existe reellement sur le disque.
 * \castest{<b>Chaque son reference par le catalogue livre existe.</b><br/>
 * \tcat Unitaire · Catalogue de sons<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Lire le catalogue livre.<br/>
 * 2. Pour chaque evenement, verifier que le fichier resolu existe dans Source/Elements/Audio.<br/>
 * \tattendu Aucune faute de frappe entre le catalogue et les fichiers reellement livres.
 * }
 */
TEST(SoundCatalogTest, AssetsDuCatalogueLivreExistent) {
    const std::filesystem::path audioDir{JADG_AUDIO_DIR};
    const hmi::SoundCatalogResult result =
        hmi::SoundCatalog::loadFromFile(audioDir / "sounds.json");
    ASSERT_TRUE(result.ok()) << result.error;

    for (const std::string& eventId : result.catalog->eventIds()) {
        const std::optional<std::string> file = result.catalog->resolve(eventId);
        ASSERT_TRUE(file.has_value()) << eventId;
        const std::filesystem::path assetPath = audioDir / *file;
        EXPECT_TRUE(std::filesystem::exists(assetPath))
            << "evenement « " << eventId << " » : fichier introuvable " << assetPath.string();
    }
}
