// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Interface/MainWindow.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QFontMetrics>
#include <QGuiApplication>
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
#include <QStyleHints>
#include <QTableWidget>
#include <QTimer>
#include <QToolBar>
#include <algorithm>
#include <filesystem>

#include "HMI/Editor/EditorStatus.h"
#include "HMI/Editor/EditorViewport.h"
#include "HMI/Editor/EntityPanel.h"
#include "HMI/Editor/EntityReferences.h"
#include "HMI/Editor/LayersPanel.h"
#include "HMI/Editor/LevelBrowserPanel.h"
#include "HMI/Editor/PalettePanel.h"
#include "HMI/HmiLog.h"
#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/EditorActions.h"
#include "HMI/Platform/ExecutableDirectory.h"
#include "ui_MainWindow.h"
#include "ui_ResizeDialog.h"
#include "ui_ShortcutsDialog.h"

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

}  // namespace

MainWindow::MainWindow()
    : _ui(std::make_unique<Ui::EditorMainWindow>()),
      _viewport(new EditorViewport()),
      _loc(hmi::executableDirectory() / "Localization") {
    _ui->setupUi(this);

    // Catalogue de traduction : français par défaut (repli), langue active depuis les réglages.
    static_cast<void>(_loc.loadDefaultLanguage("fr"));
    const QString savedLanguage =
        QSettings().value(QStringLiteral("language"), QStringLiteral("fr")).toString();
    if (savedLanguage != QLatin1String("fr")) {
        static_cast<void>(_loc.loadLanguage(savedLanguage.toStdString()));
    }
    _viewport->setLocalization(&_loc);
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
                this, text("dialog.unsaved_title"), text("dialog.unsaved_text"));
            if (answer != QMessageBox::Yes) {
                return;
            }
        }
        _viewport->openLevel(std::filesystem::path(path.toStdString()));
    });

    connectMapPanels();
    reloadEditorReferences();

    resize(1280, 720);
    retranslateUi();

    // Capture la disposition par défaut (après création des docks, avant restauration d'une
    // éventuelle disposition sauvegardée) : sert de cible à « Réinitialiser la disposition ».
    _defaultState = saveState(LAYOUT_VERSION);
    restoreLayout();
}

MainWindow::~MainWindow() = default;

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
        const char* const nameKey = kind == core::LayerKind::Decor ? "layers.default_name.decor"
                                                                   : "layers.default_name.ground";
        _viewport->addMapLayer(kind, text(nameKey).toStdString());
    });
    connect(_layers, &LayersPanel::removeRequested, this, [this](std::size_t index) {
        if (index >= _viewport->draft().layers().size()) {
            return;
        }
        const QString name = QString::fromStdString(_viewport->draft().layers()[index].name);
        if (QMessageBox::question(this, text("layers.remove_confirm_title"),
                                  text("layers.remove_confirm").arg(name)) != QMessageBox::Yes) {
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

void MainWindow::buildUi() {
    // Outils et commandes principales : une action unique par commande, partagée
    // entre la barre d'outils, le menu et son raccourci.
    _actions = new EditorActions(hmi::currentEditorTokens(), this);
    _actions->applyShortcuts(_viewport->editorBindings(), _loc);
    _toolBar = _ui->EditorToolBar;
    _actions->populateToolBar(*_toolBar);

    _palette = new PalettePanel(_ui->PalettePanel);
    _ui->PalettePanel->setWidget(_palette);
    _levels = new LevelBrowserPanel(hmi::executableDirectory() / "Levels", _ui->LevelsPanel);
    _ui->LevelsPanel->setWidget(_levels);
    _layers = new LayersPanel(_ui->LayersPanel);
    _ui->LayersPanel->setWidget(_layers);
    _entities = new EntityPanel(_ui->EntitiesPanel);
    _ui->EntitiesPanel->setWidget(_entities);

    // Cartes et Entités partagent une pile d'onglets par défaut ; chacun reste
    // déplaçable, détachable et refermable (EX-IHM-010). Doit précéder la capture de
    // _defaultState.
    tabifyDockWidget(_ui->LevelsPanel, _ui->EntitiesPanel);
    // Un changement de visibilité non provoqué par notre propre code ne peut venir que d'un choix
    // explicite de l'utilisateur : cliquer un onglet, fermer ou détacher le panneau.
    for (QDockWidget* const dock : {_ui->LevelsPanel, _ui->EntitiesPanel}) {
        connect(dock, &QDockWidget::visibilityChanged, this, [this](bool) {
            if (!_suppressPanelFocusTracking) {
                _userPickedTab = true;
            }
        });
        connect(dock, &QDockWidget::topLevelChanged, this, [this](bool) { _userPickedTab = true; });
    }

    connectToolActions();
    connectEditorCommands();
    buildThemeMenu();
    buildViewMenu();
    buildStatusBar();
}

void MainWindow::connectToolActions() {
    // Outils : la liste est DÉRIVÉE du catalogue, jamais recopiée ici -- un outil ajouté au
    // catalogue est relié au canevas par construction.
    for (const hmi::EditorActionSpec& spec : hmi::editorActionCatalog()) {
        if (spec.group != hmi::EditorActionGroup::LevelTools) {
            continue;
        }
        const std::optional<hmi::EditorTool> tool = hmi::editorActionTool(spec.id);
        if (!tool) {
            continue;
        }
        connect(_actions->action(spec.id), &QAction::toggled, _viewport,
                [this, tool = *tool](bool on) {
                    if (on) {
                        _viewport->setTool(tool);
                    }
                });
    }
}

void MainWindow::connectEditorCommands() {
    connect(_actions->action(hmi::IconId::Save), &QAction::triggered, this, [this] {
        _viewport->save();
        // Une carte enregistrée peut avoir changé ses points d'arrivée ou son nom : les portails
        // des AUTRES cartes se valident contre le fichier, et le graphe du monde le montre.
        reloadEditorReferences();
        _levels->refreshWorldGraph();
    });
    connect(_actions->action(hmi::IconId::Playtest), &QAction::triggered, _viewport,
            [this] { _viewport->startPlaytest(); });
    connect(_actions->action(hmi::IconId::Undo), &QAction::triggered, this,
            [this] { _editContext->undo(); });
    connect(_actions->action(hmi::IconId::Redo), &QAction::triggered, this,
            [this] { _editContext->redo(); });
    connect(_actions->action(hmi::IconId::Copy), &QAction::triggered, this,
            [this] { _editContext->copy(); });
    connect(_actions->action(hmi::IconId::Paste), &QAction::triggered, this,
            [this] { _editContext->paste(); });
    connect(_actions->action(hmi::IconId::ToggleGrid), &QAction::triggered, _viewport,
            [this] { _viewport->toggleGrid(); });
    connect(_actions->action(hmi::IconId::ResetCamera), &QAction::triggered, _viewport,
            [this] { _viewport->resetCamera(); });
    // Renommer la carte ouverte : même dialogue que LevelBrowserPanel::onRename.
    connect(_actions->action(hmi::IconId::Rename), &QAction::triggered, this, [this] {
        bool accepted = false;
        const QString name = QInputDialog::getText(
            this, text("map.rename"), text("map.rename_prompt"), QLineEdit::Normal,
            QString::fromStdString(_viewport->draft().name()), &accepted);
        if (!accepted || name.isEmpty()) {
            return;
        }
        if (_viewport->renameOpenLevel(name.toStdString())) {
            _levels->refresh();  // le fichier a pu changer de nom dans le dossier listé.
        }
    });
    connect(_actions->action(hmi::IconId::ShortcutsOverview), &QAction::triggered, this,
            [this] { openShortcutsDialog(); });

    // Commandes principales, réparties PAR NATURE D'ACTION (EX-IHM-074). Toujours les
    // mêmes actions que la barre d'outils : aucune seconde définition (EX-IHM-055).
    _ui->fileMenu->insertAction(_ui->actResize, _actions->action(hmi::IconId::Save));
    _ui->fileMenu->insertAction(_ui->actResize, _actions->action(hmi::IconId::Rename));
    _ui->editMenu->addAction(_actions->action(hmi::IconId::Undo));
    _ui->editMenu->addAction(_actions->action(hmi::IconId::Redo));
    _ui->editMenu->addSeparator();
    _ui->editMenu->addAction(_actions->action(hmi::IconId::Copy));
    _ui->editMenu->addAction(_actions->action(hmi::IconId::Paste));
    _ui->levelMenu->addAction(_actions->action(hmi::IconId::Playtest));
    _ui->helpMenu->addAction(_actions->action(hmi::IconId::ShortcutsOverview));

    connect(_ui->actQuit, &QAction::triggered, this, &MainWindow::close);
    connect(_ui->actResize, &QAction::triggered, this, [this] { openResizeDialog(); });
    connect(_ui->actResetLayout, &QAction::triggered, this, [this] {
        _suppressPanelFocusTracking = true;
        restoreState(_defaultState, LAYOUT_VERSION);
        _suppressPanelFocusTracking = false;
        _userPickedTab = false;  // disposition remise à neuf : la mise en avant repart.
    });
}

void MainWindow::buildThemeMenu() {
    // Thème clair/sombre de l'éditeur : réglage Système/Clair/Sombre, persisté.
    auto* const themeGroup = new QActionGroup(this);
    themeGroup->setExclusive(true);
    for (QAction* const act : {_ui->actThemeSystem, _ui->actThemeLight, _ui->actThemeDark}) {
        act->setActionGroup(themeGroup);
    }
    switch (hmi::editorThemeSetting()) {
        case hmi::EditorThemeSetting::Light:
            _ui->actThemeLight->setChecked(true);
            break;
        case hmi::EditorThemeSetting::Dark:
            _ui->actThemeDark->setChecked(true);
            break;
        case hmi::EditorThemeSetting::System:
            _ui->actThemeSystem->setChecked(true);
            break;
    }
    // Régénère palette + feuille de style + icônes depuis le thème désormais effectif.
    const auto applyThemeSetting = [this](hmi::EditorThemeSetting setting) {
        hmi::setEditorThemeSetting(setting);
        hmi::reapplyEditorTheme();
        _actions->refreshIcons(hmi::currentEditorTokens());
    };
    connect(_ui->actThemeSystem, &QAction::triggered, this,
            [applyThemeSetting] { applyThemeSetting(hmi::EditorThemeSetting::System); });
    connect(_ui->actThemeLight, &QAction::triggered, this,
            [applyThemeSetting] { applyThemeSetting(hmi::EditorThemeSetting::Light); });
    connect(_ui->actThemeDark, &QAction::triggered, this,
            [applyThemeSetting] { applyThemeSetting(hmi::EditorThemeSetting::Dark); });
    // Réglage « Système » : réagit à un changement live du thème du système d'exploitation.
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this,
            [this](Qt::ColorScheme) {
                if (hmi::editorThemeSetting() == hmi::EditorThemeSetting::System) {
                    hmi::reapplyEditorTheme();
                    _actions->refreshIcons(hmi::currentEditorTokens());
                }
            });
}

void MainWindow::buildViewMenu() {
    // Commandes de VUE, en tête du menu Affichage : ce sont les seules qui agissent tout de suite ;
    // le reste du menu est un réglage, rangé en sous-menu.
    QAction* const firstViewSeparator = _ui->viewMenu->actions().constFirst();
    _ui->viewMenu->insertAction(firstViewSeparator, _actions->action(hmi::IconId::ResetCamera));
    _ui->viewMenu->insertAction(firstViewSeparator, _actions->action(hmi::IconId::ToggleGrid));

    for (QDockWidget* const dock :
         {_ui->PalettePanel, _ui->LevelsPanel, _ui->LayersPanel, _ui->EntitiesPanel}) {
        _ui->panelsMenu->insertAction(_ui->panelsMenu->actions().constFirst(),
                                      dock->toggleViewAction());
    }

    // Mise en avant automatique du panneau de l'outil actif : persistée, active
    // par défaut.
    _actFollowActiveTool = _ui->actFollowActiveTool;
    _actFollowActiveTool->setChecked(
        QSettings().value(QString::fromLatin1(FOLLOW_ACTIVE_TOOL_KEY), true).toBool());
    connect(_actFollowActiveTool, &QAction::toggled, this, [](bool enabled) {
        QSettings().setValue(QString::fromLatin1(FOLLOW_ACTIVE_TOOL_KEY), enabled);
    });
}

void MainWindow::buildStatusBar() {
    // Barre d'état structurée : zones permanentes, jamais recouvertes par un
    // message transitoire. Largeur minimale sur les zones qui changent au survol (case, zoom) :
    // sans elle, la barre « saute » à chaque déplacement de souris.
    for (QLabel*& zone : _statusZones) {
        zone = new QLabel(this);
        statusBar()->addPermanentWidget(zone);
    }
    _statusZones[3]->setMinimumWidth(fontMetrics().horizontalAdvance(QStringLiteral("(999, 999)")));
    _statusZones[4]->setMinimumWidth(
        fontMetrics().horizontalAdvance(QStringLiteral("Zoom : 999%")));
    _statusMessageTimer = new QTimer(this);
    _statusMessageTimer->setSingleShot(true);
    connect(_statusMessageTimer, &QTimer::timeout, this, &MainWindow::refreshStatusHelp);
}

void MainWindow::openResizeDialog() {
    QDialog dialog(this);
    Ui::ResizeDialog ui;
    ui.setupUi(&dialog);
    dialog.setWindowTitle(text("dialog.resize_title"));
    ui.widthLabel->setText(text("dialog.width"));
    ui.heightLabel->setText(text("dialog.height"));
    ui.widthSpin->setValue(_viewport->levelWidth());
    ui.heightSpin->setValue(_viewport->levelHeight());
    connect(ui.buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(ui.buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    const int width = ui.widthSpin->value();
    const int height = ui.heightSpin->value();
    // Confirmation si le redimensionnement supprimerait du contenu déjà posé (EX-EDIT-012).
    if (_viewport->wouldResizeDrop(width, height)) {
        const QMessageBox::StandardButton answer = QMessageBox::question(
            this, text("dialog.resize_title"), text("dialog.resize_drop").arg(width).arg(height));
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
    Ui::ShortcutsDialog ui;
    ui.setupUi(&dialog);
    dialog.setWindowTitle(text("dialog.shortcuts_title"));
    ui.table->setHorizontalHeaderLabels(
        {text("dialog.shortcuts_command"), text("dialog.shortcuts_key")});
    ui.table->horizontalHeader()->setStretchLastSection(true);
    ui.table->verticalHeader()->setVisible(false);

    for (const hmi::EditorActionSpec& spec : hmi::editorActionCatalog()) {
        QAction* const act = _actions->action(spec.id);
        if (act->shortcut().isEmpty()) {
            continue;
        }
        const int row = ui.table->rowCount();
        ui.table->insertRow(row);
        ui.table->setItem(row, 0, new QTableWidgetItem(act->text()));
        ui.table->setItem(row, 1,
                          new QTableWidgetItem(act->shortcut().toString(QKeySequence::NativeText)));
    }
    ui.table->resizeColumnsToContents();
    connect(ui.buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
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

QString MainWindow::text(const char* key) const {
    return QString::fromStdString(_loc.text(key));
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
    const EditorStatusLines lines = editorStatusLines(context, _loc);
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

void MainWindow::applyPanelFocus(hmi::EditorTool tool) {
    if (!_actFollowActiveTool->isChecked() || _userPickedTab) {
        return;  // réglage désactivé, ou l'utilisateur a déjà imposé un onglet pour la session.
    }
    const std::optional<hmi::PanelId> panel = hmi::panelForTool(tool);
    if (!panel) {
        return;
    }
    QDockWidget* dock = nullptr;
    switch (*panel) {
        case hmi::PanelId::Palette:
            dock = _ui->PalettePanel;
            break;
        case hmi::PanelId::Levels:
            dock = _ui->LevelsPanel;
            break;
        case hmi::PanelId::Layers:
            dock = _ui->LayersPanel;
            break;
        case hmi::PanelId::Entities:
            dock = _ui->EntitiesPanel;
            break;
    }
    // raise() met l'onglet au premier plan sans voler le focus clavier au canevas -- une
    // suggestion, jamais une confiscation.
    _suppressPanelFocusTracking = true;
    dock->raise();
    _suppressPanelFocusTracking = false;
}

void MainWindow::retranslateUi() {
    setWindowTitle(text("window.title"));

    _ui->PalettePanel->setWindowTitle(text("dock.palette"));
    _ui->LevelsPanel->setWindowTitle(text("dock.levels"));
    _ui->LayersPanel->setWindowTitle(text("dock.layers"));
    _ui->EntitiesPanel->setWindowTitle(text("dock.entities"));

    _ui->fileMenu->setTitle(text("menubar.file"));
    _ui->actQuit->setText(text("menubar.quit"));
    _ui->actResize->setText(text("menubar.resize"));
    _ui->editMenu->setTitle(text("menubar.edit"));
    _ui->levelMenu->setTitle(text("menubar.level"));
    _ui->helpMenu->setTitle(text("menubar.help"));
    _ui->viewMenu->setTitle(text("menubar.view"));
    _ui->panelsMenu->setTitle(text("menubar.panels"));
    _ui->themeMenu->setTitle(text("menubar.theme"));
    _ui->actThemeSystem->setText(text("menubar.theme_system"));
    _ui->actThemeLight->setText(text("menubar.theme_light"));
    _ui->actThemeDark->setText(text("menubar.theme_dark"));
    _actFollowActiveTool->setText(text("menubar.follow_active_tool"));
    _ui->actResetLayout->setText(text("menubar.reset_layout"));
    _actions->retranslateUi(_loc);

    _palette->retranslateUi(_loc);
    _levels->retranslateUi(_loc);
    _layers->retranslateUi(_loc);
    _entities->retranslateUi(_loc);

    refreshStatusHelp();
}

}  // namespace hmi
