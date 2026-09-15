// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstddef>
#include <optional>
#include <set>

/**
 * @file HMI/Graphics/PlaneVisibility.h
 * @brief Visibilité **par rang de plan** dans l'éditeur (`EX-DEC-045`, LOT-69 TACHE-05).
 */

namespace hmi {

/**
 * @brief Jeu de visibilités des plans picturaux d'un niveau, adressé par **rang**.
 *
 * Distinct de `hmi::LayerVisibility`, et pour une raison de fond : celui-ci porte sur les
 * **calques** de rendu, dont le nombre est fixe et connu à la compilation (`EX-REN-014`) ; le
 * nombre de plans, lui, est libre et change au fil de l'édition. Un bit par calque ne pourrait
 * donc pas les exprimer.
 *
 * **Non persistée** : c'est une aide d'édition, pas une propriété du niveau. Rouvrir un niveau
 * réaffiche tous ses plans.
 *
 * Deux modes, volontairement exclusifs :
 * - **masquage** : chaque plan peut être caché individuellement (`setVisible`) ;
 * - **isolement** : un seul plan reste visible (`isolate`), les autres disparaissent quels que
 *   soient leurs masquages individuels — c'est le geste « je veux voir ce que je peins, seul ».
 *   `showAll()` sort de l'isolement **et** lève tous les masquages.
 *
 * Les rangs suivent ceux de `core::Level::planes` : retirer un plan décale les suivants, et un
 * masquage devient alors celui de son voisin. C'est assumé — l'appelant (`hmi::PlanesPanel`)
 * rétablit l'état après une mutation de la liste, plutôt que d'introduire des identités stables
 * dans le format pour un besoin d'affichage.
 */
class PlaneVisibility {
public:
    /// @return `true` si le plan de rang @p rank doit être composé.
    [[nodiscard]] bool visible(std::size_t rank) const {
        if (_isolated.has_value()) {
            return *_isolated == rank;
        }
        return !_hidden.contains(rank);
    }

    /// Masque ou réaffiche le plan de rang @p rank. Sans effet visible tant qu'un plan est isolé.
    void setVisible(std::size_t rank, bool visible) {
        if (visible) {
            _hidden.erase(rank);
        } else {
            _hidden.insert(rank);
        }
    }

    /// N'affiche plus que le plan de rang @p rank. Un second appel change le plan isolé.
    void isolate(std::size_t rank) {
        _isolated = rank;
    }

    /// Réaffiche tout : sort de l'isolement **et** lève les masquages individuels.
    void showAll() {
        _isolated.reset();
        _hidden.clear();
    }

    /// @return Le rang du plan isolé, ou `std::nullopt` si aucun ne l'est.
    [[nodiscard]] std::optional<std::size_t> isolatedRank() const noexcept {
        return _isolated;
    }

private:
    /// Rang du plan isolé, s'il y en a un — prioritaire sur `_hidden`.
    std::optional<std::size_t> _isolated;
    /// Rangs masqués individuellement. `std::set` et non un vecteur de bits : le nombre de plans
    /// est libre, et l'ensemble est presque toujours vide.
    std::set<std::size_t> _hidden;
};

}  // namespace hmi
