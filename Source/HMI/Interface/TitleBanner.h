// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QLabel>

/**
 * @file HMI/Interface/TitleBanner.h
 * @brief Titre d'écran posé sur son bandeau de grenat (`LOT-76`, `EX-IHM-075`).
 *
 * Couche **Qt** au-dessus de `hmi::titleBannerShapes` : la géométrie décide *quoi* dessiner, cette
 * classe décide *comment* le peindre et **où le texte tient**.
 *
 * ## Un `QLabel`, et pas un conteneur
 *
 * Le titre était déjà un `QLabel` nommé sur chacun des six écrans du jeu, et la feuille de style
 * l'atteint par cet objectName. En dériver plutôt que d'envelopper garde ces règles valides — un
 * sélecteur de type Qt s'applique aux sous-classes — et évite d'ajouter un niveau de disposition
 * à six `.ui` pour un ornement.
 *
 * ## Le texte tient entre les ailes, pas par-dessus
 *
 * `minimumSizeHint`/`sizeHint` réservent l'envergure des deux ailes en plus de la largeur du
 * texte, et `contentsMargins` l'y maintient. Sans cela, un titre long déborderait sur les ailes
 * et les mots seraient coupés par des doigts d'or — un défaut que l'on ne voit que sur le titre le
 * plus long, c'est-à-dire jamais pendant qu'on le règle.
 *
 * ## Une seule couleur de texte, et elle vient des jetons
 *
 * La plaque est un grenat sombre : l'encre sépia des écrans y serait illisible. Le titre passe
 * donc à `accentHover`, l'or pâle des reflets — la même valeur qui éclaire la facette du cabochon,
 * pour que le doré du bandeau soit **un** doré et non deux. La règle est posée par la feuille de
 * style (`EX-IHM-051`), jamais en dur ici.
 */

namespace hmi {

class TitleBanner : public QLabel {
    Q_OBJECT

public:
    explicit TitleBanner(QWidget* parent = nullptr);

    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void updateMargins();
};

}  // namespace hmi
