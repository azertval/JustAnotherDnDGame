// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QAbstractItemModel>
#include <QObject>
#include <QString>
#include <QtQmlIntegration>
#include <map>
#include <string>

#include "HMI/Runtime/SheetRowModel.h"

/**
 * @file HMI/Runtime/InventoryModel.h
 * @brief Ce que le personnage porte, tel que le QML le lit (`LOT-86`).
 */

namespace hmi {

/**
 * @brief Vue-modèle de l'inventaire : les seize emplacements, la bourse, la charge et le sac.
 *
 * Même frontière que `hmi::CharacterSheetModel` : cette classe décide **quelles données
 * existent**, le QML décide **comment elles se voient**.
 *
 * **Aucune statistique n'est stockée ici**, et c'est la règle du `LOT-14` qu'il ne faut pas
 * défaire : le poids porté, la capacité et la classe d'armure sont des *fonctions* du contenu,
 * recalculées à chaque chargement. Le défaut classique — appliquer un bonus à la volée et le
 * retrancher au retrait — fait dériver la valeur après trois équipements et deux retraits dans le
 * désordre, sans que rien ne le signale. Une valeur qu'on ne stocke pas ne peut pas dériver.
 */
class InventoryModel : public QObject {
    Q_OBJECT
    QML_ELEMENT

    /// Les seize emplacements d'équipement : libellé du corps, objet porté (ou tiret).
    Q_PROPERTY(QAbstractItemModel* equipmentSlots READ equipmentSlots CONSTANT)

    Q_PROPERTY(QString purse READ purse NOTIFY changed)
    Q_PROPERTY(QString carried READ carried NOTIFY changed)
    Q_PROPERTY(QString capacity READ capacity NOTIFY changed)
    Q_PROPERTY(QString backpack READ backpack NOTIFY changed)

public:
    explicit InventoryModel(QObject* parent = nullptr);

    /// Charge le personnage de démonstration. Voir `hmi::loadDemonstrationValues` : le chargement
    /// est **partagé** avec la fiche, parce que les deux écrans décrivent le même personnage.
    Q_INVOKABLE void loadDemonstrationCharacter();

    [[nodiscard]] QAbstractItemModel* equipmentSlots() {
        return &_slots;
    }
    [[nodiscard]] QString purse() const {
        return value("inventory.purse");
    }
    [[nodiscard]] QString carried() const {
        return value("inventory.carried");
    }
    [[nodiscard]] QString capacity() const {
        return value("inventory.capacity");
    }
    [[nodiscard]] QString backpack() const {
        return value("inventory.backpack");
    }

signals:
    /// Émis quand l'inventaire change en bloc. Un signal unique, pour la même raison que sur la
    /// fiche : tout est recalculé d'un coup, et prétendre le contraire coûterait plus que la
    /// précision qu'on y gagnerait.
    void changed();

private:
    [[nodiscard]] QString value(const char* key) const;

    std::map<std::string, std::string> _values;
    SheetRowModel _slots;
};

}  // namespace hmi
