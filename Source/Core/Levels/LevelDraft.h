// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

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
 * @brief Carte **mutable** en cours d'édition, distincte de `Level` (immuable).
 *
 * `LevelDraft` porte toute la mutabilité nécessaire à l'éditeur (`EX-EDIT-002` à `EX-EDIT-005`) :
 * peindre une tuile, déplacer l'entrée, éditer les couches et les entités, redimensionner la
 * grille. Il ne duplique **aucune règle de validation** : `toLevel()` reconstruit le niveau en
 * repassant par `LevelLoader::loadFromString` (même chemin que le chargement d'un fichier),
 * garantissant que le niveau produit satisfait exactement les mêmes règles (`EX-LVL-004`,
 * `EX-EDIT-010`).
 *
 * Invariant maintenu par tous les mutateurs : la grille de tuiles reste la **source de vérité**
 * de la position d'entrée (comme pour `Level`) ; `entry()` n'est qu'un accès en cache, toujours
 * synchronisé avec le contenu de `tileMap()`.
 *
 * Logique **pure**, sans dépendance rendu ni fenêtre — testable sans GPU (`EX-NFR-010`).
 */
class LevelDraft {
public:
    /**
     * @brief Crée un brouillon vierge (grille entièrement `Empty`), sans entrée.
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
     * Cas particulier : peindre `Entry` délègue à `setEntry` (unicité). Peindre un autre type sur
     * la case de l'entrée l'invalide, et peindre un type différent retire la pièce assignée à la
     * case, pour ne jamais laisser d'incohérence entre la grille et ces caches.
     * @param column Colonne visée (doit être dans les bornes).
     * @param row    Ligne visée (doit être dans les bornes).
     * @param type   Type de tuile à poser.
     */
    void paintTile(int column, int row, TileType type);

    /**
     * @brief Applique un bloc rectangulaire de types de tuiles à partir de (@p originColumn,
     *        @p originRow).
     *
     * Repasse par la même sémantique cellule-par-cellule que `paintTile` (déplacement de
     * l'entrée, retrait des pièces assignées) pour chaque case du bloc, mais ne pousse **qu'un
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
     * @name Couches de tuiles (`LOT-04`, `LOT-11`)
     *
     * Un rang désigne une entrée de `layers()`. Seules les couches **visuelles** — sol et décor
     * (`core::isVisualLayerKind`) — se créent, se retirent, se renomment, se déplacent et se
     * peignent ici : la grille de collision est `tileMap()`, que peignent `paintTile` et
     * `paintRegion`, et l'entrée `Collision` ou `Legacy` de `layers()` n'en est que le reflet.
     *
     * Un rang hors bornes, une couche non visuelle ou un type de tuile refusé
     * (`core::isVisualLayerTileType`) ne fait **rien** et n'empile **rien**.
     * @{
     */

    /**
     * @brief Ajoute une couche visuelle en fin de liste (la plus en avant).
     *
     * **Promotion d'une carte à grille unique.** Tant qu'une carte n'a aucune couche visuelle, sa
     * grille racine vaut à la fois image et collision (`LayerKind::Legacy`) ; dès qu'elle en a une,
     * la grille racine n'est plus dessinée. Ajouter la **première**
     * couche visuelle y recopie donc la grille racine, types refusés mis à part : ce qu'on voyait
     * reste ce qu'on voit, et l'auteur retire ensuite ce qui ne relève que de la collision. Une
     * couche ajoutée à une carte qui en a déjà naît vide.
     *
     * @param kind `Ground` ou `Decor`.
     * @param name Nom affiché par l'éditeur.
     * @return Le rang de la couche créée, ou `std::nullopt` si @p kind n'est pas visuel.
     */
    std::optional<std::size_t> addLayer(LayerKind kind, std::string name);

    /// Retire la couche visuelle au rang @p index.
    /// @return `false` si @p index est hors bornes ou désigne une couche non visuelle.
    bool removeLayer(std::size_t index);

    /// Renomme la couche visuelle au rang @p index. Un nom identique n'empile rien.
    bool renameLayer(std::size_t index, std::string name);

    /// Change le rôle (sol ↔ décor) de la couche visuelle au rang @p index.
    bool setLayerKind(std::size_t index, LayerKind kind);

    /**
     * @brief Échange la couche visuelle au rang @p index avec sa voisine visuelle, en avant
     *        (@p forward) ou en arrière. L'entrée de collision ne se franchit pas : elle n'a pas
     *        de rang de dessin.
     * @return Le nouveau rang ; @p index inchangé si la couche est déjà au bout ;
     *         `std::nullopt` si @p index est hors bornes ou non visuel.
     */
    std::optional<std::size_t> moveLayer(std::size_t index, bool forward);

    /// Peint @p type en (@p column, @p row) de la couche visuelle @p index.
    /// @return `false` (rien d'empilé) si le rang, la case ou le type est refusé, ou si la case
    ///         porte déjà ce type.
    bool paintLayerTile(std::size_t index, int column, int row, TileType type);

    /// Applique @p block (indexé `[ligne][colonne]`) sur la couche visuelle @p index, découpé aux
    /// bords, en **un** pas d'annulation. Un bloc contenant un type refusé est refusé en entier.
    bool paintLayerRegion(std::size_t index, int originColumn, int originRow,
                          const std::vector<std::vector<TileType>>& block);

    /** @} */

    /**
     * @name Entités de carte (`LOT-04`, `LOT-11`)
     *
     * Un rang désigne une entrée de `entities()`. Plusieurs entités peuvent partager une case ;
     * `entityAt` rend la **dernière** posée, celle qu'on voit au-dessus. Chaque mutateur empile un
     * pas d'annulation, sauf s'il est refusé ou sans effet.
     * @{
     */

    /// Pose @p entity en fin de liste.
    /// @return Son rang, ou `std::nullopt` si sa case est hors de la grille (`EX-LVL-017`).
    std::optional<std::size_t> placeEntity(MapEntity entity);

    /// Déplace l'entité @p index en @p position. Refusé hors bornes ; sans effet sur place.
    bool moveEntity(std::size_t index, GridPosition position);

    /// Retire l'entité @p index.
    bool removeEntity(std::size_t index);

    /// Assigne la propriété @p key de l'entité @p index. Une valeur identique n'empile rien.
    bool setEntityProperty(std::size_t index, const std::string& key, PropertyValue value);

    /// Retire la propriété @p key de l'entité @p index, si elle l'a.
    bool removeEntityProperty(std::size_t index, const std::string& key);

    /// @return Le rang de la dernière entité posée en @p position, s'il y en a une.
    [[nodiscard]] std::optional<std::size_t> entityAt(GridPosition position) const;

    /** @} */

    /**
     * @brief Redimensionne la grille (`EX-EDIT-005`).
     *
     * Agrandir complète les nouvelles cases en `Empty` ; réduire **tronque** silencieusement le
     * contenu hors des nouvelles bornes (entrée, entités ou pièces assignées perdues sont
     * invalidées en conséquence). L'avertissement de perte revient à l'appelant `HMI`, avant
     * d'invoquer `resize`.
     * @param width  Nouvelle largeur, en cases (> 0).
     * @param height Nouvelle hauteur, en cases (> 0).
     */
    void resize(int width, int height);

    /**
     * @brief Indique si redimensionner à (@p width, @p height) supprimerait du contenu déjà posé.
     *
     * Requête **pure** (n'altère rien) : vraie si les nouvelles bornes excluraient l'entrée, une
     * entité ou une pièce assignée actuellement posées. Permet à
     * `HMI` d'avertir avant d'appeler `resize` (`EX-EDIT-012`), sans dupliquer la logique de
     * troncature déjà portée par `resize`.
     * @param width  Largeur envisagée, en cases (> 0).
     * @param height Hauteur envisagée, en cases (> 0).
     * @return `true` si l'entrée, une entité ou une pièce assignée serait perdue.
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

    /// @return Les pièces assignées par case (`EX-EDIT-043`).
    [[nodiscard]] const std::vector<TileTextureOverride>& textureOverrides() const noexcept {
        return _textureOverrides;
    }

    /// @return Les couches de tuiles du niveau (`LOT-04`), dans leur ordre de superposition.
    ///
    /// Une entrée `Collision` ou `Legacy` y reflète la grille racine telle que le fichier l'a
    /// promue ; c'est `tileMap()` qui fait foi, et `toLevel()` l'y recopie. Les couches visuelles
    /// s'éditent par les mutateurs de couches (`LOT-11`).
    [[nodiscard]] const std::vector<TileLayer>& layers() const noexcept {
        return _layers;
    }

    /// @return Les entités placées sur la carte (`LOT-04`), éditées par les mutateurs d'entités.
    [[nodiscard]] const std::vector<MapEntity>& entities() const noexcept {
        return _entities;
    }

    /**
     * @brief Convertit le brouillon en `Level` **validé** (`EX-EDIT-007`), en repassant par la
     *        même validation que `LevelLoader` (sérialise puis recharge : aucune règle
     *        dupliquée, `EX-EDIT-010`).
     * @return Un résultat récupérable : niveau valide, ou message d'erreur exploitable si le
     *         brouillon est incomplet (pas d'entrée, …).
     */
    [[nodiscard]] LevelLoadResult toLevel() const;

private:
    LevelDraft(std::string name, TileMap tileMap);

    /// Logique de `paintTile`, sans `pushUndo()` : réutilisée cellule par cellule par
    /// `paintRegion` pour n'empiler qu'un seul snapshot par opération de bloc.
    void paintTileInternal(int column, int row, TileType type);

    /// Vrai si @p index désigne une couche visuelle de `_layers`.
    [[nodiscard]] bool isVisualLayerIndex(std::size_t index) const noexcept;

    /// Vrai si @p index désigne une entité de `_entities`.
    [[nodiscard]] bool isEntityIndex(std::size_t index) const noexcept {
        return index < _entities.size();
    }

    /// Logique de `setEntry`, sans `pushUndo()`.
    void setEntryInternal(int column, int row);

    /// Retire la pièce assignée à @p position, s'il y en a une.
    void removeTextureOverrideAt(GridPosition position);

    /// État complet du brouillon, hors historique (utilisé pour les snapshots undo/redo).
    struct State {
        std::string name;
        TileMap tileMap;
        std::optional<GridPosition> entry;
        std::vector<TileLayer> layers;
        std::vector<MapEntity> entities;
        std::vector<TileTextureOverride> textureOverrides;
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
    std::vector<TileLayer> _layers;
    std::vector<MapEntity> _entities;
    std::vector<TileTextureOverride> _textureOverrides;
    std::vector<State> _undoHistory;
    std::vector<State> _redoHistory;
};

}  // namespace core
