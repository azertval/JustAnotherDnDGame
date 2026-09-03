// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_skin_catalog.cpp
 * @brief Tests unitaires du catalogue de skins (LOT-42, EX-EDIT-042, EX-EDIT-024).
 */

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Levels/TileType.h"
#include "Core/Levels/TileTypeName.h"
#include "HMI/Graphics/SkinCatalog.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TileAutotile.h"

namespace {

/// Catalogue de reference : deux jeux, plusieurs modes, un defaut explicite.
constexpr const char* REFERENCE_JSON = R"({
  "version": 1,
  "defaut": "foret",
  "jeux": {
    "foret": {
      "solid": { "asset": "stone.png", "mode": "bitmask16" },
      "block": { "asset": "crate.png", "mode": "single" },
      "door": { "asset": "door.png", "mode": "single" }
    },
    "grotte": {
      "solid": { "asset": "rock.png", "mode": "bitmask16" }
    }
  }
})";

/// Charge le catalogue de reference, en echouant le test si la lecture ne passe pas.
hmi::SkinCatalog referenceCatalog() {
    hmi::SkinCatalogResult result = hmi::SkinCatalog::loadFromString(REFERENCE_JSON);
    EXPECT_TRUE(result.ok()) << result.error;
    return result.ok() ? *result.catalog : hmi::SkinCatalog{};
}

/// Chemin temporaire unique pour les tests d'ecriture.
std::filesystem::path tempSkinsPath(const std::string& suffix) {
    return std::filesystem::temp_directory_path() / ("jadg_skins_" + suffix + ".json");
}

/// Dimensions d'un PNG, lues dans son en-tete IHDR (largeur et hauteur, gros-boutistes).
/// Evite de dependre d'un decodeur d'image dans un test qui ne verifie que des dimensions.
std::optional<std::pair<int, int>> pngSize(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return std::nullopt;
    }
    unsigned char header[24]{};
    file.read(reinterpret_cast<char*>(header), sizeof(header));
    if (file.gcount() != static_cast<std::streamsize>(sizeof(header))) {
        return std::nullopt;
    }
    static const unsigned char signature[8] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
    if (std::memcmp(header, signature, sizeof(signature)) != 0) {
        return std::nullopt;
    }
    const auto readBigEndian = [&header](std::size_t offset) {
        return (static_cast<int>(header[offset]) << 24) |
               (static_cast<int>(header[offset + 1]) << 16) |
               (static_cast<int>(header[offset + 2]) << 8) | static_cast<int>(header[offset + 3]);
    };
    return std::pair<int, int>{readBigEndian(16), readBigEndian(20)};
}

}  // namespace

/**
 * @brief Le nom persiste d'un mode de skin fait l'aller-retour sans perte.
 * \castest{<b>Le nom persiste d'un mode de skin fait l'aller-retour sans perte.</b><br/>
 * \tcat Unitaire · Catalogue de skins<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Convertir chaque mode en nom, puis le nom en mode.<br/>
 * \tattendu Le mode d'origine est retrouve ; un nom inconnu est refuse.
 * }
 */
TEST(SkinCatalogTest, AllerRetourDuNomDeMode) {
    for (const hmi::SkinMode mode : {hmi::SkinMode::Single, hmi::SkinMode::Bitmask16}) {
        const std::optional<hmi::SkinMode> parsed = hmi::skinModeFromName(hmi::skinModeName(mode));
        ASSERT_TRUE(parsed.has_value());
        EXPECT_EQ(*parsed, mode);
    }

    EXPECT_FALSE(hmi::skinModeFromName("").has_value());
    EXPECT_FALSE(hmi::skinModeFromName("bitmask47").has_value());
}

/**
 * @brief Lecture, ecriture puis relecture rendent un catalogue identique.
 * \castest{<b>Un catalogue fait l'aller-retour lecture -> ecriture -> lecture sans perte.</b><br/>
 * \tcat Unitaire · Catalogue de skins<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Lire le catalogue de reference (deux jeux).<br/>
 * 2. Le reserialiser puis le relire.<br/>
 * \tattendu Les jeux, le defaut et toutes les assignations sont identiques.
 * }
 */
TEST(SkinCatalogTest, AllerRetourLectureEcriture) {
    const hmi::SkinCatalog original = referenceCatalog();

    hmi::SkinCatalogResult reread = hmi::SkinCatalog::loadFromString(original.toJsonString());
    ASSERT_TRUE(reread.ok()) << reread.error;

    EXPECT_EQ(reread.catalog->setNames(), original.setNames());
    EXPECT_EQ(reread.catalog->defaultSetName(), original.defaultSetName());
    for (const std::string& setName : original.setNames()) {
        EXPECT_EQ(reread.catalog->assignments(setName), original.assignments(setName))
            << "jeu divergent : " << setName;
    }
}

/**
 * @brief La resolution rend l'entree du jeu demande, avec son mode.
 * \castest{<b>La resolution rend l'asset et le mode assignes dans le jeu demande.</b><br/>
 * \tcat Unitaire · Catalogue de skins<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Resoudre Solid et Block dans le jeu « foret », puis Solid dans « grotte ».<br/>
 * \tattendu Chaque resolution rend l'asset et le mode ecrits dans le fichier.
 * }
 */
TEST(SkinCatalogTest, ResolutionDansUnJeuExistant) {
    const hmi::SkinCatalog catalog = referenceCatalog();

    const std::optional<hmi::SkinEntry> solid = catalog.resolve("foret", core::TileType::Solid);
    ASSERT_TRUE(solid.has_value());
    EXPECT_EQ(solid->asset, "stone.png");
    EXPECT_EQ(solid->mode, hmi::SkinMode::Bitmask16);

    const std::optional<hmi::SkinEntry> block = catalog.resolve("foret", core::TileType::Block);
    ASSERT_TRUE(block.has_value());
    EXPECT_EQ(block->asset, "crate.png");
    EXPECT_EQ(block->mode, hmi::SkinMode::Single);

    // Un autre jeu donne une autre apparence pour le meme type : c'est tout l'objet des jeux.
    const std::optional<hmi::SkinEntry> cave = catalog.resolve("grotte", core::TileType::Solid);
    ASSERT_TRUE(cave.has_value());
    EXPECT_EQ(cave->asset, "rock.png");
}

/**
 * @brief Un jeu inexistant retombe sur le jeu par defaut.
 * \castest{<b>Un jeu inexistant retombe sur le jeu par defaut plutot que de ne rien
 * rendre.</b><br/> \tcat Unitaire · Catalogue de skins<br/> \tcrit Majeur<br/> \tetapes 1. Resoudre
 * Solid dans un jeu qui n'existe pas.<br/>
 * 2. Resoudre Solid en demandant explicitement le defaut (nom vide).<br/>
 * \tattendu Les deux rendent l'entree du jeu « foret », designe par defaut.
 * }
 */
TEST(SkinCatalogTest, JeuInexistantRetombeSurLeDefaut) {
    const hmi::SkinCatalog catalog = referenceCatalog();

    const std::optional<hmi::SkinEntry> missing =
        catalog.resolve("banquise", core::TileType::Solid);
    ASSERT_TRUE(missing.has_value());
    EXPECT_EQ(missing->asset, "stone.png");

    const std::optional<hmi::SkinEntry> byDefault = catalog.resolve("", core::TileType::Solid);
    ASSERT_TRUE(byDefault.has_value());
    EXPECT_EQ(byDefault->asset, "stone.png");
}

/**
 * @brief Un type non skinne ne resout rien, sans erreur.
 * \castest{<b>Un type de tuile sans skin assigne ne resout aucune entree.</b><br/>
 * \tcat Unitaire · Catalogue de skins<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Resoudre un type absent du jeu « foret », puis un type absent de « grotte ».<br/>
 * \tattendu Les deux resolutions rendent une absence d'entree, sans exception.
 * }
 */
TEST(SkinCatalogTest, TypeNonSkinneNeResoudRien) {
    const hmi::SkinCatalog catalog = referenceCatalog();

    // L'appelant affichera le damier de repli : c'est un etat normal du programme d'habillage,
    // tant que tous les types n'ont pas ete habilles.
    EXPECT_FALSE(catalog.resolve("foret", core::TileType::Danger).has_value());
    EXPECT_FALSE(catalog.resolve("grotte", core::TileType::Block).has_value());
}

/**
 * @brief Un JSON malforme est signale sans exception.
 * \castest{<b>Un fichier de skins malforme est signale sans lever d'exception.</b><br/>
 * \tcat Unitaire · Catalogue de skins<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Lire une chaine qui n'est pas du JSON, puis un JSON dont la racine est un
 * tableau.<br/> \tattendu Les deux lectures echouent avec le code ParseError, sans exception.
 * }
 */
TEST(SkinCatalogTest, JsonMalformeSignale) {
    const hmi::SkinCatalogResult broken = hmi::SkinCatalog::loadFromString("{ pas du json");
    EXPECT_FALSE(broken.ok());
    EXPECT_EQ(broken.errorCode, hmi::SkinCatalogError::ParseError);
    EXPECT_FALSE(broken.error.empty());

    const hmi::SkinCatalogResult notObject = hmi::SkinCatalog::loadFromString("[1, 2, 3]");
    EXPECT_FALSE(notObject.ok());
    EXPECT_EQ(notObject.errorCode, hmi::SkinCatalogError::ParseError);
}

/**
 * @brief Une version de format inconnue est refusee, pas lue au mieux.
 * \castest{<b>Une version de format superieure a celle geree est refusee explicitement.</b><br/>
 * \tcat Unitaire · Catalogue de skins<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Lire un catalogue dont la version vaut celle geree, puis une superieure.<br/>
 * \tattendu La premiere est acceptee ; la seconde echoue avec le code UnsupportedVersion.
 * }
 */
TEST(SkinCatalogTest, VersionInconnueRefusee) {
    const hmi::SkinCatalogResult current =
        hmi::SkinCatalog::loadFromString(R"({"version": 1, "jeux": {}})");
    EXPECT_TRUE(current.ok()) << current.error;

    // Refuser plutot que lire au mieux : c'est precisement ce que le champ de version sert a
    // eviter.
    const hmi::SkinCatalogResult future =
        hmi::SkinCatalog::loadFromString(R"({"version": 99, "jeux": {}})");
    EXPECT_FALSE(future.ok());
    EXPECT_EQ(future.errorCode, hmi::SkinCatalogError::UnsupportedVersion);
}

/**
 * @brief Un type de tuile ou un mode inconnu est signale.
 * \castest{<b>Un type de tuile ou un mode de skin inconnu est signale a la lecture.</b><br/>
 * \tcat Unitaire · Catalogue de skins<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Lire un catalogue citant un type de tuile inexistant.<br/>
 * 2. Lire un catalogue citant un mode inexistant, puis une entree sans asset.<br/>
 * \tattendu Les trois echouent avec le code MalformedStructure, sans exception.
 * }
 */
TEST(SkinCatalogTest, TypeOuModeInconnuSignale) {
    const hmi::SkinCatalogResult badType =
        hmi::SkinCatalog::loadFromString(R"({"jeux": {"foret": {"mur": {"asset": "a.png"}}}})");
    EXPECT_FALSE(badType.ok());
    EXPECT_EQ(badType.errorCode, hmi::SkinCatalogError::MalformedStructure);

    const hmi::SkinCatalogResult badMode = hmi::SkinCatalog::loadFromString(
        R"({"jeux": {"foret": {"solid": {"asset": "a.png", "mode": "bitmask47"}}}})");
    EXPECT_FALSE(badMode.ok());
    EXPECT_EQ(badMode.errorCode, hmi::SkinCatalogError::MalformedStructure);

    const hmi::SkinCatalogResult noAsset =
        hmi::SkinCatalog::loadFromString(R"({"jeux": {"foret": {"solid": {"mode": "single"}}}})");
    EXPECT_FALSE(noAsset.ok());
    EXPECT_EQ(noAsset.errorCode, hmi::SkinCatalogError::MalformedStructure);
}

/**
 * @brief Un jeu par defaut inexistant est signale.
 * \castest{<b>Un jeu par defaut qui ne designe aucun jeu existant est signale.</b><br/>
 * \tcat Unitaire · Catalogue de skins<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Lire un catalogue dont le champ « defaut » cite un jeu absent de « jeux ».<br/>
 * \tattendu La lecture echoue avec le code MalformedStructure.
 * }
 */
TEST(SkinCatalogTest, DefautInexistantSignale) {
    const hmi::SkinCatalogResult result =
        hmi::SkinCatalog::loadFromString(R"({"defaut": "banquise", "jeux": {"foret": {}}})");

    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, hmi::SkinCatalogError::MalformedStructure);
}

/**
 * @brief Un mode absent vaut « single ».
 * \castest{<b>Une entree sans champ « mode » est lue en mode image unique.</b><br/>
 * \tcat Unitaire · Catalogue de skins<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Lire une entree ne portant que le champ « asset ».<br/>
 * \tattendu Elle resout en mode Single.
 * }
 */
TEST(SkinCatalogTest, ModeAbsentVautSingle) {
    const hmi::SkinCatalogResult result =
        hmi::SkinCatalog::loadFromString(R"({"jeux": {"foret": {"block": {"asset": "c.png"}}}})");
    ASSERT_TRUE(result.ok()) << result.error;

    const std::optional<hmi::SkinEntry> entry =
        result.catalog->resolve("foret", core::TileType::Block);
    ASSERT_TRUE(entry.has_value());
    EXPECT_EQ(entry->mode, hmi::SkinMode::Single);
}

/**
 * @brief Un fichier absent est signale comme tel, sans exception.
 * \castest{<b>Un fichier de skins absent est signale par un code dedie, sans exception.</b><br/>
 * \tcat Unitaire · Catalogue de skins<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Lire un chemin qui n'existe pas.<br/>
 * \tattendu La lecture echoue avec le code FileNotFound, sans exception.
 * }
 */
TEST(SkinCatalogTest, FichierAbsentSignale) {
    const hmi::SkinCatalogResult result =
        hmi::SkinCatalog::loadFromFile(tempSkinsPath("inexistant_jamais_ecrit"));

    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, hmi::SkinCatalogError::FileNotFound);
}

/**
 * @brief L'ecriture sur disque produit un fichier relisible.
 * \castest{<b>Le catalogue ecrit sur disque est relu a l'identique.</b><br/>
 * \tcat Unitaire · Catalogue de skins<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Ecrire le catalogue de reference dans un fichier temporaire.<br/>
 * 2. Le relire depuis ce fichier.<br/>
 * \tattendu La relecture reussit et rend les memes assignations ; le fichier est supprime.
 * }
 */
TEST(SkinCatalogTest, EcritureSurDisqueRelisible) {
    const std::filesystem::path path = tempSkinsPath("ecriture");
    std::filesystem::remove(path);

    const hmi::SkinCatalog catalog = referenceCatalog();
    ASSERT_TRUE(catalog.saveToFile(path));

    const hmi::SkinCatalogResult reread = hmi::SkinCatalog::loadFromFile(path);
    ASSERT_TRUE(reread.ok()) << reread.error;
    EXPECT_EQ(reread.catalog->assignments("foret"), catalog.assignments("foret"));
    EXPECT_EQ(reread.catalog->defaultSetName(), catalog.defaultSetName());

    std::filesystem::remove(path);
}

/**
 * @brief Assigner et retirer un skin modifie la resolution en consequence.
 * \castest{<b>Assigner puis retirer un skin change la resolution du type concerne.</b><br/>
 * \tcat Unitaire · Catalogue de skins<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Assigner un asset a Danger dans un catalogue vide, puis resoudre.<br/>
 * 2. Retirer l'assignation, puis resoudre a nouveau.<br/>
 * \tattendu La premiere resolution rend l'asset ; la seconde ne rend plus rien.
 * }
 */
TEST(SkinCatalogTest, AssignationPuisRetrait) {
    hmi::SkinCatalog catalog;
    catalog.assign("foret", core::TileType::Danger,
                   hmi::SkinEntry{"spikes.png", hmi::SkinMode::Single});

    // Le premier jeu cree devient le defaut : sans cela un catalogue construit par assignation
    // successive ne resoudrait rien.
    EXPECT_EQ(catalog.defaultSetName(), "foret");

    const std::optional<hmi::SkinEntry> assigned = catalog.resolve("foret", core::TileType::Danger);
    ASSERT_TRUE(assigned.has_value());
    EXPECT_EQ(assigned->asset, "spikes.png");

    catalog.clearAssignment("foret", core::TileType::Danger);
    EXPECT_FALSE(catalog.resolve("foret", core::TileType::Danger).has_value());
}

/**
 * @brief Les noms de jeux sont rendus dans un ordre stable.
 * \castest{<b>Les noms de jeux sont rendus dans un ordre alphabetique stable.</b><br/>
 * \tcat Unitaire · Catalogue de skins<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Creer trois jeux dans un ordre quelconque.<br/>
 * \tattendu setNames les rend tries, quel que soit l'ordre de creation.
 * }
 */
TEST(SkinCatalogTest, OrdreDesJeuxStable) {
    hmi::SkinCatalog catalog;
    catalog.addSet("grotte");
    catalog.addSet("banquise");
    catalog.addSet("foret");

    // Ordre stable = fichier ecrit reproductible : un diff de skins.json ne montre que ce que
    // l'utilisateur a reellement change.
    const std::vector<std::string> expected{"banquise", "foret", "grotte"};
    EXPECT_EQ(catalog.setNames(), expected);
}

/**
 * @brief Le catalogue livre avec le jeu se lit sans erreur.
 * \castest{<b>Le fichier de skins livre avec le jeu se lit sans erreur.</b><br/>
 * \tcat Unitaire · Catalogue de skins<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Lire Source/Elements/Assets/skins.json depuis les sources.<br/>
 * \tattendu La lecture reussit et le jeu par defaut designe un jeu existant.
 * }
 */
TEST(SkinCatalogTest, CatalogueLivreValide) {
    const std::filesystem::path path = std::filesystem::path(JADG_ASSETS_DIR) / "skins.json";
    ASSERT_TRUE(std::filesystem::exists(path)) << path.string();

    const hmi::SkinCatalogResult result = hmi::SkinCatalog::loadFromFile(path);
    ASSERT_TRUE(result.ok()) << result.error;

    const std::vector<std::string> sets = result.catalog->setNames();
    ASSERT_FALSE(sets.empty());
    EXPECT_NE(std::find(sets.begin(), sets.end(), result.catalog->defaultSetName()), sets.end())
        << "le jeu par defaut doit exister";
}

/**
 * @brief Chaque asset reference par le catalogue livre existe et respecte son contrat.
 * \castest{<b>Chaque asset reference par le catalogue livre existe aux bonnes dimensions.</b><br/>
 * \tcat Unitaire · Catalogue de skins<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Lire le catalogue livre et parcourir toutes les assignations de tous les jeux.<br/>
 * 2. Pour chaque asset, verifier sa presence dans Assets/Skins puis ses dimensions.<br/>
 * \tattendu Un skin single fait une case (ou une bande horizontale de cases pour un skin single
 * ANIME, `LOT-46`), une planche a raccords en fait 4x4.
 * }
 */
TEST(SkinCatalogTest, AssetsDuCatalogueLivreConformes) {
    const std::filesystem::path assets{JADG_ASSETS_DIR};
    const hmi::SkinCatalogResult result = hmi::SkinCatalog::loadFromFile(assets / "skins.json");
    ASSERT_TRUE(result.ok()) << result.error;

    constexpr int TILE = hmi::TextureAtlas::TILE_SIZE;
    for (const std::string& setName : result.catalog->setNames()) {
        for (const auto& [type, entry] : result.catalog->assignments(setName)) {
            const std::filesystem::path path = assets / "Skins" / entry.asset;
            ASSERT_TRUE(std::filesystem::exists(path))
                << "asset reference mais absent : " << path.string();

            const std::optional<std::pair<int, int>> size = pngSize(path);
            ASSERT_TRUE(size.has_value()) << "PNG illisible : " << path.string();

            // Un asset aux mauvaises dimensions est refuse au chargement (EX-REN-007) et la tuile
            // retombe au damier : le catalogue livre serait alors muet mais inoperant. Une planche
            // a raccords fait exactement 4x4 cases ; un skin single fait toujours UNE case de haut,
            // mais peut faire plusieurs cases de large s'il est anime (bande horizontale,
            // Skins/README.md, LOT-46) -- la largeur exacte depend alors du nombre d'images, propre
            // a chaque asset, pas une constante unique comme pour bitmask16.
            if (entry.mode == hmi::SkinMode::Bitmask16) {
                EXPECT_EQ(size->first, TILE * hmi::AUTOTILE_SHEET_SIDE)
                    << entry.asset << " pour le type " << core::tileTypeName(type);
                EXPECT_EQ(size->second, TILE * hmi::AUTOTILE_SHEET_SIDE) << entry.asset;
            } else {
                EXPECT_EQ(size->second, TILE)
                    << entry.asset << " pour le type " << core::tileTypeName(type);
                EXPECT_GT(size->first, 0) << entry.asset;
                EXPECT_EQ(size->first % TILE, 0)
                    << entry.asset << " : largeur non multiple d'une case (single anime attendu "
                    << "en bande horizontale, TILE*N).";
            }
        }
    }
}
