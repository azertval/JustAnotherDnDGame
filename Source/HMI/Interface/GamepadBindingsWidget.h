// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <QWidget>
#include <array>
#include <filesystem>

#include "HMI/Input/GameKeyBindings.h"  // GAME_ACTION_COUNT
#include "HMI/Input/GamepadBindings.h"
#include "HMI/Input/GamepadPoller.h"
#include "HMI/Input/InputState.h"

/**
 * @file HMI/Interface/GamepadBindingsWidget.h
 * @brief Remappage des boutons de manette (`EX-CTRL-002`), en widget (onglet Options, LOT-38).
 */

class QLabel;
class QPushButton;
class QTimer;

namespace hmi {

class Localization;

/**
 * @brief Widget de remappage des **boutons manette** (`GamepadBindings`), onglet « Commande
 *        manette » de la page Options.
 *
 * Chaque action affiche son bouton courant ; cliquer une action **sonde XInput** (via
 * `hmi::GamepadPoller`, dans un `QTimer`) jusqu'au prochain bouton pressé, puis l'assigne (échange
 * auto). Persisté dans `keybindings.json`. La partie en cours en tient compte immédiatement.
 * La page Options n'étant affichée que hors jeu (viewport masqué, boucle en pause), aucun sondage
 * XInput concurrent.
 */
class GamepadBindingsWidget : public QWidget {
    Q_OBJECT

public:
    GamepadBindingsWidget(hmi::GamepadBindings& bindings, std::filesystem::path savePath,
                          QWidget* parent = nullptr);

    /// Applique la langue active (libellés d'actions, invite de capture, aide, état manette).
    void retranslateUi(const Localization& loc);

protected:
    /// Démarre/arrête le sondage d'état manette avec la visibilité de l'onglet.
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    void startCapture(int index);
    void onCaptureTick();
    void refresh();
    /// Sonde la manette et met à jour le libellé d'état (connectée / non connectée).
    void updateStatus();

    hmi::GamepadBindings& _bindings;
    std::filesystem::path _savePath;
    int _capturing = -1;
    hmi::GamepadPoller _poller;
    hmi::InputState _input;
    QTimer* _timer;
    QTimer* _statusTimer = nullptr;  ///< Sondage périodique de l'état de connexion (hors capture).
    std::array<QPushButton*, hmi::GAME_ACTION_COUNT> _buttons{};
    std::array<QLabel*, hmi::GAME_ACTION_COUNT> _actionLabels{};
    QLabel* _help = nullptr;
    QLabel* _status = nullptr;  ///< État de connexion de la manette (informatif).
    const Localization* _loc = nullptr;
};

}  // namespace hmi
