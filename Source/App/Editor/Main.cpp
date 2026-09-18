// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file App/Editor/Main.cpp
 * @brief Point d'entrée de l'**éditeur de cartes** (`LevelEditor`) — Qt Widgets.
 *
 * Binaire distinct du jeu depuis le `LOT-86`. L'éditeur est un outil interne, fait pour l'auteur
 * seul (`LOT-EDITOR-01`) : style Fusion de Qt, sans charte ni thème, textes anglais écrits dans le
 * code, sans catalogue de traduction.
 *
 * `--crash-test` ne plante pas au démarrage, comme le jeu, mais juste après la première sauvegarde
 * automatique : c'est ce qui éprouve la reprise d'un brouillon après un plantage.
 */

#include <QApplication>
#include <QCoreApplication>
#include <QString>
#include <QStyleFactory>
#include <string>

#include "App/Common/Bootstrap.h"
#include "Editor/Ui/MainWindow.h"
#include "HMI/HmiLog.h"

/**
 * @brief Point d'entrée du programme.
 * @return Code de sortie du processus (0 en cas de succès).
 */
int main(int argc, char** argv) {
    static_cast<void>(app::installLogging(argc, argv, "LevelEditor", app::CrashTest::Deferred));
    const bool crashAfterAutosave = app::commandLineOption(argc, argv, "--crash-test").has_value();

    QApplication application(argc, argv);
    // Style choisi avant tout widget : appliqué après, il ne se propage pas aux widgets déjà
    // construits. Fusion dessine pareil sur tout poste, clair ou sombre selon le système.
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    // Identité de l'application : portée des réglages persistés (QSettings — disposition des
    // panneaux, EX-IHM-011) et du dossier des brouillons de reprise (%LOCALAPPDATA%).
    QCoreApplication::setOrganizationName(QStringLiteral("JustAnotherRpgGame"));
    QCoreApplication::setApplicationName(QStringLiteral("Editor"));

    hmi::MainWindow window(crashAfterAutosave);
    window.show();

    const int code = QApplication::exec();
    HMI_LOG_INFO("Arret de LevelEditor (code " + std::to_string(code) + ").");
    return code;
}
