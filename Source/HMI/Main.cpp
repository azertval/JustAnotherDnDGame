// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file HMI/Main.cpp
 * @brief Point d'entrée de l'application Qt (`JustAnotherDnDGame`).
 *
 * Configure la journalisation (console + mémoire en développement, niveau réglable), applique le
 * thème de l'IHM, puis ouvre la fenêtre principale (`hmi::MainWindow`) dont le widget central est
 * le viewport Direct3D 11 (`hmi::GameViewport`).
 */

#include <QApplication>
#include <QCoreApplication>
#include <QLibraryInfo>
#include <QSettings>
#include <QString>
#include <QTranslator>
#include <cstdint>
#include <ctime>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "Core/BuildConfig.h"
#include "Core/Core.h"
#include "Core/Diagnostics/ConsoleLogSink.h"
#include "Core/Diagnostics/FileLogSink.h"
#include "Core/Diagnostics/LogLevel.h"
#include "Core/Diagnostics/LogLevelParse.h"
#include "Core/Diagnostics/Logger.h"
#include "Core/Diagnostics/MemoryLogSink.h"
#include "HMI/Graphics/ProceduralAtlas.h"
#include "HMI/Graphics/TextureLoader.h"
#include "HMI/HmiLog.h"
#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/MainWindow.h"
#include "HMI/Platform/ExecutableDirectory.h"

namespace {

/// @return La valeur d'une variable d'environnement, ou `std::nullopt` si absente.
[[nodiscard]] std::optional<std::string> environmentVariable(const char* name) {
    std::size_t length = 0;
    if (getenv_s(&length, nullptr, 0, name) != 0 || length == 0) {
        return std::nullopt;
    }
    std::string value(length, '\0');
    if (getenv_s(&length, value.data(), length, name) != 0) {
        return std::nullopt;
    }
    if (!value.empty() && value.back() == '\0') {
        value.pop_back();  // getenv_s inclut le terminateur nul dans la longueur
    }
    return value;
}

/// @return La valeur d'un argument `--<nom>=…` s'il est présent sur la ligne de commande.
[[nodiscard]] std::optional<std::string> commandLineOption(int argc, char** argv,
                                                           std::string_view name) {
    for (int index = 1; index < argc; ++index) {
        const std::string_view argument = argv[index];
        if (argument.starts_with(name)) {
            return std::string(argument.substr(name.size()));
        }
    }
    return std::nullopt;
}

/// @return La valeur de l'argument `--log-level=…` s'il est présent sur la ligne de commande.
[[nodiscard]] std::optional<std::string> commandLineLogLevel(int argc, char** argv) {
    return commandLineOption(argc, argv, "--log-level=");
}

/**
 * @brief Chemin d'un fichier de log horodaté pour la session en cours, à côté de l'exécutable.
 *
 * Un nom distinct par lancement (dossier `Logs/` créé au besoin par `FileLogSink`) : un crash
 * n'écrase jamais les preuves du lancement précédent, contrairement à un nom de fichier fixe.
 */
[[nodiscard]] std::filesystem::path timestampedLogFilePath() {
    const std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_s(&local, &now);
    char stamp[32] = {};
    std::strftime(stamp, sizeof(stamp), "%Y%m%d_%H%M%S", &local);
    return hmi::executableDirectory() / "Logs" / (std::string("run_") + stamp + ".log");
}

/**
 * @brief Détermine le niveau de log minimum au lancement.
 *
 * Par défaut `Trace`. La variable d'environnement `JADG_LOG_LEVEL` puis, avec priorité,
 * l'argument `--log-level=<trace|info|warning|error>` peuvent l'ajuster. Une valeur non reconnue
 * est ignorée (signalée via @p invalidValueGiven).
 */
[[nodiscard]] core::LogLevel resolveMinimumLogLevel(int argc, char** argv,
                                                    bool& invalidValueGiven) {
    core::LogLevel level = core::LogLevel::Trace;
    invalidValueGiven = false;

    if (const std::optional<std::string> fromEnvironment =
            environmentVariable("JADG_LOG_LEVEL")) {
        if (const std::optional<core::LogLevel> parsed = core::parseLogLevel(*fromEnvironment)) {
            level = *parsed;
        } else {
            invalidValueGiven = true;
        }
    }
    if (const std::optional<std::string> fromCommandLine = commandLineLogLevel(argc, argv)) {
        if (const std::optional<core::LogLevel> parsed = core::parseLogLevel(*fromCommandLine)) {
            level = *parsed;
        } else {
            invalidValueGiven = true;
        }
    }
    return level;
}

}  // namespace

/**
 * @brief Point d'entrée du programme.
 * @return Code de sortie du processus (0 en cas de succès).
 */
int main(int argc, char** argv) {
    // Journaliseur. En développement : sortie console et capture en mémoire (`MemoryLogSink`,
    // collectée par `Core` — cf. le guide de journalisation ; export manuel possible depuis les
    // Options). En développement ET en release : fichier horodaté sous `Logs/`, flush immédiat à
    // chaque message (`FileLogSink`) — c'est ce qui reste lisible après un arrêt brutal (crash),
    // y compris en release où il n'y a ni console ni bouton d'export à atteindre après coup.
    core::MemoryLogSink* sessionLog = nullptr;
    if constexpr (core::DEVELOPER_BUILD) {
        core::defaultLogger().addSink(std::make_unique<core::ConsoleLogSink>());
        auto memorySink = std::make_unique<core::MemoryLogSink>();
        sessionLog = memorySink.get();
        core::defaultLogger().addSink(std::move(memorySink));
    }
    core::defaultLogger().addSink(std::make_unique<core::FileLogSink>(timestampedLogFilePath()));

    // Niveau de log configurable au lancement (env JADG_LOG_LEVEL ou --log-level=).
    bool invalidLogLevel = false;
    const core::LogLevel logLevel = resolveMinimumLogLevel(argc, argv, invalidLogLevel);
    core::defaultLogger().setMinimumLevel(logLevel);
    HMI_LOG_INFO(std::string("Demarrage de JustAnotherDnDGame (niveau de log : ") +
                 core::toString(logLevel) + ").");
    if (invalidLogLevel) {
        HMI_LOG_WARNING("Niveau de log fourni invalide : valeur ignoree.");
    }
    // Version du binaire et de Qt contre lequel il a ete compile (QT_VERSION_STR, fourni par les
    // en-tetes Qt) : capture dans le journal de session (LOT-61) pour qu'un rapport de defaut dise
    // contre quel Qt le binaire signale a ete construit, sans dependre d'une reproduction locale.
    HMI_LOG_INFO(std::string("JustAnotherDnDGame ") + core::Engine::version() + " (compile avec Qt " +
                 QT_VERSION_STR + ").");

    QApplication application(argc, argv);
    // Style choisi avant tout widget (LOT-56) : appliqué après, il ne se propage pas aux widgets
    // déjà construits -- condition pour que la palette et la feuille de style ci-dessous couvrent
    // l'ensemble de l'application plutôt que le sous-ensemble que le style natif honore.
    hmi::applyApplicationStyle();
    // Identité de l'application : sert de portée aux réglages persistés (QSettings — disposition
    // des panneaux de l'éditeur, EX-IHM-011).
    QCoreApplication::setOrganizationName(QStringLiteral("JustAnotherDnDGame"));
    QCoreApplication::setApplicationName(QStringLiteral("Editor"));

    // Traductions de Qt LUI-MEME (LOT-69) : les boutons standard des boites de dialogue --
    // « Oui »/« Non »/« Annuler » -- ne viennent pas du catalogue du projet mais de celui de Qt,
    // et restent en anglais tant qu'aucun QTranslator n'est installe. REGLE : un QTranslator
    // qtbase_<langue> doit etre installe sur la MEME langue que l'IHM, sans quoi une boite
    // entierement redigee en francais porterait des boutons anglais.
    // La langue est celle que l'IHM persiste (QSettings, meme cle que hmi::MainWindow) : les deux
    // catalogues doivent dire la meme chose. Fichier absent (Qt deploye sans ses traductions) :
    // on retombe simplement sur l'anglais, jamais une erreur bloquante (EX-NFR-040).
    static QTranslator qtTranslator;
    const QString language =
        QSettings().value(QStringLiteral("language"), QStringLiteral("fr")).toString();
    const QString qtCatalog = QStringLiteral("qtbase_") + language;
    // Le dossier depose a cote de l'executable D'ABORD (c'est celui d'une installation livree),
    // l'installation Qt de developpement ensuite.
    const QString deployed =
        QString::fromStdString((hmi::executableDirectory() / "Translations").string());
    if (qtTranslator.load(qtCatalog, deployed) ||
        qtTranslator.load(qtCatalog, QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        QCoreApplication::installTranslator(&qtTranslator);
    } else {
        HMI_LOG_INFO("Traductions de Qt indisponibles pour la langue '" + language.toStdString() +
                     "' : les boutons standard resteront en anglais.");
    }

    // Outil de développement (LOT-39, hors jeu) : régénère l'atlas de base en fichier PNG à partir
    // de la génération procédurale historique (seule source de vérité du contenu, cf.
    // hmi::buildProceduralAtlasImage), sans ouvrir de fenêtre. Sert à (re)créer
    // Source/Elements/Assets/atlas.png après une évolution de la génération procédurale (cf.
    // guide-rendu.md, section « pipeline de textures depuis fichiers »).
    if (const std::optional<std::string> exportAtlasPath =
            commandLineOption(argc, argv, "--export-atlas=")) {
        const hmi::ProceduralAtlasImage image = hmi::buildProceduralAtlasImage();
        const hmi::DecodedImage decoded{
            .width = image.width, .height = image.height, .pixels = image.pixels};
        // Passe par le meme chemin d'ecriture que l'atelier pixel art (LOT-54) : un seul
        // encodeur de PNG dans tout le programme.
        const bool saved = hmi::encodeImageFile(std::filesystem::path(*exportAtlasPath), decoded);
        if (!saved) {
            HMI_LOG_ERROR("Echec de l'export de l'atlas vers '" + *exportAtlasPath + "'.");
        }
        return saved ? 0 : 1;
    }

    // Thème complet (palette + feuille de style, LOT-56) du châssis d'édition (portée variable),
    // avant la construction de la fenêtre. La feuille de style couvre désormais toute l'IHM,
    // produite à partir des jetons (Source/Elements/Themes/theme-editor.qss, TACHE-02) : elle
    // remplace le chargement direct historique (repli sans fichier de thème préservé dans
    // applyStyleSheet).
    hmi::applyEditorTheme();

    hmi::MainWindow window(sessionLog);
    window.show();

    const int code = QApplication::exec();
    HMI_LOG_INFO("Arret de JustAnotherDnDGame (code " + std::to_string(code) + ").");
    return code;
}
