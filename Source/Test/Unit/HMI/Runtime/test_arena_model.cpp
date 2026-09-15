// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_arena_model.cpp
 * @brief Tests de la vue-modèle du Colisée (`LOT-24`) : un combat joué par les seuls gestes du
 *        clavier et de la manette, ce que la grille montre et cache, la réaction du joueur.
 *
 * La vue-modèle charge ses catalogues à côté de l'exécutable, comme le jeu : ces tests tournent
 * dans le dossier `bin` où le jeu déploie ses données.
 */

#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <algorithm>
#include <cstdlib>
#include <tuple>
#include <utility>

#include <gtest/gtest.h>

#include "HMI/Runtime/ArenaModel.h"

namespace {

/// Compose le personnage de démonstration contre un sanglier, ennemis joués par l'IA, et lance.
void lancer(hmi::ArenaModel& arena) {
    const QVariantList roster = arena.roster();
    ASSERT_FALSE(roster.isEmpty());
    const QString personnage = roster.front().toMap().value("id").toString();
    ASSERT_TRUE(personnage.startsWith("character:"));
    arena.addAlly(personnage);
    arena.addEnemy(QStringLiteral("boar"));
    arena.setSeed(2026);
    ASSERT_TRUE(arena.enemyAi());
    arena.launch();
    ASSERT_TRUE(arena.inCombat()) << arena.status().toStdString();
}

/// Le curseur pose sur l'ennemi le plus proche : sa case.
[[nodiscard]] std::pair<int, int> viserLePlusProche(hmi::ArenaModel& arena) {
    arena.selectAction(0);
    arena.cycleTarget(1);
    return {arena.cursorColumn(), arena.cursorRow()};
}

}  // namespace

/**
 * @brief Un combat complet se joue par les seuls gestes du clavier et de la manette.
 * \castest{<b>Un combat du Colisee se joue de bout en bout par les gestes que le clavier et la
 * manette declenchent -- cible suivante, curseur, confirmer, fin du tour --, sans un clic ;
 * l'ennemi est joue par l'IA entre deux tours du joueur.</b><br/>
 * \tcat Unitaire · IHM<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Composer le personnage de demonstration contre un sanglier, lancer.<br/>2. A chaque
 * tour du joueur : viser l'ennemi le plus proche ; s'il est hors d'allonge, recentrer, mener le
 * curseur sur une case atteignable a son contact et confirmer, puis viser et confirmer ; finir le
 * tour.<br/>
 * \tattendu Le combat atteint son issue ; le journal porte des attaques du personnage et du
 * sanglier ; chaque jet affiche dans le statut est une ligne du journal.
 * }
 */
TEST(ArenaModelTest, UnCombatSeJoueParLesSeulsGestes) {
    hmi::ArenaModel arena;
    lancer(arena);
    const QString nom = arena.allies().front().toMap().value("name").toString();

    int tours = 0;
    for (; tours < 200 && !arena.ended(); ++tours) {
        ASSERT_FALSE(arena.turnActions().isEmpty()) << "le joueur n'a pas la main";
        auto [colonne, ligne] = viserLePlusProche(arena);
        if (arena.preview().join(' ').contains(QStringLiteral("Hors d'allonge"))) {
            // S'approcher : la case atteignable au contact de la cible la plus proche du curseur.
            arena.centerCursor();
            int meilleure = -1;
            int bestColumn = 0;
            int bestRow = 0;
            for (const QVariant& entree : arena.cells()) {
                const QVariantMap c = entree.toMap();
                const int dc = std::abs(c.value("column").toInt() - colonne);
                const int dr = std::abs(c.value("row").toInt() - ligne);
                const int distance = std::max(dc, dr);
                if (c.value("reachable").toBool() && (meilleure < 0 || distance < meilleure)) {
                    meilleure = distance;
                    bestColumn = c.value("column").toInt();
                    bestRow = c.value("row").toInt();
                }
            }
            if (meilleure >= 0) {
                arena.moveCursor(bestColumn - arena.cursorColumn(), bestRow - arena.cursorRow());
                // Le chemin prévisualisé finit sur la case visée.
                const QVariantList chemin = arena.pathCells();
                ASSERT_FALSE(chemin.isEmpty());
                EXPECT_EQ(chemin.back().toMap().value("column").toInt(), bestColumn);
                EXPECT_EQ(chemin.back().toMap().value("row").toInt(), bestRow);
                arena.confirm();
            }
            if (arena.ended()) {
                break;
            }
            std::tie(colonne, ligne) = viserLePlusProche(arena);
        }
        arena.confirm();
        if (arena.status().startsWith(QStringLiteral("attaque "))) {
            // Le jet affiche est l'entree du journal.
            EXPECT_TRUE(arena.journal().contains(arena.status()));
        }
        if (!arena.ended()) {
            arena.endTurn();
        }
    }
    EXPECT_TRUE(arena.ended()) << "pas d'issue en " << tours << " tours";
    const QStringList journal = arena.journal();
    EXPECT_TRUE(std::ranges::any_of(
        journal, [&](const QString& l) { return l.startsWith(QStringLiteral("attaque ") + nom); }));
    EXPECT_TRUE(std::ranges::any_of(
        journal, [](const QString& l) { return l.startsWith(QStringLiteral("ia ")); }));
}

/**
 * @brief La grille montre le curseur, les PV des allies, et l'etat ensanglante des ennemis.
 * \castest{<b>La grille porte le curseur sur une case ; un allie montre ses points de vie, un
 * ennemi seulement s'il est ensanglante ou a terre, comme le Guide du Maitre le laisse
 * voir.</b><br/>
 * \tcat Unitaire · IHM<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Lancer un combat.<br/>2. Lire la case du curseur, celle de l'allie, celle de
 * l'ennemi.<br/>3. Deplacer le curseur hors de la grille.<br/>
 * \tattendu Le curseur sur la case du combattant actif ; « PV/maximum » pour l'allie,
 * aucun nombre pour l'ennemi ; le curseur reste dans la grille.
 * }
 */
TEST(ArenaModelTest, LaGrilleMontreLeCurseurEtCacheLesPvEnnemis) {
    hmi::ArenaModel arena;
    lancer(arena);
    const QVariantMap sousLeCurseur =
        arena.cells().at(arena.cursorRow() * arena.gridColumns() + arena.cursorColumn()).toMap();
    EXPECT_TRUE(sousLeCurseur.value("active").toBool());
    for (const QVariant& entree : arena.cells()) {
        const QVariantMap c = entree.toMap();
        if (c.value("side").toString() == "allies") {
            EXPECT_TRUE(c.value("hitPoints").toString().contains('/'));
        }
        if (c.value("side").toString() == "enemies") {
            EXPECT_FALSE(c.value("hitPoints").toString().contains('/'));
        }
    }
    arena.moveCursor(-1000, -1000);
    EXPECT_EQ(arena.cursorColumn(), 0);
    EXPECT_EQ(arena.cursorRow(), 0);
    arena.moveCursor(1000, 1000);
    EXPECT_EQ(arena.cursorColumn(), arena.gridColumns() - 1);
    EXPECT_EQ(arena.cursorRow(), arena.gridRows() - 1);
}

/**
 * @brief La reaction du joueur se bascule depuis la barre d'actions.
 * \castest{<b>La derniere action du tour est la reaction : la confirmer fait laisser passer les
 * attaques d'opportunite, la confirmer encore les fait saisir ; les actions du Manuel se
 * choisissent au numero et en boucle.</b><br/>
 * \tcat Unitaire · IHM<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Lancer ; lire les actions.<br/>2. Choisir la derniere, confirmer, relire ; confirmer
 * encore.<br/>3. Passer a l'action suivante depuis la derniere.<br/>
 * \tattendu Esquiver, se desengager, se precipiter et la reaction apres les attaques ; le libelle
 * de la reaction change a chaque confirmation ; l'action suivante de la derniere est la premiere.
 * }
 */
TEST(ArenaModelTest, LaReactionSeBasculeDepuisLaBarre) {
    hmi::ArenaModel arena;
    lancer(arena);
    QVariantList actions = arena.turnActions();
    ASSERT_GE(actions.size(), 5);
    const int derniere = static_cast<int>(actions.size()) - 1;
    EXPECT_EQ(actions[derniere - 3].toMap().value("kind").toString(), "dodge");
    EXPECT_EQ(actions[derniere - 2].toMap().value("kind").toString(), "disengage");
    EXPECT_EQ(actions[derniere - 1].toMap().value("kind").toString(), "dash");
    EXPECT_EQ(actions[derniere].toMap().value("kind").toString(), "reaction");
    const QString avant = actions[derniere].toMap().value("label").toString();

    arena.selectAction(derniere);
    EXPECT_TRUE(arena.turnActions()[derniere].toMap().value("selected").toBool());
    EXPECT_FALSE(arena.preview().isEmpty());
    arena.confirm();
    const QString apres = arena.turnActions()[derniere].toMap().value("label").toString();
    EXPECT_NE(apres, avant);
    arena.confirm();
    EXPECT_EQ(arena.turnActions()[derniere].toMap().value("label").toString(), avant);

    arena.cycleAction(1);
    EXPECT_TRUE(arena.turnActions()[0].toMap().value("selected").toBool());
}
