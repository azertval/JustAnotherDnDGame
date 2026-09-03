// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

/**
 * @file Core/Physics/PlayerInput.h
 * @brief Intention de déplacement du personnage, neutre vis-à-vis de l'entrée physique.
 */

namespace core {

/**
 * @brief **Intention** de déplacement transmise à la physique, dissociée de toute touche.
 *
 * Contrat entre la couche d'entrée (`HMI`, qui traduit clavier/manette — `EX-CTRL-010`) et la
 * simulation (`Core`, qui l'applique). `Core` ne connaît **que** cette intention : il ignore les
 * touches, ce qui garde la simulation testable et permettra remappage/manette sans le modifier.
 * Donnée pure, transitoire (re-remplie chaque frame), passée au système de déplacement.
 */
struct PlayerInput {
    /// Intention de déplacement horizontal, normalisée dans l'intervalle [-1, 1] :
    /// -1 = vers la gauche, +1 = vers la droite, 0 = immobile.
    float moveX = 0.0f;
    /// Saut **vient d'être pressé** cette frame (front montant) : déclenche le saut et alimente
    /// le *jump buffering* (`EX-CTRL-011`).
    bool jumpPressed = false;
    /// Bouton de saut **maintenu** : sert à la hauteur de saut variable (relâcher tôt = petit
    /// saut).
    bool jumpHeld = false;
    /// Intention de **visée verticale** (pour la direction du dash) : -1 haut, +1 bas, 0. Le
    /// déplacement reste horizontal ; ce champ ne sert qu'à orienter le dash (`EX-GP-017`).
    float moveY = 0.0f;
    /// **Dash** vient d'être pressé cette frame (front montant) — action dédiée (`EX-CTRL-013`).
    bool dashPressed = false;
    /// **Dash** maintenu : sert à charger un dash boosté (`EX-GP-056`, maintien de la direction
    /// opposée ET du bouton dash). N'affecte jamais le déclenchement du dash lui-même
    /// (`dashPressed`, un front) ; sert uniquement de garde pour la charge, afin qu'un simple
    /// changement de direction pendant un déplacement normal ne puisse jamais l'amorcer par
    /// accident.
    bool dashHeld = false;
    /// **Interagir** vient d'être pressée cette frame (front montant) — action dédiée
    /// (`EX-CTRL-022`), complète l'activation par contact des mécanismes sans la remplacer.
    bool interactPressed = false;
    /// **Interagir** maintenue.
    bool interactHeld = false;
    /// **Interagir** vient d'être relâchée cette frame (front descendant).
    bool interactReleased = false;
};

}  // namespace core
