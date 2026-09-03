// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include <optional>
#include <string>

#include <gtest/gtest.h>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/TileType.h"
#include "HMI/Editor/TextureAssignGesture.h"

namespace {

using core::GridPosition;
using core::TileType;
using hmi::resolveTextureAssignClick;
using hmi::TextureAssignAction;

}  // namespace

/**
 * @brief Une case vide est toujours ignorée, quels que soient la sélection et l'état de surcharge :
 * il n'y a rien à habiller là, et assigner une texture au vide laisserait une surcharge orpheline
 * dans le fichier de niveau.
 * \castest{<b>Une case vide est toujours ignorée.</b><br/>
 * \tcat Unitaire · Geste d'assignation de texture<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(TextureAssignGesture, CaseVideIgnoree) {
    const auto decision = resolveTextureAssignClick(GridPosition{1, 1}, TileType::Empty,
                                                    std::nullopt, std::string{"crate.png"}, false);
    EXPECT_EQ(decision.action, TextureAssignAction::Ignore);
}

/**
 * @brief Sans asset sélectionné, un clic sur une case sans surcharge est ignoré : il n'y a rien à
 * assigner, et l'outil ne doit pas inventer de valeur par défaut.
 * \castest{<b>Sans asset sélectionné, le clic est ignoré.</b><br/>
 * \tcat Unitaire · Geste d'assignation de texture<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(TextureAssignGesture, AucunAssetSelectionneIgnore) {
    const auto decision = resolveTextureAssignClick(GridPosition{1, 1}, TileType::Door,
                                                    std::nullopt, std::nullopt, false);
    EXPECT_EQ(decision.action, TextureAssignAction::Ignore);
}

/**
 * @brief Sur une case sans surcharge, le clic assigne l'asset sélectionné à cette case précise : le
 * cas nominal de l'habillage par instance.
 * \castest{<b>Sur une case sans surcharge, le clic assigne l'asset sélectionné.</b><br/>
 * \tcat Unitaire · Geste d'assignation de texture<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(TextureAssignGesture, CaseSansOverrideAssigneLAssetSelectionne) {
    const auto decision = resolveTextureAssignClick(
        GridPosition{2, 3}, TileType::Door, std::nullopt, std::string{"door_red.png"}, false);
    EXPECT_EQ(decision.action, TextureAssignAction::Assign);
    EXPECT_EQ(decision.cell, (GridPosition{2, 3}));
    EXPECT_EQ(decision.assetName, "door_red.png");
}

/**
 * @brief Sur une case déjà surchargée avec un **autre** asset, le clic remplace : pas besoin de
 * retirer d'abord pour changer d'avis.
 * \castest{<b>Sur une case surchargée avec un autre asset, le clic remplace.</b><br/>
 * \tcat Unitaire · Geste d'assignation de texture<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(TextureAssignGesture, CaseAvecOverrideDifferentRemplace) {
    const auto decision =
        resolveTextureAssignClick(GridPosition{2, 3}, TileType::Door, std::string{"door_blue.png"},
                                  std::string{"door_red.png"}, false);
    EXPECT_EQ(decision.action, TextureAssignAction::Assign);
    EXPECT_EQ(decision.assetName, "door_red.png");
}

/**
 * @brief Recliquer avec le **même** asset retire la surcharge : le geste est une bascule, ce qui
 * rend l'annulation atteignable sans changer d'outil ni de bouton.
 * \castest{<b>Recliquer avec le même asset retire la surcharge.</b><br/>
 * \tcat Unitaire · Geste d'assignation de texture<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(TextureAssignGesture, ReclicSurLeMemeAssetRetire) {
    const auto decision =
        resolveTextureAssignClick(GridPosition{2, 3}, TileType::Door, std::string{"door_red.png"},
                                  std::string{"door_red.png"}, false);
    EXPECT_EQ(decision.action, TextureAssignAction::Remove);
    EXPECT_EQ(decision.cell, (GridPosition{2, 3}));
}

/**
 * @brief Le clic droit retire explicitement la surcharge, sans dépendre de l'asset sélectionné : le
 * chemin direct quand on veut simplement revenir au skin du type.
 * \castest{<b>Le clic droit retire la surcharge, indépendamment de la sélection.</b><br/>
 * \tcat Unitaire · Geste d'assignation de texture<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(TextureAssignGesture, ClicDroitAvecOverrideRetire) {
    const auto decision = resolveTextureAssignClick(
        GridPosition{2, 3}, TileType::Door, std::string{"door_red.png"}, std::nullopt, true);
    EXPECT_EQ(decision.action, TextureAssignAction::Remove);
    EXPECT_EQ(decision.cell, (GridPosition{2, 3}));
}

/**
 * @brief Le clic droit sur une case sans surcharge est sans effet : il n'y a rien à retirer, et
 * l'opération ne doit pas entrer dans l'historique.
 * \castest{<b>Le clic droit sur une case sans surcharge est sans effet.</b><br/>
 * \tcat Unitaire · Geste d'assignation de texture<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(TextureAssignGesture, ClicDroitSansOverrideIgnore) {
    const auto decision = resolveTextureAssignClick(
        GridPosition{2, 3}, TileType::Door, std::nullopt, std::string{"door_red.png"}, true);
    EXPECT_EQ(decision.action, TextureAssignAction::Ignore);
}

/**
 * @brief Le clic droit sur une case vide est ignoré avant même de consulter la surcharge : la règle
 * « case vide, rien à faire » prime sur toutes les autres.
 * \castest{<b>Le clic droit sur une case vide est ignoré.</b><br/>
 * \tcat Unitaire · Geste d'assignation de texture<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(TextureAssignGesture, ClicDroitSurCaseVideIgnore) {
    const auto decision = resolveTextureAssignClick(
        GridPosition{2, 3}, TileType::Empty, std::string{"door_red.png"}, std::nullopt, true);
    EXPECT_EQ(decision.action, TextureAssignAction::Ignore);
}
