// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/RpgCharacterSheetPlate.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QStringList>
#include <QVBoxLayout>
#include <algorithm>
#include <array>
#include <filesystem>
#include <string_view>

#include "HMI/Interface/AbilityWheel.h"
#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/KeyHintText.h"
#include "HMI/Interface/MenuEntryButton.h"
#include "HMI/Interface/SheetWidgets.h"
#include "HMI/Interface/TitleBanner.h"
#include "HMI/Localization/Localization.h"
#include "HMI/Platform/ExecutableDirectory.h"
#include "ui_RpgCharacterSheetPlate.h"

namespace hmi {
namespace {

/// Le tiret cadratin, seul contenu d'un champ que rien n'alimente. Jamais un zéro.
const QString EMPTY_VALUE = QString::fromUtf8("—");

/// Les six caractéristiques, dans l'ordre des sièges de la roue — celui de la planche, et celui de
/// `core::Ability`. Le même ordre sert aux six sauvegardes.
constexpr std::array<std::string_view, ABILITY_SEAT_COUNT> ABILITY_SUFFIXES = {
    "strength", "dexterity", "constitution", "intelligence", "wisdom", "charisma"};

constexpr std::array<const char*, ABILITY_SEAT_COUNT> ABILITY_LABEL_KEYS = {
    "rpg.ability.strength",     "rpg.ability.dexterity", "rpg.ability.constitution",
    "rpg.ability.intelligence", "rpg.ability.wisdom",    "rpg.ability.charisma"};

/// Un médaillon de combat : son intitulé et son identifiant de valeur. Les identifiants sont ceux
/// de `COMBAT_FIELDS` dans la table, mot pour mot — la planche n'en invente aucun.
struct CombatDial {
    const char* labelKey;
    const char* valueId;
};
constexpr std::array<CombatDial, 4> COMBAT_DIALS = {{
    {"rpg.field.initiative", "sheet.initiative"},
    {"rpg.field.speed", "sheet.speed"},
    {"rpg.field.proficiency_bonus", "sheet.proficiency_bonus"},
    {"rpg.field.passive_perception", "sheet.passive_perception"},
}};

/// Clé de titre du bloc des compétences : la planche y prend l'ordre et les libellés des dix-huit
/// lignes, plutôt que de tenir sa propre liste — qui divergerait de la table au premier ajout.
constexpr std::string_view SKILLS_BLOCK = "rpg.block.skills";

void setRole(QWidget* widget, const char* role) {
    widget->setProperty("rpgRole", QString::fromLatin1(role));
}

[[nodiscard]] QString valueOr(const std::map<std::string, std::string>& values,
                              const std::string& key) {
    const auto found = values.find(key);
    return found == values.end() ? QString() : QString::fromStdString(found->second);
}

/// @return Le rapport @p numerator / @p denominator, ou 0 si l'un manque ou ne se lit pas. Jamais
///         une jauge pleine par défaut : une barre pleine affirme quelque chose.
[[nodiscard]] double ratioOf(const std::map<std::string, std::string>& values,
                             const std::string& numerator, const std::string& denominator) {
    bool numeratorOk = false;
    bool denominatorOk = false;
    const double top = valueOr(values, numerator).toDouble(&numeratorOk);
    const double bottom = valueOr(values, denominator).toDouble(&denominatorOk);
    if (!numeratorOk || !denominatorOk || bottom <= 0.0) {
        return 0.0;
    }
    return top / bottom;
}

}  // namespace

RpgCharacterSheetPlate::RpgCharacterSheetPlate(const RpgScreenDescriptor& descriptor,
                                               QWidget* parent)
    : QWidget(parent),
      _ui(std::make_unique<Ui::RpgCharacterSheetPlate>()),
      _descriptor(descriptor) {
    setObjectName(QString::fromLatin1(_descriptor.objectName));
    setAttribute(Qt::WA_StyledBackground, true);
    setFocusPolicy(Qt::StrongFocus);
    _ui->setupUi(this);

    const SpacingTokens& spacing = identityTokens().spacing;
    const int scale = identityScale();
    // Les MÊMES marges et le MÊME espacement que `hmi::RpgScreenFrame`, qui habille les huit autres
    // écrans. La planche s'était donné les siens -- plus serrés en haut et en bas, plus étroits à
    // droite -- et c'est ce qui la faisait lire comme une page importée : rien ne s'y alignait sur
    // la fenêtre voisine, alors que le contenu, lui, était le même genre de contenu.
    _ui->plateLayout->setContentsMargins(spacing.extraLarge * scale, spacing.large * scale,
                                         spacing.extraLarge * scale, spacing.large * scale);
    _ui->plateLayout->setSpacing(spacing.large * scale);

    // Ni la zone défilante ni les rappels de touches ne contraignent la fenêtre (EX-IHM-080).
    _ui->bodyScroll->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    _ui->hintsLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    // Le bandeau ne s'étire pas : ses ailes suivent sa hauteur, et étiré il devient deux ailes
    // démesurées autour d'une plaque restée fine.
    _ui->rpgTitle->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    _ui->abilityWheel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Le matricule et le rang, à droite du bandeau. Le matricule est tiré à la création du
    // personnage, le rang viendra de la Guilde (`LOT-45`) : ni l'un ni l'autre n'a de source
    // aujourd'hui, et tous deux restent au tiret cadratin. Un « M-3-0417 » d'exemple se lirait
    // comme un état du jeu et mentirait.
    _ui->headerRow->setSpacing(spacing.large * scale);
    setRole(_ui->registryLabel, "field");
    auto* const stamps = new QHBoxLayout();
    stamps->setSpacing(spacing.extraLarge * scale);
    for (QLabel** slot : {&_matricule, &_rank}) {
        auto* const caption = new QLabel(this);
        setRole(caption, "field");
        auto* const value = new QLabel(EMPTY_VALUE, this);
        setRole(value, "value");
        auto* const pair = new QHBoxLayout();
        pair->setSpacing(spacing.small * scale);
        pair->addWidget(caption);
        pair->addWidget(value);
        stamps->addLayout(pair);
        *slot = caption;
    }
    _ui->headerRow->addLayout(stamps);

    // Cinq éléments empilés : bandeau, ligne de délivrance, onglets, corps, pied. TOUT l'espace
    // libre va au corps (indice 3). Un poids posé sur le mauvais indice ne lève aucune erreur — il
    // donne simplement la fenêtre entière à la barre d'onglets, et le corps disparaît.
    _ui->tabBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    _ui->plateLayout->setStretch(0, 0);
    _ui->plateLayout->setStretch(1, 0);
    _ui->plateLayout->setStretch(2, 0);
    _ui->plateLayout->setStretch(3, 1);
    _ui->plateLayout->setStretch(4, 0);

    // Les rappels de touches prennent TOUT le reste de la largeur du pied, et n'en exigent aucune.
    // Sans ce poids, ils recevaient zéro pixel et n'apparaissaient jamais — alors que les huit
    // autres écrans les affichent, et que le pied annonce LB/RB. Un rappel qui manque sur un seul
    // écran est pire qu'un rappel absent partout : il apprend au joueur à ne pas les chercher.
    _ui->footerRow->setStretch(1, 0);
    _ui->footerRow->setStretch(2, 1);
    _ui->bodyRow->setStretch(0, 4);
    _ui->bodyRow->setStretch(1, 7);
    _ui->bodyRow->setStretch(2, 4);
    _ui->equipmentRow->setStretch(0, 4);
    _ui->equipmentRow->setStretch(1, 7);
    _ui->equipmentRow->setStretch(2, 4);

    // Le RETRAIT intérieur des quatre panneaux, posé ici et non laissé au défaut de Qt. Le défaut
    // vaut onze pixels, qui ne suivent pas le facteur d'agrandissement : à l'échelle 2, les
    // panneaux de la fiche respiraient donc deux fois moins que ceux des huit autres écrans, dont
    // `hmi::RpgScreenFrame` fixe le retrait à `large × échelle`. Le contenu paraissait collé au
    // cadre, et c'est une des choses qui distinguaient la fiche du reste au premier coup d'œil.
    const int inset = spacing.large * scale;
    for (QLayout* const panelLayout :
         {static_cast<QLayout*>(_ui->leftColumnLayout),
          static_cast<QLayout*>(_ui->rightColumnLayout),
          static_cast<QLayout*>(_ui->wheelPanelLayout),
          static_cast<QLayout*>(_ui->appearanceLayout), static_cast<QLayout*>(_ui->historyLayout),
          static_cast<QLayout*>(_ui->equipmentCenterLayout)}) {
        panelLayout->setContentsMargins(inset, inset, inset, inset);
    }

    buildLeftColumn();
    buildRightColumn();
    buildEquipmentPage();

    // Deux onglets, parce que deux sections existent. En annoncer cinq alors que trois ne mènent
    // nulle part serait promettre ce que l'écran ne tient pas -- le défaut même que le tiret
    // cadratin évite ailleurs.
    connect(_ui->tabBar, &SheetTabBar::currentChanged, _ui->bodyStack,
            &QStackedWidget::setCurrentIndex);

    connect(_ui->closeButton, &QPushButton::clicked, this, &RpgCharacterSheetPlate::closeRequested);
    _ui->closeButton->setAutoDefault(true);
}

RpgCharacterSheetPlate::~RpgCharacterSheetPlate() = default;

QWidget* RpgCharacterSheetPlate::widget() {
    return this;
}

QLabel* RpgCharacterSheetPlate::addField(QVBoxLayout* column, const char* key) {
    auto* const row = new QHBoxLayout();
    auto* const caption = new QLabel(this);
    setRole(caption, "field");
    auto* const value = new QLabel(EMPTY_VALUE, this);
    setRole(value, "value");
    row->addWidget(caption);
    row->addStretch(1);
    row->addWidget(value);
    column->addLayout(row);
    _translated.push_back({.label = caption, .key = key});
    return value;
}

void RpgCharacterSheetPlate::buildEquipmentPage() {
    const SpacingTokens& spacing = identityTokens().spacing;
    const int scale = identityScale();

    // --- Colonne gauche : ce que le personnage EST, par opposition à ce qu'il porte -------------
    QVBoxLayout* const appearance = _ui->appearanceLayout;
    appearance->setSpacing(spacing.small * scale);
    addHeading(appearance, "rpg.block.appearance");
    // Les six champs d'apparence n'ont aucune source : ils attendent un portrait de personnage.
    // Ils restent au tiret, et la colonne des tirets EST le périmètre restant.
    for (const char* const key : {"rpg.field.age", "rpg.field.height", "rpg.field.weight",
                                  "rpg.field.eyes", "rpg.field.skin", "rpg.field.hair"}) {
        addField(appearance, key);
    }
    addHairline(appearance);
    addHeading(appearance, "rpg.block.languages");
    _languages = new QLabel(EMPTY_VALUE, this);
    setRole(_languages, "prose");
    _languages->setWordWrap(true);
    appearance->addWidget(_languages);
    addHairline(appearance);
    addHeading(appearance, "rpg.block.other_proficiencies");
    auto* const others = new QLabel(EMPTY_VALUE, this);
    setRole(others, "value");
    appearance->addWidget(others);
    appearance->addStretch(1);

    // --- Centre : le bouclier, et les seize emplacements de part et d'autre ---------------------
    //
    // Les emplacements sont ceux de l'écran d'inventaire (`hmi::rpgScreens`), pas une liste tenue
    // ici. La deuxième planche du corpus est radiale elle aussi : là où la première pose un
    // portrait au centre d'un arc, celle-ci pose un bouclier au centre de ses emplacements.
    QVBoxLayout* const center = _ui->equipmentCenterLayout;
    center->setSpacing(spacing.medium * scale);

    std::vector<const RpgField*> slotFields;
    for (const RpgContentBlock& block :
         rpgScreenDescriptor(RpgScreenId::Inventory).layout.leftColumn) {
        if (std::string_view(block.titleKey) != "rpg.block.equipment") {
            continue;
        }
        for (const RpgField& field : block.fields) {
            slotFields.push_back(&field);
        }
    }

    auto* const slotsRow = new QHBoxLayout();
    slotsRow->setSpacing(spacing.medium * scale);
    auto* const leftSlots = new QVBoxLayout();
    auto* const rightSlots = new QVBoxLayout();
    leftSlots->setSpacing(spacing.small * scale);
    rightSlots->setSpacing(spacing.small * scale);

    const std::size_t half = (slotFields.size() + 1) / 2;
    for (std::size_t index = 0; index < slotFields.size(); ++index) {
        const bool mirrored = index >= half;
        // Le nom de l'emplacement est la fin de son identifiant de valeur : `inventory.slot.head`
        // donne `head`, qui est aussi le nom du fichier de l'icône. Le lien se lit.
        const std::string valueId = slotFields.at(index)->valueId;
        const std::size_t dot = valueId.rfind('.');
        const QString slotName =
            dot == std::string::npos ? QString() : QString::fromStdString(valueId.substr(dot + 1));
        auto* const row = new EquipmentSlotRow(slotName, mirrored, this);
        (mirrored ? rightSlots : leftSlots)->addWidget(row);
        _slots.push_back(row);
    }
    leftSlots->addStretch(1);
    rightSlots->addStretch(1);

    slotsRow->addLayout(leftSlots, 1);
    slotsRow->addWidget(new RoundShield(this), 0, Qt::AlignCenter);
    slotsRow->addLayout(rightSlots, 1);
    center->addLayout(slotsRow);

    _loadGauge = new SheetGauge(SheetGauge::Tone::Progress, this);
    center->addWidget(_loadGauge);

    auto* const purseRow = new QHBoxLayout();
    auto* const purseCaption = new QLabel(this);
    setRole(purseCaption, "field");
    _purse = new QLabel(EMPTY_VALUE, this);
    setRole(_purse, "value");
    purseRow->addWidget(purseCaption);
    purseRow->addStretch(1);
    purseRow->addWidget(_purse);
    _translated.push_back({.label = purseCaption, .key = "rpg.field.gold"});
    center->addLayout(purseRow);

    addHeading(center, "rpg.block.bag");
    _backpack = new QLabel(EMPTY_VALUE, this);
    setRole(_backpack, "prose");
    _backpack->setWordWrap(true);
    center->addWidget(_backpack);
    center->addStretch(1);

    // --- Colonne droite : d'où il vient --------------------------------------------------------
    QVBoxLayout* const history = _ui->historyLayout;
    history->setSpacing(spacing.small * scale);
    addHeading(history, "rpg.block.backstory");
    _backgroundFeature = new QLabel(EMPTY_VALUE, this);
    setRole(_backgroundFeature, "block");
    history->addWidget(_backgroundFeature);
    _backgroundText = new QLabel(EMPTY_VALUE, this);
    setRole(_backgroundText, "prose");
    _backgroundText->setWordWrap(true);
    history->addWidget(_backgroundText);
    addHairline(history);
    addHeading(history, "rpg.block.personality");
    for (const char* const key :
         {"rpg.field.traits", "rpg.field.ideals", "rpg.field.bonds", "rpg.field.flaws"}) {
        addField(history, key);
    }
    history->addStretch(1);
}

QLabel* RpgCharacterSheetPlate::addHeading(QVBoxLayout* column, const char* key) {
    auto* const heading = new QLabel(this);
    setRole(heading, "block");
    column->addWidget(heading);
    _translated.push_back({.label = heading, .key = key});
    return heading;
}

void RpgCharacterSheetPlate::addHairline(QVBoxLayout* column) {
    auto* const line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    line->setFixedHeight(std::max(1, identityScale()));
    setRole(line, "hairline");
    column->addWidget(line);
}

void RpgCharacterSheetPlate::buildLeftColumn() {
    const SpacingTokens& spacing = identityTokens().spacing;
    const int scale = identityScale();
    QVBoxLayout* const column = _ui->leftColumnLayout;
    column->setSpacing(spacing.small * scale);

    addHeading(column, "rpg.block.progression");

    // Le niveau en grand, l'expérience en jauge à côté : la composition de la planche, qui dit d'un
    // coup d'œil où l'on en est sans demander de comparer deux nombres.
    auto* const progression = new QHBoxLayout();
    progression->setSpacing(spacing.large * scale);
    auto* const levelBlock = new QVBoxLayout();
    levelBlock->setSpacing(0);
    // Le bloc du niveau reclame sa largeur : sans elle, la jauge d'experience -- extensible --
    // prend tout, et le chiffre du niveau se retrouve rogne a mi-hauteur.
    levelBlock->setContentsMargins(0, 0, spacing.small * scale, 0);
    auto* const levelCaption = new QLabel(this);
    setRole(levelCaption, "field");
    _levelValue = new QLabel(EMPTY_VALUE, this);
    setRole(_levelValue, "bigValue");
    _levelValue->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    _levelValue->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    levelBlock->addWidget(levelCaption);
    levelBlock->addWidget(_levelValue);
    _translated.push_back({.label = levelCaption, .key = "rpg.field.level"});
    progression->addLayout(levelBlock);

    _experienceGauge = new SheetGauge(SheetGauge::Tone::Progress, this);
    progression->addWidget(_experienceGauge, 1);
    column->addLayout(progression);

    _vitalityGauge = new SheetGauge(SheetGauge::Tone::Vitality, this);
    column->addWidget(_vitalityGauge);

    addHairline(column);
    addHeading(column, "rpg.block.saving_throws");
    for (PipRow*& row : _saves) {
        row = new PipRow(this);
        column->addWidget(row);
    }

    column->addStretch(1);

    // Le contreseing ferme le panneau : c'est ce qui fait du feuillet une pièce délivrée plutôt
    // qu'une fiche imprimée. Les deux lignes restent au tiret — rien ne les alimente.
    addHairline(column);
    auto* const countersign = new QHBoxLayout();
    countersign->setSpacing(spacing.medium * scale);
    auto* const lines = new QVBoxLayout();
    lines->setSpacing(spacing.small * scale);
    for (QLabel** slot : {&_enlistedOn, &_countersigned}) {
        auto* const caption = new QLabel(this);
        setRole(caption, "field");
        auto* const rule = new QLabel(EMPTY_VALUE, this);
        setRole(rule, "signature");
        lines->addWidget(caption);
        lines->addWidget(rule);
        *slot = caption;
    }
    countersign->addLayout(lines, 1);
    countersign->addWidget(new WaxSeal(this), 0, Qt::AlignBottom);
    column->addLayout(countersign);
}

void RpgCharacterSheetPlate::buildRightColumn() {
    const SpacingTokens& spacing = identityTokens().spacing;
    const int scale = identityScale();
    QVBoxLayout* const column = _ui->rightColumnLayout;
    column->setSpacing(spacing.small * scale);

    addHeading(column, "rpg.block.combat");

    // L'écu porte la classe d'armure, les quatre médaillons portent le reste. Les points de vie
    // sont passés à GAUCHE, en jauge : c'est la seule valeur de combat qui change en jouant, et
    // une jauge la donne d'un coup d'œil là où un nombre demande une comparaison.
    auto* const defence = new QHBoxLayout();
    defence->setSpacing(spacing.medium * scale);
    _armorClass = new ShieldValue(this);
    defence->addWidget(_armorClass, 0, Qt::AlignTop);

    auto* const dials = new QGridLayout();
    dials->setSpacing(spacing.small * scale);
    for (std::size_t index = 0; index < COMBAT_DIALS.size(); ++index) {
        auto* const dial = new StatMedallion(this);
        dials->addWidget(dial, static_cast<int>(index / 2), static_cast<int>(index % 2));
        _combat.at(index) = dial;
    }
    defence->addLayout(dials, 1);
    column->addLayout(defence);

    addHairline(column);
    addHeading(column, "rpg.block.skills");

    // Les dix-huit lignes viennent de la TABLE, avec leur libellé : la planche n'en devine aucune.
    for (const RpgContentBlock& block : _descriptor.layout.rightColumn) {
        if (std::string_view(block.titleKey) != SKILLS_BLOCK) {
            continue;
        }
        for ([[maybe_unused]] const RpgField& field : block.fields) {
            auto* const row = new PipRow(this);
            column->addWidget(row);
            _skills.push_back(row);
        }
    }
    column->addStretch(1);
}

void RpgCharacterSheetPlate::retranslateUi(const Localization& loc) {
    const auto t = [&loc](const char* key) { return QString::fromStdString(loc.text(key)); };

    _ui->rpgTitle->setText(t(_descriptor.titleKey));
    _ui->registryLabel->setText(t("rpg.plate.registry"));
    _ui->abilityWheel->setStamp(t("rpg.plate.enlisted_stamp"));
    _ui->closeButton->setText(t("rpg.chassis.close"));
    _ui->hintsLabel->setText(QString::fromStdString(hmi::keyHintText(
        {
            {.key = loc.text("key.shoulders"), .action = loc.text("hint.change_screen")},
            {.key = loc.text("key.confirm"), .action = loc.text("hint.confirm")},
            {.key = loc.text("key.escape"), .action = loc.text("hint.back")},
        },
        hmi::identityTokens(), hmi::identityScale())));

    // Les deux onglets existants. Leurs intitulés sont ceux des blocs de la table -- ce sont
    // les mêmes sections, vues de l'extérieur.
    _ui->tabBar->setTabs({t("rpg.block.abilities"), t("rpg.block.equipment")});

    // Les intitulés des seize emplacements viennent de la table de l'écran d'inventaire, dans son
    // ordre : le même parcours qu'à la construction, donc les mêmes lignes.
    std::size_t slotIndex = 0;
    for (const RpgContentBlock& block :
         rpgScreenDescriptor(RpgScreenId::Inventory).layout.leftColumn) {
        if (std::string_view(block.titleKey) != "rpg.block.equipment") {
            continue;
        }
        for (const RpgField& field : block.fields) {
            if (slotIndex < _slots.size()) {
                _slots.at(slotIndex)->setCaption(t(field.labelKey));
            }
            ++slotIndex;
        }
    }
    _loadGauge->setLabel(t("rpg.field.carried"));

    _matricule->setText(t("rpg.plate.matricule"));
    _rank->setText(t("rpg.plate.rank"));
    _enlistedOn->setText(t("rpg.plate.enlisted_on"));
    _countersigned->setText(t("rpg.plate.countersigned"));

    for (const TranslatedLabel& entry : _translated) {
        entry.label->setText(t(entry.key));
    }

    _experienceGauge->setLabel(t("rpg.field.experience"));
    _vitalityGauge->setLabel(t("rpg.field.hit_points"));
    _armorClass->setCaption(t("rpg.field.armor_class"));
    for (std::size_t index = 0; index < COMBAT_DIALS.size(); ++index) {
        _combat.at(index)->setCaption(t(COMBAT_DIALS.at(index).labelKey));
    }
    for (std::size_t index = 0; index < ABILITY_SEAT_COUNT; ++index) {
        _abilityLabels.at(index) = t(ABILITY_LABEL_KEYS.at(index));
        _saves.at(index)->setLabel(_abilityLabels.at(index));
    }

    std::size_t skillIndex = 0;
    for (const RpgContentBlock& block : _descriptor.layout.rightColumn) {
        if (std::string_view(block.titleKey) != SKILLS_BLOCK) {
            continue;
        }
        for (const RpgField& field : block.fields) {
            if (skillIndex < _skills.size()) {
                _skills.at(skillIndex)->setLabel(t(field.labelKey));
            }
            ++skillIndex;
        }
    }

    applyWheelValues();
}

void RpgCharacterSheetPlate::setValues(const std::map<std::string, std::string>& values) {
    _lastValues = values;

    const QString level = valueOr(values, "sheet.level");
    _levelValue->setText(level.isEmpty() ? EMPTY_VALUE : level);

    // La jauge d'expérience se remplit vers le SEUIL DU NIVEAU SUIVANT, jamais vers un maximum
    // absolu : c'est ce que « progresser » veut dire ici.
    _experienceGauge->setValue(valueOr(values, "sheet.experience") + " / " +
                               valueOr(values, "sheet.experience_next"));
    _experienceGauge->setFill(ratioOf(values, "sheet.experience", "sheet.experience_next"));

    _vitalityGauge->setValue(valueOr(values, "sheet.hit_points"));
    _vitalityGauge->setFill(ratioOf(values, "sheet.hit_points_current", "sheet.hit_points_max"));

    _armorClass->setValue(valueOr(values, "sheet.armor_class"));
    for (std::size_t index = 0; index < COMBAT_DIALS.size(); ++index) {
        _combat.at(index)->setValue(valueOr(values, COMBAT_DIALS.at(index).valueId));
    }

    for (std::size_t index = 0; index < ABILITY_SEAT_COUNT; ++index) {
        const std::string root = "sheet.save." + std::string(ABILITY_SUFFIXES.at(index));
        _saves.at(index)->setValue(valueOr(values, root));
        _saves.at(index)->setProficient(!valueOr(values, root + ".proficient").isEmpty());
    }

    std::size_t skillIndex = 0;
    for (const RpgContentBlock& block : _descriptor.layout.rightColumn) {
        if (std::string_view(block.titleKey) != SKILLS_BLOCK) {
            continue;
        }
        for (const RpgField& field : block.fields) {
            if (skillIndex < _skills.size()) {
                const std::string id = field.valueId;
                // La valeur de texte porte encore la maîtrise en caractère (« +4 • ») pour les
                // écrans qui l'affichent en liste. La planche, elle, lit le DRAPEAU et dessine la
                // forme : le caractère ferait doublon avec la pastille peinte.
                const QString reading = valueOr(values, id).split(QString::fromUtf8(" •")).first();
                _skills.at(skillIndex)->setValue(reading);
                _skills.at(skillIndex)
                    ->setProficient(!valueOr(values, id + ".proficient").isEmpty());
            }
            ++skillIndex;
        }
    }

    // --- Page « Équipement » ---
    std::size_t slotIndex = 0;
    for (const RpgContentBlock& block :
         rpgScreenDescriptor(RpgScreenId::Inventory).layout.leftColumn) {
        if (std::string_view(block.titleKey) != "rpg.block.equipment") {
            continue;
        }
        for (const RpgField& field : block.fields) {
            if (slotIndex < _slots.size()) {
                const QString item = valueOr(values, field.valueId);
                _slots.at(slotIndex)->setItem(item.isEmpty() ? EMPTY_VALUE : item);
            }
            ++slotIndex;
        }
    }

    // La charge se remplit vers la CAPACITÉ, en poids brut : les deux chaines lisibles
    // (« 23,5 kg ») sont pour l'œil, le rapport est pour la barre.
    _loadGauge->setValue(valueOr(values, "inventory.carried") + " / " +
                         valueOr(values, "inventory.capacity"));
    _loadGauge->setFill(ratioOf(values, "inventory.carried_grams", "inventory.capacity_grams"));

    for (const auto& [label, key] :
         {std::pair{_purse, "inventory.purse"}, std::pair{_backpack, "inventory.backpack"},
          std::pair{_languages, "sheet.languages"},
          std::pair{_backgroundFeature, "sheet.background_feature"},
          std::pair{_backgroundText, "sheet.background_feature_text"}}) {
        const QString reading = valueOr(values, key);
        label->setText(reading.isEmpty() ? EMPTY_VALUE : reading);
    }

    applyWheelValues();
}

void RpgCharacterSheetPlate::applyWheelValues() {
    AbilityWheel* const wheel = _ui->abilityWheel;

    const QString name = valueOr(_lastValues, "sheet.name");
    QStringList parts;
    for (const QString& part :
         {valueOr(_lastValues, "sheet.species"), valueOr(_lastValues, "sheet.class"),
          valueOr(_lastValues, "sheet.background")}) {
        if (!part.isEmpty() && part != EMPTY_VALUE) {
            parts << part;
        }
    }
    wheel->setIdentity(name.isEmpty() ? EMPTY_VALUE : name, parts.join(QString::fromUtf8("  ·  ")));

    // Le portrait est désigné par une CLÉ d'asset (`character/<id>`, `LOT-39`), jamais par un
    // chemin : la fiche ne sait pas où les illustrations sont rangées, et n'a pas à le savoir.
    const QString portraitKey = valueOr(_lastValues, "sheet.portrait");
    if (portraitKey != _loadedPortraitKey) {
        _loadedPortraitKey = portraitKey;
        QPixmap portrait;
        if (!portraitKey.isEmpty()) {
            const std::filesystem::path file = hmi::executableDirectory() / "Assets" / "Entities" /
                                               (portraitKey.toStdString() + ".png");
            // Un chargement raté laisse le pixmap NUL, et la roue rend alors le marqueur du
            // `LOT-39` : une illustration absente est un état d'avancement, pas une panne.
            portrait.load(QString::fromStdString(file.string()));
        }
        wheel->setPortrait(portrait);
    }

    for (std::size_t index = 0; index < ABILITY_SEAT_COUNT; ++index) {
        const std::string root = "sheet.ability." + std::string(ABILITY_SUFFIXES.at(index));
        wheel->setSeat(wheelSeatAt(index), _abilityLabels.at(index),
                       valueOr(_lastValues, root + ".score"),
                       valueOr(_lastValues, root + ".modifier"));
    }
}

void RpgCharacterSheetPlate::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    const ColorTokens& color = identityTokens().color;
    const int scale = identityScale();
    const QColor ornament(color.frameOrnament.r, color.frameOrnament.g, color.frameOrnament.b);
    const QColor ground(color.background.r, color.background.g, color.background.b);

    const QRect body = _ui->bodyScroll->geometry();

    // Les deux filets d'or qui soulignaient l'en-tête et le pied ont été RETIRÉS. Aucun autre écran
    // n'en porte : ce sont les encadrements des panneaux qui séparent les zones, partout ailleurs.
    // Deux filets de plus sur le seul écran de la fiche ajoutaient une règle de lecture qui ne
    // valait que là, et c'est précisément ce qui la détachait du reste de l'interface.

    // La souche perforée : le bord par lequel le feuillet a quitté son registre. Un trait rompu,
    // et cinq trous en creux. Elle reste — c'est le seul trait qui dise « pièce délivrée » plutôt
    // que « écran de jeu », et elle vit dans la MARGE, sans rien déplacer de la composition.
    const int dashX = identityTokens().spacing.medium * scale;
    QPen dashed(ornament, std::max(1, scale));
    dashed.setDashPattern({3.0, 3.0});
    painter.setPen(dashed);
    painter.drawLine(dashX, body.top(), dashX, body.bottom());

    const int holeSide = 4 * scale;
    const int holeX = dashX - (holeSide + (2 * scale));
    constexpr int HOLE_COUNT = 5;
    painter.setPen(Qt::NoPen);
    painter.setBrush(ground.darker(112));
    for (int index = 0; index < HOLE_COUNT; ++index) {
        const int y = body.top() + ((body.height() * (index + 1)) / (HOLE_COUNT + 1));
        painter.drawEllipse(QRect(holeX, y - (holeSide / 2), holeSide, holeSide));
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
