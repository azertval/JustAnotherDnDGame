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
