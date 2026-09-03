// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Core/Ecs/Components/Animation.h"
#include "Core/Ecs/Entity.h"
#include "Core/Ecs/World.h"
#include "Core/Levels/GridPosition.h"
#include "HMI/Graphics/LayerVisibility.h"
#include "HMI/Graphics/PlaneVisibility.h"
#include "HMI/Graphics/SpriteRenderer.h"

/**
 * @file HMI/Graphics/DraftRenderer.h
 * @brief Rendu d'un brouillon d'édition (`core::LevelDraft`) dans le viewport (LOT-35).
 */

namespace core {
class LevelDraft;
}

namespace hmi {

class SpriteBatch;
class TextureAtlas;
class TextureCache;
class Camera2D;

/**
 * @brief État d'affichage des liens de mécanismes (LOT-37), fourni par le viewport à chaque rendu.
 *
 * Purement informatif pour `DraftRenderer` : ne modifie jamais le brouillon, seulement la
 * présentation (surbrillance, trait provisoire).
 */
struct LinkOverlayState {
    /// Case survolée par la souris (surbrillance des liens incidents ; extrémité du trait
    /// provisoire si `pendingLink` est posé).
    std::optional<core::GridPosition> hoveredCell;
    /// Case du déclencheur/de la cible en attente d'appariement (outil « Lien », premier clic).
    std::optional<core::GridPosition> pendingLink;
    /// Liaison sélectionnée dans le panneau « Liens » (déclencheur, cible) : mise en surbrillance.
    std::optional<std::pair<core::GridPosition, core::GridPosition>> selectedLink;
};


/**
 * @brief Dessine la grille d'un `core::LevelDraft` en cours d'édition.
 *
 * Réutilise le pipeline de rendu du jeu : chaque tuile non vide du brouillon devient une entité
 * (`Transform` + `Sprite`) d'un `core::World` interne, composée par `hmi::composeWorldSprites`
 * (lecture seule, `EX-ARCH-012`). Le `core::World` n'est **reconstruit** que lorsque le brouillon
 * change (`invalidate()`), pas à chaque frame. Les blocs à taille réduite sont dessinés à leur
 * échelle réelle (`core::tileVisualScale`), comme en jeu — cohérence visuelle stricte.
 *
 * Depuis `LOT-40`, **toutes** les primitives d'une image (tuiles, grille de repère, liens de
 * mécanismes, aperçu de sélection) sont composées dans une seule `hmi::ComposedScene` puis
 * soumises en bloc : les aides d'édition portent le calque `RenderLayer::EditorOverlay`, qui les
 * place au-dessus de tout le reste par construction plutôt que par l'ordre des appels de dessin.
 * La liste obtenue est donc inspectable sans GPU (`EX-NFR-004`) et soumise au culling
 * (`EX-NFR-005`).
 */
class DraftRenderer {
public:
    DraftRenderer(SpriteBatch& batch, const TextureAtlas& atlas, TextureCache& cache);

    /// Rend le brouillon avec la caméra donnée (reconstruit la scène si invalidée). Si @p showGrid,
    /// superpose la grille de repère (frontières de cases + frontières de salles) — aide au
    /// placement, équivalent de la bascule `F10` de l'éditeur historique (`EX-EDIT-023`). Si
    /// @p highlight est présent, met en surbrillance la zone (bornes min/max incluses) — aperçu de
    /// l'outil Rectangle/Sélection. @p linkOverlay pilote l'affichage des liens de mécanismes
    /// (flèches, trait provisoire, surbrillance — `EX-IHM-030`). @p mode choisit le rendu
    /// Physique ou Texture des tuiles (`EX-REN-046`, `LOT-41`) ; les **aides d'édition** (grille,
    /// liens, aperçu) restent identiques dans les deux modes — ce sont des repères d'édition, pas
    /// de l'habillage. Si @p showTextureOverrides, les cases portant une surcharge de texture par
    /// instance (`EX-EDIT-043`, `LOT-45`) sont signalées sur le calque d'édition — actif seulement
    /// quand l'outil « Texture par instance » l'est, pour ne pas encombrer les autres outils.
    /// @p deltaSeconds avance l'aperçu des tuiles animées (`LOT-46` TACHE-05) en **temps réel** —
    /// contrairement à la simulation (`hmi::GameSession`), l'aperçu d'édition n'a aucune exigence
    /// de déterminisme (`EX-NFR-002` ne s'applique qu'en jeu) ; `0` (par défaut) fige l'animation.
    /// Les aides d'édition, comme le reste du calque `RenderLayer::EditorOverlay`, ne sont
    /// **jamais** composées en jeu ni en essai (`hmi::GameSession` ne passe jamais par
    /// `DraftRenderer`). @p visibility pilote
    /// le mode d'inspection « définition des textures » (`LOT-51`, `EX-EDIT-044`) — tout visible
    /// par défaut, sans effet sur les aides d'édition ci-dessus (jamais un calque de contenu).
    void render(const core::LevelDraft& draft, const Camera2D& camera, bool showGrid,
                const std::optional<std::pair<core::GridPosition, core::GridPosition>>& highlight,
                const LinkOverlayState& linkOverlay, RenderMode mode,
                bool showTextureOverrides = false, float deltaSeconds = 0.0f,
                const LayerVisibility& visibility = {},
                const PlaneVisibility& planeVisibility = {});

    /**
     * @brief Fixe le dossier où résoudre les images de plans (`Levels/Plans`), comme
     *        `hmi::SpriteRenderer::setPlanesDirectory`.
     * @param directory Dossier des images de plans.
     */
    void setPlanesDirectory(std::filesystem::path directory) {
        _planesDirectory = std::move(directory);
    }

    /// Marque la scène comme périmée : elle sera reconstruite au prochain `render` (à appeler après
    /// toute mutation du brouillon — peinture, undo/redo, chargement, redimensionnement).
    void invalidate() noexcept {
        _dirty = true;
    }

    /// @return La scène composée à la dernière image (primitives soumises et compteurs).
    [[nodiscard]] const ComposedScene& lastScene() const noexcept {
        return _scene;
    }

    /**
     * @brief Désigne le catalogue de skins et le jeu à utiliser en mode Texture (`LOT-42`).
     *
     * Le catalogue n'est **pas** copié : l'appelant en reste propriétaire. Réassigner un skin s'y
     * voit à l'image suivante, sans reconstruire le brouillon ni la scène.
     * @param skins   Catalogue, ou `nullptr` pour retomber entièrement sur le damier.
     * @param skinSet Nom du jeu courant ; vide pour le jeu par défaut du catalogue.
     */
    void setSkins(const SkinCatalog* skins, std::string skinSet = {}) {
        _skins = skins;
        _skinSet = std::move(skinSet);
    }

private:
    void rebuild(const core::LevelDraft& draft);
    /// Compose la grille de repère (frontières de cases + de salles) sur le calque d'édition. Si
    /// @p accentuate, les lignes sont plus opaques — indication visuelle de l'aimantation active
    /// du geste de décors (`LOT-50` TACHE-03), sinon l'auteur ne comprend pas pourquoi sa position
    /// « saute ».
    void composeGrid(const core::LevelDraft& draft);
    /// Compose la prévisualisation du cadrage de caméra du niveau (`EX-EDIT-028`, LOT-64) sur le
    /// calque d'édition : le cadre du niveau entier, la grille de salles à taille résolue
    /// (`hmi::RoomGrid`), ou le rectangle visible et la zone morte matérialisée du mode suivi
    /// (centré sur l'entrée). Composée **inconditionnellement**, comme les liens de mécanismes et
    /// les poignées de décors — pas une aide de placement (`showGrid`), une information sur ce que
    /// montrera la caméra en jeu.
    void composeCameraFraming(const core::LevelDraft& draft);
    /// Compose les liens de mécanismes (flèches déclencheur → cible) sur le calque d'édition.
    void composeLinks(const core::LevelDraft& draft, const LinkOverlayState& overlay);
    /// Compose la course aller-retour de chaque danger mobile (`EX-GP-051`) sur le calque
    /// Compose les poignées de @p handles (carrés double ton, même patron que celles des décors).
    /// @return Un quad plein couvrant @p rect, texturé par la région unie de l'atlas et teinté —
    /// brique commune des aides d'édition rectangulaires (poignées de parcours).
    [[nodiscard]] SpriteQuad solidOverlayQuad(const core::Rect& rect, float r, float g, float b,
                                              float a) const;
    /// Compose le voile d'aperçu d'une zone (outil Rectangle/Sélection) sur le calque d'édition.
    void composeHighlight(const core::GridPosition& minimum, const core::GridPosition& maximum);
    /// Signale les cases portant une surcharge de texture par instance sur le calque d'édition
    /// (`EX-EDIT-043`, `LOT-45`).
    void composeTextureOverrideMarkers(const core::LevelDraft& draft);
    /// Dossier des images de plans (`setPlanesDirectory`), vide tant qu'aucun n'est fixé.
    std::filesystem::path _planesDirectory;
    SpriteBatch& _batch;
    const TextureAtlas& _atlas;
    TextureCache& _cache;
    const SkinCatalog* _skins = nullptr;  // non possédé
    std::string _skinSet;
    ComposedScene _scene;
    core::World _world;
    bool _dirty = true;
    /// Horloge d'animation partagee par asset (LOT-46 TACHE-05), avancee en temps reel a chaque
    /// render() -- distincte de celle de GameSession (pas fixe) : l'apercu d'edition n'a pas
    /// besoin d'etre deterministe.
    std::unordered_map<std::string, core::Animation> _tileAnimations;
    /// Assets deja signales pour une combinaison exclue (bitmask16/silhouette + animation) :
    /// memorise pour ne jamais journaliser a chaque image.
    std::set<std::string> _warnedExcludedAnimations;
};

}  // namespace hmi
