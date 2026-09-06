// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>

/**
 * @file HMI/Interface/ParchmentPanel.h
 * @brief Conteneur peignant un encadrement de parchemin (`LOT-66`, `EX-IHM-070`).
 *
 * Couche **Qt** au-dessus de `hmi::parchmentFrameStrokes` : la géométrie décide *quoi* dessiner,
 * cette classe décide *comment* le peindre, en résolvant chaque rôle depuis les jetons de la portée
 * **identité** — jamais depuis ceux du châssis d'édition, qui suivent le thème clair/sombre.
 *
 * **Promu dans les `.ui`** (Qt Designer, `Source/Elements/UI/`) : la mise en page reste
 * déclarative, seul le tracé est en C++. C'est la seule façon de tenir la convention du projet
 * — « la mise en page hors code » — tout en dessinant un encadrement qu'aucune feuille de style ne
 * sait produire : une bordure QSS ne peut pas poser de cabochon d'angle par-dessus son propre
 * trait.
 */

namespace hmi {

class ParchmentPanel : public QWidget {
    Q_OBJECT

public:
    explicit ParchmentPanel(QWidget* parent = nullptr);

    /// Variante **accentuée** de l'encadrement : filet et cabochons à la couleur d'accent plutôt
    /// qu'au brun doré. Réservée aux fenêtres modales (pause, fin de niveau), où elle signale que
    /// l'écran se superpose au jeu au lieu de le remplacer.
    void setAccented(bool accented);
    [[nodiscard]] bool isAccented() const {
        return _accented;
    }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    bool _accented = false;
};

}  // namespace hmi
