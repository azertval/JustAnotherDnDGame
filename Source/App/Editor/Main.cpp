// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

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
 * châssis, ouvrir la fenêtre à docks, et servir l'outil d'export d'atlas.
 */

#include <QApplication>
#include <QCoreApplication>
#include <QSettings>
#include <QString>
#include <filesystem>
#include <optional>
#include <string>

#include "App/Common/Bootstrap.h"
#include "Core/Diagnostics/MemoryLogSink.h"
#include "HMI/Graphics/ProceduralAtlas.h"
#include "HMI/Graphics/TextureLoader.h"
#include "HMI/HmiLog.h"
#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/MainWindow.h"

/**
 * @brief Point d'entrée du programme.
 * @return Code de sortie du processus (0 en cas de succès).
 */
int main(int argc, char** argv) {
    core::MemoryLogSink* sessionLog = app::installLogging(argc, argv, "LevelEditor");

    QApplication application(argc, argv);
    // Style choisi avant tout widget (LOT-56) : appliqué après, il ne se propage pas aux widgets
    // déjà construits -- condition pour que la palette et la feuille de style ci-dessous couvrent
    // l'ensemble de l'application plutôt que le sous-ensemble que le style natif honore.
    hmi::applyApplicationStyle();
    // Identité de l'application : sert de portée aux réglages persistés (QSettings — disposition
    // des panneaux de l'éditeur, EX-IHM-011).
    QCoreApplication::setOrganizationName(QStringLiteral("JustAnotherDnDGame"));
    QCoreApplication::setApplicationName(QStringLiteral("Editor"));

    // La langue est celle que l'IHM persiste (QSettings, même clé que hmi::MainWindow) : les deux
    // catalogues doivent dire la même chose.
    const QString language =
        QSettings().value(QStringLiteral("language"), QStringLiteral("fr")).toString();
    app::installQtTranslations(language.toStdString());

    // Outil de développement (LOT-39, hors jeu) : régénère l'atlas de base en fichier PNG à partir
    // de la génération procédurale historique (seule source de vérité du contenu, cf.
    // hmi::buildProceduralAtlasImage), sans ouvrir de fenêtre. Sert à (re)créer
    // Source/Elements/Assets/atlas.png après une évolution de la génération procédurale (cf.
    // guide-rendu.md, section « pipeline de textures depuis fichiers »).
    if (const std::optional<std::string_view> exportAtlasPath =
            app::commandLineOption(argc, argv, "--export-atlas=")) {
        const hmi::ProceduralAtlasImage image = hmi::buildProceduralAtlasImage();
        const hmi::DecodedImage decoded{
            .width = image.width, .height = image.height, .pixels = image.pixels};
        // Passe par le même chemin d'écriture que l'atelier pixel art (LOT-54) : un seul encodeur
        // de PNG dans tout le programme.
        const bool saved = hmi::encodeImageFile(std::filesystem::path(*exportAtlasPath), decoded);
        if (!saved) {
            HMI_LOG_ERROR("Echec de l'export de l'atlas vers '" + std::string(*exportAtlasPath) +
                          "'.");
        }
        return saved ? 0 : 1;
    }

    // Thème complet (palette + feuille de style, LOT-56) du châssis d'édition (portée variable),
    // avant la construction de la fenêtre.
    hmi::applyEditorTheme();

    hmi::MainWindow window(sessionLog);
    window.show();

    const int code = QApplication::exec();
    HMI_LOG_INFO("Arret de LevelEditor (code " + std::to_string(code) + ").");
    return code;
}
