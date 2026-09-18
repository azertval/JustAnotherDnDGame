// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <QByteArray>
#include <QMainWindow>
#include <array>
#include <memory>
#include <optional>

#include "HMI/Editor/EditContextTarget.h"
#include "HMI/Editor/EditorTool.h"
#include "HMI/Editor/PanelFocus.h"
#include "HMI/Localization/Localization.h"

class QAction;
class QLabel;
class QTimer;
class QToolBar;

namespace Ui {
class EditorMainWindow;
}

/**
 * @file HMI/Interface/MainWindow.h
 * @brief Fenêtre de l'éditeur de cartes (`LevelEditor`) : le canevas au centre, les panneaux
 *        d'édition en docks.
 */

namespace hmi {

class EditorActions;
class EditorViewport;
class PalettePanel;
class LevelBrowserPanel;
class LayersPanel;
class EntityPanel;
struct EditorReferences;

/**
 * @brief Fenêtre principale de l'éditeur (Qt Widgets).
 *
 * Le canevas (`hmi::EditorViewport`) est le widget central ; la palette, le navigateur de cartes,
 * les couches et les entités sont des docks détachables dont la disposition est persistée
 * (`EX-IHM-010`/`011`). La fenêtre ne possède aucune donnée d'édition : le canevas est le seul
 * propriétaire du brouillon, les panneaux demandent et il applique.
 */
class MainWindow : public QMainWindow {
public:
    MainWindow();
    ~MainWindow() override;
    MainWindow(const MainWindow&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void buildUi();
    void connectMapPanels();
    void connectToolActions();
    void connectEditorCommands();
    void buildThemeMenu();
    void buildViewMenu();
    void buildStatusBar();
    void reloadEditorReferences();
    void restoreLayout();
    void saveLayout();
    void openResizeDialog();
    void openShortcutsDialog();
    void retranslateUi();
    void refreshStatusHelp();
    void showTransientStatusMessage(const QString& message, int timeoutMs);
    void applyPanelFocus(hmi::EditorTool tool);
    [[nodiscard]] QString text(const char* key) const;

    std::unique_ptr<Ui::EditorMainWindow> _ui;
    EditorViewport* _viewport;  ///< Canevas (possédé par la fenêtre, widget central).
    EditContextTarget* _editContext = nullptr;
    PalettePanel* _palette = nullptr;
    LevelBrowserPanel* _levels = nullptr;
    LayersPanel* _layers = nullptr;
    EntityPanel* _entities = nullptr;
    std::unique_ptr<EditorReferences> _references;
    EditorActions* _actions = nullptr;
    QToolBar* _toolBar = nullptr;
    QByteArray _defaultState;  ///< Disposition par défaut (pour « Réinitialiser la disposition »).
    QAction* _actFollowActiveTool = nullptr;  ///< Réglage persisté (menu Affichage).
    /// L'utilisateur a choisi un onglet lui-même : la mise en avant automatique s'efface.
    bool _userPickedTab = false;
    bool _suppressPanelFocusTracking = false;
    std::array<QLabel*, 5> _statusZones{};
    QTimer* _statusMessageTimer = nullptr;
    Localization _loc;  ///< Catalogue de traduction (i18n), source de tous les textes.
};

}  // namespace hmi
