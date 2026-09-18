// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

/**
 * @file HMI/Editor/EditContextTarget.h
 * @brief Cible des commandes Annuler/Refaire/Copier/Coller, indépendante du contexte d'édition qui
 *        les implémente (`EX-IHM-062`).
 */

namespace hmi {

/**
 * @brief Interface pure implémentée par le contexte d'édition **actif**.
 *
 * `MainWindow` dispatche les quatre actions dédupliquées (une définition chacune, `EX-IHM-062`) à
 * travers cette interface plutôt que d'appeler `EditorViewport` directement : aujourd'hui,
 * `hmi::EditorViewport` en est l'unique implémentation (édition de carte). Un second contexte
 * d'édition, avec son propre historique et son propre presse-papiers, s'y brancherait sans que ce
 * point de dispatch n'ait à être réécrit.
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
