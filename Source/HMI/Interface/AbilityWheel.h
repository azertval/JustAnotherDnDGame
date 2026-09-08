// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPixmap>
#include <QString>
#include <QWidget>
#include <array>

#include "HMI/Interface/AbilityWheelGeometry.h"

/**
 * @file HMI/Interface/AbilityWheel.h
 * @brief La roue des caractéristiques, peinte : portrait au centre, six médaillons sur leur arc.
 *
 * Couche **Qt** au-dessus de `hmi::abilityWheelLayout` : la géométrie décide *où*, cette classe
 * décide *comment*. Même partage que `hmi::TitleBanner` sur `hmi::titleBannerShapes`, et même
 * bénéfice — la composition se vérifie par test sans ouvrir de fenêtre.
 *
 * ## La roue porte le nom, et ce n'est pas un ajout
 *
 * L'arc est ouvert en bas, et cette ouverture existe **pour** le nom : c'est ce que la planche du
 * corpus y met. Confier le nom à un libellé posé sous la roue dans le `.ui` le décrocherait de
 * l'ouverture au premier changement de taille — la roue grandirait, l'ouverture avec elle, et le
 * nom resterait où la disposition l'a mis. Ils bougent ensemble ou ils ne veulent rien dire.
 *
 * ## Les tailles de texte suivent le médaillon, pas les jetons
 *
 * Un chiffre inscrit dans un médaillon doit grandir avec lui. Le prendre dans l'échelle
 * typographique — qui, elle, suit le facteur d'agrandissement de l'écran — le laisserait fixe
 * pendant que le médaillon change de taille : à la première fenêtre étroite, le « 15 » déborderait
 * de son cercle, et à la première grande, il flotterait au milieu. Les couleurs, elles, restent
 * prises aux jetons (`EX-IHM-051`) : c'est la taille qui est relative, jamais la teinte.
 *
 * ## Un portrait absent se voit
 *
 * Sans portrait, le centre reçoit le **marqueur** du `LOT-39` — deux diagonales sur un aplat — et
 * non un vide. Un rond vide se lirait comme un défaut de rendu ; les deux diagonales disent « cette
 * illustration n'est pas encore livrée », ce qui est la vérité et se distingue d'une panne.
 */

namespace hmi {

class AbilityWheel : public QWidget {
    Q_OBJECT

public:
    explicit AbilityWheel(QWidget* parent = nullptr);

    /**
     * @brief Pose le contenu d'un siège.
     *
     * @param seat     Le siège, dans l'ordre de la planche.
     * @param label    Nom de la caractéristique, **déjà traduit**.
     * @param score    Valeur, déjà formatée. Vide : le tiret cadratin reste.
     * @param modifier Modificateur signé, déjà formaté. Vide : le tiret cadratin reste.
     */
    void setSeat(WheelSeat seat, const QString& label, const QString& score,
                 const QString& modifier);

    /// Pose le nom et sa ligne d'appartenance (espèce, classe, historique), dans l'ouverture.
    void setIdentity(const QString& name, const QString& subtitle);

    /// Pose le portrait. Un pixmap nul rend le marqueur du `LOT-39`.
    void setPortrait(const QPixmap& portrait);

    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;
    [[nodiscard]] bool hasHeightForWidth() const override;
    [[nodiscard]] int heightForWidth(int width) const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    /// Ce qu'un siège affiche. Trois chaînes déjà prêtes : la roue ne formate ni ne traduit rien —
    /// c'est le même contrat que `RpgScreenFrame::setValues`, et le seul qui permette de vérifier
    /// un modificateur signé sans instancier de fenêtre.
    struct SeatText {
        QString label;
        QString score;
        QString modifier;
    };

    std::array<SeatText, ABILITY_SEAT_COUNT> _seats{};
    QString _name;
    QString _subtitle;
    QPixmap _portrait;
};

}  // namespace hmi
