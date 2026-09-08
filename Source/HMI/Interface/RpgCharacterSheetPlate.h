// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QWidget>
#include <array>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "HMI/Interface/AbilityWheelGeometry.h"
#include "HMI/Interface/RpgScreenSurface.h"
#include "HMI/Interface/RpgScreens.h"

/**
 * @file HMI/Interface/RpgCharacterSheetPlate.h
 * @brief La **planche** de la fiche de personnage (`LOT-38`). Structure dans
 *        `RpgCharacterSheetPlate.ui` (Qt Designer).
 */

namespace Ui {
class RpgCharacterSheetPlate;
}

class QLabel;
class QVBoxLayout;

namespace hmi {

class Localization;

/**
 * @brief La fiche de personnage, rendue comme la planche du corpus et non comme deux colonnes.
 *
 * C'est le premier écran dont la table dit `RpgRendering::DesignerPlate`. Il garde le cadre, la
 * navigation et le pied d'actions des huit autres ; ce qui change est **entre** les deux : la
 * feuille vierge de Tanares pose les six caractéristiques en médaillons sur un arc, autour du
 * portrait, et c'est cette composition qui la fait reconnaître.
 *
 * ## Ce que la planche prend à l'ossature, et ce qu'elle lui laisse
 *
 * L'ossature de l'écran reste décrite dans `hmi::rpgScreens()` : c'est elle qui **nomme les
 * champs**, et la planche n'en invente aucun. Deux de ses blocs sont seulement rendus autrement —
 * l'identité et les caractéristiques, que la roue reprend. Tous les autres deviennent des lignes,
 * dans les deux panneaux, exactement comme l'ossature les aurait rendues.
 *
 * Un champ ajouté à la table apparaît donc ici **sans** rouvrir Qt Designer. C'est ce qui empêche
 * la planche de devenir une seconde description de la fiche, qui divergerait de la première.
 *
 * ## La construction des lignes est recopiée, et c'est assumé
 *
 * `hmi::RpgScreenFrame` sait déjà bâtir des lignes « libellé → valeur » depuis la table, et cette
 * classe le refait. La factoriser demanderait de décider ce que deux rendus ont *vraiment* en
 * commun, et il n'y a qu'une planche : deux planches font un motif, une seule fait une supposition.
 * L'extraction se fera à la seconde, quand elle aura de quoi être juste.
 */
class RpgCharacterSheetPlate : public QWidget, public RpgScreenSurface {
    Q_OBJECT

public:
    explicit RpgCharacterSheetPlate(const RpgScreenDescriptor& descriptor,
                                    QWidget* parent = nullptr);
    ~RpgCharacterSheetPlate() override;

    void retranslateUi(const Localization& loc) override;
    void setValues(const std::map<std::string, std::string>& values) override;
    void focusDefaultAction() override;
    [[nodiscard]] QWidget* widget() override;

signals:
    /// L'écran demande à se fermer. `hmi::RpgScreenHost` décide vers où l'on revient.
    void closeRequested();

protected:
    /// `Échap` ferme la planche, comme sur les huit autres écrans.
    void keyPressEvent(QKeyEvent* event) override;

private:
    /// Une étiquette dont le texte vient du catalogue de traduction.
    struct TranslatedLabel {
        QLabel* label = nullptr;
        const char* key = "";
    };

    /// Une valeur posée par `setValues`, et l'étiquette qui la porte.
    struct ValueLabel {
        QLabel* label = nullptr;
        std::string valueId;
    };

    void buildColumns();
    /// Rend un bloc de l'ossature en lignes, dans @p column. Les blocs que la roue reprend --
    /// l'identité et les caractéristiques -- n'y passent pas.
    void buildBlock(const RpgContentBlock& block, QVBoxLayout* column);
    /// Reporte les valeurs sur la roue : le nom, sa ligne d'appartenance, et les six sièges.
    void applyWheelValues();

    /// Les libellés des six sièges, traduits. Ils sont **peints** par la roue et non portés par des
    /// étiquettes : ils ne peuvent donc pas passer par `_translated`, et doivent être retenus pour
    /// survivre à un changement de valeurs.
    std::array<QString, ABILITY_SEAT_COUNT> _abilityLabels{};

    std::unique_ptr<Ui::RpgCharacterSheetPlate> _ui;
    RpgScreenDescriptor _descriptor;
    std::vector<TranslatedLabel> _translated;
    std::vector<ValueLabel> _values;
    /// Dernières valeurs reçues : la roue est repeinte après un changement de langue, et sans
    /// elles une fiche remplie se viderait en changeant de langue.
    std::map<std::string, std::string> _lastValues;
    /// La derniere cle de portrait chargee. Retenue pour ne pas relire le fichier a chaque pose de
    /// valeurs : `setValues` est rejoue a chaque changement de langue, et une image se relit alors
    /// sans que rien n'ait change.
    QString _loadedPortraitKey;
};

}  // namespace hmi
