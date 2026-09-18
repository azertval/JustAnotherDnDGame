// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file App/Editor/Main.cpp
 * @brief Point d'entrée de l'**éditeur de niveaux** (`LevelEditor`) — Qt Widgets.
 *
 * Binaire distinct du jeu depuis le `LOT-86`. L'éditeur est un outil d'auteur : docks détachables,
 * arbres, disposition persistée (`EX-IHM-010`/`011`). Qt Widgets y est le bon outil, et le jeu, qui
 * est une image agrandie d'un facteur entier, n'a rien à faire dans le même châssis — c'est de leur
 * cohabitation que venaient les 2 472 lignes de `MainWindow.cpp`.
 *
 * Ce que ce fichier fait et que le jeu ne fait pas : choisir le style Fusion, poser le thème du
 * châssis, ouvrir la fenêtre à docks.
 */

#include <QApplication>
#include <QCoreApplication>
#include <QSettings>
#include <QString>
#include <string>

#include "App/Common/Bootstrap.h"
#include "HMI/HmiLog.h"
#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/MainWindow.h"

/**
 * @brief Point d'entrée du programme.
 * @return Code de sortie du processus (0 en cas de succès).
 */
int main(int argc, char** argv) {
    static_cast<void>(app::installLogging(argc, argv, "LevelEditor"));

    QApplication application(argc, argv);
    // Style choisi avant tout widget : appliqué après, il ne se propage pas aux widgets
    // déjà construits -- condition pour que la palette et la feuille de style ci-dessous couvrent
    // l'ensemble de l'application plutôt que le sous-ensemble que le style natif honore.
    hmi::applyApplicationStyle();
    // Identité de l'application : sert de portée aux réglages persistés (QSettings — disposition
    // des panneaux de l'éditeur, EX-IHM-011).
    QCoreApplication::setOrganizationName(QStringLiteral("JustAnotherRpgGame"));
    QCoreApplication::setApplicationName(QStringLiteral("Editor"));

    // La langue est celle que l'IHM persiste (QSettings, même clé que hmi::MainWindow) : les deux
    // catalogues doivent dire la même chose.
    const QString language =
        QSettings().value(QStringLiteral("language"), QStringLiteral("fr")).toString();
    app::installQtTranslations(language.toStdString());

    // Thème complet (palette + feuille de style) du châssis d'édition (portée variable),
    // avant la construction de la fenêtre.
    hmi::applyEditorTheme();

    hmi::MainWindow window;
    window.show();

    const int code = QApplication::exec();
    HMI_LOG_INFO("Arret de LevelEditor (code " + std::to_string(code) + ").");
    return code;
}
