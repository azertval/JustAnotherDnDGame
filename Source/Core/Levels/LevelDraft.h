// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "Core/Levels/CameraFraming.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/MapEntity.h"
#include "Core/Levels/TileLayer.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"

/**
 * @file Core/Levels/LevelDraft.h
 * @brief Représentation mutable d'un niveau en cours d'édition.
 */

namespace core {

/**
 * @brief Niveau **mutable** en cours d'édition, distinct de `Level` (immuable, LOT-07).
 *
 * `LevelDraft` porte toute la mutabilité nécessaire à l'éditeur (`EX-EDIT-002` à `EX-EDIT-005`) :
 * peindre une tuile, déplacer l'entrée/la sortie, lier/délier un mécanisme, redimensionner la
 * grille. Il ne duplique **aucune règle de validation** : `toLevel()` reconstruit le niveau en
 * repassant par `LevelLoader::loadFromString` (même chemin que le chargement d'un fichier),
 * garantissant que le niveau produit satisfait exactement les mêmes règles (`EX-LVL-004`,
 * `EX-EDIT-010`).
 *
 * Invariant maintenu par tous les mutateurs : la grille de tuiles reste la **source de vérité**
 * des positions d'entrée/sortie/interrupteurs/portes (comme pour `Level`) ; `entry()`/`exit()`
 * ne sont que des accès en cache, toujours synchronisés avec le contenu de `tileMap()`.
 *
 * Logique **pure**, sans dépendance rendu ni fenêtre — testable sans GPU (`EX-NFR-010`).
 */
class LevelDraft {
public:
    /**
     * @brief Crée un brouillon vierge (grille entièrement `Empty`), sans entrée ni sortie.
     * @param name   Nom du niveau.
     * @param width  Largeur de la grille, en cases (> 0).
     * @param height Hauteur de la grille, en cases (> 0).
     */
    [[nodiscard]] static LevelDraft empty(std::string name, int width, int height);

    /**
     * @brief Crée un brouillon à partir d'un niveau déjà chargé (édition d'un fichier existant).
     * @param level Niveau source.
     */
    [[nodiscard]] static LevelDraft fromLevel(const Level& level);

    /**
     * @brief Peint le type de tuile @p type en (column, row).
     *
     * Cas particuliers : peindre `Entry`/`Exit` délègue à `setEntry`/`setExit` (unicité). Peindre
     * un autre type sur une case qui portait l'entrée, la sortie, un interrupteur ou une porte
     * **efface** l'état associé (entrée/sortie invalidée, liaisons de mécanismes retirées) pour
     * ne jamais laisser d'incohérence entre la grille et ces caches.
     * @param column Colonne visée (doit être dans les bornes).
     * @param row    Ligne visée (doit être dans les bornes).
     * @param type   Type de tuile à poser.
     */
    void paintTile(int column, int row, TileType type);

    /**
     * @brief Applique un bloc rectangulaire de types de tuiles à partir de (@p originColumn,
     *        @p originRow).
     *
     * Repasse par la même sémantique cellule-par-cellule que `paintTile` (déplacement
     * entrée/sortie, nettoyage des liaisons) pour chaque case du bloc, mais ne pousse **qu'un
     * seul** snapshot undo pour toute l'opération — sert au remplissage rectangulaire et au
     * collage (`EX-EDIT-014`), sans dupliquer de règle de niveau (`EX-EDIT-010`). Les cases du
     * bloc hors des bornes de la grille sont silencieusement ignorées (découpe aux bords, même
     * principe que `resize`).
     * @param originColumn Colonne de la case (0,0) du bloc.
     * @param originRow    Ligne de la case (0,0) du bloc.
     * @param block        Bloc de types, indexé `[ligne][colonne]` ; sans effet si vide.
     */
    void paintRegion(int originColumn, int originRow,
                     const std::vector<std::vector<TileType>>& block);

    /**
     * @brief Place l'entrée en (column, row) ; déplace l'occurrence existante s'il y en avait
     *        une (unicité, `EX-EDIT-004`).
     */
    void setEntry(int column, int row);

    /**
     * @brief Place la sortie en (column, row) ; déplace l'occurrence existante s'il y en avait
     *        une (unicité, `EX-EDIT-004`).
     */
    void setExit(int column, int row);

    /**
     * @brief Lie un interrupteur/plaque de pression à une **porte** ou à un **danger commuté**
     *        (`EX-EDIT-003`, `EX-GP-052`), selon le type de tuile posé en @p targetPosition.
     *
     * Remplace toute liaison existante pour @p targetPosition (une cible n'a qu'un seul
     * déclencheur associé) ; plusieurs cibles peuvent en revanche partager le même déclencheur.
     * La liaison résultante est rangée dans `mechanisms()`.
     * @pre La case @p switchPosition porte un `Switch`/`PressurePlate`/`Key`, la case
     *      @p targetPosition porte une `Door`/`LockedDoor`.
     */
    void linkMechanism(GridPosition switchPosition, GridPosition targetPosition);

    /// Retire la liaison de porte à @p targetPosition, si elle en a une. Sans
    /// effet sinon.
    void unlinkMechanism(GridPosition targetPosition);
    /**
     * @brief Assigne (ou remplace) la texture affichée pour **une case précise** (`EX-EDIT-043`),
     *        prioritaire sur le skin de son type (LOT-42).
     *
     * Repeindre un **autre** type de tuile sur @p position retire l'override (funnel
     * `removeLinkedDataAt`) ; repeindre le **même** type le conserve. Ne voyage pas avec
     * `paintRegion` (collage) : un override reste attaché à sa case d'origine.
     * @pre La case @p position porte un type de tuile non `Empty`.
     */
    void setTextureOverride(GridPosition position, std::string assetName);

    /// Retire l'override de texture de @p position, s'il y en a un. Sans effet sinon.
    void removeTextureOverride(GridPosition position);

    /**
     * @name Plans picturaux (`EX-DEC-040`, LOT-69)
     *
     * Tous ces mutateurs empilent **un** pas d'annulation (`pushUndo`) et sont donc annulables
     * *et* refaisables. Un rang hors bornes est **sans effet** et n'empile rien — même convention
     * que les mutateurs de décor et de parcours.
     *
     * Contrairement aux décors, les plans ne sont pas regroupés par couche pour le
     * réordonnancement : leur rang dans la liste **est** l'ordre de superposition, et la
     * profondeur (`core::PlaneDepth`) est une propriété indépendante. Monter un plan le rapproche
     * donc du premier plan de la liste, quelle que soit sa profondeur.
     * @{
     */

    /// Ajoute @p plane en fin de liste (le plus en avant).
    void addPlane(Plane plane);

    /// Retire le plan au rang @p index. Ne touche **jamais** au fichier PNG : le brouillon annule
    /// une entrée, il ne restaurerait pas un fichier supprimé (voir `LOT-69` TACHE-08).
    void removePlane(std::size_t index);

    /// Change la densité du plan au rang @p index (`EX-DEC-041`).
    /// @return `false` si @p index est hors bornes ou si @p pixelsPerUnit n'est pas une densité
    ///         valide (`core::isValidPlaneDensity`) — auquel cas rien n'est empilé.
    bool setPlaneDensity(std::size_t index, int pixelsPerUnit);

    /// Change les facteurs de parallaxe du plan au rang @p index (`EX-DEC-043`).
    /// @return `false` si @p index est hors bornes ou si un facteur n'est pas fini.
    bool setPlaneParallax(std::size_t index, float parallaxX, float parallaxY);

    /// Change l'opacité du plan au rang @p index.
    /// @return `false` si @p index est hors bornes ou si @p opacity sort de `[0, 1]`.
    bool setPlaneOpacity(std::size_t index, float opacity);

    /// Change la profondeur du plan au rang @p index (`EX-DEC-042`).
    /// @return `false` si @p index est hors bornes.
    bool setPlaneDepth(std::size_t index, PlaneDepth depth);

    /// Avance le plan au rang @p index d'un cran ; sans effet s'il est déjà le dernier.
    /// @return Le nouveau rang, ou `std::nullopt` si @p index est hors bornes.
    std::optional<std::size_t> movePlaneForward(std::size_t index);

    /// Recule le plan au rang @p index d'un cran ; sans effet s'il est déjà le premier.
    std::optional<std::size_t> movePlaneBackward(std::size_t index);

    /// Amène le plan au rang @p index au dernier rang (le plus en avant).
    std::optional<std::size_t> movePlaneToFront(std::size_t index);

    /// Envoie le plan au rang @p index au premier rang (le plus en arrière).
    std::optional<std::size_t> movePlaneToBack(std::size_t index);

    /// Active ou désactive la parallaxe des plans pour ce niveau (`EX-DEC-043`).
    void setParallaxEnabled(bool enabled);

    /** @} */

    /**
     * @brief Redimensionne la grille (`EX-EDIT-005`).
     *
     * Agrandir complète les nouvelles cases en `Empty` ; réduire **tronque** silencieusement le
     * contenu hors des nouvelles bornes (entrée, sortie ou liaisons de mécanismes perdues sont
     * invalidées en conséquence). L'avertissement de perte revient à l'appelant `HMI`, avant
     * d'invoquer `resize`.
     * @param width  Nouvelle largeur, en cases (> 0).
     * @param height Nouvelle hauteur, en cases (> 0).
     */
    void resize(int width, int height);

    /**
     * @brief Indique si redimensionner à (@p width, @p height) supprimerait du contenu déjà posé.
     *
     * Requête **pure** (n'altère rien) : vraie si les nouvelles bornes excluraient l'entrée, la
     * sortie, ou une des deux extrémités d'une liaison de mécanisme actuellement posées. Permet à
     * `HMI` d'avertir avant d'appeler `resize` (`EX-EDIT-012`), sans dupliquer la logique de
     * troncature déjà portée par `resize`.
     * @param width  Largeur envisagée, en cases (> 0).
     * @param height Hauteur envisagée, en cases (> 0).
     * @return `true` si l'entrée, la sortie ou une liaison serait perdue.
     */
    [[nodiscard]] bool wouldResizeDropContent(int width, int height) const noexcept;

    /**
     * @brief Annule la dernière mutation (`EX-EDIT-005`).
     * @return `true` si une mutation a été annulée, `false` si l'historique était vide.
     */
    bool undo();

    /**
     * @brief Refait la dernière mutation annulée.
     *
     * Toute nouvelle mutation après un `undo()` invalide la branche de refaire (historique
     * linéaire classique) : `redo()` redevient sans effet tant qu'aucun nouvel `undo()` n'a eu
     * lieu depuis.
     * @return `true` si une mutation a été refaite, `false` si rien n'était à refaire.
     */
    bool redo();

    /// @return `true` si `undo()` aurait un effet.
    [[nodiscard]] bool canUndo() const noexcept {
        return !_undoHistory.empty();
    }

    /// @return `true` si `redo()` aurait un effet.
    [[nodiscard]] bool canRedo() const noexcept {
        return !_redoHistory.empty();
    }

    /// Renomme le niveau.
    void setName(std::string name) {
        _name = std::move(name);
    }

    /**
     * @brief Assigne l'asset de fond du niveau (`EX-REN-044`), annulable.
     * @param background Nom de l'asset (ex. `"forest.png"`), ou vide pour retirer le fond posé.
     *                    Une chaîne, jamais un handle : `Core` ignore tout du dossier d'assets.
     */
    void setBackground(std::optional<std::string> background);

    /**
     * @brief Assigne le jeu de skins du niveau (`EX-EDIT-024`), annulable.
     * @param skinSet Nom du jeu (`skins.json`), ou vide pour le jeu par défaut.
     */
    void setSkinSet(std::optional<std::string> skinSet);

    /**
     * @brief Change le cadrage de caméra du niveau (`EX-LVL-006`, `EX-EDIT-028`), annulable.
     * @param cameraFraming Nouveau cadrage résolu (mode et, pour *par salle*, taille de salle).
     */
    void setCameraFraming(CameraFramingConfig cameraFraming);

    /**
     * @brief Ajoute une zone de caméra dessinée à la main en fin de liste (mode `PerRoom`,
     *        `EX-LVL-007`, `EX-EDIT-029`), annulable.
     *
     * L'ordre d'ajout fixe la priorité en cas de chevauchement (`core::activeCameraZoneIndex`
     * côté `HMI`, même convention que `addDecor`) : une zone ajoutée après une autre ne prend le
     * dessus que là où l'autre ne la couvre pas.
     * @param zone Zone à ajouter, en cases.
     */
    void addCameraZone(CameraZone zone);

    /**
     * @brief Retire la zone de caméra au rang @p index (`EX-LVL-007`, `EX-EDIT-029`), annulable.
     * @param index Rang dans `cameraFraming().zones` ; sans effet si hors bornes.
     */
    void removeCameraZone(std::size_t index);

    /// @return Le nom courant du niveau.
    [[nodiscard]] const std::string& name() const noexcept {
        return _name;
    }

    /// @return La grille de tuiles courante.
    [[nodiscard]] const TileMap& tileMap() const noexcept {
        return _tileMap;
    }

    /// @return La position d'entrée, si elle est posée.
    [[nodiscard]] std::optional<GridPosition> entry() const noexcept {
        return _entry;
    }

    /// @return La position de sortie, si elle est posée.
    [[nodiscard]] std::optional<GridPosition> exit() const noexcept {
        return _exit;
    }

    /// @return Les liaisons de mécanismes courantes (déclencheur ↔ porte).
    [[nodiscard]] const std::vector<Mechanism>& mechanisms() const noexcept {
        return _mechanisms;
    }
    /// @return Les textures assignées par instance du niveau (`EX-EDIT-043`).
    [[nodiscard]] const std::vector<TileTextureOverride>& textureOverrides() const noexcept {
        return _textureOverrides;
    }

    /// @return Les plans picturaux courants (`EX-DEC-040`), dans leur ordre de superposition.
    [[nodiscard]] const std::vector<Plane>& planes() const noexcept {
        return _planes;
    }

    /// @return Les couches de tuiles du niveau (`LOT-04`), dans leur ordre de superposition.
    ///
    /// **Portees telles quelles, pas encore editables.** Le brouillon les recoit du fichier et les
    /// rend a l'enregistrement ; c'est le `LOT-11` qui donnera a l'editeur de quoi les creer et
    /// les peindre. Sans ce transport, ouvrir puis enregistrer une carte multi-couches les
    /// effacerait en silence -- exactement le genre de perte qu'un editeur ne doit jamais infliger.
    [[nodiscard]] const std::vector<TileLayer>& layers() const noexcept {
        return _layers;
    }

    /// @return Les entites placees sur la carte (`LOT-04`). Portees telles quelles, comme
    /// `layers()`, en attendant l'outillage d'edition du `LOT-11`.
    [[nodiscard]] const std::vector<MapEntity>& entities() const noexcept {
        return _entities;
    }

    /// @return `true` si la parallaxe des plans est active pour ce niveau (`EX-DEC-043`).
    [[nodiscard]] bool parallaxEnabled() const noexcept {
        return _parallaxEnabled;
    }

    /// @return L'asset de fond courant (`EX-REN-044`), absent si aucun n'est posé.
    [[nodiscard]] const std::optional<std::string>& background() const noexcept {
        return _background;
    }

    /// @return Le jeu de skins courant du niveau (`EX-EDIT-024`), absent pour le jeu par défaut.
    [[nodiscard]] const std::optional<std::string>& skinSet() const noexcept {
        return _skinSet;
    }

    /// @return Le cadrage de caméra courant du niveau (`EX-LVL-006`).
    [[nodiscard]] const CameraFramingConfig& cameraFraming() const noexcept {
        return _cameraFraming;
    }

    /**
     * @brief Convertit le brouillon en `Level` **validé** (`EX-EDIT-007`), en repassant par la
     *        même validation que `LevelLoader` (sérialise puis recharge : aucune règle
     *        dupliquée, `EX-EDIT-010`).
     * @return Un résultat récupérable : niveau valide, ou message d'erreur exploitable si le
     *         brouillon est incomplet (pas d'entrée/de sortie, liaison non résolue, …).
     */
    [[nodiscard]] LevelLoadResult toLevel() const;

private:
    LevelDraft(std::string name, TileMap tileMap);

    /// Logique de `paintTile`, sans `pushUndo()` : réutilisée cellule par cellule par
    /// `paintRegion` pour n'empiler qu'un seul snapshot par opération de bloc.
    void paintTileInternal(int column, int row, TileType type);

    /// Logique de `setEntry`, sans `pushUndo()`.
    void setEntryInternal(int column, int row);

    /// Logique de `setExit`, sans `pushUndo()`.
    void setExitInternal(int column, int row);

    /// Retire toute liaison/configuration référençant @p position (déclencheur, porte, danger
    /// commuté, configuration de danger mobile/temporisé, ou override de texture) — appelé avant
    /// de reposer un autre type sur une case qui en portait une, pour ne jamais laisser d'entrée
    /// orpheline. @p keepTextureOverride préserve l'override de texture (repeindre le **même**
    /// type ne doit pas effacer un habillage, `EX-EDIT-043`) ; les autres données annexes sont
    /// toujours retirées, comme avant ce paramètre.
    void removeLinkedDataAt(GridPosition position, bool keepTextureOverride = false);

    /// Configuration de la plateforme en @p position, **créée aux valeurs par défaut** si absente.
    /// Empile un unique `pushUndo()` : point de passage commun des mutateurs granulaires de route,
    /// pour qu'un geste ne coûte jamais plus d'un pas d'annulation.

    /// État complet du brouillon, hors historique (utilisé pour les snapshots undo/redo).
    struct State {
        std::string name;
        TileMap tileMap;
        std::optional<GridPosition> entry;
        std::optional<GridPosition> exit;
        std::vector<Mechanism> mechanisms;
        std::vector<TileLayer> layers;
        std::vector<MapEntity> entities;
        std::optional<std::string> background;
        std::optional<std::string> skinSet;
        std::vector<TileTextureOverride> textureOverrides;
        CameraFramingConfig cameraFraming;
        std::vector<Plane> planes;
        bool parallaxEnabled;
    };

    /// Capture l'état courant (pour empiler dans l'historique undo/redo).
    [[nodiscard]] State snapshot() const;

    /// Restitue un état capturé précédemment.
    void restore(State state);

    /// Empile l'état courant sur la pile d'annulation ; à appeler avant toute mutation
    /// undoable. Une nouvelle mutation invalide toujours la branche de refaire.
    void pushUndo();

    std::string _name;
    TileMap _tileMap;
    std::optional<GridPosition> _entry;
    std::optional<GridPosition> _exit;
    std::vector<Mechanism> _mechanisms;
    std::vector<TileLayer> _layers;
    std::vector<MapEntity> _entities;
    std::optional<std::string> _background;
    std::optional<std::string> _skinSet;
    std::vector<TileTextureOverride> _textureOverrides;
    CameraFramingConfig _cameraFraming;
    std::vector<Plane> _planes;
    bool _parallaxEnabled = true;
    std::vector<State> _undoHistory;
    std::vector<State> _redoHistory;
};

}  // namespace core
