// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QByteArray>
#include <QHash>
#include <QMainWindow>
#include <array>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>

#include "HMI/Audio/AudioEngine.h"
#include "HMI/Audio/SoundCatalog.h"
#include "HMI/Editor/EditContextTarget.h"
#include "HMI/Editor/EditorTool.h"
#include "HMI/Editor/PanelFocus.h"
#include "HMI/Editor/PixelPalette.h"
#include "HMI/Editor/PixelTool.h"
#include "HMI/Game/GameEvents.h"
#include "HMI/Input/GamepadPoller.h"
#include "HMI/Input/InputState.h"
#include "HMI/Interface/EditorWorkspace.h"
#include "HMI/Localization/Localization.h"
#include "HMI/Presentation/RpgScreens.h"
#include "HMI/Presentation/ScreenFlow.h"

/**
 * @file HMI/Interface/MainWindow.h
 * @brief Fenêtre principale de l'éditeur Qt : viewport central + panneaux dockables (LOT-35).
 */

class QAction;
class QLabel;
class QMenu;
class QStackedWidget;
class QTimer;
class QToolBar;
class QToolButton;
class QWidget;

namespace Ui {
class EditorMainWindow;
}

namespace core {
class MemoryLogSink;
}

namespace hmi {

class EditorActions;
class GameViewport;
class MainMenu;
class OptionsPage;
class PauseScreen;
class CreditsScreen;
class RpgScreenHost;
class PalettePanel;
class PlanesPanel;
class LevelBrowserPanel;
class LinkPanel;
class TexturePanel;
class PixelCanvas;
class PixelHistoryPanel;
class PixelPalettePanel;

/**
 * @brief Fenêtre principale de l'application Qt : **poste de travail d'éditeur** à panneaux
 *        dockables autour du viewport Direct3D 11.
 *
 * Le **widget central** est le viewport (`GameViewport`, LOT-34), non dockable. Autour, des
 * `QDockWidget` (Palette, Outils, Statut) **déplaçables, redimensionnables, détachables**
 * (`EX-IHM-010`). La **disposition** (géométrie + agencement des docks) est **persistée hors code**
 * via `QSettings` (`EX-IHM-011`) : restaurée au lancement, sauvegardée à la fermeture, et
 * réinitialisable à une disposition par défaut (menu « Affichage »).
 *
 * À cette tâche (LOT-35 TACHE-01), les panneaux sont des **coquilles** : leur contenu (arbre de
 * palette, barre d'outils, barre d'état) est rempli aux tâches suivantes.
 */
class MainWindow : public QMainWindow {
public:
    /// @param sessionLog Sink mémoire des logs (build développement) pour « Enregistrer les
    ///                   journaux » ; `nullptr` en Release (bouton indisponible).
    explicit MainWindow(core::MemoryLogSink* sessionLog = nullptr);
    ~MainWindow() override;

protected:
    /// Sauvegarde la disposition avant fermeture.
    void closeEvent(QCloseEvent* event) override;
    /// Resynchronise la géométrie des recouvrements d'écran (`_pauseScreen`/`_levelCompleteScreen`,
    /// `LOT-59` TACHE-02/03) -- fenêtres de haut niveau positionnées à la main, jamais
    /// redimensionnées automatiquement par `_stack`.
    void resizeEvent(QResizeEvent* event) override;
    /// Recalcule le facteur d'agrandissement des écrans du jeu depuis la hauteur courante, **borné
    /// par la zone d'affichage disponible** (`LOT-68`/`LOT-73`, `EX-IHM-070`, `EX-IHM-081`), et
    /// rejoue le thème s'il a changé. Sans effet sur le châssis d'édition, dont les grandeurs ne
    /// sont jamais multipliées.
    /// @param beforeFirstShow `true` pendant la construction, alors que la fenêtre n'est pas encore
    ///        montrée : lève la garde d'invisibilité (qui rendait sinon l'appel sans effet) et
    ///        rejoue le thème **sans différer**, pour que la première image soit peinte au bon
    ///        facteur.
    void applyIdentityScale(bool beforeFirstShow = false);
    /// Pose la feuille de style de la portée identité sur la **pile d'écrans** (`LOT-73`,
    /// `EX-IHM-082`). Jamais sur l'application : les panneaux, barres et docks n'appartiennent pas
    /// à cette portée, et les repolir à chaque changement de facteur coûtait cinq secondes en
    /// Debug pour un résultat identique.
    void applyIdentityStyleSheet();

private:
    /// Applique l'espace de travail @p workspace (`LOT-68`, `EX-IHM-073`) : masque les panneaux de
    /// l'autre espace, bascule barre d'outils et menu, restaure la disposition propre à cet espace.
    /// N'enregistre **pas** la disposition qu'on quitte — c'est `switchToWorkspace` qui sait qu'on
    /// quitte vraiment un espace, et l'initialisation n'a rien à enregistrer.
    void applyWorkspace(EditorWorkspace workspace);

    /// Bascule sur @p workspace **s'il n'est pas déjà actif**, en synchronisant le sélecteur du
    /// menu sans réémettre. Point d'entrée unique : `applyWorkspace` applique, celle-ci décide.
    void switchToWorkspace(EditorWorkspace workspace);

    /// @return La clé `QSettings` de la disposition de @p workspace. Chaque espace persiste la
    /// sienne : une disposition unique rouvrirait les docks de l'atelier par-dessus l'édition.
    [[nodiscard]] static QString layoutKeyFor(EditorWorkspace workspace);
    /// @return Le nom sous lequel @p workspace est persisté. Un **nom**, jamais l'indice de
    ///         l'énumération : le `LOT-68` écrivait 0/1, et insérer « Plans » entre les deux
    ///         aurait fait rouvrir en mode création l'éditeur laissé dans l'atelier.
    [[nodiscard]] static QString workspaceSettingsName(EditorWorkspace workspace);
    /// @return L'espace nommé @p name ; l'édition de niveau si le nom est inconnu (y compris les
    ///         anciens `0`/`1` du `LOT-68`).
    [[nodiscard]] static EditorWorkspace workspaceFromSettingsName(const QString& name);
    /// @return L'entrée de menu exclusive qui sélectionne @p workspace.
    [[nodiscard]] QAction* workspaceSelector(EditorWorkspace workspace) const;

    /// @return La table des neuf panneaux et de leur identité, relue par `setDocksVisible` **et**
    /// par `applyWorkspace`. Une seule table : deux listes divergeraient au premier dock ajouté, et
    /// le dock oublié resterait affiché dans les deux espaces.
    [[nodiscard]] std::array<std::pair<QDockWidget*, hmi::PanelId>, hmi::PANEL_COUNT>
    workspacePanels() const;

protected:
    void moveEvent(QMoveEvent* event) override;
    /// Suit la taille du viewport pour les recouvrements qui s'y superposent (pause, fin de
    /// niveau) : enfants ordinaires depuis le `LOT-69` TACHE-02, ils se redimensionnent avec leur
    /// parent plutôt que d'être repositionnés en coordonnées écran.
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    /// Branche le panneau « Plans » (`LOT-69` TACHE-08) : il demande, le viewport applique.
    void connectPlanesPanel();
    /// @return Le dossier des images de plans, à côté des niveaux.
    [[nodiscard]] std::filesystem::path planesDirectory() const;
    /// Crée un plan : un PNG entièrement transparent aux dimensions exactes, puis l'entrée dans le
    /// brouillon. Les deux vont ensemble — une entrée sans fichier afficherait un damier.
    void createPlane();
    /// Change la densité d'un plan : **rééchantillonne l'image** puis met à jour la déclaration,
    /// sans quoi le fichier et le format diraient deux choses différentes.
    void changePlaneDensity(std::size_t index, int pixelsPerUnit);

    /// Crée les panneaux (contenu des docks du `.ui`) et branche les actions de la barre de menus.
    void buildUi();
    void restoreLayout();
    void saveLayout();
    /// Ouvre la boîte de dialogue de redimensionnement du niveau (avec confirmation si
    /// destructeur).
    void openResizeDialog();

    /// Ouvre l'aperçu des raccourcis (`ShortcutsDialog.ui`). Le tableau est rempli depuis les
    /// raccourcis **effectifs** des actions, jamais un texte figé : il reste juste après un
    /// remappage.
    void openShortcutsDialog();

    /// Applique la langue active à tous les textes de l'IHM (fenêtre, menus, docks, panneaux).
    void retranslateUi();
    /// Recalcule et réaffiche la barre d'état (zones permanentes + aide contextuelle) depuis l'état
    /// courant du viewport (`LOT-57` TACHE-01, `hmi::editorStatusLines`) — seule voie de mise à
    /// jour, appelée par tout changement pertinent (outil, survol, zoom, brouillon) ainsi qu'à
    /// l'expiration d'un message transitoire, pour la restaurer.
    void refreshStatusHelp();
    /// Affiche @p message pour @p timeoutMs dans la barre d'état, puis restaure l'aide contextuelle
    /// (`refreshStatusHelp`). La barre d'état n'est jamais laissée vide : à l'expiration d'un
    /// message transitoire, elle reprend l'aide du contexte courant. C'est pourquoi le minuteur est
    /// porté ici et non délégué à `QStatusBar::showMessage`, dont l'expiration ne rappelle rien.
    void showTransientStatusMessage(const QString& message, int timeoutMs);
    /// Met en avant le panneau associé à @p tool (`hmi::panelForTool`), si le réglage est actif et
    /// que l'utilisateur n'a rien imposé lui-même (`LOT-57` TACHE-02) — jamais un masquage, une
    /// simple suggestion de premier plan parmi les onglets regroupés.
    void applyPanelFocus(hmi::EditorTool tool);
    /// Équivalent pour les outils du canevas pixel art (`hmi::panelForPixelTool`, `LOT-54`
    /// TACHE-04) — même garde, même règle de non-masquage.
    void applyPixelPanelFocus(hmi::PixelTool tool);
    /// Met en avant @p panel (résolution `PanelId` -> `QDockWidget*`), sans jamais voler le focus
    /// clavier — factorisé entre `applyPanelFocus` et `applyPixelPanelFocus`.
    void raisePanel(hmi::PanelId panel);
    /// Réassigne le contexte d'édition actif (`_editContext`) et le contexte de la barre d'état
    /// selon le widget qui vient de recevoir le focus clavier (`LOT-54` TACHE-04, `EX-IHM-062`) :
    /// le canevas pixel art si le focus y entre, le niveau sinon.
    void updateActiveEditContext(QWidget* focused);

    // Atelier pixel art : ouvrir/créer/enregistrer (LOT-54 TACHE-05).
    /// @return `true` si l'on peut poursuivre (rien à perdre, ou perte confirmée) — même patron que
    ///         le garde-fou d'ouverture de niveau (`EX-EDIT-021`).
    [[nodiscard]] bool confirmDiscardPixelChanges();
    /// Ouvre un asset existant choisi par l'utilisateur (bibliothèque `Assets/`), après le
    /// garde-fou de perte de travail.
    void openPixelAssetOpenDialog();
    /// Crée un nouvel asset à une taille choisie parmi celles admises par le contrat de sa famille
    /// (`hmi::validAssetSizes`), après le garde-fou de perte de travail.
    void openPixelAssetCreateDialog();
    /// Enregistre l'asset ouvert. @p saveAs force le choix d'un nouveau chemin (copie), même sans
    /// chemin existant ; sans @p saveAs, réutilise le chemin d'ouverture s'il y en a un. Un
    /// écrasement d'asset référencé demande confirmation, nommant les références (`LOT-43`).
    void savePixelAsset(bool saveAs);

    // Aperçu live et mode planche à raccords (LOT-54 TACHE-08).
    /// Nom logique de l'asset ouvert, relatif à `Assets/` (ex. `"Skins/mur.png"`) : la clé sous
    /// laquelle `hmi::TextureCache`/`GameViewport::invalidateAsset` connaissent cet asset — un
    /// simple nom de fichier ne suffit pas, le cache est indexé par chemin complet (préfixe de
    /// sous-dossier compris).
    [[nodiscard]] std::string pixelAssetCacheKey() const;
    /// Écrit l'image en cours sur `_pixelAssetPath` et invalide son entrée de `TextureCache`, pour
    /// que le niveau affiché reflète l'édition en cours — sans effet tant qu'aucun chemin n'est
    /// associé (asset pas encore enregistré une première fois, TACHE-05).
    void updateLivePreview();

    // Mode création : peinture d'un plan pictural dans le canevas (LOT-69 TACHE-08).
    /// Charge le PNG du plan @p index dans `_pixelCanvas`, avec ses repères (pelure d'oignon des
    /// tuiles sous le plan, plans voisins aplatis dessous et dessus, grille de tuiles). Sans effet
    /// — et le canevas est refermé — si le rang n'existe pas. Le garde-fou de perte de travail est
    /// à la charge de l'appelant : cette méthode écrase le contenu du canevas.
    void loadPlaneIntoCanvas(std::size_t index);
    /// Écrit l'image du canevas dans le PNG du plan ouvert (`Ctrl+S` dans l'espace « Plans »).
    /// Sans effet si aucun plan n'y est chargé.
    void savePlaneImage();
    /// Vide le canevas de son plan et retire les repères — l'atelier pixel art retrouve alors son
    /// comportement du `LOT-54`, sans repère hérité du mode création.
    void closePlaneInCanvas();
    /// Reconstruit les repères du plan ouvert depuis le brouillon courant : appelé après toute
    /// mutation qui les périme (ordre, densité, opacité, visibilité, taille du niveau).
    void refreshPlaneReferences();
    /// @return `true` si l'on peut poursuivre : rien à perdre dans le canevas, ou perte confirmée.
    ///         Même garde que `confirmDiscardPixelChanges`, message propre au plan — un plan perdu
    ///         est un dessin perdu, pas un asset qu'on retrouve dans la bibliothèque.
    [[nodiscard]] bool confirmDiscardPlaneChanges();
    /// @return Le chemin du PNG du plan de rang @p index, vide si le rang n'existe pas.
    [[nodiscard]] std::filesystem::path planeFilePath(std::size_t index) const;

    /// Ouvre un sélecteur de couleur (`QColorDialog`) pour choisir librement la couleur courante du
    /// canevas — seul moyen d'atteindre une couleur absente à la fois de l'image ouverte (pipette)
    /// et de la palette de projet (pastilles).
    void openPixelColorPicker();
    /// Met à jour le témoin de couleur de la barre d'outils du canevas (pastille de
    /// `_pixelColorButton`).
    void updatePixelColorButtonIcon(std::uint32_t color);

    // Palette de projet (LOT-54 TACHE-07).
    /// Recopie les couleurs de `_pixelPalette` vers `_pixelCanvas` (mode contraint) — à appeler
    /// après toute mutation qui change les couleurs ou leur ordre (l'ordre affecte le départage à
    /// distance égale, `hmi::nearestPaletteColor`).
    void syncPaletteToCanvas();
    /// Enregistre `_pixelPalette` dans `Assets/palettes.json`, journalise un échec éventuel.
    void savePixelPalette();
    /// Change la langue active (recharge le catalogue, persiste, retraduit tout).
    void changeLanguage(const QString& code);
    /// Enregistre les journaux de session accumulés dans un fichier horodaté.
    void saveSessionLogs();
    /// Raccourci : texte localisé d'une clé, en `QString`.
    [[nodiscard]] QString text(const char* key) const;

    /// Affiche le menu principal (docks et barre de menu masqués) ; rafraîchit l'état de
    /// « Continuer » (`MainMenu::setContinueEnabled`, `LOT-59` TACHE-06) à chaque affichage --
    /// seule voie de mise à jour, jamais suivi à part.
    void showMenu();
    /// Affiche l'éditeur (viewport + docks + barre de menu).
    void showEditor();
    /// Affiche la page Options (onglets) dans la fenêtre, revient à l'écran d'où elle a été
    /// ouverte (Menu ou Pause, `EX-GP-041`).
    void showOptions();
    /// Ferme la page Options, retour à l'écran d'où elle a été ouverte (`ScreenState::
    /// optionsReturnTo`) -- remplace l'ancien retour direct et systématique vers le menu.
    void closeOptions();
    /// Montre/masque tous les panneaux dockables.
    void setDocksVisible(bool visible);

    /// Résout @p event via `hmi::resolveTransition` depuis l'écran courant (`_screenState`) et
    /// applique l'habillage du nouvel écran (`applyScreenDressing`) -- @return `false` sans effet
    /// si la transition est refusée (`EX-GP-041`), auquel cas l'appelant ne doit rien faire
    /// d'autre (aucun chargement, aucun log de navigation).
    bool transitionScreen(hmi::ScreenEvent event);
    /// Bascule la page du `QStackedWidget` (seule part propre à Qt, hors de portée d'une table
    /// pure) puis applique l'habillage générique de @p screen (`hmi::dressingFor`) : docks,
    /// barres, commandes d'édition, navigation manette, minuteur de statut.
    void applyScreenDressing(hmi::ScreenId screen);

    // Invariant de taille des écrans (`LOT-73`, `EX-IHM-080`).
    /// Enveloppe @p page dans un `hmi::ScreenPageHost` défilant, l'ajoute à la pile et retient
    /// l'association. **Tout** écran passe par ici : c'est ce passage obligé, et non une convention
    /// à réappliquer dans chaque `.ui`, qui garantit qu'aucun écran ne dicte sa taille à la
    /// fenêtre. Le viewport en est exclu — c'est une surface de rendu, qui remplit sans défiler.
    void addScreenPage(QWidget* page);
    /// Affiche l'écran @p page en sélectionnant l'enveloppe qui l'héberge.
    void showScreenPage(QWidget* page);
    /// Vérifie que la taille minimale imposée par les écrans tient dans la zone d'affichage
    /// disponible, et avertit dans le journal sinon (`EX-IHM-080`). Le dépassement est
    /// silencieux côté Qt : Windows refuse la géométrie sans que rien ne le signale.
    void warnIfScreensConstrainWindow() const;
    /// Hauteur utile, en pixels logiques, de l'écran hébergeant la fenêtre ; `0` si inconnue.
    [[nodiscard]] int availableLogicalHeight() const;

    // Écran de pause (LOT-59 TACHE-02).
    /// `Échap`/bouton manette B en jeu réel (`GameViewport::pauseRequested`) : ouvre la pause.
    void openPause();
    /// « Reprendre » (bouton, `Échap`, ou B manette depuis la pause) : reprend la simulation.
    void resumeFromPause();
    /// « Quitter vers le menu » depuis la pause : demande confirmation (la partie en cours est
    /// perdue), puis abandonne la partie si confirmé.
    void quitPauseToMenu();

    /// Joue le son associé à @p event (`hmi::SoundTriggers`), sans effet si aucun son ne lui est
    /// associé (`LOT-60` TACHE-03) -- point d'appel unique pour tous les sons d'interface.
    void playInterfaceSound(GameEvent event);

    /// « Nouvelle partie » (menu) : ouvre la carte de départ. Aucune confirmation d'écrasement —
    /// elle protégeait une progression de campagne qui n'existe plus, et reviendra avec la
    /// sauvegarde du `LOT-17`, qui aura quelque chose à écraser.
    void newGame();
    /// Ouvre le châssis des écrans du RPG sur @p screen (`LOT-68`, `EX-IHM-090`). Applique la
    /// règle de superposition de l'écran ouvert (`hmi::pausesGame`, `EX-IHM-091`) : la simulation
    /// est suspendue par la fiche ou un dialogue, laissée courir par la carte.
    void openRpgScreen(hmi::RpgScreenId screen);
    /// Ferme le châssis des écrans du RPG et revient à l'écran d'où il a été ouvert
    /// (`ScreenState::rpgReturnTo`), en reprenant la simulation si elle avait été suspendue.
    void closeRpgScreen();
    /// Charge les catalogues RPG et le personnage de démonstration, puis pose ses valeurs sur
    /// l'écran de fiche (`LOT-38`). Sans effet visible si les données manquent : l'écran garde ses
    /// tirets, ce qui est la vérité (`EX-NFR-040`).
    void loadDemonstrationCharacter();

    /// Applique la règle de superposition de @p screen à la simulation, sans changer d'écran :
    /// appelé aussi lors du **passage** d'un écran du RPG à un autre, où la règle peut changer.
    void applyRpgSuperposition(hmi::RpgScreenId screen);
    /// « Crédits » (menu) : ouvre `_credits` (`LOT-60`).
    void openCredits();
    /// Retour au menu depuis l'écran de crédits.
    void closeCredits();
    /// Traduit la manette en navigation de focus Qt (menus/options) : appelé par `_menuNavTimer`.
    void pollMenuGamepad();
    /// Active/désactive la navigation manette des menus (inactive en jeu/édition).
    void setMenuGamepadActive(bool active);

    std::unique_ptr<Ui::EditorMainWindow> _ui;  ///< Mise en page (MainWindow.ui : menubar + docks).
    /// Écran courant et écran de retour d'Options (`LOT-59` TACHE-01, `EX-GP-041`) : seule source
    /// de vérité sur la navigation, mise à jour uniquement par `transitionScreen`.
    hmi::ScreenState _screenState;
    QStackedWidget* _stack;  ///< Central : empile menu principal, options et viewport.
    /// Écran → enveloppe défilante qui l'héberge dans la pile (`LOT-73`, `EX-IHM-080`). Le
    /// viewport n'y figure pas : il est ajouté tel quel.
    QHash<QWidget*, QWidget*> _screenHosts;
    MainMenu* _menu;        ///< Menu principal (page d'accueil).
    OptionsPage* _options;  ///< Page Options à onglets.
    /// Recouvrement de pause (`LOT-59` TACHE-02) : **widget enfant ordinaire** du viewport
    /// depuis le `LOT-69` TACHE-02. Il fut une fenêtre de haut niveau tant que le viewport était
    /// une fenêtre native embarquée (`QWidget::createWindowContainer`), qui peignait toujours
    /// par-dessus ses frères Qt : le portage sur `QRhiWidget` efface cette contrainte, et avec
    /// elle la géométrie synchronisée en coordonnées écran. Visibilité pilotée par
    /// `applyScreenDressing`, géométrie suivie par le filtre d'événements posé sur le viewport.
    PauseScreen* _pauseScreen = nullptr;
    /// Écran de crédits (`LOT-60`) : une page normale de `_stack`, jamais un recouvrement —
    /// atteint depuis le menu, pas en jeu, contrairement au précédent.
    CreditsScreen* _credits = nullptr;
    /// Les huit écrans du RPG et la navigation entre eux (`LOT-68`) : **une** page de `_stack`,
    /// jamais huit -- c'est ce qui permet d'aller de la fiche au journal sans que la machine à
    /// états d'écrans ait à connaître les huit.
    RpgScreenHost* _rpgScreens = nullptr;
    /// Écran Mode IA (`LOT-ANNEXE-21`) : même patron que `_levelSelectScreen`, page normale de
    /// `_stack`.
    GameViewport* _viewport;  ///< Surface de rendu D3D11 (possédée par le conteneur central).
    /// Contexte d'édition actif, cible d'Annuler/Refaire/Copier/Coller (`LOT-57` TACHE-04) : `
    /// _viewport` (niveau) ou `_pixelCanvas` (atelier pixel art, `LOT-54` TACHE-04), selon le
    /// widget qui a le focus clavier (`updateActiveEditContext`) — le dispatch lui-même ne change
    /// jamais.
    EditContextTarget* _editContext = nullptr;
    PalettePanel* _palette;  ///< Arbre de sélection du type de tuile (contenu du dock Palette).
    PlanesPanel* _planes =
        nullptr;  ///< Liste des plans picturaux (`LOT-69`, contenu du dock Plans).
    LevelBrowserPanel*
        _levels;  ///< Liste/gestion des fichiers de niveaux (contenu du dock Niveaux).
    /// Placement/inspection de décors (dock Décors, `LOT-57` amendement) — contenait déjà tout ce
    /// qui concerne les décors (`ToolPanel`, `LOT-56` TACHE-04) avant d'y accueillir aussi
    /// l'inspecteur déplacé du panneau Textures. La barre d'outils reste hors de ce panneau.
    LinkPanel* _links;        ///< Liste/gestion des liaisons de mécanismes (dock Liens, LOT-37).
    TexturePanel* _textures;  ///< Habillage : jeu de skins et assignations (dock Textures, LOT-42).
    /// Réglages de gameplay de l'élément sélectionné et du tableau (dock Propriétés, `LOT-67`).
    /// Canevas de l'atelier pixel art (dock Atelier, LOT-54 TACHE-04) : seconde implémentation de
    /// `EditContextTarget`, cible d'Annuler/Refaire/Copier/Coller quand elle a le focus clavier.
    PixelCanvas* _pixelCanvas;
    PixelHistoryPanel*
        _pixelHistoryPanel;  ///< Historique visuel de l'atelier (dock, LOT-54 TACHE-04).
    PixelPalettePanel*
        _pixelPalettePanel;  ///< Édition de la palette de projet (dock, LOT-54 TACHE-07).
    PixelPalette
        _pixelPalette;  ///< Palette de projet, chargée/enregistrée dans Assets/palettes.json.
    /// Chemin complet du fichier de l'asset ouvert dans l'atelier, vide si aucun ou pas encore
    /// enregistré (LOT-54 TACHE-05) — `PixelCanvas::assetName()` n'en garde que le nom de fichier,
    /// pour l'affichage ; ce chemin sert à `savePixelAsset` pour retrouver le dossier.
    std::filesystem::path _pixelAssetPath;
    /// Rang du plan actuellement peint dans `_pixelCanvas` (mode création, `LOT-69` TACHE-08) ;
    /// absent dès que le canevas sert à l'atelier pixel art. C'est ce qui distingue les deux
    /// usages du **même** canevas : où `Ctrl+S` écrit, quels repères afficher, et ce que la barre
    /// d'état annonce. Un plan et un asset n'y sont jamais ouverts en même temps.
    std::optional<std::size_t> _paintedPlane;
    EditorActions*
        _actions;  ///< Outils et commandes principales, barre d'outils (LOT-56 TACHE-04).
    /// Espace de travail actif (`LOT-68`). Persisté : on rouvre l'éditeur là où on l'a laissé.
    /// Vrai dès que la fenêtre a reçu sa demande de fermeture. Coupe les traitements différés qui
    /// toucheraient au thème ou à la disposition pendant le démontage.
    bool _closing = false;
    EditorWorkspace _workspace = EditorWorkspace::Level;
    QToolBar* _toolBar;       ///< Barre d'outils de l'éditeur, alimentée par `_actions`.
    QToolBar* _pixelToolBar;  ///< Barre d'outils du canevas pixel art (LOT-54 TACHE-04).
    QToolButton* _pixelColorButton =
        nullptr;  ///< Témoin + sélecteur de couleur courante (canevas).
    QMenu* _pixelMenu =
        nullptr;        ///< Menu « Atelier » : ouvrir/créer/enregistrer (LOT-54 TACHE-05).
    QMenu* _themeMenu;  ///< Sous-menu Affichage > Thème (LOT-56 TACHE-06).
    QAction* _themeSystemAction;
    QAction* _themeLightAction;
    QAction* _themeDarkAction;
    QByteArray _defaultState;  ///< Disposition par défaut (pour « Réinitialiser la disposition »).

    // Regroupement des panneaux de droite en onglets, suivant l'outil actif (LOT-57 TACHE-02).
    QAction* _actFollowActiveTool = nullptr;  ///< Réglage persisté (menu Affichage).
    /// `true` dès que l'utilisateur a choisi un onglet ou déplacé un panneau lui-même : la mise en
    /// avant automatique cesse alors pour la session (jamais persisté, cf. `hmi::panelForTool`).
    bool _userPickedTab = false;

    // Mode d'inspection par calque, déplacé du panneau Textures vers le menu Affichage
    // (LOT-57 TACHE-03) : une entrée cochable par calque, dans l'ordre de dessin, plus « tout
    // afficher ».
    std::array<QAction*, 7> _layerVisibilityActions{};
    QAction* _actShowAllLayers = nullptr;
    /// `true` pendant un changement de visibilité **provoqué par le code** (mise en avant
    /// automatique, bascule de mode, restauration de disposition) : évite qu'un tel changement soit
    /// pris pour un choix explicite de l'utilisateur (`QDockWidget::visibilityChanged`).
    bool _suppressPanelFocusTracking = false;

    // Barre d'état structurée (LOT-57 TACHE-01) : zones permanentes (widgets ajoutés via
    // addPermanentWidget, jamais recouvertes par un message transitoire), dans l'ordre décidé par
    // hmi::editorStatusLines.
    QLabel* _statusLevel = nullptr;
    QLabel* _statusDirty = nullptr;
    QLabel* _statusTool = nullptr;
    QLabel* _statusHover = nullptr;
    QLabel* _statusZoom = nullptr;
    /// Couleur courante de l'atelier pixel art (`LOT-54` TACHE-04) ; vide hors contexte d'atelier.
    QLabel* _statusColor = nullptr;
    /// Mode de cadrage de caméra du niveau courant (`EX-EDIT-028`, LOT-64) ; vide hors contexte de
    /// niveau.
    QLabel* _statusCameraFraming = nullptr;
    /// Restaure l'aide contextuelle à l'expiration d'un message transitoire
    /// (`showTransientStatusMessage`).
    QTimer* _statusMessageTimer = nullptr;

    Localization _loc;  ///< Catalogue de traduction (i18n), source de tous les textes.
    /// Moteur audio (`LOT-60`) : ouvert au démarrage (`EX-REN-047`), dégrade en silence sans
    /// périphérique (`EX-NFR-040`). Partagé avec `_viewport` (`setAudioEngine`), qui déclenche les
    /// sons de jeu ; les sons d'interface se jouent directement d'ici (`playInterfaceSound`).
    hmi::AudioEngine _audio;
    /// Catalogue de sons (`LOT-60` TACHE-02), lu une fois au démarrage depuis `Audio/sounds.json`
    /// et entièrement préchargé dans `_audio`.
    hmi::SoundCatalog _sounds;
    core::MemoryLogSink* _sessionLog;  ///< Sink mémoire des logs (nul en Release).

    // Navigation manette des menus (hors jeu) : sondage périodique -> événements clavier Qt.
    GamepadPoller _menuPad;
    InputState _menuPadInput;
    QTimer* _menuNavTimer = nullptr;
};

}  // namespace hmi
