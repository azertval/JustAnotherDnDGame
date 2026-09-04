// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include <filesystem>
#include <set>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Levels/TileType.h"
#include "HMI/Editor/TaxonomyLabels.h"
#include "HMI/Editor/TileTaxonomy.h"
#include "HMI/Localization/Localization.h"

namespace {

// Aplati la taxonomie en la liste de tous les types qu'elle contient (directs + sous-groupes),
// dans l'ordre d'affichage.
std::vector<core::TileType> flatten(const std::vector<hmi::TileCategory>& taxonomy) {
    std::vector<core::TileType> types;
    for (const hmi::TileCategory& category : taxonomy) {
        for (const hmi::TileEntry& entry : category.tiles) {
            types.push_back(entry.type);
        }
        for (const hmi::TileSubgroup& subgroup : category.subgroups) {
            for (const hmi::TileEntry& entry : subgroup.tiles) {
                types.push_back(entry.type);
            }
        }
    }
    return types;
}

// Nombre total de valeurs de l'énumération core::TileType. Dérivé de core::TILE_TYPE_COUNT
// (Core/Levels/TileType.h), seule source de vérité depuis le LOT-74 TACHE-02 : ce fichier
// recopiait jusque-là la borne « dernier énumérateur » à la main, quatrième copie d'un même fait
// dont l'ajout d'un type en fin d'énumération exigeait la mise à jour manuelle.
constexpr std::size_t TILE_TYPE_COUNT = static_cast<std::size_t>(core::TILE_TYPE_COUNT);

}  // namespace

/**
 * @brief Chaque type de tuile figure exactement une fois dans la taxonomie : aucun doublon, aucun
 * oubli. C'est le garde-fou qui casse si un `core::TileType` est ajouté sans être rangé dans la
 * palette — il resterait sinon impossible à peindre, sans qu'aucun test ne le signale.
 * \castest{<b>Chaque type de tuile figure exactement une fois dans la taxonomie.</b><br/>
 * \tcat Unitaire · Taxonomie des tuiles<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(TileTaxonomy, ChaqueTypeFigureExactementUneFois) {
    const std::vector<core::TileType> types = flatten(hmi::tileTaxonomy());

    EXPECT_EQ(types.size(), TILE_TYPE_COUNT)
        << "la taxonomie doit couvrir tous les types de tuiles";

    const std::set<core::TileType> unique(types.begin(), types.end());
    EXPECT_EQ(unique.size(), types.size()) << "aucun type ne doit apparaitre deux fois";
    EXPECT_EQ(unique.size(), TILE_TYPE_COUNT) << "aucun type ne doit manquer";
}

/**
 * @brief Toute entrée de la taxonomie — catégorie, sous-groupe et tuile — porte un libellé non
 * vide : c'est ce texte qui s'affiche dans l'arbre de la palette, une chaîne vide y produirait une
 * ligne muette impossible à identifier.
 * \castest{<b>Chaque catégorie, sous-groupe et tuile de la taxonomie porte un libellé non
 * vide.</b><br/>
 * \tcat Unitaire · Taxonomie des tuiles<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(TileTaxonomy, ChaqueEntreeAUnLibelle) {
    for (const hmi::TileCategory& category : hmi::tileTaxonomy()) {
        EXPECT_FALSE(category.label.empty());
        for (const hmi::TileEntry& entry : category.tiles) {
            EXPECT_FALSE(entry.label.empty());
        }
        for (const hmi::TileSubgroup& subgroup : category.subgroups) {
            EXPECT_FALSE(subgroup.label.empty());
            for (const hmi::TileEntry& entry : subgroup.tiles) {
                EXPECT_FALSE(entry.label.empty());
            }
        }
    }
}

/**
 * @brief Chaque libellé de la palette est **traduit** dans les deux catalogues livrés : un type
 * ajouté sans libellé s'afficherait en français dans la version anglaise, sans que rien ne
 * l'annonce (`EX-REN-033`, `LOT-08`).
 * \castest{<b>Chaque libelle de la palette est traduit dans les deux catalogues livres.</b><br/>
 * \tcat Unitaire · Taxonomie des tuiles<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger fr.lang puis en.lang.<br/>2. Resoudre la cle de chaque categorie et de
 * chaque tuile de la taxonomie.<br/>
 * \tattendu Chaque libelle a une cle, et chaque cle resout vers un texte distinct d'elle-meme.
 * }
 */
TEST(TileTaxonomy, ChaqueLibelleEstTraduitDansLesDeuxCatalogues) {
    const std::filesystem::path directory(JADG_LOCALIZATION_DIR);
    for (const std::string& language : {"fr", "en"}) {
        hmi::Localization localization(directory);
        ASSERT_TRUE(localization.loadDefaultLanguage(language)) << language;

        for (const hmi::TileCategory& category : hmi::tileTaxonomy()) {
            const std::string categoryKey = hmi::taxonomyLabelKey(category.label);
            ASSERT_FALSE(categoryKey.empty()) << category.label << " (" << language << ")";
            EXPECT_NE(localization.text(categoryKey), categoryKey)
                << categoryKey << " (" << language << ")";
            for (const hmi::TileEntry& entry : category.tiles) {
                const std::string key = hmi::taxonomyLabelKey(entry.label);
                ASSERT_FALSE(key.empty()) << entry.label << " (" << language << ")";
                EXPECT_NE(localization.text(key), key) << key << " (" << language << ")";
            }
        }
    }
}
