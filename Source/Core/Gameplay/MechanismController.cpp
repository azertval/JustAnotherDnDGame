// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Gameplay/MechanismController.h"

#include <string>

#include "Core/Gameplay/GameplayLog.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/TileType.h"
#include "Core/Physics/Aabb.h"

namespace core {
namespace {

// La boîte recouvre-t-elle la case (column, row) ? Chevauchement d'aire strictement positive.
bool overlapsCell(const Aabb& box, GridPosition cell) {
    const auto left = static_cast<float>(cell.column);
    const auto top = static_cast<float>(cell.row);
    return box.min.x < left + 1.0F && box.max.x > left && box.min.y < top + 1.0F && box.max.y > top;
}

// Seuil de poids (masse, `core::Player::mass`) requis pour activer une plaque de pression
// (`EX-GP-025`) — calé sur la masse par défaut du personnage, pour qu'une plaque fonctionne
// « prête à l'emploi » sans configuration tant qu'aucune autre masse n'existe dans le jeu.
constexpr float MIN_TRIGGER_MASS = 1.0F;

// Un poids autre que le personnage repose-t-il sur @p cell avec une masse suffisante ? (LOT-65
// TACHE-06.) Un poids trop leger touche la plaque sans l'enfoncer, exactement comme un personnage
// trop leger -- meme seuil, meme regle, pas de cas particulier pour les blocs.
bool weightRestsOn(const std::vector<TriggerWeight>& weights, GridPosition cell) {
    for (const TriggerWeight& weight : weights) {
        if (weight.mass >= MIN_TRIGGER_MASS && overlapsCell(weight.box, cell)) {
            return true;
        }
    }
    return false;
}

}  // namespace

MechanismController::MechanismController(const Level& level)
    : _collision(level.tileMap()),  // copie de la carte : on la mutera pour les portes
      _mechanisms(level.mechanisms()),
      _switchOn(level.mechanisms().size(), false),
      _playerOnSwitchPrev(level.mechanisms().size(), false) {
    // Nature de chaque déclencheur, figée d'après la tuile d'origine (avant toute mutation de
    // _collision ci-dessous — seules les portes sont réécrites, jamais les déclencheurs eux-mêmes).
    // _openType capture le type "porte ouverte" (Door ou LockedDoor) AVANT que la boucle suivante
    // ne fige toutes les portes en Solid : lu après coup, il serait toujours Solid.
    _continuous.reserve(_mechanisms.size());
    _isKey.reserve(_mechanisms.size());
    _openType.reserve(_mechanisms.size());
    for (const Mechanism& mechanism : _mechanisms) {
        const TileType triggerType =
            _collision.tile(mechanism.switchPosition.column, mechanism.switchPosition.row);
        _continuous.push_back(triggerType == TileType::PressurePlate);
        _isKey.push_back(triggerType == TileType::Key);
        _openType.push_back(
            _collision.tile(mechanism.doorPosition.column, mechanism.doorPosition.row));
    }
    // Portes fermées au départ : solides dans la grille de collision.
    for (const Mechanism& mechanism : _mechanisms) {
        _collision.setTile(mechanism.doorPosition.column, mechanism.doorPosition.row,
                           TileType::Solid);
    }
}

void MechanismController::update(const Aabb& playerBox, float playerMass, bool interactPressed,
                                 const std::vector<TriggerWeight>& weights) {
    _crushedPlayer = false;  // etat d'un pas, jamais cumule d'un pas sur l'autre.
    for (std::size_t index = 0; index < _mechanisms.size(); ++index) {
        const Mechanism& mechanism = _mechanisms[index];
        const bool onSwitch = overlapsCell(playerBox, mechanism.switchPosition);
        const GridPosition door = mechanism.doorPosition;

        if (_isKey[index]) {
            // Cle (EX-GP-023) : ramassage au contact ET a l'action « Interagir » (EX-CTRL-022,
            // son premier usage) -- le contact seul ne suffit pas, contrairement a l'interrupteur.
            // Une fois ramassee, la porte reste ouverte DEFINITIVEMENT : jamais de retour a false,
            // meme si le joueur revient sur la case (deja consommee) ou rappuie sur Interagir.
            if (!_switchOn[index] && onSwitch && interactPressed) {
                _switchOn[index] = true;
                _collision.setTile(door.column, door.row, _openType[index]);
                GAMEPLAY_LOG_TRACE("Cle #" + std::to_string(index) + " ramassee -> porte (" +
                                   std::to_string(door.column) + ", " + std::to_string(door.row) +
                                   ") ouverte definitivement");
            }
        } else if (_continuous[index]) {
            // Plaque de pression (EX-GP-025) : ouverte tant qu'un poids suffisant y repose,
            // refermee des qu'il en repart — activation CONTINUE, reevaluee a chaque pas (pas de
            // front). Le poids n'intervient QUE pour ce type de declencheur.
            const bool shouldBeOpen = (onSwitch && playerMass >= MIN_TRIGGER_MASS) ||
                                      weightRestsOn(weights, mechanism.switchPosition);
            if (shouldBeOpen != _switchOn[index]) {
                _switchOn[index] = shouldBeOpen;
                _collision.setTile(door.column, door.row,
                                   _switchOn[index] ? _openType[index] : TileType::Solid);
                if (!_switchOn[index] && overlapsCell(playerBox, door)) {
                    _crushedPlayer = true;  // la porte se referme sur le personnage : mortel.
                }
                GAMEPLAY_LOG_TRACE("Plaque de pression #" + std::to_string(index) + " -> porte (" +
                                   std::to_string(door.column) + ", " + std::to_string(door.row) +
                                   ") " + (_switchOn[index] ? "ouverte" : "fermee"));
            }
        } else {
            // Interrupteur a bascule (EX-GP-020) : bascule au FRONT seulement, comportement
            // inchange (rester dessus ne re-bascule pas ; le poids n'intervient pas).
            if (onSwitch && !_playerOnSwitchPrev[index]) {
                _switchOn[index] = !_switchOn[index];
                _collision.setTile(door.column, door.row,
                                   _switchOn[index] ? _openType[index] : TileType::Solid);
                if (!_switchOn[index] && overlapsCell(playerBox, door)) {
                    _crushedPlayer = true;  // la porte se referme sur le personnage : mortel.
                }
                GAMEPLAY_LOG_TRACE("Interrupteur #" + std::to_string(index) + " -> porte (" +
                                   std::to_string(door.column) + ", " + std::to_string(door.row) +
                                   ") " + (_switchOn[index] ? "ouverte" : "fermee"));
            }
        }
        _playerOnSwitchPrev[index] = onSwitch;
    }

}

}  // namespace core
