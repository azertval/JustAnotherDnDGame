// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Runtime/ArenaModel.h"

#include <QVariantMap>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <utility>

#include "Core/Combat/BattleGrid.h"
#include "Core/Levels/LevelLoader.h"
#include "HMI/HmiLog.h"
#include "HMI/Platform/ExecutableDirectory.h"
#include "HMI/Runtime/DemonstrationCharacter.h"

namespace hmi {

/// Un combattant composé : ce que l'écran a choisi, prêt à devenir un `core::ArenaContestant`.
struct ArenaModel::Fighter {
    QString id;
    core::ArenaContestant contestant;
};

/// Les catalogues chargés une fois, à la construction : le bestiaire, le personnage de
/// démonstration et son bonus de maîtrise, les arènes, les Marques, et la carte de l'arène jouable.
struct ArenaModel::Catalogs {
    core::Bestiary bestiary;
    std::optional<core::CharacterSheet> character;
    int characterProficiency = 2;
    core::ArenaCatalog arenas;
    core::HeroicMarkCatalog marks;
    const core::Arena* playable = nullptr;
    std::optional<core::Level> level;
    QStringList problems;
};

namespace {

[[nodiscard]] QString toQt(const std::string& text) {
    return QString::fromStdString(text);
}

[[nodiscard]] QString sideName(core::CombatSide side) {
    return side == core::CombatSide::Allies ? QStringLiteral("allies") : QStringLiteral("enemies");
}

constexpr const char* CHARACTER_PREFIX = "character:";

[[nodiscard]] int chebyshev(core::GridPosition a, core::GridPosition b) noexcept {
    return std::max(std::abs(a.column - b.column), std::abs(a.row - b.row));
}

}  // namespace

ArenaModel::ArenaModel(QObject* parent) : QObject(parent), _catalogs(std::make_unique<Catalogs>()) {
    loadCatalogs();
}

ArenaModel::~ArenaModel() = default;

void ArenaModel::loadCatalogs() {
    const std::filesystem::path root = executableDirectory();
    Catalogs& c = *_catalogs;

    c.bestiary = core::loadBestiary(root / "Rpg" / "creatures");
    for (const std::string& error : c.bestiary.errors) {
        HMI_LOG_WARNING("Arene : bestiaire, " + error);
    }
    if (c.bestiary.creatures.empty()) {
        c.problems << QStringLiteral("bestiaire vide");
    }

    // Le personnage de démonstration, par le même chemin que la fiche : un seul chargement, une
    // seule vérité sur ce qu'il porte (LOT-87).
    const DemonstrationState demonstration = loadDemonstrationState();
    if (!demonstration.sheet.name.empty()) {
        c.character = demonstration.sheet;
        c.characterProficiency =
            core::proficiencyBonus(demonstration.sheet, demonstration.experience);
    } else {
        c.problems << QStringLiteral("personnage de demonstration absent");
    }

    c.arenas = core::loadArenas(root / "World" / "arena");
    for (const std::string& error : c.arenas.errors) {
        HMI_LOG_WARNING("Arene : catalogue, " + error);
    }
    c.marks = core::loadHeroicMarks(root / "Rpg" / "rules" / "heroic-marks.json");
    for (const std::string& error : c.marks.errors) {
        HMI_LOG_WARNING("Arene : marques heroiques, " + error);
    }

    for (const core::Arena& arena : c.arenas.arenas) {
        if (!arena.map.empty()) {
            c.playable = &arena;
            break;
        }
    }
    if (c.playable == nullptr) {
        c.problems << QStringLiteral("aucune arene n'a de carte");
        return;
    }
    const core::LevelLoadResult loaded =
        core::LevelLoader::loadFromFile(root / "Levels" / c.playable->map);
    if (!loaded.ok()) {
        HMI_LOG_WARNING("Arene : carte " + c.playable->map + ", " + loaded.error);
        c.problems << QStringLiteral("carte illisible : ") + toQt(loaded.error);
        return;
    }
    c.level = loaded.level;
    _session = std::make_unique<core::ArenaSession>(*c.level);
    _status = c.problems.join(QStringLiteral(" ; "));
}

// --- Lecture ----------------------------------------------------------------------------------

QString ArenaModel::arenaName() const {
    if (_catalogs->playable == nullptr) {
        return QStringLiteral("—");
    }
    return toQt(_catalogs->playable->name);
}

QString ArenaModel::status() const {
    return _status;
}

bool ArenaModel::inCombat() const noexcept {
    return _inCombat;
}

bool ArenaModel::ended() const {
    return _inCombat && _session != nullptr && _session->outcome().has_value();
}

QVariantList ArenaModel::roster() const {
    QVariantList entries;
    if (_catalogs->character.has_value()) {
        const core::CharacterSheet& sheet = *_catalogs->character;
        entries << QVariantMap{{"id", QString(CHARACTER_PREFIX) + toQt(sheet.name)},
                               {"name", toQt(sheet.name)},
                               {"kind", QStringLiteral("personnage")},
                               {"hitPoints", sheet.maximumHitPoints},
                               {"armorClass", sheet.armorClass}};
    }
    for (const core::Creature& creature : _catalogs->bestiary.creatures) {
        entries << QVariantMap{{"id", toQt(creature.id)},
                               {"name", toQt(creature.name)},
                               {"kind", QStringLiteral("creature")},
                               {"hitPoints", creature.hitPoints},
                               {"armorClass", creature.armorClass}};
    }
    return entries;
}

namespace {

[[nodiscard]] QVariantList describeFighters(const auto& fighters) {
    QVariantList list;
    for (const auto& fighter : fighters) {
        list << QVariantMap{{"id", fighter.id},
                            {"name", toQt(fighter.contestant.profile.name)},
                            {"mark", toQt(fighter.contestant.markId)}};
    }
    return list;
}

}  // namespace

QVariantList ArenaModel::allies() const {
    return describeFighters(_allies);
}

QVariantList ArenaModel::enemies() const {
    return describeFighters(_enemies);
}

QStringList ArenaModel::marks() const {
    QStringList ids;
    for (const core::HeroicMark& mark : _catalogs->marks.marks) {
        ids << toQt(mark.id);
    }
    return ids;
}

int ArenaModel::seed() const noexcept {
    return _seed;
}

void ArenaModel::setSeed(int seed) {
    if (seed == _seed) {
        return;
    }
    _seed = seed;
    emit changed();
}

int ArenaModel::gridColumns() const {
    return _catalogs->level.has_value() ? _catalogs->level->tileMap().width() : 0;
}

int ArenaModel::gridRows() const {
    return _catalogs->level.has_value() ? _catalogs->level->tileMap().height() : 0;
}

QVariantList ArenaModel::cells() const {
    QVariantList list;
    if (!_catalogs->level.has_value()) {
        return list;
    }
    const core::BattleGrid& grid = _session->combat().grid();
    const std::optional<core::CombatantId> active = _session->combat().activeCombatant();
    const std::optional<core::ReachableArea> area =
        _inCombat ? _session->combat().reachableArea() : std::nullopt;
    for (int row = 0; row < grid.height(); ++row) {
        for (int column = 0; column < grid.width(); ++column) {
            const core::GridPosition cell{.column = column, .row = row};
            QVariantMap entry{{"column", column},
                              {"row", row},
                              {"wall", grid.isObstructed(cell, core::Locomotion::Walk)},
                              {"occupant", QString()},
                              {"side", QString()},
                              {"reachable", area.has_value() && area->canEndAt(cell)},
                              {"active", false},
                              {"down", false},
                              {"hitPoints", QString()}};
            if (const std::optional<core::CombatantId> id = grid.occupantAt(cell)) {
                if (const core::Combatant* combatant = _session->combat().find(*id)) {
                    entry["occupant"] = toQt(combatant->profile.name);
                    entry["side"] = sideName(combatant->profile.side);
                    entry["active"] = active == *id;
                    entry["down"] = combatant->status == core::CombatantStatus::Down;
                    entry["hitPoints"] = QString::number(combatant->profile.currentHitPoints) +
                                         "/" + QString::number(combatant->profile.maximumHitPoints);
                }
            }
            list << entry;
        }
    }
    return list;
}

QVariantList ArenaModel::turnOrder() const {
    QVariantList list;
    if (_session == nullptr || !_inCombat) {
        return list;
    }
    const std::optional<core::CombatantId> active = _session->combat().activeCombatant();
    for (const core::InitiativeEntry& entry : _session->combat().turnOrder().entries()) {
        const core::Combatant* combatant = _session->combat().find(entry.combatant);
        if (combatant == nullptr) {
            continue;
        }
        list << QVariantMap{{"name", toQt(combatant->profile.name)},
                            {"total", entry.total},
                            {"side", sideName(entry.side)},
                            {"active", active == entry.combatant},
                            {"down", combatant->status == core::CombatantStatus::Down}};
    }
    return list;
}

QString ArenaModel::activeName() const {
    if (_session == nullptr || !_inCombat) {
        return {};
    }
    const std::optional<core::CombatantId> active = _session->combat().activeCombatant();
    if (!active.has_value()) {
        return {};
    }
    const core::Combatant* combatant = _session->combat().find(*active);
    return combatant == nullptr ? QString() : toQt(combatant->profile.name);
}

QString ArenaModel::activeResources() const {
    if (_session == nullptr || !_inCombat) {
        return {};
    }
    const std::optional<core::CombatantId> active = _session->combat().activeCombatant();
    if (!active.has_value()) {
        return {};
    }
    const core::Combatant* combatant = _session->combat().find(*active);
    if (combatant == nullptr) {
        return {};
    }
    QStringList parts;
    for (const core::ActionResource& resource : combatant->economy.resources()) {
        parts << toQt(resource.id) + " " + QString::number(resource.remaining);
    }
    return parts.join(QStringLiteral(" · "));
}

QStringList ArenaModel::journal() const {
    QStringList lines;
    if (_session == nullptr) {
        return lines;
    }
    for (const std::string& line : _session->journal()) {
        lines << toQt(line);
    }
    return lines;
}

// --- Composition ------------------------------------------------------------------------------

std::optional<ArenaModel::Fighter> ArenaModel::fighterFor(const QString& id,
                                                          core::CombatSide side) const {
    if (id.startsWith(CHARACTER_PREFIX)) {
        if (!_catalogs->character.has_value()) {
            return std::nullopt;
        }
        const core::CharacterSheet& sheet = *_catalogs->character;
        return Fighter{
            .id = id,
            .contestant = {.profile = core::profileFor(sheet, side),
                           .kit = core::strikeKitFor(sheet, _catalogs->characterProficiency),
                           .position = std::nullopt,
                           .markId = {}}};
    }
    const core::Creature* creature = _catalogs->bestiary.find(id.toStdString());
    if (creature == nullptr) {
        return std::nullopt;
    }
    return Fighter{.id = id,
                   .contestant = {.profile = core::profileFor(*creature, side),
                                  .kit = core::strikeKitFor(*creature),
                                  .position = std::nullopt,
                                  .markId = {}}};
}

void ArenaModel::addAlly(const QString& id) {
    if (_inCombat) {
        return;
    }
    if (std::optional<Fighter> fighter = fighterFor(id, core::CombatSide::Allies)) {
        _allies.push_back(std::move(*fighter));
        emit changed();
    }
}

void ArenaModel::addEnemy(const QString& id) {
    if (_inCombat) {
        return;
    }
    if (std::optional<Fighter> fighter = fighterFor(id, core::CombatSide::Enemies)) {
        _enemies.push_back(std::move(*fighter));
        emit changed();
    }
}

void ArenaModel::removeAlly(int index) {
    if (_inCombat || index < 0 || index >= static_cast<int>(_allies.size())) {
        return;
    }
    _allies.erase(_allies.begin() + index);
    emit changed();
}

void ArenaModel::removeEnemy(int index) {
    if (_inCombat || index < 0 || index >= static_cast<int>(_enemies.size())) {
        return;
    }
    _enemies.erase(_enemies.begin() + index);
    emit changed();
}

void ArenaModel::assignMark(bool ally, int index, const QString& markId) {
    std::vector<Fighter>& camp = ally ? _allies : _enemies;
    if (_inCombat || index < 0 || index >= static_cast<int>(camp.size())) {
        return;
    }
    const std::string id = markId.toStdString();
    camp[static_cast<std::size_t>(index)].contestant.markId =
        (id.empty() || _catalogs->marks.find(id) != nullptr) ? id : std::string{};
    emit changed();
}

core::ArenaBout ArenaModel::composeBout() const {
    core::ArenaBout bout{
        .seed = static_cast<std::uint64_t>(static_cast<unsigned>(_seed)),
        .lethal = _catalogs->playable != nullptr && _catalogs->playable->lethal,
        .heroicMark = _catalogs->playable == nullptr || _catalogs->playable->heroicMark};
    for (const Fighter& fighter : _allies) {
        bout.contestants.push_back(fighter.contestant);
    }
    for (const Fighter& fighter : _enemies) {
        bout.contestants.push_back(fighter.contestant);
    }
    return bout;
}

void ArenaModel::refreshMessage(const core::ArenaMount& mount) {
    QStringList lines;
    for (const core::MountRefusal& refusal : mount.refusals) {
        QString reason = QStringLiteral("inconnu du bestiaire");
        if (refusal.placement.has_value()) {
            switch (*refusal.placement) {
                case core::PlacementResult::Placed:
                    reason = QStringLiteral("place");
                    break;
                case core::PlacementResult::OutOfBounds:
                    reason = QStringLiteral("plus de point d'entree libre");
                    break;
                case core::PlacementResult::Obstructed:
                    reason = QStringLiteral("case obstruee");
                    break;
                case core::PlacementResult::Occupied:
                    reason = QStringLiteral("case occupee");
                    break;
                case core::PlacementResult::InvalidCombatant:
                    reason = QStringLiteral("combattant invalide");
                    break;
            }
        }
        lines << QStringLiteral("refuse : ") + toQt(refusal.who) + " (" + reason + ")";
    }
    _status = lines.join(QStringLiteral(" ; "));
}

// --- Le combat --------------------------------------------------------------------------------

void ArenaModel::launch() {
    if (_session == nullptr || _inCombat) {
        return;
    }
    if (_allies.empty() || _enemies.empty()) {
        _status = QStringLiteral("Il faut au moins un combattant dans chaque camp.");
        emit changed();
        return;
    }
    const core::ArenaMount mount = _session->mount(composeBout());
    refreshMessage(mount);
    if (mount.allies.empty() || mount.enemies.empty()) {
        _status += QStringLiteral(" Un camp est vide apres le montage : rien a lancer.");
        emit changed();
        return;
    }
    _inCombat = _session->start();
    emit changed();
}

void ArenaModel::tapCell(int column, int row) {
    if (_session == nullptr || !_inCombat || ended()) {
        return;
    }
    core::CombatState& combat = _session->combat();
    const core::GridPosition cell{.column = column, .row = row};
    const std::optional<core::CombatantId> active = combat.activeCombatant();
    if (!active.has_value()) {
        return;
    }
    if (const std::optional<core::CombatantId> target = combat.grid().occupantAt(cell)) {
        const core::Combatant* attacker = combat.find(*active);
        const core::Combatant* defender = combat.find(*target);
        const std::optional<core::GridPosition> from = combat.grid().positionOf(*active);
        if (attacker != nullptr && defender != nullptr && from.has_value() &&
            defender->profile.side != attacker->profile.side) {
            if (chebyshev(*from, cell) > 1) {
                _status = QStringLiteral("Hors d'allonge : le coup d'essai porte a une case.");
            } else {
                const core::StrikeOutcome strike = _session->strike(*target);
                switch (strike.result) {
                    case core::StrikeResult::Hit:
                    case core::StrikeResult::Missed:
                        _status = toQt(_session->journal().back());
                        break;
                    case core::StrikeResult::NoAction:
                        _status = QStringLiteral("L'action de ce tour est deja depensee.");
                        break;
                    case core::StrikeResult::NoActiveTurn:
                    case core::StrikeResult::OutOfReach:
                    case core::StrikeResult::InvalidTarget:
                        _status = QStringLiteral("Coup refuse.");
                        break;
                }
            }
            emit changed();
            return;
        }
    }
    const core::MoveOutcome move = _session->move(cell);
    switch (move.result) {
        case core::MoveResult::Moved:
            _status = QStringLiteral("Deplacement : ") + QString::number(move.path.cost) +
                      QStringLiteral(" case(s).");
            break;
        case core::MoveResult::Unreachable:
            _status = QStringLiteral("Case hors de portee de ce qui reste du deplacement.");
            break;
        case core::MoveResult::NoActiveTurn:
        case core::MoveResult::NotPlaced:
            _status = QStringLiteral("Aucun combattant a deplacer.");
            break;
    }
    emit changed();
}

void ArenaModel::endTurn() {
    if (_session == nullptr || !_inCombat) {
        return;
    }
    if (_session->endTurn()) {
        _status.clear();
    }
    if (const std::optional<core::CombatOutcome> outcome = _session->outcome()) {
        _status = toQt(_session->journal().back());
        static_cast<void>(outcome);
    }
    emit changed();
}

void ArenaModel::withdraw() {
    if (_session == nullptr || !_inCombat) {
        return;
    }
    switch (_session->withdraw()) {
        case core::WithdrawResult::Withdrawn:
            _status = QStringLiteral("Sorti de l'arene.");
            break;
        case core::WithdrawResult::NotEscapable:
            _status = QStringLiteral("On ne quitte pas cette arene.");
            break;
        case core::WithdrawResult::NotInCombat:
            _status = QStringLiteral("Personne a retirer.");
            break;
    }
    emit changed();
}

void ArenaModel::replay() {
    if (_session == nullptr || !_inCombat) {
        return;
    }
    const core::ArenaMount mount = _session->replay();
    refreshMessage(mount);
    _status = QStringLiteral("Rejeu a la graine ") + QString::number(_seed) +
              (_status.isEmpty() ? QString() : QStringLiteral(" ; ") + _status);
    emit changed();
}

void ArenaModel::backToSetup() {
    if (!_inCombat) {
        return;
    }
    _inCombat = false;
    _session = std::make_unique<core::ArenaSession>(*_catalogs->level);
    _status.clear();
    emit changed();
}

}  // namespace hmi
