// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QRhiWidget>
#include <QString>
#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/LevelDraft.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Rect.h"
#include "Core/Time/FixedTimestep.h"
#include "HMI/Editor/EditContextTarget.h"
#include "HMI/Editor/EditorTool.h"
#include "HMI/Game/DiagnosticsHud.h"
#include "HMI/Game/GameSession.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/LayerVisibility.h"
#include "HMI/Graphics/PlaneVisibility.h"
#include "HMI/Graphics/RenderMode.h"
#include "HMI/Graphics/SceneResources.h"
#include "HMI/Graphics/SkinCatalog.h"
#include "HMI/Input/EditorKeyBindings.h"
#include "HMI/Input/GameKeyBindings.h"
#include "HMI/Input/GamepadBindings.h"
#include "HMI/Input/GamepadPoller.h"
#include "HMI/Input/InputState.h"

/**
 * @file HMI/Game/GameViewport.h
 * @brief Viewport de l'éditeur : rendu du brouillon d'édition et de l'essai sur QRhi
 * (LOT-34/35, porté au LOT-69 TACHE-02).
 */

namespace hmi {
class SpriteBatch;
class TextureAtlas;
class TextureCache;
class DraftRenderer;
class Localization;
class BitmapFont;
class AudioEngine;
}  // namespace hmi

namespace hmi {

/**
 * @brief Widget de rendu (`QRhiWidget`) où la scène est dessinée, composé avec le reste de
 *        l'interface.
 *
 * **Widget, et non fenêtre native, depuis le `LOT-69` TACHE-02** (`EX-REN-050`). Règle tenue par
 * ce choix : **aucun recouvrement de l'IHM ne doit dépendre d'un empilement de fenêtres natives**.
 * `QRhiWidget` rend dans une texture d'appui composée avec les autres widgets, donc l'empilement
 * est celui, ordinaire, de Qt : les recouvrements (pause, fin de niveau) sont de simples enfants,
 * dont la visibilité et le focus suivent les règles habituelles des widgets.
 *
 * La cible technique ne change pas : QRhi retient **Direct3D 11** par défaut sous Windows
 * (`EX-REN-002`, amendée — *au travers de QRhi* plutôt qu'en appelant l'API directement).
 *
 * **Mode édition** (LOT-35) : affiche le brouillon (`core::LevelDraft`) via `hmi::DraftRenderer`,
 * caméra cadrant le niveau entier par défaut. Le clic/glisser gauche **peint** le type de tuile
 * actif (`setActiveTile`, fourni par la palette) à la case survolée (`core::LevelDraft::
 * paintTile`) ; `Ctrl+Z`/`Ctrl+Y` annulent/refont. La molette **zoome** manuellement (borné entre
 * l'ajustement automatique et 4 cases visibles sur le plus petit axe) et le glisser **bouton
 * droit** déplace la caméra (« pan ») ; `0` revient au cadrage automatique (LOT-15). **Mode essai**
 * (LOT-35 TACHE-04) : une `hmi::GameSession` rejoue le niveau ; sa boucle à pas fixe
 * (`core::FixedTimestep`) et ses entrées (Qt + XInput) reprennent la discipline du `LOT-33`.
 *
 * Implémente `hmi::EditContextTarget` (`LOT-57` TACHE-04) : c'est aujourd'hui l'unique cible
 * d'Annuler/Refaire/Copier/Coller que `MainWindow` connaisse, au travers de cette interface plutôt
 * que d'un appel direct — le seuil de dispatch que le futur atelier pixel art (`LOT-54`)
 * réutilisera pour sa propre cible, sans le réécrire.
 */
class GameViewport : public QRhiWidget, public EditContextTarget {
    Q_OBJECT

public:
    explicit GameViewport(QWidget* parent = nullptr);
    ~GameViewport() override;

    GameViewport(const GameViewport&) = delete;
    GameViewport& operator=(const GameViewport&) = delete;

    /// Définit le type de tuile peint au clic (relié à la sélection de la palette).
    void setActiveTile(core::TileType type) noexcept {
        _activeTile = type;
    }

    /// Définit l'asset assigné au clic par l'outil « Texture par instance » (`LOT-45`), relié à la
    /// sélection de la bibliothèque « Objets » ; vide si aucun asset n'est sélectionné.
    void setActiveTextureAsset(std::optional<std::string> asset) noexcept {
        _activeTextureAsset = std::move(asset);
    }

    /**
     * @brief Définit l'outil d'édition actif (pinceau, rectangle, sélection, lien, texture par
     *        instance).
     *
     * Émet `toolChanged` si l'outil change réellement, pour que la barre d'outils reste
     * synchronisée quand le changement vient d'ailleurs qu'un clic sur son panneau — le raccourci
     * clavier de l'outil « Texture par instance » (`LOT-45`), aujourd'hui seul cas.
     */
    void setTool(hmi::EditorTool tool);

    /// Fournit le catalogue de traduction (les messages d'état émis sont alors localisés).
    void setLocalization(const Localization* loc) noexcept {
        _loc = loc;
    }

    /**
     * @brief Fournit le moteur audio (`LOT-60` TACHE-03) : les événements de jeu détectés par
     *        `hmi::GameSession` (saut, atterrissage, dash, mécanismes, mort, victoire) déclenchent
     *        alors un son. `nullptr` (défaut) désactive silencieusement les sons de jeu, sans
     *        plantage (`EX-NFR-040`) — état de démarrage légitime avant que `MainWindow` n'ait
     *        construit son moteur. Non possédé : l'appelant (`MainWindow`) reste propriétaire.
     */
    void setAudioEngine(AudioEngine* engine) noexcept {
        _audioEngine = engine;
    }

    /// Accès **modifiable** aux touches de jeu (pour le remappage) : la session de jeu lit ces
    /// mêmes bindings, donc un changement s'applique immédiatement.
    [[nodiscard]] hmi::GameKeyBindings& gameBindings() noexcept {
        return _gameBindings;
    }

    /// Accès **modifiable** aux boutons manette (pour le remappage), même principe que
    /// `gameBindings`.
    [[nodiscard]] hmi::GamepadBindings& gamepadBindings() noexcept {
        return _gamepadBindings;
    }

    /// Accès **modifiable** aux touches d'éditeur (pour le remappage, `LOT-57` TACHE-04), même
    /// principe que `gameBindings`.
    [[nodiscard]] hmi::EditorKeyBindings& editorBindings() noexcept {
        return _editorBindings;
    }

    /// Enregistre le brouillon (`Ctrl+S`) : valide (`LevelDraft::toLevel`) puis écrit le fichier ;
    /// un brouillon invalide n'écrit rien et rapporte l'erreur (`statusMessage`).
    void save();

    /// Ouvre un fichier de niveau comme brouillon d'édition (remplace le brouillon courant).
    void openLevel(const std::filesystem::path& path);

    /// @return true si le brouillon a des modifications non enregistrées (garde-fou d'ouverture).
    [[nodiscard]] bool isDirty() const noexcept {
        return _dirty;
    }

    /**
     * @brief Renomme le niveau **ouvert** (action Rename, `F2`, `LOT-57` TACHE-04).
     *
     * Renomme le fichier existant sur disque (`hmi::LevelFileOperations::rename`, même chemin que
     * `LevelBrowserPanel::onRename`) si le niveau a déjà été enregistré au moins une fois ; sinon,
     * seul le nom du brouillon change (rien à renommer sur disque). Sans effet si @p newName est
     * invalide (`hmi::isValidLevelName`) ou déjà pris par un autre fichier.
     * @param newName Nouveau nom, tel que saisi (espaces de bord retirés avant validation).
     * @return true si le renommage a été appliqué.
     */
    bool renameOpenLevel(const std::string& newName);

    /// Lance l'essai immédiat (`P`) sur un brouillon valide ; message d'erreur sinon.
    void startPlaytest();

    /// Annule la dernière modification du brouillon (`Ctrl+Z`) ; sans effet si rien à annuler.
    void undo() override;
    /// Refait la dernière modification annulée (`Ctrl+Y`) ; sans effet si rien à refaire.
    void redo() override;
    /// @return true s'il existe une modification à annuler (`EditContextTarget`).
    [[nodiscard]] bool canUndo() const override {
        return _draft.canUndo();
    }
    /// @return true s'il existe une modification annulée à refaire (`EditContextTarget`).
    [[nodiscard]] bool canRedo() const override {
        return _draft.canRedo();
    }
    /// Copie la sélection courante dans le presse-papiers local (`Ctrl+C`, `EditContextTarget`).
    void copy() override {
        copySelection();
    }
    /// Colle le presse-papiers à la case survolée (`Ctrl+V`, `EditContextTarget`).
    void paste() override {
        pasteClipboard();
    }
    /// @return true si une sélection existe à copier (`EditContextTarget`).
    [[nodiscard]] bool canCopy() const override {
        return _selection.has_value();
    }
    /// @return true si le presse-papiers local contient une zone à coller (`EditContextTarget`).
    [[nodiscard]] bool canPaste() const override {
        return !_clipboard.empty();
    }
    /// Bascule l'affichage de la grille de repère (`F10`).
    void toggleGrid() noexcept;
    /// Réinitialise le cadrage manuel au cadrage automatique (touche `0`).
    void resetCamera() noexcept;
    /// Bascule le mode de rendu Physique/Texture (`F8`, `EX-REN-046`) ; actif en édition, en essai
    /// et en jeu réel (LOT-56 TACHE-04 : seule commande jamais désactivée par le mode courant).
    void toggleRenderMode();

    /**
     * @brief Bascule le compteur de diagnostic (cadence, primitives, passes, pas de simulation ;
     *        `F9`, `LOT-62` TACHE-02, `EX-NFR-001`/`EX-NFR-005`).
     *
     * Touche dédiée **non remappable**, même statut que `F8` (`toggleRenderMode`) : désactivé par
     * défaut, sans effet sur la simulation (`EX-ARCH-012`). La fenêtre de mesure de cadence repart
     * de zéro à chaque activation (`hmi::FrameRateAverage::reset`) plutôt que de mélanger un temps
     * accumulé pendant que rien n'était mesuré.
     */
    void toggleDiagnosticsOverlay() noexcept;

    /// @return Le bilan du tableau en cours (`LOT-68`) : pas de simulation, morts, sauts. Remis à

    /// @return `true` si le compteur de diagnostic est actuellement affiché.
    [[nodiscard]] bool diagnosticsOverlayEnabled() const noexcept {
        return _diagnosticsEnabled;
    }

    /// Affiche ou masque le compteur de diagnostic. Même état que `toggleDiagnosticsOverlay` et
    /// que la touche `F9` — **un seul** état, atteint par deux chemins (`EX-IHM-062`), et non deux
    /// réglages qui pourraient diverger.
    void setDiagnosticsOverlayEnabled(bool enabled) noexcept;

    /// Lance le **jeu** sur la carte @p map. `Échap` ouvre la pause, dont « Quitter vers le menu »
    /// émet `exitToMenuRequested`.
    ///
    /// **Une carte, et non plus une séquence** (`LOT-67`). Le jeu visé est un bac à sable : il n'a
    /// ni ordre de tableaux, ni tableau suivant. L'enchaînement d'une carte à l'autre reviendra
    /// avec le **graphe de cartes** du `LOT-09`, et il sera déclenché par une transition placée
    /// dans le monde, pas par la fin d'une liste.
    void startGame(std::filesystem::path map);

    /// Suspend la simulation (écran de pause, `LOT-59` TACHE-02) : `tick()` cesse d'alimenter
    /// l'accumulateur de pas fixe -- aucun pas n'est consommé pendant la pause (`EX-GP-041`). Le
    /// rendu continue (la scène reste dessinée derrière l'écran de pause).
    void pauseSimulation() noexcept;
    /// Reprend la simulation après `pauseSimulation()` : réarme l'horloge de référence
    /// (`_previousFrame`) sur l'instant courant, faute de quoi le temps passé en pause serait
    /// rattrapé d'un coup au pas suivant (piège documenté par TACHE-02).
    void resumeSimulation();
    /// @return true si la simulation est actuellement suspendue (`pauseSimulation`).
    [[nodiscard]] bool simulationPaused() const noexcept {
        return _paused;
    }
    /// Abandonne la partie en cours (« Quitter vers le menu » depuis la pause, `LOT-59` TACHE-02,
    /// après confirmation côté appelant) : même nettoyage que l'ancienne sortie directe par
    /// `Échap`, désormais déclenchée par l'écran de pause plutôt que par la touche elle-même.
    void quitGame() noexcept;

    /// Active/désactive la synchronisation verticale (`EX-REN-022`). Depuis le portage QRhi, la
    /// présentation appartient au compositeur de Qt : le réglage est conservé et rapporté, mais
    /// c'est Qt qui cale l'image sur le rafraîchissement.
    void setVSync(bool enabled) noexcept;
    /// @return true si la V-Sync est active.
    [[nodiscard]] bool vsyncEnabled() const noexcept {
        return _vsync;
    }

    /// Redimensionne le niveau en cours d'édition (`EX-EDIT-005`).
    void resizeLevel(int width, int height);

    /// @return true si redimensionner à (@p width, @p height) supprimerait du contenu posé.
    [[nodiscard]] bool wouldResizeDrop(int width, int height) const;

    /// @return Dimensions courantes du niveau, en cases.
    [[nodiscard]] int levelWidth() const;
    [[nodiscard]] int levelHeight() const;

    /// @return Le brouillon en cours d'édition (lecture seule) — consommé par le panneau « Liens ».
    [[nodiscard]] const core::LevelDraft& draft() const noexcept {
        return _draft;
    }

    /// @return Le plan pictural sélectionné dans le panneau « Plans » (`LOT-69`), si un l'est.
    [[nodiscard]] std::optional<std::size_t> selectedPlaneIndex() const noexcept {
        return _selectedPlane;
    }

    /// @return La visibilité courante des plans (aide d'édition, jamais persistée).
    [[nodiscard]] const PlaneVisibility& planeVisibility() const noexcept {
        return _planeVisibility;
    }

    /// @return La case actuellement survolée par le curseur, absente si hors de la grille ou si le
    /// curseur a quitté le viewport (`LOT-57` TACHE-01, barre d'état).
    [[nodiscard]] std::optional<core::GridPosition> hoveredCell() const noexcept {
        return _hoverCell;
    }

    /// @return Le facteur de zoom courant de la caméra d'édition (`LOT-57` TACHE-01, barre d'état).
    [[nodiscard]] float zoom() const noexcept {
        return _camera.zoom();
    }

    /// @return L'outil d'édition actif.
    [[nodiscard]] EditorTool activeTool() const noexcept {
        return _tool;
    }

    /// Met en surbrillance la liaison (déclencheur, cible) dans le viewport (sélection depuis le
    /// panneau « Liens ») ; n'affecte que le rendu, pas le brouillon.
    void setHighlightedLink(std::optional<std::pair<core::GridPosition, core::GridPosition>> link);

    /// Supprime la liaison dont la cible est @p targetPosition (bouton « Supprimer » du panneau
    /// « Liens ») ; sans effet si @p targetPosition n'a pas de liaison.
    void unlinkMechanism(core::GridPosition targetPosition);

    /// Retire l'override de texture de @p position (bouton « Retirer » de la section « Objets »,
    /// `LOT-45`) ; sans effet si @p position n'en a pas.
    void removeTextureOverride(core::GridPosition position);

    /// Met en surbrillance une case portant un override (sélection depuis la section « Objets »,
    /// `LOT-45`) ; n'affecte que le rendu, pas le brouillon. Réutilise le même voile que l'outil
    /// Rectangle/Sélection (case unique).
    void setHighlightedTextureOverride(std::optional<core::GridPosition> position);

    /**
     * @brief Assigne l'asset de fond du niveau courant (section « Fond », `LOT-44`).
     * @param background Nom du fichier `.png` dans `Assets/Backgrounds`, ou absent pour retirer le
     *                    fond posé.
     */
    void setLevelBackground(std::optional<std::string> background);

    /**
     * @brief Assigne le jeu de skins du niveau courant (section « Fond », `LOT-44`).
     *
     * Propriété **persistée du niveau** (`core::LevelDraft::skinSet`) : distincte du jeu de skins
     * **courant d'édition** (`setSkinSet`, session de l'éditeur, `LOT-42`) — les deux ne se
     * contaminent jamais l'un l'autre.
     * @param skinSet Nom du jeu (`skins.json`), ou absent pour le jeu par défaut.
     */
    void setLevelSkinSet(std::optional<std::string> skinSet);

    /**
     * @brief Change le cadrage de caméra du niveau courant (section « Cadrage », `EX-EDIT-028`,
     *        LOT-64).
     * @param cameraFraming Nouveau cadrage résolu (mode et, pour *par salle*, taille de salle).
     */
    void setLevelCameraFraming(core::CameraFramingConfig cameraFraming);

    /**
     * @brief Retire la zone de caméra au rang @p index (section « Cadrage », `EX-LVL-007`,
     *        `EX-EDIT-029`) ; sans effet si hors bornes.
     * @param index Rang dans `core::LevelDraft::cameraFraming().zones`.
     */
    void removeCameraZone(std::size_t index);

    /// @return Le mode de rendu courant, commun à l'édition, à l'essai et au jeu réel.
    [[nodiscard]] RenderMode renderMode() const noexcept {
        return _renderMode;
    }

    /**
     * @brief Jeu de visibilités par calque du mode d'inspection « définition des textures »
     *        (`LOT-51`, `EX-EDIT-044`) — édition uniquement, jamais lu par `hmi::GameSession`.
     * @return Le jeu de visibilités courant.
     */
    [[nodiscard]] const LayerVisibility& layerVisibility() const noexcept {
        return _layerVisibility;
    }

    /// Affiche ou masque un calque du mode d'inspection (section « Calques » du panneau
    /// « Textures », `LOT-51`). Comme `setRenderMode`, purement visuel : la scène ECS n'est jamais
    /// invalidée, seule la résolution d'apparence de l'image suivante en tient compte. Aucune
    /// persistance entre deux sessions (TACHE-01) : contrairement à `setRenderMode`, rien n'est
    /// écrit dans les préférences.
    void setLayerVisible(RenderLayer layer, bool visible) noexcept {
        _layerVisibility.setVisible(layer, visible);
    }

    /// Rétablit la visibilité de tous les calques (action « tout afficher », TACHE-03).
    void showAllLayers() noexcept {
        _layerVisibility.showAll();
    }

    /**
     * @brief Catalogue des jeux de skins, édité par le panneau « Textures » (`LOT-42`).
     *
     * Le viewport en reste **propriétaire** : le panneau agit dessus, et le rendu voit les mêmes
     * assignations à l'image suivante, sans reconstruire la scène.
     * @return Le catalogue.
     */
    [[nodiscard]] SkinCatalog& skinCatalog() noexcept {
        return _scene.skins();
    }

    /**
     * @brief Désigne le jeu de skins à utiliser pour le rendu (`LOT-42`).
     * @param setName Nom du jeu ; vide pour le jeu par défaut du catalogue.
     */
    void setSkinSet(const std::string& setName);

    /**
     * @brief Rechargement à chaud **global** des assets graphiques (`LOT-43` TACHE-03).
     *
     * Invalide tout le `TextureCache` et relit `skins.json` — un asset a pu être modifié,
     * renommé ou ajouté hors de l'application. Ne touche ni au brouillon en cours d'édition ni à
     * son historique d'annulation : c'est une opération de **présentation**, pas d'édition.
     * L'apparence est résolue à la composition de chaque image (`hmi::DraftRenderer`), donc
     * l'image suivante suffit à montrer le résultat — aucune reconstruction de scène nécessaire.
     */
    void reloadAssets();

    /**
     * @brief Rechargement à chaud **ciblé** d'un seul asset (`LOT-43` TACHE-03).
     *
     * Évite de relire toute la bibliothèque à chaque sauvegarde — utilisé par l'atelier pixel art
     * (`LOT-54`) après chaque enregistrement.
     * @param fileName Nom logique de l'asset à invalider (relatif à `Assets/`).
     */
    void invalidateAsset(const std::string& fileName);

    /**
     * @brief Choisit le mode de rendu et **persiste** le choix (`EX-REN-046`, `EX-IHM-011`).
     *
     * Purement visuel : ni la simulation, ni la scène ECS, ni le brouillon ne sont touchés — la
     * bascule ne coûte donc pas un pas fixe ni un rechargement de niveau.
     * @param mode Mode à appliquer.
     */
    void setRenderMode(RenderMode mode);

    /**
     * @name Plans picturaux (`EX-EDIT-047`, LOT-69 TACHE-08)
     *
     * Tous ces mutateurs passent par `core::LevelDraft`, seul propriétaire : ils sont donc
     * **annulables** au même titre qu'un coup de pinceau sur les tuiles. La **visibilité**, elle,
     * n'est pas une propriété du niveau : elle n'empile rien et ne se persiste pas.
     * @{
     */
    /// Sélectionne (ou désélectionne) un plan depuis le panneau.
    void selectPlane(std::optional<std::size_t> index);
    /// Ajoute @p plane à la fin de la liste et le sélectionne.
    void addPlane(core::Plane plane);
    /// Retire l'entrée du plan @p index. **Le fichier n'est pas supprimé** : le brouillon annule
    /// l'entrée JSON, pas la disparition d'une image — un fichier orphelin est moins grave qu'un
    /// travail perdu.
    void removePlane(std::size_t index);
    /// Déplace le plan @p index d'un rang ; @p forward le rapproche du dessus de la liste.
    void movePlane(std::size_t index, bool forward);
    /// Change la profondeur du plan @p index.
    void setPlaneDepth(std::size_t index, core::PlaneDepth depth);
    /// Change la densité déclarée du plan @p index (le rééchantillonnage de l'image est fait par
    /// l'appelant, qui possède le fichier).
    void setPlaneDensity(std::size_t index, int pixelsPerUnit);
    /// Change les facteurs de parallaxe du plan @p index.
    void setPlaneParallax(std::size_t index, float parallaxX, float parallaxY);
    /// Change l'opacité du plan @p index.
    void setPlaneOpacity(std::size_t index, float opacity);
    /// Change le drapeau de parallaxe du **niveau**.
    void setLevelParallaxEnabled(bool enabled);
    /// Masque ou réaffiche le plan @p index (aide d'édition).
    void setPlaneVisible(std::size_t index, bool visible);
    /// Isole le plan @p index, ou réaffiche tout si @p isolate est faux.
    void setPlaneIsolated(std::size_t index, bool isolate);
    /// @}

signals:
    /// Message d'état à afficher (enregistrement, essai, erreur de validation…).
    void statusMessage(const QString& message);
    /// Demande de retour au menu principal (fin de la séquence de niveaux -- plus depuis `LOT-59`
    /// TACHE-02 sur `Échap` en mode jeu, qui ouvre désormais la pause via `pauseRequested`).
    void exitToMenuRequested();
    /// `Échap` (ou le bouton `B` manette) en mode jeu réel, hors pause : demande l'ouverture de
    /// l'écran de pause (`LOT-59` TACHE-02, `EX-GP-041`) -- jamais en essai depuis l'éditeur
    /// (`stopPlaytest` garde son propre chemin, inchangé).
    void pauseRequested();
    /// Le brouillon vient d'être modifié (peinture, undo/redo, chargement, lien…) — le panneau
    /// « Liens » se resynchronise dessus (`refresh`).
    void draftChanged();
    /// Le mode de rendu vient de basculer (`F8`) — la palette met ses vignettes à jour pour rester
    /// fidèle au canevas (`EX-EDIT-027`).
    void renderModeChanged(RenderMode mode);
    /// L'outil actif vient de changer par un moyen autre que le panneau Outils (raccourci clavier,
    /// `LOT-45`) — le panneau se resynchronise sans reboucler.
    void toolChanged(hmi::EditorTool tool);
    /// La sélection de plan vient de changer par un moyen autre que le panneau (retrait,
    /// réordonnancement) — le panneau se resynchronise sans reboucler.
    void planeSelectionChanged(std::optional<std::size_t> index);
    /// La case survolée vient de changer (déplacement de souris, ou sortie du viewport) —
    /// consommé par la barre d'état (`LOT-57` TACHE-01), qui évite ainsi un travail continu inutile
    /// en ne recalculant que sur changement réel.
    void hoveredCellChanged(std::optional<core::GridPosition> cell);
    /// Le zoom d'édition vient de changer (molette, pan, recadrage) — consommé par la barre d'état
    /// (`LOT-57` TACHE-01).
    void zoomChanged(float zoom);
    /// Les ressources graphiques et le catalogue de skins viennent d'être (re)créés
    /// (`QRhiWidget::initialize`) — tout ce qui a lu `skinCatalog()` **avant** la première
    /// initialisation (le câblage de `MainWindow`, à la construction) l'a lu vide et doit se
    /// reconstruire une fois ce signal reçu, sous peine de vignettes/arbre de skins sans texture au
    /// lancement de l'éditeur. Émis à nouveau si `QRhiWidget` recrée son interface de rendu (le
    /// widget a changé de fenêtre de haut niveau) : les textures sont alors perdues avec elle.
    void resourcesReady();

protected:
    /// Crée (ou recrée) les ressources graphiques quand `QRhiWidget` fournit son interface de
    /// rendu, et à chaque fois qu'il en change (`EX-NFR-040` : perte de ressources prévue, pas
    /// supposée impossible).
    void initialize(QRhiCommandBuffer* commandBuffer) override;
    /// Dessine une image : avance la simulation, compose la scène, puis la soumet.
    void render(QRhiCommandBuffer* commandBuffer) override;
    /// Libère les ressources graphiques quand `QRhiWidget` défait son interface de rendu.
    void releaseResources() override;
    bool event(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    /// Crée l'atlas, la police, le cache de textures et le pipeline de dessin sur l'interface de
    /// rendu courante, puis ouvre le brouillon de départ. Appelée par `initialize`.
    void createResources();
    /// Avance la simulation d'une image (entrées, pas fixes, sons) — sans rien dessiner.
    void tick(float elapsedSeconds);
    /// Compose et soumet une image sur @p commandBuffer.
    /// @p deltaSeconds : temps réel écoulé depuis l'image précédente (LOT-46 TACHE-05, avance
    /// l'aperçu des tuiles animées de l'éditeur).
    void renderFrame(QRhiCommandBuffer* commandBuffer, float deltaSeconds);
    /// Compose et soumet le compteur de diagnostic (`LOT-62` TACHE-02), coin haut-droit de l'écran
    /// -- sans effet si désactivé, hors session, ou avant que la localisation ne soit chargée
    /// (`EX-NFR-040`, coût nul quand éteint : rien n'est calculé au-delà du test d'entrée).
    void renderDiagnosticsOverlay(int viewportWidth, int viewportHeight);
    void stopPlaytest();  ///< Termine l'essai et restitue l'éditeur (brouillon intact).
    void loadGameMap();   ///< Charge `_gameMap` et arme la session de jeu.
    void updateMousePosition(const QMouseEvent* event);

    /// Recale la caméra d'édition : cadrage automatique sur le niveau entier, sauf pan/zoom manuel
    /// actif (molette/glisser bouton droit), auquel cas la caméra manuelle est réappliquée.
    void updateEditCamera();
    /// Position écran (pixels physiques) d'un événement souris, convertie par le ratio d'affichage
    /// — même conversion que `updateMousePosition`, réutilisée pour le pan manuel.
    [[nodiscard]] core::Vector2 screenPosition(const QMouseEvent* event) const;
    /// @return Le zoom manuel minimal autorisé : celui de l'ajustement automatique du brouillon
    /// courant (rien à voir de plus loin).
    [[nodiscard]] float minManualZoom() const;
    /// @return Le zoom manuel maximal autorisé : la valeur laissant encore 4 cases visibles sur le
    /// plus petit axe de l'écran (précision jugée suffisante pour poser un bloc).
    [[nodiscard]] float maxManualZoom() const;
    /// Case de grille sous la position écran donnée (pixels physiques), si dans les bornes.
    [[nodiscard]] std::optional<core::GridPosition> cellAt(const QMouseEvent* event);
    /// Case de grille sous @p event, **bornée** à la grille (jamais hors bornes) — pour les
    /// glissers rectangle/sélection dont le curseur peut dépasser légèrement les bords.
    [[nodiscard]] core::GridPosition clampedCell(const QMouseEvent* event);
    /// Peint le type actif à la case sous @p event, si valide (invalide la scène rendue).
    void paintAt(const QMouseEvent* event);
    /// Remplit le rectangle (bornes @p a, @p b incluses) du type actif (outil Rectangle).
    void applyRectangle(core::GridPosition a, core::GridPosition b);
    /// Ajoute une zone de caméra couvrant le rectangle (bornes @p a, @p b incluses, outil
    /// `CameraZone`, `EX-LVL-007`, `EX-EDIT-029`) au cadrage du niveau.
    void addCameraZoneFromDrag(core::GridPosition a, core::GridPosition b);
    /// Copie la zone sélectionnée dans le presse-papiers local (`Ctrl+C`).
    void copySelection();
    /// Colle le presse-papiers à la case survolée (`Ctrl+V`).
    void pasteClipboard();
    /// Zone à mettre en surbrillance (glisser en cours, sinon sélection mémorisée), le cas échéant.
    [[nodiscard]] std::optional<std::pair<core::GridPosition, core::GridPosition>> highlight()
        const;
    /// Résout le clic de l'outil Lien (`hmi::resolveLinkClick`) et l'applique (attente, liaison).
    void handleLinkClick(const QMouseEvent* event);
    /// Résout le clic de l'outil « Texture par instance » (`hmi::resolveTextureAssignClick`) et
    /// l'applique (assignation/retrait) ; @p rightClick distingue le clic droit (retrait
    /// explicite).
    void handleTextureAssignClick(const QMouseEvent* event, bool rightClick);
    /// Convertit la position écran d'un événement souris en position **monde**, non calée sur la
    /// grille (`EX-DEC-001`) — contrairement à `cellAt`/`clampedCell`, utilisées par les autres
    /// outils.
    [[nodiscard]] core::Vector2 worldPositionAt(const QMouseEvent* event);
    /// @return true si (@p switchPosition, @p targetPosition) est déjà une liaison du brouillon.
    [[nodiscard]] bool linkExists(core::GridPosition switchPosition,
                                  core::GridPosition targetPosition) const;
    /// Invalide le rendu du brouillon et notifie les panneaux dépendants (`draftChanged`) — à
    /// appeler après toute mutation de `_draft` (peinture, lien, undo/redo, chargement…).
    void markDraftMutated();

    [[nodiscard]] int pixelWidth() const;
    [[nodiscard]] int pixelHeight() const;

    /// Texte localisé d'une clé de statut (repli sur la clé si aucun catalogue).
    [[nodiscard]] QString statusText(const char* key) const;

    using Clock = std::chrono::steady_clock;

    /// Lot de sprites, atlas, police, textures et skins — communs au viewport de l'éditeur et à
    /// l'élément Qt Quick du jeu (`LOT-86`). Regroupés parce que leur ORDRE de libération est une
    /// contrainte, pas une préférence : voir `hmi::SceneResources`.
    hmi::SceneResources _scene;
    std::unique_ptr<hmi::DraftRenderer> _draftRenderer;
    /// Jeu de skins courant ; vide pour le jeu par défaut du catalogue.
    std::string _skinSet;
    hmi::GameKeyBindings _gameBindings;
    hmi::GamepadBindings _gamepadBindings;
    hmi::EditorKeyBindings _editorBindings;  ///< Raccourcis d'éditeur remappables (`LOT-45`).
    hmi::InputState _input;
    hmi::GamepadPoller _gamepad;
    core::FixedTimestep _timestep;
    Clock::time_point _previousFrame;
    bool _loopStarted = false;

    /// Compteur de diagnostic (`F9`, `LOT-62` TACHE-02) : désactivé par défaut (`EX-NFR-040`,
    /// point de départ sans effet visuel ni coût).
    /// Bilan du tableau en cours (LOT-68) : alimenté au PAS, jamais à l'image de rendu.
    bool _diagnosticsEnabled = false;
    /// Moyenne glissante de la cadence de rendu, alimentée uniquement quand `_diagnosticsEnabled`
    /// est vrai (rien n'est calculé quand l'affichage est éteint).
    hmi::FrameRateAverage _frameRateAverage;
    /// Nombre de pas de simulation consommés au dernier `tick()` -- distinct de la cadence de
    /// rendu depuis le `LOT-33` (`hmi::DiagnosticsMeasurements::simulationSteps`).
    int _lastSimulationSteps = 0;
    /// Scène dédiée du compteur de diagnostic, jamais partagée avec `_session` (dont le HUD de jeu
    /// vit dans sa propre scène) ni avec le brouillon d'édition -- même isolement que le texte de
    /// `hmi::GameSession::renderHud` (`LOT-52`).
    hmi::ComposedScene _diagnosticsScene;

    core::LevelDraft _draft;  ///< Brouillon en cours d'édition (source de vérité).
    hmi::Camera2D _camera;    ///< Caméra d'édition (cadre le niveau entier par défaut).
    /// `true` dès que la molette ou un glisser bouton droit a été utilisé : la caméra suit
    /// `_manualZoom`/`_manualCenter` plutôt que le cadrage automatique, jusqu'à la réinitialisation
    /// (`0`) ou l'ouverture d'un autre niveau. Restaure le pan/zoom manuel de l'éditeur historique
    /// (`EditorScreen`, LOT-15), perdu lors de la réécriture Direct3D11/Qt.
    bool _manualCamera = false;
    float _manualZoom = 1.0f;       ///< Zoom manuel courant, actif si `_manualCamera`.
    core::Vector2 _manualCenter{};  ///< Centre manuel courant, actif si `_manualCamera`.
    bool _rightDragging = false;    ///< Un glisser bouton droit est en cours (pan potentiel).
    /// `true` dès qu'un glisser bouton droit a dépassé le seuil de mouvement : distingue un pan
    /// d'un simple clic droit (retrait d'override, outil « Texture par instance »).
    bool _cameraPanned = false;
    core::Vector2 _rightDragLastScreen{};  ///< Dernière position écran du glisser droit en cours.
    core::TileType _activeTile = core::TileType::Solid;  ///< Type peint au clic (palette).
    /// Asset assigné au clic par l'outil « Texture par instance » (`LOT-45`), vide si aucun n'est
    /// sélectionné dans la bibliothèque « Objets ».
    std::optional<std::string> _activeTextureAsset;
    /// Plan sélectionné dans le panneau « Plans » (`LOT-69`), si un l'est.
    std::optional<std::size_t> _selectedPlane;
    /// Visibilité des plans dans l'éditeur : aide d'édition, jamais persistée ni annulable.
    hmi::PlaneVisibility _planeVisibility;
    hmi::EditorTool _tool = hmi::EditorTool::Paint;  ///< Outil d'édition actif (barre d'outils).
    bool _painting = false;             ///< Un glisser de peinture (Pinceau) est en cours.
    bool _dragging = false;             ///< Un glisser Rectangle/Sélection est en cours.
    core::GridPosition _dragStart{};    ///< Case de départ du glisser Rectangle/Sélection.
    core::GridPosition _dragCurrent{};  ///< Case courante du glisser (pour l'aperçu).
    /// Case actuellement survolée (cible du collage) ; absente hors de la grille ou après que le
    /// curseur a quitté le viewport (`LOT-57` TACHE-01, `leaveEvent` via `event()`).
    std::optional<core::GridPosition> _hoverCell;
    float _lastEmittedZoom =
        0.0f;  ///< Dernier zoom notifié (`zoomChanged`), évite l'émission en boucle.
    /// Sélection mémorisée (bornes min/max incluses), pour copier (`Ctrl+C`).
    const Localization* _loc = nullptr;  ///< Catalogue pour localiser les messages d'état.
    /// Moteur audio (`LOT-60` TACHE-03), non possédé ; nul = sons de jeu désactivés (`EX-NFR-040`).
    AudioEngine* _audioEngine = nullptr;

    std::optional<std::pair<core::GridPosition, core::GridPosition>> _selection;
    /// Case du déclencheur/de la cible en attente d'appariement (outil Lien, premier clic).
    std::optional<core::GridPosition> _pendingLink;
    /// Liaison sélectionnée dans le panneau « Liens » (déclencheur, cible) : mise en surbrillance.
    std::optional<std::pair<core::GridPosition, core::GridPosition>> _selectedLink;
    /// Case sélectionnée dans la section « Objets » du panneau « Textures » (`LOT-45`) : mise en
    /// surbrillance, même voile que l'outil Rectangle/Sélection.
    std::optional<core::GridPosition> _highlightedOverride;
    /// Presse-papiers local (types de tuiles, `[ligne][colonne]`), pour `Ctrl+C`/`Ctrl+V`.
    std::vector<std::vector<core::TileType>> _clipboard;
    bool _dirty = false;    ///< Modifications non enregistrées (garde-fou d'ouverture).
    bool _showGrid = true;  ///< Grille de repère (cases + salles) affichée (bascule F10).
    /// Mode de rendu courant (bascule `F8`, `EX-REN-046`), restauré des préférences au démarrage
    /// et réécrit à chaque bascule. Porté par le viewport parce qu'il est le widget **unique**
    /// derrière l'édition, l'essai et le jeu réel (`EX-IHM-002`) : un seul état, jamais dupliqué.
    RenderMode _renderMode = DEFAULT_RENDER_MODE;
    /// Jeu de visibilités par calque du mode d'inspection « définition des textures » (`LOT-51`) —
    /// tout visible par défaut, jamais persisté (TACHE-01), édition uniquement.
    LayerVisibility _layerVisibility;
    bool _vsync = true;      ///< Synchronisation verticale demandée (cf. `setVSyncEnabled`).
    bool _gameMode = false;  ///< La session courante est une **partie** (menu Jouer) et
                             ///< non un essai depuis l'éditeur (enchaînement/retour menu).
    /// Écran de pause affiché (`LOT-59` TACHE-02) : `tick()` n'avance plus l'accumulateur de pas
    /// fixe tant que c'est vrai -- le rendu, lui, continue.
    bool _paused = false;
    std::filesystem::path _gameMap;  ///< Carte jouée par le mode jeu (`LOT-67` : une, pas une
                                     ///< séquence).

    /// Session de jeu de l'essai immédiat ; nulle en mode édition (essai ajouté au LOT-35
    /// TACHE-04).
    std::optional<hmi::GameSession> _session;
};

}  // namespace hmi
