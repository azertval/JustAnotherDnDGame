// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <filesystem>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <DirectXMath.h>

#include "Core/Ecs/Components/Animation.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/Plane.h"
#include "HMI/Graphics/BackgroundRenderer.h"
#include "HMI/Graphics/ComposedScene.h"
#include "HMI/Graphics/PlaneVisuals.h"
#include "HMI/Graphics/SkinCatalog.h"
#include "HMI/Graphics/SpriteBatch.h"

/**
 * @file HMI/Graphics/SpriteRenderer.h
 * @brief Rend les entités de l'ECS possédant un `Transform` et un `Sprite`.
 */

namespace core {
class World;
}  // namespace core

namespace hmi {

class Camera2D;
class TextureAtlas;
class TextureCache;

/**
 * @brief Soumet une scène composée au pipeline de dessin, une passe par groupe de texture.
 *
 * Seul endroit du rendu qui reconvertit une `hmi::TextureHandle` en ressource Direct3D : c'est la
 * **frontière** entre la composition (pure, testable sans GPU) et la soumission. Émet un
 * `SpriteBatch::begin/end` par groupe **contigu** de même texture, dans l'ordre de la scène — donc
 * dans l'ordre des calques, que `ComposedScene::sort()` a rendu prioritaire (`EX-REN-043`). Le
 * contrat public de `hmi::SpriteBatch` est strictement inchangé.
 * @param batch      Pipeline de quads texturés (non possédé).
 * @param projection Matrice de projection monde → clip (fournie par la caméra).
 * @param scene      Scène **déjà triée** (`ComposedScene::sort()`).
 */
void submitComposedScene(SpriteBatch& batch, const DirectX::XMFLOAT4X4& projection,
                         const ComposedScene& scene);

/// Sous-dossier des skins de tuiles, relatif au dossier d'assets (`LOT-42`).
inline const std::string SKINS_SUBDIRECTORY = "Skins/";

/**
 * @brief Avance l'horloge d'animation partagée des tuiles animées d'un jeu de skins courant
 *        (`LOT-46` TACHE-05).
 *
 * Facteur commun à `hmi::GameSession` (pas fixe, déterministe, `EX-NFR-002`) et
 * `hmi::DraftRenderer` (temps réel de l'aperçu d'édition — la détermination n'a pas de sens hors
 * simulation) : découvre les assets animés du jeu de skins courant (mode `Single`, sans
 * silhouette — `bitmask16` et silhouette détourée excluent l'animation,
 * `hmi::animationExcludedForTile`, signalé une fois par asset via @p warnedExclusions) et avance
 * leur horloge partagée d'un pas. Une entrée par **asset**, jamais par tuile : toutes les tuiles
 * d'un même type animé restent ainsi en phase, sans coût par case.
 * @param skins           Catalogue de skins courant, ou `nullptr` (rien à faire).
 * @param skinSet         Nom du jeu courant ; vide pour le jeu par défaut du catalogue.
 * @param cache           Cache de textures (résolution de l'asset et de sa description).
 * @param deltaSeconds    Durée à avancer.
 * @param tileAnimations  Horloge partagée, mise à jour en place.
 * @param warnedExclusions Assets déjà signalés pour une combinaison exclue (mémorisation, pour ne
 *                        jamais journaliser à chaque image).
 */
void advanceTileAnimations(const SkinCatalog* skins, const std::string& skinSet,
                           TextureCache& cache, float deltaSeconds,
                           std::unordered_map<std::string, core::Animation>& tileAnimations,
                           std::set<std::string>& warnedExclusions);

/// Sous-dossier des fonds de niveau, relatif au dossier d'assets (`LOT-44`).
inline const std::string BACKGROUNDS_SUBDIRECTORY = "Backgrounds/";

/// Sous-dossier des textures d'objets interactifs, relatif au dossier d'assets (`LOT-45`).
inline const std::string OBJECTS_SUBDIRECTORY = "Objects/";

/**
 * @brief Textures liables par la composition d'une scène : atlas, damier de repli et skins.
 *
 * Point d'assemblage unique, partagé par le jeu et l'éditeur (`LOT-41`) : le damier est résolu
 * **à la demande** auprès du cache, donc créé seulement si un rendu en mode Texture a lieu. Les
 * skins du jeu courant sont chargés par le même cache, qui ne relit le disque qu'au premier accès
 * et mémorise aussi les échecs — appeler cette fonction à chaque image reste donc bon marché.
 * @param atlas   Atlas du jeu.
 * @param cache   Cache de textures, propriétaire du damier partagé et des skins.
 * @param skins   Catalogue des jeux de skins, ou `nullptr` si aucun n'est chargé (`LOT-42`).
 * @param skinSet Nom du jeu de skins courant ; vide pour le jeu par défaut du catalogue.
 * @param textureOverrides Surcharges de texture par instance du niveau courant (`EX-EDIT-043`,
 *                 `LOT-45`) ; chaque asset distinct est chargé une fois (`hmi::SceneTextures::
 *                 objects`), un asset déjà chargé ou introuvable étant silencieusement ignoré (la
 *                 résolution retombe sur le damier, avertissement déjà journalisé par le cache).
 * @param tileAnimations Horloge d'animation partagée par asset de tuile (`LOT-46` TACHE-05,
 *                 `GameSession::updateTileAnimations`, avancée au **pas fixe**) : un skin en mode
 *                 `SkinMode::Single`, sans silhouette, dont l'asset est animé et présent dans
 *                 cette table échantillonne l'image **courante** plutôt que l'image entière.
 * @return Les textures et leurs dimensions, prêtes pour `hmi::composeWorldSprites`.
 */
[[nodiscard]] SceneTextures sceneTextures(
    const TextureAtlas& atlas, TextureCache& cache, const SkinCatalog* skins = nullptr,
    const std::string& skinSet = {},
    const std::vector<core::TileTextureOverride>& textureOverrides = {},
    const std::unordered_map<std::string, core::Animation>& tileAnimations = {});

/**
 * @brief Résout la texture de fond d'un niveau (accès `TextureCache`/GPU, `LOT-44`).
 *
 * Point de résolution unique du repli en damier pour le fond (`hmi::resolveOrPlaceholder`,
 * `EX-NFR-040`) : un asset absent ou invalide bascule sur le damier magenta, après l'avertissement
 * déjà journalisé par le cache. Aucun accès cache n'a lieu si @p background est absent — même
 * séparation résolution/composition que `hmi::sceneTextures` vis-à-vis de
 * `hmi::composeWorldSprites`.
 * @param background Nom de l'asset de fond du niveau (`core::Level::background`), ou absent.
 * @param cache      Cache de textures, résout l'asset (et son repli en damier).
 * @return La texture à composer (`hmi::composeBackground`) ; `texture == nullptr` si aucun fond
 *         n'est désigné.
 */
[[nodiscard]] BackgroundTexture resolveBackgroundTexture(
    const std::optional<std::string>& background, TextureCache& cache);

/**
 * @brief Pont ECS → écran : dessine chaque entité affichable, triée par calque puis par texture.
 *
 * À chaque frame, **compose** la scène (`hmi::composeWorldSprites` : parcours des entités
 * `core::Transform` + `core::Sprite`, régions d'atlas, interpolation, culling — logique pure) puis
 * la **soumet** au `SpriteBatch`. Objet de **présentation** (`HMI`) : il **lit** l'ECS sans jamais
 * le muter (`EX-ARCH-012`). Ce n'est pas un `core::ISystem` : le rendu est découplé de la
 * simulation (`EX-REN-021`).
 *
 * La séparation composition/soumission (`LOT-40`) est ce qui rend l'ordre de dessin assertable
 * sans GPU : `lastScene()` expose la liste exacte des primitives de la dernière image
 * (`EX-NFR-004`).
 */
class SpriteRenderer {
public:
    /**
     * @brief Construit le rendu de sprites.
     * @param batch Pipeline de quads texturés utilisé pour dessiner (non possédé).
     * @param atlas Atlas fournissant la texture et ses dimensions (non possédé).
     * @param cache Cache de textures, propriétaire du damier de repli lié en mode Texture
     *              (non possédé, `LOT-41`).
     */
    SpriteRenderer(SpriteBatch& batch, const TextureAtlas& atlas, TextureCache& cache);

    /**
     * @brief Dessine toutes les entités affichables du monde, vues par la caméra.
     *
     * Seules les entités intersectant le cadrage de la caméra (marge comprise) sont soumises
     * (`EX-NFR-005`) ; une entité écartée reste simulée normalement (`EX-ARCH-012`).
     * @param world  Monde dont on lit les composants `Transform` et `Sprite`.
     * @param camera Caméra fournissant la projection monde → écran et le cadrage visible.
     * @param mode   Mode de rendu courant (`EX-REN-046`) : bascule purement **visuelle**, sans
     *               effet sur la scène ECS ni sur la simulation.
     * @param interpolationAlpha Facteur d'interpolation `[0, 1[` entre le pas de simulation
     *        précédent et le pas courant (`EX-ARCH-031`,
     * `core::FixedTimestep::interpolationAlpha`). Une entité portant un `hmi::PreviousPosition` est
     * dessinée à `lerp(précédente, courante, alpha)` (mouvement lisse) ; une entité sans ce
     * composant (tuiles fixes) est dessinée à sa position courante. `0` reproduit le comportement
     * non interpolé.
     * @param background  Nom de l'asset de fond du niveau (`core::Level::background`, `LOT-44`),
     *                     ou absent ; composé sous tout le reste (`RenderLayer::Background`),
     *                     uniquement en mode `RenderMode::Texture`.
     * @param levelWidth  Largeur du niveau, en unités monde (une case = une unité) ; ignoré si
     *                    @p background est absent.
     * @param levelHeight Hauteur du niveau, en unités monde.
     * @param textureOverrides Surcharges de texture par instance du niveau courant (`EX-EDIT-043`,
     *                    `LOT-45`), transmises telles quelles à `hmi::sceneTextures`.
     * @param tileAnimations Horloge d'animation partagée par asset de tuile (`LOT-46` TACHE-05),
     *                    transmise telle quelle à `hmi::sceneTextures`.
     * @param planes      Plans picturaux du niveau courant (`EX-DEC-040`, `LOT-69`), composés
     *                    avant tout le reste et dans l'ordre déclaré — c'est cet ordre de
     *                    composition qui porte leur ordre de dessin (cf. `hmi::composePlanes`).
     *                    Leurs images sont résolues sous `planesDirectory()`.
     * @param planeParallax Décide du décalage de parallaxe des plans (`hmi::planeParallaxActive`,
     *                    `EX-DEC-043`) : l'appelant tranche, la table étant une règle du moteur
     *                    croisée avec un drapeau de niveau, pas une propriété du rendu.
     * @param doorCollision Grille de collision courante des mécanismes (`core::
     *                    MechanismController::collisionMap`, `LOT-55`), pour que l'ombre d'une
     *                    porte suive son état ouverte/fermée plutôt que son type statique ;
     *                    `nullptr` (défaut) exclut les portes de l'ombrage.
     */
    void render(core::World& world, const Camera2D& camera, RenderMode mode,
                float interpolationAlpha, const std::optional<std::string>& background = {},
                int levelWidth = 0, int levelHeight = 0,
                const std::vector<core::TileTextureOverride>& textureOverrides = {},
                const std::unordered_map<std::string, core::Animation>& tileAnimations = {},
                const std::vector<core::Plane>& planes = {}, bool planeParallax = false,
                const core::TileMap* doorCollision = nullptr);

    /**
     * @brief Fixe le dossier où résoudre les images de plans (`Levels/Plans`).
     *
     * Un plan est une donnée de **niveau**, pas un asset : son image ne vit pas sous `Assets/` et
     * ne passe donc pas par la résolution de noms logiques du cache.
     * @param directory Dossier des images de plans.
     */
    void setPlanesDirectory(std::filesystem::path directory) {
        _planesDirectory = std::move(directory);
    }

    /// @return La scène composée à la dernière image (primitives soumises et compteurs).
    [[nodiscard]] const ComposedScene& lastScene() const noexcept {
        return _scene;
    }

    /**
     * @brief Désigne le catalogue de skins et le jeu à utiliser en mode Texture (`LOT-42`).
     *
     * Le catalogue n'est **pas** copié : l'appelant en reste propriétaire et doit le maintenir en
     * vie. Réassigner un skin dans le catalogue se voit à l'image suivante, sans reconstruire la
     * scène ECS — c'est tout l'intérêt de résoudre l'apparence à la composition.
     * @param skins   Catalogue, ou `nullptr` pour retomber entièrement sur le damier.
     * @param skinSet Nom du jeu courant ; vide pour le jeu par défaut du catalogue.
     */
    void setSkins(const SkinCatalog* skins, std::string skinSet = {}) {
        _skins = skins;
        _skinSet = std::move(skinSet);
    }

private:
    /// Journalise les compteurs de l'image **seulement** s'ils ont changé depuis la précédente :
    /// le volume soumis reste observable (`EX-NFR-005`) sans écrire une ligne par image, ce que
    /// `GraphicsLog` proscrit sur un chemin de dessin.
    void logStatisticsIfChanged();

    /// Dossier des images de plans (`setPlanesDirectory`), vide tant qu'aucun n'est fixé.
    std::filesystem::path _planesDirectory;
    SpriteBatch* _batch;                  // non possédé
    const TextureAtlas* _atlas;           // non possédé
    TextureCache* _cache;                 // non possédé
    const SkinCatalog* _skins = nullptr;  // non possédé
    std::string _skinSet;
    ComposedScene _scene;
    SceneStatistics _loggedStatistics;
};

}  // namespace hmi
