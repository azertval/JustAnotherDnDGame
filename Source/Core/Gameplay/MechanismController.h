// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <vector>

#include "Core/Levels/Level.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Physics/Aabb.h"

/**
 * @file Core/Gameplay/MechanismController.h
 * @brief Comportement des mécanismes interrupteur↔porte : état + grille de collision.
 */

namespace core {

/**
 * @brief Poids posé sur un déclencheur par autre chose que le personnage (`EX-GP-025`, `LOT-65`
 *        TACHE-06).
 *
 * Jusqu'ici, `MechanismController` ne recevait que la boîte du **joueur** : la formule « poids
 * suffisant » de `EX-GP-025` était donc sans objet, et l'idiome de puzzle le plus classique du
 * genre — poser un poids sur une plaque pour tenir une porte ouverte **et repartir** — était hors
 * d'atteinte. Ce type porte ce qu'il faut pour qu'un **bloc poussable** (`EX-GP-022`) compte comme
 * un poids : sa boîte réelle et sa masse. Volontairement minimal et sans lien avec
 * `core::BlockController`, pour que ce contrôleur reste ignorant de la manière dont ces boîtes sont
 * produites.
 */
struct TriggerWeight {
    Aabb box;           ///< Boîte réelle du poids, en unités monde.
    float mass = 1.0F;  ///< Masse, comparée au même seuil que celle du personnage.
};

/**
 * @brief Fait vivre les mécanismes **interrupteur/plaque ↔ porte** d'un niveau (`EX-GP-020`,
 *        `EX-GP-021`, `EX-GP-025`).
 *
 * Logique **pure** (aucun rendu ni fenêtre). Deux comportements de déclencheur, selon la tuile
 * d'origine (figée au chargement) :
 * - **Interrupteur** (`TileType::Switch`) : quand la boîte du personnage **recouvre** la case au
 *   **front** (première frame de contact), l'état de la porte liée **bascule** et persiste.
 * - **Plaque de pression** (`TileType::PressurePlate`, `EX-GP-025`) : la porte reste **ouverte**
 *   tant qu'un poids suffisant y repose (comparé à `MIN_TRIGGER_MASS`), et se **referme** dès que
 *   ce n'est plus le cas — activation **continue**, pas de front. Ce poids peut être celui du
 *   personnage **ou** celui d'un **bloc poussable** (`core::TriggerWeight`, `EX-GP-022`) : c'est ce
 *   qui rend possible de poser un poids et de **repartir**, la porte restant ouverte derrière soi.
 *
 * Une porte **fermée** est **solide** (bloque), **ouverte** est franchissable. Le contrôleur
 * maintient une **copie mutable** du `TileMap` (grille de **collision**) que la physique consomme,
 * laissant la carte du `Level` intacte (source de vérité). Déterministe au pas fixe (`EX-NFR-002`).
 *
 * **Clé/porte verrouillée** (`TileType::Key`/`TileType::LockedDoor`, `EX-GP-023`) : troisième
 * comportement de déclencheur, résolu depuis la **même** liste `mechanisms()` que
 * interrupteur/plaque↔porte (`LevelLoader` les y ajoute toutes deux, aucune notion de liaison
 * dupliquée). Une clé se ramasse au contact **et** à l'action « Interagir »
 * (`core::PlayerInput::interactPressed`, `EX-CTRL-022` — son premier usage) ; le contact seul ne
 * suffit pas, à la différence de l'interrupteur. Une fois ramassée, la porte liée s'ouvre
 * **définitivement** : contrairement à l'interrupteur, l'état ne bascule plus jamais en arrière.
 */
class MechanismController {
public:
    /// Construit le contrôleur depuis @p level : portes **fermées** (solides) au départ.
    explicit MechanismController(const Level& level);

    /**
     * @brief Met à jour les mécanismes pour un pas : bascule les interrupteurs touchés (front),
     *        ouvre/referme les plaques de pression selon le poids présent, ramasse les clés
     *        touchées avec « Interagir », applique l'état des portes/portes verrouillées dans la
     *        grille de collision.
     * @param playerBox       Boîte englobante du personnage, en unités monde.
     * @param playerMass      Masse du personnage (`core::Player::mass`, `EX-GP-019`), comparée au
     *                        seuil des plaques de pression (`MIN_TRIGGER_MASS`) ; sans effet sur
     *                        les interrupteurs classiques ou les clés. Valeur par défaut = masse
     *                        par défaut du personnage (compatibilité des appels existants).
     * @param interactPressed Front de l'action « Interagir » (`core::PlayerInput::
     *                        interactPressed`, `EX-CTRL-022`) pour ce pas ; seul déclencheur du
     *                        ramassage d'une clé (le contact seul ne suffit pas). Sans effet sur
     *                        les autres mécanismes. Valeur par défaut = faux (compatibilité des
     *                        appels existants, aucun niveau sans clé n'est affecté).
     * @param weights         Poids **autres que le personnage** posés sur les déclencheurs ce pas
     *                        (blocs poussables, `EX-GP-022`) — voir `core::TriggerWeight`. Ne
     *                        concerne que les plaques de pression à
     *                        activation continue : un bloc ne bascule pas un interrupteur et ne
     *                        ramasse pas une clé. Vide par défaut (compatibilité des appels
     *                        existants).
     */
    void update(const Aabb& playerBox, float playerMass = 1.0f, bool interactPressed = false,
                const std::vector<TriggerWeight>& weights = {});

    /**
     * @brief Une porte s'est-elle **refermée sur le personnage** au dernier `update` (`EX-GP-021`,
     *        `LOT-65` TACHE-06) ?
     *
     * L'écrasement est **mortel** : l'appelant (`hmi::GameSession`) traduit ce drapeau en boîte
     * de danger supplémentaire pour `core::evaluateOutcome`.
     * Sans cela, une porte qui redevient solide sur le personnage le laisse **encastré** dans un
     * mur, sans échec possible — précisément la « situation sans issue » que la conception des
     * niveaux interdit (`Documentation/Specification/niveaux.md`, Sec. 3).
     * @return true si au moins une porte s'est fermée sur la boîte passée au dernier `update`.
     */
    [[nodiscard]] bool crushedPlayer() const noexcept {
        return _crushedPlayer;
    }

    /// @return La grille de collision courante (portes à jour), à passer à la physique.
    [[nodiscard]] const TileMap& collisionMap() const noexcept {
        return _collision;
    }

    /// @return Les liaisons de mécanismes (positions interrupteur/porte).
    [[nodiscard]] const std::vector<Mechanism>& mechanisms() const noexcept {
        return _mechanisms;
    }

    /// @return true si la porte du mécanisme @p index est **ouverte** (interrupteur activé).
    [[nodiscard]] bool isDoorOpen(std::size_t index) const {
        return _switchOn[index];
    }

    /// @return true si le mécanisme @p index est une **plaque de pression** (activation continue,
    ///         `EX-GP-025`), false si c'est un **interrupteur** à bascule classique (`EX-GP-020`).
    ///         Distingue les deux sons de déclenchement (`hmi::SoundTriggers`, `LOT-60`).
    [[nodiscard]] bool isContinuous(std::size_t index) const {
        return _continuous[index];
    }

    /// @return true si le mécanisme @p index est une **clé** (`TileType::Key`, `EX-GP-023`), dont
    ///         le ramassage exige l'action « Interagir » en plus du contact. Symétrique de
    ///         `isContinuous` : même donnée figée au chargement, exposée pour que l'affichage tête
    ///         haute puisse rappeler cette action au contact (`hmi::gameHudLines`, `LOT-65`).
    [[nodiscard]] bool isKey(std::size_t index) const {
        return _isKey[index];
    }

private:
    TileMap _collision;  ///< Copie mutable : portes Solid (fermées) / Door (ouvertes).
    std::vector<Mechanism> _mechanisms;  ///< Liaisons déclencheur↔porte (positions).
    std::vector<bool> _switchOn;         ///< État de chaque déclencheur (porte ouverte ?).
    std::vector<bool>
        _playerOnSwitchPrev;  ///< Le personnage était-il sur le déclencheur au pas précédent ?
    std::vector<bool> _continuous;  ///< true = plaque de pression (activation continue), figé au
                                    ///< chargement ; false = interrupteur à bascule classique.
    std::vector<bool> _isKey;  ///< true = déclencheur `TileType::Key` (ramassage par Interagir,
                               ///< ouverture définitive), figé au chargement comme `_continuous`.
    std::vector<TileType>
        _openType;  ///< Type de tuile « porte ouverte » de chaque mécanisme (`Door` ou
                    ///< `LockedDoor`), capturé avant que le constructeur ne fige la case en
                    ///< `Solid` — la grille de collision y revient quand le mécanisme s'active.
    bool _crushedPlayer = false;  ///< Une porte s'est refermée sur le personnage au dernier pas.
};

}  // namespace core
