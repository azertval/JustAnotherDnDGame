// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <array>

#include <QtQml/qqmlregistration.h>

#include "HMI/Input/ButtonRepeat.h"
#include "HMI/Input/GamepadButton.h"
#include "HMI/Input/GamepadPoller.h"
#include "HMI/Input/InputState.h"

namespace hmi {

/**
 * @brief La manette pour un écran Qt Quick : chaque appui devient un signal nommé (`LOT-24`).
 *
 * ## Pourquoi il fallait l'écrire
 *
 * Le jeu Qt Quick du `LOT-86` liait `xinput` sans jamais le lire : le sondage de la manette
 * (`hmi::GamepadPoller`) n'était compilé que dans l'éditeur. Qt 6 n'a plus de module manette. Un
 * écran de combat qui doit se jouer **entièrement à la manette** avait donc besoin de ce pont.
 *
 * ## Ce qu'il fait, et ce qu'il ne fait pas
 *
 * Tant qu'il est `active`, il sonde la manette soixante fois par seconde et émet `pressed` avec le
 * nom du bouton : `up`, `down`, `left`, `right`, `a`, `b`, `x`, `y`, `lb`, `rb`. La croix et le
 * stick se répètent quand on les tient (`hmi::ButtonRepeat`) ; les autres boutons n'émettent qu'à
 * l'appui. Il ne sait rien de ce que l'écran en fait : le jumeau de câblage traduit un nom en
 * geste, comme il traduit une touche. Inactif, il ne sonde pas — un écran qui n'écoute pas la
 * manette ne la paie pas.
 */
class GamepadNavigator : public QObject {
    Q_OBJECT
    QML_ELEMENT

    /// Vrai pendant que l'écran écoute la manette.
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    /// Vrai quand une manette répond.
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)

public:
    explicit GamepadNavigator(QObject* parent = nullptr);

    [[nodiscard]] bool active() const noexcept {
        return _timer.isActive();
    }
    void setActive(bool active);
    [[nodiscard]] bool connected() const noexcept {
        return _connected;
    }

signals:
    void activeChanged();
    void connectedChanged();
    /// Un appui, ou une répétition de la croix tenue.
    void pressed(const QString& button);

private:
    void poll();

    QTimer _timer;
    GamepadPoller _poller;
    InputState _input;
    std::array<ButtonRepeat, 4> _directions{};
    bool _connected = false;
};

}  // namespace hmi
