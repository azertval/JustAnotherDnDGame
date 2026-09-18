// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Editor/Ui/EditorViewport.h"

#include <QEvent>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QPolygonF>
#include <QScrollBar>
#include <QStyleOptionGraphicsItem>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <limits>
#include <memory>
#include <optional>
#include <utility>

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
#include "Editor/Ui/SceneImages.h"
#include "Editor/Ui/ScenePainter.h"
#include "HMI/Game/WorldPlay.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/EntityMarkers.h"
#include "HMI/HmiLog.h"
#include "HMI/Platform/ExecutableDirectory.h"

namespace hmi {

namespace {

// Fond du canevas en édition (gris ardoise) et pendant l'essai (parchemin, la couleur du jeu) : les
// valeurs que portaient les jetons de la charte, retirés de l'éditeur (LOT-EDITOR-01).
const QColor EDIT_BACKGROUND(0x1e, 0x22, 0x2b);
const QColor PLAYTEST_BACKGROUND(0xd0, 0xc0, 0xa0);

/// Un cran de molette agrandit ou réduit d'autant.
constexpr double ZOOM_STEP = 1.25;
/// Agrandissement maximal : une unité monde fait alors 8 × 16 pixels.
constexpr double MAX_PIXELS_PER_UNIT = 8.0 * Camera2D::PIXELS_PER_UNIT;
/// Cadence de l'essai : celle du jeu (60 images par seconde).
constexpr int PLAYTEST_FRAME_MS = 16;

[[nodiscard]] std::filesystem::path keybindingsPath() {
    return hmi::executableDirectory() / "Settings" / "keybindings.json";
}

[[nodiscard]] std::filesystem::path levelsDirectory() {
    return hmi::executableDirectory() / "Levels";
}

[[nodiscard]] std::filesystem::path assetsDirectory() {
    return hmi::executableDirectory() / "Assets";
}

/// Carte ouverte au lancement : la première carte du jeu.
constexpr const char* START_MAP_ID = "coliseum";

// Révision « jamais enregistrée » : celle d'un brouillon repris, qui reste modifié quoi qu'on
// fasse jusqu'à son enregistrement. Aucune révision réelle ne l'atteint.
constexpr std::uint64_t NEVER_SAVED = std::numeric_limits<std::uint64_t>::max();

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

[[nodiscard]] QPointF toQt(core::Vector2 point) {
    return {static_cast<double>(point.x), static_cast<double>(point.y)};
}

[[nodiscard]] QPolygonF diamondOf(const core::IsoProjection& projection, core::GridPosition cell) {
    const std::array<core::Vector2, 4> vertices = isoCellDiamond(projection, cell);
    return QPolygonF{{toQt(vertices[0]), toQt(vertices[1]), toQt(vertices[2]), toQt(vertices[3])}};
}

/// Le parallélogramme iso d'un rectangle de cases, bornes incluses.
[[nodiscard]] QPolygonF isoRegion(const core::IsoProjection& projection, core::GridPosition first,
                                  core::GridPosition last) {
    const auto at = [&](int column, int row) {
        return toQt(projection.gridToWorld({static_cast<float>(column), static_cast<float>(row)}));
    };
    return QPolygonF{{at(first.column, first.row), at(last.column + 1, first.row),
                      at(last.column + 1, last.row + 1), at(first.column, last.row + 1)}};
}

/// Un crayon d'une largeur en **pixels d'écran**, quel que soit l'agrandissement.
[[nodiscard]] QPen screenPen(const QColor& color, double width) {
    QPen pen(color, width);
    pen.setCosmetic(true);
    pen.setJoinStyle(Qt::MiterJoin);
    return pen;
}

[[nodiscard]] QColor withAlpha(QColor color, float alpha) {
    color.setAlphaF(std::clamp(alpha, 0.0F, 1.0F));
    return color;
}

}  // namespace

/**
 * @brief L'élément unique de la scène : il délègue sa peinture au canevas (décision D2).
 *
 * `ItemUsesExtendedStyleOption` donne le rectangle exposé : le canevas ne peint que ce qui se voit.
 */
class EditorViewport::CanvasItem final : public QGraphicsItem {
public:
    explicit CanvasItem(EditorViewport& owner) : _owner(owner) {
        setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);
    }

    [[nodiscard]] QRectF boundingRect() const override {
        return _bounds;
    }

    void setBounds(const QRectF& bounds) {
        if (bounds != _bounds) {
            prepareGeometryChange();
            _bounds = bounds;
        }
    }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* /*widget*/) override {
        _owner.paintCanvas(*painter, option->exposedRect);
    }

private:
    EditorViewport& _owner;
    QRectF _bounds;
};

EditorViewport::EditorViewport(QWidget* parent)
    : QGraphicsView(parent),
      _canvasScene(new QGraphicsScene(this)),
      _item(new CanvasItem(*this)),
      _images(std::make_unique<SceneImages>(assetsDirectory())),
      _editorBindings(hmi::EditorKeyBindings::load(keybindingsPath())),
      _draft(core::LevelDraft::empty("New map", 24, 14)),
      _mapId(_draft.name()) {
    _flat = std::make_unique<DraftRenderer>(DraftTextures{
        .atlas = const_cast<QImage*>(&_images->atlas()),  // identité opaque, jamais écrite
        .atlasWidth = _images->atlas().width(),
        .atlasHeight = _images->atlas().height(),
        .solid = _images->solid(),
        .marker = [images = _images.get()](const std::string& key) -> TextureHandle {
            return const_cast<QImage*>(images->marker(key));
        }});

    _canvasScene->addItem(_item);
    setScene(_canvasScene);
    setBackgroundBrush(EDIT_BACKGROUND);
    // Une seule peinture par image, de tout ce qui se voit : l'élément unique couvre la scène.
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setRenderHint(QPainter::SmoothPixmapTransform, false);
    setRenderHint(QPainter::Antialiasing, false);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setMouseTracking(true);
    setFrameShape(QFrame::NoFrame);

    _playTimer.setInterval(PLAYTEST_FRAME_MS);
    connect(&_playTimer, &QTimer::timeout, this, [this] { stepPlaytest(); });

    // La première carte du jeu, comme brouillon. Échec récupérable : on garde le brouillon vierge.
    const std::filesystem::path startPath =
        levelsDirectory() / (std::string{START_MAP_ID} + ".json");
    if (!openLevel(startPath)) {
        HMI_LOG_WARNING("Editeur : echec du chargement de la carte de depart.");
        markDraftMutated();
    }
}

EditorViewport::~EditorViewport() {
    // L'élément appelle le canevas : il part avant que les membres ne meurent.
    _canvasScene->removeItem(_item);
    delete _item;
}

void EditorViewport::setTool(hmi::EditorTool tool) {
    if (_tool == tool) {
        return;
    }
    _tool = tool;
    emit toolChanged(tool);
    viewport()->update();  // le terrain de rencontre ne se montre qu'à l'outil Entité.
}

// --- Cadrage
// --------------------------------------------------------------------------------------

core::IsoProjection EditorViewport::projection() const {
    return {_draft.tileMap().width(), _draft.tileMap().height()};
}

QRectF EditorViewport::contentBounds() const {
    if (_play) {
        const core::IsoProjection played(_playSnapshot.columns, _playSnapshot.rows);
        const core::Vector2 size = played.sceneSize();
        const double margin = played.tileWidth();
        return {-margin, -margin, size.x + (2 * margin), size.y + (2 * margin)};
    }
    if (_view == CanvasView::Flat) {
        return {-1.0, -1.0, static_cast<double>(_draft.tileMap().width()) + 2.0,
                static_cast<double>(_draft.tileMap().height()) + 2.0};
    }
    const core::IsoProjection iso = projection();
    const core::Vector2 size = iso.sceneSize();
    const double margin = iso.tileWidth();
    return {-margin, -margin, size.x + (2 * margin), size.y + (2 * margin)};
}

void EditorViewport::refreshBounds() {
    const QRectF bounds = contentBounds();
    _item->setBounds(bounds);
    _canvasScene->setSceneRect(bounds);
    viewport()->update();
}

void EditorViewport::resetCamera() {
    fitInView(contentBounds(), Qt::KeepAspectRatio);
    _framed = false;
    emitZoomIfChanged();
    emit framingChanged();
}

float EditorViewport::zoom() const noexcept {
    return static_cast<float>(transform().m11()) / Camera2D::PIXELS_PER_UNIT;
}

void EditorViewport::emitZoomIfChanged() {
    const float current = zoom();
    if (current != _lastEmittedZoom) {
        _lastEmittedZoom = current;
        emit zoomChanged(current);
    }
}

void EditorViewport::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);
    // Tant que l'auteur n'a pas cadré lui-même, la carte entière reste dans la vue.
    if (!_framed && !_play) {
        resetCamera();
    }
    emit framingChanged();
}

void EditorViewport::scrollContentsBy(int dx, int dy) {
    QGraphicsView::scrollContentsBy(dx, dy);
    emit framingChanged();
}

void EditorViewport::setCanvasView(CanvasView view) {
    if (_view == view) {
        return;
    }
    _view = view;
    refreshBounds();
    if (!_play) {
        resetCamera();
    }
    emit canvasViewChanged(_view);
}

void EditorViewport::setSeeThroughRelief(bool enabled) {
    _seeThroughRelief = enabled;
    viewport()->update();
}

std::array<core::Vector2, 4> EditorViewport::visibleGridCorners() const {
    const QRect pixels = viewport()->rect();
    const QPointF corners[] = {mapToScene(pixels.topLeft()), mapToScene(pixels.topRight()),
                               mapToScene(pixels.bottomRight()), mapToScene(pixels.bottomLeft())};
    std::array<core::Vector2, 4> grid{};
    const core::IsoProjection iso = projection();
    for (std::size_t index = 0; index < grid.size(); ++index) {
        const core::Vector2 world{static_cast<float>(corners[index].x()),
                                  static_cast<float>(corners[index].y())};
        grid[index] = _view == CanvasView::Flat ? world : iso.worldToGrid(world);
    }
    return grid;
}

void EditorViewport::centerOnGridPoint(core::Vector2 gridPoint) {
    if (_play) {
        return;
    }
    centerOn(toQt(_view == CanvasView::Flat ? gridPoint : projection().gridToWorld(gridPoint)));
    _framed = true;
    emit framingChanged();
}

core::Vector2 EditorViewport::worldPosition(const QMouseEvent* event) const {
    const QPointF scenePoint = mapToScene(event->position().toPoint());
    return {static_cast<float>(scenePoint.x()), static_cast<float>(scenePoint.y())};
}

std::optional<core::GridPosition> EditorViewport::cellAt(const QMouseEvent* event) const {
    const core::Vector2 world = worldPosition(event);
    if (_view == CanvasView::Flat) {
        return pickFlatCell(world, _draft.tileMap().width(), _draft.tileMap().height());
    }
    return pickIsoCell(projection(), world);
}

core::GridPosition EditorViewport::clampedCell(const QMouseEvent* event) const {
    const core::Vector2 world = worldPosition(event);
    if (_view == CanvasView::Flat) {
        return clampedFlatCell(world, _draft.tileMap().width(), _draft.tileMap().height());
    }
    return clampedIsoCell(projection(), world);
}

// --- Peinture
// -------------------------------------------------------------------------------------

void EditorViewport::invalidateScene() {
    _isoSceneDirty = true;
    viewport()->update();
}

void EditorViewport::ensureIsoScene() {
    if (!_isoSceneDirty) {
        return;
    }
    const std::string place = scenePlaceOf(_draft.layers());
    if (place != _appearancePlace) {
        _appearancePlace = place;
        _appearance = PlaceAppearance{};
        if (!place.empty()) {
            PlaceAppearanceResult read = PlaceAppearance::loadFromFile(assetsDirectory() / "Scene" /
                                                                       place / "appearance.json");
            if (read.ok()) {
                _appearance = std::move(read.appearance);
            } else {
                HMI_LOG_WARNING("Editeur : table d'apparence du lieu " + place + " illisible, " +
                                read.message);
            }
        }
    }
    _snapshot = canvasSnapshot(_draft, _appearance);
    _images->ensure(worldTexturePaths(_snapshot));
    _isoScene.clear();
    composeWorldScene(_isoScene, _snapshot, core::IsoProjection(_snapshot.columns, _snapshot.rows),
                      _images->textures());
    _isoScene.sort();
    _isoSceneDirty = false;
}

void EditorViewport::paintCanvas(QPainter& painter, const QRectF& exposed) {
    if (_play) {
        paintPlaytest(painter, exposed);
    } else if (_view == CanvasView::Flat) {
        paintFlat(painter, exposed);
    } else {
        paintIso(painter, exposed);
    }
}

void EditorViewport::paintPlaytest(QPainter& painter, const QRectF& exposed) {
    const core::Rect visible{
        {static_cast<float>(exposed.x()), static_cast<float>(exposed.y())},
        {static_cast<float>(exposed.width()), static_cast<float>(exposed.height())}};
    paintComposedScene(painter, _playScene, visible);
}

void EditorViewport::paintFlat(QPainter& painter, const QRectF& exposed) {
    const core::Rect visible{
        {static_cast<float>(exposed.x()), static_cast<float>(exposed.y())},
        {static_cast<float>(exposed.width()), static_cast<float>(exposed.height())}};
    DraftEntityOverlay overlay;
    overlay.selectedEntity = _selectedEntity;
    overlay.terrains = &_terrains;
    overlay.showTerrain = _tool == hmi::EditorTool::Entity;
    _flat->setLayerView(_layerView);
    paintComposedScene(painter, _flat->compose(_draft, visible, _showGrid, highlight(), overlay),
                       visible);
    if (_hoverCell) {
        painter.setPen(screenPen(QColor(255, 236, 140), 2.0));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(QRectF(_hoverCell->column, _hoverCell->row, 1.0, 1.0));
    }
}

void EditorViewport::paintIso(QPainter& painter, const QRectF& exposed) {
    ensureIsoScene();
    const core::IsoProjection iso = projection();
    const core::Rect visible{
        {static_cast<float>(exposed.x()), static_cast<float>(exposed.y())},
        {static_cast<float>(exposed.width()), static_cast<float>(exposed.height())}};
    const CellRange cells = isoCellsCovering(iso, visible);
    const IsoBandOpacity bands =
        isoBandOpacity(_draft.layers(), _layerView, _activeLayer, _seeThroughRelief);
    if (_snapshot.place.empty()) {
        // Une carte qui ne nomme aucun lieu n'a aucune pièce : ses types, en couleurs.
        paintIsoTypeColors(painter, cells);
    }
    paintComposedScene(painter, _isoScene, visible, [&bands](const ComposedQuad& quad) {
        return bandOpacity(bands, quad.layer);
    });
    paintIsoOverlays(painter, cells, bands);
}

void EditorViewport::paintIsoTypeColors(QPainter& painter, const CellRange& cells) {
    const core::IsoProjection iso = projection();
    const bool visual = hasVisualLayers();
    painter.setPen(Qt::NoPen);
    const auto paintGrid = [&](const core::TileMap& tiles, float opacity) {
        if (opacity <= 0.0F) {
            return;
        }
        for (int row = cells.firstRow; row <= cells.lastRow; ++row) {
            for (int column = cells.firstColumn; column <= cells.lastColumn; ++column) {
                if (!tiles.inBounds(column, row)) {
                    continue;
                }
                const core::TileType type = tiles.tile(column, row);
                if (type == core::TileType::Empty) {
                    continue;
                }
                painter.setBrush(withAlpha(_images->tileColor(type), opacity));
                painter.drawPolygon(diamondOf(iso, {.column = column, .row = row}));
            }
        }
    };
    if (!visual) {
        paintGrid(_draft.tileMap(), _layerView.display(std::nullopt, false).effectiveOpacity());
        return;
    }
    const std::vector<core::TileLayer>& layers = _draft.layers();
    for (std::size_t index = 0; index < layers.size(); ++index) {
        if (core::isVisualLayerKind(layers[index].kind)) {
            paintGrid(layers[index].tiles, _layerView.display(index, true).effectiveOpacity());
        }
    }
}

void EditorViewport::paintIsoOverlays(QPainter& painter, const CellRange& cells,
                                      const IsoBandOpacity& bands) {
    const core::IsoProjection iso = projection();
    if (cells.empty()) {
        return;
    }
    // Masque de collision : une teinte par catégorie de règle, comme la vue à plat.
    if (bands.collision > 0.0F) {
        painter.setPen(Qt::NoPen);
        const core::TileMap& map = _draft.tileMap();
        for (int row = cells.firstRow; row <= cells.lastRow; ++row) {
            for (int column = cells.firstColumn; column <= cells.lastColumn; ++column) {
                const core::TileType type = map.tile(column, row);
                QColor tint;
                if (core::isSolid(type)) {
                    tint = QColor::fromRgbF(0.85F, 0.20F, 0.20F);
                } else if (type == core::TileType::Entry) {
                    tint = QColor::fromRgbF(0.20F, 0.85F, 0.30F);
                } else {
                    continue;
                }
                painter.setBrush(withAlpha(tint, bands.collision));
                painter.drawPolygon(diamondOf(iso, {.column = column, .row = row}));
            }
        }
    }
    // Quadrillage en losanges : les lignes de grille, du premier au dernier bord visible.
    if (_showGrid) {
        painter.setPen(screenPen(QColor(255, 255, 255, 46), 1.0));
        const auto at = [&](int column, int row) {
            return toQt(iso.gridToWorld({static_cast<float>(column), static_cast<float>(row)}));
        };
        for (int column = cells.firstColumn; column <= cells.lastColumn + 1; ++column) {
            painter.drawLine(at(column, cells.firstRow), at(column, cells.lastRow + 1));
        }
        for (int row = cells.firstRow; row <= cells.lastRow + 1; ++row) {
            painter.drawLine(at(cells.firstColumn, row), at(cells.lastColumn + 1, row));
        }
    }
    // Aperçu du rectangle ou de la sélection.
    if (const auto zone = highlight()) {
        painter.setPen(screenPen(QColor(110, 190, 255), 1.0));
        painter.setBrush(QColor::fromRgbF(0.3F, 0.7F, 1.0F, 0.28F));
        painter.drawPolygon(isoRegion(iso, zone->first, zone->second));
    }
    // Terrain de la rencontre sélectionnée (outil Entité) : zone, puis case de chaque combattant.
    if (_tool == hmi::EditorTool::Entity && _selectedEntity) {
        painter.setPen(Qt::NoPen);
        for (const core::EncounterTerrain& terrain : _terrains) {
            if (terrain.entityIndex != *_selectedEntity) {
                continue;
            }
            const bool narrow =
                std::ranges::any_of(terrain.issues, [](const core::TacticalIssue& issue) {
                    return issue.code == core::TacticalIssueCode::AreaTooNarrow;
                });
            painter.setBrush(narrow ? QColor::fromRgbF(1.0F, 0.55F, 0.10F, 0.18F)
                                    : QColor::fromRgbF(0.30F, 0.70F, 1.00F, 0.18F));
            for (const core::GridPosition& cell : terrain.area) {
                painter.drawPolygon(diamondOf(iso, cell));
            }
            for (const core::CombatantPlacement& placement : terrain.placements) {
                const bool refused = std::ranges::any_of(
                    terrain.issues, [&placement](const core::TacticalIssue& issue) {
                        return issue.code != core::TacticalIssueCode::AreaTooNarrow &&
                               issue.cell == placement.position;
                    });
                painter.setBrush(refused ? QColor::fromRgbF(0.95F, 0.20F, 0.20F, 0.55F)
                                         : QColor::fromRgbF(0.25F, 0.85F, 0.35F, 0.55F));
                painter.drawPolygon(diamondOf(iso, placement.position));
            }
        }
    }
    // Entités : le marqueur de leur famille au centre de leur losange (LOT-39) ; leurs figurines
    // et leurs poignées viendront au LOT-EDITOR-05.
    const float markerSide = iso.tileHeight() * 0.7F;
    const std::vector<core::MapEntity>& entities = _draft.entities();
    for (std::size_t index = 0; index < entities.size(); ++index) {
        const core::MapEntity& entity = entities[index];
        if (!cells.contains(entity.position)) {
            continue;
        }
        const QPointF center = toQt(iso.tileToWorld(entity.position));
        const QRectF target(center.x() - (markerSide / 2.0), center.y() - (markerSide / 2.0),
                            markerSide, markerSide);
        if (const QImage* const marker = _images->marker(entityMarkerKey(entity.type))) {
            painter.drawImage(target, *marker);
        } else {
            painter.fillRect(target, QColor(255, 0, 255, 204));
        }
        if (_selectedEntity == index) {
            painter.setBrush(Qt::NoBrush);
            painter.setPen(screenPen(QColor(13, 13, 13), 4.0));
            painter.drawPolygon(diamondOf(iso, entity.position));
            painter.setPen(screenPen(QColor(255, 242, 89), 2.0));
            painter.drawPolygon(diamondOf(iso, entity.position));
        }
    }
    // La case survolée, par son losange : c'est elle que le prochain geste touchera.
    if (_hoverCell) {
        painter.setBrush(Qt::NoBrush);
        painter.setPen(screenPen(QColor(255, 236, 140), 2.0));
        painter.drawPolygon(diamondOf(iso, *_hoverCell));
    }
}

QColor EditorViewport::tileColor(core::TileType type) const {
    return _images->tileColor(type);
}

std::string EditorViewport::hoveredPieces() const {
    if (!_hoverCell || _play) {
        return {};
    }
    return cellPieces(_snapshot, *_hoverCell);
}

// --- Brouillon
// ------------------------------------------------------------------------------------

bool EditorViewport::hasVisualLayers() const {
    return std::ranges::any_of(_draft.layers(), [](const core::TileLayer& layer) {
        return core::isVisualLayerKind(layer.kind);
    });
}

void EditorViewport::paintAt(const QMouseEvent* event) {
    if (const std::optional<core::GridPosition> cell = cellAt(event)) {
        paintActiveRegion(cell->column, cell->row, {{_activeTile}});
    }
}

bool EditorViewport::paintActiveRegion(int originColumn, int originRow,
                                       const std::vector<std::vector<core::TileType>>& block) {
    // Une couche verrouillée se voit, mais aucun geste ne la peint (LOT-EDITOR-02, phase 3).
    if (_layerView.display(_activeLayer, hasVisualLayers()).locked) {
        if (!_refusalReported) {
            _refusalReported = true;
            emit statusMessage(QStringLiteral("The active layer is locked."));
        }
        return false;
    }
    if (!_activeLayer) {
        _draft.paintRegion(originColumn, originRow, block);
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
    // La scène iso est recomposée tout de suite : la barre d'état lit les pièces de la case.
    _isoSceneDirty = true;
    ensureIsoScene();
    refreshBounds();
    emit draftChanged();
}

// --- Essai immédiat
// -------------------------------------------------------------------------------

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

void EditorViewport::stepPlaytest() {
    if (!_play) {
        return;
    }
    const Clock::time_point now = Clock::now();
    const float elapsedSeconds = std::chrono::duration<float>(now - _previousFrame).count();
    _previousFrame = now;

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
    if (!_playSceneDirty) {
        return;
    }
    _playSceneDirty = false;
    const int previousColumns = _playSnapshot.columns;
    const int previousRows = _playSnapshot.rows;
    _playSnapshot = _play->snapshot();
    _images->ensure(worldTexturePaths(_playSnapshot));
    const core::IsoProjection played(_playSnapshot.columns, _playSnapshot.rows);
    _playScene.clear();
    composeWorldScene(_playScene, _playSnapshot, played, _images->textures());
    _playScene.sort();
    if (_playSnapshot.columns != previousColumns || _playSnapshot.rows != previousRows) {
        refreshBounds();  // un portail a mené sur une autre carte.
    }
    // La caméra suit le héros, comme en jeu (`hmi::worldCamera`).
    const core::CellPoint hero = _play->session().heroPoint();
    centerOn(toQt(played.gridToWorld({hero.column, hero.row})));
    viewport()->update();
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
    auto play = std::make_unique<WorldPlay>(std::move(loader), assetsDirectory());
    if (!play->enter(_mapId, {})) {
        HMI_LOG_WARNING("Editeur : essai refuse, la carte ne s'ouvre pas.");
        emit statusMessage(QStringLiteral("Cannot playtest: the map does not open."));
        return;
    }
    _editTransform = transform();
    _editCenter = mapToScene(viewport()->rect().center());
    _play = std::move(play);
    _playSnapshot = WorldSceneSnapshot{};
    _playSceneDirty = true;
    _heldKeys.clear();
    _interactRequested = false;
    _timestep = core::FixedTimestep{};
    _previousFrame = Clock::now();
    setBackgroundBrush(PLAYTEST_BACKGROUND);
    // L'agrandissement du jeu : entier, 1 jusqu'à 720 lignes, 2 au double (`hmi::worldCamera`).
    const double scale = static_cast<double>(Camera2D::PIXELS_PER_UNIT) *
                         std::max(1, viewport()->height() / WORLD_ART_HEIGHT_PIXELS);
    setTransform(QTransform::fromScale(scale, scale));
    stepPlaytest();
    _playTimer.start();
    setFocus();
    HMI_LOG_INFO("Editeur : essai immediat demarre.");
    emit statusMessage(QStringLiteral("Playtesting — Esc to return to editing."));
}

void EditorViewport::stopPlaytest() {
    if (!_play) {
        return;
    }
    _playTimer.stop();
    _play.reset();
    _playScene.clear();
    _heldKeys.clear();
    setBackgroundBrush(EDIT_BACKGROUND);
    refreshBounds();
    setTransform(_editTransform);
    centerOn(_editCenter);
    emit statusMessage(QStringLiteral("Back to editing."));
}

// --- Événements
// -----------------------------------------------------------------------------------

bool EditorViewport::viewportEvent(QEvent* event) {
    if (event->type() == QEvent::Leave && _hoverCell) {
        _hoverCell.reset();
        emit hoveredCellChanged(std::nullopt);
        viewport()->update();
    }
    return QGraphicsView::viewportEvent(event);
}

void EditorViewport::focusOutEvent(QFocusEvent* event) {
    _heldKeys.clear();
    QGraphicsView::focusOutEvent(event);
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
        QGraphicsView::keyPressEvent(event);
        return;
    }
    // Retrait de l'entité sélectionnée (outil Entité, LOT-11) : Suppr, comme dans tout éditeur.
    if (event->key() == Qt::Key_Delete && _tool == hmi::EditorTool::Entity && _selectedEntity) {
        removeEntity(*_selectedEntity);
        return;
    }
    QGraphicsView::keyPressEvent(event);
}

void EditorViewport::keyReleaseEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) {
        return;
    }
    _heldKeys.erase(event->key());
}

void EditorViewport::mousePressEvent(QMouseEvent* event) {
    if (_play) {
        return;
    }
    if (event->button() == Qt::RightButton) {
        _rightDragging = true;  // le bouton droit déplace la vue.
        _rightDragLast = event->position().toPoint();
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
            viewport()->update();
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
        _dragging = false;
        if (_tool == hmi::EditorTool::Rectangle) {
            applyRectangle(_dragStart, _dragCurrent);
        } else if (_tool == hmi::EditorTool::Selection) {
            _selection = std::make_pair(
                core::GridPosition{.column = std::min(_dragStart.column, _dragCurrent.column),
                                   .row = std::min(_dragStart.row, _dragCurrent.row)},
                core::GridPosition{.column = std::max(_dragStart.column, _dragCurrent.column),
                                   .row = std::max(_dragStart.row, _dragCurrent.row)});
        }
        viewport()->update();
    }
    _painting = false;
}

void EditorViewport::mouseMoveEvent(QMouseEvent* event) {
    if (_play) {
        return;
    }
    if (_rightDragging) {
        const QPoint current = event->position().toPoint();
        const QPoint delta = current - _rightDragLast;
        _rightDragLast = current;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        _framed = true;
    }
    const std::optional<core::GridPosition> cell = cellAt(event);  // cible du collage (Ctrl+V)
    if (cell != _hoverCell) {
        _hoverCell = cell;
        emit hoveredCellChanged(_hoverCell);
        viewport()->update();
    }
    if (_painting) {
        paintAt(event);
    } else if (_dragging) {
        const core::GridPosition current = clampedCell(event);
        if (current != _dragCurrent) {
            _dragCurrent = current;
            viewport()->update();
        }
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
    // Plus petit que la carte entière n'a pas d'usage ; plus grand que 8 pixels d'art non plus.
    const QRectF bounds = contentBounds();
    const double fit = std::min(static_cast<double>(viewport()->width()) / bounds.width(),
                                static_cast<double>(viewport()->height()) / bounds.height());
    const double current = transform().m11();
    const double wanted = std::clamp(current * std::pow(ZOOM_STEP, notches),
                                     std::min(fit, MAX_PIXELS_PER_UNIT), MAX_PIXELS_PER_UNIT);
    const double factor = wanted / current;
    scale(factor, factor);  // ancré sous le pointeur (`AnchorUnderMouse`).
    _framed = true;
    emitZoomIfChanged();
    emit framingChanged();
}

// --- Fichier
// --------------------------------------------------------------------------------------

bool EditorViewport::save() {
    const core::LevelLoadResult validated = _draft.toLevel();
    if (!validated.ok()) {
        HMI_LOG_WARNING("Editeur : enregistrement refuse (brouillon invalide) : " +
                        validated.error);
        emit statusMessage(
            QStringLiteral("Cannot save: %1").arg(QString::fromStdString(validated.error)));
        return false;
    }
    const std::filesystem::path path = levelPath();
    if (core::LevelWriter::saveToFile(*validated.level, path)) {
        _savedRevision = _draft.revision();
        _diskFingerprint = fingerprintFile(path);
        HMI_LOG_INFO("Editeur : carte enregistree : " + path.string());
        emit statusMessage(
            QStringLiteral("Map saved: %1").arg(QString::fromStdString(path.filename().string())));
    } else {
        HMI_LOG_ERROR("Editeur : echec d'ecriture de la carte : " + path.string());
        emit statusMessage(QStringLiteral("Failed to write file."));
        return false;
    }
    emit draftChanged();  // la barre d'état relit l'indicateur de modification.
    return true;
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
    _diskFingerprint = fingerprintFile(renamedPath);
    markDraftMutated();
    HMI_LOG_INFO("Editeur : carte renommee en « " + trimmed + " ».");
    emit statusMessage(QStringLiteral("Map renamed: %1").arg(QString::fromStdString(trimmed)));
    return true;
}

bool EditorViewport::openLevel(const std::filesystem::path& path) {
    core::LevelLoadResult loaded = core::LevelLoader::loadFromFile(path);
    if (!loaded.ok()) {
        HMI_LOG_WARNING("Editeur : ouverture impossible (" + path.string() + ") : " + loaded.error);
        emit statusMessage(
            QStringLiteral("Cannot open: %1").arg(QString::fromStdString(loaded.error)));
        return false;
    }
    stopPlaytest();
    _draft = core::LevelDraft::fromLevel(*loaded.level);
    _mapId = mapIdOf(path);
    // Une carte ouverte repart de sa collision, tout affiché, rien de sélectionné : les réglages
    // de la carte précédente n'ont aucun sens pour celle-ci.
    _layerView.reset();
    setActiveLayer(std::nullopt);
    selectEntity(std::nullopt);
    _selection.reset();
    _savedRevision = _draft.revision();
    _diskFingerprint = fingerprintFile(path);
    markDraftMutated();
    resetCamera();
    HMI_LOG_INFO("Editeur : carte ouverte : " + path.string());
    emit statusMessage(
        QStringLiteral("Map opened: %1").arg(QString::fromStdString(path.filename().string())));
    return true;
}

std::filesystem::path EditorViewport::levelPath() const {
    return levelsDirectory() / (_mapId + ".json");
}

bool EditorViewport::restoreDraft(const std::string& mapId, const std::string& draftJson) {
    core::LevelLoadResult loaded = core::LevelLoader::loadFromString(draftJson);
    if (!loaded.ok()) {
        HMI_LOG_WARNING("Editeur : brouillon de reprise illisible (" + mapId +
                        ") : " + loaded.error);
        return false;
    }
    stopPlaytest();
    _draft = core::LevelDraft::fromLevel(*loaded.level);
    _mapId = mapId;
    _layerView.reset();
    setActiveLayer(std::nullopt);
    selectEntity(std::nullopt);
    _selection.reset();
    // Le brouillon repris n'est pas le fichier : il reste modifié jusqu'à l'enregistrement. Le
    // fichier, lui, est pris tel qu'il est maintenant -- c'est contre lui que la garde compare.
    _savedRevision = NEVER_SAVED;
    _diskFingerprint = fingerprintFile(levelPath());
    markDraftMutated();
    resetCamera();
    HMI_LOG_INFO("Editeur : brouillon repris : " + mapId);
    emit statusMessage(
        QStringLiteral("Draft recovered: %1 (not saved yet).").arg(QString::fromStdString(mapId)));
    return true;
}

DiskChange EditorViewport::diskChange() const {
    return compareFingerprints(_diskFingerprint, fingerprintFile(levelPath()));
}

void EditorViewport::acceptDiskVersion() {
    _diskFingerprint = fingerprintFile(levelPath());
}

void EditorViewport::undo() {
    if (_draft.undo()) {
        markDraftMutated();
    }
}

void EditorViewport::redo() {
    if (_draft.redo()) {
        markDraftMutated();
    }
}

void EditorViewport::toggleGrid() noexcept {
    _showGrid = !_showGrid;
    viewport()->update();
}

void EditorViewport::resizeLevel(int width, int height) {
    _draft.resize(width, height);
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

// --- Couches et entités (LOT-11)
// ------------------------------------------------------------------

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
    _selection.reset();    // une sélection copiée d'une autre couche tromperait le collage.
    viewport()->update();  // le masque de collision iso suit la couche active.
    emit activeLayerChanged(_activeLayer);
}

void EditorViewport::setMapLayerVisible(LayerSlot slot, bool visible) {
    _layerView.setVisible(slot, visible);
    viewport()->update();
    emit layerViewChanged();
}

void EditorViewport::setMapLayerOpacity(LayerSlot slot, float opacity) {
    _layerView.setOpacity(slot, opacity);
    viewport()->update();
    emit layerViewChanged();
}

void EditorViewport::setMapLayerDimmed(LayerSlot slot, bool dimmed) {
    _layerView.setDimmed(slot, dimmed);
    viewport()->update();
    emit layerViewChanged();
}

void EditorViewport::setMapLayerLocked(LayerSlot slot, bool locked) {
    _layerView.setLocked(slot, locked);
    emit layerViewChanged();
}

void EditorViewport::addMapLayer(core::LayerKind kind, const std::string& name) {
    const std::optional<std::size_t> index = _draft.addLayer(kind, name);
    if (!index) {
        return;
    }
    markDraftMutated();
    setActiveLayer(*index);
}

void EditorViewport::removeMapLayer(std::size_t index) {
    if (_draft.removeLayer(index)) {
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
    markDraftMutated();
    if (followActive) {
        setActiveLayer(*moved);
    }
}

void EditorViewport::renameMapLayer(std::size_t index, const std::string& name) {
    if (!name.empty() && _draft.renameLayer(index, name)) {
        markDraftMutated();
    }
}

void EditorViewport::setEditorReferences(const EditorReferences* references) {
    _references = references;
    refreshDiagnostics();
    viewport()->update();
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
    viewport()->update();
    emit entitySelectionChanged(_selectedEntity);
}

void EditorViewport::setEntityProperty(std::size_t index, const std::string& key,
                                       core::PropertyValue value) {
    if (_draft.setEntityProperty(index, key, std::move(value))) {
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
        markDraftMutated();
        emit statusMessage(QStringLiteral("Entity moved to (%1, %2).")
                               .arg(decision.cell.column)
                               .arg(decision.cell.row));
    }
}

}  // namespace hmi
