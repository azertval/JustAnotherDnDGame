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

/**
 * @brief Un graphe vide se dispose sans nœud, sans flèche et sans rayon.
 * \castest{<b>Un graphe vide ne dessine rien.</b><br/>
 * \tcat Unitaire · Graphe du monde<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Disposer un graphe sans carte ni portail.<br/>
 * \tattendu Aucun noeud, aucune fleche, rayon nul.
 * }
 */
TEST(WorldGraphLayout, GrapheVideSansNoeudNiFleche) {
    const hmi::WorldGraphLayout disposition = hmi::layoutWorldGraph(core::WorldGraph{});
    EXPECT_TRUE(disposition.nodes.empty());
    EXPECT_TRUE(disposition.edges.empty());
    EXPECT_FLOAT_EQ(disposition.circleRadius, 0.0f);
}

/**
 * @brief Une carte seule se place au centre, avec son nom.
 * \castest{<b>Une carte seule est au centre.</b><br/>
 * \tcat Unitaire · Graphe du monde<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Disposer un graphe d'une seule carte.<br/>
 * \tattendu Un noeud reel, au centre, qui porte le nom de la carte.
 * }
 */
TEST(WorldGraphLayout, UneSeuleCarteAuCentre) {
    core::WorldGraph graphe;
    graphe.maps = {carte("seule")};
    const hmi::WorldGraphLayout disposition = hmi::layoutWorldGraph(graphe);
    ASSERT_EQ(disposition.nodes.size(), 1U);
    EXPECT_EQ(disposition.nodes[0].center, core::Vector2(0.0f, 0.0f));
    EXPECT_EQ(disposition.nodes[0].name, "Nom seule");
    EXPECT_FALSE(disposition.nodes[0].ghost);
}

/**
 * @brief Les cartes se rangent sur le cercle par identifiant, la première en haut, puis dans le
 * sens horaire.
 * \castest{<b>Les cartes sont rangees sur le cercle.</b><br/>
 * \tcat Unitaire · Graphe du monde<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Disposer trois cartes donnees dans le desordre.<br/>
 * \tattendu Ordre a, b, c ; toutes sur le cercle ; a en haut, b a droite.
 * }
 */
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

/**
 * @brief Le rayon du cercle a un plancher, puis croît pour tenir l'écart entre deux cartes
 * voisines.
 * \castest{<b>Le rayon du cercle espace les cartes.</b><br/>
 * \tcat Unitaire · Graphe du monde<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Calculer le rayon de 0 a 41 cartes.<br/>
 * \tattendu Nul jusqu'a une carte, plancher a deux, corde jamais sous l'ecart voulu, rayon
 * croissant.
 * }
 */
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

/**
 * @brief Deux dispositions du même graphe sont identiques.
 * \castest{<b>La disposition est deterministe.</b><br/>
 * \tcat Unitaire · Graphe du monde<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Disposer deux fois le meme graphe, fantome compris.<br/>
 * \tattendu Memes noeuds aux memes places, meme nombre de fleches.
 * }
 */
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

/**
 * @brief Une carte illisible reste un nœud, marqué et porteur de son erreur.
 * \castest{<b>Une carte illisible reste un noeud marque.</b><br/>
 * \tcat Unitaire · Graphe du monde<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Disposer une carte en erreur de chargement et une carte saine.<br/>
 * \tattendu Le premier noeud est marque illisible avec son erreur, le second non.
 * }
 */
TEST(WorldGraphLayout, CarteIllisibleResteUnNoeudMarque) {
    core::WorldGraph graphe;
    graphe.maps = {carte("abimee", "JSON invalide"), carte("saine")};
    const hmi::WorldGraphLayout disposition = hmi::layoutWorldGraph(graphe);
    ASSERT_EQ(disposition.nodes.size(), 2U);
    EXPECT_TRUE(disposition.nodes[0].unreadable);
    EXPECT_EQ(disposition.nodes[0].loadError, "JSON invalide");
    EXPECT_FALSE(disposition.nodes[1].unreadable);
}

/**
 * @brief Les portails vers une carte inconnue, ou sans cible, mènent à un fantôme par identifiant.
 * \castest{<b>Les cibles inconnues deviennent des fantomes.</b><br/>
 * \tcat Unitaire · Graphe du monde<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Disposer quatre portails : deux vers une carte inconnue, deux sans cible.<br/>
 * \tattendu Deux fantomes apres les cartes reelles, sur le cercle ; quatre fleches cassees vers
 * eux.
 * }
 */
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

/**
 * @brief Les portails d'une même paire ordonnée se dessinent en une flèche, avec leur compte.
 * \castest{<b>Les portails d'une meme paire font une fleche.</b><br/>
 * \tcat Unitaire · Graphe du monde<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Disposer deux portails de a vers b et un de b vers a.<br/>
 * \tattendu Deux fleches : a vers b comptant 2, b vers a comptant 1.
 * }
 */
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

/**
 * @brief Une flèche est cassée dès qu'un de ses portails l'est, avec le statut du premier non
 * résolu.
 * \castest{<b>Une fleche est cassee des qu'un portail l'est.</b><br/>
 * \tcat Unitaire · Graphe du monde<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Disposer trois portails de a vers b, dont deux non resolus.<br/>
 * \tattendu Une fleche cassee, au statut du premier non resolu, comptant 3.
 * }
 */
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

/**
 * @brief Un portail vers une carte illisible vise cette carte, pas un fantôme.
 * \castest{<b>Une cible illisible reste la carte.</b><br/>
 * \tcat Unitaire · Graphe du monde<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Disposer un portail vers une carte en erreur de chargement.<br/>
 * \tattendu Deux noeuds, une fleche cassee vers la carte illisible.
 * }
 */
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

/**
 * @brief Un portail vers sa propre carte est une boucle, au-dessus du nœud.
 * \castest{<b>Un portail vers sa propre carte est une boucle.</b><br/>
 * \tcat Unitaire · Graphe du monde<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Disposer un portail de a vers a.<br/>2. Calculer son trace.<br/>
 * \tattendu Une boucle au-dessus et a droite du noeud, sur son bord.
 * }
 */
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

/**
 * @brief Une flèche part du bord de sa source et arrive au bord de sa cible.
 * \castest{<b>Une fleche va de bord a bord.</b><br/>
 * \tcat Unitaire · Graphe du monde<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Calculer le trace d'une fleche de a vers b.<br/>
 * \tattendu Depart et arrivee a un rayon de noeud des deux centres.
 * }
 */
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

/**
 * @brief L'aller et le retour entre deux cartes se décalent pour ne pas se superposer.
 * \castest{<b>Les fleches opposees se decalent.</b><br/>
 * \tcat Unitaire · Graphe du monde<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Calculer les traces de a vers b et de b vers a.<br/>
 * \tattendu Leurs pastilles ne se confondent pas.
 * }
 */
TEST(WorldGraphLayout, FlechesOpposeesDecaleesPourNePasSeSuperposer) {
    core::WorldGraph graphe;
    graphe.maps = {carte("a"), carte("b")};
    graphe.portals = {portail("a", "b"), portail("b", "a")};
    const hmi::WorldGraphLayout disposition = hmi::layoutWorldGraph(graphe);
    const hmi::WorldGraphEdgeGeometry aller = hmi::worldGraphEdgeGeometry(disposition, 0);
    const hmi::WorldGraphEdgeGeometry retour = hmi::worldGraphEdgeGeometry(disposition, 1);
    EXPECT_GT(distance(aller.badge, retour.badge), 1.0f);
}

/**
 * @brief Le pointeur désigne le nœud qu'il survole, bord compris.
 * \castest{<b>Le pointeur designe le noeud survole.</b><br/>
 * \tcat Unitaire · Graphe du monde<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Designer le centre d'un noeud, puis son bord, puis un point vide.<br/>
 * \tattendu Le noeud, le noeud, rien.
 * }
 */
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

/**
 * @brief Entre deux nœuds qui se recouvrent, le plus proche l'emporte, puis le plus petit indice.
 * \castest{<b>Deux noeuds recouverts se departagent.</b><br/>
 * \tcat Unitaire · Graphe du monde<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Designer le point equidistant de deux noeuds, puis un point plus proche du
 * second.<br/>
 * \tattendu Le premier a egalite, le plus proche sinon.
 * }
 */
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

/**
 * @brief Le pointeur désigne une flèche ou une boucle par sa pastille.
 * \castest{<b>Le pointeur designe une fleche ou une boucle.</b><br/>
 * \tcat Unitaire · Graphe du monde<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Designer la pastille d'une fleche, celle d'une boucle, puis un point lointain.<br/>
 * \tattendu La fleche, la boucle, rien.
 * }
 */
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
