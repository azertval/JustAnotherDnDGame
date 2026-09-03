// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_editor_workspace.cpp
 * @brief Tests unitaires des espaces de travail de l'éditeur (`LOT-68`, `EX-IHM-073`).
 */

#include <set>

#include <gtest/gtest.h>

#include "HMI/Editor/EditorTool.h"
#include "HMI/Editor/PanelFocus.h"
#include "HMI/Editor/PixelTool.h"
#include "HMI/Interface/EditorWorkspace.h"

namespace {

/// Les neuf panneaux, énumérés une seule fois ici pour que les tests ci-dessous les parcourent
/// tous. `PANEL_COUNT` ferme la boucle : un panneau ajouté à `PanelId` sans être ajouté ici fait
/// échouer le premier test, plutôt que de passer inaperçu.
constexpr hmi::PanelId ALL_PANELS[] = {
    hmi::PanelId::Palette,     hmi::PanelId::Planes,       hmi::PanelId::Levels,
    hmi::PanelId::Links,       hmi::PanelId::Textures,
    hmi::PanelId::PixelCanvas, hmi::PanelId::PixelHistory, hmi::PanelId::PixelPalette,
};

/// Les trois espaces de travail, même rôle de garde que `ALL_PANELS`.
constexpr hmi::EditorWorkspace ALL_WORKSPACES[] = {
    hmi::EditorWorkspace::Level,
    hmi::EditorWorkspace::Planes,
    hmi::EditorWorkspace::PixelArt,
};

constexpr hmi::EditorTool ALL_TOOLS[] = {
    hmi::EditorTool::Paint, hmi::EditorTool::Rectangle,     hmi::EditorTool::Selection,
    hmi::EditorTool::Link,  hmi::EditorTool::TextureAssign, hmi::EditorTool::CameraZone,
};

constexpr hmi::PixelTool ALL_PIXEL_TOOLS[] = {
    hmi::PixelTool::Brush,      hmi::PixelTool::Eraser,    hmi::PixelTool::Fill,
    hmi::PixelTool::Eyedropper, hmi::PixelTool::Selection,
};

}  // namespace

/**
 * @brief Chaque panneau appartient à **au moins un** espace, et les neuf sont couverts.
 *
 * La garde a changé de nature au `LOT-69` TACHE-08 : elle vérifiait « exactement un espace », elle
 * vérifie désormais « **masque non vide** ». Le canevas, l'historique et la palette servent aux
 * deux espaces de peinture, et les dupliquer donnerait deux canevas et deux historiques à tenir
 * synchronisés. Un panneau sans aucun espace, lui, resterait affiché partout — ce qui viderait la
 * séparation de son sens, et ne se verrait qu'à l'écran, jamais en relecture.
 * \castest{<b>Chaque panneau appartient a au moins un espace de travail.</b><br/>
 * \tcat Unitaire · Espaces de travail<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Parcourir les neuf panneaux et resoudre leur masque d'espaces.<br/>2. Verifier que
 * le compte couvert vaut PANEL_COUNT et que les trois espaces sont representes.<br/>
 * \tattendu Les neuf panneaux ont un masque non vide, et chaque espace affiche au moins un
 * panneau.
 * }
 */
TEST(EditorWorkspaceTest, ChaquePanneauAppartientAAuMoinsUnEspace) {
    std::set<hmi::PanelId> seen;
    std::set<int> workspacesCovered;
    for (const hmi::PanelId panel : ALL_PANELS) {
        EXPECT_TRUE(seen.insert(panel).second) << "panneau enumere deux fois";
        const hmi::EditorWorkspaceMask mask = hmi::workspacesForPanel(panel);
        EXPECT_NE(mask, 0u) << "un panneau sans espace resterait affiche partout";
        for (const hmi::EditorWorkspace workspace : ALL_WORKSPACES) {
            if (hmi::workspaceMaskContains(mask, workspace)) {
                workspacesCovered.insert(static_cast<int>(workspace));
            }
        }
    }
    EXPECT_EQ(seen.size(), hmi::PANEL_COUNT)
        << "un panneau de PanelId n'a pas de masque : il resterait affiche partout";
    EXPECT_EQ(workspacesCovered.size(), hmi::EDITOR_WORKSPACE_COUNT)
        << "un espace de travail n'affiche aucun panneau";
}

/**
 * @brief Le canevas, l'historique et la palette sont **partagés** entre les deux espaces de
 * peinture — c'est précisément ce que le masque existe pour exprimer.
 * \castest{<b>Le canevas et ses panneaux servent aux deux espaces de peinture.</b><br/>
 * \tcat Unitaire · Espaces de travail<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Resoudre le masque du canevas, de l'historique et de la palette.<br/>
 * \tattendu Chacun contient a la fois l'atelier pixel art et le mode creation.
 * }
 */
TEST(EditorWorkspaceTest, CanevasPartageEntreLesDeuxEspacesDePeinture) {
    for (const hmi::PanelId panel :
         {hmi::PanelId::PixelCanvas, hmi::PanelId::PixelHistory, hmi::PanelId::PixelPalette}) {
        const hmi::EditorWorkspaceMask mask = hmi::workspacesForPanel(panel);
        EXPECT_TRUE(hmi::workspaceMaskContains(mask, hmi::EditorWorkspace::PixelArt));
        EXPECT_TRUE(hmi::workspaceMaskContains(mask, hmi::EditorWorkspace::Planes));
        EXPECT_FALSE(hmi::workspaceMaskContains(mask, hmi::EditorWorkspace::Level));
    }
}

/**
 * @brief Les deux barres d'outils ne sont **jamais** visibles ensemble. C'est la propriété que ce
 *        lot existe pour établir : c'est leur affichage simultané qui produisait une trentaine de
 *        contrôles permanents dont les deux tiers étaient hors contexte.
 * \castest{<b>Les deux barres d'outils ne sont jamais visibles simultanement.</b><br/>
 * \tcat Unitaire · Espaces de travail<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Lire l'habillage des deux espaces.<br/>2. Verifier qu'exactement une barre est
 * visible dans chacun.<br/>
 * \tattendu Une seule barre par espace, et elle correspond a l'espace.
 * }
 */
TEST(EditorWorkspaceTest, UneSeuleBarreDOutilsParEspace) {
    // Exhaustif sur les trois espaces : jamais les deux barres ensemble, jamais aucune -- c'est la
    // propriete que le LOT-68 existe pour etablir, et que le troisieme espace ne doit pas rompre.
    for (const hmi::EditorWorkspace workspace : ALL_WORKSPACES) {
        const hmi::WorkspaceDressing dressing = hmi::dressingForWorkspace(workspace);
        EXPECT_NE(dressing.levelToolBarVisible, dressing.pixelToolBarVisible)
            << "espace " << static_cast<int>(workspace);
    }

    const hmi::WorkspaceDressing level = hmi::dressingForWorkspace(hmi::EditorWorkspace::Level);
    EXPECT_TRUE(level.levelToolBarVisible);

    // Mode creation : on y peint une image, on n'y pose pas de tuiles -- ce sont donc les outils de
    // peinture qui s'affichent, pas ceux du niveau.
    const hmi::WorkspaceDressing planes = hmi::dressingForWorkspace(hmi::EditorWorkspace::Planes);
    EXPECT_TRUE(planes.pixelToolBarVisible);

    const hmi::WorkspaceDressing pixel = hmi::dressingForWorkspace(hmi::EditorWorkspace::PixelArt);
    EXPECT_TRUE(pixel.pixelToolBarVisible);
}

/**
 * @brief Le menu « Atelier » n'existe que dans l'espace de l'atelier : ses commandes n'ont aucune
 *        cible ailleurs, et un menu grisé en permanence n'informe de rien.
 * \castest{<b>Le menu de l'atelier n'existe que dans l'espace de l'atelier.</b><br/>
 * \tcat Unitaire · Espaces de travail<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Lire l'habillage des trois espaces.<br/>
 * \tattendu Le menu de l'atelier n'est pas visible dans l'espace d'edition de niveau.
 * }
 */
TEST(EditorWorkspaceTest, MenuAtelierReserveASonEspace) {
    EXPECT_FALSE(hmi::dressingForWorkspace(hmi::EditorWorkspace::Level).workshopMenuVisible);
    EXPECT_TRUE(hmi::dressingForWorkspace(hmi::EditorWorkspace::PixelArt).workshopMenuVisible);
    // Mode creation : les commandes de l'atelier (annuler, palette, enregistrer l'image) y ont une
    // cible, le menu a donc lieu d'y etre.
    EXPECT_TRUE(hmi::dressingForWorkspace(hmi::EditorWorkspace::Planes).workshopMenuVisible);
}

/**
 * @brief Les deux familles d'outils restent **disjointes** : tout `EditorTool` mène à l'espace de
 *        niveau, tout `PixelTool` à celui de l'atelier. C'est ce qui permet à la sélection d'un
 *        outil de basculer d'espace sans que personne n'écrive de condition sur un outil précis.
 * \castest{<b>Chaque famille d'outils mene a son propre espace.</b><br/>
 * \tcat Unitaire · Espaces de travail<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Resoudre l'espace de chacun des huit outils de niveau.<br/>2. Resoudre celui des
 * cinq outils de canevas.<br/>
 * \tattendu Les outils de niveau menent tous a Level, ceux de canevas tous a PixelArt.
 * }
 */
TEST(EditorWorkspaceTest, LesDeuxFamillesDOutilsSontDisjointes) {
    std::size_t levelTools = 0;
    for (const hmi::EditorTool tool : ALL_TOOLS) {
        EXPECT_EQ(hmi::workspaceForTool(tool), hmi::EditorWorkspace::Level);
        ++levelTools;
    }
    EXPECT_EQ(levelTools, hmi::EDITOR_TOOL_COUNT)
        << "un outil de EditorTool n'est pas couvert par ce test";

    for (const hmi::PixelTool tool : ALL_PIXEL_TOOLS) {
        EXPECT_EQ(hmi::workspaceForPixelTool(tool), hmi::EditorWorkspace::PixelArt);
    }
}

/**
 * @brief Chaque entrée de la table de mise en avant désigne un panneau du **même** espace que son
 *        outil. Un outil de niveau qui mettrait en avant un panneau de l'atelier demanderait à la
 *        fenêtre d'afficher un dock que l'espace actif ne contient pas.
 * \castest{<b>La mise en avant ne traverse jamais la frontiere entre espaces.</b><br/>
 * \tcat Unitaire · Espaces de travail<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Parcourir la table outil -> panneau, puis celle des outils de canevas.<br/>
 * 2. Comparer l'espace de l'outil a celui du panneau qu'il met en avant.<br/>
 * \tattendu Les deux espaces coincident pour chaque entree des deux tables.
 * }
 */
TEST(EditorWorkspaceTest, LaMiseEnAvantResteDansSonEspace) {
    for (const hmi::PanelFocusEntry& entry : hmi::panelFocusCatalog()) {
        EXPECT_TRUE(hmi::workspaceMaskContains(hmi::workspacesForPanel(entry.panel),
                                               hmi::workspaceForTool(entry.tool)))
            << "un outil de niveau met en avant un panneau qui n'est pas affiche dans son espace";
    }
    for (const hmi::PixelPanelFocusEntry& entry : hmi::pixelPanelFocusCatalog()) {
        EXPECT_TRUE(hmi::workspaceMaskContains(hmi::workspacesForPanel(entry.panel),
                                               hmi::workspaceForPixelTool(entry.tool)))
            << "un outil de canevas met en avant un panneau qui n'est pas affiche dans son espace";
    }
}
