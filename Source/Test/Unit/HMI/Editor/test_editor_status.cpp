// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_editor_status.cpp
 * @brief Tests unitaires du choix de contenu de la barre d'état de l'éditeur
 *        (EX-IHM-060). Fonction pure, sans Qt/GPU.
 */

#include <filesystem>

#include <gtest/gtest.h>

#include "HMI/Editor/EditorStatus.h"
#include "HMI/Localization/Localization.h"

namespace {

hmi::Localization testLocalization() {
    hmi::Localization localization;
    localization.setDefaultCatalog("fr", {{"status.zone.level", "Carte : %1"},
                                          {"status.zone.dirty", "Modifie"},
                                          {"status.zone.hover", "(%1, %2)"},
                                          {"status.zone.zoom", "Zoom : %1%"},
                                          {"tool.brush", "Pinceau"},
                                          {"tool.rectangle", "Rectangle"},
                                          {"tool.selection", "Selection"},
                                          {"tool.entity", "Entite"},
                                          {"status.help_paint", "Aide pinceau"},
                                          {"status.help_rectangle", "Aide rectangle"},
                                          {"status.help_selection", "Aide selection"},
                                          {"status.help_entity", "Aide entite"}});
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
    const hmi::EditorStatusLines lines =
        hmi::editorStatusLines(hmi::EditorStatusContext{}, testLocalization());

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

    const hmi::EditorStatusLines lines = hmi::editorStatusLines(context, testLocalization());

    EXPECT_EQ(lines.permanent[0], "Carte : Salle des epreuves");
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
 * @brief L'aide affichee change avec l'outil actif.
 * \castest{<b>L'aide contextuelle change avec l'outil actif.</b><br/>
 * \tcat Unitaire · Barre d'etat de l'editeur<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Calculer les lignes avec l'outil Pinceau, puis Entite.<br/>2. Comparer l'aide.<br/>
 * \tattendu L'aide differe entre les deux outils et correspond a la cle attendue.
 * }
 */
TEST(EditorStatusTest, AideChangeAvecLOutilActif) {
    const hmi::Localization localization = testLocalization();

    hmi::EditorStatusContext paintContext;
    paintContext.level = baseLevel();
    EXPECT_EQ(hmi::editorStatusLines(paintContext, localization).help, "Aide pinceau");

    hmi::LevelStatusInfo entityLevel = baseLevel();
    entityLevel.tool = hmi::EditorTool::Entity;
    hmi::EditorStatusContext entityContext;
    entityContext.level = entityLevel;
    EXPECT_EQ(hmi::editorStatusLines(entityContext, localization).help, "Aide entite");
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
    const char* const keys[] = {"status.zone.level",     "status.zone.dirty",
                                "status.zone.hover",     "status.zone.zoom",
                                "status.help_paint",     "status.help_rectangle",
                                "status.help_selection", "status.help_entity"};
    for (const std::string& language : {"fr", "en"}) {
        hmi::Localization localization(directory);
        ASSERT_TRUE(localization.loadDefaultLanguage(language)) << language;
        for (const char* const key : keys) {
            EXPECT_NE(localization.text(key), key) << language << " / " << key;
        }
    }
}
