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
#include <QQuickWindow>
#include <QSettings>
#include <QString>
#include <QTimer>
#include <QUrl>
#include <filesystem>
#include <optional>
#include <string>

#include "App/Common/Bootstrap.h"
#include "HMI/HmiLog.h"
#include "HMI/Platform/ExecutableDirectory.h"

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
    app::installLogging(argc, argv, "JustAnotherDnDGame");

    QGuiApplication application(argc, argv);
    // Identité de l'application : portée des réglages persistés (QSettings).
    QCoreApplication::setOrganizationName(QStringLiteral("JustAnotherDnDGame"));
    QCoreApplication::setApplicationName(QStringLiteral("Game"));

    const QString language =
        QSettings().value(QStringLiteral("language"), QStringLiteral("fr")).toString();
    app::installQtTranslations(language.toStdString());

    registerIdentityFonts();

    QQmlApplicationEngine engine;
    // Les erreurs de l'engine vont, par defaut, sur la sortie d'erreur de Qt -- que personne ne
    // lit apres coup, et qui n'existe pas dans un binaire livre. On les verse dans le journal de
    // session : un ecran QML casse doit se diagnostiquer depuis Logs/, comme tout le reste.
    QObject::connect(&engine, &QQmlEngine::warnings, &application,
                     [](const QList<QQmlError>& warnings) {
                         for (const QQmlError& warning : warnings) {
                             HMI_LOG_ERROR("QML : " + warning.toString().toStdString());
                         }
                     });
    // Un échec de chargement QML ne lève pas : sans cette garde, le programme rendrait 0 avec une
    // fenêtre absente — la panne muette que le châssis Qt documente déjà pour d'autres raisons.
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &application,
        []() {
            HMI_LOG_ERROR("Chargement de l'interface QML impossible : arret.");
            QCoreApplication::exit(1);
        },
        Qt::QueuedConnection);
    // Capture d'ecran non interactive (--screenshot=<chemin>). Une fenetre Qt Quick est rendue par
    // le GPU : les API de capture de Windows en tirent une image NOIRE, seul Qt sait relire son
    // propre graphe de scene. C'est ce qui rend la verification visuelle des ecrans reproductible,
    // au lieu de dependre d'un oeil devant l'ecran au bon moment.
    //
    // Declenchee par minuterie sur le fil graphique, et non depuis `frameSwapped` : ce signal est
    // emis par le FIL DE RENDU, et `grabWindow` -- qui attend ce meme fil -- n'y rendait jamais la
    // main. Le programme restait ouvert sans rien ecrire.
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

        // Filet de securite : en mode capture, le programme ne doit JAMAIS rester ouvert. Sans
        // cette sortie, un echec de chargement de la fenetre laisserait un processus vivant qu'il
        // faudrait tuer a la main -- et, en integration continue, un travail suspendu.
        QTimer::singleShot(15000, &application, []() {
            HMI_LOG_ERROR("Capture : delai depasse, aucune image produite.");
            QCoreApplication::exit(2);
        });
    }

    engine.loadFromModule("Jadg.Ui", "Main");

    const int code = QGuiApplication::exec();
    HMI_LOG_INFO("Arret de JustAnotherDnDGame (code " + std::to_string(code) + ").");
    return code;
}
