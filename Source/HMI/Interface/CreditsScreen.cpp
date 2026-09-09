// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/CreditsScreen.h"

#include <QPushButton>

#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/KeyHintText.h"
#include "HMI/Localization/Localization.h"
#include "ui_CreditsScreen.h"

namespace hmi {

CreditsScreen::CreditsScreen(QWidget* parent)
    : QWidget(parent), _ui(std::make_unique<Ui::CreditsScreen>()) {
    setObjectName(QStringLiteral("CreditsScreen"));  // ciblé par le thème (theme-identity.qss)
    setAttribute(Qt::WA_StyledBackground, true);
    _ui->setupUi(this);
    // Le theme que le .ui porte pour Qt Designer est jete ici (LOT-85, EX-IHM-006). Il y est fige
    // au facteur x2 ; le jeu, lui, recalcule le sien avec la hauteur de la fenetre, et une feuille
    // posee sur CE widget primerait celle de la pile d'ecrans -- la mise a l'echelle cesserait.
    setStyleSheet(QString());

    // Marges de la mise en page, depuis l'echelle d'espacement des jetons (LOT-56 TACHE-03) --
    // meme patron que MainMenu/OptionsPage.
    const SpacingTokens& spacing = identityTokens().spacing;
    _ui->verticalLayout->setContentsMargins(spacing.extraLarge * 3, spacing.extraLarge * 2,
                                            spacing.extraLarge * 3, spacing.extraLarge * 2);

    connect(_ui->backButton, &QPushButton::clicked, this, &CreditsScreen::backRequested);
    // Sans autoDefault explicite, Entree reste sans effet sur ce bouton : cet ecran n'est pas un
    // vrai QDialog (meme regle Qt que MainMenu).
    _ui->backButton->setAutoDefault(true);
}

CreditsScreen::~CreditsScreen() = default;

void CreditsScreen::retranslateUi(const Localization& loc) {
    const auto t = [&loc](const char* key) { return QString::fromStdString(loc.text(key)); };

    _ui->creditsTitle->setText(t("credits.title"));
    _ui->developmentLabel->setText(t("credits.development"));
    _ui->audioLabel->setText(t("credits.audio"));
    _ui->graphicsLabel->setText(t("credits.graphics"));
    _ui->fontsLabel->setText(t("credits.fonts"));
    _ui->librariesLabel->setText(t("credits.libraries"));
    _ui->licenseLabel->setText(t("credits.license"));
    // Seuls les INTITULES de section sont traduits. Les lignes de credit elles-memes ne le sont
    // pas : ce sont des noms propres et des noms de licence, qui ne se traduisent jamais -- et la
    // LGPLv3 comme la SIL OFL demandent une mention identifiable, pas une paraphrase localisee.
    _ui->backButton->setText(t("options.back"));
    // Rappels de touches (LOT-68) : la navigation a la manette repose sur le
    // parcours de focus, encore faut-il savoir quelle touche l'avance.
    _ui->hintsLabel->setText(QString::fromStdString(hmi::keyHintText(
        {
            {.key = loc.text("key.back"), .action = loc.text("hint.back")},
        },
        hmi::identityTokens(), hmi::identityScale())));
}

void CreditsScreen::focusDefaultAction() {
    _ui->backButton->setFocus();
}

}  // namespace hmi
