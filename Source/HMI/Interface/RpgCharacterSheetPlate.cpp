// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/RpgCharacterSheetPlate.h"

#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QStringList>
#include <QVBoxLayout>
#include <array>
#include <string_view>

#include "HMI/Interface/AbilityWheel.h"
#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/KeyHintText.h"
#include "HMI/Interface/MenuEntryButton.h"
#include "HMI/Interface/TitleBanner.h"
#include "HMI/Localization/Localization.h"
#include "ui_RpgCharacterSheetPlate.h"

namespace hmi {
namespace {

/// Le tiret cadratin, seul contenu d'un champ que rien n'alimente.
const QString EMPTY_VALUE = QString::fromUtf8("—");

/// Les deux blocs de l'ossature que la **roue** reprend, désignés par leur clé de titre. Les
/// nommer ici et non par leur position : insérer un bloc dans la table ne doit pas déplacer ce que
/// la planche rend autrement.
constexpr std::string_view IDENTITY_BLOCK = "rpg.block.identity";
constexpr std::string_view ABILITIES_BLOCK = "rpg.block.abilities";

/// Les six caractéristiques, dans l'ordre des sièges de la roue -- qui est celui de la planche, et
/// celui de `core::Ability`. La correspondance est positionnelle et le reste : la table des champs
/// (`ABILITY_FIELDS`) suit le même ordre, et un test du `LOT-68` le tient.
constexpr std::array<std::string_view, ABILITY_SEAT_COUNT> ABILITY_SUFFIXES = {
    "strength", "dexterity", "constitution", "intelligence", "wisdom", "charisma"};

constexpr std::array<const char*, ABILITY_SEAT_COUNT> ABILITY_LABEL_KEYS = {
    "rpg.ability.strength",     "rpg.ability.dexterity", "rpg.ability.constitution",
    "rpg.ability.intelligence", "rpg.ability.wisdom",    "rpg.ability.charisma"};

void setRole(QWidget* widget, const char* role) {
    widget->setProperty("rpgRole", QString::fromLatin1(role));
}

[[nodiscard]] QString valueOr(const std::map<std::string, std::string>& values,
                              const std::string& key) {
    const auto found = values.find(key);
    return found == values.end() ? QString() : QString::fromStdString(found->second);
}

}  // namespace

RpgCharacterSheetPlate::RpgCharacterSheetPlate(const RpgScreenDescriptor& descriptor,
                                               QWidget* parent)
    : QWidget(parent),
      _ui(std::make_unique<Ui::RpgCharacterSheetPlate>()),
      _descriptor(descriptor) {
    setObjectName(QString::fromLatin1(_descriptor.objectName));
    setAttribute(Qt::WA_StyledBackground, true);
    // La planche reçoit le clavier elle-même : sans cela, `Échap` n'atteint jamais keyPressEvent
    // quand le focus est sur un bouton qui ne gère pas cette touche.
    setFocusPolicy(Qt::StrongFocus);
    _ui->setupUi(this);

    // Ni la zone defilante ni les rappels de touches ne doivent contraindre la fenetre : la
    // planche est plus grande qu'un 1280x720 et le restera, mais un ecran ne decide pas de la
    // taille de la fenetre (EX-IHM-080). Meme reglage que RpgScreenFrame, et pour la meme raison
    // -- sans lui, la fenetre ne peut plus retrecir sous la taille de la roue et de ses panneaux.
    _ui->bodyScroll->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    _ui->hintsLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    // Le bandeau ne s'etire PAS en hauteur : ses ailes suivent sa hauteur (LOT-76), et un bandeau
    // etire sur la hauteur libre de la fenetre devient deux ailes d'or demesurees de part et
    // d'autre d'une plaque restee fine. Le defaut se voit tout de suite, et seulement a l'ecran.
    _ui->rpgTitle->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // La roue prend la place qu'on lui donne. Sans ceci, les deux panneaux extensibles se
    // partagent toute la largeur, la roue tombe sous son plancher de lisibilite et ne peint
    // RIEN -- l'ecran parait alors vide en son centre, sans qu'aucune erreur ne soit levee.
    _ui->abilityWheel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Les poids de repartition, en code : la propriete `stretch` d'un `.ui` produit un appel que
    // uic ne sait pas ecrire pour un QBoxLayout.
    //
    // En hauteur, TOUT l'espace libre va au corps (indice 1) : sinon l'en-tete se l'approprie et
    // le bandeau s'etire. En largeur, 4 / 7 / 4 -- les proportions de la maquette, 320 / 560 / 320
    // dans un 1280.
    _ui->plateLayout->setStretch(0, 0);
    _ui->plateLayout->setStretch(1, 1);
    _ui->plateLayout->setStretch(2, 0);
    _ui->bodyRow->setStretch(0, 4);
    _ui->bodyRow->setStretch(1, 7);
    _ui->bodyRow->setStretch(2, 4);

    buildColumns();

    connect(_ui->closeButton, &QPushButton::clicked, this, &RpgCharacterSheetPlate::closeRequested);
    // Qt n'active `autoDefault` que sous un vrai QDialog. Sans ce réglage, un bouton qui a le
    // focus ne répond qu'à Espace, jamais à Entrée -- même correctif que sur les écrans de menu.
    _ui->closeButton->setAutoDefault(true);
}

RpgCharacterSheetPlate::~RpgCharacterSheetPlate() = default;

QWidget* RpgCharacterSheetPlate::widget() {
    return this;
}

void RpgCharacterSheetPlate::buildColumns() {
    for (const RpgContentBlock& block : _descriptor.layout.leftColumn) {
        buildBlock(block, _ui->leftColumnLayout);
    }
    for (const RpgContentBlock& block : _descriptor.layout.rightColumn) {
        buildBlock(block, _ui->rightColumnLayout);
    }
    _ui->leftColumnLayout->addStretch(1);
    _ui->rightColumnLayout->addStretch(1);
}

void RpgCharacterSheetPlate::buildBlock(const RpgContentBlock& block, QVBoxLayout* column) {
    const std::string_view title = block.titleKey;
    if (title == IDENTITY_BLOCK || title == ABILITIES_BLOCK) {
        return;  // La roue les porte : les rendre aussi en lignes les afficherait deux fois.
    }

    if (block.titleKey[0] != '\0') {
        auto* const heading = new QLabel(this);
        setRole(heading, "block");
        column->addWidget(heading);
        _translated.push_back({.label = heading, .key = block.titleKey});
    }

    switch (block.kind) {
        case RpgBlockKind::Fields: {
            auto* const grid = new QGridLayout();
            int row = 0;
            for (const RpgField& field : block.fields) {
                auto* const label = new QLabel(this);
                setRole(label, "field");
                auto* const value = new QLabel(EMPTY_VALUE, this);
                setRole(value, "value");
                grid->addWidget(label, row, 0);
                grid->addWidget(value, row, 1);
                _translated.push_back({.label = label, .key = field.labelKey});
                if (field.valueId[0] != '\0') {
                    _values.push_back({.label = value, .valueId = field.valueId});
                }
                ++row;
            }
            grid->setColumnStretch(1, 1);
            column->addLayout(grid);
            break;
        }
        case RpgBlockKind::List: {
            for (const char* const valueId : block.valueIds) {
                auto* const value = new QLabel(EMPTY_VALUE, this);
                setRole(value, "value");
                column->addWidget(value);
                if (valueId[0] != '\0') {
                    _values.push_back({.label = value, .valueId = valueId});
                }
            }
            break;
        }
        case RpgBlockKind::Grid:
        case RpgBlockKind::Prose:
        case RpgBlockKind::Portrait:
        case RpgBlockKind::Track:
        case RpgBlockKind::ActionBar:
            // Aucun de ces genres n'est sur la fiche. Les rendre « au cas où » poserait des cases
            // vides qu'aucune donnée ne remplirait, et le prochain lecteur les prendrait pour un
            // manque plutôt que pour du code sans emploi.
            break;
    }
}

void RpgCharacterSheetPlate::retranslateUi(const Localization& loc) {
    const auto t = [&loc](const char* key) { return QString::fromStdString(loc.text(key)); };

    _ui->rpgTitle->setText(t(_descriptor.titleKey));
    _ui->registryLabel->setText(t("rpg.plate.registry"));
    _ui->closeButton->setText(t("rpg.chassis.close"));
    _ui->hintsLabel->setText(QString::fromStdString(hmi::keyHintText(
        {
            {.key = loc.text("key.shoulders"), .action = loc.text("hint.change_screen")},
            {.key = loc.text("key.confirm"), .action = loc.text("hint.confirm")},
            {.key = loc.text("key.escape"), .action = loc.text("hint.back")},
        },
        hmi::identityTokens(), hmi::identityScale())));

    for (const TranslatedLabel& entry : _translated) {
        entry.label->setText(t(entry.key));
    }

    // Les libellés des six sièges sont PEINTS par la roue, pas portés par des étiquettes : ils ne
    // peuvent pas passer par `_translated`, et doivent être retenus pour être repassés à chaque
    // changement de valeurs.
    for (std::size_t index = 0; index < ABILITY_SEAT_COUNT; ++index) {
        _abilityLabels.at(index) = t(ABILITY_LABEL_KEYS.at(index));
    }

    // Les libellés des six sièges vivent sur la roue, qui les peint : elle doit donc être
    // repassée après un changement de langue, avec ses valeurs.
    applyWheelValues();
}

void RpgCharacterSheetPlate::setValues(const std::map<std::string, std::string>& values) {
    _lastValues = values;
    for (const ValueLabel& entry : _values) {
        const auto found = values.find(entry.valueId);
        entry.label->setText(found == values.end() ? EMPTY_VALUE
                                                   : QString::fromStdString(found->second));
    }
    applyWheelValues();
}

void RpgCharacterSheetPlate::applyWheelValues() {
    AbilityWheel* const wheel = _ui->abilityWheel;

    const QString name = valueOr(_lastValues, "sheet.name");
    const QString species = valueOr(_lastValues, "sheet.species");
    const QString characterClass = valueOr(_lastValues, "sheet.class");
    const QString background = valueOr(_lastValues, "sheet.background");
    QStringList parts;
    for (const QString& part : {species, characterClass, background}) {
        if (!part.isEmpty() && part != EMPTY_VALUE) {
            parts << part;
        }
    }
    wheel->setIdentity(name.isEmpty() ? EMPTY_VALUE : name, parts.join(QString::fromUtf8("  ·  ")));

    for (std::size_t index = 0; index < ABILITY_SEAT_COUNT; ++index) {
        const std::string root = "sheet.ability." + std::string(ABILITY_SUFFIXES.at(index));
        wheel->setSeat(wheelSeatAt(index), _abilityLabels.at(index),
                       valueOr(_lastValues, root + ".score"),
                       valueOr(_lastValues, root + ".modifier"));
    }
}

void RpgCharacterSheetPlate::focusDefaultAction() {
    _ui->closeButton->setFocus(Qt::OtherFocusReason);
}

void RpgCharacterSheetPlate::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        emit closeRequested();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

}  // namespace hmi
