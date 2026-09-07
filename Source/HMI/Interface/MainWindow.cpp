// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/MainWindow.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QColor>
#include <QColorDialog>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QFontMetrics>
#include <QFormLayout>
#include <QGuiApplication>
#include <QHeaderView>
#include <QIcon>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMoveEvent>
#include <QPixmap>
#include <QPoint>
#include <QRect>
#include <QResizeEvent>
#include <QScreen>
#include <QSettings>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStatusBar>
#include <QString>
#include <QStyleHints>
#include <QTableWidget>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
#include <algorithm>
#include <array>
#include <ctime>
#include <filesystem>
#include <vector>

#include "Core/Diagnostics/MemoryLogSink.h"
#include "HMI/Audio/SoundTriggers.h"
#include "HMI/Diagnostics/SessionLog.h"
#include "HMI/Editor/AssetReferences.h"
#include "HMI/Editor/EditorStatus.h"
#include "HMI/Editor/LevelBrowserPanel.h"
#include "HMI/Editor/LevelFileOperations.h"
#include "HMI/Editor/LinkPanel.h"
#include "HMI/Editor/PalettePanel.h"
#include "HMI/Editor/PanelFocus.h"
#include "HMI/Editor/PixelAssetIO.h"
#include "HMI/Editor/PixelCanvas.h"
#include "HMI/Editor/PixelHistoryPanel.h"
#include "HMI/Editor/PixelPalette.h"
#include "HMI/Editor/PixelPalettePanel.h"
#include "HMI/Editor/PlaneFileNaming.h"
#include "HMI/Editor/PlaneReference.h"
#include "HMI/Editor/PlanesPanel.h"
#include "HMI/Editor/TexturePanel.h"
#include "HMI/Game/GameViewport.h"
#include "HMI/Graphics/AssetContract.h"
#include "HMI/Graphics/PlaneVisuals.h"
#include "HMI/Graphics/TextureLoader.h"
#include "HMI/HmiLog.h"
#include "HMI/Input/GamepadButton.h"
#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/CreditsScreen.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/EditorActions.h"
#include "HMI/Interface/EditorWorkspace.h"
#include "HMI/Interface/IdentityScale.h"
#include "HMI/Interface/MainMenu.h"
#include "HMI/Interface/OptionsPage.h"
#include "HMI/Interface/PauseScreen.h"
#include "HMI/Interface/RpgScreenHost.h"
#include "HMI/Interface/ScreenPageHost.h"
#include "HMI/Platform/ExecutableDirectory.h"
#include "ui_MainWindow.h"
#include "ui_ResizeDialog.h"
#include "ui_ShortcutsDialog.h"

namespace hmi {

namespace {

// Version de la disposition sérialisée : à incrémenter si l'ensemble des docks change, pour
// invalider proprement une disposition sauvegardée devenue incompatible (`restoreState`).
constexpr int LAYOUT_VERSION =
    9;  // 9 : retrait du panneau Décors avec le système de décors (LOT-69 TACHE-04)
        // 8 : espaces de travail exclusifs, une disposition par espace (LOT-68)
        // 7 : panneau de palette de l'atelier pixel art rejoint le regroupement (LOT-54 TACHE-07)
        // 6 : atelier pixel art (canevas + historique) rejoint le regroupement Niveaux/Liens
        //     (LOT-54 TACHE-04)
        // 5 : panneau Outils devenu Decors (barre d'outils + inspecteur), Textures sort du
        //     regroupement en onglets (LOT-57, amendement post-essai manuel)

// Clés de persistance (portée application ; l'organisation/appli sont fixées dans `main`,
// HMI/Main.cpp).
constexpr char GEOMETRY_KEY[] = "mainWindow/geometry";
constexpr char STATE_KEY[] = "mainWindow/state";
// Réglage de mise en avant automatique des panneaux (LOT-57 TACHE-02) : local à MainWindow, pas
// une extension d'ApplicationTheme.cpp (qui concerne le thème, pas ce comportement).
constexpr char FOLLOW_ACTIVE_TOOL_KEY[] = "panels/followActiveTool";
// Espace de travail actif (LOT-68) : meme portee QSettings que la disposition et le theme.
constexpr char WORKSPACE_KEY[] = "mainWindow/workspace";
/// Opacite des reperes du mode creation (pelure d'oignon, plans voisins) : assez visible pour
/// situer, assez efface pour qu'on ne confonde jamais un repere avec ce qu'on peint.
constexpr float PLANE_REFERENCE_OPACITY = 0.45f;
// Reglage "contraindre a la palette" de l'atelier pixel art (LOT-54 TACHE-07).
constexpr char CONSTRAIN_TO_PALETTE_KEY[] = "pixelEditor/constrainToPalette";

}  // namespace

MainWindow::MainWindow(core::MemoryLogSink* sessionLog)
    : _ui(std::make_unique<Ui::EditorMainWindow>()),
      _stack(nullptr),
      _menu(nullptr),
      _options(nullptr),
      _viewport(new GameViewport()),
      _palette(nullptr),
      _levels(nullptr),
      _links(nullptr),
      _textures(nullptr),
      _pixelCanvas(nullptr),
      _pixelHistoryPanel(nullptr),
      _pixelPalettePanel(nullptr),
      _actions(nullptr),
      _toolBar(nullptr),
      _pixelToolBar(nullptr),
      _themeMenu(nullptr),
      _themeSystemAction(nullptr),
      _themeLightAction(nullptr),
      _themeDarkAction(nullptr),
      _loc(hmi::executableDirectory() / "Localization"),
      _sessionLog(sessionLog) {
    _ui->setupUi(this);  // barre de menus + docks (coquilles) depuis MainWindow.ui.

    // Catalogue de traduction : français par défaut (repli), langue active depuis les réglages.
    static_cast<void>(_loc.loadDefaultLanguage("fr"));
    const QString savedLanguage =
        QSettings().value(QStringLiteral("language"), QStringLiteral("fr")).toString();
    if (savedLanguage != QLatin1String("fr")) {
        static_cast<void>(_loc.loadLanguage(savedLanguage.toStdString()));
    }
    _viewport->setLocalization(&_loc);
    _editContext = _viewport;  ///< Seule implémentation aujourd'hui (LOT-57 TACHE-04).

    // Audio (LOT-60) : catalogue lu une fois, chaque son préchargé -- jamais au premier
    // déclenchement (QSoundEffect charge son fichier de façon asynchrone, TACHE-01). Absent ou
    // illisible : catalogue vide, le jeu reste jouable en silence (EX-NFR-040).
    const std::filesystem::path audioDirectory = hmi::executableDirectory() / "Audio";
    if (const hmi::SoundCatalogResult result =
            hmi::SoundCatalog::loadFromFile(audioDirectory / "sounds.json");
        result.ok()) {
        _sounds = std::move(*result.catalog);
    }
    for (const std::string& eventId : _sounds.eventIds()) {
        if (const std::optional<std::string> file = _sounds.resolve(eventId)) {
            _audio.preload(eventId, audioDirectory / *file);
        }
    }
    _viewport->setAudioEngine(&_audio);

    // Le viewport est un widget ordinaire depuis le LOT-69 TACHE-02 (QRhiWidget) : il entre
    // directement dans la pile centrale, sans conteneur de fenêtre native intermédiaire.
    _viewport->setMinimumSize(320, 240);
    _viewport->setFocusPolicy(Qt::StrongFocus);
    _viewport->installEventFilter(this);

    // Central : menu principal, options et viewport empilés (remplace le centralHost du .ui).
    _menu = new MainMenu();
    _options = new OptionsPage(_viewport, &_audio,
                               hmi::executableDirectory() / "Settings" / "keybindings.json");
    _credits = new CreditsScreen();
    _rpgScreens = new RpgScreenHost();
    _stack = new QStackedWidget(this);
    // Chaque ecran passe par une enveloppe defilante (LOT-73, EX-IHM-080) : sa taille minimale ne
    // remonte plus jusqu'a la fenetre. Le VIEWPORT en est exclu -- surface de rendu QRhi, il
    // remplit la page sans jamais defiler, et son minimum (320x240) tient sur tout ecran.
    addScreenPage(_menu);
    addScreenPage(_options);
    addScreenPage(_credits);
    addScreenPage(_rpgScreens);
    _stack->addWidget(_viewport);
    setCentralWidget(_stack);
    connect(_credits, &CreditsScreen::backRequested, this, &MainWindow::closeCredits);
    connect(_rpgScreens, &RpgScreenHost::closeRequested, this, &MainWindow::closeRpgScreen);
    // Le passage d'un ecran du RPG a un autre peut CHANGER la regle de superposition -- passer de
    // la carte (qui se consulte en marchant) a la fiche (qui suspend) doit suspendre. La regle est
    // donc appliquee a chaque changement d'ecran, pas seulement a l'ouverture (EX-IHM-091).
    connect(_rpgScreens, &RpgScreenHost::screenChanged, this, &MainWindow::applyRpgSuperposition);

    // Recouvrement de pause (LOT-59 TACHE-02) : widget ENFANT ORDINAIRE du viewport depuis le
    // LOT-69 TACHE-02. Il avait dû devenir une fenêtre de haut niveau (Qt::Dialog) parce qu'un
    // widget frère ne se dessinait jamais de façon fiable par-dessus la fenêtre native embarquée
    // par createWindowContainer -- deux défauts réels payés au LOT-59 (l'écran ne s'affichait pas,
    // puis Qt::Tool cassait activateWindow() sur Windows). QRhiWidget rendant dans une texture
    // composée avec le reste de l'interface, l'empilement redevient celui de Qt : un enfant
    // raise() suffit, le focus s'obtient sans activation de fenêtre, et la géométrie se donne en
    // coordonnées locales.
    _pauseScreen = new PauseScreen(_viewport);
    _pauseScreen->setAttribute(Qt::WA_TranslucentBackground);
    _pauseScreen->hide();
    connect(_pauseScreen, &PauseScreen::resumeRequested, this, &MainWindow::resumeFromPause);
    connect(_pauseScreen, &PauseScreen::optionsRequested, this, &MainWindow::showOptions);
    connect(_pauseScreen, &PauseScreen::quitToMenuRequested, this, &MainWindow::quitPauseToMenu);
    connect(_viewport, &GameViewport::pauseRequested, this, &MainWindow::openPause);

    buildUi();  // contenu des docks (panneaux) + branchement des actions de la barre de menus.

    // Contexte d'edition actif (LOT-54 TACHE-04, EX-IHM-062) : suit le focus clavier entre le
    // niveau (_viewport) et l'atelier pixel art (_pixelCanvas) -- Annuler/Refaire/Copier/Coller
    // (deja dispatches via _editContext) et la barre d'etat visent ainsi toujours le meme widget.
    connect(qApp, &QApplication::focusChanged, this,
            [this](QWidget*, QWidget* now) { updateActiveEditContext(now); });

    // Sélectionner une tuile dans la palette définit le type peint au clic dans le viewport.
    connect(_palette, &PalettePanel::tileSelected, _viewport,
            [this](core::TileType type) { _viewport->setActiveTile(type); });
    // Raccourci clavier de l'outil « Texture par instance » (LOT-45, « touche dédiée ») :
    // resynchronise la barre d'outils (LOT-56 TACHE-04), sans reboucler (setActiveTool n'émet
    // rien).
    connect(_viewport, &GameViewport::toolChanged, _actions, &EditorActions::setActiveTool);
    // Les messages d'état du viewport (enregistrement, essai, erreurs) s'affichent en bas, puis
    // laissent la main à l'aide contextuelle (LOT-57 TACHE-01).
    connect(_viewport, &GameViewport::statusMessage, this,
            [this](const QString& message) { showTransientStatusMessage(message, 5000); });
    // Barre d'état : zones permanentes (LOT-57 TACHE-01), recalculées à chaque changement
    // pertinent -- outil, survol, zoom, brouillon (nom, modifications).
    connect(_viewport, &GameViewport::toolChanged, this,
            [this](hmi::EditorTool) { refreshStatusHelp(); });
    // Mise en avant du panneau pertinent selon l'outil actif (LOT-57 TACHE-02).
    connect(_viewport, &GameViewport::toolChanged, this, &MainWindow::applyPanelFocus);
    connect(_viewport, &GameViewport::hoveredCellChanged, this,
            [this](std::optional<core::GridPosition>) { refreshStatusHelp(); });
    connect(_viewport, &GameViewport::zoomChanged, this, [this](float) { refreshStatusHelp(); });
    // Ouvrir un niveau depuis le panneau : garde-fou des modifications non enregistrées d'abord.
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

    // Panneau Liens : reste synchronise avec le brouillon (LOT-37) ; selectionner une ligne
    // surligne la liaison dans le viewport, supprimer delegue au viewport (seul proprietaire).
    // Section « Fond » (LOT-44) : meme synchronisation -- le viewport reste seul proprietaire du
    // brouillon, le panneau ne fait que refleter fond/jeu de skins du niveau courant.
    connect(_viewport, &GameViewport::draftChanged, this, [this] {
        _links->refresh(_viewport->draft());
        _textures->setLevelProperties(_viewport->draft().background(),
                                      _viewport->draft().skinSet());
        _textures->setLevelCameraFraming(_viewport->draft().cameraFraming());
        _textures->refreshObjects(_viewport->draft());
        _planes->refresh(_viewport->draft(), _viewport->selectedPlaneIndex(),
                         _viewport->planeVisibility());
        refreshStatusHelp();  // nom du niveau et indicateur de modification (LOT-57 TACHE-01).
    });
    connect(_links, &LinkPanel::linkSelected, _viewport, &GameViewport::setHighlightedLink);
    connect(_links, &LinkPanel::deleteRequested, _viewport, &GameViewport::unlinkMechanism);
    _links->refresh(_viewport->draft());  // etat initial (avant tout draftChanged).

    // Section « Objets » (LOT-45) : meme separation que le panneau Liens -- choisir un asset arme
    // l'outil « Texture par instance », la selection d'une ligne surligne dans le viewport, le
    // retrait passe par le viewport (seul proprietaire du brouillon).
    connect(_textures, &TexturePanel::textureOverrideAssetSelected, _viewport,
            [this](const QString& fileName) {
                _viewport->setActiveTextureAsset(
                    fileName.isEmpty() ? std::nullopt : std::make_optional(fileName.toStdString()));
            });
    connect(_textures, &TexturePanel::textureOverrideSelectionChanged, _viewport,
            &GameViewport::setHighlightedTextureOverride);
    connect(_textures, &TexturePanel::textureOverrideRemoveRequested, _viewport,
            &GameViewport::removeTextureOverride);

    _textures->refreshObjects(_viewport->draft());  // etat initial (avant tout draftChanged).

    // Panneau « Plans » (LOT-69 TACHE-08) : le panneau ne mute rien, il demande. Le viewport,
    // seul proprietaire du brouillon, applique -- donc tout passe par l'historique, sauf la
    // visibilite, qui est une aide d'edition.
    connectPlanesPanel();

    // Panneau Textures : agit sur le catalogue dont le viewport est proprietaire, et lui signale
    // le jeu courant. Aucune scene n'est reconstruite -- l'apparence est resolue a la composition,
    // donc l'image suivante suffit a montrer le resultat (LOT-42).
    _textures->setCatalog(&_viewport->skinCatalog());
    _textures->setLevelProperties(_viewport->draft().background(), _viewport->draft().skinSet());
    _textures->setLevelCameraFraming(_viewport->draft().cameraFraming());

    // Section « Fond » (LOT-44) : les deux modifications passent par le viewport (seul
    // proprietaire du brouillon), exactement comme le panneau Liens ci-dessus.
    connect(_textures, &TexturePanel::backgroundChanged, _viewport, [this](const QString& name) {
        _viewport->setLevelBackground(name.isEmpty() ? std::nullopt
                                                     : std::make_optional(name.toStdString()));
    });
    connect(_textures, &TexturePanel::levelSkinSetChanged, _viewport, [this](const QString& name) {
        _viewport->setLevelSkinSet(name.isEmpty() ? std::nullopt
                                                  : std::make_optional(name.toStdString()));
    });
    // Section « Cadrage » (LOT-64, EX-EDIT-028) : meme separation.
    connect(_textures, &TexturePanel::cameraFramingChanged, _viewport,
            &GameViewport::setLevelCameraFraming);
    connect(_textures, &TexturePanel::cameraZoneRemoveRequested, _viewport,
            &GameViewport::removeCameraZone);

    // Palette fidele au canevas (EX-EDIT-027) : elle interroge le MEME catalogue, et se rafraichit
    // aux trois evenements qui rendent ses vignettes obsoletes -- bascule de mode, changement de
    // jeu, reassignation. Peindre sans voir ce que l'on pose serait une regression d'usage.
    _palette->setSkinSource(hmi::executableDirectory() / "Assets" / "Skins",
                            &_viewport->skinCatalog());
    _palette->refreshThumbnails(_viewport->renderMode(), _textures->currentSet());

    connect(_textures, &TexturePanel::assignmentsChanged, this, [this] {
        _viewport->setSkinSet(_textures->currentSet());
        _palette->refreshThumbnails(_viewport->renderMode(), _textures->currentSet());
    });
    connect(_viewport, &GameViewport::renderModeChanged, this, [this](RenderMode mode) {
        _palette->refreshThumbnails(mode, _textures->currentSet());
    });

    // Le catalogue de skins n'est reellement charge qu'a la premiere exposition du canevas
    // (`GameViewport::ensureResources`, differe l'initialisation Direct3D) -- posterieure a ce
    // cablage, execute a la construction de la fenetre. Sans ce rafraichissement, l'arbre de
    // textures et la palette s'ouvrent vides et le restent jusqu'a la premiere bascule de mode ou
    // de jeu de skins (vieux defaut : "pas de texture au lancement du mode edition").
    connect(_viewport, &GameViewport::resourcesReady, this, [this] {
        _textures->setCatalog(&_viewport->skinCatalog());
        _palette->refreshThumbnails(_viewport->renderMode(), _textures->currentSet());
    });

    // Rechargement a chaud (LOT-43 TACHE-03) : un asset modifie/renomme/ajoute hors de
    // l'application n'est repris qu'a la demande explicite -- une surveillance automatique de
    // dossier a ete ecartee (editeurs d'image externes ecrivant en plusieurs passes, risque de
    // recharger un fichier partiellement ecrit). Invalider le TextureCache PUIS vider les caches
    // de vignettes, dans cet ordre : les vignettes redecoderont depuis un cache deja purge.
    connect(_textures, &TexturePanel::reloadRequested, this, [this] {
        _viewport->reloadAssets();
        _textures->reloadAssets();
        _palette->clearThumbnailCache();
        _palette->refreshThumbnails(_viewport->renderMode(), _textures->currentSet());
        showTransientStatusMessage(text("textures.reload_done"), 3000);
    });

    // Navigation depuis le menu principal.
    connect(_menu, &MainMenu::editorRequested, this, &MainWindow::showEditor);
    // Jouer (LOT-59 TACHE-06) : trois intentions distinctes remplacent l'ancien "Jouer" unique.
    connect(_menu, &MainMenu::newGameRequested, this, &MainWindow::newGame);
    connect(_menu, &MainMenu::optionsRequested, this, &MainWindow::showOptions);
    connect(_menu, &MainMenu::creditsRequested, this, &MainWindow::openCredits);
    connect(_menu, &MainMenu::quitRequested, this, &MainWindow::close);
    // Retour au menu à la fin d'une partie (ou Échap en mode jeu).
    connect(_viewport, &GameViewport::exitToMenuRequested, this, &MainWindow::showMenu);
    // Page Options : retour à l'écran d'origine (Menu ou Pause, EX-GP-041), bascule plein écran,
    // changement de langue, sauvegarde des logs.
    connect(_options, &OptionsPage::backRequested, this, &MainWindow::closeOptions);
    connect(_options, &OptionsPage::fullscreenRequested, this,
            [this](bool enabled) { enabled ? showFullScreen() : showNormal(); });
    connect(_options, &OptionsPage::languageChanged, this, &MainWindow::changeLanguage);
    connect(_options, &OptionsPage::saveLogsRequested, this, &MainWindow::saveSessionLogs);
    // Un remappage d'editeur (onglet Options > Éditeur, LOT-57 TACHE-04) doit se refleter
    // immediatement sur les raccourcis effectifs des actions (menu/barre d'outils).
    connect(_options, &OptionsPage::editorBindingsChanged, this,
            [this] { _actions->applyShortcuts(_viewport->editorBindings(), _loc); });

    // Navigation manette des menus : sondage périodique, actif seulement hors jeu/édition.
    _menuNavTimer = new QTimer(this);
    _menuNavTimer->setInterval(150);
    connect(_menuNavTimer, &QTimer::timeout, this, &MainWindow::pollMenuGamepad);

    resize(1280, 720);
    retranslateUi();  // applique la langue active à tous les textes construits ci-dessus.

    // Capture la disposition par défaut (après création des docks, avant restauration d'une
    // éventuelle disposition sauvegardée) : sert de cible à « Réinitialiser la disposition ».
    _defaultState = saveState(LAYOUT_VERSION);
    restoreLayout();

    // APRES restoreLayout : le facteur d'agrandissement est derive de la hauteur FINALE de la
    // fenetre, et la geometrie restauree peut differer du resize(1280, 720) ci-dessus. Le poser
    // ici, et non avant, evite que la premiere image du menu soit peinte a une echelle qu'un
    // resizeEvent devrait ensuite corriger sous les yeux du joueur.
    applyIdentityScale(true);

    // Espace de travail persiste (LOT-68) : on rouvre l'editeur la ou on l'a laisse. Applique
    // APRES restoreLayout, qui restaurerait sinon des docks des deux espaces.
    // Le mode creation n'est JAMAIS restaure au demarrage, meme si l'editeur y a ete laisse : il
    // suppose un niveau ouvert et un plan selectionne, dont rien ne garantit qu'ils existent
    // encore. On rouvre alors sur l'edition de niveau, d'ou « Peindre » y ramene en un clic.
    const EditorWorkspace restored =
        workspaceFromSettingsName(QSettings().value(QString::fromLatin1(WORKSPACE_KEY)).toString());
    const EditorWorkspace startWorkspace =
        restored == EditorWorkspace::Planes ? EditorWorkspace::Level : restored;
    workspaceSelector(startWorkspace)->setChecked(true);
    applyWorkspace(startWorkspace);

    // Contrainte de taille imposee par les ecrans, verifiee une fois au demarrage (EX-IHM-080).
    warnIfScreensConstrainWindow();

    showMenu();  // l'application démarre sur le menu principal.
}

void MainWindow::setDocksVisible(bool visible) {
    // Visibilite d'un dock = chassis d'edition visible ET dock appartenant a l'espace de travail
    // actif (LOT-68). Une CONJONCTION, jamais deux regles appliquees separement : la seconde
    // condition seule reste vraie hors edition, la premiere seule ignore le masquage par espace.
    // Les docks sont retrouves DYNAMIQUEMENT, jamais enumeres a la main : la garde doit rester
    // exhaustive quand un dock est ajoute, sans quoi le nouveau venu echapperait au masquage et
    // resterait affiche par-dessus le menu principal et le jeu.
    // Les panneaux d'edition n'ont de sens qu'en mode edition (EX-IHM-010).
    // Bascule de mode (edition/jeu/menu), pas un choix d'onglet : ne doit pas etre pris pour un
    // "l'utilisateur a impose un panneau" (LOT-57 TACHE-02).
    _suppressPanelFocusTracking = true;
    for (const auto& [dock, panel] : workspacePanels()) {
        dock->setVisible(visible &&
                         hmi::workspaceMaskContains(hmi::workspacesForPanel(panel), _workspace));
    }
    _suppressPanelFocusTracking = false;
}

std::array<std::pair<QDockWidget*, hmi::PanelId>, hmi::PANEL_COUNT> MainWindow::workspacePanels()
    const {
    // Table unique, relue par setDocksVisible ET par applyWorkspace : deux listes divergeraient au
    // premier dock ajoute, et le dock oublie resterait affiche dans les deux espaces.
    return {{
        {_ui->PalettePanel, hmi::PanelId::Palette},
        {_ui->PlanesPanel, hmi::PanelId::Planes},
        {_ui->LevelsPanel, hmi::PanelId::Levels},
        {_ui->LinksPanel, hmi::PanelId::Links},
        {_ui->TexturesPanel, hmi::PanelId::Textures},
        {_ui->PixelCanvasPanel, hmi::PanelId::PixelCanvas},
        {_ui->PixelHistoryPanel, hmi::PanelId::PixelHistory},
        {_ui->PixelPalettePanel, hmi::PanelId::PixelPalette},
    }};
}

bool MainWindow::transitionScreen(ScreenEvent event) {
    const std::optional<ScreenState> next = resolveTransition(_screenState, event);
    if (!next) {
        HMI_LOG_WARNING(
            "Transition d'ecran refusee (evenement non autorise depuis l'ecran "
            "courant, EX-GP-041).");
        return false;
    }
    _screenState = *next;
    applyScreenDressing(_screenState.screen);
    return true;
}

void MainWindow::applyScreenDressing(ScreenId screen) {
    // Choix de la page du QStackedWidget : seule part propre a Qt (pointeurs de widgets), hors de
    // portee d'une table pure (hmi::ScreenDressing). Pause recouvre Game (meme page) : son widget
    // se dessine PAR-DESSUS, la scene reste visible derriere.
    switch (screen) {
        case ScreenId::Menu:
            showScreenPage(_menu);
            break;
        case ScreenId::Options:
            showScreenPage(_options);
            break;
        case ScreenId::Credits:
            showScreenPage(_credits);
            break;
        case ScreenId::RpgScreen:
            showScreenPage(_rpgScreens);
            break;
        case ScreenId::Editor:
        case ScreenId::Game:
        case ScreenId::Pause:
            _stack->setCurrentWidget(_viewport);
            break;
    }

    // Recouvrement de pause (LOT-59 TACHE-02) : visible et au premier plan seulement sur cet
    // écran -- jamais une page de _stack (la scène doit rester dessinée derrière, cf. le
    // commentaire de construction de _pauseScreen). Ne touche jamais à l'état de pause du
    // viewport lui-même (GameViewport::pauseSimulation/resumeSimulation) : c'est le rôle exclusif
    // de openPause/resumeFromPause/quitPauseToMenu, jamais un effet de bord de
    // l'affichage -- une visite par Options (Pause -> Options -> Pause) ne doit pas reprendre puis
    // re-suspendre la simulation.
    const bool showPauseOverlay = screen == ScreenId::Pause;
    _pauseScreen->setVisible(showPauseOverlay);
    if (showPauseOverlay) {
        _pauseScreen->setGeometry(_viewport->rect());
        _pauseScreen->raise();
        // L'ecran de pause est un widget ENFANT (LOT-69 TACHE-02), pas une fenetre de haut niveau :
        // poser le focus est donc synchrone. Aucune activation de fenetre a attendre, donc aucun
        // report a un tour de boucle ulterieur avant de router le clavier.
        _pauseScreen->focusDefaultAction();
    }

    if (!showPauseOverlay && (screen == ScreenId::Editor || screen == ScreenId::Game)) {
        _viewport->setFocus();
    } else if (screen == ScreenId::Credits) {
        _credits->focusDefaultAction();
    } else if (screen == ScreenId::RpgScreen) {
        _rpgScreens->focusDefaultAction();
    }

    const ScreenDressing dressing = hmi::dressingFor(screen);
    setDocksVisible(dressing.docksVisible);
    menuBar()->setVisible(dressing.menuBarVisible);
    // Barres d'outils : mode ET espace de travail. dressing.pixelToolBarVisible dit que le
    // chassis d'edition est a l'ecran, hmi::dressingForWorkspace dit laquelle des deux barres --
    // les composer evite de rouvrir la barre de l'atelier en pleine edition de niveau.
    const hmi::WorkspaceDressing workspaceDressing = hmi::dressingForWorkspace(_workspace);
    _toolBar->setVisible(dressing.toolBarVisible && workspaceDressing.levelToolBarVisible);
    _pixelToolBar->setVisible(dressing.pixelToolBarVisible &&
                              workspaceDressing.pixelToolBarVisible);
    _actions->setEditingCommandsEnabled(dressing.editingCommandsEnabled);
    setMenuGamepadActive(dressing.gamepadNavigationActive);
    _statusMessageTimer->stop();
    refreshStatusHelp();
}

void MainWindow::connectPlanesPanel() {
    const auto refreshPlanes = [this] {
        _planes->refresh(_viewport->draft(), _viewport->selectedPlaneIndex(),
                         _viewport->planeVisibility());
        // Les reperes du mode creation montrent les plans VOISINS : toute mutation qui change leur
        // ordre, leur opacite ou leur visibilite les perime. Les rafraichir ici couvre chaque
        // mutation d'un seul geste, plutot qu'une ligne oubliee sur la neuvieme.
        refreshPlaneReferences();
    };

    connect(_planes, &PlanesPanel::planeSelected, _viewport, &GameViewport::selectPlane);
    connect(_viewport, &GameViewport::planeSelectionChanged, this,
            [refreshPlanes](std::optional<std::size_t>) { refreshPlanes(); });
    connect(_planes, &PlanesPanel::addRequested, this, &MainWindow::createPlane);
    connect(_planes, &PlanesPanel::removeRequested, this, [this](std::size_t index) {
        _viewport->removePlane(index);
        // Le FICHIER n'est pas supprime : le brouillon annule l'entree JSON, pas la disparition
        // d'une image. Un fichier orphelin est moins grave qu'un travail perdu -- et le dire dans
        // la barre d'etat evite que l'auteur croie avoir tout perdu.
        showTransientStatusMessage(text("planes.removed"), 4000);
    });
    connect(_planes, &PlanesPanel::paintRequested, this, [this](std::size_t index) {
        _viewport->selectPlane(index);
        if (_workspace == hmi::EditorWorkspace::Planes) {
            // Deja en mode creation : switchToWorkspace ne ferait rien (meme espace), c'est donc
            // ici que le changement de plan doit charger le nouveau sujet dans le canevas.
            if (_paintedPlane != index && confirmDiscardPlaneChanges()) {
                loadPlaneIntoCanvas(index);
            }
            return;
        }
        switchToWorkspace(hmi::EditorWorkspace::Planes);
    });
    connect(_planes, &PlanesPanel::reorderRequested, _viewport, &GameViewport::movePlane);
    connect(_planes, &PlanesPanel::depthChangeRequested, this,
            [this, refreshPlanes](std::size_t index, core::PlaneDepth depth) {
                _viewport->setPlaneDepth(index, depth);
                refreshPlanes();
            });
    connect(_planes, &PlanesPanel::densityChangeRequested, this, &MainWindow::changePlaneDensity);
    connect(_planes, &PlanesPanel::parallaxChangeRequested, _viewport,
            &GameViewport::setPlaneParallax);
    connect(_planes, &PlanesPanel::opacityChangeRequested, _viewport,
            &GameViewport::setPlaneOpacity);
    connect(_planes, &PlanesPanel::levelParallaxToggled, _viewport,
            &GameViewport::setLevelParallaxEnabled);
    connect(_planes, &PlanesPanel::visibilityToggled, this,
            [this, refreshPlanes](std::size_t index, bool visible) {
                _viewport->setPlaneVisible(index, visible);
                // Differe : ce signal vient d'un itemChanged emis PAR planeTree lui-meme (case a
                // cocher de la colonne Visible) -- refreshPlanes() reconstruit cet arbre (clear()
                // dans PlanesPanel::rebuildTree). Le faire de facon synchrone detruit l'item que
                // QTreeWidget est encore en train de traiter (le clic n'a pas fini de se propager
                // dans sa pile d'appels interne) -> crash. Un cran d'event loop suffit a laisser
                // Qt terminer son propre traitement avant qu'on ne vide l'arbre.
                QTimer::singleShot(0, this, refreshPlanes);
            });
    connect(_planes, &PlanesPanel::isolateToggled, this,
            [this, refreshPlanes](std::size_t index, bool isolate) {
                _viewport->setPlaneIsolated(index, isolate);
                refreshPlanes();
            });
}

std::filesystem::path MainWindow::planesDirectory() const {
    return hmi::executableDirectory() / "Levels" / "Plans";
}

void MainWindow::createPlane() {
    const core::LevelDraft& draft = _viewport->draft();
    core::Plane plane;
    plane.pixelsPerUnit = core::PLANE_NATIVE_PIXELS_PER_UNIT;

    // Nom derive de celui du niveau, unique par suffixe : un dossier de plans doit rester lisible
    // a l'oeil, et un plan se retrouver sans ouvrir l'editeur.
    std::vector<std::string> existing;
    for (const core::Plane& present : draft.planes()) {
        existing.push_back(present.fileName);
    }
    plane.fileName = hmi::uniquePlaneFileName(draft.name(), existing);

    const hmi::PlanePixelSize size =
        hmi::planePixelSize(draft.tileMap().width(), draft.tileMap().height(), plane.pixelsPerUnit);
    if (plane.fileName.empty() || size.width <= 0) {
        showTransientStatusMessage(text("planes.create_failed"), 4000);
        return;
    }

    // PNG entierement TRANSPARENT aux dimensions exactes : l'auteur peint dessus, il ne repart pas
    // d'un fond a effacer.
    hmi::DecodedImage image;
    image.width = size.width;
    image.height = size.height;
    image.pixels.assign(
        static_cast<std::size_t>(size.width) * static_cast<std::size_t>(size.height), 0u);

    std::error_code error;
    std::filesystem::create_directories(planesDirectory(), error);
    if (!hmi::encodeImageFile(planesDirectory() / plane.fileName, image)) {
        showTransientStatusMessage(text("planes.create_failed"), 4000);
        return;
    }

    const QString fileName = QString::fromStdString(plane.fileName);
    _viewport->addPlane(std::move(plane));
    showTransientStatusMessage(text("planes.added").arg(fileName), 3000);
}

void MainWindow::changePlaneDensity(std::size_t index, int pixelsPerUnit) {
    const core::LevelDraft& draft = _viewport->draft();
    if (index >= draft.planes().size()) {
        return;
    }
    const core::Plane& plane = draft.planes()[index];
    if (plane.pixelsPerUnit == pixelsPerUnit) {
        return;
    }

    // L'image suit la densite declaree : sans ce reechantillonnage, le fichier et le format
    // diraient deux choses differentes, et le controle de coherence (TACHE-10) le refuserait.
    // La perte est reelle et assumee -- descendre puis remonter ne restitue pas l'original.
    const std::filesystem::path path = planesDirectory() / plane.fileName;
    if (const std::optional<hmi::DecodedImage> current = hmi::decodeImageFile(path)) {
        const hmi::DecodedImage resampled =
            hmi::resamplePlane(*current, plane.pixelsPerUnit, pixelsPerUnit);
        if (resampled.width > 0 && !hmi::encodeImageFile(path, resampled)) {
            showTransientStatusMessage(text("planes.create_failed"), 4000);
            return;
        }
    }
    _viewport->setPlaneDensity(index, pixelsPerUnit);
    _viewport->reloadAssets();
    _planes->refresh(_viewport->draft(), _viewport->selectedPlaneIndex(),
                     _viewport->planeVisibility());
    // Le plan peint vient peut-etre de changer de resolution sous le canevas : le relire est plus
    // sur que de rehausser l'image en place, l'image de reference du disque etant l'autorite.
    if (_paintedPlane == index) {
        loadPlaneIntoCanvas(index);
    }
}

std::filesystem::path MainWindow::planeFilePath(std::size_t index) const {
    const std::vector<core::Plane>& planes = _viewport->draft().planes();
    if (index >= planes.size()) {
        return {};
    }
    return planesDirectory() / planes[index].fileName;
}

void MainWindow::loadPlaneIntoCanvas(std::size_t index) {
    const std::filesystem::path path = planeFilePath(index);
    if (path.empty()) {
        closePlaneInCanvas();
        return;
    }
    const core::Plane& plane = _viewport->draft().planes()[index];

    // Image absente ou illisible : on repart d'une surface TRANSPARENTE aux dimensions attendues
    // plutot que de refuser d'ouvrir. Le fichier a pu etre supprime hors de l'editeur, et un plan
    // qu'on ne peut plus peindre serait une impasse -- l'enregistrement le recreera (EX-NFR-040).
    const hmi::PlanePixelSize size =
        hmi::planePixelSize(_viewport->levelWidth(), _viewport->levelHeight(), plane.pixelsPerUnit);
    std::optional<hmi::DecodedImage> image = hmi::decodeImageFile(path);
    if (!image || image->width != size.width || image->height != size.height) {
        if (size.width <= 0) {
            closePlaneInCanvas();
            showTransientStatusMessage(text("planes.paint_failed"), 4000);
            return;
        }
        hmi::DecodedImage blank;
        blank.width = size.width;
        blank.height = size.height;
        blank.pixels.assign(
            static_cast<std::size_t>(size.width) * static_cast<std::size_t>(size.height), 0u);
        image = std::move(blank);
    }

    // setImage vide l'historique : un coup de pinceau dans un plan n'apparait JAMAIS dans
    // l'historique d'edition du niveau, ni reciproquement (critere d'acceptation du lot). Les deux
    // piles sont deja distinctes -- LevelDraft d'un cote, PixelHistory de l'autre -- et changer de
    // sujet dans le canevas repart d'une pile vierge.
    _pixelCanvas->setImage(std::move(*image));
    _pixelCanvas->setAssetName(plane.fileName);
    _pixelAssetPath.clear();  // un plan n'est pas un asset : Ctrl+S passe par savePlaneImage.
    _paintedPlane = index;
    refreshPlaneReferences();
    syncPaletteToCanvas();
    refreshStatusHelp();
}

void MainWindow::refreshPlaneReferences() {
    if (!_paintedPlane) {
        return;
    }
    const core::LevelDraft& draft = _viewport->draft();
    const std::vector<core::Plane>& planes = draft.planes();

    // Le rang du plan peint est re-resolu par son NOM DE FICHIER a chaque rafraichissement, jamais
    // conserve tel quel : monter ou descendre un plan change les rangs, et un rang memorise
    // designerait alors le voisin -- on peindrait dans une image et on en verrait une autre. Le
    // nom, lui, suit le plan. S'il a disparu de la liste, le plan a ete retire : on referme.
    const std::string& painted = _pixelCanvas->assetName();
    const auto found = std::ranges::find_if(
        planes, [&painted](const core::Plane& plane) { return plane.fileName == painted; });
    if (found == planes.end()) {
        closePlaneInCanvas();
        return;
    }
    _paintedPlane = static_cast<std::size_t>(std::distance(planes.begin(), found));
    const int density = found->pixelsPerUnit;

    // Les plans voisins, aplatis de part et d'autre du plan peint : ceux qui le precedent sous
    // l'image editee, ceux qui le suivent au-dessus. C'est ce qui rend l'ordre de la liste visible
    // pendant qu'on peint, plutot qu'apres coup a l'essai.
    std::vector<hmi::PlaneLayer> below;
    std::vector<hmi::PlaneLayer> above;
    std::vector<std::optional<hmi::DecodedImage>> images(planes.size());
    for (std::size_t rank = 0; rank < planes.size(); ++rank) {
        if (rank == *_paintedPlane) {
            continue;
        }
        images[rank] = hmi::decodeImageFile(planeFilePath(rank));
        if (!images[rank]) {
            continue;
        }
        hmi::PlaneLayer layer;
        layer.image = &*images[rank];
        layer.pixelsPerUnit = planes[rank].pixelsPerUnit;
        layer.opacity = planes[rank].opacity;
        layer.visible = _viewport->planeVisibility().visible(rank);
        (rank < *_paintedPlane ? below : above).push_back(layer);
    }

    const int width = _viewport->levelWidth();
    const int height = _viewport->levelHeight();
    // Pelure d'oignon des tuiles SOUS les plans arriere : la geometrie du niveau est le repere le
    // plus lointain, tout le reste se peint par-dessus.
    hmi::DecodedImage underlay = hmi::buildTileOnionSkin(draft, density);
    if (!below.empty()) {
        std::vector<hmi::PlaneLayer> stack;
        hmi::PlaneLayer tiles;
        tiles.image = &underlay;
        tiles.pixelsPerUnit = density;
        stack.push_back(tiles);
        stack.insert(stack.end(), below.begin(), below.end());
        underlay = hmi::flattenPlanes(stack, density, width, height);
    }
    _pixelCanvas->setUnderlay(std::move(underlay), PLANE_REFERENCE_OPACITY);
    _pixelCanvas->setOverlay(hmi::flattenPlanes(above, density, width, height),
                             PLANE_REFERENCE_OPACITY);
    // Grille de TUILES, distincte de la grille de pixels : sur un plan, c'est elle qui permet de
    // viser une case. Son pas est la densite meme du plan (un pixel par unite x densite).
    _pixelCanvas->setReferenceGridStep(density);
}

void MainWindow::closePlaneInCanvas() {
    if (!_paintedPlane) {
        return;
    }
    _paintedPlane.reset();
    _pixelCanvas->setImage(hmi::DecodedImage{});
    _pixelCanvas->setAssetName({});
    // Les reperes appartiennent au mode creation : les laisser derriere soi les ferait apparaitre
    // dans l'atelier pixel art, qui n'en a jamais demande (LOT-54 inchange).
    _pixelCanvas->setUnderlay(hmi::DecodedImage{});
    _pixelCanvas->setOverlay(hmi::DecodedImage{});
    _pixelCanvas->setReferenceGridStep(0);
    refreshStatusHelp();
}

void MainWindow::savePlaneImage() {
    if (!_paintedPlane) {
        return;
    }
    const std::filesystem::path path = planeFilePath(*_paintedPlane);
    if (path.empty()) {
        closePlaneInCanvas();
        return;
    }
    std::error_code error;
    std::filesystem::create_directories(path.parent_path(), error);
    if (!hmi::encodeImageFile(path, _pixelCanvas->image())) {
        showTransientStatusMessage(text("planes.save_failed"), 4000);
        return;
    }
    _pixelCanvas->markSaved();
    // Le niveau affiche derriere doit montrer ce qu'on vient d'enregistrer : sans cette
    // invalidation, le viewport garderait la texture chargee a l'ouverture du niveau.
    _viewport->reloadAssets();
    showTransientStatusMessage(
        text("planes.saved").arg(QString::fromStdString(path.filename().string())), 3000);
    refreshStatusHelp();
}

bool MainWindow::confirmDiscardPlaneChanges() {
    if (!_paintedPlane || !_pixelCanvas->isDirty()) {
        return true;
    }
    const QMessageBox::StandardButton answer =
        QMessageBox::question(this, text("planes.discard_title"), text("planes.discard_text"),
                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (answer == QMessageBox::Cancel) {
        return false;
    }
    if (answer == QMessageBox::Save) {
        savePlaneImage();
        return !_pixelCanvas->isDirty();  // l'ecriture a pu echouer : ne rien perdre alors.
    }
    return true;
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    applyIdentityScale();
}

void MainWindow::addScreenPage(QWidget* page) {
    auto* const host = new hmi::ScreenPageHost(page, _stack);
    _screenHosts.insert(page, host);
    _stack->addWidget(host);
}

void MainWindow::showScreenPage(QWidget* page) {
    // Repli sur la page elle-meme : le viewport n'est pas enveloppe, et un ecran ajoute sans
    // passer par addScreenPage doit rester affichable plutot que de ne rien afficher.
    _stack->setCurrentWidget(_screenHosts.value(page, page));
}

int MainWindow::availableLogicalHeight() const {
    const QScreen* const hostScreen = screen();
    // availableGeometry est exprimee en pixels LOGIQUES en Qt 6, comme height() : les deux
    // grandeurs sont directement comparables, sans passer par devicePixelRatio.
    return hostScreen == nullptr ? 0 : hostScreen->availableGeometry().height();
}

void MainWindow::warnIfScreensConstrainWindow() const {
    // `QStackedWidget::minimumSizeHint` est le MAXIMUM sur toutes ses pages, y compris celles qu'on
    // ne regarde pas : un seul ecran trop dense fixerait la taille minimale de la FENETRE, que
    // Windows refuse ensuite de retailler -- elle deborde sous la barre des taches en rognant son
    // contenu, sans rien dire. Le defaut s'est produit trois fois.
    //
    // Depuis le LOT-73 chaque ecran passe par une enveloppe defilante qui contribue ZERO
    // (EX-IHM-080), de sorte que ce plancher ne devrait plus jamais approcher la zone utile. On ne
    // se contente donc plus de journaliser la valeur : on la CONFRONTE a l'ecran, et un
    // depassement devient un avertissement dans le journal que l'utilisateur envoie.
    const QSize required = _stack->minimumSizeHint();
    HMI_LOG_INFO("Taille minimale imposee par les ecrans : " + std::to_string(required.width()) +
                 "x" + std::to_string(required.height()) + " (facteur " +
                 std::to_string(hmi::identityScale()) + ").");

    const QScreen* const hostScreen = screen();
    if (hostScreen == nullptr) {
        return;
    }
    const QSize available = hostScreen->availableGeometry().size();
    if (required.width() > available.width() || required.height() > available.height()) {
        HMI_LOG_WARNING("Un ecran impose une taille minimale de " +
                        std::to_string(required.width()) + "x" + std::to_string(required.height()) +
                        ", superieure a la zone disponible (" + std::to_string(available.width()) +
                        "x" + std::to_string(available.height()) +
                        ") : la fenetre debordera et son contenu sera rogne (EX-IHM-080).");
    }
}

void MainWindow::applyIdentityScale(bool beforeFirstShow) {
    // Fenetre en cours de fermeture ou de destruction : ne rien recalculer. Qt envoie encore des
    // evenements de redimensionnement pendant le demontage d'une QMainWindow a docks, ce qui
    // ramenerait le facteur a 1 et declencherait un rejeu de theme sur des widgets a moitie
    // detruits.
    //
    // L'exception est la CONSTRUCTION : la fenetre n'y est pas encore montree, et la garde
    // d'invisibilite y rendait l'appel sans effet -- l'application demarrait donc toujours au
    // facteur 1, le vrai facteur n'arrivant qu'au premier redimensionnement, sous les yeux du
    // joueur. C'est exactement ce que le commentaire de l'appelant pretendait eviter.
    if (_closing || (!isVisible() && !beforeFirstShow)) {
        return;
    }
    // Facteur ENTIER des ecrans du jeu (LOT-68, EX-IHM-070), derive de la hauteur LOGIQUE de la
    // fenetre : Qt applique la mise a l echelle systeme par-dessus. Le theme n est rejoue que
    // lorsque le facteur CHANGE -- le refaire a chaque pixel de redimensionnement reconstruirait
    // la feuille de style des dizaines de fois par seconde.
    //
    // BORNE par la zone d'affichage disponible, jamais par la seule hauteur de fenetre
    // (EX-IHM-081) : le facteur grossit les grandeurs d'habillage, qui grossissent la taille
    // minimale des ecrans, qui grossit celle de la fenetre -- laquelle relancerait le calcul un
    // cran plus haut, sans que rien ne redescende jamais. La zone disponible, elle, ne depend
    // d'aucune decision de l'application : c'est ce qui ferme la boucle.
    const bool scaleChanged =
        hmi::setIdentityScale(hmi::identityScaleForDisplay(height(), availableLogicalHeight()));
    // A la CONSTRUCTION, la feuille est posee meme si le facteur n'a pas bouge : la pile d'ecrans
    // n'en porte encore aucune, et la portee identite ne vit plus dans la feuille applicative
    // depuis le LOT-73. Sans cette exception, une fenetre ouvrant au facteur 1 -- la valeur
    // initiale -- laisserait les ecrans du jeu sans habillage.
    if (!scaleChanged && !beforeFirstShow) {
        return;
    }
    // Seule la feuille de la portee IDENTITE est reposee, et seulement sur la pile d'ecrans
    // (LOT-73, EX-IHM-082). C'est la difference qui fait disparaitre le gel.
    //
    // Auparavant, le facteur vivait dans la feuille de style de l'APPLICATION : en changer
    // repolissait ses 862 widgets, avec recalcul de metriques, de tailles et de dispositions pour
    // chacun -- cinq secondes par appel en Debug. Il avait fallu REGROUPER ces rejeux derriere un
    // minuteur, un glisser de bordure franchissant un seuil de facteur des dizaines de fois. Le
    // regroupement rendait le cout supportable sans le supprimer, et le faisait atterrir APRES le
    // relachement de la souris, quand la fenetre etait deja placee -- d'ou un recalage visible.
    //
    // Le cout est desormais proportionnel a ce qui change reellement : les quelques dizaines de
    // widgets des ecrans du jeu. Plus rien a differer, donc plus rien a regrouper -- le facteur
    // s'applique dans le redimensionnement lui-meme, ou l'utilisateur l'attend.
    applyIdentityStyleSheet();
}

void MainWindow::applyIdentityStyleSheet() {
    const QString sheet = hmi::identityStyleSheet();
    // Feuille absente ou invalide : garder la precedente. Un ecran sans habillage serait pire que
    // le meme ecran a un facteur perime (EX-NFR-040).
    if (sheet.isEmpty()) {
        return;
    }
    // Posee sur la PILE, jamais sur l'application : la pile ne contient que les ecrans et le
    // viewport. Les panneaux dockables, les barres et la barre d'etat -- l'essentiel des 862
    // widgets -- n'appartiennent pas a cette portee et n'ont donc pas a etre repolis.
    _stack->setStyleSheet(sheet);
}

void MainWindow::moveEvent(QMoveEvent* event) {
    QMainWindow::moveEvent(event);
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    if (watched == _viewport && event->type() == QEvent::Resize) {
        // Un recouvrement visible doit couvrir exactement le viewport : le suivre a sa taille
        // suffit desormais, les deux vivant dans le meme systeme de coordonnees.
        if (_pauseScreen != nullptr && _pauseScreen->isVisible()) {
            _pauseScreen->setGeometry(_viewport->rect());
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::playInterfaceSound(GameEvent event) {
    if (const std::optional<std::string> soundId = soundForEvent(event)) {
        _audio.play(*soundId);
    }
}

void MainWindow::openPause() {
    if (!transitionScreen(ScreenEvent::OpenPause)) {
        return;
    }
    HMI_LOG_INFO("Navigation : pause.");
    _viewport->pauseSimulation();
}

void MainWindow::resumeFromPause() {
    if (!transitionScreen(ScreenEvent::ResumePause)) {
        return;
    }
    HMI_LOG_INFO("Navigation : reprise depuis la pause.");
    _viewport->resumeSimulation();
}

void MainWindow::quitPauseToMenu() {
    const QMessageBox::StandardButton answer = QMessageBox::question(
        this, text("pause.quit_confirm_title"), text("pause.quit_confirm_text"));
    if (answer != QMessageBox::Yes) {
        return;
    }
    if (!transitionScreen(ScreenEvent::QuitPauseToMenu)) {
        return;
    }
    HMI_LOG_INFO("Navigation : partie abandonnee depuis la pause, retour au menu.");
    _viewport->quitGame();
}

void MainWindow::showMenu() {
    if (!transitionScreen(ScreenEvent::OpenMenu)) {
        return;
    }
    HMI_LOG_INFO("Navigation : menu principal.");
    // `_menu->setContinueEnabled(...)` : posé dans `applyScreenDressing` (cas `ScreenId::Menu`),
    // pas ici -- la plupart des retours au menu ne passent pas par cette méthode.
}

void MainWindow::showEditor() {
    if (!transitionScreen(ScreenEvent::OpenEditor)) {
        return;
    }
    HMI_LOG_INFO("Navigation : editeur.");
}

void MainWindow::newGame() {
    // ECHAFAUDAGE ASSUME (LOT-68). Cette entree devrait ouvrir une carte, et elle le fera : la
    // carte de depart arrive avec le contenu du LOT-27. Elle n'en a AUCUNE aujourd'hui -- le
    // LOT-01 a purge les niveaux du jeu de plateforme, et `demo-deplacement.json` n'existe pas.
    // Elle chargeait donc un fichier absent, et le LOT-67 l'a ecrit plutot que de le laisser
    // decouvrir.
    //
    // Elle ouvre en attendant le CHASSIS des ecrans du RPG, sur la fiche de personnage. Ce n'est
    // pas un pis-aller : les huit ecrans de ce lot sont vides par construction, et huit ecrans
    // qu'aucun chemin n'atteint ne se relisent pas, ne se naviguent pas et ne se valident pas. La
    // ligne a remplacer le jour ou il y aura une carte est CELLE-CI, et elle est seule.
    openRpgScreen(hmi::RpgScreenId::CharacterSheet);
}

void MainWindow::openRpgScreen(hmi::RpgScreenId screen) {
    if (!transitionScreen(ScreenEvent::OpenRpgScreen)) {
        return;
    }
    HMI_LOG_INFO("Navigation : ecran du RPG.");
    _rpgScreens->showScreen(screen);
}

void MainWindow::applyRpgSuperposition(hmi::RpgScreenId screen) {
    // Regle de superposition (EX-IHM-091) : la fiche, l'inventaire, le journal, le dialogue, le
    // marchand et le tableau de la Guilde suspendent la simulation ; la carte et l'ATH de combat
    // se consultent en marchant. La table le dit, ce code l'applique -- il ne redecide rien.
    if (_screenState.rpgReturnTo != ScreenId::Game && _screenState.rpgReturnTo != ScreenId::Pause) {
        return;  // ouvert depuis le menu : aucune simulation a suspendre.
    }
    if (hmi::pausesGame(screen)) {
        _viewport->pauseSimulation();
    } else {
        _viewport->resumeSimulation();
    }
}

void MainWindow::closeRpgScreen() {
    const ScreenId returnTo = _screenState.rpgReturnTo;
    if (!transitionScreen(ScreenEvent::CloseRpgScreen)) {
        return;
    }
    HMI_LOG_INFO("Navigation : fermeture d'un ecran du RPG.");
    // La simulation reprend si l'ecran ferme l'avait suspendue -- et SEULEMENT si l'on revient au
    // jeu : revenir a la pause doit laisser la scene figee, c'est tout son objet.
    if (returnTo == ScreenId::Game) {
        _viewport->resumeSimulation();
    }
}

void MainWindow::openCredits() {
    if (!transitionScreen(ScreenEvent::OpenCredits)) {
        return;
    }
    HMI_LOG_INFO("Navigation : credits.");
}

void MainWindow::closeCredits() {
    if (!transitionScreen(ScreenEvent::CloseCredits)) {
        return;
    }
    HMI_LOG_INFO("Navigation : retour au menu depuis les credits.");
}

void MainWindow::showOptions() {
    if (!transitionScreen(ScreenEvent::OpenOptions)) {
        return;
    }
    HMI_LOG_INFO("Navigation : options.");
}

void MainWindow::closeOptions() {
    if (!transitionScreen(ScreenEvent::CloseOptions)) {
        return;
    }
    HMI_LOG_INFO("Navigation : retour depuis les options.");
}

MainWindow::~MainWindow() = default;

void MainWindow::buildUi() {
    // Outils et commandes principales (LOT-56 TACHE-04) : une action unique par commande, partagée
    // entre la barre d'outils, le menu et son raccourci (plus de double définition). Icônes
    // construites depuis le thème d'éditeur actuellement effectif ; régénérées par
    // `EditorActions::refreshIcons` lors d'un changement de thème (TACHE-06).
    _actions = new EditorActions(hmi::currentEditorTokens(), this);
    // Raccourcis effectifs synchronises depuis les touches d'editeur remappables (LOT-57 TACHE-04)
    // : ActionCatalog reste sans dependance Qt (valeurs par defaut litterales), c'est ici que le
    // raccourci REELLEMENT actif est branche sur EditorKeyBindings.
    _actions->applyShortcuts(_viewport->editorBindings(), _loc);
    // Barres d'outils : declarees dans le .ui depuis le LOT-68, plus construites ici. Une seule
    // est visible a la fois, celle de l'espace de travail actif (applyWorkspace).
    _toolBar = _ui->EditorToolBar;
    _actions->populateToolBar(*_toolBar);
    // Barre d'outils DEDIEE du canevas pixel art (LOT-54 TACHE-04) : groupe d'actions distinct
    // (EditorActionGroup::PixelTools), jamais melange a la barre d'outils du niveau ci-dessus.
    _pixelToolBar = _ui->PixelToolBar;
    _actions->populatePixelToolBar(*_pixelToolBar);
    // Temoin + bouton du selecteur de couleur courante : cree ici (barre d'outils), mais rempli et
    // branche plus bas, APRES la construction de _pixelCanvas (sinon acces a un pointeur nul).
    _pixelToolBar->addSeparator();
    _pixelColorButton = new QToolButton(this);
    _pixelToolBar->addWidget(_pixelColorButton);

    // Contenu des docks : les coquilles (`PalettePanel`/`LevelsPanel`) et leur agencement
    // viennent du `.ui` ; leurs widgets, paramétrés (chemins, dépendances), sont créés en code.
    _palette = new PalettePanel(_ui->PalettePanel);
    _ui->PalettePanel->setWidget(_palette);
    // Panneau « Plans » (LOT-69 TACHE-08) : liste ordonnee des plans picturaux du niveau.
    _planes = new PlanesPanel(_ui->PlanesPanel);
    _ui->PlanesPanel->setWidget(_planes);
    _levels = new LevelBrowserPanel(hmi::executableDirectory() / "Levels", _ui->LevelsPanel);
    _ui->LevelsPanel->setWidget(_levels);
    _links = new LinkPanel(_ui->LinksPanel);
    _ui->LinksPanel->setWidget(_links);
    // Panneau d'habillage (LOT-42) : écrit `skins.json` au chemin **déployé**, exactement comme
    // l'enregistrement d'un niveau — aucun nouveau mécanisme d'écriture.
    _textures =
        new TexturePanel(hmi::executableDirectory() / "Assets" / "Skins",
                         hmi::executableDirectory() / "Assets" / "skins.json",
                         hmi::executableDirectory() / "Assets" / "Backgrounds",
                         hmi::executableDirectory() / "Assets" / "Objects", _ui->TexturesPanel);
    _ui->TexturesPanel->setWidget(_textures);
    // Panneau « Proprietes » (LOT-67) : reglages de GAMEPLAY, volontairement separes de
    // l'habillage porte par le panneau Textures ci-dessus.
    // Atelier pixel art (LOT-54 TACHE-04) : canevas et historique visuel, meme patron que les
    // panneaux ci-dessus (coquille du .ui, contenu branche en code).
    _pixelCanvas = new PixelCanvas(_ui->PixelCanvasPanel);
    _pixelCanvas->setLocalization(&_loc);  // infobulle de case en mode planche (TACHE-08).
    _ui->PixelCanvasPanel->setWidget(_pixelCanvas);
    // Temoin + selecteur de couleur courante (bouton cree plus haut, dans la barre d'outils) : pas
    // une action themee (les icones d'action sont recolorees depuis les jetons, EX-IHM-051) mais
    // un simple bouton dont la pastille montre la VRAIE couleur courante -- seul moyen d'atteindre
    // une couleur absente de l'image ouverte (pipette) et de la palette de projet (TACHE-07).
    updatePixelColorButtonIcon(_pixelCanvas->currentColor());
    connect(_pixelColorButton, &QToolButton::clicked, this, [this] { openPixelColorPicker(); });
    connect(_pixelCanvas, &PixelCanvas::currentColorChanged, this,
            [this](std::uint32_t color) { updatePixelColorButtonIcon(color); });
    _pixelHistoryPanel = new PixelHistoryPanel(_ui->PixelHistoryPanel);
    _ui->PixelHistoryPanel->setWidget(_pixelHistoryPanel);
    _pixelPalettePanel = new PixelPalettePanel(_ui->PixelPalettePanel);
    _ui->PixelPalettePanel->setWidget(_pixelPalettePanel);

    // Regroupement par defaut des panneaux Niveaux/Liens/Atelier/Historique/Palette en onglets
    // (LOT-57 TACHE-02, etendu LOT-54 TACHE-04/TACHE-07) : chacun reste individuellement
    // deplacable/detachable/refermable (EX-IHM-010), seule la disposition par defaut change.
    // REGLE (LOT-68) : une pile d'onglets ne regroupe que des panneaux du MEME domaine -- edition
    // de niveau d'un cote, atelier pixel art de l'autre. Le changement de domaine appartient au
    // selecteur d'espace de travail, pas a un onglet perdu au milieu d'un autre domaine. Textures
    // et Palette restent des docks independants. Doit preceder la capture de _defaultState
    // (constructeur, apres buildUi()).
    tabifyDockWidget(_ui->LevelsPanel, _ui->LinksPanel);
    tabifyDockWidget(_ui->PixelCanvasPanel, _ui->PixelHistoryPanel);
    tabifyDockWidget(_ui->PixelHistoryPanel, _ui->PixelPalettePanel);
    // Un changement de visibilite d'un de ces docks NON provoque par notre propre code (mise en
    // avant, bascule de mode, restauration de disposition -- toutes gardees par
    // _suppressPanelFocusTracking) ne peut venir que d'un choix explicite de l'utilisateur :
    // cliquer un onglet ou fermer/rouvrir le panneau. Meme principe pour un detachement
    // (topLevelChanged), toujours explicite, jamais gardee.
    for (QDockWidget* const dock : {_ui->LevelsPanel, _ui->LinksPanel, _ui->PixelCanvasPanel,
                                    _ui->PixelHistoryPanel, _ui->PixelPalettePanel}) {
        connect(dock, &QDockWidget::visibilityChanged, this, [this](bool) {
            if (!_suppressPanelFocusTracking) {
                _userPickedTab = true;
            }
        });
        connect(dock, &QDockWidget::topLevelChanged, this, [this](bool) { _userPickedTab = true; });
    }

    // Outils de niveau : la liste est DERIVEE du catalogue, jamais recopiee ici. C'est ce qui
    // garantit qu'un outil ajoute au catalogue est relie au viewport par construction -- une
    // liste ecrite a la main le rendrait cochable dans la barre d'outils sans le brancher, et
    // rien ne le signalerait a la compilation.
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
                        // Choisir un outil amene dans SON espace (LOT-68) : sinon l'outil devient
                        // actif dans un espace qui ne montre ni son canevas ni ses panneaux, et
                        // rien a l'ecran ne dit pourquoi il ne repond pas.
                        switchToWorkspace(hmi::workspaceForTool(tool));
                    }
                });
    }
    // Outils du canevas pixel art (LOT-54 TACHE-04) : meme patron que les outils de niveau
    // ci-dessus, sur le groupe d'actions distinct EditorActionGroup::PixelTools. Pas de touche
    // dediee a resynchroniser aujourd'hui (aucun raccourci clavier sur ces quatre actions) :
    // l'action est l'unique source de verite, contrairement aux outils de niveau.
    for (const hmi::PixelTool tool :
         {hmi::PixelTool::Brush, hmi::PixelTool::Eraser, hmi::PixelTool::Fill,
          hmi::PixelTool::Eyedropper, hmi::PixelTool::Selection}) {
        connect(_actions->pixelToolAction(tool), &QAction::toggled, _pixelCanvas,
                [this, tool](bool on) {
                    if (!on) {
                        return;
                    }
                    _pixelCanvas->setActiveTool(tool);
                    // JAMAIS depuis l'espace Plans (LOT-69) : la barre d'outils pixel y est
                    // partagee avec l'Atelier (hmi::dressingForWorkspace), donc ce bouton n'est
                    // visible que dans ces deux espaces -- mais hmi::workspaceForPixelTool renvoie
                    // toujours PixelArt. Le suivre depuis Plans fermerait le plan en cours
                    // (switchToWorkspace y vide le canevas, closePlaneInCanvas) alors que rien ne
                    // le demande : choisir la Gomme pendant qu'on peint un plan ne doit jamais
                    // faire perdre ce qu'on est en train de peindre.
                    if (_workspace != hmi::EditorWorkspace::Planes) {
                        switchToWorkspace(hmi::workspaceForPixelTool(tool));
                    }
                    refreshStatusHelp();
                    applyPixelPanelFocus(tool);
                });
    }
    // Canevas pixel art : recalcule de la barre d'etat a chaque changement pertinent (LOT-54
    // TACHE-04), meme discipline que le viewport ci-dessous. L'historique visuel se reconstruit a
    // chaque changement de l'historique (nouvelle entree, annuler, refaire).
    connect(_pixelCanvas, &PixelCanvas::imageChanged, this, [this] {
        refreshStatusHelp();
        updateLivePreview();
    });
    connect(_pixelCanvas, &PixelCanvas::hoveredPixelChanged, this,
            [this](std::optional<std::pair<int, int>>) { refreshStatusHelp(); });
    connect(_pixelCanvas, &PixelCanvas::historyChanged, this, [this] {
        _pixelHistoryPanel->refresh(_pixelCanvas->history());
        refreshStatusHelp();
    });
    connect(_pixelHistoryPanel, &PixelHistoryPanel::jumpRequested, _pixelCanvas,
            &PixelCanvas::jumpHistoryTo);
    _pixelHistoryPanel->refresh(_pixelCanvas->history());  // etat initial (historique vide).

    // Palette de projet de l'atelier pixel art (LOT-54 TACHE-07) : donnee d'auteur persistee dans
    // Assets/palettes.json, distincte des jetons de design (epic.md, decision de cadrage).
    _pixelPalette =
        hmi::PixelPalette::loadFromFile(hmi::executableDirectory() / "Assets" / "palettes.json");
    _pixelPalettePanel->refresh(_pixelPalette);
    syncPaletteToCanvas();
    _pixelPalettePanel->setConstrainEnabled(
        QSettings().value(QString::fromLatin1(CONSTRAIN_TO_PALETTE_KEY), false).toBool());
    _pixelCanvas->setPaletteConstrained(_pixelPalettePanel->constrainEnabled());

    connect(_pixelPalettePanel, &PixelPalettePanel::addRequested, this, [this] {
        const std::string name = text("pixel_palette.new_color_name")
                                     .arg(static_cast<int>(_pixelPalette.entries().size()) + 1)
                                     .toStdString();
        _pixelPalette.add(name, _pixelCanvas->currentColor());
        _pixelPalettePanel->refresh(_pixelPalette);
        syncPaletteToCanvas();
        savePixelPalette();
    });
    connect(_pixelPalettePanel, &PixelPalettePanel::removeRequested, this,
            [this](std::size_t index) {
                if (_pixelPalette.removeAt(index)) {
                    _pixelPalettePanel->refresh(_pixelPalette);
                    syncPaletteToCanvas();
                    savePixelPalette();
                }
            });
    connect(_pixelPalettePanel, &PixelPalettePanel::renameRequested, this,
            [this](std::size_t index) {
                if (index >= _pixelPalette.entries().size()) {
                    return;
                }
                bool accepted = false;
                const QString newName = QInputDialog::getText(
                    this, text("pixel_palette.rename"), text("pixel_palette.rename_prompt"),
                    QLineEdit::Normal, QString::fromStdString(_pixelPalette.entries()[index].name),
                    &accepted);
                if (!accepted || newName.isEmpty()) {
                    return;
                }
                _pixelPalette.renameAt(index, newName.toStdString());
                _pixelPalettePanel->refresh(_pixelPalette);
                savePixelPalette();
            });
    connect(_pixelPalettePanel, &PixelPalettePanel::moveRequested, this,
            [this](std::size_t index, bool up) {
                const std::size_t target = up ? index - 1 : index + 1;
                if (_pixelPalette.moveEntry(index, target)) {
                    _pixelPalettePanel->refresh(_pixelPalette);
                    syncPaletteToCanvas();
                    savePixelPalette();
                }
            });
    connect(_pixelPalettePanel, &PixelPalettePanel::extractRequested, this, [this] {
        for (const hmi::PixelPaletteExtractionEntry& extracted :
             hmi::extractPalette(_pixelCanvas->image())) {
            const std::string name = text("pixel_palette.new_color_name")
                                         .arg(static_cast<int>(_pixelPalette.entries().size()) + 1)
                                         .toStdString();
            _pixelPalette.add(name, extracted.color);
        }
        _pixelPalettePanel->refresh(_pixelPalette);
        syncPaletteToCanvas();
        savePixelPalette();
    });
    connect(_pixelPalettePanel, &PixelPalettePanel::constrainToggled, this, [this](bool enabled) {
        _pixelCanvas->setPaletteConstrained(enabled);
        QSettings().setValue(QString::fromLatin1(CONSTRAIN_TO_PALETTE_KEY), enabled);
        refreshStatusHelp();
    });
    connect(_pixelPalettePanel, &PixelPalettePanel::colorActivated, this,
            [this](std::uint32_t color) {
                _pixelCanvas->setCurrentColor(color);
                refreshStatusHelp();
            });

    // Commandes de fichier de l'atelier pixel art (LOT-54 TACHE-05).
    connect(_actions->action(hmi::IconId::PixelOpen), &QAction::triggered, this,
            [this] { openPixelAssetOpenDialog(); });
    connect(_actions->action(hmi::IconId::PixelCreate), &QAction::triggered, this,
            [this] { openPixelAssetCreateDialog(); });
    connect(_actions->action(hmi::IconId::PixelSave), &QAction::triggered, this,
            [this] { savePixelAsset(false); });
    connect(_actions->action(hmi::IconId::PixelSaveAs), &QAction::triggered, this,
            [this] { savePixelAsset(true); });
    // Menu dedie (decouvrabilite, EX-EDIT-015), memes actions que la barre d'outils du canevas --
    // aucune seconde definition. Le menu vient du .ui depuis le LOT-68 ; il n'est visible que dans
    // l'espace « Atelier pixel art ».
    _pixelMenu = _ui->workshopMenu;
    _pixelMenu->addAction(_actions->action(hmi::IconId::PixelOpen));
    _pixelMenu->addAction(_actions->action(hmi::IconId::PixelCreate));
    _pixelMenu->addAction(_actions->action(hmi::IconId::PixelSave));
    _pixelMenu->addAction(_actions->action(hmi::IconId::PixelSaveAs));

    // Commandes de region (LOT-54 TACHE-06) : Copier/Coller reutilisent le dispatch existant
    // (_editContext, IconId::Copy/Paste ci-dessous) -- rien a cabler ici pour elles.
    connect(_actions->action(hmi::IconId::PixelFlipHorizontal), &QAction::triggered, _pixelCanvas,
            [this] { _pixelCanvas->applyFlipHorizontal(); });
    connect(_actions->action(hmi::IconId::PixelFlipVertical), &QAction::triggered, _pixelCanvas,
            [this] { _pixelCanvas->applyFlipVertical(); });
    connect(_actions->action(hmi::IconId::PixelRotateClockwise), &QAction::triggered, _pixelCanvas,
            [this] { _pixelCanvas->applyRotateClockwise(); });
    connect(_actions->action(hmi::IconId::PixelRotateCounterClockwise), &QAction::triggered,
            _pixelCanvas, [this] { _pixelCanvas->applyRotateCounterClockwise(); });
    _pixelMenu->addSeparator();
    _pixelMenu->addAction(_actions->action(hmi::IconId::PixelFlipHorizontal));
    _pixelMenu->addAction(_actions->action(hmi::IconId::PixelFlipVertical));
    _pixelMenu->addAction(_actions->action(hmi::IconId::PixelRotateClockwise));
    _pixelMenu->addAction(_actions->action(hmi::IconId::PixelRotateCounterClockwise));

    // Ctrl+S ecrit ce que l'espace courant edite : le PNG du plan en mode creation, le JSON du
    // niveau partout ailleurs (LOT-69 TACHE-08). Deux notions de « modifie » distinctes depuis le
    // LOT-54 -- le canevas et le brouillon -- donc deux enregistrements distincts.
    connect(_actions->action(hmi::IconId::Save), &QAction::triggered, this, [this] {
        if (_workspace == hmi::EditorWorkspace::Planes) {
            savePlaneImage();
            return;
        }
        _viewport->save();
    });
    connect(_actions->action(hmi::IconId::Playtest), &QAction::triggered, _viewport,
            [this] { _viewport->startPlaytest(); });
    // Annuler/Refaire/Copier/Coller dispatchent via le contexte d'edition actif (_editContext,
    // GameViewport aujourd'hui) plutot que directement sur _viewport : le seuil de dispatch que
    // LOT-54 reutilisera pour sa propre cible (LOT-57 TACHE-04, EX-IHM-062).
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
    connect(_actions->action(hmi::IconId::ToggleRenderMode), &QAction::triggered, _viewport,
            [this] { _viewport->toggleRenderMode(); });
    // Renommer le niveau ouvert (LOT-57 TACHE-04) : meme dialogue que LevelBrowserPanel::onRename,
    // pre-rempli du nom courant.
    connect(_actions->action(hmi::IconId::Rename), &QAction::triggered, this, [this] {
        bool accepted = false;
        const QString name = QInputDialog::getText(
            this, text("map.rename"), text("map.rename_prompt"), QLineEdit::Normal,
            QString::fromStdString(_viewport->draft().name()), &accepted);
        if (!accepted || name.isEmpty()) {
            return;
        }
        if (_viewport->renameOpenLevel(name.toStdString())) {
            _levels->refresh();  // le fichier a pu changer de nom dans le dossier liste.
        }
    });
    // Aperçu des raccourcis (LOT-57 TACHE-04, concretise EX-EDIT-015) : lit les raccourcis
    // EFFECTIFS des actions a l'ouverture, jamais un texte fige -- toujours a jour apres un
    // remappage.
    connect(_actions->action(hmi::IconId::ShortcutsOverview), &QAction::triggered, this,
            [this] { openShortcutsDialog(); });

    // Commandes principales, reparties PAR NATURE D'ACTION (LOT-68, EX-IHM-074) et non plus
    // entassees dans un menu « Niveau » qui n'etait ni fichier ni edition. Toujours les memes
    // actions que la barre d'outils : aucune seconde definition (EX-IHM-055).
    _ui->fileMenu->insertAction(_ui->actResize, _actions->action(hmi::IconId::Save));
    _ui->fileMenu->insertAction(_ui->actResize, _actions->action(hmi::IconId::Rename));
    _ui->editMenu->addAction(_actions->action(hmi::IconId::Undo));
    _ui->editMenu->addAction(_actions->action(hmi::IconId::Redo));
    _ui->editMenu->addSeparator();
    _ui->editMenu->addAction(_actions->action(hmi::IconId::Copy));
    _ui->editMenu->addAction(_actions->action(hmi::IconId::Paste));
    _ui->levelMenu->addAction(_actions->action(hmi::IconId::Playtest));
    _ui->helpMenu->addAction(_actions->action(hmi::IconId::ShortcutsOverview));

    // Branchement du fonctionnel sur les actions restantes, déclarées dans le `.ui`.
    connect(_ui->actMainMenu, &QAction::triggered, this, &MainWindow::showMenu);
    connect(_ui->actQuit, &QAction::triggered, this, &MainWindow::close);
    connect(_ui->actResize, &QAction::triggered, this, [this] { openResizeDialog(); });
    connect(_ui->actResetLayout, &QAction::triggered, this, [this] {
        _suppressPanelFocusTracking = true;
        restoreState(_defaultState, LAYOUT_VERSION);
        _suppressPanelFocusTracking = false;
        _userPickedTab =
            false;  // repart sur la mise en avant automatique, disposition remise a neuf.
    });

    // Thème clair/sombre de l'éditeur (LOT-56 TACHE-06) : réglage Système/Clair/Sombre, persisté,
    // sans effet sur l'identité du jeu (menu principal/Options), qui reste toujours sombre.
    _themeMenu = _ui->themeMenu;
    _themeSystemAction = _ui->actThemeSystem;
    _themeLightAction = _ui->actThemeLight;
    _themeDarkAction = _ui->actThemeDark;
    auto* const themeGroup = new QActionGroup(this);
    themeGroup->setExclusive(true);
    for (QAction* const act : {_themeSystemAction, _themeLightAction, _themeDarkAction}) {
        act->setActionGroup(themeGroup);
    }
    switch (hmi::editorThemeSetting()) {
        case hmi::EditorThemeSetting::Light:
            _themeLightAction->setChecked(true);
            break;
        case hmi::EditorThemeSetting::Dark:
            _themeDarkAction->setChecked(true);
            break;
        case hmi::EditorThemeSetting::System:
            _themeSystemAction->setChecked(true);
            break;
    }
    // Re-genere palette + feuille de style + icones depuis le theme desormais effectif : les seuls
    // elements qui suivent les jetons de couleur (les vignettes d'assets, elles, sont un contenu
    // de jeu independant du thème de l'IHM, cf. epic.md).
    const auto applyThemeSetting = [this](hmi::EditorThemeSetting setting) {
        hmi::setEditorThemeSetting(setting);
        hmi::reapplyEditorTheme();
        _actions->refreshIcons(hmi::currentEditorTokens());
    };
    connect(_themeSystemAction, &QAction::triggered, this,
            [applyThemeSetting] { applyThemeSetting(hmi::EditorThemeSetting::System); });
    connect(_themeLightAction, &QAction::triggered, this,
            [applyThemeSetting] { applyThemeSetting(hmi::EditorThemeSetting::Light); });
    connect(_themeDarkAction, &QAction::triggered, this,
            [applyThemeSetting] { applyThemeSetting(hmi::EditorThemeSetting::Dark); });
    // Reglage "Systeme" : reagit a un changement live du theme du systeme d'exploitation.
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this,
            [this](Qt::ColorScheme) {
                if (hmi::editorThemeSetting() == hmi::EditorThemeSetting::System) {
                    hmi::reapplyEditorTheme();
                    _actions->refreshIcons(hmi::currentEditorTokens());
                }
            });
    // Commandes de VUE, en tete du menu Affichage : ce sont les seules qui agissent tout de
    // suite ; tout le reste du menu est un reglage, range en sous-menu.
    QAction* const firstViewSeparator = _ui->viewMenu->actions().constFirst();
    _ui->viewMenu->insertAction(firstViewSeparator, _actions->action(hmi::IconId::ResetCamera));
    _ui->viewMenu->insertAction(firstViewSeparator, _actions->action(hmi::IconId::ToggleGrid));
    _ui->viewMenu->insertAction(firstViewSeparator,
                                _actions->action(hmi::IconId::ToggleRenderMode));

    // Bascules de visibilité des docks : dynamiques, donc ajoutées ici. Elles rejoignent le
    // sous-menu « Panneaux » plutôt que la racine du menu Affichage, qui alignait vingt-trois
    // entrées à plat.
    for (QDockWidget* const dock : {_ui->PalettePanel, _ui->PlanesPanel, _ui->LevelsPanel,
                                    _ui->LinksPanel, _ui->TexturesPanel, _ui->PixelCanvasPanel,
                                    _ui->PixelHistoryPanel, _ui->PixelPalettePanel}) {
        _ui->panelsMenu->insertAction(_ui->panelsMenu->actions().constFirst(),
                                      dock->toggleViewAction());
    }

    // Mise en avant automatique des panneaux de droite selon l'outil actif (LOT-57 TACHE-02) :
    // reglage persiste, actif par defaut.
    // Selecteur d'espace de travail (LOT-68) : groupe exclusif, aucun etat intermediaire.
    auto* const workspaceGroup = new QActionGroup(this);
    workspaceGroup->setExclusive(true);
    // Entrees DERIVEES de l'enumeration EditorWorkspace, jamais recopiees une a une : ajouter un
    // espace de travail suffit a lui donner son selecteur, sans retouche ici. Toutes passent par
    // switchToWorkspace -- point d'entree unique, seul a savoir enregistrer la disposition qu'on
    // quitte et changer le sujet du canevas.
    for (const hmi::EditorWorkspace workspace :
         {hmi::EditorWorkspace::Level, hmi::EditorWorkspace::Planes,
          hmi::EditorWorkspace::PixelArt}) {
        QAction* const selector = workspaceSelector(workspace);
        selector->setActionGroup(workspaceGroup);
        connect(selector, &QAction::triggered, this,
                [this, workspace] { switchToWorkspace(workspace); });
    }

    _actFollowActiveTool = _ui->actFollowActiveTool;
    _actFollowActiveTool->setChecked(
        QSettings().value(QString::fromLatin1(FOLLOW_ACTIVE_TOOL_KEY), true).toBool());
    connect(_actFollowActiveTool, &QAction::toggled, this, [](bool enabled) {
        QSettings().setValue(QString::fromLatin1(FOLLOW_ACTIVE_TOOL_KEY), enabled);
    });

    // Mode d'inspection par calque (LOT-57 TACHE-03) : deplace depuis l'onglet Calques du panneau
    // Textures -- DECOMPOSE le rendu pour auditer chaque calque, jamais lu par hmi::GameSession, a
    // l'inverse de F8 qui le COMPOSE tel que le joueur le verra (EX-REN-046, la bascule Physique/
    // Texture est traitee en TACHE-04). Cases dans l'ORDRE DE DESSIN (hmi::RenderLayer,
    // EX-REN-014), toutes cochees par defaut, jamais persistees entre deux sessions.
    constexpr std::array<hmi::RenderLayer, 7> LAYER_ORDER{
        hmi::RenderLayer::Background, hmi::RenderLayer::Plane,  hmi::RenderLayer::Shadow,
        hmi::RenderLayer::Tile,       hmi::RenderLayer::Object, hmi::RenderLayer::Player,
        hmi::RenderLayer::Foreground};
    const std::array<QAction*, 7> LAYER_ACTIONS{_ui->actLayerBackground, _ui->actLayerPlaneBehind,
                                                _ui->actLayerShadow,     _ui->actLayerTileSkin,
                                                _ui->actLayerObjects,    _ui->actLayerPlayer,
                                                _ui->actLayerPlaneFront};
    for (std::size_t i = 0; i < LAYER_ORDER.size(); ++i) {
        const hmi::RenderLayer layer = LAYER_ORDER[i];
        QAction* const act = LAYER_ACTIONS[i];
        connect(act, &QAction::toggled, _viewport,
                [this, layer](bool checked) { _viewport->setLayerVisible(layer, checked); });
        _layerVisibilityActions[i] = act;
    }
    _actShowAllLayers = _ui->actShowAllLayers;
    connect(_actShowAllLayers, &QAction::triggered, this, [this] {
        _viewport->showAllLayers();
        // showAllLayers() n'emet pas de signal par calque : resynchronise les cases sans
        // redeclencher setLayerVisible sept fois (deja tout affiche cote rendu).
        for (QAction* const act : _layerVisibilityActions) {
            const QSignalBlocker blocker(act);
            act->setChecked(true);
        }
    });
    // Bascule Physique/Texture : posee plus haut avec les autres commandes de vue (LOT-68). Elle
    // n'apparait plus qu'a CET endroit -- elle figurait jusqu'ici trois fois (barre d'outils, menu
    // Niveau, menu Affichage), ce qui obligeait a deviner laquelle faisait autorite.

    // Barre d'état structurée (LOT-57 TACHE-01) : zones permanentes, ajoutées via
    // addPermanentWidget -- jamais recouvertes par un message transitoire (showMessage), à
    // l'inverse de l'ancienne chaîne unique `status.edit_help`. Largeur minimale sur les zones qui
    // changent au survol (case, zoom) : sans elle, la barre "saute" à chaque déplacement de souris.
    _statusLevel = new QLabel(this);
    _statusDirty = new QLabel(this);
    _statusTool = new QLabel(this);
    _statusHover = new QLabel(this);
    _statusHover->setMinimumWidth(fontMetrics().horizontalAdvance(QStringLiteral("(999, 999)")));
    _statusZoom = new QLabel(this);
    _statusZoom->setMinimumWidth(fontMetrics().horizontalAdvance(QStringLiteral("Zoom : 999%")));
    _statusColor = new QLabel(this);  // Couleur courante de l'atelier pixel art (LOT-54 TACHE-04).
    _statusCameraFraming = new QLabel(this);  // Cadrage de camera du niveau (EX-EDIT-028, LOT-64).
    for (QLabel* const zone : {_statusLevel, _statusDirty, _statusTool, _statusHover, _statusZoom,
                               _statusColor, _statusCameraFraming}) {
        statusBar()->addPermanentWidget(zone);
    }
    _statusMessageTimer = new QTimer(this);
    _statusMessageTimer->setSingleShot(true);
    connect(_statusMessageTimer, &QTimer::timeout, this, &MainWindow::refreshStatusHelp);
}

void MainWindow::openResizeDialog() {
    // Mise en page dans ResizeDialog.ui (LOT-68) : ici, seulement les bornes, la taille courante et
    // la confirmation d'une perte de contenu.
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
    // Lit les raccourcis EFFECTIFS des actions a l'ouverture, jamais un texte fige : l'apercu reste
    // juste apres un remappage (LOT-57 TACHE-04, EX-EDIT-015). Les commandes SANS raccourci sont
    // omises -- une ligne vide n'apprendrait rien.
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

QString MainWindow::layoutKeyFor(EditorWorkspace workspace) {
    switch (workspace) {
        case EditorWorkspace::Level:
            return QStringLiteral("mainWindow/state.level");
        case EditorWorkspace::Planes:
            return QStringLiteral("mainWindow/state.planes");
        case EditorWorkspace::PixelArt:
            return QStringLiteral("mainWindow/state.pixelart");
    }
    return QStringLiteral("mainWindow/state.level");
}

// Nom persiste d'un espace de travail. Un NOM, pas l'indice de l'enumeration : le LOT-68 ecrivait
// 0/1, et l'insertion de « Plans » entre les deux aurait fait rouvrir en mode creation l'editeur
// laisse dans l'atelier. Un nom inconnu (dont ces anciens 0/1) retombe sur l'edition de niveau.
QString MainWindow::workspaceSettingsName(EditorWorkspace workspace) {
    switch (workspace) {
        case EditorWorkspace::Level:
            return QStringLiteral("level");
        case EditorWorkspace::Planes:
            return QStringLiteral("planes");
        case EditorWorkspace::PixelArt:
            return QStringLiteral("pixelart");
    }
    return QStringLiteral("level");
}

EditorWorkspace MainWindow::workspaceFromSettingsName(const QString& name) {
    if (name == QStringLiteral("planes")) {
        return EditorWorkspace::Planes;
    }
    if (name == QStringLiteral("pixelart")) {
        return EditorWorkspace::PixelArt;
    }
    return EditorWorkspace::Level;
}

QAction* MainWindow::workspaceSelector(EditorWorkspace workspace) const {
    switch (workspace) {
        case EditorWorkspace::Level:
            return _ui->actWorkspaceLevel;
        case EditorWorkspace::Planes:
            return _ui->actWorkspacePlanes;
        case EditorWorkspace::PixelArt:
            return _ui->actWorkspacePixelArt;
    }
    return _ui->actWorkspaceLevel;
}

void MainWindow::switchToWorkspace(EditorWorkspace workspace) {
    if (_workspace == workspace) {
        return;  // deja la : ne pas resauvegarder ni rejouer une disposition pour rien.
    }
    // Bascule annulee par un garde-fou : l'entree de menu s'est deja cochee toute seule (elles
    // sont exclusives dans un QActionGroup), il faut donc rendre la coche a l'espace ou l'on reste
    // -- sinon le menu annoncerait un espace different de celui qui est affiche.
    const auto cancel = [this] {
        QAction* const stay = workspaceSelector(_workspace);
        const QSignalBlocker blocker(stay);
        stay->setChecked(true);
    };
    // Le canevas est PARTAGE par les deux espaces de peinture (LOT-69 TACHE-08) : changer d'espace
    // change son sujet, donc peut perdre ce qui n'y est pas enregistre. Le garde-fou est pose ici,
    // une fois, plutot que sur chacun des chemins qui menent a un espace.
    if (workspace == EditorWorkspace::Planes) {
        // Un plan OU un asset, jamais les deux : le canevas n'a qu'un sujet a la fois, et poser
        // les deux gardes en serie ferait poser deux fois la meme question.
        if (_paintedPlane ? !confirmDiscardPlaneChanges() : !confirmDiscardPixelChanges()) {
            cancel();
            return;
        }
    } else if (_paintedPlane && workspace == EditorWorkspace::PixelArt) {
        // L'atelier ne doit jamais retrouver un plan dans le canevas : Ctrl+S y ecrirait un asset.
        if (!confirmDiscardPlaneChanges()) {
            cancel();
            return;
        }
        closePlaneInCanvas();
    }
    // La disposition de l'espace QUITTE est sauvegardee ICI, et non dans applyWorkspace : c'est
    // cette methode qui sait qu'on quitte vraiment un espace. A l'initialisation, il n'y a rien a
    // sauvegarder, et applyWorkspace est appelee directement.
    QSettings().setValue(layoutKeyFor(_workspace), saveState(LAYOUT_VERSION));
    // Le selecteur suit l'etat, sans reemettre : c'est le MEME etat atteint par deux chemins
    // (menu, choix d'outil), jamais deux etats (EX-IHM-062).
    QAction* const selector = workspaceSelector(workspace);
    const QSignalBlocker blocker(selector);
    selector->setChecked(true);
    applyWorkspace(workspace);

    // Charger le plan APRES l'habillage : le canevas doit etre visible quand il recoit son image,
    // sinon son premier cadrage se calcule sur une taille de widget qui n'est pas encore la sienne.
    if (workspace == EditorWorkspace::Planes) {
        if (const std::optional<std::size_t> selected = _viewport->selectedPlaneIndex()) {
            loadPlaneIntoCanvas(*selected);
        } else {
            closePlaneInCanvas();
            showTransientStatusMessage(text("planes.none_selected"), 4000);
        }
    }
}

void MainWindow::applyWorkspace(EditorWorkspace workspace) {
    QSettings settings;
    _workspace = workspace;
    settings.setValue(QString::fromLatin1(WORKSPACE_KEY), workspaceSettingsName(workspace));

    // Toute la manipulation est gardee : masquer un dock emet visibilityChanged, que le suivi de
    // panneau prendrait sinon pour un choix d'onglet de l'utilisateur.
    _suppressPanelFocusTracking = true;

    const hmi::WorkspaceDressing dressing = hmi::dressingForWorkspace(workspace);
    const bool toolBarsAllowed = hmi::dressingFor(_screenState.screen).toolBarVisible;
    _toolBar->setVisible(dressing.levelToolBarVisible && toolBarsAllowed);
    _pixelToolBar->setVisible(dressing.pixelToolBarVisible && toolBarsAllowed);
    _pixelMenu->menuAction()->setVisible(dressing.workshopMenuVisible);

    // Panneaux : la table decide, la fenetre applique. Aucune condition ecrite en dur sur un dock.
    const auto PANELS = workspacePanels();
    // Hors mode edition (menu principal, jeu), aucun dock ne doit reapparaitre : la bascule
    // d'espace ne rend pas le chassis d'edition visible, elle dit seulement lequel le serait.
    const bool editing = hmi::dressingFor(_screenState.screen).docksVisible;
    for (const auto& [dock, panel] : PANELS) {
        const bool belongsHere =
            hmi::workspaceMaskContains(hmi::workspacesForPanel(panel), workspace);
        // La bascule de visibilite du menu suit : un panneau d'un autre espace n'a pas a etre
        // proposable depuis celui-ci.
        dock->toggleViewAction()->setVisible(belongsHere);
        dock->setVisible(belongsHere && editing);
    }

    // Disposition propre a l'espace, si on y est deja venu.
    const QByteArray state = settings.value(layoutKeyFor(workspace)).toByteArray();
    if (!state.isEmpty()) {
        restoreState(state, LAYOUT_VERSION);
        // restoreState reaffiche les docks tels qu'ils etaient enregistres, y compris ceux de
        // l'autre espace si une disposition ancienne en portait : on les remasque.
        for (const auto& [dock, panel] : PANELS) {
            if (!hmi::workspaceMaskContains(hmi::workspacesForPanel(panel), workspace)) {
                dock->setVisible(false);
            }
        }
    }

    _suppressPanelFocusTracking = false;
    _userPickedTab = false;  // nouvel espace : la mise en avant automatique repart.
    refreshStatusHelp();
}

void MainWindow::restoreLayout() {
    const QSettings settings;
    const QByteArray geometry = settings.value(QString::fromLatin1(GEOMETRY_KEY)).toByteArray();
    const QByteArray state = settings.value(QString::fromLatin1(STATE_KEY)).toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
        // La geometrie persistee n'a jamais ete bornee a l'ecran (EX-IHM-081) : une session qui
        // s'est terminee sur une fenetre debordante la restituait telle quelle, et le defaut
        // survivait au redemarrage. On la ramene dans la zone utile -- taille PUIS position, dans
        // cet ordre : deplacer une fenetre trop grande ne la ferait pas tenir.
        const QScreen* const hostScreen = screen();
        if (hostScreen != nullptr) {
            const QRect available = hostScreen->availableGeometry();
            // Bordures de fenetre comprises : c'est le CADRE qui doit tenir dans la zone utile, pas
            // la seule zone client. resize() dimensionne la zone client, d'ou le retrait.
            const QSize decorations = frameGeometry().size() - size();
            const QSize fittedFrame = frameGeometry().size().boundedTo(available.size());
            if (fittedFrame != frameGeometry().size()) {
                resize(fittedFrame - decorations);
            }
            // move() positionne le CADRE d'une fenetre de haut niveau : aucune conversion a faire.
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
    // Pose AVANT toute autre chose : a partir d'ici, plus aucun evenement differe ne doit toucher
    // au theme ni a la disposition (cf. applyIdentityScale).
    _closing = true;
    saveLayout();
    QMainWindow::closeEvent(event);
}

void MainWindow::setMenuGamepadActive(bool active) {
    if (active) {
        _menuNavTimer->start();
    } else {
        _menuNavTimer->stop();
    }
}

void MainWindow::pollMenuGamepad() {
    _menuPad.poll(_menuPadInput);

    // Poste un événement clavier Qt à la cible focalisée (repli : page courante de la pile), pour
    // que la manette pilote la **navigation de focus** standard de Qt sans code de layout dédié.
    QWidget* const target = QApplication::focusWidget() != nullptr ? QApplication::focusWidget()
                                                                   : _stack->currentWidget();
    const auto post = [target](Qt::Key key, Qt::KeyboardModifiers mods) {
        if (target == nullptr) {
            return;
        }
        QApplication::postEvent(target, new QKeyEvent(QEvent::KeyPress, key, mods));
        QApplication::postEvent(target, new QKeyEvent(QEvent::KeyRelease, key, mods));
    };

    // Bas/Droite -> focus suivant ; Haut/Gauche -> focus précédent ; A -> activer.
    if (_menuPadInput.gamepadButtonPressed(GamepadButton::Down) ||
        _menuPadInput.gamepadButtonPressed(GamepadButton::Right)) {
        post(Qt::Key_Tab, Qt::NoModifier);
        playInterfaceSound(GameEvent::MenuNavigate);
    }
    if (_menuPadInput.gamepadButtonPressed(GamepadButton::Up) ||
        _menuPadInput.gamepadButtonPressed(GamepadButton::Left)) {
        post(Qt::Key_Backtab, Qt::ShiftModifier);
        playInterfaceSound(GameEvent::MenuNavigate);
    }
    if (_menuPadInput.gamepadButtonPressed(GamepadButton::A)) {
        post(Qt::Key_Return, Qt::NoModifier);
        playInterfaceSound(GameEvent::MenuConfirm);
    }
    // Epaules : passage d'un ecran du RPG a l'autre (LOT-68). C'est le pendant manette du bouton
    // du pied de page, et la raison pour laquelle le rappel de touches annonce LB/RB : un rappel
    // qui nomme une touche inerte est pire que pas de rappel du tout (EX-IHM-072).
    if (_screenState.screen == ScreenId::RpgScreen) {
        if (_menuPadInput.gamepadButtonPressed(GamepadButton::RightShoulder)) {
            _rpgScreens->showNextScreen();
            playInterfaceSound(GameEvent::MenuNavigate);
        }
        if (_menuPadInput.gamepadButtonPressed(GamepadButton::LeftShoulder)) {
            _rpgScreens->showPreviousScreen();
            playInterfaceSound(GameEvent::MenuNavigate);
        }
    }

    // B : retour contextuel (depuis Options vers son écran d'origine, ou reprise depuis la pause
    // -- LOT-59 TACHE-02), sans quitter depuis le menu principal.
    if (_menuPadInput.gamepadButtonPressed(GamepadButton::B)) {
        if (_screenState.screen == ScreenId::Options) {
            closeOptions();
        } else if (_screenState.screen == ScreenId::Pause) {
            resumeFromPause();
        } else if (_screenState.screen == ScreenId::Credits) {
            closeCredits();
        } else if (_screenState.screen == ScreenId::RpgScreen) {
            closeRpgScreen();
        }
    }

    _menuPadInput.beginFrame();
}

QString MainWindow::text(const char* key) const {
    return QString::fromStdString(_loc.text(key));
}

void MainWindow::refreshStatusHelp() {
    EditorStatusContext context;
    // Contexte actif seulement en édition (pas en jeu/essai ni au menu/Options) : même condition
    // que l'ancien rechargement de `status.edit_help` en changement de langue. Lequel des deux
    // contextes (niveau/atelier) dépend du widget qui a le focus clavier (_editContext, LOT-54
    // TACHE-04) -- jamais les deux en même temps (EX-IHM-062).
    if (_stack->currentWidget() == _viewport && menuBar()->isVisible()) {
        // En mode creation, le sujet de la barre d'etat est le PLAN, independamment du focus
        // clavier : l'espace n'a pas d'autre sujet, et le niveau ou son zoom de camera n'y
        // apprennent rien a qui peint une image. Ailleurs, la regle du LOT-54 s'applique -- c'est
        // le widget focalise qui decide.
        const bool paintingPlane = _workspace == hmi::EditorWorkspace::Planes && _paintedPlane;
        if (paintingPlane || _editContext == static_cast<EditContextTarget*>(_pixelCanvas)) {
            PixelEditStatusInfo pixel;
            pixel.assetName = _pixelCanvas->assetName();
            pixel.dirty = _pixelCanvas->isDirty();
            pixel.tool = _pixelCanvas->activeTool();
            pixel.hoveredPixel = _pixelCanvas->hoveredPixel();
            pixel.zoom = _pixelCanvas->view().zoom;
            pixel.currentColor = _pixelCanvas->currentColor();
            pixel.paletteConstrained = _pixelCanvas->paletteConstrained();
            // Mode creation : le canevas peint un plan, pas un asset. Trois informations de plus
            // -- resolution, densite, poids memoire (EX-NFR-043) -- et rien d'autre ne change :
            // meme canevas, memes outils, meme couleur courante.
            if (_paintedPlane && *_paintedPlane < _viewport->draft().planes().size()) {
                const core::Plane& plane = _viewport->draft().planes()[*_paintedPlane];
                hmi::PlaneEditStatusInfo planeInfo;
                planeInfo.widthPixels = _pixelCanvas->image().width;
                planeInfo.heightPixels = _pixelCanvas->image().height;
                planeInfo.pixelsPerUnit = plane.pixelsPerUnit;
                planeInfo.textureBytes = hmi::planeTextureMemoryBytes(
                    plane, _viewport->levelWidth(), _viewport->levelHeight());
                pixel.plane = planeInfo;
            }
            context.pixelEdit = pixel;
        } else {
            LevelStatusInfo level;
            level.name = _viewport->draft().name();
            level.dirty = _viewport->isDirty();
            level.tool = _viewport->activeTool();
            level.hoveredCell = _viewport->hoveredCell();
            level.zoom = _viewport->zoom();
            level.cameraFraming = _viewport->draft().cameraFraming().mode;
            context.level = level;
        }
    }
    const EditorStatusLines lines = editorStatusLines(context, _loc);
    _statusLevel->setText(QString::fromStdString(lines.permanent[0]));
    _statusDirty->setText(QString::fromStdString(lines.permanent[1]));
    _statusTool->setText(QString::fromStdString(lines.permanent[2]));
    _statusHover->setText(QString::fromStdString(lines.permanent[3]));
    _statusZoom->setText(QString::fromStdString(lines.permanent[4]));
    _statusColor->setText(QString::fromStdString(lines.permanent[5]));
    _statusCameraFraming->setText(QString::fromStdString(lines.permanent[6]));
    statusBar()->showMessage(QString::fromStdString(lines.help));
}

void MainWindow::showTransientStatusMessage(const QString& message, int timeoutMs) {
    // Timeout laisse a 0 (defaut de showMessage) : le message reste affiche jusqu'a la
    // restauration explicite par _statusMessageTimer, seul maitre de sa duree de vie. Confier ce
    // delai a Qt le viderait sans que le timer le sache.
    statusBar()->showMessage(message);
    _statusMessageTimer->start(timeoutMs);
}

void MainWindow::applyPanelFocus(hmi::EditorTool tool) {
    if (!_actFollowActiveTool->isChecked() || _userPickedTab) {
        return;  // reglage desactive, ou l'utilisateur a deja impose un onglet pour la session.
    }
    const std::optional<hmi::PanelId> panel = hmi::panelForTool(tool);
    if (!panel) {
        return;  // cet outil n'a pas de panneau dedie.
    }
    raisePanel(*panel);
}

void MainWindow::applyPixelPanelFocus(hmi::PixelTool tool) {
    if (!_actFollowActiveTool->isChecked() || _userPickedTab) {
        return;
    }
    const std::optional<hmi::PanelId> panel = hmi::panelForPixelTool(tool);
    if (!panel) {
        return;
    }
    raisePanel(*panel);
}

void MainWindow::raisePanel(hmi::PanelId panel) {
    QDockWidget* dock = nullptr;
    switch (panel) {
        case hmi::PanelId::Levels:
            dock = _ui->LevelsPanel;
            break;
        case hmi::PanelId::Links:
            dock = _ui->LinksPanel;
            break;
        case hmi::PanelId::Textures:
            dock = _ui->TexturesPanel;
            break;
        case hmi::PanelId::PixelCanvas:
            dock = _ui->PixelCanvasPanel;
            break;
        case hmi::PanelId::PixelHistory:
            dock = _ui->PixelHistoryPanel;
            break;
    }
    // raise() met l'onglet au premier plan sans voler le focus clavier au canevas -- une
    // suggestion, jamais une confiscation (ligne rouge de cette tache).
    _suppressPanelFocusTracking = true;
    dock->raise();
    _suppressPanelFocusTracking = false;
}

void MainWindow::updateActiveEditContext(QWidget* focused) {
    if (focused == nullptr) {
        return;  // perte de focus (fenetre inactive) : conserve le contexte actuel.
    }
    EditContextTarget* const target =
        (focused == _pixelCanvas || _pixelCanvas->isAncestorOf(focused))
            ? static_cast<EditContextTarget*>(_pixelCanvas)
            : static_cast<EditContextTarget*>(_viewport);
    if (target != _editContext) {
        _editContext = target;
        refreshStatusHelp();  // les zones affichees dependent du contexte actif (LOT-54 TACHE-04).
    }
}

bool MainWindow::confirmDiscardPixelChanges() {
    if (!_pixelCanvas->isDirty()) {
        return true;
    }
    // Meme patron que le garde-fou d'ouverture de niveau (EX-EDIT-021) : la meme paire de cles de
    // traduction convient, la question posee est identique pour un autre type de document.
    const QMessageBox::StandardButton answer =
        QMessageBox::question(this, text("dialog.unsaved_title"), text("dialog.unsaved_text"));
    return answer == QMessageBox::Yes;
}

std::string MainWindow::pixelAssetCacheKey() const {
    std::error_code error;
    const std::filesystem::path assetsDirectory = hmi::executableDirectory() / "Assets";
    const std::filesystem::path relative =
        std::filesystem::relative(_pixelAssetPath, assetsDirectory, error);
    if (error) {
        return _pixelAssetPath.filename().string();  // repli degrade, jamais une exception.
    }
    return relative.generic_string();  // barres obliques, meme convention que "Skins/mur.png".
}

void MainWindow::updateLivePreview() {
    if (_pixelAssetPath.empty()) {
        return;  // asset pas encore enregistre une premiere fois : rien a montrer (TACHE-08).
    }
    if (!hmi::encodeImageFile(_pixelAssetPath, _pixelCanvas->image())) {
        return;  // echec silencieux : l'apercu live n'est pas une operation critique.
    }
    // Invalidation CIBLEE (LOT-40/LOT-43, TextureCache::invalidate) : regroupee par geste, puisque
    // imageChanged n'est emis qu'une fois par geste complet (TACHE-02/TACHE-03), jamais par pixel.
    _viewport->invalidateAsset(pixelAssetCacheKey());
}

void MainWindow::updatePixelColorButtonIcon(std::uint32_t color) {
    constexpr int SWATCH_SIZE = 20;
    QPixmap pixmap(SWATCH_SIZE, SWATCH_SIZE);
    pixmap.fill(QColor(static_cast<int>(color & 0xFFu), static_cast<int>((color >> 8) & 0xFFu),
                       static_cast<int>((color >> 16) & 0xFFu),
                       static_cast<int>((color >> 24) & 0xFFu)));
    _pixelColorButton->setIcon(QIcon(pixmap));
}

void MainWindow::openPixelColorPicker() {
    const std::uint32_t current = _pixelCanvas->currentColor();
    const QColor initial(
        static_cast<int>(current & 0xFFu), static_cast<int>((current >> 8) & 0xFFu),
        static_cast<int>((current >> 16) & 0xFFu), static_cast<int>((current >> 24) & 0xFFu));
    const QColor chosen = QColorDialog::getColor(initial, this, text("pixel.color_picker_title"),
                                                 QColorDialog::ShowAlphaChannel);
    if (!chosen.isValid()) {
        return;
    }
    const std::uint32_t packed = static_cast<std::uint32_t>(chosen.red()) |
                                 (static_cast<std::uint32_t>(chosen.green()) << 8) |
                                 (static_cast<std::uint32_t>(chosen.blue()) << 16) |
                                 (static_cast<std::uint32_t>(chosen.alpha()) << 24);
    _pixelCanvas->setCurrentColor(packed);
}

void MainWindow::openPixelAssetOpenDialog() {
    if (!confirmDiscardPixelChanges()) {
        return;
    }
    const QString directory =
        QString::fromStdString((hmi::executableDirectory() / "Assets").string());
    const QString path = QFileDialog::getOpenFileName(this, text("pixel.open_title"), directory,
                                                      QStringLiteral("PNG (*.png)"));
    if (path.isEmpty()) {
        return;
    }
    const std::filesystem::path fsPath(path.toStdString());
    const std::optional<hmi::DecodedImage> decoded = hmi::decodeImageFile(fsPath);
    if (!decoded) {
        showTransientStatusMessage(text("pixel.open_failed"), 5000);
        return;
    }
    _pixelCanvas->setImage(*decoded);
    _pixelCanvas->setAssetName(fsPath.filename().string());
    _pixelAssetPath = fsPath;
    refreshStatusHelp();
}

void MainWindow::openPixelAssetCreateDialog() {
    if (!confirmDiscardPixelChanges()) {
        return;
    }

    // Familles creables depuis l'atelier : Atlas exclu (fichier historique unique, jamais recree a
    // la main) et Font exclu (decoupe par ses metriques, hors perimetre d'un canevas generique).
    static constexpr std::array<hmi::AssetFamily, 5> FAMILIES{
        hmi::AssetFamily::TileSkin,       hmi::AssetFamily::AutotileSheet, hmi::AssetFamily::Object,
        hmi::AssetFamily::CharacterSheet, hmi::AssetFamily::Background,
    };

    QDialog dialog(this);
    dialog.setWindowTitle(text("pixel.create_title"));

    auto* const familyCombo = new QComboBox(&dialog);
    for (const hmi::AssetFamily family : FAMILIES) {
        familyCombo->addItem(QString::fromUtf8(hmi::assetFamilyName(family)));
    }
    auto* const sizeCombo = new QComboBox(&dialog);
    auto* const widthSpin = new QSpinBox(&dialog);
    widthSpin->setRange(1, 2048);
    auto* const heightSpin = new QSpinBox(&dialog);
    heightSpin->setRange(1, 2048);

    const auto refreshSizeControls = [&](int index) {
        if (index < 0) {
            return;
        }
        const std::vector<std::pair<int, int>> sizes =
            hmi::validAssetSizes(FAMILIES[static_cast<std::size_t>(index)]);
        sizeCombo->clear();
        for (const auto& [width, height] : sizes) {
            sizeCombo->addItem(QStringLiteral("%1 x %2").arg(width).arg(height));
        }
        const bool freeform = sizes.empty();
        sizeCombo->setVisible(!freeform);
        widthSpin->setVisible(freeform);
        heightSpin->setVisible(freeform);
    };
    connect(familyCombo, &QComboBox::currentIndexChanged, &dialog, refreshSizeControls);
    refreshSizeControls(0);

    auto* const form = new QFormLayout(&dialog);
    form->addRow(text("pixel.create_family"), familyCombo);
    form->addRow(text("pixel.create_size"), sizeCombo);
    form->addRow(widthSpin);
    form->addRow(heightSpin);
    auto* const buttons =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const hmi::AssetFamily chosenFamily =
        FAMILIES[static_cast<std::size_t>(familyCombo->currentIndex())];
    const std::vector<std::pair<int, int>> sizes = hmi::validAssetSizes(chosenFamily);
    int width = 0;
    int height = 0;
    if (!sizes.empty()) {
        const auto& [sizeWidth, sizeHeight] =
            sizes[static_cast<std::size_t>(sizeCombo->currentIndex())];
        width = sizeWidth;
        height = sizeHeight;
    } else {
        width = widthSpin->value();
        height = heightSpin->value();
    }

    hmi::DecodedImage image;
    image.width = width;
    image.height = height;
    image.pixels.assign(static_cast<std::size_t>(width) * static_cast<std::size_t>(height), 0u);
    _pixelCanvas->setImage(
        image);  // assetName/chemin restent vides : nouvel asset, pas encore enregistre.
    _pixelAssetPath.clear();
    refreshStatusHelp();
}

void MainWindow::savePixelAsset(bool saveAs) {
    std::filesystem::path target = _pixelAssetPath;
    if (saveAs || target.empty()) {
        const std::filesystem::path startDirectory =
            target.empty() ? hmi::executableDirectory() / "Assets" : target.parent_path();
        const QString path = QFileDialog::getSaveFileName(
            this, text("pixel.save_title"), QString::fromStdString(startDirectory.string()),
            QStringLiteral("PNG (*.png)"));
        if (path.isEmpty()) {
            return;
        }
        target = std::filesystem::path(path.toStdString());
    }

    // Garde-fou d'ecrasement (LOT-43) : un asset REFERENCE demande confirmation, en nommant les
    // references -- jamais silencieux (EX-EDIT-026). Un asset absent ou non reference s'ecrit sans
    // demander.
    std::error_code existsError;
    if (std::filesystem::exists(target, existsError)) {
        const std::vector<hmi::AssetReference> references =
            hmi::findSkinCatalogReferences(_viewport->skinCatalog(), target.filename().string());
        if (!references.empty()) {
            const QMessageBox::StandardButton answer = QMessageBox::question(
                this, text("pixel.overwrite_title"),
                text("pixel.overwrite_text")
                    .arg(QString::fromStdString(target.filename().string()),
                         QString::fromStdString(hmi::describeReferences(references))));
            if (answer != QMessageBox::Yes) {
                return;
            }
        }
    }

    if (!hmi::encodeImageFile(target, _pixelCanvas->image())) {
        showTransientStatusMessage(text("pixel.save_failed"), 5000);
        return;
    }
    _pixelAssetPath = target;
    _pixelCanvas->setAssetName(target.filename().string());
    _pixelCanvas->markSaved();
    // Invalidation CIBLEE du niveau (LOT-40/LOT-43/TACHE-08) : un seul asset a relire, jamais tout
    // le TextureCache. Les caches de vignettes des panneaux (Textures/Palette), eux,
    // n'exposent qu'un rechargement complet -- acceptable ici, sur un enregistrement explicite
    // plutot qu'a chaque geste (updateLivePreview, plus haut, ne les touche pas).
    _viewport->invalidateAsset(pixelAssetCacheKey());
    _textures->reloadAssets();
    _palette->clearThumbnailCache();
    _palette->refreshThumbnails(_viewport->renderMode(), _textures->currentSet());
    showTransientStatusMessage(
        text("pixel.save_done").arg(QString::fromStdString(target.filename().string())), 3000);
    refreshStatusHelp();
}

void MainWindow::syncPaletteToCanvas() {
    std::vector<std::uint32_t> colors;
    colors.reserve(_pixelPalette.entries().size());
    for (const hmi::PixelPaletteEntry& entry : _pixelPalette.entries()) {
        colors.push_back(entry.color);
    }
    _pixelCanvas->setPaletteColors(std::move(colors));
}

void MainWindow::savePixelPalette() {
    if (!_pixelPalette.saveToFile(hmi::executableDirectory() / "Assets" / "palettes.json")) {
        HMI_LOG_WARNING("Echec de l'enregistrement de la palette de projet (palettes.json).");
    }
}

void MainWindow::retranslateUi() {
    setWindowTitle(text("window.title"));

    // Panneaux dockables (les actions « toggle » du menu Affichage suivent le titre du dock).
    _ui->PalettePanel->setWindowTitle(text("dock.palette"));
    _ui->PlanesPanel->setWindowTitle(text("dock.planes"));
    _ui->LevelsPanel->setWindowTitle(text("dock.levels"));
    _ui->LinksPanel->setWindowTitle(text("dock.links"));
    _ui->TexturesPanel->setWindowTitle(text("dock.textures"));
    _ui->PixelCanvasPanel->setWindowTitle(text("dock.pixel_canvas"));
    _ui->PixelHistoryPanel->setWindowTitle(text("dock.pixel_history"));
    _ui->PixelPalettePanel->setWindowTitle(text("dock.pixel_palette"));
    _pixelColorButton->setToolTip(text("pixel.color_picker_title"));

    // Barre de menus, organisee par nature d'action (LOT-68).
    _ui->fileMenu->setTitle(text("menubar.file"));
    _ui->actMainMenu->setText(text("menubar.main_menu"));
    _ui->actQuit->setText(text("menubar.quit"));
    _ui->actResize->setText(text("menubar.resize"));
    _ui->editMenu->setTitle(text("menubar.edit"));
    _ui->levelMenu->setTitle(text("menubar.level"));
    _pixelMenu->setTitle(text("menubar.pixel"));
    _ui->helpMenu->setTitle(text("menubar.help"));
    _ui->viewMenu->setTitle(text("menubar.view"));
    _ui->workspaceMenu->setTitle(text("menubar.workspace"));
    _ui->actWorkspaceLevel->setText(text("menubar.workspace_level"));
    _ui->actWorkspacePlanes->setText(text("menubar.workspace_planes"));
    _ui->actWorkspacePixelArt->setText(text("menubar.workspace_pixel_art"));
    _ui->layersMenu->setTitle(text("menubar.layers"));
    _ui->panelsMenu->setTitle(text("menubar.panels"));
    _themeMenu->setTitle(text("menubar.theme"));
    _themeSystemAction->setText(text("menubar.theme_system"));
    _themeLightAction->setText(text("menubar.theme_light"));
    _themeDarkAction->setText(text("menubar.theme_dark"));
    _actFollowActiveTool->setText(text("menubar.follow_active_tool"));
    static constexpr std::array<const char*, 7> LAYER_ACTION_KEYS{
        "menubar.layer_background", "menubar.layer_plane_behind", "menubar.layer_shadow",
        "menubar.layer_tile_skin",  "menubar.layer_objects",      "menubar.layer_player",
        "menubar.layer_plane_front"};
    for (std::size_t i = 0; i < _layerVisibilityActions.size(); ++i) {
        _layerVisibilityActions[i]->setText(text(LAYER_ACTION_KEYS[i]));
    }
    _actShowAllLayers->setText(text("menubar.layer_show_all"));
    _ui->actResetLayout->setText(text("menubar.reset_layout"));
    _actions->retranslateUi(_loc);

    // Panneaux et pages (chacun retraduit son propre contenu depuis le catalogue).
    _menu->retranslateUi(_loc);
    _pauseScreen->retranslateUi(_loc);
    _credits->retranslateUi(_loc);
    _rpgScreens->retranslateUi(_loc);
    _options->retranslateUi(_loc);
    _palette->retranslateUi(_loc);
    _planes->retranslateUi(_loc);
    _levels->retranslateUi(_loc);
    _links->retranslateUi(_loc);
    _textures->retranslateUi(_loc);
    _pixelHistoryPanel->retranslateUi(_loc);
    _pixelPalettePanel->retranslateUi(_loc);

    // Recalcule la barre d'état dans la nouvelle langue (zones + aide) : un changement de langue ne
    // doit pas rester sur une aide figée dans l'ancienne (LOT-57 TACHE-01).
    refreshStatusHelp();
}

void MainWindow::changeLanguage(const QString& code) {
    if (code.toStdString() == _loc.activeLanguage()) {
        return;
    }
    if (!_loc.loadLanguage(code.toStdString())) {
        HMI_LOG_WARNING("Langue introuvable, conservee : " + code.toStdString());
        return;
    }
    QSettings().setValue(QStringLiteral("language"), code);
    HMI_LOG_INFO("Langue changee : " + code.toStdString());
    retranslateUi();
}

void MainWindow::saveSessionLogs() {
    if (_sessionLog == nullptr) {
        showTransientStatusMessage(text("options.logs_unavailable"), 5000);
        return;
    }
    // Nom de fichier horodaté, à côté de l'exécutable (dossier Logs créé au besoin).
    const std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_s(&local, &now);
    std::array<char, 32> stamp{};
    std::strftime(stamp.data(), stamp.size(), "%Y%m%d_%H%M%S", &local);
    const std::filesystem::path path =
        hmi::executableDirectory() / "Logs" / (std::string("session_") + stamp.data() + ".log");

    if (hmi::saveSessionLog(_sessionLog->entries(), path)) {
        HMI_LOG_INFO("Journaux de session enregistres : " + path.string());
        showTransientStatusMessage(
            text("options.logs_saved").arg(QString::fromStdString(path.filename().string())), 5000);
    } else {
        HMI_LOG_ERROR("Echec de l'enregistrement des journaux : " + path.string());
        showTransientStatusMessage(text("options.logs_failed"), 5000);
    }
}

}  // namespace hmi
