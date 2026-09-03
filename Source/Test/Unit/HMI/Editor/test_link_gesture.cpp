// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/TileType.h"
#include "HMI/Editor/LinkGesture.h"

namespace {

using core::GridPosition;
using core::TileType;
using hmi::LinkGestureAction;
using hmi::PendingLink;
using hmi::resolveLinkClick;

}  // namespace

/**
 * @brief Une case qui n'est ni déclencheur ni cible est ignorée, sans attente en cours : l'outil
 * de liaison partage le canevas avec tout le reste, un clic à côté ne doit rien amorcer.
 * \castest{<b>Un clic hors catégorie est ignoré quand aucune attente n'est en cours.</b><br/>
 * \tcat Unitaire · Geste de liaison<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(LinkGesture, ClicHorsCategorieIgnore) {
    const auto decision =
        resolveLinkClick(std::nullopt, GridPosition{1, 1}, TileType::Solid, false);
    EXPECT_EQ(decision.action, LinkGestureAction::Ignore);
}

/**
 * @brief Sans attente, un clic sur un déclencheur pose l'attente sur cette case : c'est la
 * première moitié du geste en deux temps.
 * \castest{<b>Sans attente, un clic sur un déclencheur pose l'attente.</b><br/>
 * \tcat Unitaire · Geste de liaison<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(LinkGesture, PremierClicSurDeclencheurPoseLAttente) {
    const auto decision =
        resolveLinkClick(std::nullopt, GridPosition{2, 3}, TileType::Switch, false);
    EXPECT_EQ(decision.action, LinkGestureAction::SetPending);
    EXPECT_EQ(decision.cell, (GridPosition{2, 3}));
}

/**
 * @brief Un clic sur une **cible** pose l'attente de la même façon : l'ordre est indifférent,
 * l'auteur peut partir de la porte comme de l'interrupteur.
 * \castest{<b>Sans attente, un clic sur une cible pose aussi l'attente.</b><br/>
 * \tcat Unitaire · Geste de liaison<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(LinkGesture, PremierClicSurCiblePoseLAttente) {
    const auto decision = resolveLinkClick(std::nullopt, GridPosition{4, 0}, TileType::Door, false);
    EXPECT_EQ(decision.action, LinkGestureAction::SetPending);
    EXPECT_EQ(decision.cell, (GridPosition{4, 0}));
}

/**
 * @brief Deux déclencheurs de suite ne se lient pas entre eux : le second **remplace** l'attente.
 * C'est le rattrapage naturel après un clic sur le mauvais interrupteur, sans geste d'annulation
 * à connaître.
 * \castest{<b>Deux déclencheurs successifs remplacent l'attente au lieu de se lier.</b><br/>
 * \tcat Unitaire · Geste de liaison<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(LinkGesture, DeuxDeclencheursRemplacentLAttente) {
    const PendingLink pending{GridPosition{0, 0}, TileType::Switch};
    const auto decision =
        resolveLinkClick(pending, GridPosition{1, 0}, TileType::PressurePlate, false);
    EXPECT_EQ(decision.action, LinkGestureAction::ReplacePending);
    EXPECT_EQ(decision.cell, (GridPosition{1, 0}));
}

/**
 * @brief Déclencheur en attente puis clic sur une cible pas encore liée : la liaison est créée,
 * avec le déclencheur et la cible dans leurs rôles respectifs — jamais dans l'ordre des clics.
 * \castest{<b>Déclencheur en attente puis cible non liée crée la liaison.</b><br/>
 * \tcat Unitaire · Geste de liaison<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(LinkGesture, DeclencheurPuisCibleNonLieeCreeLaLiaison) {
    const PendingLink pending{GridPosition{0, 0}, TileType::Switch};
    const auto decision = resolveLinkClick(pending, GridPosition{3, 3}, TileType::Door, false);
    EXPECT_EQ(decision.action, LinkGestureAction::Link);
    EXPECT_EQ(decision.switchPosition, (GridPosition{0, 0}));
    EXPECT_EQ(decision.targetPosition, (GridPosition{3, 3}));
}

/**
 * @brief Une attente périmée — la case retenue a été repeinte entre-temps et ne porte plus un
 * type liable — est traitée comme une absence d'attente : le clic courant devient la nouvelle
 * attente, au lieu de créer une liaison vers une case qui n'est plus un mécanisme.
 * \castest{<b>Une attente périmée est ignorée et remplacée par le clic courant.</b><br/>
 * \tcat Unitaire · Geste de liaison<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(LinkGesture, AttentePerimeeEstIgnoreeEtRemplacee) {
    const PendingLink pending{GridPosition{0, 0}, TileType::Solid};  // repeint depuis
    const auto decision = resolveLinkClick(pending, GridPosition{3, 3}, TileType::Door, false);
    EXPECT_EQ(decision.action, LinkGestureAction::SetPending);
    EXPECT_EQ(decision.cell, (GridPosition{3, 3}));
}

/**
 * @brief Un clic hors catégorie alors qu'une attente est en cours est ignoré **sans** perdre
 * l'attente : un clic imprécis à côté du mécanisme visé ne doit pas obliger à tout recommencer.
 * \castest{<b>Un clic hors catégorie est ignoré sans perdre l'attente en cours.</b><br/>
 * \tcat Unitaire · Geste de liaison<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(LinkGesture, ClicHorsCategorieAvecAttenteIgnore) {
    const PendingLink pending{GridPosition{0, 0}, TileType::Switch};
    const auto decision = resolveLinkClick(pending, GridPosition{1, 1}, TileType::Solid, false);
    EXPECT_EQ(decision.action, LinkGestureAction::Ignore);
}
