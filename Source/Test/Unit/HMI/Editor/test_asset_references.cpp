// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_asset_references.cpp
 * @brief Tests unitaires de la détection des références à un asset (LOT-43 TACHE-02).
 */

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Levels/TileType.h"
#include "HMI/Editor/AssetReferences.h"
#include "HMI/Graphics/SkinCatalog.h"

/**
 * @brief Un asset cité par un jeu de skins est retrouvé, avec le nom du jeu qui le cite : c'est
 * ce qui permet d'avertir avant de supprimer ou renommer un fichier encore utilisé.
 * \castest{<b>Un asset cité par un jeu de skins est retrouvé, avec le nom du jeu.</b><br/>
 * \tcat Unitaire · Références d'assets<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(AssetReferencesTest, AssetCiteParUnJeuEstTrouve) {
    hmi::SkinCatalog catalog;
    catalog.assign("foret", core::TileType::Solid,
                   hmi::SkinEntry{"stone.png", hmi::SkinMode::Bitmask16});

    const std::vector<hmi::AssetReference> references =
        hmi::findSkinCatalogReferences(catalog, "stone.png");

    ASSERT_EQ(references.size(), 1U);
    EXPECT_EQ(references.front().setName, "foret");
}

/**
 * @brief Chaque citation compte séparément : un même fichier utilisé par deux types dans un jeu
 * et par un troisième dans un autre donne **trois** références. L'avertissement doit énumérer tout
 * ce qui casserait, pas seulement le premier usage rencontré.
 * \castest{<b>Un asset cité par plusieurs jeux et types donne une référence par citation.</b><br/>
 * \tcat Unitaire · Références d'assets<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(AssetReferencesTest, AssetCiteParPlusieursJeuxEtTypesEstTrouveAChaqueFois) {
    hmi::SkinCatalog catalog;
    catalog.assign("foret", core::TileType::Solid,
                   hmi::SkinEntry{"stone.png", hmi::SkinMode::Bitmask16});
    catalog.assign("foret", core::TileType::Block,
                   hmi::SkinEntry{"stone.png", hmi::SkinMode::Single});
    catalog.assign("grotte", core::TileType::Solid,
                   hmi::SkinEntry{"stone.png", hmi::SkinMode::Bitmask16});

    const std::vector<hmi::AssetReference> references =
        hmi::findSkinCatalogReferences(catalog, "stone.png");

    EXPECT_EQ(references.size(), 3U);
}

/**
 * @brief Un asset que rien ne cite ne donne aucune référence : c'est le cas où la suppression est
 * sans danger, et où aucun avertissement ne doit venir déranger l'auteur.
 * \castest{<b>Un asset non référencé ne donne aucune référence.</b><br/>
 * \tcat Unitaire · Références d'assets<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(AssetReferencesTest, AssetNonReferenceNeDonneAucuneReference) {
    hmi::SkinCatalog catalog;
    catalog.assign("foret", core::TileType::Solid,
                   hmi::SkinEntry{"stone.png", hmi::SkinMode::Bitmask16});

    EXPECT_TRUE(hmi::findSkinCatalogReferences(catalog, "grass.png").empty());
}

/**
 * @brief La comparaison porte sur le nom **entier**, pas sur un préfixe : `stone2.png` ne compte
 * pas comme une référence à `stone.png`. Un faux positif ferait renoncer à une suppression
 * pourtant sans risque, et userait la confiance dans l'avertissement.
 * \castest{<b>Un nom proche mais différent ne déclenche pas de fausse référence.</b><br/>
 * \tcat Unitaire · Références d'assets<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(AssetReferencesTest, NomProcheMaisDifferentNeDeclenchePasDeFauxPositif) {
    hmi::SkinCatalog catalog;
    catalog.assign("foret", core::TileType::Solid,
                   hmi::SkinEntry{"stone2.png", hmi::SkinMode::Bitmask16});

    EXPECT_TRUE(hmi::findSkinCatalogReferences(catalog, "stone.png").empty());
}

/**
 * @brief Sans référence, le message descriptif est vide — pas une phrase du genre « utilisé par
 * (rien) » que l'appelant devrait ensuite filtrer : une chaîne vide se teste directement.
 * \castest{<b>Sans référence, le message descriptif est vide.</b><br/>
 * \tcat Unitaire · Références d'assets<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(AssetReferencesTest, DescribeReferencesEstVideSansReference) {
    EXPECT_TRUE(hmi::describeReferences({}).empty());
}

/**
 * @brief Le message nomme **les jeux et les types** concernés, pas seulement leur nombre : c'est
 * ce qui permet à l'auteur d'aller corriger les usages avant de confirmer une suppression.
 * \castest{<b>Le message descriptif nomme les jeux de skins et les types concernés.</b><br/>
 * \tcat Unitaire · Références d'assets<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(AssetReferencesTest, DescribeReferencesNommeLesJeuxEtLesTypes) {
    const std::vector<hmi::AssetReference> references{
        {"foret", "solid"},
        {"foret", "block"},
        {"grotte", "solid"},
    };

    const std::string message = hmi::describeReferences(references);

    EXPECT_NE(message.find("foret"), std::string::npos);
    EXPECT_NE(message.find("grotte"), std::string::npos);
    EXPECT_NE(message.find("solid"), std::string::npos);
    EXPECT_NE(message.find("block"), std::string::npos);
}
