// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_diagnostics_hud.cpp
 * @brief Tests unitaires de la moyenne glissante de cadence et du choix de contenu du compteur de
 *        diagnostic (LOT-62 TACHE-02, EX-NFR-001, EX-NFR-005). Logique pure, sans GPU.
 */

#include <filesystem>

#include <gtest/gtest.h>

#include "HMI/Game/DiagnosticsHud.h"
#include "HMI/Localization/Localization.h"

namespace {

hmi::Localization testLocalization() {
    hmi::Localization localization;
    localization.setDefaultCatalog("fr",
                                   {{"diag.fps", "Images/s : %1"},
                                    {"diag.primitives", "Primitives : %1 composees / %2 soumises"},
                                    {"diag.batches", "Passes : %1"},
                                    {"diag.steps", "Pas de simulation : %1"}});
    return localization;
}

}  // namespace

/**
 * @brief Sans aucun échantillon, la moyenne glissante vaut zéro : pas de division par zéro.
 * \castest{<b>Sans echantillon, la cadence moyenne vaut zero.</b><br/>
 * \tcat Unitaire · Compteur de diagnostic<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire une moyenne glissante neuve.<br/>2. Lire la cadence.<br/>
 * \tattendu La cadence vaut 0, sans plantage.
 * }
 */
TEST(FrameRateAverageTest, SansEchantillonLaCadenceVautZero) {
    const hmi::FrameRateAverage average;
    EXPECT_FLOAT_EQ(average.framesPerSecond(), 0.0f);
}

/**
 * @brief Une cadence stable (durée d'image constante) converge vers l'inverse de cette durée.
 * \castest{<b>Une cadence stable converge vers l'inverse de la duree d'image.</b><br/>
 * \tcat Unitaire · Compteur de diagnostic<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Ajouter de nombreux echantillons de 1/60 seconde.<br/>2. Lire la cadence
 * moyenne.<br/>
 * \tattendu La cadence moyenne vaut environ 60 images par seconde.
 * }
 */
TEST(FrameRateAverageTest, CadenceStableConvergeVersLInverseDeLaDureeDImage) {
    hmi::FrameRateAverage average;
    constexpr float FRAME_DURATION = 1.0f / 60.0f;
    for (int i = 0; i < 120; ++i) {
        average.addSample(FRAME_DURATION);
    }
    EXPECT_NEAR(average.framesPerSecond(), 60.0f, 0.5f);
}

/**
 * @brief La moyenne glissante est exacte sur une courte suite de durées connues.
 * \castest{<b>La moyenne glissante est exacte sur une suite de durees connues.</b><br/>
 * \tcat Unitaire · Compteur de diagnostic<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Ajouter trois echantillons de 0,1 seconde (0,3s < fenetre).<br/>2. Lire la
 * cadence.<br/>
 * \tattendu La cadence vaut exactement 3 images / 0,3 seconde = 10 images par seconde.
 * }
 */
TEST(FrameRateAverageTest, MoyenneExacteSurUneSuiteDeDureesConnues) {
    hmi::FrameRateAverage average;
    average.addSample(0.1f);
    average.addSample(0.1f);
    average.addSample(0.1f);
    EXPECT_NEAR(average.framesPerSecond(), 10.0f, 1e-3f);
}

/**
 * @brief Une rafale d'images lentes sortie de la fenêtre n'influence plus la cadence une fois que
 *        des images rapides et nombreuses l'ont remplacée.
 * \castest{<b>Une rafale ancienne sort de la fenetre et n'influence plus la moyenne.</b><br/>
 * \tcat Unitaire · Compteur de diagnostic<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Ajouter une image tres lente (1 seconde).<br/>2. Ajouter ensuite de nombreuses
 * images rapides (1/120 seconde) jusqu'a largement depasser la fenetre.<br/>
 * \tattendu La cadence finale reflete les images rapides recentes, pas la rafale lente initiale.
 * }
 */
TEST(FrameRateAverageTest, RafaleAncienneSortDeLaFenetre) {
    hmi::FrameRateAverage average;
    average.addSample(1.0f);  // image tres lente, loin dans le passe une fois la fenetre remplie.
    constexpr float FRAME_DURATION = 1.0f / 120.0f;
    for (int i = 0; i < 240; ++i) {
        average.addSample(FRAME_DURATION);
    }
    EXPECT_NEAR(average.framesPerSecond(), 120.0f, 1.0f);
}

/**
 * @brief `reset()` oublie tous les échantillons accumulés : la cadence retombe à zéro.
 * \castest{<b>reset oublie tous les echantillons accumules.</b><br/>
 * \tcat Unitaire · Compteur de diagnostic<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Ajouter des echantillons.<br/>2. Appeler reset.<br/>3. Lire la cadence.<br/>
 * \tattendu La cadence vaut de nouveau 0.
 * }
 */
TEST(FrameRateAverageTest, ResetOublieTousLesEchantillons) {
    hmi::FrameRateAverage average;
    average.addSample(1.0f / 60.0f);
    average.addSample(1.0f / 60.0f);
    ASSERT_GT(average.framesPerSecond(), 0.0f);

    average.reset();

    EXPECT_FLOAT_EQ(average.framesPerSecond(), 0.0f);
}

/**
 * @brief Une durée nulle ou négative est ignorée (robustesse) : elle ne fait pas dériver la
 *        cadence.
 * \castest{<b>Une duree nulle ou negative est ignoree.</b><br/>
 * \tcat Unitaire · Compteur de diagnostic<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Ajouter un echantillon negatif puis nul sur une moyenne neuve.<br/>
 * \tattendu La cadence reste a 0 (aucun echantillon valide accepte).
 * }
 */
TEST(FrameRateAverageTest, DureeNulleOuNegativeEstIgnoree) {
    hmi::FrameRateAverage average;
    average.addSample(-0.5f);
    average.addSample(0.0f);
    EXPECT_FLOAT_EQ(average.framesPerSecond(), 0.0f);
}

/**
 * @brief Des valeurs mesurées produisent les quatre lignes attendues, dans l'ordre documenté.
 * \castest{<b>Des valeurs mesurees produisent les quatre lignes attendues.</b><br/>
 * \tcat Unitaire · Compteur de diagnostic<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer les lignes a partir de mesures connues.<br/>
 * \tattendu Cadence, primitives, passes et pas de simulation apparaissent, dans cet ordre, avec
 * les valeurs attendues.
 * }
 */
TEST(DiagnosticsHudTest, MesuresConnuesProduisentLesQuatreLignesAttendues) {
    hmi::DiagnosticsMeasurements measurements;
    measurements.framesPerSecond = 59.6f;
    measurements.sceneStatistics.considered = 128;
    measurements.sceneStatistics.culled = 32;
    measurements.sceneStatistics.submitted = 96;
    measurements.sceneStatistics.batches = 5;
    measurements.simulationSteps = 1;

    const std::vector<std::string> lines =
        hmi::composeDiagnosticsHudLines(measurements, testLocalization());

    ASSERT_EQ(lines.size(), 4u);
    EXPECT_EQ(lines[0], "Images/s : 59.6");
    EXPECT_EQ(lines[1], "Primitives : 128 composees / 96 soumises");
    EXPECT_EQ(lines[2], "Passes : 5");
    EXPECT_EQ(lines[3], "Pas de simulation : 1");
}

/**
 * @brief Le cas limite « aucune image mesurée » (démarrage, avant la première image) produit des
 *        lignes valides, sans plantage.
 * \castest{<b>Aucune image mesuree produit des lignes valides.</b><br/>
 * \tcat Unitaire · Compteur de diagnostic<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Composer les lignes a partir de mesures toutes nulles (etat de depart).<br/>
 * \tattendu Les quatre lignes sont produites, avec des valeurs nulles bien formees.
 * }
 */
TEST(DiagnosticsHudTest, AucuneImageMesureeProduitDesLignesValides) {
    const hmi::DiagnosticsMeasurements measurements;  // tout par defaut : etat de depart.

    const std::vector<std::string> lines =
        hmi::composeDiagnosticsHudLines(measurements, testLocalization());

    ASSERT_EQ(lines.size(), 4u);
    EXPECT_EQ(lines[0], "Images/s : 0.0");
    EXPECT_EQ(lines[1], "Primitives : 0 composees / 0 soumises");
    EXPECT_EQ(lines[2], "Passes : 0");
    EXPECT_EQ(lines[3], "Pas de simulation : 0");
}

/**
 * @brief Chaque clé de traduction utilisée par le compteur de diagnostic existe, traduite, dans
 *        les deux catalogues livrés (français et anglais).
 * \castest{<b>Les cles de traduction du compteur existent dans les deux catalogues livres.</b><br/>
 * \tcat Unitaire · Compteur de diagnostic<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger fr.lang puis en.lang depuis les catalogues livres.<br/>2. Resoudre les cles
 * du compteur de diagnostic.<br/>
 * \tattendu Les quatre cles resolvent vers un texte traduit, distinct de la cle elle-meme.
 * }
 */
TEST(DiagnosticsHudTest, ClesDeTraductionExistentDansLesDeuxCatalogues) {
    const std::filesystem::path directory(JADG_LOCALIZATION_DIR);
    for (const std::string& language : {"fr", "en"}) {
        hmi::Localization localization(directory);
        ASSERT_TRUE(localization.loadDefaultLanguage(language)) << language;
        for (const char* key : {"diag.fps", "diag.primitives", "diag.batches", "diag.steps"}) {
            EXPECT_NE(localization.text(key), key) << language << " / " << key;
        }
    }
}
