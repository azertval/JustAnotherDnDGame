// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QColor>
#include <QQuickRhiItem>
#include <QtQmlIntegration>

/**
 * @file HMI/Game/GameViewportItem.h
 * @brief La surface de rendu du **jeu**, en Qt Quick (`LOT-86`).
 */

namespace hmi {

/**
 * @brief Surface QRhi du jeu, composée avec le reste de l'interface QML.
 *
 * Le type appartient au module QML runtime `Jadg.Runtime`. Il est volontairement absent des
 * formulaires `.ui.qml` afin que Qt Design Studio puisse les éditer sans charger QRhi.
 */
class GameViewportItem : public QQuickRhiItem {
    Q_OBJECT
    QML_NAMED_ELEMENT(GameViewport)

    Q_PROPERTY(QColor clearColor READ clearColor WRITE setClearColor NOTIFY clearColorChanged)

public:
    explicit GameViewportItem(QQuickItem* parent = nullptr);

    [[nodiscard]] QColor clearColor() const noexcept {
        return _clearColor;
    }
    void setClearColor(const QColor& color);

    [[nodiscard]] QQuickRhiItemRenderer* createRenderer() override;

signals:
    void clearColorChanged();

private:
    QColor _clearColor{0xd0, 0xc0, 0xa0};
};

}  // namespace hmi
