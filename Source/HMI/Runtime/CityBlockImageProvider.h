// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <QImage>
#include <QQuickImageProvider>
#include <QSize>
#include <QString>
#include <filesystem>

/**
 * @file HMI/Runtime/CityBlockImageProvider.h
 * @brief Les images des îlots du plan, dessinées à la demande (`image://cityblock/…`, `LOT-96`).
 */

namespace hmi {

/**
 * @brief Dessine l'îlot qu'une adresse `image://cityblock/<carte>|<îlot>|<figurine>|<col>|<lig>`
 *        désigne (`hmi::CityDistrictModel::blockImage`).
 *
 * La carte du quartier, sa table d'apparence, ses PNJ et le héros s'il y est : le lieu tel qu'on le
 * parcourt, cadré sur l'îlot (`hmi::renderCityBlock`). Une adresse qui ne mène à rien rend une
 * image nulle ; l'écran affiche alors son fond, pas une erreur.
 */
class CityBlockImageProvider : public QQuickImageProvider {
public:
    /// @param dataDirectory Le dossier de l'exécutable, où vivent `Levels/` et `Assets/`.
    explicit CityBlockImageProvider(std::filesystem::path dataDirectory);

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

private:
    std::filesystem::path _data;
};

}  // namespace hmi
