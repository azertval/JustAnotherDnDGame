// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "Core/Ecs/Systems/AnimationSystem.h"
#include "Core/Ecs/Systems/ParticleSystem.h"
#include "Core/Ecs/World.h"
#include "Core/Gameplay/MechanismController.h"
#include "Core/Levels/CameraFraming.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/PlayerInput.h"
#include "HMI/Game/GameEvents.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/CameraZones.h"
#include "HMI/Graphics/FollowCamera.h"
#include "HMI/Graphics/MechanismVisuals.h"
#include "HMI/Graphics/RoomGrid.h"
#include "HMI/Graphics/SpriteRenderer.h"
#include "HMI/Input/GameKeyBindings.h"
#include "HMI/Input/GamepadBindings.h"

/**
 * @file HMI/Game/GameSession.h
 * @brief Session de jeu réutilisable : simule et rend **un** niveau, sans dépendance d'écran.
 */

namespace hmi {

class SpriteBatch;
class TextureAtlas;
class TextureCache;
class InputState;
class BitmapFont;
class Localization;

/**
 * @brief Simule et rend **un seul niveau** (déjà validé, en mémoire), indépendamment de toute
 *        infrastructure d'écran.
 *
 * Extraite de l'écran de jeu historique (`LOT-34`) pour être utilisée par l'éditeur Qt (viewport) :
 * toute la logique de jeu (chargement de la scène ECS, apparition du personnage, physique,
 * mécanismes, blocs, dangers, caméra par salle, animation, interpolation) vit ici, une seule fois.
 *
 * `update()` avance d'un **pas fixe** et renvoie l'issue (`core::LevelOutcome`) ; sur un **échec**
 * (danger/chute), le niveau est **rechargé** en interne (personnage à l'entrée, mécanismes/budgets
 * remis) — l'appelant décide quoi faire d'une **réussite** (enchaîner, revenir au menu, etc.).
 * `render()` dessine la scène via le `SpriteRenderer` (lecture seule de l'ECS, `EX-ARCH-012`), avec
 * interpolation (`EX-ARCH-031`). Un échec de chargement est **récupérable** (`EX-NFR-040`) : la
 * session reste dans un état neutre, `loaded()` est faux et `loadError()` renseigné.
 */
class GameSession {
public:
    /**
     * @brief Construit la session et charge la scène du niveau donné.
     * @param batch           Lot de sprites partagé (rendu).
     * @param atlas           Atlas de tuiles fournissant les régions de sprites.
     * @param viewportWidth   Largeur initiale de la surface de rendu, en pixels.
     * @param viewportHeight  Hauteur initiale de la surface de rendu, en pixels.
     * @param cache           Cache de textures, propriétaire du damier de repli du mode Texture
     *                        (référence conservée, doit survivre à la session, `LOT-41`).
     * @param level           Niveau déjà validé à jouer.
     * @param gameBindings    Touches clavier de jeu (référence conservée, doit survivre à la
     * session).
     * @param gamepadBindings Boutons manette de jeu (référence conservée, doit survivre à la
     * session).
     * @param font          Police bitmap de l'affichage tête haute (référence conservée, doit
     *                      survivre à la session, `LOT-52`).
     * @param localization  Catalogue de traduction des libellés du HUD (`EX-REN-033`) ;
     *                      `nullptr` désactive le HUD sans plantage (`EX-NFR-040`), état de
     *                      démarrage légitime avant que le catalogue ne soit chargé.
     */
    GameSession(SpriteBatch& batch, const TextureAtlas& atlas, TextureCache& cache,
                int viewportWidth, int viewportHeight, core::Level level,
                const GameKeyBindings& gameBindings, const GamepadBindings& gamepadBindings,
                const BitmapFont& font, const Localization* localization = nullptr);

    /**
     * @brief Simule le niveau d'un pas fixe (mécanismes, blocs, physique, animation, dangers).
     * @param input      État des entrées de ce pas.
     * @param fixedDelta Durée du pas fixe, en secondes.
     * @return L'issue : `Playing`, `Won` (l'appelant décide), ou `Lost` (niveau déjà rechargé).
     */
    core::LevelOutcome update(const InputState& input, float fixedDelta);

    /**
     * @brief Simule le niveau d'un pas fixe à partir d'une intention déjà résolue, sans passer par
     *        `hmi::InputState`/`toPlayerInput` (`LOT-ANNEXE-18`, TACHE-01) : cœur de simulation
     *        partagé, utilisé aussi bien par le jeu réel (via `update(const InputState&, float)`,
     *        qui ne fait plus que traduire puis déléguer ici) que par la lecture d'un rejeu
     *        (`hmi::ReplayPlayback`), sans aucune divergence possible entre les deux chemins.
     * @param input      Intention de jeu déjà résolue (entrée réelle traduite, ou rejeu
     * enregistré).
     * @param fixedDelta Durée du pas fixe, en secondes.
     * @return L'issue : `Playing`, `Won` (l'appelant décide), ou `Lost` (niveau déjà rechargé).
     */
    core::LevelOutcome update(const core::PlayerInput& input, float fixedDelta);

    /// Recharge le niveau courant (personnage à l'entrée, mécanismes et budgets remis).
    void reload();

    /// Dessine la scène du niveau (rien si le chargement a échoué). @p mode choisit le rendu
    /// Physique ou Texture (`EX-REN-046`, `LOT-41`) : bascule purement visuelle, sans effet sur la
    /// simulation ni sur la scène ECS.
    void render(int viewportWidth, int viewportHeight, RenderMode mode, float interpolationAlpha);

    /**
     * @brief Désigne le catalogue de skins et le jeu à utiliser en mode Texture (`LOT-42`).
     *
     * Le catalogue n'est **pas** copié : l'appelant en reste propriétaire et doit le maintenir en
     * vie au moins aussi longtemps que la session.
     * @param skins   Catalogue, ou `nullptr` pour retomber entièrement sur le damier.
     * @param skinSet Nom du jeu courant ; vide pour le jeu par défaut du catalogue.
     */
    void setSkins(const SkinCatalog* skins, std::string skinSet = {}) {
        _tileSkins = skins;
        _tileSkinSet = skinSet;
        _renderer.setSkins(skins, std::move(skinSet));
    }

    /// @return true si un niveau est chargé et simulable.
    [[nodiscard]] bool loaded() const {
        return _level.has_value();
    }

    /// @return Message d'erreur si le chargement a échoué, chaîne vide sinon.
    [[nodiscard]] const std::string& loadError() const {
        return _loadError;
    }

    /**
     * @brief Événements de jeu détectés lors du **dernier** appel à `update()` (`LOT-60 TACHE-03`).
     *
     * Transitions du personnage et des mécanismes, plus l'issue du pas (`Died`/`LevelCompleted`).
     * Vide si aucune transition, ou lors du tout premier pas après un chargement (état initial,
     * rien à signaler — même principe que `MechanismVisualState::initialized`). Présentation pure :
     * ne modifie jamais la simulation (`EX-ARCH-012`), à consommer par l'appelant pour déclencher
     * des sons (`hmi::SoundTriggers`) ou, plus tard, des particules (`LOT-53`).
     */
    [[nodiscard]] const std::vector<GameEvent>& lastStepEvents() const noexcept {
        return _lastStepEvents;
    }

    /// @return Les compteurs de primitives de la **dernière** image rendue (`hmi::SpriteRenderer::
    /// lastScene`, `EX-NFR-005`) : composées, écartées, soumises, passes de dessin — consommé par
    /// le compteur de diagnostic (`hmi::DiagnosticsHud`, `LOT-62` TACHE-02).
    [[nodiscard]] SceneStatistics renderStatistics() const noexcept {
        return _renderer.lastScene().statistics();
    }

private:
    void loadLevel(core::Level level);
    void spawnPlayer(core::GridPosition entry);
    void snapshotPreviousPositions();
    /// Apparence des mécanismes pilotée par leur état logique (`LOT-47`, `EX-REN-006`) : projette
    /// l'état de chaque mécanisme suivi sur un clip (correspondance + transitions), au **pas fixe**
    /// — la simulation n'en dépend jamais, seule l'apparence en résulte (`EX-ARCH-012`).
    void updateMechanismVisuals(float fixedDelta);
    /// Applique la correspondance état → clip à **une** entité-tuile de mécanisme : résout l'asset
    /// effectivement lié (via le point de résolution unique `hmi::resolveTileAppearance`, jamais
    /// dupliqué ici), avance son horloge propre et écrit l'image courante sur son `TileSkinTag`
    /// (`LOT-47` TACHE-02). Sans effet (répli sur l'image statique existante) si l'entité n'est pas
    /// repérée, si son type n'est pas un mécanisme à état, ou si l'asset résolu n'est pas animé.
    void applyMechanismVisual(core::Entity entity, bool active, MechanismVisualState& state,
                              const SceneTextures& textures, float fixedDelta);
    /// Modulation d'opacité de **diagnostic**, réservée au mode Physique (`LOT-47` TACHE-03) : ce
    /// mode n'a pas d'assets animés et doit conserver un moyen de distinguer un mécanisme actif
    /// d'un mécanisme inactif. Appelée à chaque `render()` (décision purement visuelle, dépendante
    /// du mode courant) ; force l'alpha à 1 en mode Texture, où l'état se voit désormais au clip.
    void refreshMechanismDiagnosticTint(RenderMode mode);
    /// @return Les boîtes mortelles **à état** du pas courant, en plus des tuiles `Danger` que
    ///         `core::evaluateOutcome` lit directement dans la grille : aujourd'hui la seule
    ///         porte qui se referme sur le personnage (`EX-GP-021`).
    [[nodiscard]] std::vector<core::Aabb> collectActiveDangerBoxes();
    void refreshPlayerSprite();
    void centerCameraOnRoom(core::GridPosition roomIndex);
    void updateCurrentRoom();
    /// Centre la caméra sur une zone dessinée à la main (mode `PerRoom`, `_cameraFraming.zones`
    /// non vide, `EX-LVL-007`) : symétrique à `centerCameraOnRoom`, pour le découpage manuel
    /// plutôt qu'automatique.
    void centerCameraOnZone(const core::CameraZone& zone);
    /// Équivalent de `updateCurrentRoom` quand `_cameraFraming.zones` est non vide : résout la
    /// zone active (`hmi::activeCameraZoneIndex`) plutôt que l'indice de grille de `_roomGrid`.
    /// Repli sur `centerCameraOnWholeLevel` si aucune zone ne contient le personnage (jamais un
    /// état indéfini).
    void updateCurrentCameraZone();
    /// Centre la caméra sur le niveau entier (mode `WholeLevel`, LOT-64) : centre fixe, posé une
    /// fois au chargement -- symétrique à `centerCameraOnRoom`, jamais recalculé au pas fixe.
    void centerCameraOnWholeLevel();
    /// Avance la caméra de suivi d'un pas fixe (mode `Follow`, LOT-64 TACHE-02) : lit la position
    /// simulée du personnage, sans jamais l'écrire (`EX-ARCH-012`).
    void updateFollowCamera(float fixedDelta);
    /// Sélectionne le zoom et le centre effectifs de `_camera` selon le mode de cadrage résolu du
    /// niveau (`core::CameraFramingMode`, LOT-64) : seul point d'application du cadrage, appelé
    /// une fois par image depuis `render()`.
    void applyCameraFraming(int viewportWidth, int viewportHeight, float interpolationAlpha);
    /// Avance l'horloge d'animation partagée des tuiles animées (`LOT-46` TACHE-05), au **pas
    /// fixe** : une entrée de `_tileAnimations` par asset animé du jeu de skins courant, en mode
    /// `SkinMode::Single` sans silhouette (`bitmask16` et silhouette détourée excluent
    /// l'animation, signalé une fois par asset via `_warnedExcludedAnimations`).
    void updateTileAnimations(float fixedDelta);
    /// Compose et soumet l'affichage tête haute (budgets, nom du tableau), en espace écran, sur
    /// sa propre projection (`hmi::screenProjectionMatrix`) — jamais affecté par le zoom de la
    /// caméra ni par son culling (`LOT-52` TACHE-02/03). Sans effet si `_localization` est nul.
    void renderHud(int viewportWidth, int viewportHeight);

    const TextureAtlas& _atlas;
    TextureCache& _cache;
    const GameKeyBindings& _gameBindings;
    const GamepadBindings& _gamepadBindings;
    SpriteBatch& _batch;
    const BitmapFont& _font;
    const Localization* _localization;  // non possede ; nul = HUD desactive (EX-NFR-040)
    core::World _world;
    Camera2D _camera;
    SpriteRenderer _renderer;
    /// Scène composée du HUD, distincte de celle de `_renderer` (`LOT-52` TACHE-02) : jamais de
    /// cadrage de culling actif dessus (`ComposedScene::setVisibleBounds`), le texte est en
    /// espace écran et n'a pas de position monde.
    ComposedScene _hudScene;
    std::optional<core::Level> _level;
    std::optional<core::MechanismController> _mechanisms;
    std::vector<core::Entity> _doorEntities;
    /// Entité-tuile du déclencheur (interrupteur/plaque de pression) de chaque mécanisme, même
    /// ordre que `_doorEntities` (`LOT-47` : le déclencheur change aussi d'apparence, plus
    /// seulement la porte qu'il actionne).
    std::vector<core::Entity> _switchEntities;
    core::AnimationSystem _animation;
    /// Particules du personnage (dash, atterrissage, mort ; `LOT-53` TACHE-02) : simulées au pas
    /// fixe, vidées à chaque (re)chargement (`loadLevel`) comme le reste de l'état de session.
    core::ParticleSystem _particles;
    /// Secousse d'écran courante (`LOT-53` TACHE-03) : déclenchée à un atterrissage lourd et à la
    /// mort, avancée au pas fixe, appliquée à la caméra de rendu uniquement (`render()`).
    ScreenShakeState _screenShake;
    core::Entity _player{};
    int _levelWidth = 0;
    int _levelHeight = 0;
    std::optional<RoomGrid> _roomGrid;
    core::GridPosition _currentRoomIndex{};
    /// Zone de caméra active courante (mode `PerRoom` avec zones dessinées à la main,
    /// `EX-LVL-007`) ;
    /// `std::nullopt` = aucune zone ne contient le personnage (repli niveau entier). Seulement
    /// significatif quand `_cameraFraming.zones` est non vide -- sinon `_roomGrid`/
    /// `_currentRoomIndex` font foi (découpage automatique en grille, inchangé).
    std::optional<std::size_t> _currentZoneIndex;
    /// Cadrage de caméra résolu du niveau courant (LOT-64), copié une fois au chargement --
    /// n'affecte jamais la simulation (`EX-ARCH-012`), seulement `applyCameraFraming`/
    /// `updateFollowCamera`.
    core::CameraFramingConfig _cameraFraming;
    /// État de la caméra de suivi (mode `Follow`), avancé au pas fixe par `updateFollowCamera`.
    FollowCameraState _followCameraState;
    /// Centre de la caméra de suivi au pas fixe PRÉCÉDENT, capturé par `snapshotPreviousPositions`
    /// -- même patron que `PreviousPosition` pour les entités : `render()` interpole entre les deux
    /// pour rester synchrone avec le personnage (lui-même interpolé), sinon il tremblerait par
    /// rapport au décor (`tache-02`).
    core::Vector2 _previousFollowCenter{};
    std::string _loadError;

    // Habillage des tuiles (LOT-46 TACHE-05) : copie locale de ce que setSkins() transmet aussi a
    // _renderer, necessaire pour decouvrir les assets animes au pas fixe (update()), independamment
    // du rendu (render()).
    const SkinCatalog* _tileSkins = nullptr;  // non possede
    std::string _tileSkinSet;
    /// Horloge d'animation partagee, une entree par asset de tuile anime (nom de fichier -> etat).
    /// Avancee une fois par asset et par pas fixe, jamais une fois par case (LOT-46 TACHE-05).
    std::unordered_map<std::string, core::Animation> _tileAnimations;
    /// Assets pour lesquels la combinaison (bitmask16 ou silhouette) + animation a deja ete
    /// signalee : evite de journaliser a chaque pas fixe (GraphicsLog proscrit un message par
    /// image/pas).
    std::set<std::string> _warnedExcludedAnimations;

    // Apparence des mecanismes pilotee par l'etat (LOT-47) : une horloge par instance suivie
    // (porte, declencheur, danger commute/temporise/mobile), independante de _tileAnimations
    // ci-dessus.
    std::vector<MechanismVisualState> _doorVisuals;
    std::vector<MechanismVisualState> _switchVisuals;
    std::vector<MechanismVisualState> _dangerSwitchedVisuals;
    std::vector<MechanismVisualState> _dangerBlinkVisuals;
    std::vector<MechanismVisualState> _dangerMoverVisuals;
    /// Couples (asset, clip attendu) deja signales comme manquants : un seul message par
    /// combinaison pour toute la session, meme principe que _warnedExcludedAnimations (LOT-47
    /// TACHE-02).
    std::set<std::string> _warnedMissingMechanismClips;

    // Détection d'événements (LOT-60 TACHE-03) : snapshots du pas précédent, pour ne diffuser une
    // transition qu'au pas où elle a réellement lieu (même discipline que MechanismVisualState
    // ci-dessus -- previousXxx/initialized).
    PlayerEventState _previousPlayerEventState;
    MechanismEventState _previousMechanismEventState;
    bool _gameEventsInitialized = false;
    std::vector<GameEvent> _lastStepEvents;
};

}  // namespace hmi
