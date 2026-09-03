// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_pixel_history.cpp
 * @brief Tests unitaires de l'historique nommé annuler/refaire du canevas pixel art
 *        (LOT-54 TACHE-02, `EX-EDIT-045`).
 */

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "HMI/Editor/PixelHistory.h"
#include "HMI/Editor/PixelOperations.h"
#include "HMI/Localization/Localization.h"

namespace {

hmi::DecodedImage uniformImage(int width, int height, std::uint32_t color) {
    hmi::DecodedImage image;
    image.width = width;
    image.height = height;
    image.pixels.assign(static_cast<std::size_t>(width) * static_cast<std::size_t>(height), color);
    return image;
}

constexpr std::uint32_t RED = 0xFF0000FFu;
constexpr std::uint32_t GREEN = 0xFF00FF00u;
constexpr std::uint32_t BLUE = 0xFFFF0000u;

// Applique une operation ponctuelle (setPixel) au (x, y) donne et l'enregistre dans l'historique
// sous le nom "kind" -- reproduit ce que fera le canevas (TACHE-03) pour un seul pixel.
void applyAndRecord(hmi::DecodedImage& image, hmi::PixelHistory& history,
                    hmi::PixelOperationKind kind, int x, int y, std::uint32_t color) {
    const std::vector<std::uint32_t> before = hmi::readRegion(image, hmi::PixelRegion{x, y, x, y});
    const hmi::PixelRegion region = hmi::setPixel(image, x, y, color);
    const std::vector<std::uint32_t> after = hmi::readRegion(image, region);
    history.push(kind, region, before, after);
}

}  // namespace

/**
 * @brief Une suite d'opérations peut être annulée puis rétablie, restituant exactement les
 *        mêmes tampons à chaque étape.
 * \castest{<b>Une suite d'operations est annulable puis retablissable a l'identique.</b><br/>
 * \tcat Unitaire · Historique pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Appliquer trois operations successives.<br/>2. Annuler deux fois.<br/>
 * 3. Retablir deux fois.<br/>
 * \tattendu Chaque annulation/retablissement restitue exactement le tampon attendu a cette
 * etape.
 * }
 */
TEST(PixelHistoryTest, SuiteDOperationsAnnulableEtRetablissable) {
    hmi::DecodedImage image = uniformImage(3, 1, 0);
    hmi::PixelHistory history;

    applyAndRecord(image, history, hmi::PixelOperationKind::Brush, 0, 0, RED);
    applyAndRecord(image, history, hmi::PixelOperationKind::Brush, 1, 0, GREEN);
    applyAndRecord(image, history, hmi::PixelOperationKind::Brush, 2, 0, BLUE);
    ASSERT_EQ(hmi::pickColor(image, 0, 0), RED);
    ASSERT_EQ(hmi::pickColor(image, 1, 0), GREEN);
    ASSERT_EQ(hmi::pickColor(image, 2, 0), BLUE);

    ASSERT_TRUE(history.undo(image));
    EXPECT_EQ(hmi::pickColor(image, 2, 0), 0u);
    ASSERT_TRUE(history.undo(image));
    EXPECT_EQ(hmi::pickColor(image, 1, 0), 0u);
    EXPECT_EQ(hmi::pickColor(image, 0, 0), RED) << "la premiere operation n'est pas touchee";

    ASSERT_TRUE(history.redo(image));
    EXPECT_EQ(hmi::pickColor(image, 1, 0), GREEN);
    ASSERT_TRUE(history.redo(image));
    EXPECT_EQ(hmi::pickColor(image, 2, 0), BLUE);
}

/**
 * @brief Une nouvelle opération après une annulation vide la pile de rétablissement (la branche
 *        abandonnée ne redevient jamais valide).
 * \castest{<b>Une nouvelle operation apres une annulation vide la pile de retablissement.</b><br/>
 * \tcat Unitaire · Historique pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Appliquer deux operations.<br/>2. Annuler la derniere.<br/>3. Appliquer une
 * operation differente.<br/>
 * \tattendu Refaire n'a plus aucun effet : canRedo() est faux.
 * }
 */
TEST(PixelHistoryTest, NouvelleOperationApresAnnulationVideLeRetablissement) {
    hmi::DecodedImage image = uniformImage(2, 1, 0);
    hmi::PixelHistory history;

    applyAndRecord(image, history, hmi::PixelOperationKind::Brush, 0, 0, RED);
    applyAndRecord(image, history, hmi::PixelOperationKind::Brush, 1, 0, GREEN);
    ASSERT_TRUE(history.undo(image));
    ASSERT_TRUE(history.canRedo());

    applyAndRecord(image, history, hmi::PixelOperationKind::Eraser, 1, 0, 0u);

    EXPECT_FALSE(history.canRedo());
    EXPECT_FALSE(history.redo(image));
}

/**
 * @brief Annuler au-delà du fond de la pile reste sans effet, plutôt que de corrompre l'état ou
 *        de lever une exception.
 * \castest{<b>Annuler au-dela du fond de pile est sans effet.</b><br/>
 * \tcat Unitaire · Historique pixel<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Sur un historique vide, appeler undo().<br/>2. Appliquer une operation, l'annuler,
 * puis annuler une seconde fois.<br/>
 * \tattendu Chaque appel en trop renvoie false, sans modifier le tampon.
 * }
 */
TEST(PixelHistoryTest, AnnulerAuDelaDuFondDePileSansEffet) {
    hmi::DecodedImage image = uniformImage(1, 1, RED);
    hmi::PixelHistory history;

    EXPECT_FALSE(history.undo(image));

    applyAndRecord(image, history, hmi::PixelOperationKind::Brush, 0, 0, GREEN);
    ASSERT_TRUE(history.undo(image));
    EXPECT_FALSE(history.undo(image));
    EXPECT_EQ(hmi::pickColor(image, 0, 0), RED) << "le tampon reste a l'etat annule, non corrompu";
}

/**
 * @brief Sauter directement à l'entrée *n* donne exactement le même tampon que *n* annulations
 *        successives depuis l'état le plus récent.
 * \castest{<b>jumpTo produit le meme tampon que des annulations successives.</b><br/>
 * \tcat Unitaire · Historique pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Appliquer quatre operations sur deux tampons identiques independants.<br/>
 * 2. Sur le premier, annuler successivement jusqu'a l'entree d'index 1.<br/>3. Sur le second,
 * appeler jumpTo(1) directement.<br/>
 * \tattendu Les deux tampons sont strictement identiques.
 * }
 */
TEST(PixelHistoryTest, JumpToEquivautADesAnnulationsSuccessives) {
    hmi::DecodedImage byUndo = uniformImage(4, 1, 0);
    hmi::DecodedImage byJump = uniformImage(4, 1, 0);
    hmi::PixelHistory undoHistory;
    hmi::PixelHistory jumpHistory;

    const std::uint32_t colors[4] = {RED, GREEN, BLUE, RED};
    for (int i = 0; i < 4; ++i) {
        applyAndRecord(byUndo, undoHistory, hmi::PixelOperationKind::Brush, i, 0, colors[i]);
        applyAndRecord(byJump, jumpHistory, hmi::PixelOperationKind::Brush, i, 0, colors[i]);
    }

    // n annulations successives depuis l'etat le plus recent (4 entrees) jusqu'a l'entree
    // d'index 1 (0-based) : deux annulations (entrees 3 puis 2 defaites).
    ASSERT_TRUE(undoHistory.undo(byUndo));
    ASSERT_TRUE(undoHistory.undo(byUndo));

    ASSERT_TRUE(jumpHistory.jumpTo(byJump, 1));

    EXPECT_EQ(byUndo.pixels, byJump.pixels);
}

/**
 * @brief `jumpTo` avec un index hors bornes échoue proprement, sans modifier le tampon.
 * \castest{<b>jumpTo hors bornes echoue proprement.</b><br/>
 * \tcat Unitaire · Historique pixel<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Appliquer une operation.<br/>2. Appeler jumpTo avec un index hors bornes.<br/>
 * \tattendu jumpTo renvoie false et le tampon reste inchange.
 * }
 */
TEST(PixelHistoryTest, JumpToHorsBornesEchoueProprement) {
    hmi::DecodedImage image = uniformImage(1, 1, RED);
    hmi::PixelHistory history;
    applyAndRecord(image, history, hmi::PixelOperationKind::Brush, 0, 0, GREEN);

    EXPECT_FALSE(history.jumpTo(image, 5));
    EXPECT_EQ(hmi::pickColor(image, 0, 0), GREEN);
}

/**
 * @brief Au-delà de la profondeur maximale, les entrées les plus anciennes sont oubliées sans
 *        corrompre l'état courant du tampon.
 * \castest{<b>Au-dela de la profondeur maximale, les entrees les plus anciennes sont
 * oubliees.</b><br/>
 * \tcat Unitaire · Historique pixel<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un historique de profondeur 2.<br/>2. Appliquer trois operations.<br/>
 * \tattendu Seules deux entrees restent (les deux plus recentes) ; le tampon courant reflete
 * toujours les trois operations appliquees.
 * }
 */
TEST(PixelHistoryTest, ProfondeurPlafonneeOublieLesEntreesLesPlusAnciennes) {
    hmi::DecodedImage image = uniformImage(3, 1, 0);
    hmi::PixelHistory history(2);

    applyAndRecord(image, history, hmi::PixelOperationKind::Brush, 0, 0, RED);
    applyAndRecord(image, history, hmi::PixelOperationKind::Brush, 1, 0, GREEN);
    applyAndRecord(image, history, hmi::PixelOperationKind::Brush, 2, 0, BLUE);

    EXPECT_EQ(history.appliedEntries().size(), 2U);
    // Le tampon courant n'est jamais affecte par l'oubli : les trois operations sont visibles.
    EXPECT_EQ(hmi::pickColor(image, 0, 0), RED);
    EXPECT_EQ(hmi::pickColor(image, 1, 0), GREEN);
    EXPECT_EQ(hmi::pickColor(image, 2, 0), BLUE);
}

/**
 * @brief `appliedEntries()` expose la liste ordonnée (chronologique) des opérations en cours,
 *        de quoi alimenter le panneau d'historique visuel (TACHE-04).
 * \castest{<b>appliedEntries expose la liste ordonnee des operations.</b><br/>
 * \tcat Unitaire · Historique pixel<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Appliquer pinceau puis gomme.<br/>2. Lire appliedEntries().<br/>
 * \tattendu La liste contient les deux entrees, dans l'ordre chronologique.
 * }
 */
TEST(PixelHistoryTest, AppliedEntriesExposeLaListeOrdonnee) {
    hmi::DecodedImage image = uniformImage(2, 1, 0);
    hmi::PixelHistory history;

    applyAndRecord(image, history, hmi::PixelOperationKind::Brush, 0, 0, RED);
    applyAndRecord(image, history, hmi::PixelOperationKind::Eraser, 0, 0, 0u);

    ASSERT_EQ(history.appliedEntries().size(), 2U);
    EXPECT_EQ(history.appliedEntries()[0].kind, hmi::PixelOperationKind::Brush);
    EXPECT_EQ(history.appliedEntries()[1].kind, hmi::PixelOperationKind::Eraser);
}

/**
 * @brief Chaque type d'opération produit une clé de traduction, et chaque clé existe, traduite,
 *        dans les deux catalogues livrés.
 * \castest{<b>Chaque cle de nom d'operation existe dans les deux catalogues.</b><br/>
 * \tcat Unitaire · Historique pixel<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger fr.lang puis en.lang.<br/>2. Resoudre la cle de chaque type
 * d'operation.<br/>
 * \tattendu Chaque cle resout vers un texte traduit, distinct de la cle elle-meme.
 * }
 */
TEST(PixelHistoryTest, ClesDeNomDOperationExistentDansLesDeuxCatalogues) {
    const std::filesystem::path directory(JADG_LOCALIZATION_DIR);
    const hmi::PixelOperationKind kinds[] = {
        hmi::PixelOperationKind::Brush,
        hmi::PixelOperationKind::Eraser,
        hmi::PixelOperationKind::Fill,
        hmi::PixelOperationKind::Move,
        hmi::PixelOperationKind::FlipHorizontal,
        hmi::PixelOperationKind::FlipVertical,
        hmi::PixelOperationKind::RotateClockwise,
        hmi::PixelOperationKind::RotateCounterClockwise,
        hmi::PixelOperationKind::Paste,
    };
    for (const std::string& language : {"fr", "en"}) {
        hmi::Localization localization(directory);
        ASSERT_TRUE(localization.loadDefaultLanguage(language)) << language;
        for (const hmi::PixelOperationKind kind : kinds) {
            const std::string key(hmi::pixelOperationTranslationKey(kind));
            EXPECT_NE(localization.text(key), key) << language << " / " << key;
        }
    }
}
