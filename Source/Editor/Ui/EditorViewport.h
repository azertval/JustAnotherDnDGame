// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <QColor>
#include <QGraphicsView>
#include <QString>
#include <QTimer>
#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Core/Combat/IsoProjection.h"
#include "Core/Combat/TacticalTerrain.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/LevelDraft.h"
#include "Core/Levels/LevelProperties.h"
#include "Core/Levels/TileLayer.h"
#include "Core/Levels/TileType.h"
#include "Core/Time/FixedTimestep.h"
#include "Core/World/EntityKinds.h"
#include "Editor/Logic/BrushGesture.h"
#include "Editor/Logic/CanvasPicking.h"
#include "Editor/Logic/CanvasScene.h"
#include "Editor/Logic/DiskGuard.h"
#include "Editor/Logic/EditContextTarget.h"
#include "Editor/Logic/EditorDiagnostics.h"
#include "Editor/Logic/EditorKeyBindings.h"
#include "Editor/Logic/EditorTool.h"
#include "Editor/Logic/LayerView.h"
#include "Editor/Logic/PieceCatalog.h"
#include "HMI/Graphics/ComposedScene.h"
#include "HMI/Graphics/PlaceAppearance.h"
#include "HMI/Graphics/WorldSceneComposer.h"

class QGraphicsScene;
class QPainter;

/**
 * @file Editor/Ui/EditorViewport.h
 * @brief Le canevas de l'éditeur : le lieu qu'on édite, rendu comme dans le jeu, et l'essai
 *        immédiat (`LOT-EDITOR-02`, `EX-EDIT-059`).
 *
 * Une `QGraphicsView` et **un seul élément peint** (décision D2) : il parcourt la `ComposedScene`
 * que la composition du jeu produit — mêmes primitives, même ordre — et la peint par `QPainter`
 * (`hmi::paintComposedScene`), en ne touchant que la partie visible. Par-dessus, les aides
 * d'édition : quadrillage en losanges, case survolée, masque de collision, aperçu des outils,
 * marqueurs d'entité.
 *
 * Deux vues, en bascule (décision D1) : **iso** par défaut, le lieu tel qu'on le jouera ; **à
 * plat**, une case par unité et les types en couleurs (`hmi::DraftRenderer`), pour lire types et
 * collision. Tout geste passe par le pointage (`Editor/Logic/CanvasPicking.h`) et parle en cases :
 * les outils ne savent pas quelle vue est affichée.
 *
 * Deux états, jamais mêlés. En **édition**, le brouillon est la seule source. En **essai**, la
 * carte est jouée par `hmi::WorldPlay` et composée comme dans le jeu (`EX-EDIT-055`) ; le même
 * peintre la dessine, cadrée sur le héros.
 */

namespace hmi {
class DraftRenderer;
class SceneImages;
class WorldPlay;
struct EditorReferences;
}  // namespace hmi

namespace hmi {

/// @brief Le canevas : le brouillon peint en iso ou à plat, les gestes des outils, l'essai.
class EditorViewport : public QGraphicsView, public EditContextTarget {
    Q_OBJECT

public:
    explicit EditorViewport(QWidget* parent = nullptr);
    ~EditorViewport() override;
    EditorViewport(const EditorViewport&) = delete;
    EditorViewport& operator=(const EditorViewport&) = delete;

    // --- Pinceau (LOT-EDITOR-03) ---
    /// Arme le pinceau d'un type de tuile (palette des types) : il peint la couche active.
    void setActiveTile(core::TileType type);
    /**
     * @brief Arme le pinceau d'une pièce (palette des pièces) : elle va sur sa couche, qui devient
     *        la couche active (`hmi::pieceTargetLayer`).
     * @param floor La pièce est un sol.
     */
    void setActivePiece(const std::string& piece, bool floor);
    /// Arme la gomme de la couche active.
    void setEraser();
    /// @return Le pinceau armé.
    [[nodiscard]] const CanvasBrush& brush() const noexcept {
        return _brush;
    }
    /// @return Le catalogue des pièces du lieu de la carte ouverte (`hmi::pieceCatalog`).
    [[nodiscard]] std::vector<PieceCatalogGroup> pieceCatalog() const;
    /// @return Le dossier des images du lieu (`Assets/Scene/<lieu>`), vide sans lieu.
    [[nodiscard]] std::filesystem::path placeDirectory() const;
    /// @return Vrai si la collision de la case survolée est forcée à la main.
    [[nodiscard]] bool hoveredCellForced() const;
    void setTool(hmi::EditorTool tool);
    [[nodiscard]] hmi::EditorKeyBindings& editorBindings() noexcept {
        return _editorBindings;
    }

    /// Enregistre le brouillon sous `Levels/<nom>.json`, après validation (`EX-EDIT-007`).
    /// @return `true` si le fichier a été écrit.
    bool save();
    /// Ouvre @p path comme brouillon ; sort d'un éventuel essai.
    /// @return `true` si la carte s'est ouverte ; en cas d'échec, le brouillon reste intact.
    bool openLevel(const std::filesystem::path& path);
    /// Le brouillon porte des modifications : sa révision n'est plus celle de la dernière
    /// ouverture ou du dernier enregistrement (`core::LevelDraft::revision`). Défaire jusqu'à
    /// l'état enregistré rend un brouillon propre.
    [[nodiscard]] bool isDirty() const noexcept {
        return _draft.revision() != _savedRevision;
    }
    bool renameOpenLevel(const std::string& newName);

    // --- Sauvegarde automatique et garde du fichier (LOT-EDITOR-01) ---
    /// @return L'identifiant de la carte ouverte (`capital/martpart`).
    [[nodiscard]] const std::string& mapId() const noexcept {
        return _mapId;
    }
    /// @return Le fichier de la carte ouverte, qu'il existe ou non.
    [[nodiscard]] std::filesystem::path levelPath() const;
    /// @return Le brouillon en JSON non validé, pour la sauvegarde automatique.
    [[nodiscard]] std::string draftJson() const {
        return _draft.toJson();
    }
    /**
     * @brief Reprend un brouillon sauvegardé automatiquement : il remplace le brouillon courant et
     *        reste marqué modifié, jusqu'à ce qu'on l'enregistre.
     * @return `false` si le brouillon ne se relit pas (incomplet, format inconnu) : rien n'a
     *         changé, et le fichier de reprise reste en place.
     */
    bool restoreDraft(const std::string& mapId, const std::string& draftJson);
    /// @return Ce qui a changé sur disque depuis la dernière lecture ou écriture de la carte.
    [[nodiscard]] DiskChange diskChange() const;
    /// Prend l'état actuel du fichier pour référence : l'auteur a choisi de garder son brouillon,
    /// le prochain enregistrement écrasera la version du disque (mise de côté par l'appelant).
    void acceptDiskVersion();

    /// Joue le brouillon avec le moteur du jeu (`EX-EDIT-008`, `EX-EDIT-055`).
    void startPlaytest();
    [[nodiscard]] bool playtesting() const noexcept {
        return _play != nullptr;
    }

    void undo() override;
    void redo() override;
    [[nodiscard]] bool canUndo() const override {
        return _draft.canUndo();
    }
    [[nodiscard]] bool canRedo() const override {
        return _draft.canRedo();
    }
    void copy() override {
        copySelection();
    }
    void paste() override {
        pasteClipboard();
    }
    [[nodiscard]] bool canCopy() const override {
        return _selection.has_value();
    }
    [[nodiscard]] bool canPaste() const override {
        return !_clipboard.empty();
    }

    void toggleGrid() noexcept;
    /// Recadre la vue sur toute la carte.
    void resetCamera();

    // --- Vues du canevas (LOT-EDITOR-02) ---
    /// Bascule entre la vue iso (le lieu) et la vue à plat (les types), cadrage recalculé.
    void setCanvasView(CanvasView view);
    [[nodiscard]] CanvasView canvasView() const noexcept {
        return _view;
    }
    /// Reliefs en transparence : on voit ce qu'on pointe derrière un mur.
    void setSeeThroughRelief(bool enabled);
    [[nodiscard]] bool seeThroughRelief() const noexcept {
        return _seeThroughRelief;
    }
    /// @return La couleur d'un type de tuile, celle de la vue à plat (pour la mini-carte).
    [[nodiscard]] QColor tileColor(core::TileType type) const;
    /// @return Les pièces de la case survolée (`street · wall-left`), vide sinon.
    [[nodiscard]] std::string hoveredPieces() const;
    /**
     * @brief Les quatre coins de la partie visible, en coordonnées de grille continues (colonne,
     *        ligne) : un rectangle à plat, un losange en iso. Pour la mini-carte.
     */
    [[nodiscard]] std::array<core::Vector2, 4> visibleGridCorners() const;
    /// Centre la vue sur un point de grille continu (colonne, ligne).
    void centerOnGridPoint(core::Vector2 gridPoint);

    void resizeLevel(int width, int height);
    [[nodiscard]] bool wouldResizeDrop(int width, int height) const;
    [[nodiscard]] int levelWidth() const;
    [[nodiscard]] int levelHeight() const;

    [[nodiscard]] const core::LevelDraft& draft() const noexcept {
        return _draft;
    }
    [[nodiscard]] std::optional<core::GridPosition> hoveredCell() const noexcept {
        return _hoverCell;
    }
    /// @return Le facteur d'agrandissement : 1 quand une unité monde fait 16 pixels.
    [[nodiscard]] float zoom() const noexcept;
    [[nodiscard]] EditorTool activeTool() const noexcept {
        return _tool;
    }

    // --- Couches et entités (LOT-11) ---
    void setActiveLayer(LayerSlot slot);
    [[nodiscard]] LayerSlot activeLayer() const noexcept {
        return _activeLayer;
    }
    [[nodiscard]] const LayerViewState& layerView() const noexcept {
        return _layerView;
    }
    void setMapLayerVisible(LayerSlot slot, bool visible);
    void setMapLayerOpacity(LayerSlot slot, float opacity);
    void setMapLayerDimmed(LayerSlot slot, bool dimmed);
    void setMapLayerLocked(LayerSlot slot, bool locked);
    void addMapLayer(core::LayerKind kind, const std::string& name);
    void removeMapLayer(std::size_t index);
    void moveMapLayer(std::size_t index, bool forward);
    void renameMapLayer(std::size_t index, const std::string& name);

    /// Catalogues que les entités référencent (non possédés), relus par la fenêtre.
    void setEditorReferences(const EditorReferences* references);
    void setEntityKindToPlace(std::string type);
    void selectEntity(std::optional<std::size_t> index);
    [[nodiscard]] std::optional<std::size_t> selectedEntity() const noexcept {
        return _selectedEntity;
    }
    void setEntityProperty(std::size_t index, const std::string& key, core::PropertyValue value);
    void removeEntity(std::size_t index);
    [[nodiscard]] const std::vector<EditorDiagnostic>& diagnostics() const noexcept {
        return _diagnostics;
    }
    [[nodiscard]] const core::EntityReferenceContext& entityReferenceContext() const noexcept {
        return _referenceContext;
    }

    /// Peint le canevas : appelé par l'élément unique de la scène, @p exposed en unités monde.
    void paintCanvas(QPainter& painter, const QRectF& exposed);

signals:
    void activeLayerChanged(hmi::LayerSlot slot);
    void layerViewChanged();
    void entitySelectionChanged(std::optional<std::size_t> index);
    void statusMessage(const QString& message);
    void draftChanged();
    void toolChanged(hmi::EditorTool tool);
    void hoveredCellChanged(std::optional<core::GridPosition> cell);
    void zoomChanged(float zoom);
    /// Le cadrage a bougé (défilement, agrandissement, vue) : la mini-carte suit.
    void framingChanged();
    void canvasViewChanged(hmi::CanvasView view);

protected:
    bool viewportEvent(QEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void scrollContentsBy(int dx, int dy) override;

private:
    class CanvasItem;

    /// Avance l'essai des pas fixes dus, puis recompose sa scène.
    void stepPlaytest();
    /// Termine l'essai et rend la main à l'édition (brouillon intact).
    void stopPlaytest();
    /// Direction que composent les touches enfoncées, normalisée — la règle de `GameView.qml`.
    [[nodiscard]] core::Vector2 heldDirection() const;

    // --- Peinture ---
    /// Recompose la scène iso du brouillon si elle est périmée.
    void ensureIsoScene();
    void paintIso(QPainter& painter, const QRectF& exposed);
    void paintFlat(QPainter& painter, const QRectF& exposed);
    void paintPlaytest(QPainter& painter, const QRectF& exposed);
    /// Les types en couleurs, en losanges : une carte sans lieu (aucune planche à peindre).
    void paintIsoTypeColors(QPainter& painter, const CellRange& cells);
    void paintIsoOverlays(QPainter& painter, const CellRange& cells, const IsoBandOpacity& bands);
    /// @return Le rectangle du monde qu'occupe le contenu de la vue courante, marge comprise.
    [[nodiscard]] QRectF contentBounds() const;
    /// Recalcule l'étendue de la scène (vue, taille de carte, essai) et redemande une peinture.
    void refreshBounds();
    /// Invalide la scène iso et redemande une peinture.
    void invalidateScene();
    /// @return La projection iso du brouillon.
    [[nodiscard]] core::IsoProjection projection() const;
    [[nodiscard]] core::Vector2 worldPosition(const QMouseEvent* event) const;
    void emitZoomIfChanged();

    [[nodiscard]] std::optional<core::GridPosition> cellAt(const QMouseEvent* event) const;
    [[nodiscard]] core::GridPosition clampedCell(const QMouseEvent* event) const;
    /// Donne un coup de pinceau sur la case pointée ; @p continuing : le geste prolonge un glisser.
    void paintAt(const QMouseEvent* event, bool continuing);
    /// Le pinceau armé, son type recalculé d'après la table du lieu courant.
    [[nodiscard]] CanvasBrush currentBrush() const;
    /// Applique le résultat d'un geste : brouillon redessiné, ou refus dit une fois par geste.
    void reportBrush(const BrushResult& result);
    /// Le masque des cases forcées, en losanges (iso) ou en carrés (à plat).
    void paintForcedMask(QPainter& painter, const CellRange& cells, bool iso);
    void applyRectangle(core::GridPosition a, core::GridPosition b);
    void copySelection();
    void pasteClipboard();
    [[nodiscard]] std::optional<std::pair<core::GridPosition, core::GridPosition>> highlight()
        const;
    /// Invalide le rendu du brouillon et notifie les panneaux (`draftChanged`) — après toute
    /// mutation de `_draft`.
    void markDraftMutated();
    void syncEditingState();
    void refreshDiagnostics();
    [[nodiscard]] const core::TileMap& activeLayerTiles() const;
    void handleEntityPress(const QMouseEvent* event);
    void handleEntityRelease(const QMouseEvent* event);
    [[nodiscard]] bool hasVisualLayers() const;

    using Clock = std::chrono::steady_clock;

    QGraphicsScene* _canvasScene;
    CanvasItem* _item;
    std::unique_ptr<SceneImages> _images;
    std::unique_ptr<DraftRenderer> _flat;
    hmi::EditorKeyBindings _editorBindings;

    core::LevelDraft _draft;
    CanvasView _view = CanvasView::Iso;
    bool _seeThroughRelief = false;
    /// Scène iso du brouillon, recomposée seulement quand il change.
    bool _isoSceneDirty = true;
    PlaceAppearance _appearance;
    std::string _appearancePlace;
    /// Le manifeste des pièces du lieu : le brouillon en tire emprises et collision.
    std::shared_ptr<const core::ScenePieceManifest> _manifest;
    WorldSceneSnapshot _snapshot;
    ComposedScene _isoScene;

    bool _rightDragging = false;
    QPoint _rightDragLast;
    CanvasBrush _brush;
    hmi::EditorTool _tool = hmi::EditorTool::Paint;
    bool _painting = false;
    bool _dragging = false;
    core::GridPosition _dragStart{};
    core::GridPosition _dragCurrent{};
    std::optional<core::GridPosition> _hoverCell;
    float _lastEmittedZoom = 0.0F;
    std::optional<std::pair<core::GridPosition, core::GridPosition>> _selection;
    std::vector<std::vector<core::TileType>> _clipboard;
    /// Identifiant de carte du brouillon (`capital/martpart`) : son chemin sous `Levels/`, sans
    /// extension ; son nom tant qu'il n'a jamais été ouvert ni enregistré.
    std::string _mapId;
    /// Révision du brouillon à la dernière ouverture ou au dernier enregistrement.
    std::uint64_t _savedRevision = 0;
    /// Empreinte du fichier de la carte à la dernière lecture ou écriture de l'éditeur.
    FileFingerprint _diskFingerprint;
    bool _showGrid = true;
    /// Le cadrage a été fixé pour ce contenu : un redimensionnement ne le refait pas.
    bool _framed = false;

    // --- Essai immédiat : la carte jouée par le moteur du jeu ---
    std::unique_ptr<WorldPlay> _play;
    QTimer _playTimer;
    /// Le cadrage d'édition, rendu à la fin de l'essai.
    QTransform _editTransform;
    QPointF _editCenter;
    Clock::time_point _previousFrame;
    core::FixedTimestep _timestep;
    WorldSceneSnapshot _playSnapshot;
    ComposedScene _playScene;
    /// Touches de déplacement enfoncées (codes `Qt::Key`).
    std::set<int> _heldKeys;
    /// Interaction demandée depuis le dernier pas.
    bool _interactRequested = false;
    /// La scène de l'essai doit être recomposée avant la prochaine image.
    bool _playSceneDirty = true;

    // --- Couches et entités (LOT-11) ---
    LayerSlot _activeLayer;
    hmi::LayerViewState _layerView;
    bool _refusalReported = false;
    const EditorReferences* _references = nullptr;
    std::string _entityKindToPlace;
    std::optional<std::size_t> _selectedEntity;
    std::optional<std::size_t> _grabbedEntity;
    core::GridPosition _entityPressCell{};
    core::EntityReferenceContext _referenceContext;
    std::vector<core::EncounterTerrain> _terrains;
    std::vector<EditorDiagnostic> _diagnostics;
};

}  // namespace hmi
