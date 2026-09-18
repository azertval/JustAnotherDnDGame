// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Editor/Ui/MainWindow.h"

#include <QAction>
#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QFontMetrics>
#include <QFormLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QRect>
#include <QScreen>
#include <QSettings>
#include <QSpinBox>
#include <QStatusBar>
#include <QString>
#include <QTableWidget>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include <algorithm>
#include <filesystem>

#include "Editor/Logic/EditorStatus.h"
#include "Editor/Logic/EntityReferences.h"
#include "Editor/Ui/EditorActions.h"
#include "Editor/Ui/EditorViewport.h"
#include "Editor/Ui/EntityPanel.h"
#include "Editor/Ui/LayersPanel.h"
#include "Editor/Ui/LevelBrowserPanel.h"
#include "Editor/Ui/PalettePanel.h"
#include "HMI/Platform/ExecutableDirectory.h"

namespace hmi {

namespace {

// Version de la disposition sérialisée : à incrémenter si l'ensemble des docks change, pour
// invalider proprement une disposition sauvegardée devenue incompatible (`restoreState`).
constexpr int LAYOUT_VERSION = 11;  // 11 : un seul espace, quatre panneaux (LOT-88)

// Clés de persistance (portée application ; l'organisation/appli sont fixées dans `main`).
constexpr const char* GEOMETRY_KEY = "mainWindow/geometry";
constexpr const char* STATE_KEY = "mainWindow/state";
// Réglage de mise en avant automatique des panneaux.
constexpr const char* FOLLOW_ACTIVE_TOOL_KEY = "panels/followActiveTool";

// Taille maximale d'une carte dans la boîte « Resize ».
constexpr int MAXIMUM_MAP_SIDE = 100;

}  // namespace

MainWindow::MainWindow() : _viewport(new EditorViewport()) {
    setWindowTitle(QStringLiteral("Just Another RPG Game — Editor"));
    setDockNestingEnabled(true);
    _editContext = _viewport;

    _viewport->setMinimumSize(320, 240);
    _viewport->setFocusPolicy(Qt::StrongFocus);
    setCentralWidget(_viewport);

    buildUi();

    // Sélectionner une tuile dans la palette définit le type peint au clic dans le canevas.
    connect(_palette, &PalettePanel::tileSelected, _viewport,
            [this](core::TileType type) { _viewport->setActiveTile(type); });
    // Le canevas change d'outil de lui-même (une famille d'entité choisie arme l'outil Entité) :
    // la barre d'outils suit, sans reboucler (setActiveTool n'émet rien).
    connect(_viewport, &EditorViewport::toolChanged, _actions, &EditorActions::setActiveTool);
    // Les messages d'état du canevas (enregistrement, essai, erreurs) s'affichent en bas, puis
    // laissent la main à l'aide contextuelle.
    connect(_viewport, &EditorViewport::statusMessage, this,
            [this](const QString& message) { showTransientStatusMessage(message, 5000); });
    connect(_viewport, &EditorViewport::toolChanged, this,
            [this](hmi::EditorTool) { refreshStatusHelp(); });
    connect(_viewport, &EditorViewport::toolChanged, this, &MainWindow::applyPanelFocus);
    connect(_viewport, &EditorViewport::hoveredCellChanged, this,
            [this](std::optional<core::GridPosition>) { refreshStatusHelp(); });
    connect(_viewport, &EditorViewport::zoomChanged, this, [this](float) { refreshStatusHelp(); });
    connect(_viewport, &EditorViewport::draftChanged, this, [this] { refreshStatusHelp(); });
    // Ouvrir une carte depuis le panneau : garde-fou des modifications non enregistrées d'abord.
    connect(_levels, &LevelBrowserPanel::levelOpenRequested, this, [this](const QString& path) {
        if (_viewport->isDirty()) {
            const QMessageBox::StandardButton answer = QMessageBox::question(
                this, QStringLiteral("Unsaved changes"),
                QStringLiteral("The current map has unsaved changes. Discard them and open the "
                               "other map?"));
            if (answer != QMessageBox::Yes) {
                return;
            }
        }
        _viewport->openLevel(std::filesystem::path(path.toStdString()));
    });

    connectMapPanels();
    reloadEditorReferences();

    resize(1280, 720);
    refreshStatusHelp();

    // Capture la disposition par défaut (après création des docks, avant restauration d'une
    // éventuelle disposition sauvegardée) : sert de cible à « Reset layout ».
    _defaultState = saveState(LAYOUT_VERSION);
    restoreLayout();
}

MainWindow::~MainWindow() = default;

QDockWidget* MainWindow::addPanel(const QString& objectName, const QString& title, QWidget* content,
                                  Qt::DockWidgetArea area) {
    auto* const dock = new QDockWidget(title, this);
    // objectName stable : c'est la clé de la disposition persistée (`saveState`).
    dock->setObjectName(objectName);
    dock->setWidget(content);
    addDockWidget(area, dock);
    return dock;
}

void MainWindow::buildUi() {
    // Outils et commandes : une action unique par commande, partagée entre la barre d'outils, le
    // menu et son raccourci.
    _actions = new EditorActions(this);
    _actions->applyShortcuts(_viewport->editorBindings());
    _toolBar = addToolBar(QStringLiteral("Tools"));
    _toolBar->setObjectName(QStringLiteral("EditorToolBar"));
    _toolBar->setMovable(false);
    _toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    _actions->populateToolBar(*_toolBar);

    _palette = new PalettePanel;
    _levels = new LevelBrowserPanel(hmi::executableDirectory() / "Levels");
    _layers = new LayersPanel;
    _entities = new EntityPanel;
    _docks = {
        addPanel(QStringLiteral("PalettePanel"), QStringLiteral("Palette"), _palette,
                 Qt::LeftDockWidgetArea),
        addPanel(QStringLiteral("LevelsPanel"), QStringLiteral("Maps"), _levels,
                 Qt::RightDockWidgetArea),
        addPanel(QStringLiteral("LayersPanel"), QStringLiteral("Layers"), _layers,
                 Qt::LeftDockWidgetArea),
        addPanel(QStringLiteral("EntitiesPanel"), QStringLiteral("Entities"), _entities,
                 Qt::RightDockWidgetArea),
    };

    // Cartes et Entités partagent une pile d'onglets par défaut ; chacun reste déplaçable,
    // détachable et refermable. Doit précéder la capture de _defaultState.
    QDockWidget* const levelsDock = dockFor(PanelId::Levels);
    QDockWidget* const entitiesDock = dockFor(PanelId::Entities);
    tabifyDockWidget(levelsDock, entitiesDock);
    // Un changement de visibilité non provoqué par notre propre code ne peut venir que d'un choix
    // explicite de l'utilisateur : cliquer un onglet, fermer ou détacher le panneau.
    for (QDockWidget* const dock : {levelsDock, entitiesDock}) {
        connect(dock, &QDockWidget::visibilityChanged, this, [this](bool) {
            if (!_suppressPanelFocusTracking) {
                _userPickedTab = true;
            }
        });
        connect(dock, &QDockWidget::topLevelChanged, this, [this](bool) { _userPickedTab = true; });
    }

    buildMenus();
    connectToolActions();
    connectEditorCommands();
    buildStatusBar();
}

void MainWindow::buildMenus() {
    // Menus PAR NATURE D'ACTION ; les commandes sont les mêmes QAction que la barre d'outils.
    QMenu* const fileMenu = menuBar()->addMenu(QStringLiteral("&File"));
    fileMenu->addAction(_actions->action(EditorCommand::Save));
    fileMenu->addAction(_actions->action(EditorCommand::Rename));
    _resizeAction = fileMenu->addAction(QStringLiteral("Resize…"));
    fileMenu->addSeparator();
    QAction* const quit = fileMenu->addAction(QStringLiteral("Quit"));
    connect(quit, &QAction::triggered, this, &MainWindow::close);

    QMenu* const editMenu = menuBar()->addMenu(QStringLiteral("&Edit"));
    editMenu->addAction(_actions->action(EditorCommand::Undo));
    editMenu->addAction(_actions->action(EditorCommand::Redo));
    editMenu->addSeparator();
    editMenu->addAction(_actions->action(EditorCommand::Copy));
    editMenu->addAction(_actions->action(EditorCommand::Paste));

    QMenu* const mapMenu = menuBar()->addMenu(QStringLiteral("&Map"));
    mapMenu->addAction(_actions->action(EditorCommand::Playtest));

    QMenu* const viewMenu = menuBar()->addMenu(QStringLiteral("&View"));
    viewMenu->addAction(_actions->action(EditorCommand::ResetCamera));
    viewMenu->addAction(_actions->action(EditorCommand::ToggleGrid));
    viewMenu->addSeparator();
    QMenu* const panelsMenu = viewMenu->addMenu(QStringLiteral("Panels"));
    for (QDockWidget* const dock : _docks) {
        panelsMenu->addAction(dock->toggleViewAction());
    }
    panelsMenu->addSeparator();
    // Mise en avant automatique du panneau de l'outil actif : persistée, active par défaut.
    _actFollowActiveTool = panelsMenu->addAction(QStringLiteral("Follow active tool"));
    _actFollowActiveTool->setCheckable(true);
    _actFollowActiveTool->setChecked(
        QSettings().value(QString::fromLatin1(FOLLOW_ACTIVE_TOOL_KEY), true).toBool());
    connect(_actFollowActiveTool, &QAction::toggled, this, [](bool enabled) {
        QSettings().setValue(QString::fromLatin1(FOLLOW_ACTIVE_TOOL_KEY), enabled);
    });
    _resetLayoutAction = panelsMenu->addAction(QStringLiteral("Reset layout"));

    QMenu* const helpMenu = menuBar()->addMenu(QStringLiteral("&Help"));
    helpMenu->addAction(_actions->action(EditorCommand::ShortcutsOverview));
}

void MainWindow::connectMapPanels() {
    const auto refreshLayers = [this] {
        _layers->refresh(_viewport->draft(), _viewport->activeLayer(), _viewport->layerView());
    };
    const auto refreshEntities = [this] {
        _entities->refresh(_viewport->draft(), _viewport->selectedEntity(),
                           _viewport->entityReferenceContext(), _viewport->diagnostics());
    };
    connect(_viewport, &EditorViewport::draftChanged, this, [refreshLayers, refreshEntities] {
        refreshLayers();
        refreshEntities();
    });
    connect(_viewport, &EditorViewport::activeLayerChanged, this,
            [refreshLayers](hmi::LayerSlot) { refreshLayers(); });
    connect(_viewport, &EditorViewport::layerViewChanged, this, refreshLayers);
    connect(_viewport, &EditorViewport::entitySelectionChanged, this,
            [refreshEntities](std::optional<std::size_t>) { refreshEntities(); });

    // Couches : le panneau demande, le canevas applique -- l'historique pour la structure, une
    // simple aide d'édition pour la visibilité et l'opacité.
    connect(_layers, &LayersPanel::activeLayerRequested, _viewport,
            &EditorViewport::setActiveLayer);
    connect(_layers, &LayersPanel::visibilityRequested, _viewport,
            &EditorViewport::setMapLayerVisible);
    connect(_layers, &LayersPanel::opacityRequested, _viewport,
            &EditorViewport::setMapLayerOpacity);
    connect(_layers, &LayersPanel::addRequested, this, [this](core::LayerKind kind) {
        _viewport->addMapLayer(kind, kind == core::LayerKind::Decor ? "decor" : "ground");
    });
    connect(_layers, &LayersPanel::removeRequested, this, [this](std::size_t index) {
        if (index >= _viewport->draft().layers().size()) {
            return;
        }
        const QString name = QString::fromStdString(_viewport->draft().layers()[index].name);
        if (QMessageBox::question(
                this, QStringLiteral("Remove layer"),
                QStringLiteral("Remove layer \"%1\"? Ctrl+Z undoes it.").arg(name)) !=
            QMessageBox::Yes) {
            return;
        }
        _viewport->removeMapLayer(index);
    });
    connect(_layers, &LayersPanel::moveRequested, _viewport, &EditorViewport::moveMapLayer);
    connect(_layers, &LayersPanel::renameRequested, this,
            [this](std::size_t index, const QString& name) {
                _viewport->renameMapLayer(index, name.toStdString());
            });

    // Entités : choisir une famille à poser arme l'outil Entité.
    connect(_entities, &EntityPanel::kindToPlaceChanged, this, [this](const QString& type) {
        _viewport->setEntityKindToPlace(type.toStdString());
        if (!type.isEmpty()) {
            _viewport->setTool(hmi::EditorTool::Entity);
        }
    });
    connect(_entities, &EntityPanel::entitySelected, _viewport, &EditorViewport::selectEntity);
    connect(_entities, &EntityPanel::propertyChanged, this,
            [this](std::size_t index, const QString& key, const core::PropertyValue& value) {
                _viewport->setEntityProperty(index, key.toStdString(), value);
            });
    connect(_entities, &EntityPanel::removeRequested, _viewport, &EditorViewport::removeEntity);

    refreshLayers();  // état initial (avant tout draftChanged).
    refreshEntities();
}

void MainWindow::reloadEditorReferences() {
    _references =
        std::make_unique<EditorReferences>(hmi::loadEditorReferences(hmi::executableDirectory()));
    _viewport->setEditorReferences(_references.get());
}

void MainWindow::connectToolActions() {
    for (std::size_t index = 0; index < EDITOR_COMMAND_COUNT; ++index) {
        const auto command = static_cast<EditorCommand>(index);
        const std::optional<hmi::EditorTool> tool = EditorActions::toolOf(command);
        if (!tool) {
            continue;
        }
        connect(_actions->action(command), &QAction::toggled, _viewport,
                [this, tool = *tool](bool on) {
                    if (on) {
                        _viewport->setTool(tool);
                    }
                });
    }
}

void MainWindow::connectEditorCommands() {
    connect(_actions->action(EditorCommand::Save), &QAction::triggered, this, [this] {
        _viewport->save();
        // Une carte enregistrée peut avoir changé ses points d'arrivée ou son nom : les portails
        // des AUTRES cartes se valident contre le fichier, et le graphe du monde le montre.
        reloadEditorReferences();
        _levels->refreshWorldGraph();
    });
    connect(_actions->action(EditorCommand::Playtest), &QAction::triggered, _viewport,
            [this] { _viewport->startPlaytest(); });
    connect(_actions->action(EditorCommand::Undo), &QAction::triggered, this,
            [this] { _editContext->undo(); });
    connect(_actions->action(EditorCommand::Redo), &QAction::triggered, this,
            [this] { _editContext->redo(); });
    connect(_actions->action(EditorCommand::Copy), &QAction::triggered, this,
            [this] { _editContext->copy(); });
    connect(_actions->action(EditorCommand::Paste), &QAction::triggered, this,
            [this] { _editContext->paste(); });
    connect(_actions->action(EditorCommand::ToggleGrid), &QAction::triggered, _viewport,
            [this] { _viewport->toggleGrid(); });
    connect(_actions->action(EditorCommand::ResetCamera), &QAction::triggered, _viewport,
            [this] { _viewport->resetCamera(); });
    // Renommer la carte ouverte : même dialogue que LevelBrowserPanel::onRename.
    connect(_actions->action(EditorCommand::Rename), &QAction::triggered, this, [this] {
        bool accepted = false;
        const QString name = QInputDialog::getText(
            this, QStringLiteral("Rename"), QStringLiteral("New name:"), QLineEdit::Normal,
            QString::fromStdString(_viewport->draft().name()), &accepted);
        if (!accepted || name.isEmpty()) {
            return;
        }
        if (_viewport->renameOpenLevel(name.toStdString())) {
            _levels->refresh();  // le fichier a pu changer de nom dans le dossier listé.
        }
    });
    connect(_actions->action(EditorCommand::ShortcutsOverview), &QAction::triggered, this,
            [this] { openShortcutsDialog(); });
    connect(_resizeAction, &QAction::triggered, this, [this] { openResizeDialog(); });
    connect(_resetLayoutAction, &QAction::triggered, this, [this] {
        _suppressPanelFocusTracking = true;
        restoreState(_defaultState, LAYOUT_VERSION);
        _suppressPanelFocusTracking = false;
        _userPickedTab = false;  // disposition remise à neuf : la mise en avant repart.
    });
}

void MainWindow::buildStatusBar() {
    // Barre d'état structurée : zones permanentes, jamais recouvertes par un message transitoire.
    // Largeur minimale sur les zones qui changent au survol (case, zoom) : sans elle, la barre
    // « saute » à chaque déplacement de souris.
    for (QLabel*& zone : _statusZones) {
        zone = new QLabel(this);
        statusBar()->addPermanentWidget(zone);
    }
    _statusZones[3]->setMinimumWidth(fontMetrics().horizontalAdvance(QStringLiteral("(999, 999)")));
    _statusZones[4]->setMinimumWidth(fontMetrics().horizontalAdvance(QStringLiteral("Zoom: 999%")));
    _statusMessageTimer = new QTimer(this);
    _statusMessageTimer->setSingleShot(true);
    connect(_statusMessageTimer, &QTimer::timeout, this, &MainWindow::refreshStatusHelp);
}

void MainWindow::openResizeDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("Resize map"));
    auto* const widthSpin = new QSpinBox(&dialog);
    auto* const heightSpin = new QSpinBox(&dialog);
    for (QSpinBox* const spin : {widthSpin, heightSpin}) {
        spin->setRange(1, MAXIMUM_MAP_SIDE);
    }
    widthSpin->setValue(_viewport->levelWidth());
    heightSpin->setValue(_viewport->levelHeight());
    auto* const buttons =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    auto* const form = new QFormLayout(&dialog);
    form->addRow(QStringLiteral("Width (cells)"), widthSpin);
    form->addRow(QStringLiteral("Height (cells)"), heightSpin);
    form->addRow(buttons);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    const int width = widthSpin->value();
    const int height = heightSpin->value();
    // Confirmation si le redimensionnement supprimerait du contenu déjà posé (EX-EDIT-012).
    if (_viewport->wouldResizeDrop(width, height)) {
        const QMessageBox::StandardButton answer = QMessageBox::question(
            this, QStringLiteral("Resize map"),
            QStringLiteral(
                "Shrinking to %1 × %2 will remove content (entry or entities). Continue?")
                .arg(width)
                .arg(height));
        if (answer != QMessageBox::Yes) {
            return;
        }
    }
    _viewport->resizeLevel(width, height);
}

void MainWindow::openShortcutsDialog() {
    // Lit les raccourcis EFFECTIFS des actions à l'ouverture, jamais un texte figé (EX-EDIT-015).
    // Les commandes SANS raccourci sont omises -- une ligne vide n'apprendrait rien.
    QDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("Keyboard shortcuts"));
    auto* const table = new QTableWidget(0, 2, &dialog);
    table->setHorizontalHeaderLabels({QStringLiteral("Command"), QStringLiteral("Shortcut")});
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(false);
    table->setAlternatingRowColors(true);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setShowGrid(false);
    for (QAction* const act : _actions->all()) {
        if (act->shortcut().isEmpty()) {
            continue;
        }
        const int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(act->text()));
        table->setItem(row, 1,
                       new QTableWidgetItem(act->shortcut().toString(QKeySequence::NativeText)));
    }
    table->resizeColumnsToContents();
    auto* const buttons = new QDialogButtonBox(QDialogButtonBox::Ok, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    auto* const layout = new QVBoxLayout(&dialog);
    layout->addWidget(table);
    layout->addWidget(buttons);
    dialog.exec();
}

void MainWindow::restoreLayout() {
    const QSettings settings;
    const QByteArray geometry = settings.value(QString::fromLatin1(GEOMETRY_KEY)).toByteArray();
    const QByteArray state = settings.value(QString::fromLatin1(STATE_KEY)).toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
        // La géométrie persistée n'a jamais été bornée à l'écran (EX-IHM-081) : on la ramène dans
        // la zone utile -- taille PUIS position, dans cet ordre : déplacer une fenêtre trop grande
        // ne la ferait pas tenir.
        const QScreen* const hostScreen = screen();
        if (hostScreen != nullptr) {
            const QRect available = hostScreen->availableGeometry();
            const QSize decorations = frameGeometry().size() - size();
            const QSize fittedFrame = frameGeometry().size().boundedTo(available.size());
            if (fittedFrame != frameGeometry().size()) {
                resize(fittedFrame - decorations);
            }
            QRect placed = frameGeometry();
            placed.moveLeft(
                std::clamp(placed.left(), available.left(),
                           std::max(available.left(), available.right() - placed.width() + 1)));
            placed.moveTop(
                std::clamp(placed.top(), available.top(),
                           std::max(available.top(), available.bottom() - placed.height() + 1)));
            if (placed.topLeft() != frameGeometry().topLeft()) {
                move(placed.topLeft());
            }
        }
    }
    if (!state.isEmpty()) {
        _suppressPanelFocusTracking = true;
        restoreState(state, LAYOUT_VERSION);
        _suppressPanelFocusTracking = false;
    }
}

void MainWindow::saveLayout() {
    QSettings settings;
    settings.setValue(QString::fromLatin1(GEOMETRY_KEY), saveGeometry());
    settings.setValue(QString::fromLatin1(STATE_KEY), saveState(LAYOUT_VERSION));
}

void MainWindow::closeEvent(QCloseEvent* event) {
    saveLayout();
    QMainWindow::closeEvent(event);
}

void MainWindow::refreshStatusHelp() {
    EditorStatusContext context;
    // L'essai n'édite rien : la barre d'état ne décrit alors aucun outil.
    if (!_viewport->playtesting()) {
        LevelStatusInfo level;
        level.name = _viewport->draft().name();
        level.dirty = _viewport->isDirty();
        level.tool = _viewport->activeTool();
        level.hoveredCell = _viewport->hoveredCell();
        level.zoom = _viewport->zoom();
        context.level = level;
    }
    const EditorStatusLines lines = editorStatusLines(context);
    for (std::size_t index = 0; index < _statusZones.size(); ++index) {
        _statusZones[index]->setText(QString::fromStdString(lines.permanent[index]));
    }
    // Un message transitoire en cours garde la barre jusqu'à son expiration.
    if (!_statusMessageTimer->isActive()) {
        statusBar()->showMessage(QString::fromStdString(lines.help));
    }
}

void MainWindow::showTransientStatusMessage(const QString& message, int timeoutMs) {
    // Timeout laissé à 0 (défaut de showMessage) : le message reste affiché jusqu'à la
    // restauration explicite par _statusMessageTimer, seul maître de sa durée de vie.
    statusBar()->showMessage(message);
    _statusMessageTimer->start(timeoutMs);
}

QDockWidget* MainWindow::dockFor(PanelId panel) const {
    return _docks[static_cast<std::size_t>(panel)];
}

void MainWindow::applyPanelFocus(hmi::EditorTool tool) {
    if (!_actFollowActiveTool->isChecked() || _userPickedTab) {
        return;  // réglage désactivé, ou l'utilisateur a déjà imposé un onglet pour la session.
    }
    const std::optional<hmi::PanelId> panel = hmi::panelForTool(tool);
    if (!panel) {
        return;
    }
    // raise() met l'onglet au premier plan sans voler le focus clavier au canevas -- une
    // suggestion, jamais une confiscation.
    _suppressPanelFocusTracking = true;
    dockFor(*panel)->raise();
    _suppressPanelFocusTracking = false;
}

}  // namespace hmi
