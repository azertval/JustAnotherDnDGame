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
 * @file HMI/Runtime/CharacterSheetModel.h
 * @brief La fiche de personnage, telle que le QML la lit (`LOT-86`).
 */

namespace hmi {

/**
 * @brief Vue-modèle de la fiche : ce que le jeu **sait dire** d'un personnage.
 *
 * ## Où passe la frontière
 *
 * Cette classe est la frontière entre le développeur et la conception. Elle décide **quelles
 * données existent** ; le QML décide **comment elles se voient**. Ajouter un champ ici demande un
 * développeur ; le déplacer, le colorer, l'animer ou le supprimer de l'écran ne demande personne.
 *
 * Elle ne connaît donc ni QML, ni asset, ni feuille de style : elle ne lie que `Qt6::Qml`, pour
 * pouvoir être déclarée au moteur — jamais `Qt6::Quick` ni `Qt6::Widgets`, et un lint le vérifie
 * (`EX-IHM-101`).
 *
 * ## Ce qu'elle ne refait pas
 *
 * Le **formatage** — le signe d'un modificateur, le « 25 / 30 » des points de vie, le point qui
 * marque une maîtrise — reste dans `hmi::characterSheetValues`, fonction **pure** et couverte par
 * ses tests. Cette classe la lit et la publie ; elle ne la réécrit pas. Deux endroits qui savent
 * écrire un modificateur finiraient par ne plus l'écrire pareil.
 *
 * ## Pourquoi des propriétés nommées plutôt que la table brute
 *
 * La couche pure produit une table indexée par chaîne (`sheet.hit_points`), et c'est le bon
 * contrat **entre C++** : un champ ajouté n'y touche qu'un endroit. Mais un artiste travaillant
 * dans Qt Design Studio verrait alors des clés de chaîne dans le panneau des propriétés, sans
 * complétion ni vérification. Les propriétés ci-dessous nomment ces clés une fois, ici — elles ne
 * constituent pas une deuxième source, seulement une façade lisible sur la première.
 */
class CharacterSheetModel : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString name READ name NOTIFY changed)
    Q_PROPERTY(QString species READ species NOTIFY changed)
    Q_PROPERTY(QString background READ background NOTIFY changed)
    Q_PROPERTY(QString classAndLevel READ classAndLevel NOTIFY changed)
    Q_PROPERTY(QString level READ level NOTIFY changed)
    Q_PROPERTY(QString experience READ experience NOTIFY changed)

    Q_PROPERTY(QString hitPoints READ hitPoints NOTIFY changed)
    Q_PROPERTY(QString hitPointsMax READ hitPointsMax NOTIFY changed)
    Q_PROPERTY(QString hitDice READ hitDice NOTIFY changed)
    Q_PROPERTY(QString armorClass READ armorClass NOTIFY changed)
    Q_PROPERTY(QString initiative READ initiative NOTIFY changed)
    Q_PROPERTY(QString speed READ speed NOTIFY changed)
    Q_PROPERTY(QString proficiencyBonus READ proficiencyBonus NOTIFY changed)
    Q_PROPERTY(QString passivePerception READ passivePerception NOTIFY changed)

    Q_PROPERTY(QAbstractItemModel* abilities READ abilities CONSTANT)
    Q_PROPERTY(QAbstractItemModel* savingThrows READ savingThrows CONSTANT)
    Q_PROPERTY(QAbstractItemModel* skills READ skills CONSTANT)

public:
    explicit CharacterSheetModel(QObject* parent = nullptr);

    /**
     * @brief Charge le personnage de **démonstration** livré en donnée.
     *
     * ÉCHAFAUDAGE, et il est écrit comme tel : il n'existe encore ni groupe ni sauvegarde d'où
     * tirer un personnage réel, et un écran de fiche qui n'affiche aucune fiche ne se relit pas.
     * Le jour où une partie en fournira un, c'est **la source** qui change ici — pas l'écran, qui
     * ne consomme que des valeurs nommées, d'où qu'elles viennent.
     *
     * Une donnée manquante n'interrompt rien : l'erreur est journalisée en nommant son fichier
     * (`EX-CNT-010`) et la fiche s'affiche partielle, avec ses tirets. Une fiche partielle vaut
     * mieux qu'un écran vide.
     */
    Q_INVOKABLE void loadDemonstrationCharacter();

    [[nodiscard]] QString name() const {
        return value("sheet.name");
    }
    [[nodiscard]] QString species() const {
        return value("sheet.species");
    }
    [[nodiscard]] QString background() const {
        return value("sheet.background");
    }
    [[nodiscard]] QString classAndLevel() const {
        return value("sheet.class_and_level");
    }
    [[nodiscard]] QString level() const {
        return value("sheet.level");
    }
    [[nodiscard]] QString experience() const {
        return value("sheet.experience");
    }
    [[nodiscard]] QString hitPoints() const {
        return value("sheet.hit_points");
    }
    [[nodiscard]] QString hitPointsMax() const {
        return value("sheet.hit_points_max");
    }
    [[nodiscard]] QString hitDice() const {
        return value("sheet.hit_dice");
    }
    [[nodiscard]] QString armorClass() const {
        return value("sheet.armor_class");
    }
    [[nodiscard]] QString initiative() const {
        return value("sheet.initiative");
    }
    [[nodiscard]] QString speed() const {
        return value("sheet.speed");
    }
    [[nodiscard]] QString proficiencyBonus() const {
        return value("sheet.proficiency_bonus");
    }
    [[nodiscard]] QString passivePerception() const {
        return value("sheet.passive_perception");
    }

    [[nodiscard]] QAbstractItemModel* abilities() {
        return &_abilities;
    }
    [[nodiscard]] QAbstractItemModel* savingThrows() {
        return &_savingThrows;
    }
    [[nodiscard]] QAbstractItemModel* skills() {
        return &_skills;
    }

signals:
    /// Émis quand la fiche change en bloc. Un signal unique et non un par propriété : la fiche est
    /// recalculée d'un coup à chaque changement de source, et prétendre le contraire obligerait à
    /// comparer quatorze champs pour n'en notifier que ceux qui bougent — un coût réel pour une
    /// économie que personne ne mesurerait.
    void changed();

private:
    /// @return La valeur formatée de @p key, ou le tiret cadratin si la fiche ne la porte pas.
    /// Jamais une chaîne vide : à l'écran, « pas de valeur » doit se voir comme tel.
    [[nodiscard]] QString value(const char* key) const;

    std::map<std::string, std::string> _values;
    SheetRowModel _abilities;
    SheetRowModel _savingThrows;
    SheetRowModel _skills;
};

}  // namespace hmi
