// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/RpgUiScreen.h"

#include <QBoxLayout>
#include <QFrame>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QScrollArea>
#include <QSizePolicy>
#include <QVBoxLayout>

#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/KeyHintText.h"
#include "HMI/Localization/Localization.h"
#include "HMI/Interface/MenuEntryButton.h"
#include "HMI/Interface/TitleBanner.h"

namespace hmi {
namespace {

/// Le côté d'une case : celui du sac, des jetons d'initiative et des cases d'action. Une seule
/// écriture, pour que les trois ne puissent pas diverger.
int cellSide() {
    return identityTokens().spacing.extraLarge * identityScale();
}

/// Le rôle porté par un widget, ou une chaîne vide.
QString roleOf(const QWidget* widget) {
    return widget->property("rpgRole").toString();
}

}  // namespace

RpgUiScreen::RpgUiScreen(const RpgScreenDescriptor& descriptor, QWidget* parent)
    : QWidget(parent), _descriptor(descriptor) {
    // Le châssis reçoit le clavier lui-même : sans cela, `Échap` n'atteint jamais keyPressEvent
    // quand le focus est sur un bouton qui ne gère pas cette touche.
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_StyledBackground, true);
}

void RpgUiScreen::bind() {
    // Le theme que la planche porte pour Qt Designer est jete ici (LOT-85, EX-IHM-006). Il y est
    // fige au facteur x2 ; le jeu recalcule le sien avec la hauteur de la fenetre, et une feuille
    // posee sur CE widget primerait celle de la pile d'ecrans -- la mise a l'echelle cesserait.
    setStyleSheet(QString());

    _title = findChild<TitleBanner*>(QStringLiteral("rpgTitle"));
    _closeButton = findChild<MenuEntryButton*>(QStringLiteral("closeButton"));
    _previousButton = findChild<MenuEntryButton*>(QStringLiteral("previousButton"));
    _nextButton = findChild<MenuEntryButton*>(QStringLiteral("nextButton"));
    _hints = findChild<QLabel*>(QStringLiteral("hintsLabel"));

    for (MenuEntryButton* const button : {_closeButton, _previousButton, _nextButton}) {
        if (button == nullptr) {
            continue;
        }
        // Sans autoDefault explicite, Entrée reste sans effet : ces écrans ne sont pas de vrais
        // QDialog (même règle Qt que MainMenu et PauseScreen).
        button->setAutoDefault(true);
        button->setCursor(Qt::PointingHandCursor);
    }
    if (_closeButton != nullptr) {
        connect(_closeButton, &MenuEntryButton::clicked, this, &RpgUiScreen::closeRequested);
    }
    if (_previousButton != nullptr) {
        connect(_previousButton, &MenuEntryButton::clicked, this,
                &RpgUiScreen::previousScreenRequested);
    }
    if (_nextButton != nullptr) {
        connect(_nextButton, &MenuEntryButton::clicked, this, &RpgUiScreen::nextScreenRequested);
    }
    if (_hints != nullptr) {
        _hints->setTextFormat(Qt::RichText);
        _hints->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        // Contribue ZERO a la largeur minimale : ces rappels sont une aide, et une aide ne doit
        // pas decider de la largeur de la fenetre.
        _hints->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    }

    collectBindings();
    applyMetrics();
}

void RpgUiScreen::collectBindings() {
    for (QLabel* const label : findChildren<QLabel*>()) {
        const QString cle = label->property("rpgKey").toString();
        if (!cle.isEmpty()) {
            _translated.push_back({.label = label, .key = cle.toStdString()});
        }
        // `rpgValue` peut etre ABSENT sur un champ dont rien n'alimente la valeur : le champ garde
        // alors son tiret cadratin, et c'est ce qui distingue « pas de source » de « vaut zero ».
        if (roleOf(label) == QLatin1String("value") ||
            roleOf(label) == QLatin1String("prose")) {
            _values.push_back(
                {.label = label, .key = label->property("rpgValue").toString().toStdString()});
        }
    }
}

void RpgUiScreen::applyMetrics() {
    const SpacingTokens& spacing = identityTokens().spacing;
    const int scale = identityScale();
    const int side = cellSide();

    if (auto* const page = findChild<QVBoxLayout*>(QStringLiteral("pageLayout"))) {
        page->setContentsMargins(spacing.extraLarge * scale, spacing.large * scale,
                                 spacing.extraLarge * scale, spacing.large * scale);
        page->setSpacing(spacing.large * scale);
    }
    if (auto* const body = findChild<QHBoxLayout*>(QStringLiteral("bodyRow"))) {
        body->setSpacing(spacing.large * scale);
    }
    if (auto* const footer = findChild<QHBoxLayout*>(QStringLiteral("footerRow"))) {
        footer->setSpacing(spacing.medium * scale);
    }

    // Le retrait du parchemin est porté par le layout `*Inset`, DEHORS de la zone défilante : à
    // l'intérieur, il défilerait avec le contenu et le texte toucherait le cadre.
    for (QBoxLayout* const layout : findChildren<QBoxLayout*>()) {
        const QString nom = layout->objectName();
        if (nom.endsWith(QLatin1String("Inset"))) {
            const int inset = spacing.large * scale;
            layout->setContentsMargins(inset, inset, inset, inset);
            layout->setSpacing(0);
        } else if (nom.endsWith(QLatin1String("Layout")) && nom.contains(QLatin1String("Block"))) {
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(spacing.small * scale);
        } else if (nom.endsWith(QLatin1String("ColumnLayout"))) {
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(spacing.large * scale);
        }
    }
    for (QGridLayout* const grid : findChildren<QGridLayout*>()) {
        if (grid->objectName().endsWith(QLatin1String("Grid"))) {
            grid->setHorizontalSpacing(spacing.large * scale);
            grid->setVerticalSpacing(spacing.small * scale);
            grid->setColumnStretch(1, 1);
        } else {
            grid->setSpacing(spacing.small * scale);
            // Une colonne vide qui prend le reste : sans elle, Qt repartit la largeur du bloc
            // entre les cases, et six cases s'eloignent jusqu'a ne plus se lire comme une grille.
            grid->setColumnStretch(grid->columnCount(), 1);
        }
    }

    // CHAQUE encart défile pour son compte : une seule zone défilante pour tout l'écran faisait
    // payer à tous les blocs le débordement d'un seul. `Ignored` en HAUTEUR seulement — sur les
    // deux axes, les colonnes perdaient toute revendication de largeur.
    for (QScrollArea* const area : findChildren<QScrollArea*>()) {
        area->viewport()->setAutoFillBackground(false);
        area->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
    }

    for (QWidget* const widget : findChildren<QWidget*>()) {
        const QString role = roleOf(widget);
        if (role == QLatin1String("cell") || role == QLatin1String("chip")) {
            widget->setFixedSize(side, side);
        } else if (role == QLatin1String("action")) {
            widget->setFixedHeight(side);
            widget->setMinimumWidth(side * 2);
        } else if (role == QLatin1String("illustration")) {
            widget->setMinimumSize(side * 3, side * 3);
            widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        } else if (role == QLatin1String("prose")) {
            widget->setMinimumHeight(side);
        } else if (role == QLatin1String("row")) {
            if (auto* const ligne = qobject_cast<QBoxLayout*>(widget->layout())) {
                ligne->setContentsMargins(spacing.small * scale, spacing.extraSmall * scale,
                                          spacing.small * scale, spacing.extraSmall * scale);
            }
        }
    }
}

void RpgUiScreen::retranslateUi(const Localization& loc) {
    const auto t = [&loc](const std::string& key) { return QString::fromStdString(loc.text(key)); };

    if (_title != nullptr) {
        _title->setText(t(_descriptor.titleKey));
    }
    for (const Bound& entry : _translated) {
        entry.label->setText(t(entry.key));
    }
    // Toutes les valeurs repassent au tiret, puis celles qu'on connaît sont reposées : un
    // changement de langue ne doit pas vider un écran rempli, et laisser les anciennes valeurs en
    // place ne conviendrait pas non plus -- certaines sont traduites.
    const QString empty = t("rpg.empty");
    for (const Bound& value : _values) {
        value.label->setText(empty);
    }
    setValues(_lastValues);

    if (_closeButton != nullptr) {
        _closeButton->setText(t("rpg.chassis.close"));
    }
    if (_previousButton != nullptr) {
        _previousButton->setText(t("rpg.chassis.previous_screen"));
    }
    if (_nextButton != nullptr) {
        _nextButton->setText(t("rpg.chassis.next_screen"));
    }
    if (_hints != nullptr) {
        _hints->setText(QString::fromStdString(hmi::keyHintText(
            {
                {.key = loc.text("key.shoulders"), .action = loc.text("hint.change_screen")},
                {.key = loc.text("key.confirm"), .action = loc.text("hint.confirm")},
                {.key = loc.text("key.escape"), .action = loc.text("hint.back")},
            },
            hmi::identityTokens(), hmi::identityScale())));
    }
}

void RpgUiScreen::setValues(const std::map<std::string, std::string>& values) {
    _lastValues = values;
    for (const Bound& value : _values) {
        if (value.key.empty()) {
            continue;  // aucun identifiant : ce champ n'a pas de source, il garde son tiret.
        }
        const auto trouve = values.find(value.key);
        if (trouve != values.end()) {
            value.label->setText(QString::fromStdString(trouve->second));
        }
    }
}

void RpgUiScreen::focusDefaultAction() {
    if (_closeButton != nullptr) {
        _closeButton->setFocus();
    }
}

void RpgUiScreen::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        emit closeRequested();
        return;
    }
    QWidget::keyPressEvent(event);
}

}  // namespace hmi
