// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <QWidget>
#include <array>
#include <filesystem>

#include "HMI/Input/EditorKeyBindings.h"

/**
 * @file HMI/Interface/EditorKeybindingsWidget.h
 * @brief Remappage des touches d'éditeur (`EX-CTRL-012`), en widget embarquable (onglet Options,
 *        `LOT-57` TACHE-04).
 */

class QFormLayout;
class QLabel;
class QPushButton;

namespace hmi {

class Localization;

/**
 * @brief Widget de remappage des **touches d'éditeur** (`EditorKeyBindings`), destiné à l'onglet
 *        « Éditeur » de la page Options.
 *
 * Patron identique à `KeybindingsWidget` (touches de jeu) : chaque action affiche sa touche
 * courante, cliquer capture la **prochaine touche pressée** (échange auto si déjà prise),
 * persistée immédiatement dans le fichier partagé. `bindingsChanged` **oblige** l'appelant à
 * resynchroniser les raccourcis effectifs des `QAction` (`hmi::EditorActions::applyShortcuts`) :
 * ce widget écrit la liaison, il ne touche pas aux actions, et un remappage non resynchronisé
 * resterait invisible dans le menu et la barre d'outils.
 */
class EditorKeybindingsWidget : public QWidget {
    Q_OBJECT

public:
    EditorKeybindingsWidget(hmi::EditorKeyBindings& bindings, std::filesystem::path savePath,
                            QWidget* parent = nullptr);

    /// Applique la langue active (libellés d'actions, invite de capture, aide).
    void retranslateUi(const Localization& loc);

signals:
    /// Émis après chaque remappage réussi (touche capturée et sauvegardée).
    void bindingsChanged();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    void refresh();

    hmi::EditorKeyBindings& _bindings;
    std::filesystem::path _savePath;
    int _capturing = -1;  ///< Indice de l'action en cours de capture, ou -1.
    std::array<QPushButton*, hmi::EDITOR_ACTION_COUNT> _buttons{};
    std::array<QLabel*, hmi::EDITOR_ACTION_COUNT> _actionLabels{};
    QLabel* _help = nullptr;
    const Localization* _loc = nullptr;
};

}  // namespace hmi
