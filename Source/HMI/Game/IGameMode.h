// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string_view>
#include <vector>

#include "Core/Levels/LevelOutcome.h"
#include "Core/Physics/PlayerInput.h"

/**
 * @file HMI/Game/IGameMode.h
 * @brief Mode de jeu : l'**ordre des passes** d'un pas fixe, séparé de l'orchestrateur (`LOT-05`).
 */

namespace hmi {

/**
 * @brief Les passes d'un pas fixe, telles que l'orchestrateur les offre à un mode de jeu.
 *
 * Chaque méthode est **une** passe : elle fait ce que son nom dit, ni plus ni moins, et ne décide
 * jamais de ce qui vient après. C'est le mode (`hmi::IGameMode`) qui les enchaîne — c'est là toute
 * la séparation qu'apporte le `LOT-05` (`EX-ARCH-002`).
 *
 * Interface **sans Qt et sans GPU**, ne parlant que de `core::` : c'est ce qui rend un mode de jeu
 * testable sans fenêtre, alors que `hmi::GameSession` — son unique implémentation réelle — exige
 * un atlas, un lot de sprites et une police. Un mode se vérifie donc contre une implémentation
 * d'essai qui **enregistre** l'ordre des appels, sans simuler quoi que ce soit.
 */
class IGameModePasses {
public:
    IGameModePasses() = default;
    virtual ~IGameModePasses() = default;
    IGameModePasses(const IGameModePasses&) = delete;
    IGameModePasses& operator=(const IGameModePasses&) = delete;
    IGameModePasses(IGameModePasses&&) = delete;
    IGameModePasses& operator=(IGameModePasses&&) = delete;

    /// Fige la position **courante** de chaque entité mobile comme sa position « précédente »,
    /// avant que le pas ne la modifie (interpolation de rendu, `EX-ARCH-031`).
    virtual void snapshotPreviousPositions() = 0;
    /// Avance les particules déjà émises (`LOT-53`), avant que ce pas n'en émette de nouvelles.
    virtual void advanceParticles(float fixedDelta) = 0;
    /// Fait décroître la secousse d'écran courante (`LOT-53`).
    virtual void advanceScreenShake(float fixedDelta) = 0;
    /// Déplace le personnage selon l'intention de ce pas.
    virtual void moveCharacter(const core::PlayerInput& input, float fixedDelta) = 0;
    /// Avance les animations d'entités et l'horloge des tuiles animées (`EX-REN-012`).
    virtual void advanceAnimations(float fixedDelta) = 0;
    /// Recadre la caméra selon le mode de cadrage résolu du niveau (`EX-LVL-006`).
    virtual void updateCamera(float fixedDelta) = 0;
    /// Résout les mécanismes à partir de la boîte du personnage et de son intention.
    virtual void updateMechanisms(const core::PlayerInput& input) = 0;
    /// Relève les transitions du pas (personnage, mécanismes) en événements de jeu (`LOT-60`).
    virtual void detectEvents() = 0;
    /// Projette l'état logique des mécanismes sur leur apparence (`LOT-47`, `EX-REN-006`).
    virtual void updateMechanismVisuals(float fixedDelta) = 0;
    /// @return L'issue du niveau pour ce pas, sans rien y changer.
    [[nodiscard]] virtual core::LevelOutcome evaluateOutcome() = 0;
    /// Conséquences d'un échec : éclatement, secousse, puis rechargement du niveau.
    virtual void onLevelLost() = 0;
};

/**
 * @brief Un **mode de jeu** : l'ordre dans lequel les passes d'un pas fixe s'enchaînent.
 *
 * `hmi::GameSession` mêlait deux rôles — orchestrateur (monde ECS, caméra, événements, HUD, pas
 * fixe, interpolation) et mode de jeu (l'ordre des passes lui-même). Tant qu'il n'y avait qu'un
 * genre, la confusion ne coûtait rien. Le RPG a besoin d'au moins **trois** ordres différents :
 * exploration (déplacement, animation, caméra, mécanismes, issue), dialogue (monde gelé, seul le
 * runner de dialogue avance, `LOT-15`) et combat (initiative, tour actif, résolution d'action —
 * l'exploration ne tourne plus du tout, `LOT-18`). Sans cette séparation, les trois s'entasseraient
 * en `if` dans une fonction déjà longue.
 *
 * La sélection se fait par **polymorphisme**, jamais par un `if (mode == …)` dans l'orchestrateur.
 */
class IGameMode {
public:
    IGameMode() = default;
    virtual ~IGameMode() = default;
    IGameMode(const IGameMode&) = delete;
    IGameMode& operator=(const IGameMode&) = delete;
    IGameMode(IGameMode&&) = delete;
    IGameMode& operator=(IGameMode&&) = delete;

    /// @return Le nom du mode, pour les diagnostics et les journaux.
    [[nodiscard]] virtual std::string_view name() const = 0;

    /**
     * @brief L'ordre des passes que `step()` enchaîne, nommées, pour les diagnostics.
     *
     * Documentation **exécutable** : un test compare cette liste à la séquence réellement appelée,
     * de sorte qu'un ordre modifié sans mettre la liste à jour échoue au lieu de mentir.
     */
    [[nodiscard]] virtual std::vector<std::string_view> passOrder() const = 0;

    /// Appelé quand le mode devient actif, une fois le niveau chargé.
    virtual void onLoad(IGameModePasses& passes) = 0;

    /**
     * @brief Avance d'un pas fixe, en enchaînant les passes dans l'ordre propre au mode.
     * @param passes     Les passes offertes par l'orchestrateur.
     * @param input      Intention de jeu déjà résolue pour ce pas.
     * @param fixedDelta Durée du pas fixe, en secondes.
     * @return L'issue du niveau à l'issue du pas.
     */
    virtual core::LevelOutcome step(IGameModePasses& passes, const core::PlayerInput& input,
                                    float fixedDelta) = 0;

    /// Appelé quand le mode cesse d'être actif (bascule vers un autre mode, ou fin de session).
    virtual void onUnload(IGameModePasses& passes) = 0;
};

}  // namespace hmi
