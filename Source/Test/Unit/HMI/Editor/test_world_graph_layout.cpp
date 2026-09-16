// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_world_graph_layout.cpp
 * @brief Tests de la disposition du graphe du monde (LOT-11) : cercle, fantomes, fleches
 *        regroupees, boucles, et designation au pointeur.
 */

#include <cmath>
#include <cstddef>
#include <numbers>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "Core/World/WorldGraph.h"
#include "HMI/Editor/WorldGraphLayout.h"

namespace {

using core::PortalLinkStatus;

core::WorldMapNode carte(std::string id, std::string erreur = {}) {
    return core::WorldMapNode{
        .mapId = id, .name = "Nom " + id, .arrivalPoints = {}, .loadError = std::move(erreur)};
}

core::WorldPortalLink portail(std::string depuis, std::string vers,
                              PortalLinkStatus statut = PortalLinkStatus::Resolved,
                              core::GridPosition position = {}) {
    return core::WorldPortalLink{.fromMap = std::move(depuis),
                                 .position = position,
                                 .toMap = std::move(vers),
                                 .arrival = "entree",
                                 .status = statut};
}

float distance(core::Vector2 a, core::Vector2 b) {
    return (a - b).length();
}

}  // namespace

TEST(WorldGraphLayout, GrapheVideSansNoeudNiFleche) {
    const hmi::WorldGraphLayout disposition = hmi::layoutWorldGraph(core::WorldGraph{});
    EXPECT_TRUE(disposition.nodes.empty());
    EXPECT_TRUE(disposition.edges.empty());
    EXPECT_FLOAT_EQ(disposition.circleRadius, 0.0f);
}

TEST(WorldGraphLayout, UneSeuleCarteAuCentre) {
    core::WorldGraph graphe;
    graphe.maps = {carte("seule")};
    const hmi::WorldGraphLayout disposition = hmi::layoutWorldGraph(graphe);
    ASSERT_EQ(disposition.nodes.size(), 1U);
    EXPECT_EQ(disposition.nodes[0].center, core::Vector2(0.0f, 0.0f));
    EXPECT_EQ(disposition.nodes[0].name, "Nom seule");
    EXPECT_FALSE(disposition.nodes[0].ghost);
}

TEST(WorldGraphLayout, CartesSurLeCercleTrieesParIdentifiantPremiereEnHaut) {
    core::WorldGraph graphe;
    graphe.maps = {carte("c"), carte("a"), carte("b")};
    const hmi::WorldGraphLayout disposition = hmi::layoutWorldGraph(graphe);
    ASSERT_EQ(disposition.nodes.size(), 3U);
    EXPECT_EQ(disposition.nodes[0].mapId, "a");
    EXPECT_EQ(disposition.nodes[1].mapId, "b");
    EXPECT_EQ(disposition.nodes[2].mapId, "c");
    for (const hmi::WorldGraphLayoutNode& noeud : disposition.nodes) {
        EXPECT_NEAR(noeud.center.length(), disposition.circleRadius, 0.01f);
    }
    EXPECT_NEAR(disposition.nodes[0].center.x, 0.0f, 0.01f);
    EXPECT_LT(disposition.nodes[0].center.y, 0.0f);  // en haut (Y vers le bas)
    EXPECT_GT(disposition.nodes[1].center.x, 0.0f);  // puis sens horaire
}

TEST(WorldGraphLayout, RayonPlancherPuisCroissantPourQueLesEtiquettesNeSeChevauchentPas) {
    EXPECT_FLOAT_EQ(hmi::worldGraphCircleRadius(0), 0.0f);
    EXPECT_FLOAT_EQ(hmi::worldGraphCircleRadius(1), 0.0f);
    EXPECT_FLOAT_EQ(hmi::worldGraphCircleRadius(2), hmi::WORLD_GRAPH_MIN_CIRCLE_RADIUS);
    for (std::size_t n = 2; n <= 40; ++n) {
        const float rayon = hmi::worldGraphCircleRadius(n);
        const float corde =
            2.0f * rayon * std::sin(std::numbers::pi_v<float> / static_cast<float>(n));
        EXPECT_GE(corde + 0.01f, hmi::WORLD_GRAPH_NODE_SPACING) << n;
        EXPECT_GE(hmi::worldGraphCircleRadius(n + 1), rayon) << n;
    }
}

TEST(WorldGraphLayout, DispositionDeterministe) {
    core::WorldGraph graphe;
    graphe.maps = {carte("a"), carte("b")};
    graphe.portals = {portail("a", "b"), portail("b", "x", PortalLinkStatus::UnknownMap)};
    const hmi::WorldGraphLayout premiere = hmi::layoutWorldGraph(graphe);
    const hmi::WorldGraphLayout seconde = hmi::layoutWorldGraph(graphe);
    ASSERT_EQ(premiere.nodes.size(), seconde.nodes.size());
    for (std::size_t i = 0; i < premiere.nodes.size(); ++i) {
        EXPECT_EQ(premiere.nodes[i].mapId, seconde.nodes[i].mapId);
        EXPECT_EQ(premiere.nodes[i].center, seconde.nodes[i].center);
    }
    EXPECT_EQ(premiere.edges.size(), seconde.edges.size());
}

TEST(WorldGraphLayout, CarteIllisibleResteUnNoeudMarque) {
    core::WorldGraph graphe;
    graphe.maps = {carte("abimee", "JSON invalide"), carte("saine")};
    const hmi::WorldGraphLayout disposition = hmi::layoutWorldGraph(graphe);
    ASSERT_EQ(disposition.nodes.size(), 2U);
    EXPECT_TRUE(disposition.nodes[0].unreadable);
    EXPECT_EQ(disposition.nodes[0].loadError, "JSON invalide");
    EXPECT_FALSE(disposition.nodes[1].unreadable);
}

TEST(WorldGraphLayout, CiblesInconnuesRegroupeesEnUnFantomeParIdentifiant) {
    core::WorldGraph graphe;
    graphe.maps = {carte("a"), carte("b")};
    graphe.portals = {portail("a", "perdue", PortalLinkStatus::UnknownMap),
                      portail("b", "perdue", PortalLinkStatus::UnknownMap),
                      portail("a", "", PortalLinkStatus::MissingTarget),
                      portail("b", "", PortalLinkStatus::MissingTarget)};
    const hmi::WorldGraphLayout disposition = hmi::layoutWorldGraph(graphe);
    ASSERT_EQ(disposition.nodes.size(), 4U);
    // Fantômes après les cartes réelles, triés : l'identifiant vide d'abord.
    EXPECT_TRUE(disposition.nodes[2].ghost);
    EXPECT_EQ(disposition.nodes[2].mapId, "");
    EXPECT_TRUE(disposition.nodes[3].ghost);
    EXPECT_EQ(disposition.nodes[3].mapId, "perdue");
    ASSERT_EQ(disposition.edges.size(), 4U);
    EXPECT_EQ(disposition.edges[0].to, 3U);
    EXPECT_EQ(disposition.edges[1].to, 3U);
    EXPECT_EQ(disposition.edges[2].to, 2U);
    EXPECT_EQ(disposition.edges[3].to, 2U);
    for (const hmi::WorldGraphLayoutEdge& fleche : disposition.edges) {
        EXPECT_TRUE(fleche.broken);
    }
    // Les fantômes partagent le cercle des cartes réelles.
    EXPECT_NEAR(disposition.nodes[3].center.length(), disposition.circleRadius, 0.01f);
}

TEST(WorldGraphLayout, PortailsDUneMemePaireOrdonneeDessinesUneFoisAvecLeurCompte) {
    core::WorldGraph graphe;
    graphe.maps = {carte("a"), carte("b")};
    graphe.portals = {portail("a", "b"), portail("a", "b"), portail("b", "a")};
    const hmi::WorldGraphLayout disposition = hmi::layoutWorldGraph(graphe);
    ASSERT_EQ(disposition.edges.size(), 2U);
    EXPECT_EQ(disposition.edges[0].from, 0U);
    EXPECT_EQ(disposition.edges[0].to, 1U);
    EXPECT_EQ(disposition.edges[0].count(), 2U);
    EXPECT_EQ(disposition.edges[0].portals, (std::vector<std::size_t>{0, 1}));
    EXPECT_FALSE(disposition.edges[0].broken);
    EXPECT_EQ(disposition.edges[0].status, PortalLinkStatus::Resolved);
    EXPECT_EQ(disposition.edges[1].from, 1U);
    EXPECT_EQ(disposition.edges[1].to, 0U);
    EXPECT_EQ(disposition.edges[1].count(), 1U);
}

TEST(WorldGraphLayout, FlecheCasseeDesQuUnPortailLEstStatutDuPremierNonResolu) {
    core::WorldGraph graphe;
    graphe.maps = {carte("a"), carte("b")};
    graphe.portals = {portail("a", "b"), portail("a", "b", PortalLinkStatus::UnknownArrival),
                      portail("a", "b", PortalLinkStatus::MissingArrival)};
    const hmi::WorldGraphLayout disposition = hmi::layoutWorldGraph(graphe);
    ASSERT_EQ(disposition.edges.size(), 1U);
    EXPECT_TRUE(disposition.edges[0].broken);
    EXPECT_EQ(disposition.edges[0].status, PortalLinkStatus::UnknownArrival);
    EXPECT_EQ(disposition.edges[0].count(), 3U);
}

TEST(WorldGraphLayout, CibleIllisibleVisePasUnFantomeMaisLaCarte) {
    core::WorldGraph graphe;
    graphe.maps = {carte("a"), carte("b", "illisible")};
    graphe.portals = {portail("a", "b", PortalLinkStatus::TargetUnreadable)};
    const hmi::WorldGraphLayout disposition = hmi::layoutWorldGraph(graphe);
    ASSERT_EQ(disposition.nodes.size(), 2U);
    ASSERT_EQ(disposition.edges.size(), 1U);
    EXPECT_EQ(disposition.edges[0].to, 1U);
    EXPECT_TRUE(disposition.edges[0].broken);
}

TEST(WorldGraphLayout, PortailVersSaPropreCarteEstUneBoucle) {
    core::WorldGraph graphe;
    graphe.maps = {carte("a")};
    graphe.portals = {portail("a", "a")};
    const hmi::WorldGraphLayout disposition = hmi::layoutWorldGraph(graphe);
    ASSERT_EQ(disposition.edges.size(), 1U);
    EXPECT_TRUE(disposition.edges[0].selfLoop);
    const hmi::WorldGraphEdgeGeometry trace = hmi::worldGraphEdgeGeometry(disposition, 0);
    EXPECT_GT(trace.loopRadius, 0.0f);
    EXPECT_LT(trace.loopCenter.y, 0.0f);  // la boucle monte : l'étiquette est dessous
    EXPECT_GT(trace.loopCenter.x, 0.0f);  // nœud au centre : à droite
    EXPECT_NEAR(distance(trace.loopCenter, disposition.nodes[0].center),
                hmi::WORLD_GRAPH_NODE_RADIUS, 0.01f);
}

TEST(WorldGraphLayout, TraceDUneFlecheDuBordSourceAuBordCible) {
    core::WorldGraph graphe;
    graphe.maps = {carte("a"), carte("b")};
    graphe.portals = {portail("a", "b")};
    const hmi::WorldGraphLayout disposition = hmi::layoutWorldGraph(graphe);
    const hmi::WorldGraphEdgeGeometry trace = hmi::worldGraphEdgeGeometry(disposition, 0);
    EXPECT_NEAR(distance(trace.start, disposition.nodes[0].center), hmi::WORLD_GRAPH_NODE_RADIUS,
                0.01f);
    EXPECT_NEAR(distance(trace.end, disposition.nodes[1].center), hmi::WORLD_GRAPH_NODE_RADIUS,
                0.01f);
}

TEST(WorldGraphLayout, FlechesOpposeesDecaleesPourNePasSeSuperposer) {
    core::WorldGraph graphe;
    graphe.maps = {carte("a"), carte("b")};
    graphe.portals = {portail("a", "b"), portail("b", "a")};
    const hmi::WorldGraphLayout disposition = hmi::layoutWorldGraph(graphe);
    const hmi::WorldGraphEdgeGeometry aller = hmi::worldGraphEdgeGeometry(disposition, 0);
    const hmi::WorldGraphEdgeGeometry retour = hmi::worldGraphEdgeGeometry(disposition, 1);
    EXPECT_GT(distance(aller.badge, retour.badge), 1.0f);
}

TEST(WorldGraphLayout, NodeAtTrouveLeNoeudSousLePointeur) {
    core::WorldGraph graphe;
    graphe.maps = {carte("a"), carte("b")};
    const hmi::WorldGraphLayout disposition = hmi::layoutWorldGraph(graphe);
    const core::Vector2 centreB = disposition.nodes[1].center;
    EXPECT_EQ(hmi::nodeAt(disposition, centreB, hmi::WORLD_GRAPH_NODE_RADIUS),
              std::optional<std::size_t>(1));
    EXPECT_EQ(hmi::nodeAt(disposition, centreB + core::Vector2(hmi::WORLD_GRAPH_NODE_RADIUS, 0.0f),
                          hmi::WORLD_GRAPH_NODE_RADIUS),
              std::optional<std::size_t>(1));  // bord inclus
    EXPECT_EQ(hmi::nodeAt(disposition, core::Vector2(0.0f, 0.0f), hmi::WORLD_GRAPH_NODE_RADIUS),
              std::nullopt);
}

TEST(WorldGraphLayout, NodeAtDepartageParLePlusPetitIndice) {
    hmi::WorldGraphLayout disposition;
    disposition.nodes.resize(2);
    disposition.nodes[0].center = core::Vector2(-10.0f, 0.0f);
    disposition.nodes[1].center = core::Vector2(10.0f, 0.0f);
    EXPECT_EQ(hmi::nodeAt(disposition, core::Vector2(0.0f, 0.0f), 20.0f),
              std::optional<std::size_t>(0));
    EXPECT_EQ(hmi::nodeAt(disposition, core::Vector2(4.0f, 0.0f), 20.0f),
              std::optional<std::size_t>(1));  // le plus proche l'emporte
}

TEST(WorldGraphLayout, EdgeAtTrouveLaFlecheEtLaBoucle) {
    core::WorldGraph graphe;
    graphe.maps = {carte("a"), carte("b")};
    graphe.portals = {portail("a", "b"), portail("b", "b")};
    const hmi::WorldGraphLayout disposition = hmi::layoutWorldGraph(graphe);
    const hmi::WorldGraphEdgeGeometry fleche = hmi::worldGraphEdgeGeometry(disposition, 0);
    EXPECT_EQ(hmi::edgeAt(disposition, fleche.badge, 4.0f), std::optional<std::size_t>(0));
    const hmi::WorldGraphEdgeGeometry boucle = hmi::worldGraphEdgeGeometry(disposition, 1);
    EXPECT_EQ(hmi::edgeAt(disposition, boucle.badge, 4.0f), std::optional<std::size_t>(1));
    EXPECT_EQ(hmi::edgeAt(disposition, core::Vector2(500.0f, 500.0f), 4.0f), std::nullopt);
}
