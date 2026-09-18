// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <QByteArray>
#include <QMainWindow>
#include <array>
#include <memory>
#include <optional>

#include "Editor/Logic/EditContextTarget.h"
#include "Editor/Logic/EditorTool.h"
#include "Editor/Logic/PanelFocus.h"

class QAction;
class QDockWidget;
class QLabel;
class QMenu;
class QTimer;
class QToolBar;

/**
 * @file Editor/Ui/MainWindow.h
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
 * @brief Fenêtre principale de l'éditeur (Qt Widgets, style Fusion, textes anglais).
 *
 * Le canevas (`hmi::EditorViewport`) est le widget central ; la palette, le navigateur de cartes,
 * les couches et les entités sont des docks détachables dont la disposition est persistée
 * (`EX-IHM-011`). Tout est construit en code (`LOT-EDITOR-01`). La fenêtre ne possède aucune donnée
 * d'édition : le canevas est le seul propriétaire du brouillon, les panneaux demandent et il
 * applique.
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
    [[nodiscard]] QDockWidget* addPanel(const QString& objectName, const QString& title,
                                        QWidget* content, Qt::DockWidgetArea area);
    void buildMenus();
    void connectMapPanels();
    void connectToolActions();
    void connectEditorCommands();
    void buildStatusBar();
    void reloadEditorReferences();
    void restoreLayout();
    void saveLayout();
    void openResizeDialog();
    void openShortcutsDialog();
    void refreshStatusHelp();
    void showTransientStatusMessage(const QString& message, int timeoutMs);
    void applyPanelFocus(hmi::EditorTool tool);
    [[nodiscard]] QDockWidget* dockFor(PanelId panel) const;

    EditorViewport* _viewport;  ///< Canevas (possédé par la fenêtre, widget central).
    EditContextTarget* _editContext = nullptr;
    PalettePanel* _palette = nullptr;
    LevelBrowserPanel* _levels = nullptr;
    LayersPanel* _layers = nullptr;
    EntityPanel* _entities = nullptr;
    std::array<QDockWidget*, PANEL_COUNT> _docks{};  ///< Dans l'ordre de `PanelId`.
    std::unique_ptr<EditorReferences> _references;
    EditorActions* _actions = nullptr;
    QToolBar* _toolBar = nullptr;
    QAction* _resizeAction = nullptr;
    QAction* _resetLayoutAction = nullptr;
    QByteArray _defaultState;                 ///< Disposition par défaut (pour « Reset layout »).
    QAction* _actFollowActiveTool = nullptr;  ///< Réglage persisté (menu View).
    /// L'utilisateur a choisi un onglet lui-même : la mise en avant automatique s'efface.
    bool _userPickedTab = false;
    bool _suppressPanelFocusTracking = false;
    std::array<QLabel*, 5> _statusZones{};
    QTimer* _statusMessageTimer = nullptr;
};

}  // namespace hmi
