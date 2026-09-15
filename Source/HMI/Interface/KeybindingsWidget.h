// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <QWidget>
#include <array>
#include <filesystem>

#include "HMI/Input/GameKeyBindings.h"

/**
 * @file HMI/Interface/KeybindingsWidget.h
 * @brief Remappage des touches de jeu (`EX-CTRL-012`), en widget embarquable (onglet Options,
 * LOT-38).
 */

class QFormLayout;
class QLabel;
class QPushButton;

namespace hmi {

class Localization;

/**
 * @brief Widget de remappage des **touches de jeu** (`GameKeyBindings`), destiné à l'onglet
 *        « Commande clavier » de la page Options.
 *
 * Chaque action affiche sa touche courante ; cliquer une action capture la **prochaine touche
 * pressée** et l'assigne (échange auto si déjà prise). Toute modification est **persistée** dans le
 * fichier partagé (`keybindings.json`) ; la partie en cours en tient compte immédiatement (mêmes
 * bindings lus par la session de jeu). `Échap` annule une capture en cours.
 */
class KeybindingsWidget : public QWidget {
    Q_OBJECT

public:
    KeybindingsWidget(hmi::GameKeyBindings& bindings, std::filesystem::path savePath,
                      QWidget* parent = nullptr);

    /// Applique la langue active (libellés d'actions, invite de capture, aide).
    void retranslateUi(const Localization& loc);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    void refresh();

    hmi::GameKeyBindings& _bindings;
    std::filesystem::path _savePath;
    int _capturing = -1;  ///< Indice de l'action en cours de capture, ou -1.
    std::array<QPushButton*, hmi::GAME_ACTION_COUNT> _buttons{};
    std::array<QLabel*, hmi::GAME_ACTION_COUNT> _actionLabels{};
    QLabel* _help = nullptr;
    const Localization* _loc = nullptr;
};

}  // namespace hmi
