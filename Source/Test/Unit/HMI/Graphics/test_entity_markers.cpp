// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_entity_markers.cpp
 * @brief Tests unitaires du marqueur d'entité de carte : clé d'asset et pixels (LOT-11). Pur, sans
 *        GPU.
 */

#include <cstdint>
#include <string>

#include <gtest/gtest.h>

#include "Core/Resources/AssetKey.h"
#include "Core/Resources/AssetMarker.h"
#include "Core/World/EntityKinds.h"
#include "HMI/Graphics/EntityMarkers.h"

/**
 * @brief Un type `camelCase` devient une clé `marker/kebab-case`.
 * \castest{<b>Le type d'entite devient une cle de marqueur en kebab-case.</b><br/>
 * \tcat Unitaire · Marqueurs d'entite<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Convertir spawnPoint, arenaEntry, chest, NPCGuard et chest2.<br/>
 * \tattendu marker/spawn-point, marker/arena-entry, marker/chest, marker/npc-guard, marker/chest2.
 * }
 */
TEST(EntityMarkersTest, TypeCamelCaseDevientUneCleKebabCase) {
    EXPECT_EQ(hmi::entityMarkerKey("spawnPoint"), "marker/spawn-point");
    EXPECT_EQ(hmi::entityMarkerKey("arenaEntry"), "marker/arena-entry");
    EXPECT_EQ(hmi::entityMarkerKey("chest"), "marker/chest");
    EXPECT_EQ(hmi::entityMarkerKey("NPCGuard"), "marker/npc-guard");
    EXPECT_EQ(hmi::entityMarkerKey("chest2"), "marker/chest2");
}

/**
 * @brief Séparateurs regroupés, caractères interdits ignorés, tirets de bord supprimés.
 * \castest{<b>Caracteres interdits ignores, separateurs normalises.</b><br/>
 * \tcat Unitaire · Marqueurs d'entite<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Convertir des types avec tirets bas, espaces, ponctuation et accents.<br/>
 * \tattendu Des cles sans tiret double ni tiret de bord, sans caractere interdit.
 * }
 */
TEST(EntityMarkersTest, CaracteresInterditsIgnoresEtSeparateursNormalises) {
    EXPECT_EQ(hmi::entityMarkerKey("_old__chest_"), "marker/old-chest");
    EXPECT_EQ(hmi::entityMarkerKey("  wooden - sign "), "marker/wooden-sign");
    EXPECT_EQ(hmi::entityMarkerKey("a.b!c"), "marker/abc");
    EXPECT_EQ(hmi::entityMarkerKey("a.B"), "marker/a-b");
    EXPECT_EQ(hmi::entityMarkerKey("coffre\xC3\xA9"), "marker/coffre");
}

/**
 * @brief Un type vide, ou sans aucun caractère utilisable, donne `marker/inconnu`.
 * \castest{<b>Un type vide donne le marqueur inconnu.</b><br/>
 * \tcat Unitaire · Marqueurs d'entite<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Convertir "", "---" et "!?".<br/>
 * \tattendu marker/inconnu dans les trois cas.
 * }
 */
TEST(EntityMarkersTest, TypeVideDonneLeMarqueurInconnu) {
    EXPECT_EQ(hmi::entityMarkerKey(""), "marker/inconnu");
    EXPECT_EQ(hmi::entityMarkerKey("---"), "marker/inconnu");
    EXPECT_EQ(hmi::entityMarkerKey("!?"), "marker/inconnu");
}

/**
 * @brief Toute famille connue de l'éditeur, et quelques types hostiles, donnent une clé valide dont
 *        le marqueur se peint.
 * \castest{<b>Chaque cle de marqueur est une cle d'asset valide qui se peint.</b><br/>
 * \tcat Unitaire · Marqueurs d'entite<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Convertir chaque type de knownEntityKinds et des types hostiles.<br/>2. Valider la
 * cle et peindre le marqueur.<br/>
 * \tattendu Cle valide, image non vide a la taille d'un marqueur.
 * }
 */
TEST(EntityMarkersTest, ChaqueCleEstValideEtSePeint) {
    std::vector<std::string> types;
    for (const core::EntityKind& kind : core::knownEntityKinds()) {
        types.emplace_back(kind.type);
    }
    types.insert(types.end(), {"", "Weird Type", "x/y", "--a--", "\xFF"});
    for (const std::string& type : types) {
        const std::string key = hmi::entityMarkerKey(type);
        EXPECT_TRUE(core::isValidAssetKey(key)) << type << " -> " << key;
        const core::MarkerImage image =
            core::assetMarker(key, hmi::ENTITY_MARKER_SIZE_PIXELS, hmi::ENTITY_MARKER_SIZE_PIXELS);
        EXPECT_FALSE(image.isEmpty()) << key;
    }
}

/**
 * @brief Les pixels sont empaquetés `R | G<<8 | B<<16 | A<<24`, ligne par ligne.
 * \castest{<b>Les pixels du marqueur sont empaquetes au format de createTexture.</b><br/>
 * \tcat Unitaire · Marqueurs d'entite<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Empaqueter une image 2x1 connue.<br/>2. Empaqueter une image vide.<br/>
 * \tattendu Octets dans l'ordre RGBA petit-boutiste ; vecteur vide pour l'image vide.
 * }
 */
TEST(EntityMarkersTest, PixelsEmpaquetesEnRgba8) {
    core::MarkerImage image;
    image.width = 2;
    image.height = 1;
    image.pixels = {core::MarkerColor{0x11, 0x22, 0x33, 0x44},
                    core::MarkerColor{0xAA, 0xBB, 0xCC, 0xFF}};
    const std::vector<std::uint32_t> pixels = hmi::markerPixelsRgba8(image);
    ASSERT_EQ(pixels.size(), 2u);
    EXPECT_EQ(pixels[0], 0x44332211u);
    EXPECT_EQ(pixels[1], 0xFFCCBBAAu);

    EXPECT_TRUE(hmi::markerPixelsRgba8(core::MarkerImage{}).empty());
}
