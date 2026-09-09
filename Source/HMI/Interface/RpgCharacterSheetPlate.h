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
class EquipmentSlotRow;
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
    /// La page « Équipement » : le bouclier, les seize emplacements, la charge et le sac.
    void buildEquipmentPage();
    /// La page « Dons et traits » : ce que l'espèce, l'historique et la classe accordent.
    void buildTraitsPage();
    /// La page « Incantation » : ce qui gouverne les sorts, et les emplacements par niveau.
    void buildSpellsPage();
    /// La page « Équipe » : la compagnie, son blason et ses desseins (`LOT-45`, `LOT-83`).
    void buildTeamPage();
    /// Une ligne « intitulé / valeur » simple, pour les champs que rien n'alimente encore.
    QLabel* addField(QVBoxLayout* column, const char* key);
    /// Un intitulé de section, dans le style des titres de bloc.
    QLabel* addHeading(QVBoxLayout* column, const char* key);
    /// Un filet d'or pleine largeur, qui sépare deux sections d'un panneau.
    void addHairline(QVBoxLayout* column);
    /// Pose une ligne par langue, et masque les lignes en trop.
    void applyLanguages(const std::map<std::string, std::string>& values);
    /// Pose une ligne par pile du sac : l'objet, puis sa quantité.
    void applyBackpack(const std::map<std::string, std::string>& values);
    /// Pose les traits d'espèce, les maîtrises de l'historique et les aptitudes de classe.
    void applyTraits(const std::map<std::string, std::string>& values);
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

    // --- Page « Équipement » ---
    /// Les seize emplacements, dans l'ordre de la table de l'écran d'inventaire : c'est la même
    /// donnée qui les nomme, et un dix-septième apparaîtrait sans que cette planche soit touchée.
    std::vector<EquipmentSlotRow*> _slots;
    SheetGauge* _loadGauge = nullptr;
    QLabel* _purse = nullptr;

    /// Une ligne du sac : l'objet à gauche, sa quantité à droite.
    struct BagRow {
        QLabel* name = nullptr;
        QLabel* quantity = nullptr;
    };
    /// Les lignes du sac et celles des langues, créées **à la demande** quand les valeurs
    /// arrivent : leur nombre n'est pas connu à la construction, et une réserve de lignes fixée
    /// d'avance couperait le sac au onzième objet sans que rien ne le dise. Les lignes en trop
    /// sont masquées plutôt que détruites — un inventaire qui varie recréerait sinon des widgets
    /// à chaque ramassage.
    std::vector<BagRow> _bagRows;
    QVBoxLayout* _bagLayout = nullptr;
    std::vector<QLabel*> _languageRows;
    QVBoxLayout* _languagesLayout = nullptr;

    // --- Page « Dons et traits » ---
    /// Un bloc nommé : un intitulé en accent, son texte dessous. C'est la forme d'un trait
    /// d'espèce comme d'un don d'historique — les deux sont la même chose vue de deux sources.
    struct NamedBlock {
        QLabel* name = nullptr;
        QLabel* text = nullptr;
    };
    QLabel* _speciesName = nullptr;
    std::vector<NamedBlock> _speciesTraits;
    QVBoxLayout* _speciesTraitsLayout = nullptr;

    QLabel* _backgroundName = nullptr;
    QLabel* _backgroundFeature = nullptr;
    QLabel* _backgroundText = nullptr;
    std::vector<QLabel*> _backgroundSkills;
    QVBoxLayout* _backgroundSkillsLayout = nullptr;

    QLabel* _className = nullptr;

    /// Le seul membre d'équipe que la donnée porte aujourd'hui : le personnage lui-même. Les
    /// autres viendront de la Guilde (`LOT-45`).
    QLabel* _teamMember = nullptr;
    QLabel* _teamName = nullptr;
    /// Une aptitude de classe : le niveau où elle s'obtient, son identifiant, et le tiret cadratin
    /// à la place de son texte. Le catalogue qui les nommera n'existe pas encore.
    struct FeatureRow {
        QLabel* level = nullptr;
        QLabel* name = nullptr;
        QLabel* text = nullptr;
    };
    std::vector<FeatureRow> _classFeatures;
    QVBoxLayout* _classFeaturesLayout = nullptr;

    // --- En-tête ---
    QLabel* _matricule = nullptr;
    QLabel* _rank = nullptr;
};

}  // namespace hmi
