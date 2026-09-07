// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/PauseScreen.h"

#include <QKeyEvent>
#include <QPushButton>

#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/KeyHintText.h"
#include "HMI/Localization/Localization.h"
#include "ui_PauseScreen.h"

namespace hmi {

PauseScreen::PauseScreen(QWidget* parent)
    : QWidget(parent), _ui(std::make_unique<Ui::PauseScreen>()) {
    setObjectName(QStringLiteral("PauseScreen"));
    setAttribute(Qt::WA_StyledBackground, true);
    _ui->setupUi(this);

    // Fond translucide pose ici plutot que dans le theme (theme-identity.qss) : propre a ce
    // recouvrement par-dessus la scene figee, pas un jeton de theme editeur (LOT-56) -- la carte du
    // menu garde un fond opaque pour rester lisible par-dessus n'importe quel niveau.
    setStyleSheet(
        QStringLiteral("#PauseScreen { background-color: rgba(0, 0, 0, 160); }"
                       "#menuCard { background-color: rgba(32, 32, 32, 230); "
                       "border-radius: 8px; padding: 24px; }"
                       "#menuCard QLabel, #menuCard QPushButton { color: white; }"));

    connect(_ui->resumeButton, &QPushButton::clicked, this, &PauseScreen::resumeRequested);
    connect(_ui->optionsButton, &QPushButton::clicked, this, &PauseScreen::optionsRequested);
    connect(_ui->quitButton, &QPushButton::clicked, this, &PauseScreen::quitToMenuRequested);

    // Qt n'active `autoDefault` que sur un bouton dont un ancêtre est un vrai QDialog. Ces
    // écrans n'en sont pas (même affichés comme fenêtre `Qt::Dialog`), et sans ce réglage
    // explicite un bouton qui a le focus ne répond qu'à Espace, jamais à Entrée. Posé sur
    // chacun pour que le bouton *ayant le focus* réponde à Entrée, quel qu'il soit.
    for (QPushButton* const button : findChildren<QPushButton*>()) {
        button->setAutoDefault(true);
    }
}

PauseScreen::~PauseScreen() = default;

void PauseScreen::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        emit resumeRequested();
        return;
    }
    QWidget::keyPressEvent(event);
}

void PauseScreen::focusDefaultAction() {
    _ui->resumeButton->setFocus();
}

void PauseScreen::retranslateUi(const Localization& loc) {
    _ui->pauseTitle->setText(QString::fromStdString(loc.text("pause.title")));
    _ui->resumeButton->setText(QString::fromStdString(loc.text("pause.resume")));
    _ui->optionsButton->setText(QString::fromStdString(loc.text("pause.options")));
    _ui->quitButton->setText(QString::fromStdString(loc.text("pause.quit_to_menu")));
    // Rappels de touches (LOT-68) : la navigation a la manette repose sur le
    // parcours de focus, encore faut-il savoir quelle touche l'avance.
    _ui->hintsLabel->setText(QString::fromStdString(hmi::keyHintText(
        {
            {.key = loc.text("key.escape"), .action = loc.text("hint.resume")},
            {.key = loc.text("key.back"), .action = loc.text("hint.back")},
        },
        hmi::identityTokens(), hmi::identityScale())));
}

}  // namespace hmi
