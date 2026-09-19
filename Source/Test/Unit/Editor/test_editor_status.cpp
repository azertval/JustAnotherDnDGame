// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_editor_status.cpp
 * @brief Tests unitaires du choix de contenu de la barre d'état de l'éditeur
 *        Fonction pure, sans Qt/GPU.
 */

#include <gtest/gtest.h>

#include "Editor/Logic/EditorStatus.h"

namespace {

hmi::LevelStatusInfo baseLevel() {
    hmi::LevelStatusInfo level;
    level.name = "Salle des epreuves";
    level.dirty = false;
    level.tool = hmi::EditorTool::Paint;
    level.hoveredCell = std::nullopt;
    level.zoom = 1.0f;
    return level;
}

}  // namespace

/**
 * @brief Un contexte sans niveau ouvert n'affiche aucune zone ni aide.
 * \castest{<b>Aucun niveau ouvert n'affiche aucune zone ni aide.</b><br/>
 * \tcat Unitaire · Barre d'etat de l'editeur<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire un contexte sans niveau.<br/>2. Calculer les lignes.<br/>
 * \tattendu Les cinq zones permanentes et l'aide sont vides.
 * }
 */
TEST(EditorStatusTest, AucunNiveauOuvertNAfficheRien) {
    const hmi::EditorStatusLines lines = hmi::editorStatusLines(hmi::EditorStatusContext{});

    ASSERT_EQ(lines.permanent.size(), hmi::EDITOR_STATUS_ZONE_COUNT);
    for (const std::string& zone : lines.permanent) {
        EXPECT_EQ(zone, "");
    }
    EXPECT_EQ(lines.help, "");
}

/**
 * @brief Aucune case survolee n'affiche une zone de coordonnees vide, sans casser les autres zones.
 * \castest{<b>Aucune case survolee laisse la zone de coordonnees vide.</b><br/>
 * \tcat Unitaire · Barre d'etat de l'editeur<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un niveau sans case survolee.<br/>2. Calculer les lignes.<br/>
 * \tattendu La zone de coordonnees (index 3) est vide ; les autres zones sont renseignees.
 * }
 */
TEST(EditorStatusTest, AucuneCaseSurvoleeLaisseLaZoneVide) {
    hmi::EditorStatusContext context;
    context.level = baseLevel();

    const hmi::EditorStatusLines lines = hmi::editorStatusLines(context);

    EXPECT_EQ(lines.permanent[0], "Map: Salle des epreuves");
    EXPECT_EQ(lines.permanent[3], "");
}

/**
 * @brief L'indicateur de modification est present apres une edition, absent apres enregistrement.
 * \castest{<b>L'indicateur de modification suit l'etat "dirty" du niveau.</b><br/>
 * \tcat Unitaire · Barre d'etat de l'editeur<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Calculer les lignes avec dirty=true, puis dirty=false.<br/>2. Comparer la zone
 * d'indicateur.<br/>
 * \tattendu La zone est non vide avec dirty=true, vide avec dirty=false.
 * }
 */
TEST(EditorStatusTest, IndicateurDeModificationSuitLEtatDirty) {
    hmi::LevelStatusInfo dirtyLevel = baseLevel();
    dirtyLevel.dirty = true;
    hmi::EditorStatusContext dirtyContext;
    dirtyContext.level = dirtyLevel;

    hmi::EditorStatusContext cleanContext;
    cleanContext.level = baseLevel();  // dirty = false par defaut.

    EXPECT_EQ(hmi::editorStatusLines(dirtyContext).permanent[1], "Modified");
    EXPECT_EQ(hmi::editorStatusLines(cleanContext).permanent[1], "");
}

/**
 * @brief L'aide affichee change avec l'outil actif.
 * \castest{<b>L'aide contextuelle change avec l'outil actif.</b><br/>
 * \tcat Unitaire · Barre d'etat de l'editeur<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Calculer les lignes avec l'outil Pinceau, puis Entite.<br/>2. Comparer l'aide.<br/>
 * \tattendu L'aide differe entre les deux outils et commence par le nom de l'outil.
 * }
 */
TEST(EditorStatusTest, AideChangeAvecLOutilActif) {
    hmi::EditorStatusContext paintContext;
    paintContext.level = baseLevel();
    EXPECT_EQ(hmi::editorStatusLines(paintContext).help.rfind("Paint:", 0), 0U);

    hmi::LevelStatusInfo entityLevel = baseLevel();
    entityLevel.tool = hmi::EditorTool::Entity;
    hmi::EditorStatusContext entityContext;
    entityContext.level = entityLevel;
    EXPECT_EQ(hmi::editorStatusLines(entityContext).help.rfind("Entity:", 0), 0U);
}

/**
 * @brief La restauration apres un message transitoire redonne la meme aide, pour le meme outil.
 *
 * `editorStatusLines` est PURE : restaurer la barre d'etat consiste donc simplement a la rappeler
 * avec le meme contexte, sans avoir a memoriser ce qui y etait affiche.
 * \castest{<b>La decision est deterministe : meme contexte, meme resultat.</b><br/>
 * \tcat Unitaire · Barre d'etat de l'editeur<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Calculer deux fois les lignes pour le meme contexte.<br/>2. Comparer.<br/>
 * \tattendu Les deux resultats sont identiques.
 * }
 */
TEST(EditorStatusTest, MemeContexteProduitLaMemeAide) {
    hmi::EditorStatusContext context;
    context.level = baseLevel();

    const hmi::EditorStatusLines first = hmi::editorStatusLines(context);
    const hmi::EditorStatusLines second = hmi::editorStatusLines(context);

    EXPECT_EQ(first.help, second.help);
    EXPECT_EQ(first.permanent, second.permanent);
}

/**
 * @brief La case survolée montre ses pièces, la zone du zoom dit la vue du canevas.
 * \castest{<b>La barre d'etat nomme les pieces survolees et la vue du canevas.</b><br/>
 * 	cat Unitaire · Barre d'etat de l'editeur<br/>
 * 	crit Mineur<br/>
 * 	etapes 1. Survoler une case qui porte un sol et un relief, en vue iso.<br/>
 *          2. Passer en vue a plat.<br/>
 * 	attendu Les coordonnees sont suivies des pieces ; la zone du zoom finit par « Iso », puis par
 *           « Flat » (LOT-EDITOR-02).
 * }
 */
TEST(EditorStatusTest, LesPiecesEtLaVueSeLisent) {
    hmi::EditorStatusContext context;
    hmi::LevelStatusInfo level = baseLevel();
    level.hoveredCell = core::GridPosition{.column = 12, .row = 7};
    level.hoveredPieces = "street · wall-left";
    context.level = level;
    hmi::EditorStatusLines lines = hmi::editorStatusLines(context);
    EXPECT_EQ(lines.permanent[3], "(12, 7) street · wall-left");
    EXPECT_EQ(lines.permanent[4], "Zoom: 100% · Iso");

    level.isoView = false;
    context.level = level;
    lines = hmi::editorStatusLines(context);
    EXPECT_EQ(lines.permanent[4], "Zoom: 100% · Flat");
}

/**
 * @brief La barre d'état nomme le pinceau armé, signale une case forcée et rappelle, la collision
 *        active, ce que pinceau et gomme y font (`LOT-EDITOR-03`).
 * \castest{<b>La barre d'etat dit le pinceau et les cases forcees.</b><br/>
 * 	cat Unitaire · Barre d'etat de l'editeur<br/>
 * 	crit Mineur<br/>
 * 	etapes 1. Armer la piece `wall-left`, survoler une case forcee, la collision active.<br/>
 *          2. Passer a l'outil Entite.<br/>
 * 	attendu « Brush · wall-left », « · forced collision », et l'aide de la collision ; l'outil
 *           Entite ne nomme plus de pinceau.
 * }
 */
TEST(EditorStatusTest, LePinceauEtLesCasesForceesSeLisent) {
    hmi::EditorStatusContext context;
    hmi::LevelStatusInfo level = baseLevel();
    level.brush = "wall-left";
    level.hoveredCell = core::GridPosition{.column = 3, .row = 4};
    level.hoveredForced = true;
    level.collisionActive = true;
    context.level = level;
    hmi::EditorStatusLines lines = hmi::editorStatusLines(context);
    EXPECT_EQ(lines.permanent[2], "Brush · wall-left");
    EXPECT_EQ(lines.permanent[3], "(3, 4) · forced collision");
    EXPECT_NE(lines.help.find("the eraser releases it"), std::string::npos);

    level.tool = hmi::EditorTool::Entity;
    context.level = level;
    lines = hmi::editorStatusLines(context);
    EXPECT_EQ(lines.permanent[2], "Entity");
    EXPECT_EQ(lines.help.find("the eraser releases it"), std::string::npos);
}

/**
 * @brief La barre d'état dit le miroir, la mesure en cours et la note de la case survolée ; la
 *        ligne et le seau nomment le pinceau comme le pinceau (`LOT-EDITOR-04`).
 * \castest{<b>La barre d'etat dit le miroir, la mesure et la note.</b><br/>
 * \tcat Unitaire · Barre d'etat de l'editeur<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Outil Ligne, pinceau `wall-left`, miroir actif, une note sous le curseur.<br/>
 *          2. Outil Mesure, une mesure en cours.<br/>
 * \tattendu « Line · wall-left · Mirror », « (3, 4) · Note: well » ; puis « Measure · Mirror ·
 *           7 × 4 · 6 cells = 30 ft » et l'aide des pieds.
 * }
 */
TEST(EditorStatusTest, LeMiroirLaMesureEtLaNoteSeLisent) {
    hmi::EditorStatusContext context;
    hmi::LevelStatusInfo level = baseLevel();
    level.tool = hmi::EditorTool::Line;
    level.brush = "wall-left";
    level.mirror = true;
    level.hoveredCell = core::GridPosition{.column = 3, .row = 4};
    level.hoveredNote = "well";
    context.level = level;
    hmi::EditorStatusLines lines = hmi::editorStatusLines(context);
    EXPECT_EQ(lines.permanent[2], "Line · wall-left · Mirror");
    EXPECT_EQ(lines.permanent[3], "(3, 4) · Note: well");

    level.tool = hmi::EditorTool::Measure;
    level.measure = "7 × 4 · 6 cells = 30 ft";
    context.level = level;
    lines = hmi::editorStatusLines(context);
    EXPECT_EQ(lines.permanent[2], "Measure · Mirror · 7 × 4 · 6 cells = 30 ft");
    EXPECT_NE(lines.help.find("1 cell = 5 ft"), std::string::npos);
}
