// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file App/Game/Main.cpp
 * @brief Point d'entrée du **jeu** (`JustAnotherDnDGame`) — Qt Quick, sans un seul widget.
 *
 * `QGuiApplication` et non `QApplication` : la cible ne lie pas `Qt6::Widgets`, et ce n'est pas un
 * détail de dépendance mais la garantie qui porte tout le `LOT-86`. Un widget ne peut pas
 * réapparaître dans le jeu par inadvertance — l'édition de liens échouerait.
 *
 * L'éditeur de niveaux est un binaire séparé (`App/Editor/Main.cpp`), lui en Qt Widgets : c'est un
 * outil d'auteur, où docks et arbres sont le bon outil.
 */

#include <QFontDatabase>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlError>
#include <QQmlEngine>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QSettings>
#include <QString>
#include <QSurfaceFormat>
#include <QTimer>
#include <QTranslator>
#include <QUrl>
#include <filesystem>
#include <optional>
#include <string>

#include "App/Common/Bootstrap.h"
#include "Core/Diagnostics/MemoryLogSink.h"
#include "HMI/Audio/AudioEngine.h"
#include "HMI/Game/GameViewportItem.h"
#include "HMI/HmiLog.h"
#include "HMI/Platform/ExecutableDirectory.h"
#include "HMI/Presentation/OptionsModel.h"

namespace {

/**
 * @brief Enregistre les polices embarquées de l'identité auprès de Qt.
 *
 * Les fichiers sont déposés à côté de l'exécutable (`Assets/Fonts/`). Le QML ne les charge pas
 * lui-même : il les désigne par **nom de famille** (`Tokens.qml`), et Qt Design Studio les prend de
 * son côté via `FontFiles` du `.qmlproject`. Les deux voient donc les mêmes noms, et changer de
 * police reste une modification de `Tokens.qml` — jamais de C++.
 *
 * Un fichier absent n'est pas fatal : Qt retombe sur une famille générique et le journal le dit
 * (`EX-NFR-040`).
 */
void registerIdentityFonts() {
    const std::filesystem::path fonts = hmi::executableDirectory() / "Assets" / "Fonts";
    for (const char* file :
         {"PixelifySans-Regular.ttf", "PixelifySans-Bold.ttf", "PressStart2P-Regular.ttf"}) {
        const std::filesystem::path path = fonts / file;
        const int id = QFontDatabase::addApplicationFont(QString::fromStdString(path.string()));
        if (id < 0) {
            HMI_LOG_WARNING(std::string("Police d'identite non enregistree (") + file +
                            ") : famille generique.");
        }
    }
}

}  // namespace

/**
 * @brief Point d'entrée du programme.
 * @return Code de sortie du processus (0 en cas de succès).
 */
int main(int argc, char** argv) {
    core::MemoryLogSink* const sessionLog = app::installLogging(argc, argv, "JustAnotherDnDGame");

    QCoreApplication::setOrganizationName(QStringLiteral("JustAnotherDnDGame"));
    QCoreApplication::setApplicationName(QStringLiteral("Game"));

    {
        QSurfaceFormat format = QSurfaceFormat::defaultFormat();
        format.setSwapInterval(QSettings().value(QStringLiteral("vsync"), true).toBool() ? 1 : 0);
        QSurfaceFormat::setDefaultFormat(format);
    }

    QGuiApplication application(argc, argv);

    const QString language =
        QSettings().value(QStringLiteral("language"), QStringLiteral("fr")).toString();
    app::installQtTranslations(language.toStdString());

    static QTranslator gameTranslator;
    if (language != QLatin1String("fr")) {
        if (gameTranslator.load(QStringLiteral(":/i18n/jadg_") + language)) {
            QCoreApplication::installTranslator(&gameTranslator);
        } else {
            HMI_LOG_WARNING("Catalogue de traduction du jeu absent pour '" +
                            language.toStdString() + "' : l'interface restera en francais.");
        }
    }

    registerIdentityFonts();
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    hmi::AudioEngine audio;

    // GameViewportItem est un type C++ runtime. Il ne peut pas être découvert par le seul
    // `qmldir` de conception, qui doit rester indépendant du moteur. L'enregistrer explicitement
    // ici garantit que `GameView.qml` le trouve aussi bien en build normal qu'en mode
    // JADG_QML_FROM_SOURCE=1. Le formulaire `GameViewForm.ui.qml` ne le référence jamais.
    qmlRegisterType<hmi::GameViewportItem>("Jadg.Ui", 1, 0, "GameViewport");

    QQmlApplicationEngine engine;

    if (qEnvironmentVariable("JADG_QML_FROM_SOURCE") == QLatin1String("1")) {
        engine.addImportPath(QStringLiteral(JADG_QML_DEV_IMPORT_PATH));
        HMI_LOG_INFO("Interface lue depuis les sources : " JADG_QML_DEV_IMPORT_PATH);
    }

    QObject::connect(&engine, &QQmlEngine::warnings, &application,
                     [](const QList<QQmlError>& warnings) {
                         for (const QQmlError& warning : warnings) {
                             HMI_LOG_ERROR("QML : " + warning.toString().toStdString());
                         }
                     });
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &application,
        []() {
            HMI_LOG_ERROR("Chargement de l'interface QML impossible : arret.");
            QCoreApplication::exit(1);
        },
        Qt::QueuedConnection);

    if (const std::optional<std::string_view> shot =
            app::commandLineOption(argc, argv, "--screenshot=")) {
        const QString path = QString::fromUtf8(shot->data(), static_cast<qsizetype>(shot->size()));
        QObject::connect(
            &engine, &QQmlApplicationEngine::objectCreated, &application,
            [path](QObject* root, const QUrl&) {
                auto* window = qobject_cast<QQuickWindow*>(root);
                if (window == nullptr) {
                    HMI_LOG_ERROR("Capture impossible : la racine QML n'est pas une fenetre.");
                    QCoreApplication::exit(1);
                    return;
                }
                QTimer::singleShot(1200, window, [window, path]() {
                    const bool saved = window->grabWindow().save(path);
                    HMI_LOG_INFO((saved ? "Capture ecrite : " : "Echec de la capture : ") +
                                 path.toStdString());
                    QCoreApplication::exit(saved ? 0 : 1);
                });
            },
            Qt::SingleShotConnection);

        QTimer::singleShot(15000, &application, []() {
            HMI_LOG_ERROR("Capture : delai depasse, aucune image produite.");
            QCoreApplication::exit(2);
        });
    }

    if (const std::optional<std::string_view> screen =
            app::commandLineOption(argc, argv, "--screen=")) {
        engine.setInitialProperties(
            {{QStringLiteral("startScreen"),
              QString::fromUtf8(screen->data(), static_cast<qsizetype>(screen->size()))}});
    }

    if (auto* const options =
            engine.singletonInstance<hmi::OptionsModel*>("Jadg.Ui", "OptionsModel")) {
        options->setSessionLog(sessionLog);
        audio.setVolume(static_cast<float>(options->volume()) / 100.0F);
        QObject::connect(options, &hmi::OptionsModel::volumeChanged, options, [options, &audio]() {
            audio.setVolume(static_cast<float>(options->volume()) / 100.0F);
        });
        QObject::connect(
            options, &hmi::OptionsModel::languageChanged, &engine, [options, &engine]() {
                QCoreApplication::removeTranslator(&gameTranslator);
                if (options->language() != QLatin1String("fr") &&
                    gameTranslator.load(QStringLiteral(":/i18n/jadg_") + options->language())) {
                    QCoreApplication::installTranslator(&gameTranslator);
                }
                engine.retranslate();
            });
    } else {
        HMI_LOG_ERROR("Reglages introuvables : le volume et la langue ne seront pas appliques.");
    }

    engine.loadFromModule("Jadg.Ui", "Main");

    const int code = QGuiApplication::exec();
    HMI_LOG_INFO("Arret de JustAnotherDnDGame (code " + std::to_string(code) + ").");
    return code;
}
