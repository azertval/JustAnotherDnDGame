// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_level_name_validation.cpp
 * @brief Tests unitaires de la validation d'un nom de niveau saisi (EX-EDIT-009).
 */

#include <gtest/gtest.h>

#include "HMI/Editor/LevelNameValidation.h"

/**
 * @brief Un nom simple, sans caractère interdit, est valide.
 * \castest{<b>Un nom simple, sans caractère interdit, est valide.</b><br/>
 * \tcat Unitaire · Level Name Validation<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un nom simple, sans caractère interdit, est valide.
 * }
 */
TEST(LevelNameValidationTest, NomSimpleValide) {
    EXPECT_TRUE(hmi::isValidLevelName("Niveau 1"));
    EXPECT_TRUE(hmi::isValidLevelName("Foret enchantee"));
}

/**
 * @brief Un nom vide ou composé uniquement d'espaces est invalide.
 * \castest{<b>Un nom vide ou composé uniquement d'espaces est invalide.</b><br/>
 * \tcat Unitaire · Level Name Validation<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un nom vide ou composé uniquement d'espaces est invalide.
 * }
 */
TEST(LevelNameValidationTest, NomVideOuEspacesInvalide) {
    EXPECT_FALSE(hmi::isValidLevelName(""));
    EXPECT_FALSE(hmi::isValidLevelName("   "));
}

/**
 * @brief Un nom contenant un caractère interdit par le système de fichiers Windows est invalide.
 * \castest{<b>Un nom contenant un caractère interdit par le système de fichiers Windows est
 * invalide.</b><br/>
 * \tcat Unitaire · Level Name Validation<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un nom contenant un caractère interdit par le système de fichiers Windows est
 * invalide.
 * }
 */
TEST(LevelNameValidationTest, CaractereInterditInvalide) {
    EXPECT_FALSE(hmi::isValidLevelName("Niveau:1"));
    EXPECT_FALSE(hmi::isValidLevelName("Niveau/1"));
    EXPECT_FALSE(hmi::isValidLevelName("Niveau\\1"));
    EXPECT_FALSE(hmi::isValidLevelName("Niveau*1"));
    EXPECT_FALSE(hmi::isValidLevelName("Niveau?1"));
    EXPECT_FALSE(hmi::isValidLevelName("Niveau\"1"));
    EXPECT_FALSE(hmi::isValidLevelName("Niveau<1"));
    EXPECT_FALSE(hmi::isValidLevelName("Niveau>1"));
    EXPECT_FALSE(hmi::isValidLevelName("Niveau|1"));
}

/**
 * @brief Un nom accentué (Unicode) reste valide : la liste noire ne porte que sur les caractères
 *        interdits par le système de fichiers, pas sur une liste blanche restrictive.
 * \castest{<b>Un nom accentué (Unicode) reste valide.</b><br/>
 * \tcat Unitaire · Level Name Validation<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un nom accentué (Unicode) reste valide.
 * }
 */
TEST(LevelNameValidationTest, NomAccentueValide) {
    EXPECT_TRUE(hmi::isValidLevelName("Fort\xC3\xA9resse"));  // "Fortéresse" (UTF-8)
}

/**
 * @brief trimLevelName retire les espaces de bord sans toucher au contenu.
 * \castest{<b>trimLevelName retire les espaces de bord sans toucher au contenu.</b><br/>
 * \tcat Unitaire · Level Name Validation<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu trimLevelName retire les espaces de bord sans toucher au contenu.
 * }
 */
TEST(LevelNameValidationTest, TrimRetireLesEspacesDeBord) {
    EXPECT_EQ(hmi::trimLevelName("  Niveau 1  "), "Niveau 1");
    EXPECT_EQ(hmi::trimLevelName("Niveau 1"), "Niveau 1");
    EXPECT_EQ(hmi::trimLevelName("   "), "");
}
