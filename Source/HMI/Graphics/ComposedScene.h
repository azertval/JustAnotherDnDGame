// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "Core/Math/Rect.h"
#include "HMI/Graphics/LayerVisibility.h"
#include "HMI/Graphics/Quad.h"
#include "HMI/Graphics/RenderLayer.h"
#include "HMI/Graphics/RenderMode.h"
#include "HMI/Graphics/TileAppearance.h"

/**
 * @file HMI/Graphics/ComposedScene.h
 * @brief Composition du rendu : liste ordonnée des primitives d'une image, **sans GPU**
 *        (`EX-NFR-004`, `EX-NFR-005`).
 */

namespace core {
class World;
}

namespace hmi {

/// Nature d'une primitive composée : rectangle aligné aux axes, ou segment épais orienté.
enum class QuadKind {
    /// `hmi::SpriteQuad` : rectangle aligné sur les axes.
    Sprite,
    /// `hmi::LineQuad` : segment épais, orienté librement.
    Line,
};

/**
 * @brief Une primitive prête à soumettre, avec tout ce qui détermine **quand** la dessiner.
 *
 * Les deux primitives possibles sont stockées côte à côte plutôt que dans une union ou un
 * `std::variant` : la composition doit rester une simple liste ordonnée, parcourue en séquence par
 * la soumission comme par les tests. Le surcoût mémoire (quelques dizaines d'octets par primitive,
 * dans un tampon réutilisé d'une image à l'autre) est sans commune mesure avec la complexité
 * qu'introduirait un accès polymorphe sur le chemin de dessin.
 */
struct ComposedQuad {
    /// Calque de dessin (critère de tri **prioritaire**, `EX-REN-014`).
    RenderLayer layer = DEFAULT_RENDER_LAYER;
    /// Texture liée (identité opaque) ; `nullptr` = primitive non dessinable.
    TextureHandle texture = nullptr;
    /// Rang de **première apparition** de la texture dans la scène (critère de regroupement).
    int textureRank = 0;
    /// Tri **fin à l'intérieur** d'un calque et d'une texture (`core::Sprite::layer`).
    std::int32_t sortOrder = 0;
    /// Laquelle des deux primitives ci-dessous est valide.
    QuadKind kind = QuadKind::Sprite;
    /// Primitive rectangle, valide si `kind == QuadKind::Sprite`.
    SpriteQuad sprite{};
    /// Primitive segment, valide si `kind == QuadKind::Line`.
    LineQuad line{};
};

/**
 * @brief Sous-divisions de profondeur par unité monde, pour la quantification du tri par Y.
 *
 * Le rendu est du pixel art à 16 px/unité (`EX-ARCH-021`) : deux sprites dont les pieds sont
 * distants de moins d'un pixel ne peuvent pas se départager **à l'écran**, et les départager dans
 * le tri ne ferait que les faire scintiller au gré des arrondis flottants. Quantifier au pixel est
 * donc à la fois suffisant et stabilisateur.
 */
inline constexpr float DEPTH_SUBDIVISIONS_PER_UNIT = 16.0f;

/**
 * @brief Ordre de profondeur d'une primitive, à partir du **pied** de son quad.
 *
 * Le pied — le bord **bas** — et non le coin haut : c'est le point de contact avec le sol qui
 * décide de la profondeur. Deux sprites de hauteurs différentes posés sur la même case doivent
 * s'ordonner de la même façon, ce que leur coin haut ne dit pas (`EX-REN-018`).
 *
 * Fonction **pure**, sans GPU. Un Y plus grand (plus bas à l'écran, `y` vers le bas) donne un
 * ordre plus grand, donc un dessin **plus tard**, donc au-dessus.
 * @param footWorldY Ordonnée du bord bas du quad, en unités monde.
 * @return L'ordre de tri correspondant.
 */
[[nodiscard]] std::int32_t depthSortOrder(float footWorldY) noexcept;

/**
 * @brief Compteurs d'une image composée, pour l'observabilité du volume soumis (`EX-NFR-005`).
 */
struct SceneStatistics {
    /// Primitives examinées par la composition (avant culling).
    int considered = 0;
    /// Primitives écartées parce qu'invisibles (culling).
    int culled = 0;
    /// Primitives conservées, donc soumises au GPU.
    int submitted = 0;
    /// Passes `begin/end` nécessaires : nombre de groupes contigus de même texture.
    int batches = 0;
    /// Mémoire de texture des **plans picturaux** liés par cette image, en octets
    /// (`EX-NFR-043`, `LOT-69` TACHE-09).
    ///
    /// Second axe d'observabilité, à côté des compteurs de primitives : les plans ajoutent **un
    /// seul quad** chacun mais occupent une texture à l'échelle du niveau — un plafond exprimé en
    /// primitives ne les verrait pas grossir. Seuls les plans l'alimentent : les autres primitives
    /// se partagent l'atlas, dont le coût ne dépend ni du niveau ni de leur nombre.
    std::size_t textureBytes = 0;
};

/**
 * @brief Liste **ordonnée** des primitives d'une image, indépendante de Direct3D.
 *
 * Sépare la **composition** (quels quads, dans quel ordre, avec quelle texture — logique pure) de
 * la **soumission** (`hmi::SpriteBatch`, GPU). C'est cette séparation qui rend l'ordre des
 * calques, le regroupement par texture et le culling **assertables sans GPU** (`EX-NFR-004`),
 * plutôt que vérifiables à l'œil. Même principe que `hmi::buildProceduralAtlasImage` (pur, testé)
 * vis-à-vis de `hmi::TextureAtlas` (GPU).
 *
 * Ordre de dessin, établi par `sort()` : **calque** (`RenderLayer`, ordre fixe et prioritaire),
 * puis **texture** (regroupement, dans l'ordre de première apparition), puis `sortOrder`
 * (`core::Sprite::layer`, tri fin existant) — **jamais** l'inverse : regrouper par texture ne doit
 * pouvoir réordonner deux primitives de calques différents sous aucun prétexte. Le tri est
 * **stable**, donc à clé égale l'ordre de composition est préservé.
 *
 * Le regroupement se fait sur le **rang de première apparition** de la texture, pas sur la valeur
 * du pointeur : l'ordre de dessin de deux textures d'un même calque reste ainsi déterministe d'une
 * exécution à l'autre.
 *
 * L'instance est un **tampon réutilisé** d'une image à l'autre (`clear()` conserve la capacité) :
 * la composition n'alloue plus après les premières images, et le rendu ne paie aucun détour
 * d'inspection.
 */
class ComposedScene {
public:
    /**
     * @brief Marge de culling, en unités monde, ajoutée de chaque côté du cadrage visible.
     *
     * Une case entière : une entité à cheval sur la frontière (personnage, décor débordant, effet)
     * doit rester composée, sinon elle disparaîtrait avant d'être sortie de l'écran.
     */
    static constexpr float CULLING_MARGIN_UNITS = 1.0f;

    /// Vide la scène (capacité conservée) et remet les compteurs à zéro. Le cadrage est conservé.
    void clear() noexcept;

    /**
     * @brief Restreint la composition aux primitives visibles dans un cadrage donné
     *        (`EX-NFR-005`).
     * @param worldBounds Rectangle cadré par la caméra, en unités monde (la marge est ajoutée
     *                    par la scène, l'appelant n'a pas à la connaître).
     */
    void setVisibleBounds(const core::Rect& worldBounds) noexcept;

    /// Désactive le culling : toutes les primitives ajoutées sont conservées.
    void clearVisibleBounds() noexcept;

    /// @return `true` si un cadrage a été fixé, donc si le culling est actif.
    [[nodiscard]] bool isCullingEnabled() const noexcept {
        return _visibleBounds.has_value();
    }

    /**
     * @brief Rectangle réellement utilisé pour le culling : le cadrage élargi de la marge.
     * @pre `isCullingEnabled()`.
     * @return Le cadrage augmenté de `CULLING_MARGIN_UNITS` de chaque côté.
     */
    [[nodiscard]] core::Rect cullingBounds() const noexcept;

    /**
     * @brief Ajoute un rectangle texturé à la scène, s'il est visible.
     * @param layer     Calque de dessin.
     * @param texture   Texture liée (identité opaque).
     * @param sortOrder Tri fin à l'intérieur du calque et de la texture.
     * @param quad      Primitive à composer (unités monde).
     * @return `true` si la primitive a été conservée, `false` si le culling l'a écartée.
     */
    bool addSprite(RenderLayer layer, TextureHandle texture, std::int32_t sortOrder,
                   const SpriteQuad& quad);

    /**
     * @brief Ajoute un segment épais à la scène, s'il est visible.
     * @param layer     Calque de dessin.
     * @param texture   Texture liée (identité opaque).
     * @param sortOrder Tri fin à l'intérieur du calque et de la texture.
     * @param quad      Primitive à composer (unités monde).
     * @return `true` si la primitive a été conservée, `false` si le culling l'a écartée.
     */
    bool addLine(RenderLayer layer, TextureHandle texture, std::int32_t sortOrder,
                 const LineQuad& quad);

    /**
     * @brief Déclare la mémoire de texture d'une primitive déjà ajoutée (`EX-NFR-043`).
     *
     * Séparé de `addSprite` plutôt qu'ajouté à `hmi::SpriteQuad` : seuls les plans picturaux
     * portent une texture dont la taille dépend du niveau, et grossir chaque primitive de la scène
     * — atlas, damier, texte, particules — de deux champs jamais lus coûterait sur le chemin
     * parcouru des centaines de fois par image, pour une information qu'un seul émetteur possède.
     * @param bytes Poids de la texture liée, en octets.
     */
    void addTextureBytes(std::size_t bytes) noexcept;

    /// Ordonne la scène (calque, puis texture, puis `sortOrder`), de façon **stable**.
    void sort();

    /// @return Les primitives composées, dans l'ordre de dessin après `sort()`.
    [[nodiscard]] const std::vector<ComposedQuad>& quads() const noexcept {
        return _quads;
    }

    /// @return Le nombre de primitives conservées.
    [[nodiscard]] std::size_t size() const noexcept {
        return _quads.size();
    }

    /// @return Le nombre de passes `begin/end` : groupes **contigus** de même texture.
    [[nodiscard]] int batchCount() const noexcept;

    /// @return Les compteurs de l'image composée (`EX-NFR-005`).
    [[nodiscard]] SceneStatistics statistics() const noexcept;

private:
    /// Rang de première apparition d'une texture, ajoutée à la table si elle est nouvelle.
    [[nodiscard]] int textureRank(TextureHandle texture);
    /// @return `true` si la boîte englobante est visible (ou si le culling est désactivé).
    [[nodiscard]] bool isVisible(const core::Rect& bounds) const;

    std::vector<ComposedQuad> _quads;
    std::vector<TextureHandle> _textureOrder;
    /// Cadrage **marge comprise**, calculé une fois par `setVisibleBounds` : c'est le rectangle
    /// comparé à chaque primitive, sur un chemin parcouru des centaines de fois par image.
    std::optional<core::Rect> _visibleBounds;
    int _considered = 0;
    int _culled = 0;
    std::size_t _textureBytes = 0;
};

/**
 * @brief Résumé des compteurs d'une image, pour la journalisation de diagnostic (`EX-NFR-005`).
 * @param statistics Compteurs à formater.
 * @return Une ligne lisible : composées, écartées, soumises, passes.
 */
[[nodiscard]] std::string formatSceneStatistics(const SceneStatistics& statistics);

/**
 * @brief Boîte englobante d'un rectangle texturé, en unités monde.
 * @param quad Primitive à borner.
 * @return Son rectangle englobant.
 */
[[nodiscard]] core::Rect spriteQuadBounds(const SpriteQuad& quad) noexcept;

/**
 * @brief Boîte englobante d'un segment épais, en unités monde.
 *
 * Englobe les deux extrémités **et** l'épaisseur (élargie d'une demi-épaisseur de chaque côté) :
 * un segment horizontal ou vertical aurait sinon une boîte d'aire nulle, systématiquement écartée
 * par le culling.
 * @param quad Primitive à borner.
 * @return Son rectangle englobant.
 */
[[nodiscard]] core::Rect lineQuadBounds(const LineQuad& quad) noexcept;

/**
 * @brief Compose les entités affichables d'un monde ECS en primitives.
 *
 * Parcourt les entités portant un `core::Transform` **et** un `core::Sprite`, résout la région
 * d'atlas en coordonnées de texture normalisées, applique l'interpolation de rendu
 * (`hmi::PreviousPosition`) et empile un `hmi::SpriteQuad` par entité. Lecture **seule** de l'ECS
 * (`EX-ARCH-012`) : ni la simulation ni l'interpolation ne sont affectées, y compris pour une
 * entité écartée par le culling (elle reste simulée, elle n'est simplement pas dessinée).
 *
 * Le calque de chaque entité vient de son `hmi::RenderLayerTag`, ou vaut
 * `hmi::DEFAULT_RENDER_LAYER` en son absence ; `core::Sprite::layer` reste le tri **fin** à
 * l'intérieur de ce calque.
 *
 * L'apparence de chaque entité — texture liée et région échantillonnée — est résolue **ici**, à la
 * composition, par `hmi::resolveTileAppearance` (`LOT-41`) : changer de mode de rendu ne
 * reconstruit donc jamais la scène ECS.
 * @param scene              Scène à remplir (non vidée : la composition peut être cumulative).
 * @param world              Monde dont on lit `Transform`, `Sprite`, `PreviousPosition`,
 *                           `RenderLayerTag`.
 * @param mode               Mode de rendu courant (`EX-REN-046`).
 * @param textures           Textures liables et leurs dimensions (atlas et damier de repli).
 * @param interpolationAlpha Facteur d'interpolation `[0, 1[` entre le pas de simulation précédent
 *                           et le pas courant (`EX-ARCH-031`) ; `0` reproduit le rendu non
 *                           interpolé.
 * @param visibility         Jeu de visibilités par calque du mode d'inspection de l'éditeur
 *                           (`hmi::LayerVisibility`, `LOT-51`, `EX-EDIT-044`) : tout visible par
 *                           défaut, donc sans effet pour `hmi::GameSession` qui ne le fournit
 *                           jamais.
 */
void composeWorldSprites(ComposedScene& scene, core::World& world, RenderMode mode,
                         const SceneTextures& textures, float interpolationAlpha,
                         const LayerVisibility& visibility = LayerVisibility{});

}  // namespace hmi
