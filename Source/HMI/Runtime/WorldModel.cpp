// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Runtime/WorldModel.h"

#include <utility>
#include <variant>

#include "Core/Levels/Level.h"
#include "Core/Rpg/Dialogue.h"
#include "Core/World/WorldTravel.h"
#include "HMI/HmiLog.h"
#include "HMI/Platform/ExecutableDirectory.h"

namespace hmi {

namespace {

/// Durée d'une image des bandes de figurine, en secondes (`idle.anim.json`, `frameDuration`).
constexpr float FIGURE_FRAME_SECONDS = 0.15F;

/// @return La valeur texte d'une propriété d'entité, vide si elle n'en est pas une.
[[nodiscard]] std::string textOf(const core::MapEntity& entity, std::string_view key) {
    const auto found = entity.properties.find(std::string{key});
    if (found == entity.properties.end()) {
        return {};
    }
    const std::string* text = std::get_if<std::string>(&found->second);
    return text != nullptr ? *text : std::string{};
}

}  // namespace

WorldModel::WorldModel(QObject* parent) : QObject(parent) {
    _session = std::make_unique<core::ExplorationSession>(
        core::WorldTravel::directoryLoader(executableDirectory() / "Levels"));
    _clock.setInterval(STEP_MILLISECONDS);
    _clock.setTimerType(Qt::PreciseTimer);
    connect(&_clock, &QTimer::timeout, this, &WorldModel::step);
}

WorldModel::~WorldModel() = default;

bool WorldModel::startNewGame() {
    if (!_startMapOverride.isEmpty()) {
        return enterMap(_startMapOverride, _startArrivalOverride);
    }
    return enterMap(QString::fromLatin1(START_MAP), QString{});
}

void WorldModel::setStartOverride(const QString& mapId, const QString& arrival) {
    _startMapOverride = mapId;
    _startArrivalOverride = arrival;
}

bool WorldModel::enterMap(const QString& mapId, const QString& arrival) {
    const std::string carte = mapId.toStdString();
    if (!_session->start(carte, arrival.toStdString())) {
        // Un échec de chargement est récupérable (`EX-NFR-040`) : l'écran le dit et reste debout.
        _status = tr("La carte « %1 » ne s'ouvre pas.").arg(mapId);
        _clock.stop();
        ++_sceneRevision;
        emit changed();
        HMI_LOG_WARNING("Monde : la carte " + carte + " ne s'ouvre pas.");
        return false;
    }
    _status.clear();
    _elapsed = 0.0F;
    _move = {};
    _interact = false;
    reloadAppearance();
    ++_sceneRevision;
    _clock.start();
    emit changed();
    emit heroMoved();
    emit mapEntered(mapId);
    return true;
}

void WorldModel::reloadAppearance() {
    const core::Level* const carte = _session->map();
    if (carte == nullptr) {
        _appearance = PlaceAppearance{};
        return;
    }
    const std::string lieu = scenePlaceOf(*carte);
    if (lieu.empty()) {
        // Une carte qui ne nomme aucun lieu ne dessine aucune pièce : le dire vaut mieux que
        // couvrir la carte de damiers.
        _appearance = PlaceAppearance{};
        HMI_LOG_WARNING("Monde : la carte " + _session->mapId() +
                        " ne declare aucun lieu (propriete « scene » de couche).");
        return;
    }
    PlaceAppearanceResult lue = PlaceAppearance::loadFromFile(executableDirectory() / "Assets" /
                                                              "Scene" / lieu / "appearance.json");
    if (!lue.ok()) {
        HMI_LOG_WARNING("Monde : table d'apparence du lieu " + lieu + " illisible, " + lue.message);
        _appearance = PlaceAppearance{};
        return;
    }
    _appearance = std::move(lue.appearance);
}

void WorldModel::setMove(qreal x, qreal y) {
    _move = {static_cast<float>(x), static_cast<float>(y)};
}

void WorldModel::interact() {
    _interact = true;
}

void WorldModel::step() {
    if (_session->map() == nullptr) {
        return;
    }
    const float seconds = static_cast<float>(STEP_MILLISECONDS) / 1000.0F;
    const core::CellPoint avant = _session->heroPoint();
    const core::ExplorationIntent intention{.move = _move, .interact = _interact};
    _interact = false;

    const std::vector<core::ExplorationEvent> evenements = _session->update(intention, seconds);
    const bool marche = !_session->frozen() && (_move.x != 0.0F || _move.y != 0.0F);
    if (marche != _walking) {
        _walking = marche;
        ++_sceneRevision;
    }
    _elapsed += seconds;

    // Un heros qui pousse contre un mur ne change pas de case, mais sa bande continue de tourner :
    // la scene doit se redessiner autant que s'il avait bouge.
    if (_session->heroPoint() != avant || marche) {
        ++_sceneRevision;
        emit heroMoved();
    }

    for (const core::ExplorationEvent& evenement : evenements) {
        switch (evenement.kind) {
            case core::ExplorationEventKind::MapEntered:
                reloadAppearance();
                ++_sceneRevision;
                emit changed();
                emit heroMoved();
                emit mapEntered(QString::fromStdString(evenement.value));
                break;
            case core::ExplorationEventKind::Dialogue:
                emit dialogueRequested(QString::fromStdString(evenement.value));
                break;
            case core::ExplorationEventKind::Encounter:
                emit encounterRequested(QString::fromStdString(evenement.value));
                break;
            case core::ExplorationEventKind::PortalLocked:
                emit portalLocked(QString::fromStdString(evenement.value));
                break;
            case core::ExplorationEventKind::PortalBroken:
                emit portalBroken(QString::fromStdString(evenement.value));
                break;
            case core::ExplorationEventKind::Interacted:
                emit changed();
                break;
        }
    }
}

QString WorldModel::mapId() const {
    return QString::fromStdString(_session->mapId());
}

QString WorldModel::mapName() const {
    const core::Level* const carte = _session->map();
    return carte != nullptr ? QString::fromStdString(carte->name()) : QString{};
}

bool WorldModel::loaded() const {
    return _session->map() != nullptr;
}

int WorldModel::columns() const {
    const core::Level* const carte = _session->map();
    return carte != nullptr ? carte->tileMap().width() : 0;
}

int WorldModel::rows() const {
    const core::Level* const carte = _session->map();
    return carte != nullptr ? carte->tileMap().height() : 0;
}

qreal WorldModel::heroColumn() const {
    return _session->heroPoint().column;
}

qreal WorldModel::heroRow() const {
    return _session->heroPoint().row;
}

void WorldModel::setHeroFigure(const QString& figure) {
    if (_heroFigure == figure) {
        return;
    }
    _heroFigure = figure;
    ++_sceneRevision;
    emit changed();
}

bool WorldModel::frozen() const {
    return _session->frozen();
}

void WorldModel::setFrozen(bool frozen) {
    if (_session->frozen() == frozen) {
        return;
    }
    _session->freeze(frozen);
    emit changed();
}

std::vector<WorldFigureSnapshot> WorldModel::figures() const {
    std::vector<WorldFigureSnapshot> figurines;
    const core::Level* const carte = _session->map();
    if (carte == nullptr) {
        return figurines;
    }
    const int image = static_cast<int>(_elapsed / FIGURE_FRAME_SECONDS);

    // Les PNJ d'abord, le héros ensuite : à égalité de profondeur, c'est lui qui passe devant.
    for (const core::MapEntity& objet : carte->entities()) {
        if (objet.type != core::NPC_ENTITY_TYPE) {
            continue;
        }
        std::string figurine = textOf(objet, core::NPC_FIGURE_PROPERTY);
        if (figurine.empty()) {
            continue;  // Un PNJ sans figurine ne se dessine pas : il n'est pas encore dessiné.
        }
        figurines.push_back(
            WorldFigureSnapshot{.figure = std::move(figurine),
                                .clip = "idle",
                                .point = {static_cast<float>(objet.position.column) + 0.5F,
                                          static_cast<float>(objet.position.row) + 0.5F},
                                .frame = image});
    }
    figurines.push_back(
        WorldFigureSnapshot{.figure = _heroFigure.toStdString(),
                            .clip = _walking ? "walk" : "idle",
                            .point = {_session->heroPoint().column, _session->heroPoint().row},
                            .frame = image});
    return figurines;
}

WorldSceneSnapshot WorldModel::snapshot() const {
    const core::Level* const carte = _session->map();
    if (carte == nullptr) {
        return WorldSceneSnapshot{};
    }
    return snapshotWorldScene(*carte, _appearance, figures());
}

}  // namespace hmi
