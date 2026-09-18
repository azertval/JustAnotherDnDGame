// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Editor/Ui/EditorViewport.h"

#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <memory>
#include <optional>
#include <utility>

// QRhi est une API privée de QtGui : l'en-tête vit sous rhi/, pas parmi les classes publiques.
#include <rhi/qrhi.h>

#include "Core/Levels/Level.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelWriter.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileTypeName.h"
#include "Core/Math/Vector2.h"
#include "Core/World/WorldTravel.h"
#include "Editor/Logic/EntityGesture.h"
#include "Editor/Logic/EntityReferences.h"
#include "Editor/Logic/LevelFileOperations.h"
#include "Editor/Logic/LevelNameValidation.h"
#include "Editor/Ui/DraftRenderer.h"
#include "HMI/Game/WorldPlay.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/Graphics/WorldSceneRenderer.h"
#include "HMI/HmiLog.h"
#include "HMI/Platform/ExecutableDirectory.h"

namespace hmi {

namespace {

// Fond du canevas en édition (gris ardoise) et pendant l'essai (parchemin, la couleur du jeu) : les
// valeurs que portaient les jetons de la charte, retirés de l'éditeur (LOT-EDITOR-01).
constexpr std::array<float, 4> EDIT_CLEAR_COLOR = {0x1e / 255.0F, 0x22 / 255.0F, 0x2b / 255.0F,
                                                   1.0F};
constexpr std::array<float, 4> PLAYTEST_CLEAR_COLOR = {0xd0 / 255.0F, 0xc0 / 255.0F, 0xa0 / 255.0F,
                                                       1.0F};

[[nodiscard]] std::filesystem::path keybindingsPath() {
    return hmi::executableDirectory() / "Settings" / "keybindings.json";
}

[[nodiscard]] std::filesystem::path levelsDirectory() {
    return hmi::executableDirectory() / "Levels";
}

/// Carte ouverte au lancement : la première carte du jeu.
constexpr const char* START_MAP_ID = "coliseum";

/// @return L'identifiant de carte de @p path : son chemin sous `Levels/`, sans extension, en
///         barres obliques — celui qu'un portail écrit. Hors du dossier : le nom du fichier.
[[nodiscard]] std::string mapIdOf(const std::filesystem::path& path) {
    std::error_code error;
    std::filesystem::path relative = std::filesystem::relative(path, levelsDirectory(), error);
    if (error || relative.empty() || *relative.begin() == "..") {
        relative = path.filename();
    }
    relative.replace_extension();
    return relative.generic_string();
}

/// Touches du déplacement de l'essai : celles du jeu (`GameView.qml`), azerty comme qwerty.
[[nodiscard]] bool isUpKey(int key) {
    return key == Qt::Key_Up || key == Qt::Key_W || key == Qt::Key_Z;
}
[[nodiscard]] bool isDownKey(int key) {
    return key == Qt::Key_Down || key == Qt::Key_S;
}
[[nodiscard]] bool isLeftKey(int key) {
    return key == Qt::Key_Left || key == Qt::Key_A || key == Qt::Key_Q;
}
[[nodiscard]] bool isRightKey(int key) {
    return key == Qt::Key_Right || key == Qt::Key_D;
}

}  // namespace

EditorViewport::EditorViewport(QWidget* parent)
    : QRhiWidget(parent),
      _editorBindings(hmi::EditorKeyBindings::load(keybindingsPath())),
      _draft(core::LevelDraft::empty("Nouvelle carte", 24, 14)),
      _camera(1280, 720),
      _mapId(_draft.name()) {}

EditorViewport::~EditorViewport() = default;

void EditorViewport::setTool(hmi::EditorTool tool) {
    if (_tool == tool) {
        return;
    }
    _tool = tool;
    emit toolChanged(tool);
    if (_draftRenderer) {
        _draftRenderer->invalidate();  // le terrain de rencontre ne se montre qu'à l'outil Entité.
    }
}

int EditorViewport::pixelWidth() const {
    return std::max(1, static_cast<int>(static_cast<qreal>(width()) * devicePixelRatio()));
}

int EditorViewport::pixelHeight() const {
    return std::max(1, static_cast<int>(static_cast<qreal>(height()) * devicePixelRatio()));
}

void EditorViewport::createResources() {
    HMI_LOG_INFO("Viewport : initialisation du rendu sur QRhi (" + std::to_string(pixelWidth()) +
                 "x" + std::to_string(pixelHeight()) + ").");
    _scene.create(rhi(), _scene.context().updates);
    _draftRenderer =
        std::make_unique<hmi::DraftRenderer>(_scene.sprites(), _scene.atlas(), _scene.textures());

    // La première carte du jeu, comme brouillon. Échec récupérable : on garde le brouillon vierge.
    const std::filesystem::path levelPath =
        levelsDirectory() / (std::string{START_MAP_ID} + ".json");
    core::LevelLoadResult result = core::LevelLoader::loadFromFile(levelPath);
    if (result.ok()) {
        _draft = core::LevelDraft::fromLevel(*result.level);
        _mapId = START_MAP_ID;
        markDraftMutated();
    } else {
        HMI_LOG_WARNING("Editeur : echec du chargement de la carte de depart : " + result.error);
    }
}

void EditorViewport::updateEditCamera() {
    _camera.setViewportSize(pixelWidth(), pixelHeight());
    if (_manualCamera) {
        _camera.setZoom(_manualZoom);
        _camera.setCenter(_manualCenter);
        return;
    }
    const int levelWidth = _draft.tileMap().width();
    const int levelHeight = _draft.tileMap().height();
    _camera.setZoom(hmi::Camera2D::fitZoom(
        static_cast<float>(pixelWidth()), static_cast<float>(pixelHeight()),
        static_cast<float>(levelWidth), static_cast<float>(levelHeight), 0.92F));
    _camera.setCenter(core::Vector2{static_cast<float>(levelWidth) * 0.5F,
                                    static_cast<float>(levelHeight) * 0.5F});
}

core::Vector2 EditorViewport::screenPosition(const QMouseEvent* event) const {
    const qreal ratio = devicePixelRatio();
    return core::Vector2{static_cast<float>(event->position().x() * ratio),
                         static_cast<float>(event->position().y() * ratio)};
}

float EditorViewport::minManualZoom() const {
    return hmi::Camera2D::fitZoom(static_cast<float>(pixelWidth()),
                                  static_cast<float>(pixelHeight()),
                                  static_cast<float>(_draft.tileMap().width()),
                                  static_cast<float>(_draft.tileMap().height()), 0.92F);
}

float EditorViewport::maxManualZoom() const {
    // Laisse au moins 4 cases visibles sur le plus petit axe de l'écran. Borné au minimum : une
    // carte plus petite que 4 cases rendrait sinon ce maximum inférieur au minimum.
    constexpr float MINIMUM_VISIBLE_CELLS = 4.0F;
    const float smallerAxis = static_cast<float>((std::min)(pixelWidth(), pixelHeight()));
    const float rawMax = smallerAxis / (MINIMUM_VISIBLE_CELLS * hmi::Camera2D::PIXELS_PER_UNIT);
    return (std::max)(rawMax, minManualZoom());
}

std::optional<core::GridPosition> EditorViewport::cellAt(const QMouseEvent* event) {
    updateEditCamera();
    const core::Vector2 world = _camera.screenToWorld(screenPosition(event));
    const int column = static_cast<int>(std::floor(world.x));
    const int row = static_cast<int>(std::floor(world.y));
    if (!_draft.tileMap().inBounds(column, row)) {
        return std::nullopt;
    }
    return core::GridPosition{.column = column, .row = row};
}

core::GridPosition EditorViewport::clampedCell(const QMouseEvent* event) {
    updateEditCamera();
    const core::Vector2 world = _camera.screenToWorld(screenPosition(event));
    const int width = _draft.tileMap().width();
    const int height = _draft.tileMap().height();
    return core::GridPosition{
        .column = std::clamp(static_cast<int>(std::floor(world.x)), 0, width - 1),
        .row = std::clamp(static_cast<int>(std::floor(world.y)), 0, height - 1)};
}

void EditorViewport::paintAt(const QMouseEvent* event) {
    if (const std::optional<core::GridPosition> cell = cellAt(event)) {
        paintActiveRegion(cell->column, cell->row, {{_activeTile}});
    }
}

bool EditorViewport::paintActiveRegion(int originColumn, int originRow,
                                       const std::vector<std::vector<core::TileType>>& block) {
    if (!_activeLayer) {
        _draft.paintRegion(originColumn, originRow, block);
        _dirty = true;
        markDraftMutated();
        return true;
    }
    for (const std::vector<core::TileType>& row : block) {
        for (const core::TileType type : row) {
            if (!core::isVisualLayerTileType(type)) {
                if (!_refusalReported) {
                    _refusalReported = true;
                    emit statusMessage(
                        QStringLiteral("\"%1\" cannot be painted on a visual layer: the entry "
                                       "lives in the collision grid.")
                            .arg(QString::fromStdString(std::string{core::tileTypeName(type)})));
                }
                return false;
            }
        }
    }
    const bool changed =
        block.size() == 1 && block.front().size() == 1
            ? _draft.paintLayerTile(*_activeLayer, originColumn, originRow, block.front().front())
            : _draft.paintLayerRegion(*_activeLayer, originColumn, originRow, block);
    if (changed) {
        _dirty = true;
        markDraftMutated();
    }
    return changed;
}

const core::TileMap& EditorViewport::activeLayerTiles() const {
    if (_activeLayer && *_activeLayer < _draft.layers().size()) {
        return _draft.layers()[*_activeLayer].tiles;
    }
    return _draft.tileMap();
}

void EditorViewport::applyRectangle(core::GridPosition a, core::GridPosition b) {
    const int minColumn = std::min(a.column, b.column);
    const int maxColumn = std::max(a.column, b.column);
    const int minRow = std::min(a.row, b.row);
    const int maxRow = std::max(a.row, b.row);
    const std::vector<std::vector<core::TileType>> block(
        static_cast<std::size_t>(maxRow - minRow + 1),
        std::vector<core::TileType>(static_cast<std::size_t>(maxColumn - minColumn + 1),
                                    _activeTile));
    paintActiveRegion(minColumn, minRow, block);  // un seul pas d'annulation pour tout le rectangle
}

void EditorViewport::copySelection() {
    if (!_selection) {
        return;
    }
    const core::GridPosition mn = _selection->first;
    const core::GridPosition mx = _selection->second;
    const core::TileMap& map = activeLayerTiles();
    _clipboard.clear();
    for (int row = mn.row; row <= mx.row; ++row) {
        std::vector<core::TileType> line;
        for (int column = mn.column; column <= mx.column; ++column) {
            line.push_back(map.tile(column, row));
        }
        _clipboard.push_back(std::move(line));
    }
    emit statusMessage(QStringLiteral("Region copied (%1 × %2).")
                           .arg(mx.column - mn.column + 1)
                           .arg(mx.row - mn.row + 1));
}

void EditorViewport::pasteClipboard() {
    if (_clipboard.empty() || !_hoverCell) {
        return;
    }
    if (paintActiveRegion(_hoverCell->column, _hoverCell->row, _clipboard)) {
        emit statusMessage(QStringLiteral("Region pasted."));
    }
    _refusalReported = false;
}

std::optional<std::pair<core::GridPosition, core::GridPosition>> EditorViewport::highlight() const {
    if (_dragging) {
        return std::make_pair(
            core::GridPosition{.column = std::min(_dragStart.column, _dragCurrent.column),
                               .row = std::min(_dragStart.row, _dragCurrent.row)},
            core::GridPosition{.column = std::max(_dragStart.column, _dragCurrent.column),
                               .row = std::max(_dragStart.row, _dragCurrent.row)});
    }
    return _selection;
}

void EditorViewport::markDraftMutated() {
    syncEditingState();
    if (_draftRenderer) {
        _draftRenderer->invalidate();
    }
    emit draftChanged();
}

// Crée (ou recrée) les ressources graphiques quand QRhiWidget fournit son interface de rendu.
void EditorViewport::initialize(QRhiCommandBuffer* commandBuffer) {
    if (_scene.context().rhi == rhi()) {
        return;  // même interface : les ressources déjà créées restent valides.
    }
    // Changement d'interface (première image, ou widget passé sous une autre fenêtre) : tout ce
    // qui tient une texture est caduc. Un essai en cours s'arrête — il repartirait de l'entrée.
    stopPlaytest();
    releaseResources();
    _scene.context().rhi = rhi();
    _scene.context().updates = _scene.context().rhi->nextResourceUpdateBatch();
    createResources();
    commandBuffer->resourceUpdate(_scene.context().updates);
    _scene.context().updates = nullptr;
    _previousFrame = Clock::now();
}

void EditorViewport::releaseResources() {
    // Ce qui tient une texture meurt avant elle : le rendu de l'essai et celui du brouillon
    // d'abord, puis la grappe de ressources dans l'ordre que `SceneResources::release` fixe.
    _world.reset();
    _draftRenderer.reset();
    _scene.release();
}

void EditorViewport::render(QRhiCommandBuffer* commandBuffer) {
    if (!_scene.created()) {
        return;  // initialize() n'a pas encore pu créer les ressources.
    }
    const Clock::time_point now = Clock::now();
    const float elapsedSeconds = std::chrono::duration<float>(now - _previousFrame).count();
    _previousFrame = now;

    if (_play) {
        renderPlaytest(commandBuffer, elapsedSeconds);
    } else {
        renderDraft(commandBuffer);
    }
    // Animation continue : la prochaine image est demandée dès celle-ci terminée.
    update();
}

void EditorViewport::renderDraft(QRhiCommandBuffer* commandBuffer) {
    _scene.context().updates = _scene.context().rhi->nextResourceUpdateBatch();
    _scene.sprites().beginFrame();
    const std::array<float, 4> clear = EDIT_CLEAR_COLOR;
    updateEditCamera();
    if (_camera.zoom() != _lastEmittedZoom) {
        _lastEmittedZoom = _camera.zoom();
        emit zoomChanged(_lastEmittedZoom);
    }
    _draftRenderer->setLayerView(_layerView);
    hmi::DraftEntityOverlay entityOverlay;
    entityOverlay.selectedEntity = _selectedEntity;
    entityOverlay.terrains = &_terrains;
    entityOverlay.showTerrain = _tool == hmi::EditorTool::Entity;
    _draftRenderer->render(_draft, _camera, _showGrid, highlight(), entityOverlay);
    // Téléversement unique puis passe unique (cf. `hmi::SpriteBatch`, enregistrement en deux
    // phases).
    _scene.sprites().submit(commandBuffer, renderTarget(), _scene.context().updates, clear.data());
    _scene.context().updates = nullptr;
}

core::Vector2 EditorViewport::heldDirection() const {
    const auto held = [this](bool (*matches)(int)) {
        return std::ranges::any_of(_heldKeys, matches);
    };
    float x = (held(isRightKey) ? 1.0F : 0.0F) - (held(isLeftKey) ? 1.0F : 0.0F);
    float y = (held(isDownKey) ? 1.0F : 0.0F) - (held(isUpKey) ? 1.0F : 0.0F);
    const float length = std::sqrt((x * x) + (y * y));
    if (length > 0.0F) {
        x /= length;
        y /= length;
    }
    return {x, y};
}

void EditorViewport::renderPlaytest(QRhiCommandBuffer* commandBuffer, float elapsedSeconds) {
    const int steps = _timestep.advance(elapsedSeconds);
    for (int step = 0; step < steps && _play; ++step) {
        const core::ExplorationIntent intent{.move = heldDirection(),
                                             .interact = _interactRequested};
        _interactRequested = false;
        const WorldPlayStep result = _play->step(intent, _timestep.fixedDeltaSeconds());
        _playSceneDirty = _playSceneDirty || result.sceneChanged || result.heroMoved;
        for (const core::ExplorationEvent& event : result.events) {
            // L'éditeur n'ouvre ni dialogue ni combat : il dit ce que le jeu ferait, et l'essai
            // continue. C'est l'information qu'on vient chercher en essayant une carte.
            QString message;
            switch (event.kind) {
                case core::ExplorationEventKind::MapEntered:
                    message = QStringLiteral("Playtest: entered map %1.");
                    break;
                case core::ExplorationEventKind::Dialogue:
                    message = QStringLiteral("Playtest: dialogue %1 would open.");
                    break;
                case core::ExplorationEventKind::Encounter:
                    message = QStringLiteral("Playtest: encounter %1 would start.");
                    break;
                case core::ExplorationEventKind::PortalLocked:
                    message = QStringLiteral("Playtest: portal locked, requires %1.");
                    break;
                case core::ExplorationEventKind::PortalBroken:
                    message = QStringLiteral("Playtest: broken portal to %1.");
                    break;
                case core::ExplorationEventKind::Interacted:
                    message = QStringLiteral("Playtest: interacted with %1.");
                    break;
            }
            emit statusMessage(message.arg(QString::fromStdString(event.value)));
        }
    }

    if (!_world) {
        _world = std::make_unique<WorldSceneRenderer>(hmi::executableDirectory() / "Assets");
    }
    if (!_world->ensureResources(rhi())) {
        return;
    }
    if (_playSceneDirty) {
        _world->setSnapshot(_play->snapshot());
        _playSceneDirty = false;
    }
    const core::CellPoint hero = _play->session().heroPoint();
    _world->setFocus({hero.column, hero.row});
    const std::array<float, 4> clear = PLAYTEST_CLEAR_COLOR;
    _world->render(commandBuffer, renderTarget(), clear.data());
}

bool EditorViewport::event(QEvent* event) {
    switch (event->type()) {
        case QEvent::FocusOut:
            _heldKeys.clear();
            break;
        case QEvent::Leave:
            if (_hoverCell) {
                _hoverCell.reset();
                emit hoveredCellChanged(std::nullopt);
            }
            break;
        default:
            break;
    }
    return QRhiWidget::event(event);
}

void EditorViewport::keyPressEvent(QKeyEvent* event) {
    if (_play) {
        if (event->key() == Qt::Key_Escape) {
            stopPlaytest();
            return;
        }
        if (event->isAutoRepeat()) {
            return;
        }
        if (event->key() == Qt::Key_E || event->key() == Qt::Key_Space) {
            _interactRequested = true;
            return;
        }
        _heldKeys.insert(event->key());
        return;
    }
    // Annuler/refaire/enregistrer/essai/grille/recadrer/copier/coller sont des actions Qt uniques
    // (`hmi::EditorActions`) : aucun second traitement ici, sous peine de double déclenchement.
    if (event->isAutoRepeat()) {
        return;
    }
    // Retrait de l'entité sélectionnée (outil Entité, LOT-11) : Suppr, comme dans tout éditeur.
    if (event->key() == Qt::Key_Delete && _tool == hmi::EditorTool::Entity && _selectedEntity) {
        removeEntity(*_selectedEntity);
        return;
    }
    QRhiWidget::keyPressEvent(event);
}

void EditorViewport::keyReleaseEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) {
        return;
    }
    _heldKeys.erase(event->key());
}

void EditorViewport::save() {
    const core::LevelLoadResult validated = _draft.toLevel();
    if (!validated.ok()) {
        HMI_LOG_WARNING("Editeur : enregistrement refuse (brouillon invalide) : " +
                        validated.error);
        emit statusMessage(
            QStringLiteral("Cannot save: %1").arg(QString::fromStdString(validated.error)));
        return;
    }
    const std::filesystem::path path = levelsDirectory() / (_mapId + ".json");
    if (core::LevelWriter::saveToFile(*validated.level, path)) {
        _dirty = false;
        HMI_LOG_INFO("Editeur : carte enregistree : " + path.string());
        emit statusMessage(
            QStringLiteral("Map saved: %1").arg(QString::fromStdString(path.filename().string())));
    } else {
        HMI_LOG_ERROR("Editeur : echec d'ecriture de la carte : " + path.string());
        emit statusMessage(QStringLiteral("Failed to write file."));
    }
}

bool EditorViewport::renameOpenLevel(const std::string& newName) {
    if (!hmi::isValidLevelName(newName)) {
        emit statusMessage(QStringLiteral("Invalid map name."));
        return false;
    }
    const std::string trimmed = hmi::trimLevelName(newName);
    if (trimmed == _draft.name()) {
        return true;
    }
    const std::filesystem::path oldPath = levelsDirectory() / (_mapId + ".json");
    const std::filesystem::path renamedPath = oldPath.parent_path() / (trimmed + ".json");
    if (std::filesystem::exists(oldPath)) {
        // Carte déjà enregistrée : on renomme le fichier, même chemin que le navigateur de cartes.
        const hmi::LevelFileOperations ops(oldPath.parent_path());
        const hmi::FileOperationResult result = ops.rename(oldPath, trimmed);
        if (!result.ok()) {
            HMI_LOG_WARNING("Editeur : renommage refuse : " + result.error);
            emit statusMessage(
                QStringLiteral("Rename failed: %1").arg(QString::fromStdString(result.error)));
            return false;
        }
    }
    _draft.setName(trimmed);
    _mapId = mapIdOf(renamedPath);
    markDraftMutated();
    HMI_LOG_INFO("Editeur : carte renommee en « " + trimmed + " ».");
    emit statusMessage(QStringLiteral("Map renamed: %1").arg(QString::fromStdString(trimmed)));
    return true;
}

void EditorViewport::openLevel(const std::filesystem::path& path) {
    core::LevelLoadResult loaded = core::LevelLoader::loadFromFile(path);
    if (!loaded.ok()) {
        HMI_LOG_WARNING("Editeur : ouverture impossible (" + path.string() + ") : " + loaded.error);
        emit statusMessage(
            QStringLiteral("Cannot open: %1").arg(QString::fromStdString(loaded.error)));
        return;
    }
    stopPlaytest();
    _draft = core::LevelDraft::fromLevel(*loaded.level);
    _mapId = mapIdOf(path);
    // Une carte ouverte repart de sa collision, tout affiché, rien de sélectionné : les réglages
    // de la carte précédente n'ont aucun sens pour celle-ci.
    _layerView.reset();
    setActiveLayer(std::nullopt);
    selectEntity(std::nullopt);
    _dirty = false;
    _manualCamera = false;
    markDraftMutated();
    HMI_LOG_INFO("Editeur : carte ouverte : " + path.string());
    emit statusMessage(
        QStringLiteral("Map opened: %1").arg(QString::fromStdString(path.filename().string())));
}

void EditorViewport::startPlaytest() {
    if (_play) {
        return;
    }
    core::LevelLoadResult validated = _draft.toLevel();
    if (!validated.ok()) {
        HMI_LOG_WARNING("Editeur : essai refuse (brouillon invalide) : " + validated.error);
        emit statusMessage(
            QStringLiteral("Cannot playtest: %1").arg(QString::fromStdString(validated.error)));
        return;
    }
    // Le brouillon est servi sous l'identifiant de sa carte ; toute autre carte vient du disque,
    // comme en jeu. Un portail qui ramène ici retrouve donc le brouillon, pas le fichier d'avant.
    auto edited = std::make_shared<const core::Level>(std::move(*validated.level));
    core::WorldTravel::MapLoader fromDisk = core::WorldTravel::directoryLoader(levelsDirectory());
    core::WorldTravel::MapLoader loader = [mapId = _mapId, edited,
                                           fromDisk](std::string_view requested) {
        if (requested == mapId) {
            core::LevelLoadResult served;
            served.level = *edited;
            return served;
        }
        return fromDisk(requested);
    };
    auto play =
        std::make_unique<WorldPlay>(std::move(loader), hmi::executableDirectory() / "Assets");
    if (!play->enter(_mapId, {})) {
        HMI_LOG_WARNING("Editeur : essai refuse, la carte ne s'ouvre pas.");
        emit statusMessage(QStringLiteral("Cannot playtest: %1").arg(QString{}));
        return;
    }
    _play = std::move(play);
    _playSceneDirty = true;
    _heldKeys.clear();
    _interactRequested = false;
    _timestep = core::FixedTimestep{};
    setFocus();
    HMI_LOG_INFO("Editeur : essai immediat demarre.");
    emit statusMessage(QStringLiteral("Playtesting — Esc to return to editing."));
}

void EditorViewport::stopPlaytest() {
    if (!_play) {
        return;
    }
    _play.reset();
    _heldKeys.clear();
    if (_draftRenderer) {
        _draftRenderer->invalidate();  // ré-affiche le brouillon (intact).
    }
    emit statusMessage(QStringLiteral("Back to editing."));
}

void EditorViewport::undo() {
    if (_draft.undo()) {
        _dirty = true;
        markDraftMutated();
    }
}

void EditorViewport::redo() {
    if (_draft.redo()) {
        _dirty = true;
        markDraftMutated();
    }
}

void EditorViewport::toggleGrid() noexcept {
    _showGrid = !_showGrid;
}

void EditorViewport::resetCamera() noexcept {
    _manualCamera = false;
}

void EditorViewport::resizeLevel(int width, int height) {
    _draft.resize(width, height);
    _dirty = true;
    markDraftMutated();
    emit statusMessage(QStringLiteral("Map resized: %1 × %2").arg(width).arg(height));
}

bool EditorViewport::wouldResizeDrop(int width, int height) const {
    return _draft.wouldResizeDropContent(width, height);
}

int EditorViewport::levelWidth() const {
    return _draft.tileMap().width();
}

int EditorViewport::levelHeight() const {
    return _draft.tileMap().height();
}

void EditorViewport::mousePressEvent(QMouseEvent* event) {
    if (_play) {
        return;
    }
    if (event->button() == Qt::RightButton) {
        _rightDragging = true;  // le bouton droit déplace la vue.
        _rightDragLastScreen = screenPosition(event);
        return;
    }
    if (event->button() != Qt::LeftButton) {
        return;
    }
    switch (_tool) {
        case hmi::EditorTool::Paint:
            _painting = true;
            paintAt(event);
            break;
        case hmi::EditorTool::Rectangle:
        case hmi::EditorTool::Selection:
            _dragging = true;
            _dragStart = clampedCell(event);
            _dragCurrent = _dragStart;
            break;
        case hmi::EditorTool::Entity:
            handleEntityPress(event);
            break;
    }
}

void EditorViewport::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        _rightDragging = false;
        return;
    }
    if (event->button() != Qt::LeftButton || _play) {
        return;
    }
    if (_tool == hmi::EditorTool::Entity) {
        handleEntityRelease(event);
    }
    _refusalReported = false;
    if (_dragging) {
        _dragCurrent = clampedCell(event);
        if (_tool == hmi::EditorTool::Rectangle) {
            applyRectangle(_dragStart, _dragCurrent);
        } else if (_tool == hmi::EditorTool::Selection) {
            _selection = std::make_pair(
                core::GridPosition{.column = std::min(_dragStart.column, _dragCurrent.column),
                                   .row = std::min(_dragStart.row, _dragCurrent.row)},
                core::GridPosition{.column = std::max(_dragStart.column, _dragCurrent.column),
                                   .row = std::max(_dragStart.row, _dragCurrent.row)});
        }
        _dragging = false;
    }
    _painting = false;
}

void EditorViewport::mouseMoveEvent(QMouseEvent* event) {
    if (_play) {
        return;
    }
    if (_rightDragging) {
        const core::Vector2 current = screenPosition(event);
        const core::Vector2 delta = current - _rightDragLastScreen;
        if (!_manualCamera) {
            // Premier glisser : le cadrage manuel part du cadrage automatique, sans saut de vue.
            updateEditCamera();
            _manualZoom = _camera.zoom();
            _manualCenter = _camera.center();
            _manualCamera = true;
        }
        const float scale = hmi::Camera2D::PIXELS_PER_UNIT * _manualZoom;
        _manualCenter.x -= delta.x / scale;
        _manualCenter.y -= delta.y / scale;
        _rightDragLastScreen = current;
    }
    const std::optional<core::GridPosition> cell = cellAt(event);  // cible du collage (Ctrl+V)
    if (cell != _hoverCell) {
        _hoverCell = cell;
        emit hoveredCellChanged(_hoverCell);
    }
    if (_painting) {
        paintAt(event);
    } else if (_dragging) {
        _dragCurrent = clampedCell(event);
    }
}

void EditorViewport::wheelEvent(QWheelEvent* event) {
    if (_play) {
        return;
    }
    const int notches = event->angleDelta().y() / 120;  // 120 = un cran de molette.
    if (notches == 0) {
        return;
    }
    if (!_manualCamera) {
        updateEditCamera();
        _manualZoom = _camera.zoom();
        _manualCenter = _camera.center();
        _manualCamera = true;
    }
    _manualZoom =
        std::clamp(_manualZoom + static_cast<float>(notches), minManualZoom(), maxManualZoom());
}

// --- Couches et entités (LOT-11) ---

void EditorViewport::syncEditingState() {
    const LayerSlot active = validActiveLayer(_draft.layers(), _activeLayer);
    _layerView.sync(_draft.layers().size());
    if (active != _activeLayer) {
        _activeLayer = active;
        emit activeLayerChanged(_activeLayer);
    }
    if (_selectedEntity && *_selectedEntity >= _draft.entities().size()) {
        _selectedEntity.reset();
        emit entitySelectionChanged(_selectedEntity);
    }
    if (_grabbedEntity && *_grabbedEntity >= _draft.entities().size()) {
        _grabbedEntity.reset();
    }
    refreshDiagnostics();
}

void EditorViewport::refreshDiagnostics() {
    static const hmi::EditorReferences emptyReferences;
    const hmi::EditorReferences& references =
        _references != nullptr ? *_references : emptyReferences;
    _referenceContext = hmi::referenceContext(references, _mapId, _draft.entities());
    const std::vector<core::EntityIssue> issues =
        core::validateMapEntities(_draft.entities(), _referenceContext);
    _terrains = core::analyzeEncounterTerrain(_draft.tileMap(), _draft.entities(),
                                              references.encounters, &references.bestiary);
    _diagnostics = hmi::editorDiagnostics(_draft.entities(), issues, _terrains);
}

void EditorViewport::setActiveLayer(LayerSlot slot) {
    const LayerSlot valid = validActiveLayer(_draft.layers(), slot);
    if (valid == _activeLayer) {
        return;
    }
    _activeLayer = valid;
    _selection.reset();  // une sélection copiée d'une autre couche tromperait le collage.
    emit activeLayerChanged(_activeLayer);
}

void EditorViewport::setMapLayerVisible(LayerSlot slot, bool visible) {
    _layerView.setVisible(slot, visible);
    if (_draftRenderer) {
        _draftRenderer->invalidate();
    }
    emit layerViewChanged();
}

void EditorViewport::setMapLayerOpacity(LayerSlot slot, float opacity) {
    _layerView.setOpacity(slot, opacity);
    if (_draftRenderer) {
        _draftRenderer->invalidate();
    }
    emit layerViewChanged();
}

void EditorViewport::addMapLayer(core::LayerKind kind, const std::string& name) {
    const std::optional<std::size_t> index = _draft.addLayer(kind, name);
    if (!index) {
        return;
    }
    _dirty = true;
    markDraftMutated();
    setActiveLayer(*index);
}

void EditorViewport::removeMapLayer(std::size_t index) {
    if (_draft.removeLayer(index)) {
        _dirty = true;
        markDraftMutated();
    }
}

void EditorViewport::moveMapLayer(std::size_t index, bool forward) {
    const std::optional<std::size_t> moved = _draft.moveLayer(index, forward);
    if (!moved || *moved == index) {
        return;
    }
    _layerView.swap(index, *moved);
    const bool followActive = _activeLayer == index;
    _dirty = true;
    markDraftMutated();
    if (followActive) {
        setActiveLayer(*moved);
    }
}

void EditorViewport::renameMapLayer(std::size_t index, const std::string& name) {
    if (!name.empty() && _draft.renameLayer(index, name)) {
        _dirty = true;
        markDraftMutated();
    }
}

void EditorViewport::setEditorReferences(const EditorReferences* references) {
    _references = references;
    refreshDiagnostics();
    emit draftChanged();  // les panneaux relisent avertissements et choix proposés.
}

void EditorViewport::setEntityKindToPlace(std::string type) {
    _entityKindToPlace = std::move(type);
}

void EditorViewport::selectEntity(std::optional<std::size_t> index) {
    if (index && *index >= _draft.entities().size()) {
        index.reset();
    }
    if (index == _selectedEntity) {
        return;
    }
    _selectedEntity = index;
    emit entitySelectionChanged(_selectedEntity);
}

void EditorViewport::setEntityProperty(std::size_t index, const std::string& key,
                                       core::PropertyValue value) {
    if (_draft.setEntityProperty(index, key, std::move(value))) {
        _dirty = true;
        markDraftMutated();
    }
}

void EditorViewport::removeEntity(std::size_t index) {
    if (!_draft.removeEntity(index)) {
        return;
    }
    if (_selectedEntity == index) {
        _selectedEntity.reset();
        emit entitySelectionChanged(_selectedEntity);
    } else if (_selectedEntity && *_selectedEntity > index) {
        selectEntity(*_selectedEntity - 1);  // la même entité, un rang plus haut.
    }
    _grabbedEntity.reset();
    _dirty = true;
    markDraftMutated();
    emit statusMessage(QStringLiteral("Entity removed."));
}

void EditorViewport::handleEntityPress(const QMouseEvent* event) {
    const std::optional<core::GridPosition> cell = cellAt(event);
    if (!cell) {
        return;
    }
    const bool forcePlace = event->modifiers().testFlag(Qt::ControlModifier);
    const hmi::EntityGestureDecision decision =
        hmi::resolveEntityPress(_draft, *cell, _entityKindToPlace, forcePlace);
    _grabbedEntity.reset();
    switch (decision.action) {
        case hmi::EntityGestureAction::Ignore:
        case hmi::EntityGestureAction::Move:  // jamais rendu à l'appui.
            break;
        case hmi::EntityGestureAction::Deselect:
            selectEntity(std::nullopt);
            break;
        case hmi::EntityGestureAction::Select:
            selectEntity(decision.entityIndex);
            _grabbedEntity = decision.entityIndex;
            _entityPressCell = decision.cell;
            break;
        case hmi::EntityGestureAction::Place: {
            const core::EntityKind* const kind = core::findEntityKind(_entityKindToPlace);
            core::MapEntity entity = kind != nullptr ? core::makeEntity(*kind, decision.cell)
                                                     : core::MapEntity{.type = _entityKindToPlace,
                                                                       .position = decision.cell,
                                                                       .properties = {}};
            if (const std::optional<std::size_t> placed = _draft.placeEntity(std::move(entity))) {
                _dirty = true;
                markDraftMutated();
                selectEntity(*placed);
                emit statusMessage(QStringLiteral("%1 placed at (%2, %3).")
                                       .arg(QString::fromStdString(_entityKindToPlace))
                                       .arg(decision.cell.column)
                                       .arg(decision.cell.row));
            }
            break;
        }
    }
}

void EditorViewport::handleEntityRelease(const QMouseEvent* event) {
    const std::optional<core::GridPosition> cell = cellAt(event);
    const std::optional<std::size_t> grabbed = _grabbedEntity;
    _grabbedEntity.reset();
    if (!cell) {
        return;
    }
    const hmi::EntityGestureDecision decision =
        hmi::resolveEntityRelease(grabbed, _entityPressCell, *cell);
    if (decision.action == hmi::EntityGestureAction::Move &&
        _draft.moveEntity(decision.entityIndex, decision.cell)) {
        _dirty = true;
        markDraftMutated();
        emit statusMessage(QStringLiteral("Entity moved to (%1, %2).")
                               .arg(decision.cell.column)
                               .arg(decision.cell.row));
    }
}

}  // namespace hmi
