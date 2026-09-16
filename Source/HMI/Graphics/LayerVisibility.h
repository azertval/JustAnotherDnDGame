// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <array>
#include <cstddef>

#include "HMI/Graphics/RenderLayer.h"

/**
 * @file HMI/Graphics/LayerVisibility.h
 * @brief Jeu de visibilités par calque du mode d'inspection de l'éditeur (`LOT-51`, `EX-EDIT-044`).
 */

namespace hmi {

/// Nombre de valeurs de `hmi::RenderLayer` — dimensionne un tableau indexé par la valeur plutôt que
/// par une liste de champs écrite à la main (voir `hmi::LayerVisibility`).
inline constexpr std::size_t RENDER_LAYER_COUNT =
    static_cast<std::size_t>(RenderLayer::EditorOverlay) + 1;

/**
 * @brief Jeu de visibilités par calque, pour le mode d'inspection « définition des textures »
 *        (`LOT-51`).
 *
 * **Indexé par la valeur de `hmi::RenderLayer`**, plutôt que par une liste de champs nommés écrite
 * à la main : un calque futur (`EditorOverlay` mis à part, jamais exposé) n'oblige donc pas à
 * réécrire cette classe, seul `RENDER_LAYER_COUNT` grandit. Tous les calques sont visibles par
 * défaut, et rien n'est persisté entre deux sessions (TACHE-01) — contrairement à `hmi::RenderMode`
 * (`F8`), qui survit au redémarrage.
 *
 * `RenderLayer::Object` prend ici, pour la première fois, une signification concrète : aucune
 * entité ne porte ce calque à ce jour (les objets interactifs restent des tuiles ordinaires sur
 * `RenderLayer::Tile`, `LOT-45`). Ce bit sert donc d'**axe de résolution** — « la surcharge par
 * instance d'une tuile doit-elle s'afficher ? » — par opposition au bit `RenderLayer::Tile`, qui
 * demande « le skin de son type doit-il s'afficher ? ». Les deux entités physiques restent sur
 * `RenderLayer::Tile` pour l'ordre de dessin (`EX-REN-014`, jamais changé par ce lot) ; seule la
 * *résolution* de leur apparence consulte ces deux bits séparément (`hmi::resolveTileAppearance`,
 * TACHE-02). Quand les deux valent `true` (le défaut), le résultat est inchangé depuis `LOT-45` :
 * surcharge > skin > damier, sans distinction.
 *
 * Pour tout autre calque (`Background`, `Plane`, `Shadow`, `Player`, `Foreground`), le bit masque
 * grossièrement : une entité de ce calque n'émet aucune primitive si son bit est à `false`.
 * `UI`/`EditorOverlay` ne sont pas des calques de **contenu** (aides d'édition, jamais affectées) :
 * la classe leur réserve tout de même une entrée, mais rien ne l'interroge.
 */
class LayerVisibility {
public:
    /// @return `true` si @p layer doit être composé (tout calque est visible par défaut).
    [[nodiscard]] bool visible(RenderLayer layer) const noexcept {
        return _visible[static_cast<std::size_t>(layer)];
    }

    /// Affiche ou masque @p layer.
    void setVisible(RenderLayer layer, bool isVisible) noexcept {
        _visible[static_cast<std::size_t>(layer)] = isVisible;
    }

    /// Rétablit la visibilité de tous les calques (action « tout afficher », TACHE-03).
    void showAll() noexcept {
        _visible.fill(true);
    }

private:
    std::array<bool, RENDER_LAYER_COUNT> _visible = [] {
        std::array<bool, RENDER_LAYER_COUNT> all{};
        all.fill(true);
        return all;
    }();
};

}  // namespace hmi
