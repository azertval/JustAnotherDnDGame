// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QWidget>

/**
 * @file HMI/Interface/SheetWidgets.h
 * @brief Les cinq pièces peintes de la planche de la fiche (`LOT-38`).
 *
 * ## Pourquoi un seul fichier pour cinq classes
 *
 * Ce sont cinq formes d'une **même** famille, tracées avec la même encre, le même filet et le même
 * rapport de trait, et posées par un seul écran. Les séparer en cinq paires de fichiers ferait
 * cinq en-têtes qui se recopient et cinq occasions de laisser l'une dériver — c'est le défaut que
 * le générateur d'icônes d'emplacement évite déjà, transposé au code.
 *
 * ## Pourquoi peintes, et non composées de widgets
 *
 * Une jauge faite d'un `QFrame` dans un `QFrame` suivrait la feuille de style, donc les marges du
 * thème, et ne pourrait pas porter le filet d'or **en réserve** qui fait lire un cadre
 * (`LOT-66`). Les tracer, c'est aussi ce qui permet à leurs proportions de suivre la taille
 * demandée au lieu de s'étirer — la leçon de `hmi::ParchmentOrnaments`.
 *
 * Chacune prend ses couleurs aux jetons de la portée identité (`EX-IHM-051`) et **aucune** ne
 * porte de teinte en dur.
 */

namespace hmi {

/**
 * @brief Une jauge : un intitulé, une valeur écrite, et une barre.
 *
 * La barre donne l'allure, le nombre donne le fait. Une barre seule ne se lit pas au point de vie
 * près, et un nombre seul ne dit pas d'un coup d'œil si le personnage va bien — la planche du
 * corpus porte les deux, et c'est pourquoi.
 */
class SheetGauge : public QWidget {
    Q_OBJECT

public:
    /// Ce que la barre représente, et donc de quelle encre elle se remplit.
    enum class Tone {
        Vitality,  ///< Points de vie : le grenat du cachet, la seule autre encre rouge de l'écran.
        Progress,  ///< Expérience : l'or des filets.
    };

    explicit SheetGauge(Tone tone, QWidget* parent = nullptr);

    /// @param label Intitulé traduit. @param value Valeur déjà formatée, affichée à droite.
    void setLabel(const QString& label);
    void setValue(const QString& value);
    /// Remplissage, de 0 à 1. Hors bornes, il est ramené dedans : une jauge qui déborde de son
    /// cadre se lirait comme un défaut de rendu.
    void setFill(double fill);

    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    Tone _tone;
    QString _label;
    QString _value;
    double _fill = 0.0;
};

/**
 * @brief La classe d'armure, sur son écu.
 *
 * La seule silhouette non ronde de la planche, et c'est délibéré : sur la feuille du corpus, la
 * classe d'armure est la seule valeur qui se défend. Lui donner un médaillon comme aux autres la
 * ferait lire comme une caractéristique de plus.
 */
class ShieldValue : public QWidget {
    Q_OBJECT

public:
    explicit ShieldValue(QWidget* parent = nullptr);

    void setCaption(const QString& caption);
    void setValue(const QString& value);

    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QString _caption;
    QString _value;
};

/**
 * @brief Un petit médaillon de combat : une valeur dans un rond, son intitulé dessous.
 *
 * Le même vocabulaire rond que les six caractéristiques, en plus petit — c'est le groupe que la
 * planche pose en haut à gauche (maîtrise, inspiration, initiative). Les rendre en lignes de texte
 * les sortirait de ce vocabulaire, et l'écran redeviendrait un tableau.
 */
class StatMedallion : public QWidget {
    Q_OBJECT

public:
    explicit StatMedallion(QWidget* parent = nullptr);

    void setCaption(const QString& caption);
    void setValue(const QString& value);

    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QString _caption;
    QString _value;
};

/**
 * @brief Une ligne de sauvegarde ou de compétence : pastille, nom, valeur.
 *
 * La pastille **pleine** est la maîtrise — ce que porte la case cochée de la feuille. Elle est une
 * forme, jamais une teinte : une nuance ne se suit pas à la manette et ne dit rien à qui distingue
 * mal les couleurs (`EX-IHM-071`).
 */
class PipRow : public QWidget {
    Q_OBJECT

public:
    explicit PipRow(QWidget* parent = nullptr);

    void setLabel(const QString& label);
    void setValue(const QString& value);
    void setProficient(bool proficient);

    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QString _label;
    QString _value;
    bool _proficient = false;
};

/**
 * @brief Le cachet de cire du contreseing.
 *
 * Le seul relief de la planche, et le seul rouge avec la jauge de vitalité. C'est lui qui fait
 * qu'un feuillet est *délivré* plutôt qu'imprimé.
 */
class WaxSeal : public QWidget {
    Q_OBJECT

public:
    explicit WaxSeal(QWidget* parent = nullptr);

    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
};

}  // namespace hmi
