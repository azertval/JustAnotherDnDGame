// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_editor_status.cpp
 * @brief Tests unitaires du choix de contenu de la barre d'état de l'éditeur (LOT-57 TACHE-01,
 *        EX-IHM-060). Fonction pure, sans Qt/GPU.
 */

#include <filesystem>

#include <gtest/gtest.h>

#include "HMI/Editor/EditorStatus.h"
#include "HMI/Localization/Localization.h"

namespace {

hmi::Localization testLocalization() {
    hmi::Localization localization;
    localization.setDefaultCatalog("fr",
                                   {{"status.zone.level", "Niveau : %1"},
                                    {"status.zone.asset", "Asset : %1"},
                                    {"status.zone.dirty", "Modifie"},
                                    {"status.zone.hover", "(%1, %2)"},
                                    {"status.zone.zoom", "Zoom : %1%"},
                                    {"status.zone.color", "Couleur : %1"},
                                    {"status.zone.color_constrained", "Couleur : %1 (contrainte)"},
                                    {"status.zone.camera_framing", "Cadrage : %1"},
                                    {"camera_framing.whole_level", "Niveau entier"},
                                    {"camera_framing.per_room", "Par salle"},
                                    {"camera_framing.follow", "Suivi du personnage"},
                                    {"tool.brush", "Pinceau"},
                                    {"tool.rectangle", "Rectangle"},
                                    {"tool.selection", "Selection"},
                                    {"tool.link", "Lien"},
                                    {"tool.texture_assign", "Texture"},
                                    {"tool.camera_zone", "Zone de camera"},
                                    {"status.help_paint", "Aide pinceau"},
                                    {"status.help_rectangle", "Aide rectangle"},
                                    {"status.help_selection", "Aide selection"},
                                    {"status.help_link", "Aide lien"},
                                    {"status.help_texture_assign", "Aide texture"},
                                    {"status.help_camera_zone", "Aide zone de camera"},
                                    {"pixel_tool.brush", "Pinceau"},
                                    {"pixel_tool.eraser", "Gomme"},
                                    {"pixel_tool.fill", "Pot de peinture"},
                                    {"pixel_tool.eyedropper", "Pipette"},
                                    {"pixel_tool.selection", "Selection"},
                                    {"status.help_pixel_brush", "Aide pinceau pixel"},
                                    {"status.help_pixel_eraser", "Aide gomme"},
                                    {"status.help_pixel_fill", "Aide pot de peinture"},
                                    {"status.help_pixel_eyedropper", "Aide pipette"},
                                    {"status.help_pixel_selection", "Aide selection"}});
    return localization;
}

hmi::LevelStatusInfo baseLevel() {
    hmi::LevelStatusInfo level;
    level.name = "Salle des epreuves";
    level.dirty = false;
    level.tool = hmi::EditorTool::Paint;
    level.hoveredCell = std::nullopt;
    level.zoom = 1.0f;
    return level;
}

hmi::PixelEditStatusInfo basePixelEdit() {
    hmi::PixelEditStatusInfo pixel;
    pixel.assetName = "mur.png";
    pixel.dirty = false;
    pixel.tool = hmi::PixelTool::Brush;
    pixel.hoveredPixel = std::nullopt;
    pixel.zoom = 8;
    pixel.currentColor = 0xFF0000FFu;  // rouge opaque (R8G8B8A8_UNORM).
    return pixel;
}

}  // namespace

/**
 * @brief Un contexte sans niveau ouvert n'affiche aucune zone ni aide.
 * \castest{<b>Aucun niveau ouvert n'affiche aucune zone ni aide.</b><br/>
 * \tcat Unitaire · Barre d'etat de l'editeur<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire un contexte sans niveau.<br/>2. Calculer les lignes.<br/>
 * \tattendu Les sept zones permanentes et l'aide sont vides.
 * }
 */
TEST(EditorStatusTest, AucunNiveauOuvertNAfficheRien) {
    const hmi::EditorStatusLines lines =
        hmi::editorStatusLines(hmi::EditorStatusContext{}, testLocalization());

    // Sept zones depuis LOT-64 (ajout du cadrage de camera, contexte niveau seulement).
    ASSERT_EQ(lines.permanent.size(), 7u);
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

    const hmi::EditorStatusLines lines = hmi::editorStatusLines(context, testLocalization());

    EXPECT_EQ(lines.permanent[0], "Niveau : Salle des epreuves");
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

    const hmi::Localization localization = testLocalization();
    EXPECT_EQ(hmi::editorStatusLines(dirtyContext, localization).permanent[1], "Modifie");
    EXPECT_EQ(hmi::editorStatusLines(cleanContext, localization).permanent[1], "");
}

/**
 * @brief La zone de cadrage affiche le mode courant, traduit -- présente en permanence
 * (`EX-EDIT-028`), jamais seulement au survol ou sur demande.
 * \castest{<b>La zone de cadrage affiche le mode courant, traduit.</b><br/>
 * \tcat Unitaire · Barre d'etat de l'editeur<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Calculer les lignes pour un niveau en mode *par salle*, puis *suivi*.<br/>2.
 * Comparer la zone de cadrage (index 6).<br/>
 * \tattendu La zone reflète le mode courant, traduit, pour chacun des deux modes.
 * }
 */
TEST(EditorStatusTest, ZoneDeCadrageAfficheLeModeCourant) {
    const hmi::Localization localization = testLocalization();

    hmi::LevelStatusInfo perRoom = baseLevel();
    perRoom.cameraFraming = core::CameraFramingMode::PerRoom;
    hmi::EditorStatusContext perRoomContext;
    perRoomContext.level = perRoom;
    EXPECT_EQ(hmi::editorStatusLines(perRoomContext, localization).permanent[6],
              "Cadrage : Par salle");

    hmi::LevelStatusInfo follow = baseLevel();
    follow.cameraFraming = core::CameraFramingMode::Follow;
    hmi::EditorStatusContext followContext;
    followContext.level = follow;
    EXPECT_EQ(hmi::editorStatusLines(followContext, localization).permanent[6],
              "Cadrage : Suivi du personnage");
}

/**
 * @brief L'aide affichee change avec l'outil actif.
 * \castest{<b>L'aide contextuelle change avec l'outil actif.</b><br/>
 * \tcat Unitaire · Barre d'etat de l'editeur<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Calculer les lignes avec l'outil Pinceau, puis Lien.<br/>2. Comparer l'aide.<br/>
 * \tattendu L'aide differe entre les deux outils et correspond a la cle attendue.
 * }
 */
TEST(EditorStatusTest, AideChangeAvecLOutilActif) {
    const hmi::Localization localization = testLocalization();

    hmi::EditorStatusContext paintContext;
    paintContext.level = baseLevel();
    EXPECT_EQ(hmi::editorStatusLines(paintContext, localization).help, "Aide pinceau");

    hmi::LevelStatusInfo linkLevel = baseLevel();
    linkLevel.tool = hmi::EditorTool::Link;
    hmi::EditorStatusContext linkContext;
    linkContext.level = linkLevel;
    EXPECT_EQ(hmi::editorStatusLines(linkContext, localization).help, "Aide lien");
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
    const hmi::Localization localization = testLocalization();

    const hmi::EditorStatusLines first = hmi::editorStatusLines(context, localization);
    const hmi::EditorStatusLines second = hmi::editorStatusLines(context, localization);

    EXPECT_EQ(first.help, second.help);
    EXPECT_EQ(first.permanent, second.permanent);
}

/**
 * @brief Le contexte d'atelier pixel art produit les six zones attendues (asset, modifie, outil,
 *        pixel survole, zoom, couleur) et l'aide de l'outil de canevas actif -- sans toucher au
 *        contexte de niveau (mutuellement exclusifs).
 * \castest{<b>Le contexte d'atelier pixel art produit les zones et l'aide attendues.</b><br/>
 * \tcat Unitaire · Barre d'etat de l'editeur<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire un contexte d'edition d'asset avec un pixel survole.<br/>2. Calculer les
 * lignes.<br/>
 * \tattendu Les six zones portent les valeurs attendues et l'aide correspond a l'outil actif.
 * }
 */
TEST(EditorStatusTest, ContextePixelEditProduitLesZonesEtLAideAttendues) {
    hmi::PixelEditStatusInfo pixel = basePixelEdit();
    pixel.hoveredPixel = std::make_pair(3, 5);
    hmi::EditorStatusContext context;
    context.pixelEdit = pixel;

    const hmi::EditorStatusLines lines = hmi::editorStatusLines(context, testLocalization());

    EXPECT_EQ(lines.permanent[0], "Asset : mur.png");
    EXPECT_EQ(lines.permanent[1], "");  // dirty = false.
    EXPECT_EQ(lines.permanent[2], "Pinceau");
    EXPECT_EQ(lines.permanent[3], "(3, 5)");
    EXPECT_EQ(lines.permanent[4], "Zoom : 800%");
    EXPECT_EQ(lines.permanent[5], "Couleur : #ff0000ff");
    EXPECT_EQ(lines.help, "Aide pinceau pixel");
}

/**
 * @brief Le mode contraint figure dans la zone couleur de la barre d'état, distinct du mode libre.
 * \castest{<b>Le mode contraint figure dans la zone couleur.</b><br/>
 * \tcat Unitaire · Barre d'etat de l'editeur<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Calculer les lignes avec le mode contraint actif, puis inactif.<br/>2. Comparer la
 * zone couleur (index 5).<br/>
 * \tattendu La zone differe entre les deux modes et correspond a la cle attendue.
 * }
 */
TEST(EditorStatusTest, ModeContraintFigureDansLaZoneCouleur) {
    const hmi::Localization localization = testLocalization();

    hmi::PixelEditStatusInfo constrained = basePixelEdit();
    constrained.paletteConstrained = true;
    hmi::EditorStatusContext constrainedContext;
    constrainedContext.pixelEdit = constrained;
    EXPECT_EQ(hmi::editorStatusLines(constrainedContext, localization).permanent[5],
              "Couleur : #ff0000ff (contrainte)");

    hmi::EditorStatusContext freeContext;
    freeContext.pixelEdit = basePixelEdit();  // paletteConstrained = false par defaut.
    EXPECT_EQ(hmi::editorStatusLines(freeContext, localization).permanent[5],
              "Couleur : #ff0000ff");
}

/**
 * @brief Aucun asset ouvert laisse la zone d'asset vide, sans libellé de remplacement, exactement
 *        comme l'absence de niveau ouvert.
 * \castest{<b>Aucun asset ouvert laisse la zone d'asset vide.</b><br/>
 * \tcat Unitaire · Barre d'etat de l'editeur<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un contexte d'atelier sans nom d'asset.<br/>2. Calculer les lignes.<br/>
 * \tattendu La zone d'asset (index 0) est vide ; les autres zones restent renseignees.
 * }
 */
TEST(EditorStatusTest, AucunAssetOuvertLaisseLaZoneVide) {
    hmi::PixelEditStatusInfo pixel = basePixelEdit();
    pixel.assetName.clear();
    hmi::EditorStatusContext context;
    context.pixelEdit = pixel;

    const hmi::EditorStatusLines lines = hmi::editorStatusLines(context, testLocalization());

    EXPECT_EQ(lines.permanent[0], "");
    EXPECT_EQ(lines.permanent[2], "Pinceau");
}

/**
 * @brief L'aide contextuelle de l'atelier change avec l'outil de canevas actif.
 * \castest{<b>L'aide de l'atelier change avec l'outil de canevas actif.</b><br/>
 * \tcat Unitaire · Barre d'etat de l'editeur<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Calculer les lignes avec l'outil Gomme, puis Pipette.<br/>2. Comparer l'aide.<br/>
 * \tattendu L'aide differe entre les deux outils et correspond a la cle attendue.
 * }
 */
TEST(EditorStatusTest, AideDeLAtelierChangeAvecLOutilActif) {
    const hmi::Localization localization = testLocalization();

    hmi::PixelEditStatusInfo eraser = basePixelEdit();
    eraser.tool = hmi::PixelTool::Eraser;
    hmi::EditorStatusContext eraserContext;
    eraserContext.pixelEdit = eraser;
    EXPECT_EQ(hmi::editorStatusLines(eraserContext, localization).help, "Aide gomme");

    hmi::PixelEditStatusInfo eyedropper = basePixelEdit();
    eyedropper.tool = hmi::PixelTool::Eyedropper;
    hmi::EditorStatusContext eyedropperContext;
    eyedropperContext.pixelEdit = eyedropper;
    EXPECT_EQ(hmi::editorStatusLines(eyedropperContext, localization).help, "Aide pipette");
}

/**
 * @brief Chaque cle de traduction utilisee par la barre d'etat existe, traduite, dans les deux
 *        catalogues livres (francais et anglais).
 * \castest{<b>Les cles de traduction de la barre d'etat existent dans les deux catalogues.</b><br/>
 * \tcat Unitaire · Barre d'etat de l'editeur<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger fr.lang puis en.lang depuis les catalogues livres.<br/>2. Resoudre chaque
 * cle utilisee.<br/>
 * \tattendu Chaque cle resout vers un texte traduit, distinct de la cle elle-meme.
 * }
 */
TEST(EditorStatusTest, ClesDeTraductionExistentDansLesDeuxCatalogues) {
    const std::filesystem::path directory(JADG_LOCALIZATION_DIR);
    const char* const keys[] = {"status.zone.level",
                                "status.zone.asset",
                                "status.zone.dirty",
                                "status.zone.hover",
                                "status.zone.zoom",
                                "status.zone.color",
                                "status.zone.color_constrained",
                                "status.zone.camera_framing",
                                "camera_framing.whole_level",
                                "camera_framing.per_room",
                                "camera_framing.follow",
                                "status.help_paint",
                                "status.help_rectangle",
                                "status.help_selection",
                                "status.help_link",
                                "status.help_texture_assign",
                                "status.help_pixel_brush",
                                "status.help_pixel_eraser",
                                "status.help_pixel_fill",
                                "status.help_pixel_eyedropper",
                                "status.help_pixel_selection",
                                "tool.camera_zone",
                                "status.help_camera_zone"};
    for (const std::string& language : {"fr", "en"}) {
        hmi::Localization localization(directory);
        ASSERT_TRUE(localization.loadDefaultLanguage(language)) << language;
        for (const char* const key : keys) {
            EXPECT_NE(localization.text(key), key) << language << " / " << key;
        }
    }
}
