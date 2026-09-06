// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>

class QPainter;
class QRect;

/**
 * @file HMI/Interface/FocusMarker.h
 * @brief Marque explicite de focus des écrans du jeu (`LOT-66`, `EX-IHM-071`).
 */

namespace hmi {

/**
 * @brief Peint le fleuron de focus dans @p box, à la couleur d'accent de l'identité.
 *
 * Fonction **libre** et non méthode : la marque est peinte à deux endroits — par `hmi::FocusMarker`
 * pour les contrôles ordinaires, par `hmi::MenuEntryButton` dans sa propre gouttière — et
 * `EX-IHM-071` demande *une* marque, pas deux qui se ressemblent. Deux tracés séparés dériveraient
 * l'un de l'autre à la première retouche, sans que rien ne le signale : le joueur verrait une
 * pointe dans le menu et une autre dans les options, et croirait à deux états différents.
 *
 * @param painter Peintre déjà positionné dans le repère du widget hôte.
 * @param box     Carré englobant du fleuron. Un rectangle non carré est rogné à son plus petit
 *                côté, puis centré : une marque étirée ne se reconnaît plus.
 */
void paintFocusFleuron(QPainter& painter, const QRect& box);

/**
 * @brief Marque de focus flottante, posée à gauche du contrôle focalisé de l'écran hôte.
 *
 * `EX-IHM-071` demande que l'élément focalisé d'un écran du jeu soit signalé par une **marque
 * explicite**, et non par la seule teinte : la navigation à la manette repose entièrement sur le
 * parcours de focus, qu'une nuance de couleur rend difficile à suivre — et impossible pour un
 * joueur qui distingue mal les couleurs. Une feuille de style ne sachant pas ajouter de contenu,
 * cette marque est nécessairement peinte.
 *
 * `hmi::MenuEntryButton` la peint lui-même, dans sa propre gouttière. Ce widget couvre l'autre cas
 * : les contrôles **ordinaires** de Qt (boutons d'action, listes, onglets, champs numériques),
 * qu'on ne peut pas tous sous-classer. Il se place **à côté** du contrôle focalisé plutôt que
 * dedans, dans le repère de l'écran hôte, et se hisse au-dessus de ses frères.
 *
 * Transparent aux événements de souris : il ne s'interpose jamais entre le joueur et le contrôle
 * qu'il désigne.
 *
 * Le suivi du focus reste à la charge de l'hôte (`follow`), qui seul sait quels contrôles lui
 * appartiennent — un `QApplication::focusChanged` global désignerait aussi ceux des autres écrans.
 */
class FocusMarker : public QWidget {
    Q_OBJECT

public:
    /// @param host Écran suivi ; la marque en devient un enfant, et vit dans son repère.
    explicit FocusMarker(QWidget* host);

    /**
     * @brief Place la marque devant @p focused, ou la masque si le focus a quitté l'hôte.
     * @param focused Contrôle actuellement focalisé (peut être nul, ou hors de l'hôte).
     */
    void follow(QWidget* focused);

protected:
    void paintEvent(QPaintEvent* event) override;
};

}  // namespace hmi
