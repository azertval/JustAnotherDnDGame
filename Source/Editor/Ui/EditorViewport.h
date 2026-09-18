// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <QRhiWidget>
#include <QString>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Core/Combat/TacticalTerrain.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/LevelDraft.h"
#include "Core/Levels/LevelProperties.h"
#include "Core/Levels/TileLayer.h"
#include "Core/Levels/TileType.h"
#include "Core/Time/FixedTimestep.h"
#include "Core/World/EntityKinds.h"
#include "Editor/Logic/DiskGuard.h"
#include "Editor/Logic/EditContextTarget.h"
#include "Editor/Logic/EditorDiagnostics.h"
#include "Editor/Logic/EditorKeyBindings.h"
#include "Editor/Logic/EditorTool.h"
#include "Editor/Logic/LayerView.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/SceneResources.h"

/**
 * @file Editor/Ui/EditorViewport.h
 * @brief Le canevas de l'éditeur : le brouillon qu'on peint, et l'essai immédiat.
 *
 * Deux états, jamais mêlés. En **édition**, le brouillon est dessiné à plat — une couleur par type
 * de tuile, les entités par leur marqueur — sous les aides d'édition. En **essai**, la carte est
 * jouée par `hmi::WorldPlay` et dessinée par `hmi::WorldSceneRenderer` : la mise en scène du jeu,
 * à l'identique (`EX-EDIT-055`). Un essai qui montrerait autre chose que le jeu ne vérifierait
 * rien.
 */

namespace hmi {
class DraftRenderer;
class WorldPlay;
class WorldSceneRenderer;
struct EditorReferences;
}  // namespace hmi

namespace hmi {

class EditorViewport : public QRhiWidget, public EditContextTarget {
    Q_OBJECT

public:
    explicit EditorViewport(QWidget* parent = nullptr);
    ~EditorViewport() override;
    EditorViewport(const EditorViewport&) = delete;
    EditorViewport& operator=(const EditorViewport&) = delete;

    /// Type de tuile peint au clic (choisi dans la palette).
    void setActiveTile(core::TileType type) noexcept {
        _activeTile = type;
    }
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
    void resetCamera() noexcept;

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
    [[nodiscard]] float zoom() const noexcept {
        return _camera.zoom();
    }
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

signals:
    void activeLayerChanged(hmi::LayerSlot slot);
    void layerViewChanged();
    void entitySelectionChanged(std::optional<std::size_t> index);
    void statusMessage(const QString& message);
    void draftChanged();
    void toolChanged(hmi::EditorTool tool);
    void hoveredCellChanged(std::optional<core::GridPosition> cell);
    void zoomChanged(float zoom);

protected:
    void initialize(QRhiCommandBuffer* commandBuffer) override;
    void render(QRhiCommandBuffer* commandBuffer) override;
    void releaseResources() override;
    bool event(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    /// Crée le pipeline de dessin du brouillon, puis ouvre la carte de départ.
    void createResources();
    /// Dessine le brouillon et ses aides d'édition.
    void renderDraft(QRhiCommandBuffer* commandBuffer);
    /// Avance l'essai des pas fixes dus, puis le dessine comme le jeu.
    void renderPlaytest(QRhiCommandBuffer* commandBuffer, float elapsedSeconds);
    /// Termine l'essai et rend la main à l'édition (brouillon intact).
    void stopPlaytest();
    /// Direction que composent les touches enfoncées, normalisée — la règle de `GameView.qml`.
    [[nodiscard]] core::Vector2 heldDirection() const;

    void updateEditCamera();
    [[nodiscard]] core::Vector2 screenPosition(const QMouseEvent* event) const;
    [[nodiscard]] float minManualZoom() const;
    [[nodiscard]] float maxManualZoom() const;
    [[nodiscard]] std::optional<core::GridPosition> cellAt(const QMouseEvent* event);
    [[nodiscard]] core::GridPosition clampedCell(const QMouseEvent* event);
    void paintAt(const QMouseEvent* event);
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
    bool paintActiveRegion(int originColumn, int originRow,
                           const std::vector<std::vector<core::TileType>>& block);
    [[nodiscard]] const core::TileMap& activeLayerTiles() const;
    void handleEntityPress(const QMouseEvent* event);
    void handleEntityRelease(const QMouseEvent* event);
    [[nodiscard]] int pixelWidth() const;
    [[nodiscard]] int pixelHeight() const;

    using Clock = std::chrono::steady_clock;

    hmi::SceneResources _scene;
    std::unique_ptr<hmi::DraftRenderer> _draftRenderer;
    hmi::EditorKeyBindings _editorBindings;
    Clock::time_point _previousFrame;

    core::LevelDraft _draft;
    hmi::Camera2D _camera;
    bool _manualCamera = false;
    float _manualZoom = 1.0F;
    core::Vector2 _manualCenter{};
    bool _rightDragging = false;
    core::Vector2 _rightDragLastScreen{};
    core::TileType _activeTile = core::TileType::Solid;
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

    // --- Essai immédiat : la carte jouée par le moteur du jeu ---
    std::unique_ptr<WorldPlay> _play;
    std::unique_ptr<WorldSceneRenderer> _world;
    core::FixedTimestep _timestep;
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
