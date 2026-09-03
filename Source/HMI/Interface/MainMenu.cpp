// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/MainMenu.h"

#include <QColor>
#include <QPainter>
#include <QPushButton>

#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/KeyHintText.h"
#include "HMI/Interface/MenuBackdropGeometry.h"
#include "HMI/Localization/Localization.h"
#include "ui_MainMenu.h"

namespace hmi {

MainMenu::MainMenu(QWidget* parent) : QWidget(parent), _ui(std::make_unique<Ui::MainMenu>()) {
    setObjectName(QStringLiteral("MainMenu"));    // ciblé par le thème (theme-identity.qss)
    setAttribute(Qt::WA_StyledBackground, true);  // pour que la couleur de fond du thème s'applique
    _ui->setupUi(this);

    // Marges de la mise en page, depuis l'echelle d'espacement des jetons (LOT-56 TACHE-03) --
    // remplace les nombres jusqu'ici figes dans MainMenu.ui.
    const SpacingTokens& spacing = identityTokens().spacing;
    _ui->verticalLayout->setContentsMargins(spacing.extraLarge * 3, spacing.extraLarge * 3,
                                            spacing.extraLarge * 2, spacing.extraLarge * 2);

    connect(_ui->continueButton, &QPushButton::clicked, this, &MainMenu::continueRequested);
    connect(_ui->newGameButton, &QPushButton::clicked, this, &MainMenu::newGameRequested);
    connect(_ui->selectLevelButton, &QPushButton::clicked, this, &MainMenu::selectLevelRequested);
    connect(_ui->editorButton, &QPushButton::clicked, this, &MainMenu::editorRequested);
    connect(_ui->optionsButton, &QPushButton::clicked, this, &MainMenu::optionsRequested);
    connect(_ui->creditsButton, &QPushButton::clicked, this, &MainMenu::creditsRequested);
    connect(_ui->quitButton, &QPushButton::clicked, this, &MainMenu::quitRequested);

    // Qt n'active `autoDefault` que sur un bouton dont un ancêtre est un vrai QDialog. Ces
    // écrans n'en sont pas (même affichés comme fenêtre `Qt::Dialog`), et sans ce réglage
    // explicite un bouton qui a le focus ne répond qu'à Espace, jamais à Entrée. Posé sur
    // chacun pour que le bouton *ayant le focus* réponde à Entrée, quel qu'il soit.
    for (QPushButton* const button : findChildren<QPushButton*>()) {
        button->setAutoDefault(true);
    }
}

MainMenu::~MainMenu() = default;

void MainMenu::retranslateUi(const Localization& loc) {
    _ui->menuTitle->setText(QString::fromStdString(loc.text("menu.title")));
    _ui->continueButton->setText(QString::fromStdString(loc.text("menu.continue")));
    _ui->newGameButton->setText(QString::fromStdString(loc.text("menu.new_game")));
    _ui->selectLevelButton->setText(QString::fromStdString(loc.text("menu.select_level")));
    _ui->editorButton->setText(QString::fromStdString(loc.text("menu.edit_mode")));
    _ui->optionsButton->setText(QString::fromStdString(loc.text("menu.options")));
    _ui->creditsButton->setText(QString::fromStdString(loc.text("menu.credits")));
    _ui->quitButton->setText(QString::fromStdString(loc.text("menu.quit")));
    _ui->hintsLabel->setText(QString::fromStdString(hmi::keyHintText(
        {
            {.key = loc.text("key.up_down"), .action = loc.text("hint.navigate")},
            {.key = loc.text("key.confirm"), .action = loc.text("hint.confirm")},
        },
        hmi::identityTokens(), hmi::identityScale())));
}

void MainMenu::setContinueEnabled(bool enabled) {
    _ui->continueButton->setEnabled(enabled);
}

void MainMenu::paintEvent(QPaintEvent* event) {
    // Le decor d'abord, les enfants ensuite : QWidget::paintEvent peint le fond de la feuille de
    // style, sur lequel on pose les paves, et Qt dessine les boutons par-dessus.
    QWidget::paintEvent(event);

    const ColorTokens& color = identityTokens().color;
    // Nuances DERIVEES des jetons (hmi::mixColor) plutot qu'ecrites en dur : une teinte litterale
    // ne suivrait pas un changement de palette (EX-IHM-051).
    const auto shade = [&color](const DesignColor& to, float ratio) {
        return mixColor(color.background, to, ratio);
    };

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);  // EX-IHM-053 : aucun bord adouci.
    for (const BackdropQuad& quad : menuBackdropQuads(width(), height(), identityScale())) {
        DesignColor fill{};
        switch (quad.role) {
            case BackdropRole::SkyHigh:
                fill = color.background;
                break;
            case BackdropRole::SkyMid:
                fill = shade(color.surface, 0.55f);
                break;
            case BackdropRole::SkyLow:
                fill = shade(color.surfaceAlt, 0.85f);
                break;
            case BackdropRole::Star:
                fill = color.text;
                break;
            case BackdropRole::StarDim:
                fill = color.textMuted;
                break;
            case BackdropRole::Moon:
                fill = color.text;
                break;
            case BackdropRole::MoonCrater:
                fill = mixColor(color.text, color.textMuted, 0.55f);
                break;
            case BackdropRole::Silhouette:
                // Plan le plus lointain : le plus proche du ciel, pour reculer.
                fill = shade(color.border, 0.55f);
                break;
            case BackdropRole::Hill:
                fill = shade(color.surface, 0.35f);
                break;
            case BackdropRole::Ground:
                fill = shade(color.surface, 0.20f);
                break;
            case BackdropRole::GroundEdge:
                fill = shade(color.border, 0.35f);
                break;
            case BackdropRole::Veil:
                // Bandes SUPERPOSEES : chacune ajoute la meme opacite faible, et leur empilement
                // produit le fondu par paliers. Une seule bande opaque cacherait le decor.
                fill = DesignColor{.r = color.background.r,
                                   .g = color.background.g,
                                   .b = color.background.b,
                                   .a = 90};
                break;
        }
        painter.fillRect(quad.x, quad.y, quad.width, quad.height,
                         QColor(fill.r, fill.g, fill.b, fill.a));
    }
}

}  // namespace hmi
