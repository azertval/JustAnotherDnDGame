// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QtQmlIntegration>

/**
 * @file HMI/Runtime/WorldMapModel.h
 * @brief Les régions de l'atlas, posées sur la carte du monde, exposées au QML (`LOT-87`, T3.6).
 */

namespace hmi {

/**
 * @brief Vue-modèle de la carte du monde : les treize régions de l'atlas et leurs lieux.
 *
 * Charge l'atlas du `LOT-37` (`World/`, à côté de l'exécutable) et les ancres relevées sur la
 * carte (`Maps/world-map-regions.json`), puis les joint par `hmi::worldMapRegions`. Une région sans
 * ancre, ou une ancre sans région, est journalisée plutôt que tue.
 *
 * `regions` est une liste de tables — `regionId`, `name`, `x`, `y` (fractions de la carte),
 * `government`, `locations` (un nom par ligne) et `locationCount` — lue par un `Repeater`.
 */
class WorldMapModel : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QVariantList regions READ regions NOTIFY changed)
    /// Nombre total de lieux répartis dans les régions posées.
    Q_PROPERTY(int locationCount READ locationCount NOTIFY changed)

public:
    explicit WorldMapModel(QObject* parent = nullptr);

    /// Charge l'atlas et les ancres livrés à côté de l'exécutable.
    Q_INVOKABLE void load();

    [[nodiscard]] QVariantList regions() const {
        return _regions;
    }
    [[nodiscard]] int locationCount() const noexcept {
        return _locationCount;
    }

signals:
    void changed();

private:
    QVariantList _regions;
    int _locationCount = 0;
};

}  // namespace hmi
