// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/LevelDraft.h"
#include "Core/Levels/TileType.h"
#include "HMI/Editor/LinkGeometry.h"

namespace {

using core::GridPosition;
using core::TileType;

}  // namespace


/**
 * @brief Le trait relie les **centres** des deux cases (origine en haut à gauche, demi-case
 * ajoutée) :
 * partir du coin ferait pointer les flèches à côté des mécanismes qu'elles relient.
 * \castest{<b>Le trait de liaison relie les centres des deux cases.</b><br/>
 * \tcat Unitaire · Géométrie des liens<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(LinkGeometry, LinkSegmentRelieLesCentresDeCase) {
    const hmi::LinkSegment segment = hmi::linkSegment(GridPosition{0, 0}, GridPosition{3, 0});
    EXPECT_FLOAT_EQ(segment.a.x, 0.5f);
    EXPECT_FLOAT_EQ(segment.a.y, 0.5f);
    EXPECT_FLOAT_EQ(segment.b.x, 3.5f);
    EXPECT_FLOAT_EQ(segment.b.y, 0.5f);
}

/**
 * @brief Un lien vertical ne fait varier que l'ordonnée, dans le sens du déclencheur vers la cible
 * :
 * l'orientation du segment porte le sens de lecture de la flèche.
 * \castest{<b>Un lien vertical ne fait varier que l'ordonnée, du déclencheur vers la
 * cible.</b><br/>
 * \tcat Unitaire · Géométrie des liens<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(LinkGeometry, LinkSegmentVertical) {
    const hmi::LinkSegment segment = hmi::linkSegment(GridPosition{2, 1}, GridPosition{2, 5});
    EXPECT_FLOAT_EQ(segment.a.x, segment.b.x);
    EXPECT_LT(segment.a.y, segment.b.y);
}

/**
 * @brief Un lien diagonal fait varier les deux coordonnées et, s'il est seul, ne subit **aucun**
 * décalage : l'écartement en éventail ne se paie que lorsqu'il sert à distinguer des frères.
 * \castest{<b>Un lien diagonal seul relie les centres sans aucun décalage.</b><br/>
 * \tcat Unitaire · Géométrie des liens<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(LinkGeometry, LinkSegmentDiagonalSansDecalage) {
    const hmi::LinkSegment segment = hmi::linkSegment(GridPosition{0, 0}, GridPosition{2, 2});
    EXPECT_FLOAT_EQ(segment.a.x, 0.5f);
    EXPECT_FLOAT_EQ(segment.a.y, 0.5f);
    EXPECT_FLOAT_EQ(segment.b.x, 2.5f);
    EXPECT_FLOAT_EQ(segment.b.y, 2.5f);
}

/**
 * @brief Plusieurs liens partant du même déclencheur gardent une **base commune** et n'écartent que
 * leur extrémité cible, symétriquement autour du centre et de façon déterministe. Sans cet
 * éventail, trois liens vers la même zone se superposeraient en un seul trait illisible ; sans
 * déterminisme, ils sauteraient d'une frame à l'autre.
 * \castest{<b>Des liens frères gardent une base commune et écartent symétriquement leur extrémité
 * cible.</b><br/>
 * \tcat Unitaire · Géométrie des liens<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(LinkGeometry, LinkSegmentDecalageAntiSuperpositionBaseCommune) {
    constexpr GridPosition trigger{0, 0};
    constexpr GridPosition target{4, 0};
    const hmi::LinkSegment first = hmi::linkSegment(trigger, target, 0, 3);
    const hmi::LinkSegment second = hmi::linkSegment(trigger, target, 1, 3);
    const hmi::LinkSegment third = hmi::linkSegment(trigger, target, 2, 3);

    // Base commune (centre du declencheur) : identique quel que soit le decalage du frere.
    EXPECT_FLOAT_EQ(first.a.x, second.a.x);
    EXPECT_FLOAT_EQ(first.a.y, second.a.y);
    EXPECT_FLOAT_EQ(second.a.x, third.a.x);
    EXPECT_FLOAT_EQ(second.a.y, third.a.y);

    // Segment du milieu (index 1 sur 3) : cible non decalee (offset nul par symetrie).
    EXPECT_FLOAT_EQ(second.b.y, 0.5f);

    // Les deux extremes sont decales de part et d'autre de la cible, de la meme distance.
    EXPECT_NE(first.b.y, second.b.y);
    EXPECT_NE(third.b.y, second.b.y);
    EXPECT_FLOAT_EQ(second.b.y - first.b.y, third.b.y - second.b.y);

    // Meme appel deux fois -> meme resultat (deterministe).
    EXPECT_EQ(hmi::linkSegment(trigger, target, 0, 3).b.y, first.b.y);
}

/**
 * @brief La pointe de flèche a ses deux ailes en retrait de la cible et symétriques par rapport à
 * l'axe
 * du segment : c'est ce qui rend le **sens** de la liaison lisible d'un coup d'œil.
 * \castest{<b>La pointe de flèche a ses ailes en retrait et symétriques par rapport à
 * l'axe.</b><br/>
 * \tcat Unitaire · Géométrie des liens<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(LinkGeometry, ArrowHeadOrienteeVersLaCible) {
    const hmi::ArrowHead head =
        hmi::arrowHead(core::Vector2{0.0f, 0.0f}, core::Vector2{4.0f, 0.0f});
    // Les deux ailes reculent par rapport a la pointe (x < 4) et sont symetriques en y.
    EXPECT_LT(head.left.x, 4.0f);
    EXPECT_LT(head.right.x, 4.0f);
    EXPECT_FLOAT_EQ(head.left.y, -head.right.y);
    EXPECT_NE(head.left.y, 0.0f);
}

/**
 * @brief Un segment dégénéré (mêmes extrémités) n'a pas de direction : la pointe se replie sur le
 * point
 * lui-même plutôt que de produire des coordonnées non définies issues d'une normalisation par
 * zéro.
 * \castest{<b>Un segment dégénéré replie la pointe sur le point, sans division par zéro.</b><br/>
 * \tcat Unitaire · Géométrie des liens<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(LinkGeometry, ArrowHeadSegmentDegenere) {
    const core::Vector2 point{1.0f, 1.0f};
    const hmi::ArrowHead head = hmi::arrowHead(point, point);
    EXPECT_FLOAT_EQ(head.left.x, point.x);
    EXPECT_FLOAT_EQ(head.left.y, point.y);
    EXPECT_FLOAT_EQ(head.right.x, point.x);
    EXPECT_FLOAT_EQ(head.right.y, point.y);
}


/**
 * @brief Un brouillon sans aucune liaison donne une liste vide : le panneau affiche alors son état
 * vide, il n'a pas de cas particulier à traiter.
 * \castest{<b>Un brouillon sans liaison donne une liste vide.</b><br/>
 * \tcat Unitaire · Géométrie des liens<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(LinkGeometry, BuildLinkRowsVideSansLiaison) {
    const core::LevelDraft draft = core::LevelDraft::empty("N", 4, 4);
    EXPECT_TRUE(hmi::buildLinkRows(draft).empty());
}
