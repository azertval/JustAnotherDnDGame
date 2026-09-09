// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/OptionsPage.h"

#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QSettings>
#include <QSignalBlocker>
#include <QSlider>
#include <QString>
#include <QTabWidget>
#include <algorithm>
#include <optional>
#include <utility>

#include "HMI/Audio/AudioEngine.h"
#include "HMI/Audio/SoundTriggers.h"
#include "HMI/Game/GameViewport.h"
#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/EditorKeybindingsWidget.h"
#include "HMI/Interface/GamepadBindingsWidget.h"
#include "HMI/Interface/KeyHintText.h"
#include "HMI/Interface/KeybindingsWidget.h"
#include "HMI/Localization/Localization.h"
#include "ui_OptionsPage.h"

namespace hmi {

namespace {
// Cle de preference du volume, meme portee QSettings que la langue et le mode de rendu
// (GameViewport.cpp) -- persistee/relue au meme endroit et selon le meme mecanisme (TACHE-04).
constexpr const char* VOLUME_SETTINGS_KEY = "volume";
// Compteur de diagnostic (LOT-68) : meme portee QSettings que le volume et la langue -- aucun
// nouveau mecanisme de persistance a inventer.
constexpr const char* DIAGNOSTICS_SETTINGS_KEY = "diagnostics_overlay";
}  // namespace

OptionsPage::OptionsPage(GameViewport* viewport, AudioEngine* audio,
                         std::filesystem::path keybindingsPath, QWidget* parent)
    : QWidget(parent), _ui(std::make_unique<Ui::OptionsPage>()), _audio(audio) {
    setObjectName(QStringLiteral("OptionsPage"));  // ciblé par le thème (theme-identity.qss)
    setAttribute(Qt::WA_StyledBackground, true);
    _ui->setupUi(this);
    // Le theme que le .ui porte pour Qt Designer est jete ici (LOT-85, EX-IHM-006). Il y est fige
    // au facteur x2 ; le jeu, lui, recalcule le sien avec la hauteur de la fenetre, et une feuille
    // posee sur CE widget primerait celle de la pile d'ecrans -- la mise a l'echelle cesserait.
    setStyleSheet(QString());

    // Marges de la mise en page, depuis l'echelle d'espacement des jetons (LOT-56 TACHE-03) --
    // remplace les nombres jusqu'ici figes dans OptionsPage.ui.
    const SpacingTokens& spacing = identityTokens().spacing;
    _ui->verticalLayout->setContentsMargins(spacing.extraLarge * 3, spacing.extraLarge * 2,
                                            spacing.extraLarge * 3, spacing.extraLarge * 2);

    // Onglet Vidéo : V-Sync, plein écran et compteur de diagnostic — les trois agissent
    // réellement, aucun réglage décoratif (`EX-IHM-072`).
    _ui->vsyncCheck->setChecked(viewport->vsyncEnabled());
    connect(_ui->vsyncCheck, &QCheckBox::toggled, viewport,
            [viewport](bool on) { viewport->setVSync(on); });
    connect(_ui->fullscreenCheck, &QCheckBox::toggled, this,
            [this](bool on) { emit fullscreenRequested(on); });

    // Compteur de diagnostic (LOT-68) : le recouvrement existe depuis le LOT-62, en haut à droite,
    // mais n'était atteignable que par `F9` — invisible pour qui ne lit pas la documentation. Ce
    // réglage l'expose et le PERSISTE ; la touche reste un second chemin vers le MÊME état, jamais
    // un second état (`EX-IHM-062`).
    const bool diagnosticsOn =
        QSettings().value(QString::fromLatin1(DIAGNOSTICS_SETTINGS_KEY), false).toBool();
    viewport->setDiagnosticsOverlayEnabled(diagnosticsOn);
    _ui->diagnosticsCheck->setChecked(diagnosticsOn);
    connect(_ui->diagnosticsCheck, &QCheckBox::toggled, viewport, [viewport](bool on) {
        viewport->setDiagnosticsOverlayEnabled(on);
        QSettings().setValue(QString::fromLatin1(DIAGNOSTICS_SETTINGS_KEY), on);
    });

    // Onglet Audio (LOT-60 TACHE-04) : volume global, persiste dans la meme portee QSettings que
    // la langue/le mode de rendu -- releve au lancement, applique immediatement au moteur, retenu
    // pour le lancement suivant.
    const int savedVolumePercent = std::clamp(
        QSettings().value(QString::fromLatin1(VOLUME_SETTINGS_KEY), 100).toInt(), 0, 100);
    _ui->volumeSlider->setValue(savedVolumePercent);
    if (_audio) {
        _audio->setVolume(static_cast<float>(savedVolumePercent) / 100.0f);
    }
    connect(_ui->volumeSlider, &QSlider::valueChanged, this, [this](int value) {
        QSettings().setValue(QString::fromLatin1(VOLUME_SETTINGS_KEY), value);
        if (_audio) {
            _audio->setVolume(static_cast<float>(value) / 100.0f);
        }
    });
    // Retour sonore AU RELACHEMENT seulement (jamais a chaque pas du curseur, TACHE-04) : sans
    // quoi le reglage se ferait a l'aveugle, mais un echantillon par pas transformerait le curseur
    // en mitraillette.
    connect(_ui->volumeSlider, &QSlider::sliderReleased, this, [this]() {
        if (!_audio) {
            return;
        }
        if (const std::optional<std::string> soundId = soundForEvent(GameEvent::MenuNavigate)) {
            _audio->play(*soundId);
        }
    });

    // Onglet Général (mise en page dans OptionsPage.ui) : langue (index 0 = fr, 1 = en) et logs.
    connect(_ui->languageCombo, &QComboBox::currentIndexChanged, this, [this](int index) {
        emit languageChanged(index == 0 ? QStringLiteral("fr") : QStringLiteral("en"));
    });
    connect(_ui->saveLogsButton, &QPushButton::clicked, this, &OptionsPage::saveLogsRequested);

    // Onglets à contenu **dynamique** (une ligne par action) : générés en code (exception admise).
    _keyboard = new KeybindingsWidget(viewport->gameBindings(), keybindingsPath, this);
    _keyboardTabIndex = _ui->tabWidget->addTab(_keyboard, QString());
    // Onglet Éditeur (LOT-57 TACHE-04) : remappage des touches d'éditeur, jusqu'ici définies et
    // jamais exposées à l'écran. bindingsChanged fait resynchroniser les raccourcis effectifs des
    // actions (menu/barre d'outils), sans quoi un remappage ici resterait invisible ailleurs.
    _editorKeyboard =
        new EditorKeybindingsWidget(viewport->editorBindings(), keybindingsPath, this);
    _editorKeyboardTabIndex = _ui->tabWidget->addTab(_editorKeyboard, QString());
    connect(_editorKeyboard, &EditorKeybindingsWidget::bindingsChanged, this,
            &OptionsPage::editorBindingsChanged);
    _gamepad =
        new GamepadBindingsWidget(viewport->gamepadBindings(), std::move(keybindingsPath), this);
    _gamepadTabIndex = _ui->tabWidget->addTab(_gamepad, QString());

    connect(_ui->backButton, &QPushButton::clicked, this, &OptionsPage::backRequested);
}

OptionsPage::~OptionsPage() = default;

void OptionsPage::retranslateUi(const Localization& loc) {
    const auto t = [&loc](const char* key) { return QString::fromStdString(loc.text(key)); };

    _ui->optionsTitle->setText(t("options.title"));

    // Onglets et libellés de l'onglet Vidéo.
    _ui->tabWidget->setTabText(0, t("tab.video"));
    _ui->tabWidget->setTabText(1, t("tab.audio"));
    _ui->tabWidget->setTabText(2, t("tab.general"));
    _ui->vsyncLabel->setText(t("options.vsync"));
    _ui->vsyncCheck->setText(t("options.enabled"));
    _ui->displayLabel->setText(t("options.display"));
    _ui->fullscreenCheck->setText(t("options.fullscreen"));
    _ui->diagnosticsLabel->setText(t("options.diagnostics"));
    _ui->diagnosticsCheck->setText(t("options.diagnostics_show"));
    _ui->volumeLabel->setText(t("options.volume"));
    _ui->backButton->setText(t("options.back"));
    // Rappels de touches (LOT-68) : la navigation a la manette repose sur le
    // parcours de focus, encore faut-il savoir quelle touche l'avance.
    _ui->hintsLabel->setText(QString::fromStdString(hmi::keyHintText(
        {
            {.key = loc.text("key.left_right"), .action = loc.text("hint.tab")},
            {.key = loc.text("key.back"), .action = loc.text("hint.back")},
        },
        hmi::identityTokens(), hmi::identityScale())));

    // Onglet Général.
    _ui->languageLabel->setText(t("options.language"));
    _ui->languageCombo->setItemText(0, t("language.fr"));
    _ui->languageCombo->setItemText(1, t("language.en"));
    _ui->logsLabel->setText(t("options.logs"));
    _ui->saveLogsButton->setText(t("options.save_logs"));

    // Aligne la sélection de langue sur la langue active, sans réémettre languageChanged.
    const int activeIndex = loc.activeLanguage() == "en" ? 1 : 0;
    if (activeIndex != _ui->languageCombo->currentIndex()) {
        const QSignalBlocker blocker(_ui->languageCombo);
        _ui->languageCombo->setCurrentIndex(activeIndex);
    }

    // Onglets de remappage (contenu dynamique).
    _ui->tabWidget->setTabText(_keyboardTabIndex, t("tab.keyboard"));
    _ui->tabWidget->setTabText(_editorKeyboardTabIndex, t("tab.editor"));
    _ui->tabWidget->setTabText(_gamepadTabIndex, t("tab.gamepad"));
    _keyboard->retranslateUi(loc);
    _editorKeyboard->retranslateUi(loc);
    _gamepad->retranslateUi(loc);
}

}  // namespace hmi
