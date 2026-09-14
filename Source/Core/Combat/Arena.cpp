// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Combat/Arena.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <system_error>
#include <tuple>
#include <utility>
#include <variant>

#include "Core/Combat/BattleGrid.h"
#include "Core/Data/JsonDocument.h"
#include "Core/Rpg/Ability.h"

namespace core {
namespace {

// Les entrees de catalogue ne portent pas de champ `version` : meme convention que
// `loadEncounters` (LOT-18) et `loadEquipment` (LOT-34).
constexpr int SANS_GARDE_DE_VERSION = 0;

[[nodiscard]] std::string lireTexte(const nlohmann::json& objet, const char* champ) {
    const auto trouve = objet.find(champ);
    return (trouve != objet.end() && trouve->is_string()) ? trouve->get<std::string>()
                                                          : std::string{};
}

[[nodiscard]] bool lireBooleen(const nlohmann::json& objet, const char* champ, bool defaut) {
    const auto trouve = objet.find(champ);
    return (trouve != objet.end() && trouve->is_boolean()) ? trouve->get<bool>() : defaut;
}

[[nodiscard]] std::vector<std::filesystem::path> fichiersJson(const std::filesystem::path& dossier,
                                                              std::error_code& code) {
    std::vector<std::filesystem::path> fichiers;
    for (const auto& entree : std::filesystem::directory_iterator(dossier, code)) {
        if (entree.is_regular_file(code) && entree.path().extension() == ".json") {
            fichiers.push_back(entree.path());
        }
    }
    std::ranges::sort(fichiers);
    return fichiers;
}

[[nodiscard]] std::optional<CombatSide> campDepuis(const PropertyMap& proprietes) {
    const auto trouve = proprietes.find(std::string(ARENA_SIDE_PROPERTY));
    if (trouve == proprietes.end()) {
        return std::nullopt;
    }
    const std::string* texte = std::get_if<std::string>(&trouve->second);
    if (texte == nullptr) {
        return std::nullopt;
    }
    if (*texte == "allies") {
        return CombatSide::Allies;
    }
    if (*texte == "enemies") {
        return CombatSide::Enemies;
    }
    return std::nullopt;
}

[[nodiscard]] int rangDepuis(const PropertyMap& proprietes) {
    const auto trouve = proprietes.find(std::string(ARENA_RANK_PROPERTY));
    if (trouve == proprietes.end()) {
        return 0;
    }
    if (const std::int64_t* entier = std::get_if<std::int64_t>(&trouve->second)) {
        return static_cast<int>(*entier);
    }
    if (const double* reel = std::get_if<double>(&trouve->second)) {
        return static_cast<int>(*reel);
    }
    return 0;
}

[[nodiscard]] int chebyshev(GridPosition a, GridPosition b) noexcept {
    return std::max(std::abs(a.column - b.column), std::abs(a.row - b.row));
}

[[nodiscard]] std::string_view nomDuCrochet(CombatHook crochet) noexcept {
    switch (crochet) {
        case CombatHook::BeforeFirstTurn:
            return "avant le premier tour";
        case CombatHook::RoundStart:
            return "round";
        case CombatHook::InitiativeCount:
            return "repere";
        case CombatHook::TurnStart:
            return "debut du tour";
        case CombatHook::TurnEnd:
            return "fin du tour";
        case CombatHook::AttackDeclared:
            return "attaque declaree";
        case CombatHook::CombatantJoined:
            return "entree";
        case CombatHook::CombatantLeft:
            return "sortie";
        case CombatHook::CombatEnded:
            return "issue";
    }
    return "?";
}

[[nodiscard]] std::string_view nomDeLIssue(CombatOutcome issue) noexcept {
    switch (issue) {
        case CombatOutcome::Victory:
            return "victoire";
        case CombatOutcome::Flight:
            return "fuite";
        case CombatOutcome::Defeat:
            return "defaite";
    }
    return "?";
}

}  // namespace

// --- Points d'entree --------------------------------------------------------------------------

std::vector<ArenaEntryPoint> arenaEntryPoints(const Level& level) {
    std::vector<ArenaEntryPoint> entrees;
    for (const MapEntity& entite : level.entities()) {
        if (entite.type != ARENA_ENTRY_ENTITY_TYPE) {
            continue;
        }
        const std::optional<CombatSide> camp = campDepuis(entite.properties);
        if (!camp.has_value()) {
            continue;
        }
        entrees.push_back(
            {.side = *camp, .rank = rangDepuis(entite.properties), .position = entite.position});
    }
    std::ranges::sort(entrees, [](const ArenaEntryPoint& a, const ArenaEntryPoint& b) {
        return std::tuple(a.side, a.rank, a.position.row, a.position.column) <
               std::tuple(b.side, b.rank, b.position.row, b.position.column);
    });
    return entrees;
}

// --- Catalogues -------------------------------------------------------------------------------

const HeroicMark* HeroicMarkCatalog::find(std::string_view id) const {
    const auto trouve = std::ranges::find(marks, id, &HeroicMark::id);
    return trouve == marks.end() ? nullptr : &*trouve;
}

HeroicMarkCatalog loadHeroicMarks(const std::filesystem::path& file) {
    HeroicMarkCatalog catalogue;
    const JsonDocument document = readJsonObjectFromFile(file, SANS_GARDE_DE_VERSION);
    if (!document.ok()) {
        catalogue.errors.push_back(document.message);
        return catalogue;
    }
    const auto marques = document.root.find("marks");
    if (marques == document.root.end() || !marques->is_array()) {
        catalogue.errors.push_back(file.filename().string() + " : aucune liste « marks ».");
        return catalogue;
    }
    for (const nlohmann::json& entree : *marques) {
        if (!entree.is_object()) {
            continue;
        }
        HeroicMark marque{.id = lireTexte(entree, "id"),
                          .name = lireTexte(entree, "name"),
                          .text = lireTexte(entree, "text")};
        if (marque.id.empty()) {
            catalogue.errors.push_back(file.filename().string() + " : marque sans identifiant.");
            continue;
        }
        catalogue.marks.push_back(std::move(marque));
    }
    return catalogue;
}

const Arena* ArenaCatalog::find(std::string_view id) const {
    const auto trouve = std::ranges::find(arenas, id, &Arena::id);
    return trouve == arenas.end() ? nullptr : &*trouve;
}

ArenaCatalog loadArenas(const std::filesystem::path& directory) {
    ArenaCatalog catalogue;
    std::error_code code;
    if (!std::filesystem::is_directory(directory, code)) {
        catalogue.errors.push_back(directory.string() + " : dossier absent ou illisible.");
        return catalogue;
    }
    for (const std::filesystem::path& chemin : fichiersJson(directory, code)) {
        const JsonDocument document = readJsonObjectFromFile(chemin, SANS_GARDE_DE_VERSION);
        if (!document.ok()) {
            catalogue.errors.push_back(document.message);
            continue;
        }
        const nlohmann::json& racine = document.root;
        Arena arene{.id = lireTexte(racine, "id"),
                    .name = lireTexte(racine, "name"),
                    .source = lireTexte(racine, "source"),
                    .region = lireTexte(racine, "region"),
                    .map = lireTexte(racine, "map"),
                    .lethal = lireBooleen(racine, "lethal", false),
                    .heroicMark = lireBooleen(racine, "heroicMark", true)};
        if (arene.id.empty()) {
            catalogue.errors.push_back(chemin.filename().string() + " : arene sans identifiant.");
            continue;
        }
        catalogue.arenas.push_back(std::move(arene));
    }
    return catalogue;
}

// --- Kits -------------------------------------------------------------------------------------

StrikeKit strikeKitFor(const Creature& creature) {
    StrikeKit kit{.armorClass = creature.armorClass, .attackBonus = 0, .damage = {}, .label = {}};
    for (const CreatureAction& action : creature.actions) {
        if (action.attackBonus.has_value() && action.damage.has_value()) {
            kit.attackBonus = *action.attackBonus;
            kit.damage = *action.damage;
            kit.label = action.name;
            return kit;
        }
    }
    // Une creature sans action qui frappe (un familier, une nuee) se defend sans attaquer : un
    // coup a zero de bonus et sans degats, qui ne touche donc jamais rien. Le journal le dit.
    kit.label = "aucune attaque";
    return kit;
}

StrikeKit strikeKitFor(const CharacterSheet& sheet, int proficiencyBonus) {
    const int force = sheet.modifier(Ability::Strength);
    return {.armorClass = sheet.armorClass,
            .attackBonus = proficiencyBonus + force,
            .damage = Dice{.count = 0, .faces = 0, .modifier = 1 + force},
            .label = "coup a mains nues"};
}

// --- Session ----------------------------------------------------------------------------------

ArenaSession::ArenaSession(Level level)
    : _level(std::move(level)), _combat(std::make_unique<CombatState>(BattleGrid(_level))) {}

void ArenaSession::record(std::string line) {
    _journal.push_back(std::move(line));
}

void ArenaSession::subscribe() {
    const auto nommer = [this](std::optional<CombatantId> id) -> std::string {
        if (!id.has_value()) {
            return "-";
        }
        const Combatant* c = _combat->find(*id);
        return (c == nullptr ? std::string("?") : c->profile.name) + " #" +
               std::to_string(static_cast<std::uint32_t>(*id));
    };
    constexpr std::array<CombatHook, 9> CROCHETS{
        CombatHook::BeforeFirstTurn, CombatHook::RoundStart,    CombatHook::InitiativeCount,
        CombatHook::TurnStart,       CombatHook::TurnEnd,       CombatHook::AttackDeclared,
        CombatHook::CombatantJoined, CombatHook::CombatantLeft, CombatHook::CombatEnded};
    for (const CombatHook crochet : CROCHETS) {
        _combat->subscribe(crochet, [this, nommer](CombatState& etat, const CombatEvent& e) {
            std::string ligne = std::string(nomDuCrochet(e.hook));
            switch (e.hook) {
                case CombatHook::RoundStart:
                    ligne += " " + std::to_string(e.round);
                    break;
                case CombatHook::InitiativeCount:
                    ligne += " " + e.marker;
                    break;
                case CombatHook::TurnStart:
                case CombatHook::TurnEnd:
                case CombatHook::CombatantJoined:
                case CombatHook::CombatantLeft:
                    ligne += " " + nommer(e.combatant);
                    break;
                case CombatHook::AttackDeclared:
                    ligne += " " + nommer(e.combatant) + " -> " + nommer(e.target);
                    break;
                case CombatHook::CombatEnded:
                    ligne += " : ";
                    ligne += etat.outcome().has_value() ? nomDeLIssue(*etat.outcome()) : "?";
                    break;
                case CombatHook::BeforeFirstTurn:
                    break;
            }
            record(std::move(ligne));
            if (e.hook == CombatHook::CombatEnded) {
                restoreAll();
            }
        });
    }
}

void ArenaSession::restoreAll() {
    if (_bout.lethal) {
        return;
    }
    // La Marque Heroique releve tout le monde, y compris ceux qui sont tombes : personne ne meurt
    // dans une arene, et un affrontement se rejoue autant de fois qu'on veut.
    for (const CombatantId id : _combat->combatants()) {
        const Combatant* c = _combat->find(id);
        if (c != nullptr && c->status != CombatantStatus::Withdrawn) {
            _combat->heal(id, c->profile.maximumHitPoints - c->profile.currentHitPoints);
        }
    }
    record("marque heroique : tous releves");
}

ArenaMount ArenaSession::mount(const ArenaBout& bout) {
    _bout = bout;
    _random = DeterministicRandom(bout.seed);
    _combat = std::make_unique<CombatState>(BattleGrid(_level));
    _kits.clear();
    _journal.clear();
    subscribe();
    _combat->setEscapable(true);

    ArenaMount montage;
    std::vector<ArenaEntryPoint> entrees = arenaEntryPoints(_level);
    const auto prochaineEntree = [&](CombatSide camp) -> std::optional<GridPosition> {
        for (auto it = entrees.begin(); it != entrees.end(); ++it) {
            if (it->side == camp) {
                const GridPosition position = it->position;
                entrees.erase(it);
                return position;
            }
        }
        return std::nullopt;
    };

    for (const ArenaContestant& concurrent : bout.contestants) {
        const std::optional<GridPosition> place = concurrent.position.has_value()
                                                      ? concurrent.position
                                                      : prochaineEntree(concurrent.profile.side);
        if (!place.has_value()) {
            montage.refusals.push_back({.who = concurrent.profile.name,
                                        .position = {},
                                        .placement = PlacementResult::OutOfBounds});
            continue;
        }
        const EnlistResult enrolement = _combat->enlist(concurrent.profile, *place);
        if (!enrolement.combatant.has_value()) {
            montage.refusals.push_back({.who = concurrent.profile.name,
                                        .position = *place,
                                        .placement = enrolement.placement});
            continue;
        }
        const CombatantId id = *enrolement.combatant;
        _kits[id] = concurrent.kit;
        if (bout.heroicMark) {
            _combat->economy(id)->declare(HEROIC_ACTION_RESOURCE, 1);
        }
        (concurrent.profile.side == CombatSide::Allies ? montage.allies : montage.enemies)
            .push_back(id);
    }
    record("montage : " + std::to_string(montage.allies.size()) + " allies, " +
           std::to_string(montage.enemies.size()) + " ennemis, " +
           std::to_string(montage.refusals.size()) + " refus");
    return montage;
}

bool ArenaSession::start() {
    if (!_combat->start(_random)) {
        return false;
    }
    for (const InitiativeEntry& place : _combat->turnOrder().entries()) {
        const Combatant* c = _combat->find(place.combatant);
        record("initiative " + (c == nullptr ? std::string("?") : c->profile.name) + " #" +
               std::to_string(static_cast<std::uint32_t>(place.combatant)) + " = " +
               std::to_string(place.total));
    }
    return true;
}

ArenaMount ArenaSession::replay() {
    ArenaMount montage = mount(_bout);
    start();
    return montage;
}

const StrikeKit* ArenaSession::kit(CombatantId combatant) const {
    const auto trouve = _kits.find(combatant);
    return trouve == _kits.end() ? nullptr : &trouve->second;
}

StrikeOutcome ArenaSession::strike(CombatantId target) {
    const std::optional<CombatantId> actif = _combat->activeCombatant();
    if (!actif.has_value() || _combat->phase() != CombatPhase::TurnActive) {
        return {.result = StrikeResult::NoActiveTurn};
    }
    const Combatant* attaquant = _combat->find(*actif);
    const Combatant* cible = _combat->find(target);
    const StrikeKit* kitAttaquant = kit(*actif);
    const StrikeKit* kitCible = kit(target);
    if (cible == nullptr || kitAttaquant == nullptr || kitCible == nullptr || target == *actif) {
        return {.result = StrikeResult::InvalidTarget};
    }
    const std::optional<GridPosition> de = _combat->grid().positionOf(*actif);
    const std::optional<GridPosition> vers = _combat->grid().positionOf(target);
    if (cible->profile.side == attaquant->profile.side ||
        cible->status != CombatantStatus::Standing || !de.has_value() || !vers.has_value() ||
        chebyshev(*de, *vers) > 1) {
        return {.result = StrikeResult::OutOfReach};
    }
    if (attaquant->economy.remaining(ACTION_RESOURCE) <= 0) {
        return {.result = StrikeResult::NoAction};
    }

    _combat->declareAttack(*actif, target);
    _combat->spend(ACTION_RESOURCE);
    const std::array<Modifier, 1> bonus{
        Modifier{.source = kitAttaquant->label, .value = kitAttaquant->attackBonus}};
    StrikeOutcome coup;
    coup.roll = rollCheck(kitCible->armorClass, bonus, RollStance::Normal, _random);
    if (!coup.roll->succeeded()) {
        coup.result = StrikeResult::Missed;
        record("coup " + attaquant->profile.name + " -> " + cible->profile.name + " : " +
               coup.roll->describe() + " : rate");
        return coup;
    }
    const DiceRoll degats = rollDice(kitAttaquant->damage, _random);
    coup.result = StrikeResult::Hit;
    coup.damage = std::max(degats.total, 0);
    record("coup " + attaquant->profile.name + " -> " + cible->profile.name + " : " +
           coup.roll->describe() + " : touche, " + degats.describe());
    _combat->applyDamage(target, coup.damage);
    return coup;
}

MoveOutcome ArenaSession::move(GridPosition destination) {
    const std::optional<CombatantId> actif = _combat->activeCombatant();
    MoveOutcome deplacement = _combat->move(destination);
    if (deplacement.result == MoveResult::Moved && actif.has_value()) {
        record("pas " + _combat->find(*actif)->profile.name + " " +
               std::to_string(destination.column) + "," + std::to_string(destination.row) + " (" +
               std::to_string(deplacement.path.cost) + ")");
    }
    return deplacement;
}

bool ArenaSession::endTurn() {
    return _combat->endTurn();
}

WithdrawResult ArenaSession::withdraw() {
    const std::optional<CombatantId> actif = _combat->activeCombatant();
    if (!actif.has_value()) {
        return WithdrawResult::NotInCombat;
    }
    return _combat->withdraw(*actif);
}

std::optional<CombatOutcome> ArenaSession::outcome() const {
    return _combat->outcome();
}

}  // namespace core
