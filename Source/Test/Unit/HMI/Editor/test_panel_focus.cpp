// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_panel_focus.cpp
 * @brief Tests unitaires de la correspondance outil → panneau (EX-IHM-061).
 *        Logique pure, sans Qt.
 */

#include <gtest/gtest.h>

#include "HMI/Editor/PanelFocus.h"

/**
 * @brief L'outil Entité met en avant le panneau Entités.
 * \castest{<b>L'outil Entite met en avant le panneau Entites.</b><br/>
 * \tcat Unitaire · Mise en avant des panneaux<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Interroger la table pour l'outil Entite.<br/>2. Verifier le panneau retourne.<br/>
 * \tattendu Le panneau Entites est retourne.
 * }
 */
TEST(PanelFocusTest, OutilEntiteMetEnAvantLePanneauEntites) {
    EXPECT_EQ(hmi::panelForTool(hmi::EditorTool::Entity), hmi::PanelId::Entities);
}

/**
 * @brief Les outils sans panneau dedie (Pinceau, Rectangle, Selection) ne mettent rien en
 *        avant : leurs controles vivent ailleurs (la palette).
 * \castest{<b>Les outils sans panneau dedie ne mettent rien en avant.</b><br/>
 * \tcat Unitaire · Mise en avant des panneaux<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Interroger la table pour chaque outil sans panneau dedie.<br/>2. Verifier
 * l'absence de resultat.<br/>
 * \tattendu Chaque appel retourne std::nullopt.
 * }
 */
TEST(PanelFocusTest, OutilsSansPanneauDedieNeMettentRienEnAvant) {
    for (const hmi::EditorTool tool :
         {hmi::EditorTool::Paint, hmi::EditorTool::Rectangle, hmi::EditorTool::Selection}) {
        EXPECT_EQ(hmi::panelForTool(tool), std::nullopt) << "outil " << static_cast<int>(tool);
    }
}

/**
 * @brief La table ne contient aucun doublon d'outil : chaque outil n'a au plus une entree.
 * \castest{<b>La table ne contient aucun doublon d'outil.</b><br/>
 * \tcat Unitaire · Mise en avant des panneaux<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Parcourir toutes les paires d'entrees de la table.<br/>2. Comparer leurs outils.<br/>
 * \tattendu Aucune paire distincte ne partage le meme outil.
 * }
 */
TEST(PanelFocusTest, AucunDoublonDOutilDansLaTable) {
    const auto& catalog = hmi::panelFocusCatalog();
    for (std::size_t i = 0; i < catalog.size(); ++i) {
        for (std::size_t j = i + 1; j < catalog.size(); ++j) {
            EXPECT_NE(catalog[i].tool, catalog[j].tool) << i << " / " << j;
        }
    }
}

