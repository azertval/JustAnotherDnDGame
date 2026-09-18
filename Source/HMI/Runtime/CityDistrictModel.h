// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QtQmlIntegration>

/**
 * @file HMI/Runtime/CityDistrictModel.h
 * @brief Un quartier sur le plan de la ville, et ses îlots (`LOT-96`).
 */

namespace hmi {

/**
 * @brief Vue-modèle des deux niveaux que l'écran « Carte » gagne sous la ville : le **quartier**
 *        et l'**îlot**.
 *
 * Elle lit la carte du quartier (`Levels/<carte>.json`, à côté de l'exécutable) et en rend les
 * îlots (`core::cityBlocksOf`) : leur nom traduit (`city_block.<nom>` des catalogues), leur
 * rectangle de cases, et leur centre **en fractions de la carte**. La carte d'un quartier est
 * tracée nord en haut, comme le plan de la ville (`carte_quartiers.py`) : ces fractions posent
 * donc les îlots au bon endroit sur l'agrandissement du plan.
 *
 * L'image d'un îlot n'est pas un fichier : `blockImage` rend l'adresse que
 * `hmi::CityBlockImageProvider` dessine à la demande (*décision de l'auteur*, 18 septembre 2026).
 */
class CityDistrictModel : public QObject {
    Q_OBJECT
    QML_ELEMENT

public:
    explicit CityDistrictModel(QObject* parent = nullptr);

    /**
     * @brief Le quartier dont @p mapId est la carte : `mapId`, `columns`, `rows` et `blocks` (une
     *        table par îlot : `blockId`, `name`, `column`, `row`, `width`, `height`, `x`, `y`).
     * @return Une table vide si la carte ne se lit pas.
     */
    [[nodiscard]] Q_INVOKABLE QVariantMap district(const QString& mapId) const;

    /**
     * @brief L'adresse de l'image de l'îlot @p blockId de @p mapId, héros compris si @p figure
     *        n'est pas vide (à la case continue @p heroColumn, @p heroRow).
     */
    [[nodiscard]] Q_INVOKABLE QString blockImage(const QString& mapId, const QString& blockId,
                                                 const QString& figure, qreal heroColumn,
                                                 qreal heroRow) const;

    /// @return L'îlot de @p mapId qui contient la case (@p column, @p row), vide sinon.
    [[nodiscard]] Q_INVOKABLE QString blockAt(const QString& mapId, int column, int row) const;
};

}  // namespace hmi
