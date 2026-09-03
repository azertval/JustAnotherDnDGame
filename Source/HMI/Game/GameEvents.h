// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "Core/Gameplay/MechanismController.h"
#include "Core/Levels/LevelOutcome.h"

/**
 * @file HMI/Game/GameEvents.h
 * @brief Détection pure des transitions d'état du jeu et de l'IHM (`EX-REN-047`, `LOT-60`).
 */

namespace hmi {

/**
 * @brief Événement discret survenu sur un pas de simulation ou une action d'interface.
 *
 * Unifie les transitions de jeu (personnage, mécanismes) et d'interface (menus, écrans du
 * `LOT-59`) sous une seule énumération : c'est ce qui permet à `hmi::SoundTriggers` de leur
 * associer un son par une table exhaustive unique, et à `LOT-53` de réutiliser la même
 * énumération pour ses particules sans dupliquer la détection.
 */
enum class GameEvent {
    // Personnage. **Sans producteur depuis le `LOT-06`** : ces quatre transitions étaient celles
    // d'un personnage de plateforme, et le déplacement en vue de dessus n'a ni saut, ni
    // atterrissage, ni glissade murale. Les valeurs restent déclarées — la table de sons
    // (`hmi::soundForEvent`) et les statistiques d'essai (`hmi::LevelRunStats`) s'y accrochent, et
    // c'est le combat (`LOT-21`) puis l'audio (`LOT-28`) qui diront ce que le RPG met à leur place.
    Jumped,
    Landed,
    Dashed,
    WallContactEnter,
    // Mécanismes (détecté par `detectMechanismEvents`).
    SwitchToggled,
    DoorOpened,
    DoorClosed,
    PressurePlatePressed,
    PressurePlateReleased,
    BlockPushed,
    // Issue du tableau (détecté par `detectOutcomeEvent`, depuis `core::LevelOutcome`).
    Died,
    LevelCompleted,
    // Interface (`LOT-59`) : raisés directement aux points de signal Qt existants, pas par
    // diffusion d'état — la table de sons ci-dessous les couvre malgré tout, pour l'uniformité.
    MenuNavigate,
    MenuConfirm,
    MenuBack,
    PauseOpened,
    SequenceCompleted,
};

/// Nombre de valeurs de `GameEvent` — pour les tests d'exhaustivité (parcours de l'énumération).
inline constexpr int GAME_EVENT_COUNT = static_cast<int>(GameEvent::SequenceCompleted) + 1;

/// État « porte ouverte » de chaque mécanisme, même index que
/// `core::MechanismController::mechanisms()`.
struct MechanismEventState {
    std::vector<bool> doorOpen;

    /// @return L'état extrait de @p mechanisms, à un pas de simulation donné.
    [[nodiscard]] static MechanismEventState capture(const core::MechanismController& mechanisms) {
        MechanismEventState state;
        state.doorOpen.reserve(mechanisms.mechanisms().size());
        for (std::size_t index = 0; index < mechanisms.mechanisms().size(); ++index) {
            state.doorOpen.push_back(mechanisms.isDoorOpen(index));
        }
        return state;
    }
};

/**
 * @brief Détecte les transitions des mécanismes entre deux pas de simulation consécutifs.
 *
 * Un même mécanisme (`isDoorOpen`) sert à la fois de déclencheur et de porte : une seule
 * transition produit **un seul** événement (celui du déclencheur), jamais un second pour la
 * porte — sans quoi une simple activation jouerait deux sons superposés.
 * @param previous État des mécanismes au pas précédent.
 * @param current État des mécanismes au pas courant.
 * @param isContinuous Pour chaque mécanisme (même index que @p previous / @p current) : true =
 *        plaque de pression (joue « pressée »/« relâchée »), false = interrupteur à bascule
 *        (joue « basculé »). Un désaccord de taille entre les trois vecteurs ignore l'index en
 *        trop plutôt que d'accéder hors bornes.
 */
[[nodiscard]] std::vector<GameEvent> detectMechanismEvents(const MechanismEventState& previous,
                                                           const MechanismEventState& current,
                                                           const std::vector<bool>& isContinuous);

/**
 * @brief Traduit l'issue d'un pas de simulation en événement.
 * @param outcome Issue renvoyée par `core::evaluateOutcome` / `core::GameSession::update`.
 * @return `Died` pour `Lost`, `LevelCompleted` pour `Won`, `std::nullopt` pour `Playing`.
 */
[[nodiscard]] std::optional<GameEvent> detectOutcomeEvent(core::LevelOutcome outcome);

}  // namespace hmi
