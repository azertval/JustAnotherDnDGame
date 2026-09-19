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
 *
 * `--check` et `--migrate` (`LOT-EDITOR-12`, décision D9), `--apply` et `--render`
 * (`LOT-EDITOR-13`) s'exécutent **sans fenêtre** et rendent la main aussitôt : ni `QApplication` ni
 * affichage, ce qui les fait tourner en CI (`hmi::runMapCommand`, `hmi::runRenderCommand`).
 */

#include <QApplication>
#include <QCoreApplication>
#include <QString>
#include <QStyleFactory>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "App/Common/Bootstrap.h"
#include "Editor/Logic/MapFormat.h"
#include "Editor/Ui/MainWindow.h"
#include "Editor/Ui/MapRender.h"
#include "HMI/HmiLog.h"

/**
 * @brief Point d'entrée du programme.
 * @return Code de sortie du processus (0 en cas de succès).
 */
int main(int argc, char** argv) {
    static_cast<void>(app::installLogging(argc, argv, "LevelEditor", app::CrashTest::Deferred));
    const bool crashAfterAutosave = app::commandLineOption(argc, argv, "--crash-test").has_value();

    // Les commandes sans fenêtre, avant toute construction Qt. La racine des données par défaut est
    // le dossier de l'exécutable, où la construction recopie Levels/ et Assets/.
    const std::vector<std::string> arguments(argv + 1, argv + argc);
    std::string report;
    const std::filesystem::path executable = std::filesystem::absolute(argv[0]);
    if (const std::optional<int> code =
            hmi::runMapCommand(arguments, executable.parent_path(), report)) {
        std::cout << report << std::flush;
        return *code;
    }
    if (const std::optional<int> code =
            hmi::runRenderCommand(arguments, executable.parent_path(), report)) {
        std::cout << report << std::flush;
        return *code;
    }

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
