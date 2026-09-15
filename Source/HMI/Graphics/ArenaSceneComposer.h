// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <string_view>

#include "HMI/Graphics/ComposedScene.h"
#include "HMI/Graphics/RenderLayer.h"

/**
 * @file HMI/Graphics/ArenaSceneComposer.h
 * @brief La scène de combat du Colisée composée en primitives, **sans GPU** : ce que dessinait
 *        `ArenaScene.ui.qml`/`ArenaTile.ui.qml` (`LOT-50`), prêt pour `hmi::SpriteBatch`.
 *
 * ## Ce qui va où
 *
 * | Pièce | Calque | Tri |
 * |---|---|---|
 * | sol (sable, pierre, dalle claire) | `RenderLayer::Tile` | texture, puis profondeur de case |
 * | enceinte (mur, colonne, bannière, torche, arche) | `RenderLayer::Object` | pied de la case |
 * | figurine d'un combattant | `RenderLayer::Player` | pied de son emprise |
 *
 * `Object` et `Player` partagent la bande de profondeur (`hmi::sortsByDepth`) : un mur plus bas à
 * l'écran passe devant une figurine, un mur plus haut derrière — ce que faisait le `z: c + r` de la
 * scène QML, sans que `hmi::ComposedScene::sort` change.
 *
 * ## La profondeur
 *
 * `sortOrder` = `depthSortOrder(pied) × ARENA_DEPTH_SLOTS + rang`. Le pied est le **sommet bas** du
 * losange de la case (de l'emprise, pour une figurine), pas le bord bas du quad : une bannière
 * posée plus haut que son mur doit rester devant lui. Le rang (`ArenaDepthSlot`) départage les
 * pièces d'une même case dans l'ordre où la brique QML les empilait ; laissé à égalité, le tri
 * trancherait par rang de texture, qui dépend de la première case composée.
 *
 * ## Les combattants
 *
 * - `Standing` : une figurine, à l'image que donne `hmi::ArenaAnimationState` ;
 * - `Down` : une figurine **quand même** — le combattant garde sa case et sa place dans l'ordre.
 *   Un allié montre la dernière image de sa bande `death.png`, un ennemi la dernière de `idle.png`,
 *   estompée (`ARENA_DOWN_ENEMY_ALPHA`), comme la brique QML ;
 * - `Withdrawn` : **aucun** quad — il a quitté la grille et l'ordre.
 *
 * Une créature de plus d'une case (`core::footprintSide`) a **une** figurine, centrée sur son
 * emprise et agrandie à sa taille, là où la scène QML en posait une par case tenue.
 *
 * ## Ce qui n'y est pas
 *
 * Les surbrillances de case (atteignable, au tour), la jauge et les points de vie : ce sont des
 * rectangles et du texte d'interface, pas des pièces de la planche, et le curseur de ciblage reste
 * en QML (`LOT-24`).
 *
 * ## Lecture seule
 *
 * La session n'est vue que par `const core::ArenaSession&` (`EX-ARCH-012`) : aucune méthode
 * mutante n'est appelable, et c'est le compilateur qui le garantit — `combat()` et `grid()` n'y
 * rendent que leur version `const`.
 */

namespace core {
class ArenaSession;
class IsoProjection;
}  // namespace core

namespace hmi {

class ArenaAppearanceCatalog;
struct ArenaAnimationState;

/// Largeur d'une image des bandes d'animation de la planche, en pixels (`manifest.json`, `frame`).
inline constexpr int ARENA_FIGURE_FRAME_WIDTH_PIXELS = 48;
/// Hauteur d'une image des bandes d'animation, en pixels.
inline constexpr int ARENA_FIGURE_FRAME_HEIGHT_PIXELS = 64;
/// Agrandissement de la figurine par rapport à la planche (`ArenaTile.ui.qml`, `1.25`).
inline constexpr float ARENA_FIGURE_SCALE = 1.25f;
/// Opacité d'un ennemi à terre (`ArenaTile.ui.qml`, `0.45`).
inline constexpr float ARENA_DOWN_ENEMY_ALPHA = 0.45f;

/**
 * @brief Rang d'une pièce à l'intérieur d'une même profondeur : l'ordre d'empilement de la brique
 *        `ArenaTile`, du sol vers le ciel.
 */
enum class ArenaDepthSlot : std::int32_t {
    Wall = 0,
    WallDecoration,
    Gate,
    Figure,
};

/// Nombre de rangs par profondeur : le multiplicateur de `depthSortOrder` dans le `sortOrder`.
inline constexpr std::int32_t ARENA_DEPTH_SLOTS = 4;

/**
 * @brief Ordre de tri d'une pièce de la bande de profondeur du Colisée.
 * @param footWorldY Ordonnée du sommet bas du losange (ou de l'emprise) qui porte la pièce.
 * @param slot       Rang de la pièce dans sa case.
 */
[[nodiscard]] std::int32_t arenaDepthSortOrder(float footWorldY, ArenaDepthSlot slot) noexcept;

/// @brief Une texture liable et ses dimensions en pixels (pour normaliser les UV).
struct ArenaTexture {
    TextureHandle texture = nullptr;
    int width = 0;
    int height = 0;
};

/**
 * @brief Les textures du Colisée, adressées par leur chemin sous `Source/Elements/Assets/Coliseum/`
 *        (`"terrain/sand.png"`, `"characters/bram/idle.png"`).
 *
 * Même rôle que `hmi::SceneTextures` pour l'exploration : la composition ne demande rien au GPU.
 * Un chemin absent se lie au damier `missing` (`EX-NFR-040`) ; si lui aussi manque, la pièce n'est
 * pas composée.
 */
struct ArenaSceneTextures {
    /// Textures chargées, par chemin. Comparateur transparent : la recherche se fait sans chaîne
    /// temporaire.
    std::map<std::string, ArenaTexture, std::less<>> byPath;
    /// Damier de repli.
    ArenaTexture missing;

    /// @return La texture de @p path, le damier si elle n'est pas chargée.
    [[nodiscard]] const ArenaTexture& resolve(std::string_view path) const {
        const auto found = byPath.find(path);
        return found != byPath.end() ? found->second : missing;
    }
};

/**
 * @brief Compose la scène de combat dans un tampon réutilisé.
 *
 * Le tampon n'est **ni vidé ni trié** : même contrat que `hmi::composeWorldSprites`, l'appelant
 * enchaîne `clear()`, les compositions, puis `sort()`.
 * @param scene      Scène à remplir.
 * @param session    La session d'arène, lue seulement.
 * @param catalog    Rôle des cases et figurines.
 * @param animation  Image courante de chaque figurine.
 * @param projection Projection isométrique de la grille (ses dimensions devraient être celles de
 *                   la grille ; la composition parcourt la grille).
 * @param textures   Textures liables.
 */
void composeArenaScene(ComposedScene& scene, const core::ArenaSession& session,
                       const ArenaAppearanceCatalog& catalog, const ArenaAnimationState& animation,
                       const core::IsoProjection& projection, const ArenaSceneTextures& textures);

/**
 * @brief Compose la scène de combat dans une scène neuve, **triée**.
 *
 * Commodité pour les tests et les captures ; le rendu garde un tampon et appelle la forme
 * ci-dessus.
 */
[[nodiscard]] ComposedScene composeArenaScene(const core::ArenaSession& session,
                                              const ArenaAppearanceCatalog& catalog,
                                              const ArenaAnimationState& animation,
                                              const core::IsoProjection& projection,
                                              const ArenaSceneTextures& textures);

}  // namespace hmi
