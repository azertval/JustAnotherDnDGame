// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

/**
 * @file HMI/Editor/EditContextTarget.h
 * @brief Cible des commandes Annuler/Refaire/Copier/Coller, indépendante du contexte d'édition qui
 *        les implémente (`LOT-57` TACHE-04, `EX-IHM-062`).
 */

namespace hmi {

/**
 * @brief Interface pure implémentée par le contexte d'édition **actif**.
 *
 * `MainWindow` dispatche les quatre actions dédupliquées (une définition chacune, `EX-IHM-062`) à
 * travers cette interface plutôt que d'appeler `GameViewport` directement : aujourd'hui, `hmi::
 * GameViewport` en est l'unique implémentation (édition de niveau). Le futur atelier pixel art
 * (`LOT-54`) lui donnera une seconde cible (canevas de pixel art, historique et presse-papiers
 * indépendants de `core::LevelDraft`) sans que ce point de dispatch n'ait à être réécrit — c'est la
 * raison d'être de cette interface plutôt qu'un appel direct.
 */
class EditContextTarget {
public:
    virtual ~EditContextTarget() = default;

    [[nodiscard]] virtual bool canUndo() const = 0;
    virtual void undo() = 0;
    [[nodiscard]] virtual bool canRedo() const = 0;
    virtual void redo() = 0;
    [[nodiscard]] virtual bool canCopy() const = 0;
    virtual void copy() = 0;
    [[nodiscard]] virtual bool canPaste() const = 0;
    virtual void paste() = 0;
};

}  // namespace hmi
