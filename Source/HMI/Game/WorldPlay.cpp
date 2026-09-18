// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Game/WorldPlay.h"

#include <utility>
#include <variant>

#include "Core/Levels/Level.h"
#include "Core/Levels/MapEntity.h"
#include "Core/Rpg/Dialogue.h"
#include "HMI/HmiLog.h"

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

WorldPlay::WorldPlay(core::WorldTravel::MapLoader loader, std::filesystem::path assetsDirectory)
    : _session(std::move(loader)), _assetsDirectory(std::move(assetsDirectory)) {}

bool WorldPlay::enter(std::string_view mapId, std::string_view arrival) {
    if (!_session.start(mapId, arrival)) {
        return false;
    }
    _elapsed = 0.0F;
    _walking = false;
    reloadAppearance();
    return true;
}

WorldPlayStep WorldPlay::step(const core::ExplorationIntent& intent, float seconds) {
    WorldPlayStep result;
    if (_session.map() == nullptr) {
        return result;
    }
    const core::CellPoint before = _session.heroPoint();
    result.events = _session.update(intent, seconds);
    _elapsed += seconds;

    const bool walking = !_session.frozen() && (intent.move.x != 0.0F || intent.move.y != 0.0F);
    if (walking != _walking) {
        _walking = walking;
        result.sceneChanged = true;
    }
    // Un héros qui pousse contre un mur ne change pas de case, mais sa bande continue de tourner :
    // la scène doit se redessiner autant que s'il avait bougé.
    result.heroMoved = _session.heroPoint() != before || walking;
    for (const core::ExplorationEvent& event : result.events) {
        if (event.kind == core::ExplorationEventKind::MapEntered) {
            _elapsed = 0.0F;
            reloadAppearance();
            result.sceneChanged = true;
            result.heroMoved = true;
        }
    }
    return result;
}

void WorldPlay::reloadAppearance() {
    const core::Level* const map = _session.map();
    if (map == nullptr) {
        _appearance = PlaceAppearance{};
        return;
    }
    const std::string place = scenePlaceOf(*map);
    if (place.empty()) {
        // Une carte qui ne nomme aucun lieu ne dessine aucune pièce : le dire vaut mieux que
        // couvrir la carte de damiers.
        _appearance = PlaceAppearance{};
        HMI_LOG_WARNING("Monde : la carte " + _session.mapId() +
                        " ne declare aucun lieu (propriete « scene » de couche).");
        return;
    }
    PlaceAppearanceResult read =
        PlaceAppearance::loadFromFile(_assetsDirectory / "Scene" / place / "appearance.json");
    if (!read.ok()) {
        HMI_LOG_WARNING("Monde : table d'apparence du lieu " + place + " illisible, " +
                        read.message);
        _appearance = PlaceAppearance{};
        return;
    }
    _appearance = std::move(read.appearance);
}

std::vector<WorldFigureSnapshot> WorldPlay::figures() const {
    std::vector<WorldFigureSnapshot> figures;
    const core::Level* const map = _session.map();
    if (map == nullptr) {
        return figures;
    }
    const int frame = static_cast<int>(_elapsed / FIGURE_FRAME_SECONDS);

    // Les PNJ d'abord, le héros ensuite : à égalité de profondeur, c'est lui qui passe devant.
    for (const core::MapEntity& entity : map->entities()) {
        if (entity.type != core::NPC_ENTITY_TYPE) {
            continue;
        }
        std::string figure = textOf(entity, core::NPC_FIGURE_PROPERTY);
        if (figure.empty()) {
            continue;  // Un PNJ sans figurine ne se dessine pas : il n'est pas encore dessiné.
        }
        figures.push_back(
            WorldFigureSnapshot{.figure = std::move(figure),
                                .clip = "idle",
                                .point = {static_cast<float>(entity.position.column) + 0.5F,
                                          static_cast<float>(entity.position.row) + 0.5F},
                                .frame = frame});
    }
    figures.push_back(WorldFigureSnapshot{.figure = _heroFigure,
                                          .clip = _walking ? "walk" : "idle",
                                          .point = {_session.heroPoint().column,
                                                    _session.heroPoint().row},
                                          .frame = frame});
    return figures;
}

WorldSceneSnapshot WorldPlay::snapshot() const {
    const core::Level* const map = _session.map();
    if (map == nullptr) {
        return WorldSceneSnapshot{};
    }
    return snapshotWorldScene(*map, _appearance, figures());
}

}  // namespace hmi
