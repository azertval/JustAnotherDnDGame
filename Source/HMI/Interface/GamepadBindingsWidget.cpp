// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Interface/GamepadBindingsWidget.h"

#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QTimer>
#include <QVBoxLayout>
#include <array>
#include <optional>
#include <utility>

#include "HMI/Input/GamepadButtonName.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Localization/Localization.h"

namespace hmi {

namespace {

// Clés de traduction des actions (même ordre que `hmi::GameAction`, partagées avec le clavier).
constexpr std::array<const char*, hmi::GAME_ACTION_COUNT> ACTION_KEYS{
    "keybindings.action.left",     "keybindings.action.right", "keybindings.action.aim_up",
    "keybindings.action.aim_down", "keybindings.action.jump",  "keybindings.action.dash",
    "keybindings.action.interact"};

[[nodiscard]] hmi::GameAction actionAt(int index) {
    return static_cast<hmi::GameAction>(index);
}

}  // namespace

GamepadBindingsWidget::GamepadBindingsWidget(hmi::GamepadBindings& bindings,
                                             std::filesystem::path savePath, QWidget* parent)
    : QWidget(parent),
      _bindings(bindings),
      _savePath(std::move(savePath)),
      _timer(new QTimer(this)) {
    _timer->setInterval(30);
    connect(_timer, &QTimer::timeout, this, &GamepadBindingsWidget::onCaptureTick);

    auto* const form = new QFormLayout();
    for (int index = 0; index < hmi::GAME_ACTION_COUNT; ++index) {
        auto* const button = new QPushButton(this);
        // Largeur minimale unifiee avec KeybindingsWidget (LOT-56, jeton controlMinWidth).
        button->setMinimumWidth(editorDarkTokens().size.controlMinWidth);
        connect(button, &QPushButton::clicked, this, [this, index] { startCapture(index); });
        _buttons[static_cast<std::size_t>(index)] = button;
        auto* const label = new QLabel(this);
        _actionLabels[static_cast<std::size_t>(index)] = label;
        form->addRow(label, button);
    }

    auto* const layout = new QVBoxLayout(this);
    layout->addLayout(form);
    _help = new QLabel(this);
    _help->setWordWrap(true);
    layout->addWidget(_help);
    _status = new QLabel(this);  // état de connexion (informatif, non navigable).
    layout->addWidget(_status);
    layout->addStretch();

    // Sondage périodique de l'état de connexion, actif seulement quand l'onglet est visible et
    // qu'aucune capture n'est en cours (le sondage de capture, plus rapide, prend alors le relais).
    _statusTimer = new QTimer(this);
    _statusTimer->setInterval(500);
    connect(_statusTimer, &QTimer::timeout, this, &GamepadBindingsWidget::updateStatus);

    refresh();
}

void GamepadBindingsWidget::updateStatus() {
    _poller.poll(_input);
    _input.beginFrame();
    if (_loc == nullptr) {
        return;
    }
    _status->setText(QString::fromStdString(_loc->text(
        _input.gamepadConnected() ? "options.gamepad_connected" : "options.gamepad_disconnected")));
}

void GamepadBindingsWidget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    if (_capturing < 0) {
        updateStatus();
        _statusTimer->start();
    }
}

void GamepadBindingsWidget::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    _statusTimer->stop();
}

void GamepadBindingsWidget::retranslateUi(const Localization& loc) {
    _loc = &loc;
    for (int index = 0; index < hmi::GAME_ACTION_COUNT; ++index) {
        _actionLabels[static_cast<std::size_t>(index)]->setText(
            QString::fromStdString(loc.text(ACTION_KEYS[static_cast<std::size_t>(index)])));
    }
    _help->setText(QString::fromStdString(loc.text("keybindings.gamepad_hint")));
    updateStatus();
    refresh();
}

void GamepadBindingsWidget::startCapture(int index) {
    _statusTimer->stop();  // évite deux sondages concurrents sur le même InputState.
    _capturing = index;
    _buttons[static_cast<std::size_t>(index)]->setText(
        _loc != nullptr ? QString::fromStdString(_loc->text("keybindings.press_button"))
                        : QStringLiteral("…"));
    // Établit une ligne de base : un bouton déjà maintenu ne sera pas capturé (seul un nouvel appui
    // — front — l'est ensuite, cf. capturedGamepadButton).
    _poller.poll(_input);
    _input.beginFrame();
    _timer->start();
}

void GamepadBindingsWidget::onCaptureTick() {
    _poller.poll(_input);
    if (const std::optional<hmi::GamepadButton> button = hmi::capturedGamepadButton(_input)) {
        _bindings.setKey(actionAt(_capturing), *button);
        static_cast<void>(_bindings.save(_savePath));
        _capturing = -1;
        _timer->stop();
        refresh();
        if (isVisible()) {
            _statusTimer->start();  // reprend le suivi de connexion.
        }
        return;
    }
    _input.beginFrame();  // avance les fronts pour la détection au prochain tick
}

void GamepadBindingsWidget::refresh() {
    for (int index = 0; index < hmi::GAME_ACTION_COUNT; ++index) {
        _buttons[static_cast<std::size_t>(index)]->setText(QString::fromStdString(
            hmi::gamepadButtonDisplayName(_bindings.button(actionAt(index)))));
    }
}

}  // namespace hmi
