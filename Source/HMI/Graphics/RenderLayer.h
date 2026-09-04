// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>

/**
 * @file HMI/Graphics/RenderLayer.h
 * @brief Ordonnancement de calques **unique et explicite** du rendu (`EX-REN-014`).
 */

namespace hmi {

/**
 * @brief Calques de dessin, du plus **arrière** au plus **avant** (`EX-REN-014`, `EX-DEC-002`).
 *
 * L'**ordre de déclaration est l'ordre de dessin** : une valeur déclarée plus bas est dessinée
 * par-dessus les précédentes. C'est le **seul** ordonnancement de calques du projet — aucun lot ne
 * doit en inventer un concurrent, sous peine de rendre l'ordre visuel dépendant de qui dessine en
 * dernier (`EX-REN-014`).
 *
 * Toutes les valeurs sont réservées **dès `LOT-40`**, même celles qu'aucun code n'utilise encore
 * (`Background` en `LOT-44`, `Plane`/`Foreground` en `LOT-49`, `Shadow` en `LOT-55`, `Object` en
 * `LOT-45`, `UI` en `LOT-52`) : c'est précisément cette anticipation qui évite qu'un lot suivant
 * réinvente son propre empilement. Seules `Tile`, `Player` et `EditorOverlay` sont utilisées à ce
 * jour, et elles reproduisent exactement l'ordre d'avant le lot (tuiles sous personnage, aides
 * d'édition au-dessus de tout).
 *
 * `Foreground` est **au-dessus** de `Player` par construction : c'est le contrat de lecture « ce
 * qui passe devant le personnage n'est pas physique » (`EX-DEC-042`).
 *
 * Le nombre de **plans picturaux** d'un niveau est libre (`LOT-69`), mais aucun n'ajoute de valeur
 * ici : les plans occupent `Plane` ou `Foreground` selon leur profondeur, et leur **rang** dans la
 * liste du niveau les ordonne à l'intérieur du calque. Donner une valeur d'énumération à chaque
 * plan figerait dans le rendu ce que le format déclare variable, et ferait enfler
 * `RENDER_LAYER_COUNT`, donc `hmi::LayerVisibility`.
 *
 * Notion de **présentation** (`HMI`) : `Core` l'ignore et continue de ne connaître que
 * `core::Sprite::layer`, entier de tri **fin à l'intérieur** d'un calque (`EX-NFR-011`).
 */
enum class RenderLayer : std::int32_t {
    /// Image de fond du niveau (`LOT-44`, `EX-REN-044`).
    Background = 0,
    /// Plans picturaux **derrière** les tuiles, éventuellement en parallaxe (`LOT-69`,
    /// `EX-DEC-042`). Nommé `Decor` jusqu'au `LOT-69` : laisser un calque porter le nom d'un
    /// système retiré serait exactement la dette que `EX-REN-014` cherche à éviter.
    Plane,
    /// Ombres portées des tuiles solides (`LOT-55`, `EX-REN-045`).
    Shadow,
    /// Tuiles physiques du niveau — le seul calque peuplé avant `LOT-42`.
    Tile,
    /// Objets interactifs (interrupteurs, portes, dangers) et **décor traversé** : tout ce qui
    /// partage la profondeur du personnage (`LOT-45`/`LOT-47`, `LOT-07`).
    Object,
    /// Personnage joueur. Même **bande de profondeur** que `Object` depuis le `LOT-07` : les deux
    /// s'ordonnent entre eux par le Y de leur pied, pas par leur rang de calque.
    Player,
    /// Plans picturaux **devant** le personnage (`LOT-69`, `EX-DEC-042`).
    Foreground,
    /// Texte et interface en scène (`LOT-52`, `EX-REN-031`).
    UI,
    /// Aides d'édition : grille de repère, liens de mécanismes, aperçu de sélection.
    EditorOverlay,
};

/**
 * @brief Le calque appartient-il à la **bande de profondeur** ?
 *
 * Dans une vue de dessus, « devant » et « derrière » ne se décident plus par un rang de calque
 * fixe mais par la **position** : le personnage passe devant ce qui est au-dessus de lui à
 * l'écran, derrière ce qui est en dessous. Les primitives d'une bande de profondeur s'ordonnent
 * donc entre elles par leur `sortOrder` — le Y de leur pied (`hmi::depthSortOrder`) — **avant**
 * tout regroupement de texture.
 *
 * C'est le prix à payer, et il est assumé : regrouper par texture d'abord ferait passer tout le
 * décor devant le personnage, ou tout derrière, selon l'ordre d'apparition des textures. Un
 * personnage et un arbre n'ayant jamais la même texture, il n'existe aucun ordre de calque qui
 * rende les deux cas justes — seule la profondeur le peut, au prix de passes de dessin
 * supplémentaires (`EX-REN-018`).
 */
[[nodiscard]] constexpr bool sortsByDepth(RenderLayer layer) noexcept {
    return layer == RenderLayer::Object || layer == RenderLayer::Player;
}

/**
 * @brief Bande de tri d'un calque : `Object` et `Player` en partagent **une seule**.
 *
 * Sans cela, le rang de calque trancherait avant la profondeur et le personnage passerait
 * **toujours** devant un objet, quelle que soit sa position — exactement ce que le tri par Y doit
 * corriger. Tous les autres calques restent leur propre bande : un plan pictural ne se mélange
 * jamais au décor, et les aides d'édition restent au-dessus de tout (`EX-REN-014`).
 */
[[nodiscard]] constexpr std::int32_t renderBand(RenderLayer layer) noexcept {
    return static_cast<std::int32_t>(sortsByDepth(layer) ? RenderLayer::Object : layer);
}

/**
 * @brief Identité **opaque** d'une texture liée, du point de vue de la composition.
 *
 * La composition du rendu ne fait que **comparer** et **regrouper** des textures ; elle n'a
 * jamais besoin d'en connaître le type Direct3D. Ce typage volontairement opaque permet de la
 * garder libre de toute dépendance GPU (et donc testable sans carte, `EX-NFR-004`) : côté
 * soumission, `hmi::SpriteBatch` reconvertit la valeur en `QRhiTexture*`, seule couche qui en
 * connaisse le type réel. Une valeur nulle désigne « aucune texture liée ».
 */
using TextureHandle = void*;

/**
 * @brief Composant de présentation fixant le calque de dessin d'une entité.
 *
 * Vit côté `HMI` (comme `hmi::PreviousPosition`) : `Core` ne connaît pas `RenderLayer`
 * (`EX-NFR-011`). Une entité **sans** ce composant est dessinée sur `RenderLayer::Tile`, qui est le
 * cas de très loin le plus fréquent (toutes les tuiles d'un niveau) — l'absence de composant est
 * donc le défaut utile, et non un oubli.
 */
struct RenderLayerTag {
    /// Calque de dessin de l'entité.
    RenderLayer value = RenderLayer::Tile;
};

/// Calque appliqué à une entité dépourvue de `hmi::RenderLayerTag`.
inline constexpr RenderLayer DEFAULT_RENDER_LAYER = RenderLayer::Tile;

/**
 * @brief Nom lisible d'un calque, pour la journalisation et les messages d'échec de test.
 * @param layer Calque à nommer.
 * @return Le nom du calque (chaîne statique, jamais nulle).
 */
[[nodiscard]] constexpr const char* renderLayerName(RenderLayer layer) noexcept {
    switch (layer) {
        case RenderLayer::Background:
            return "Background";
        case RenderLayer::Plane:
            return "Plane";
        case RenderLayer::Shadow:
            return "Shadow";
        case RenderLayer::Tile:
            return "Tile";
        case RenderLayer::Object:
            return "Object";
        case RenderLayer::Player:
            return "Player";
        case RenderLayer::Foreground:
            return "Foreground";
        case RenderLayer::UI:
            return "UI";
        case RenderLayer::EditorOverlay:
            return "EditorOverlay";
    }
    return "Inconnu";
}

}  // namespace hmi
