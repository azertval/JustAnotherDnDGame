// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/RpgScreenFrame.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QScrollArea>
#include <QSizePolicy>
#include <QString>
#include <QVBoxLayout>

#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/KeyHintText.h"
#include "HMI/Interface/MenuEntryButton.h"
#include "HMI/Interface/ParchmentPanel.h"
#include "HMI/Interface/TitleBanner.h"
#include "HMI/Localization/Localization.h"

namespace hmi {
namespace {

/// Rôle d'habillage d'une étiquette ou d'une case, posé en **propriété dynamique** et lu par la
/// feuille de style (`theme-identity.qss`). Un rôle, jamais un nom d'objet : l'ossature est
/// construite à partir d'une table, et nommer chaque widget reviendrait à tenir dans la feuille de
/// style la liste des blocs — qui divergerait au premier écran ajouté (même leçon que les rôles de
/// l'écran de crédits, `LOT-66`).
void setRole(QWidget* widget, const char* role) {
    widget->setProperty("rpgRole", QString::fromLatin1(role));
}

/// @return Le côté, en pixels, d'une case de grille au facteur d'agrandissement courant. Une case
///         d'inventaire tient une icône d'objet et rien d'autre : la doubler donnait une grille de
///         sac plus large que la fenêtre, et six colonnes coupées au bord droit.
[[nodiscard]] int cellSide() {
    return identityTokens().spacing.extraLarge * identityScale();
}

}  // namespace

RpgScreenFrame::RpgScreenFrame(const RpgScreenDescriptor& descriptor, QWidget* parent)
    : QWidget(parent), _descriptor(descriptor) {
    buildChrome(nullptr);
}

RpgScreenFrame::RpgScreenFrame(const RpgScreenDescriptor& descriptor, QWidget* content,
                               QWidget* parent)
    : QWidget(parent), _descriptor(descriptor) {
    buildChrome(content);
}

void RpgScreenFrame::buildChrome(QWidget* content) {
    setObjectName(QString::fromLatin1(_descriptor.objectName));
    setAttribute(Qt::WA_StyledBackground, true);
    // Le châssis reçoit le clavier lui-même : sans cela, `Échap` n'atteint jamais keyPressEvent
    // quand le focus est sur un bouton qui ne gère pas cette touche.
    setFocusPolicy(Qt::StrongFocus);

    const SpacingTokens& spacing = identityTokens().spacing;
    const int scale = identityScale();

    auto* const page = new QVBoxLayout(this);
    page->setContentsMargins(spacing.extraLarge * scale, spacing.large * scale,
                             spacing.extraLarge * scale, spacing.large * scale);
    page->setSpacing(spacing.large * scale);

    _title = new TitleBanner(this);
    _title->setObjectName(QStringLiteral("rpgTitle"));
    page->addWidget(_title);

    // Le CONTENU defile, le pied d'actions ne bouge pas.
    //
    // La pile d'ecrans enveloppe deja chaque page dans une zone defilante (EX-IHM-080), et cela
    // suffit a ne pas contraindre la fenetre -- mais pas a garder le pied a l'ecran : une fiche de
    // personnage haute de deux mille pixels y poussait « Fermer » et « Ecran suivant » sous la
    // ligne de flottaison, c'est-a-dire hors de vue de qui vient d'ouvrir l'ecran. Une seconde
    // zone defilante, interieure au chassis, borne la hauteur du CONTENU seul.
    QWidget* bodyHost = content;
    if (bodyHost == nullptr) {
        // Aucune planche fournie : le corps est rendu depuis la table (LOT-68).
        bodyHost = new QWidget(this);
        auto* const body = new QHBoxLayout(bodyHost);
        body->setContentsMargins(0, 0, 0, 0);
        body->setSpacing(spacing.large * scale);
        if (QWidget* const left = buildColumn(_descriptor.layout.leftColumn); left != nullptr) {
            body->addWidget(left, 1);
        }
        if (QWidget* const right = buildColumn(_descriptor.layout.rightColumn); right != nullptr) {
            body->addWidget(right, 1);
        }
    }
    auto* const bodyScroll = new QScrollArea(this);
    bodyScroll->setWidgetResizable(true);
    bodyScroll->setFrameShape(QFrame::NoFrame);
    // Sans cela, la zone defilante peint le fond opaque de la palette systeme : un rectangle gris
    // au milieu du parchemin (meme piege que les zones defilantes du Mode IA, LOT-73).
    bodyScroll->viewport()->setAutoFillBackground(false);
    bodyScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    bodyScroll->setWidget(bodyHost);
    // Contribue ZERO a la hauteur minimale du chassis : c'est ce qui garde le pied d'actions
    // visible quel que soit le contenu, et la fenetre libre de sa taille (EX-IHM-080).
    bodyScroll->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    page->addWidget(bodyScroll, 1);

    // Pied d'actions : les trois mêmes intentions sur tous les écrans, au même endroit. C'est la
    // moitié de ce que le LOT-68 livrait -- un écran qui se ferme autrement que son voisin oblige
    // à réapprendre à chaque écran.
    auto* const footer = new QHBoxLayout();
    footer->setSpacing(spacing.medium * scale);
    _closeButton = new MenuEntryButton(this);
    _previousButton = new MenuEntryButton(this);
    _nextButton = new MenuEntryButton(this);
    for (MenuEntryButton* const button : {_closeButton, _previousButton, _nextButton}) {
        // Sans autoDefault explicite, Entrée reste sans effet : ces écrans ne sont pas de vrais
        // QDialog (même règle Qt que MainMenu et PauseScreen).
        button->setAutoDefault(true);
        button->setCursor(Qt::PointingHandCursor);
        footer->addWidget(button);
    }
    _hints = new QLabel(this);
    _hints->setTextFormat(Qt::RichText);
    _hints->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    // Contribue ZERO a la largeur minimale : ces rappels sont une aide, et une aide ne doit pas
    // decider de la largeur de la fenetre -- c'etait la, avec le pied d'actions, la raison pour
    // laquelle la colonne de droite sortait du cadre.
    _hints->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    // Prend TOUT le reste de la largeur (facteur d'etirement), et n'en exige AUCUNE (politique
    // ignoree ci-dessus) : les rappels s'effacent d'eux-memes dans une fenetre etroite au lieu d'y
    // pousser une barre de defilement horizontale.
    footer->addWidget(_hints, 1);
    page->addLayout(footer);

    connect(_closeButton, &MenuEntryButton::clicked, this, &RpgScreenFrame::closeRequested);
    connect(_previousButton, &MenuEntryButton::clicked, this,
            &RpgScreenFrame::previousScreenRequested);
    connect(_nextButton, &MenuEntryButton::clicked, this, &RpgScreenFrame::nextScreenRequested);
}

QWidget* RpgScreenFrame::buildColumn(std::span<const RpgContentBlock> blocks) {
    if (blocks.empty()) {
        return nullptr;
    }
    auto* const container = new QWidget(this);
    auto* const column = new QVBoxLayout(container);
    column->setContentsMargins(0, 0, 0, 0);
    column->setSpacing(identityTokens().spacing.large * identityScale());
    for (const RpgContentBlock& block : blocks) {
        buildBlock(column, block);
    }
    column->addStretch(1);
    return container;
}

void RpgScreenFrame::buildBlock(QVBoxLayout* column, const RpgContentBlock& block) {
    const SpacingTokens& spacing = identityTokens().spacing;
    const int scale = identityScale();

    // Chaque bloc est un encadrement de parchemin : c'est l'habillage que le LOT-66 a posé pour
    // les cartes des écrans, et il vaut ici sans exception -- un bloc à nu se lirait comme un
    // panneau d'éditeur au milieu d'une feuille de personnage.
    auto* const panel = new ParchmentPanel(nullptr);
    auto* const inner = new QVBoxLayout(panel);
    inner->setContentsMargins(spacing.large * scale, spacing.large * scale, spacing.large * scale,
                              spacing.large * scale);
    inner->setSpacing(spacing.small * scale);

    if (block.titleKey[0] != '\0') {
        auto* const title = new QLabel(panel);
        setRole(title, "block");
        inner->addWidget(title);
        _translated.push_back({.label = title, .key = block.titleKey});
    }

    // Une étiquette de valeur, et l'identifiant sous lequel `setValues` la remplira. Identifiant
    // vide : rien n'alimente ce champ, et elle gardera son tiret.
    const auto addValue = [this](QWidget* host, const char* valueId) {
        auto* const value = new QLabel(host);
        setRole(value, "value");
        _values.push_back({.label = value, .key = valueId});
        return value;
    };

    switch (block.kind) {
        case RpgBlockKind::Fields: {
            auto* const grid = new QGridLayout();
            grid->setHorizontalSpacing(spacing.large * scale);
            grid->setVerticalSpacing(spacing.small * scale);
            int row = 0;
            for (const RpgField& field : block.fields) {
                auto* const label = new QLabel(panel);
                setRole(label, "field");
                grid->addWidget(label, row, 0);
                grid->addWidget(addValue(panel, field.valueId), row, 1);
                _translated.push_back({.label = label, .key = field.labelKey});
                ++row;
            }
            grid->setColumnStretch(1, 1);
            inner->addLayout(grid);
            break;
        }
        case RpgBlockKind::Grid: {
            auto* const grid = new QGridLayout();
            grid->setSpacing(spacing.small * scale);
            const int side = cellSide();
            for (int row = 0; row < block.rows; ++row) {
                for (int col = 0; col < block.columns; ++col) {
                    auto* const cell = new QFrame(panel);
                    setRole(cell, "cell");
                    cell->setFixedSize(side, side);
                    grid->addWidget(cell, row, col);
                }
            }
            // Une colonne vide qui prend le reste : sans elle, Qt repartit la largeur du bloc
            // entre les cases, et six cases d'inventaire s'eloignent les unes des autres jusqu'a
            // ne plus se lire comme une grille.
            grid->setColumnStretch(block.columns, 1);
            inner->addLayout(grid);
            break;
        }
        case RpgBlockKind::List: {
            // Le nombre de lignes vient des identifiants quand il y en a, de `rows` sinon : deux
            // sources qui pourraient dire deux nombres différents n'en font qu'une.
            const std::size_t lignes = block.valueIds.empty() ? static_cast<std::size_t>(block.rows)
                                                              : block.valueIds.size();
            for (std::size_t row = 0; row < lignes; ++row) {
                auto* const line = new QFrame(panel);
                setRole(line, "row");
                auto* const lineLayout = new QHBoxLayout(line);
                lineLayout->setContentsMargins(spacing.small * scale, spacing.extraSmall * scale,
                                               spacing.small * scale, spacing.extraSmall * scale);
                lineLayout->addWidget(
                    addValue(line, block.valueIds.empty() ? "" : block.valueIds[row]));
                inner->addWidget(line);
            }
            break;
        }
        case RpgBlockKind::Prose: {
            auto* const prose =
                addValue(panel, block.valueIds.empty() ? "" : block.valueIds.front());
            setRole(prose, "prose");
            prose->setWordWrap(true);
            prose->setMinimumHeight(cellSide());
            prose->setAlignment(Qt::AlignTop | Qt::AlignLeft);
            inner->addWidget(prose);
            break;
        }
        case RpgBlockKind::Portrait: {
            auto* const illustration = new QFrame(panel);
            setRole(illustration, "illustration");
            illustration->setMinimumSize(cellSide() * 3, cellSide() * 3);
            illustration->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            inner->addWidget(illustration);
            break;
        }
        case RpgBlockKind::Track: {
            auto* const strip = new QHBoxLayout();
            strip->setSpacing(spacing.small * scale);
            for (int chip = 0; chip < block.columns; ++chip) {
                auto* const token = new QFrame(panel);
                setRole(token, "chip");
                token->setFixedSize(cellSide(), cellSide());
                strip->addWidget(token);
            }
            strip->addStretch(1);
            inner->addLayout(strip);
            break;
        }
        case RpgBlockKind::ActionBar: {
            auto* const bar = new QHBoxLayout();
            bar->setSpacing(spacing.small * scale);
            for (int action = 0; action < block.columns; ++action) {
                auto* const slot = new QFrame(panel);
                setRole(slot, "action");
                slot->setFixedHeight(cellSide());
                slot->setMinimumWidth(cellSide() * 2);
                bar->addWidget(slot);
            }
            inner->addLayout(bar);
            break;
        }
    }

    column->addWidget(panel);
}

void RpgScreenFrame::retranslateUi(const Localization& loc) {
    const auto t = [&loc](const char* key) { return QString::fromStdString(loc.text(key)); };

    _title->setText(t(_descriptor.titleKey));
    for (const TranslatedLabel& entry : _translated) {
        entry.label->setText(t(entry.key));
    }
    // Toutes les valeurs repassent au tiret, puis celles qu'on connaît sont reposées : un
    // changement de langue ne doit pas vider une fiche remplie, et laisser les anciennes valeurs
    // en place ne conviendrait pas non plus -- certaines sont traduites.
    const QString empty = t("rpg.empty");
    for (const TranslatedLabel& value : _values) {
        value.label->setText(empty);
    }
    setValues(_lastValues);
    _closeButton->setText(t("rpg.chassis.close"));
    _previousButton->setText(t("rpg.chassis.previous_screen"));
    _nextButton->setText(t("rpg.chassis.next_screen"));
    _hints->setText(QString::fromStdString(hmi::keyHintText(
        {
            {.key = loc.text("key.shoulders"), .action = loc.text("hint.change_screen")},
            {.key = loc.text("key.confirm"), .action = loc.text("hint.confirm")},
            {.key = loc.text("key.escape"), .action = loc.text("hint.back")},
        },
        hmi::identityTokens(), hmi::identityScale())));
}

void RpgScreenFrame::setValues(const std::map<std::string, std::string>& values) {
    _lastValues = values;
    for (const TranslatedLabel& value : _values) {
        if (value.key[0] == '\0') {
            continue;  // aucun identifiant : ce champ n'a pas de source, il garde son tiret.
        }
        const auto trouve = values.find(value.key);
        if (trouve != values.end()) {
            value.label->setText(QString::fromStdString(trouve->second));
        }
    }
}

void RpgScreenFrame::focusDefaultAction() {
    _closeButton->setFocus();
}

void RpgScreenFrame::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        emit closeRequested();
        return;
    }
    QWidget::keyPressEvent(event);
}

}  // namespace hmi
