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
class PipRow;
class SheetGauge;
class ShieldValue;
class StatMedallion;

/**
 * @brief La fiche de personnage, rendue comme la planche du corpus et non comme deux colonnes.
 *
 * C'est le premier écran dont la table dit `RpgRendering::DesignerPlate`. Il garde le cadre, la
 * navigation et le pied d'actions des huit autres ; ce qui change est **entre** les deux.
 *
 * ## La disposition est écrite, les valeurs restent nommées par la table
 *
 * Les huit autres écrans tirent leur *disposition* de `hmi::rpgScreens()` : c'est le bon outil
 * pour un écran dont aucune maquette n'existe. Celui-ci en a une, gravée, et une ossature
 * générique ne sait pas la rendre — une jauge, un écu, un médaillon et une pastille de maîtrise ne
 * sont pas des lignes « libellé → valeur ».
 *
 * La disposition est donc **explicite ici**, et c'est tout le sens de `DesignerPlate`. Ce qui reste
 * commun est ce qui compte : les **identifiants de valeur** sont ceux de la table, mot pour mot, et
 * les libellés viennent du catalogue de traduction. La planche n'invente ni champ ni intitulé ; si
 * un identifiant disparaissait de la table, la valeur cesserait d'arriver et le tiret cadratin le
 * dirait — ce qui est exactement le comportement voulu.
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
    /// Le chrome du document — souche perforée et filets — est peint **sous** les widgets.
    void paintEvent(QPaintEvent* event) override;

private:
    /// Une étiquette dont le texte vient du catalogue de traduction.
    struct TranslatedLabel {
        QLabel* label = nullptr;
        const char* key = "";
    };

    void buildLeftColumn();
    void buildRightColumn();
    /// Un intitulé de section, dans le style des titres de bloc.
    QLabel* addHeading(QVBoxLayout* column, const char* key);
    /// Un filet d'or pleine largeur, qui sépare deux sections d'un panneau.
    void addHairline(QVBoxLayout* column);
    void applyWheelValues();

    std::unique_ptr<Ui::RpgCharacterSheetPlate> _ui;
    RpgScreenDescriptor _descriptor;
    std::vector<TranslatedLabel> _translated;
    std::map<std::string, std::string> _lastValues;

    /// Les libellés des six sièges, traduits. Ils sont **peints** par la roue et non portés par des
    /// étiquettes : ils ne peuvent donc pas passer par `_translated`.
    std::array<QString, ABILITY_SEAT_COUNT> _abilityLabels{};
    /// La dernière clé de portrait chargée, pour ne pas relire le fichier à chaque pose de valeurs.
    QString _loadedPortraitKey;

    // --- Colonne gauche ---
    QLabel* _levelValue = nullptr;
    SheetGauge* _experienceGauge = nullptr;
    SheetGauge* _vitalityGauge = nullptr;
    std::array<PipRow*, ABILITY_SEAT_COUNT> _saves{};
    QLabel* _enlistedOn = nullptr;
    QLabel* _countersigned = nullptr;

    // --- Colonne droite ---
    ShieldValue* _armorClass = nullptr;
    /// Initiative, vitesse, bonus de maîtrise, perception passive — dans cet ordre.
    std::array<StatMedallion*, 4> _combat{};
    std::vector<PipRow*> _skills;

    // --- En-tête ---
    QLabel* _matricule = nullptr;
    QLabel* _rank = nullptr;
};

}  // namespace hmi
