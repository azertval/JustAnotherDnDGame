// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_arena.cpp
 * @brief Tests unitaires du Colisée (`LOT-50`) : points d'entrée, catalogues, kits, montage,
 *        coup d'essai, rejeu et non-létalité.
 */

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Combat/Arena.h"
#include "Core/Combat/BattleGrid.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/TileMap.h"
#include "Core/Rpg/Ability.h"

namespace {

using core::CombatantId;
using core::CombatSide;

/// Une piste 12x8 ceinte de murs, trois entrées par camp, un pilier au centre.
core::Level piste() {
    core::TileMap carte(12, 8);
    for (int x = 0; x < 12; ++x) {
        carte.setTile(x, 0, core::TileType::Wall);
        carte.setTile(x, 7, core::TileType::Wall);
    }
    for (int y = 0; y < 8; ++y) {
        carte.setTile(0, y, core::TileType::Wall);
        carte.setTile(11, y, core::TileType::Wall);
    }
    carte.setTile(6, 3, core::TileType::Wall);
    std::vector<core::MapEntity> entites;
    const auto entree = [&](CombatSide camp, int rang, int x, int y) {
        core::MapEntity e;
        e.type = std::string(core::ARENA_ENTRY_ENTITY_TYPE);
        e.position = {x, y};
        e.properties[std::string(core::ARENA_SIDE_PROPERTY)] =
            std::string(camp == CombatSide::Allies ? "allies" : "enemies");
        e.properties[std::string(core::ARENA_RANK_PROPERTY)] = static_cast<std::int64_t>(rang);
        entites.push_back(std::move(e));
    };
    entree(CombatSide::Enemies, 2, 9, 2);
    entree(CombatSide::Allies, 1, 2, 3);
    entree(CombatSide::Enemies, 1, 9, 3);
    entree(CombatSide::Allies, 2, 2, 2);
    entree(CombatSide::Allies, 3, 2, 4);
    entree(CombatSide::Enemies, 3, 9, 4);
    // Une entree sans camp lisible : ignoree, pas une carte invalide.
    core::MapEntity muette;
    muette.type = std::string(core::ARENA_ENTRY_ENTITY_TYPE);
    muette.position = {5, 5};
    entites.push_back(muette);
    return core::Level(core::LevelData{.name = "piste",
                                       .tileMap = std::move(carte),
                                       .entities = std::move(entites),
                                       .entry = {1, 1},
                                       .exit = {10, 6}});
}

core::ArenaContestant concurrent(const std::string& nom, CombatSide camp, int pv, int dexterite,
                                 int bonus, const char* degats, int ca) {
    core::CombatantProfile profil{.name = nom,
                                  .side = camp,
                                  .maximumHitPoints = pv,
                                  .currentHitPoints = pv,
                                  .dexterity = dexterite,
                                  .initiativeModifier = core::abilityModifier(dexterite),
                                  .movement = 6};
    return {.profile = profil,
            .kit = {.armorClass = ca,
                    .attackBonus = bonus,
                    .damage = *core::parseDice(degats),
                    .label = nom},
            .position = std::nullopt,
            .markId = {}};
}

core::ArenaBout escarmouche(std::uint64_t graine, bool letale) {
    core::ArenaBout bout{.seed = graine, .lethal = letale, .heroicMark = true};
    bout.contestants.push_back(concurrent("Guerriere", CombatSide::Allies, 20, 12, 5, "1d8+3", 16));
    bout.contestants.push_back(concurrent("Pretre", CombatSide::Allies, 14, 10, 4, "1d6+2", 15));
    for (int rang = 0; rang < 3; ++rang) {
        bout.contestants.push_back(
            concurrent("Gobelin", CombatSide::Enemies, 7, 14, 4, "1d6+2", 15));
    }
    return bout;
}

int chebyshev(core::GridPosition a, core::GridPosition b) {
    return std::max(std::abs(a.column - b.column), std::abs(a.row - b.row));
}

/// Joue un affrontement par la tactique élémentaire du `LOT-20` : marcher vers l'ennemi debout le
/// plus proche, frapper au contact, terminer son tour. @return Le journal de la session.
std::vector<std::string> jouer(core::ArenaSession& session) {
    core::CombatState& combat = session.combat();
    for (int garde = 0; garde < 500 && combat.phase() != core::CombatPhase::Ended; ++garde) {
        const CombatantId actif = *combat.activeCombatant();
        const CombatSide camp = combat.find(actif)->profile.side;
        const core::GridPosition depart = *combat.grid().positionOf(actif);
        std::optional<CombatantId> cible;
        int distance = 0;
        for (const CombatantId autre : combat.combatants()) {
            const core::Combatant* c = combat.find(autre);
            if (c->profile.side == camp || c->status != core::CombatantStatus::Standing) {
                continue;
            }
            const int d = chebyshev(depart, *combat.grid().positionOf(autre));
            if (!cible.has_value() || d < distance) {
                cible = autre;
                distance = d;
            }
        }
        const core::GridPosition visee = *combat.grid().positionOf(*cible);
        if (distance > 1) {
            std::optional<core::GridPosition> meilleure;
            for (const core::GridPosition destination : combat.reachableArea()->destinations()) {
                if (chebyshev(destination, visee) < distance) {
                    distance = chebyshev(destination, visee);
                    meilleure = destination;
                }
            }
            if (meilleure.has_value()) {
                EXPECT_EQ(session.move(*meilleure).result, core::MoveResult::Moved);
            }
        }
        if (distance == 1) {
            const core::StrikeOutcome coup = session.strike(*cible);
            EXPECT_TRUE(coup.result == core::StrikeResult::Hit ||
                        coup.result == core::StrikeResult::Missed);
        }
        if (combat.phase() != core::CombatPhase::Ended) {
            EXPECT_TRUE(session.endTurn());
        }
    }
    return session.journal();
}

}  // namespace

/**
 * @brief Les points d'entree d'une carte se lisent de ses entites, ranges par camp et par rang.
 * \castest{<b>Les points d'entree de l'arene se lisent de la carte, ranges par camp puis par
 * rang.</b><br/>
 * \tcat Unitaire · Combat<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Une piste avec trois entrees par camp, declarees dans le desordre, et une entree
 * sans camp.<br/>2. Lire les points d'entree.<br/>
 * \tattendu Six points, allies d'abord, par rang croissant ; l'entree sans camp est ignoree.
 * }
 */
TEST(ArenaTest, LesPointsDEntreeSeLisentDeLaCarte) {
    const std::vector<core::ArenaEntryPoint> entrees = core::arenaEntryPoints(piste());
    ASSERT_EQ(entrees.size(), 6U);
    const std::vector<core::ArenaEntryPoint> attendu{
        {CombatSide::Allies, 1, {2, 3}},  {CombatSide::Allies, 2, {2, 2}},
        {CombatSide::Allies, 3, {2, 4}},  {CombatSide::Enemies, 1, {9, 3}},
        {CombatSide::Enemies, 2, {9, 2}}, {CombatSide::Enemies, 3, {9, 4}}};
    EXPECT_EQ(entrees, attendu);
}

/**
 * @brief La premiere carte livree se charge, et les catalogues de l'arene avec elle.
 * \castest{<b>La premiere carte de Source/Elements/Levels se charge et accueille les deux camps ;
 * les arenes et les huit Marques Heroiques se chargent.</b><br/>
 * \tcat Unitaire · Combat<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Charger le catalogue des arenes livre et les Marques Heroiques.<br/>2. Charger la
 * carte que la premiere arene nomme.<br/>3. Lire ses points d'entree et verifier qu'aucun n'est
 * dans un mur.<br/>
 * \tattendu Aucune erreur de chargement ; une arene jouable non letale a Marque Heroique ; huit
 * marques ; au moins quatre entrees libres par camp.
 * }
 */
TEST(ArenaTest, LaPremiereCarteSeChargeEtAccueilleLesDeuxCamps) {
    const core::ArenaCatalog arenes =
        core::loadArenas(std::filesystem::path(JADG_WORLD_DIR) / "arena");
    EXPECT_TRUE(arenes.errors.empty());
    const core::HeroicMarkCatalog marques =
        core::loadHeroicMarks(std::filesystem::path(JADG_RPG_RULES_DIR) / "heroic-marks.json");
    EXPECT_TRUE(marques.errors.empty());
    EXPECT_EQ(marques.marks.size(), 8U);
    EXPECT_NE(marques.find("tank"), nullptr);
    EXPECT_EQ(marques.find("paladin"), nullptr);

    const core::Arena* jouable = nullptr;
    for (const core::Arena& arene : arenes.arenas) {
        if (!arene.map.empty()) {
            jouable = &arene;
            break;
        }
    }
    ASSERT_NE(jouable, nullptr);
    EXPECT_FALSE(jouable->lethal);
    EXPECT_TRUE(jouable->heroicMark);
    EXPECT_EQ(arenes.find(jouable->id), jouable);

    const core::LevelLoadResult carte =
        core::LevelLoader::loadFromFile(std::filesystem::path(JADG_LEVELS_DIR) / jouable->map);
    ASSERT_TRUE(carte.ok()) << carte.error;
    const core::BattleGrid grille(*carte.level);
    int allies = 0;
    int ennemis = 0;
    for (const core::ArenaEntryPoint& entree : core::arenaEntryPoints(*carte.level)) {
        EXPECT_FALSE(grille.isObstructed(entree.position, core::Locomotion::Walk));
        (entree.side == CombatSide::Allies ? allies : ennemis) += 1;
    }
    EXPECT_GE(allies, 4);
    EXPECT_GE(ennemis, 4);
}

/**
 * @brief Le kit d'un coup d'essai se tire du bestiaire ou de la fiche.
 * \castest{<b>Le kit du coup d'essai vient de la premiere action qui frappe d'une creature, ou du
 * coup a mains nues d'un personnage.</b><br/>
 * \tcat Unitaire · Combat<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Une creature dont la premiere action ne frappe pas et la seconde frappe.<br/>2. Une
 * creature sans attaque.<br/>3. Une fiche de Force 16, maitrise +2, CA 14.<br/>
 * \tattendu La seconde action donne le kit ; la creature sans attaque a un kit sans degats ; la
 * fiche frappe a +5 pour 1 + 3.
 * }
 */
TEST(ArenaTest, LesKitsSeTirentDuBestiaireEtDeLaFiche) {
    core::Creature gobelin;
    gobelin.armorClass = 15;
    gobelin.actions.push_back({.name = "Fuite agile", .text = "..."});
    gobelin.actions.push_back({.name = "Cimeterre",
                               .text = "...",
                               .attackBonus = 4,
                               .reach = 1.5F,
                               .damage = core::parseDice("1d6+2")});
    const core::StrikeKit kit = core::strikeKitFor(gobelin);
    EXPECT_EQ(kit.armorClass, 15);
    EXPECT_EQ(kit.attackBonus, 4);
    EXPECT_EQ(kit.damage, *core::parseDice("1d6+2"));
    EXPECT_EQ(kit.label, "Cimeterre");

    core::Creature nuee;
    nuee.armorClass = 12;
    const core::StrikeKit sansCoup = core::strikeKitFor(nuee);
    EXPECT_EQ(sansCoup.attackBonus, 0);
    EXPECT_EQ(sansCoup.damage.maximum(), 0);

    core::CharacterSheet fiche;
    fiche.name = "Brenna";
    fiche.armorClass = 14;
    fiche.abilities[static_cast<std::size_t>(core::Ability::Strength)] = 16;
    const core::StrikeKit mains = core::strikeKitFor(fiche, 2);
    EXPECT_EQ(mains.armorClass, 14);
    EXPECT_EQ(mains.attackBonus, 5);
    EXPECT_EQ(mains.damage.minimum(), 4);
    EXPECT_EQ(mains.damage.maximum(), 4);
}

/**
 * @brief Le montage pose chacun a son entree, et refuse en le disant.
 * \castest{<b>Le montage place chaque combattant au prochain point d'entree libre de son camp, ou
 * a la case demandee, et nomme chaque refus.</b><br/>
 * \tcat Unitaire · Combat<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Quatre allies sur trois entrees, un ennemi a une case demandee dans le pilier, un
 * ennemi libre.<br/>2. Monter avec la Marque Heroique, puis sans.<br/>
 * \tattendu Trois allies places dans l'ordre des rangs, le quatrieme refuse faute de place,
 * l'ennemi du pilier refuse comme obstrue ; la troisieme economie d'action n'existe qu'avec la
 * Marque.
 * }
 */
TEST(ArenaTest, LeMontagePlaceAuxEntreesEtRefuseEnLeDisant) {
    core::ArenaBout bout{.seed = 1, .lethal = false, .heroicMark = true};
    for (int rang = 0; rang < 4; ++rang) {
        bout.contestants.push_back(
            concurrent("Allie" + std::to_string(rang), CombatSide::Allies, 10, 10, 2, "1d4", 12));
    }
    core::ArenaContestant pilier = concurrent("Golem", CombatSide::Enemies, 30, 8, 5, "2d8+4", 17);
    pilier.position = core::GridPosition{6, 3};
    bout.contestants.push_back(pilier);
    bout.contestants.push_back(concurrent("Loup", CombatSide::Enemies, 11, 15, 4, "2d4+2", 13));

    core::ArenaSession session(piste());
    const core::ArenaMount montage = session.mount(bout);
    EXPECT_EQ(montage.allies,
              (std::vector<CombatantId>{CombatantId{1}, CombatantId{2}, CombatantId{3}}));
    EXPECT_EQ(montage.enemies, (std::vector<CombatantId>{CombatantId{4}}));
    ASSERT_EQ(montage.refusals.size(), 2U);
    EXPECT_EQ(montage.refusals[0].who, "Allie3");
    EXPECT_EQ(montage.refusals[0].placement, core::PlacementResult::OutOfBounds);
    EXPECT_EQ(montage.refusals[1].who, "Golem");
    EXPECT_EQ(montage.refusals[1].placement, core::PlacementResult::Obstructed);
    EXPECT_EQ(session.combat().grid().positionOf(CombatantId{1}), (core::GridPosition{2, 3}));
    EXPECT_EQ(session.combat().grid().positionOf(CombatantId{2}), (core::GridPosition{2, 2}));
    EXPECT_EQ(session.combat().grid().positionOf(CombatantId{4}), (core::GridPosition{9, 3}));
    EXPECT_TRUE(session.combat().economy(CombatantId{1})->has(core::HEROIC_ACTION_RESOURCE));
    EXPECT_NE(session.kit(CombatantId{4}), nullptr);
    EXPECT_EQ(session.kit(CombatantId{9}), nullptr);

    bout.heroicMark = false;
    session.mount(bout);
    EXPECT_FALSE(session.combat().economy(CombatantId{1})->has(core::HEROIC_ACTION_RESOURCE));
}

/**
 * @brief Le coup d'essai se refuse hors tour, hors allonge et sans action, et porte au contact.
 * \castest{<b>Le coup d'essai refuse hors tour actif, hors allonge, contre un allie et sans action
 * ; au contact il jette le d20 contre la classe d'armure.</b><br/>
 * \tcat Unitaire · Combat<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Heroine en (2,3), compagnon en (2,2), gobelin en (3,3) a la CA 30, graine 3.<br/>2.
 * Frapper avant le debut ; frapper le compagnon ; frapper un identifiant inconnu.<br/>3. Frapper
 * le gobelin, puis frapper encore.<br/>
 * \tattendu NoActiveTurn, OutOfReach, InvalidTarget ; puis un jet contre 30 qui rate et est
 * restitue ; puis NoAction.
 * }
 */
TEST(ArenaTest, LeCoupDEssaiSeRefuseEtPorte) {
    core::ArenaBout bout{.seed = 3, .lethal = false, .heroicMark = true};
    core::ArenaContestant heroine =
        concurrent("Heroine", CombatSide::Allies, 20, 20, 5, "1d8+3", 16);
    heroine.position = core::GridPosition{2, 3};
    core::ArenaContestant compagnon =
        concurrent("Compagnon", CombatSide::Allies, 12, 10, 3, "1d6", 14);
    compagnon.position = core::GridPosition{2, 2};
    core::ArenaContestant gobelin =
        concurrent("Gobelin", CombatSide::Enemies, 7, 8, 4, "1d6+2", 30);
    gobelin.position = core::GridPosition{3, 3};
    bout.contestants = {heroine, compagnon, gobelin};

    core::ArenaSession session(piste());
    session.mount(bout);
    EXPECT_EQ(session.strike(CombatantId{3}).result, core::StrikeResult::NoActiveTurn);
    ASSERT_TRUE(session.start());
    // Les tours des autres passent sans rien faire jusqu'a celui de l'heroine : l'initiative est
    // un jet, et ce test ne parie pas sur ses des.
    while (session.combat().activeCombatant() != CombatantId{1}) {
        ASSERT_TRUE(session.endTurn());
    }
    EXPECT_EQ(session.strike(CombatantId{2}).result, core::StrikeResult::OutOfReach);
    EXPECT_EQ(session.strike(CombatantId{9}).result, core::StrikeResult::InvalidTarget);
    EXPECT_EQ(session.strike(CombatantId{1}).result, core::StrikeResult::InvalidTarget);

    const core::StrikeOutcome coup = session.strike(CombatantId{3});
    EXPECT_EQ(coup.result, core::StrikeResult::Missed);
    ASSERT_TRUE(coup.roll.has_value());
    EXPECT_EQ(coup.roll->target, 30);
    EXPECT_EQ(coup.roll->modifiers.size(), 1U);
    EXPECT_EQ(coup.roll->modifiers[0].value, 5);
    EXPECT_EQ(session.combat().find(CombatantId{3})->profile.currentHitPoints, 7);
    EXPECT_EQ(session.strike(CombatantId{3}).result, core::StrikeResult::NoAction);
    EXPECT_NE(std::ranges::find_if(
                  session.journal(),
                  [](const std::string& l) { return l.starts_with("attaque declaree"); }),
              session.journal().end());
}

/**
 * @brief Un affrontement se joue, se rejoue a l'identique, et personne n'y meurt.
 * \castest{<b>Un affrontement se joue jusqu'a son issue ; le rejeu a la meme graine donne le meme
 * journal ; a la fin, la Marque Heroique releve tout le monde, sauf dans une arene letale.</b><br/>
 * \tcat Unitaire · Combat<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Deux allies contre trois gobelins, graine 2026, jouer jusqu'a l'issue.<br/>2.
 * Rejouer par `replay`, puis a une autre graine.<br/>3. Meme affrontement dans une arene
 * letale.<br/>
 * \tattendu Une issue ; meme journal au rejeu, autre journal a une autre graine ; tous a leurs
 * points de vie maximaux et debout dans l'arene non letale ; au moins un a terre dans l'arene
 * letale.
 * }
 */
TEST(ArenaTest, UnAffrontementSeJoueSeRejoueEtPersonneNYMeurt) {
    core::ArenaSession session(piste());
    session.mount(escarmouche(2026, false));
    ASSERT_TRUE(session.start());
    const std::vector<std::string> journal = jouer(session);
    ASSERT_TRUE(session.outcome().has_value());
    EXPECT_GE(session.combat().round(), 2);
    EXPECT_EQ(std::count_if(journal.begin(), journal.end(),
                            [](const std::string& l) { return l.starts_with("issue"); }),
              1);
    EXPECT_EQ(journal.back(), "marque heroique : tous releves");
    for (const CombatantId id : session.combat().combatants()) {
        const core::Combatant* c = session.combat().find(id);
        EXPECT_EQ(c->status, core::CombatantStatus::Standing);
        EXPECT_EQ(c->profile.currentHitPoints, c->profile.maximumHitPoints);
    }

    const std::optional<core::CombatOutcome> issue = session.outcome();
    const core::ArenaMount remontage = session.replay();
    EXPECT_EQ(remontage.allies.size(), 2U);
    EXPECT_EQ(remontage.enemies.size(), 3U);
    EXPECT_FALSE(session.outcome().has_value());
    EXPECT_EQ(jouer(session), journal);
    EXPECT_EQ(session.outcome(), issue);

    core::ArenaSession autre(piste());
    autre.mount(escarmouche(7, false));
    ASSERT_TRUE(autre.start());
    EXPECT_NE(jouer(autre), journal);

    core::ArenaSession letale(piste());
    letale.mount(escarmouche(2026, true));
    ASSERT_TRUE(letale.start());
    const std::vector<std::string> journalLetal = jouer(letale);
    ASSERT_TRUE(letale.outcome().has_value());
    EXPECT_NE(journalLetal.back(), "marque heroique : tous releves");
    EXPECT_TRUE(std::ranges::any_of(letale.combat().combatants(), [&](CombatantId id) {
        return letale.combat().find(id)->status == core::CombatantStatus::Down;
    }));
}
