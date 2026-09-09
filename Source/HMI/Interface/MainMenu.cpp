// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/MainMenu.h"

#include <QColor>
#include <QLinearGradient>
#include <QPainter>
#include <QPoint>
#include <QPushButton>
#include <QRect>
#include <QSize>

#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/KeyHintText.h"
#include "HMI/Interface/MenuBackdropGeometry.h"
#include "HMI/Localization/Localization.h"
#include "HMI/Platform/ExecutableDirectory.h"
#include "ui_MainMenu.h"

namespace hmi {

MainMenu::MainMenu(QWidget* parent) : QWidget(parent), _ui(std::make_unique<Ui::MainMenu>()) {
    setObjectName(QStringLiteral("MainMenu"));    // ciblé par le thème (theme-identity.qss)
    setAttribute(Qt::WA_StyledBackground, true);  // pour que la couleur de fond du thème s'applique
    _ui->setupUi(this);
    // Le theme que le .ui porte pour Qt Designer est jete ici (LOT-85, EX-IHM-006). Il y est fige
    // au facteur x2 ; le jeu, lui, recalcule le sien avec la hauteur de la fenetre, et une feuille
    // posee sur CE widget primerait celle de la pile d'ecrans -- la mise a l'echelle cesserait.
    setStyleSheet(QString());

    // Marges de la mise en page, depuis l'echelle d'espacement des jetons (LOT-56 TACHE-03) --
    // remplace les nombres jusqu'ici figes dans MainMenu.ui.
    const SpacingTokens& spacing = identityTokens().spacing;
    _ui->verticalLayout->setContentsMargins(spacing.extraLarge * 3, spacing.extraLarge * 3,
                                            spacing.extraLarge * 2, spacing.extraLarge * 2);

    connect(_ui->newGameButton, &QPushButton::clicked, this, &MainMenu::newGameRequested);
    connect(_ui->editorButton, &QPushButton::clicked, this, &MainMenu::editorRequested);
    connect(_ui->optionsButton, &QPushButton::clicked, this, &MainMenu::optionsRequested);
    connect(_ui->creditsButton, &QPushButton::clicked, this, &MainMenu::creditsRequested);
    connect(_ui->quitButton, &QPushButton::clicked, this, &MainMenu::quitRequested);

    // Carte du monde en fond (LOT-67). Chargee UNE FOIS, ici : un `QPixmap::load` par paintEvent
    // relirait 700 Ko de JPEG a chaque image. Membre du widget et non statique -- il meurt avec le
    // menu, donc avant QGuiApplication, et n'expose pas le depot a la panne de fermeture que les
    // garde-fous de `test_theme_teardown_guards` documentent.
    static_cast<void>(_backdrop.load(QString::fromStdString(
        (hmi::executableDirectory() / "Assets" / "UI" / BACKDROP_FILE).string())));

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
    _ui->newGameButton->setText(QString::fromStdString(loc.text("menu.new_game")));
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

    // La carte du monde, si elle est la. Recadree en COUVERTURE : le rapport d'aspect est
    // preserve et le debord est rogne, jamais deforme -- une carte etiree se voit immediatement,
    // les cotes ayant des formes que le joueur reconnait.
    const bool hasBackdrop = !_backdrop.isNull();
    if (hasBackdrop) {
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        const QSize scaled = _backdrop.size().scaled(size(), Qt::KeepAspectRatioByExpanding);
        painter.drawPixmap(
            QRect(QPoint((width() - scaled.width()) / 2, (height() - scaled.height()) / 2), scaled),
            _backdrop);

        // Voile de lisibilite, en DEGRADE CONTINU et non en paliers. Le decor trace, lui, se voile
        // par bandes superposees -- c'est correct sur des aplats, qui n'ont pas de detail a
        // trahir. Sur une carte peinte, chaque palier se lit comme une bande verticale claire en
        // travers du relief : le procede se voit, et c'est tout ce qu'on voit.
        //
        // Il ne couvre que la GAUCHE, ou vivent les entrees du menu. Assombrir l'image entiere
        // rendrait le texte lisible en effacant ce qu'on vient de mettre derriere.
        QLinearGradient veil(0.0, 0.0, width() * 0.55, 0.0);
        const DesignColor& ground = color.background;
        veil.setColorAt(0.0, QColor(ground.r, ground.g, ground.b, 225));
        veil.setColorAt(0.45, QColor(ground.r, ground.g, ground.b, 150));
        veil.setColorAt(1.0, QColor(ground.r, ground.g, ground.b, 0));
        painter.fillRect(rect(), veil);
    }

    painter.setRenderHint(QPainter::Antialiasing, false);  // EX-IHM-053 : aucun bord adouci.
    if (hasBackdrop) {
        // Le decor trace est le REPLI, pas un complement : avec la carte, il ne peint plus rien.
        // Son voile a deja ete remplace ci-dessus par un degrade, et repeindre son ciel et ses
        // collines effacerait la carte que l'on vient de poser.
        return;
    }
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
