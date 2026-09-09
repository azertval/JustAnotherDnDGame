// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file Tools/IdentityQss/Main.cpp
 * @brief Écrit la feuille d'identité RÉSOLUE, celle que Qt Designer peut appliquer (`LOT-85`,
 *        `EX-IHM-006`).
 *
 * ## Pourquoi un outil C++ et non un script
 *
 * `theme-identity.qss` est un gabarit : 108 marqueurs `${identity.*}` que
 * `hmi::substituteStyleSheetTemplate` remplace par les jetons de design, et cinq d'entre eux
 * nomment des polices résolues parmi celles réellement installées. Réécrire cette substitution en
 * Python en aurait fait une **seconde implémentation**, qui aurait dérivé de la première sans que
 * rien ne le signale — exactement le défaut que `check_design_tokens.py` et
 * `check_qt_version_pin.py` existent pour rendre impossible ailleurs.
 *
 * L'outil appelle donc le code du jeu, et rien d'autre.
 *
 * ## Ce que le fichier produit porte en tête
 *
 * Les empreintes du gabarit et de `DesignTokens.cpp`. Elles rendent la péremption détectable
 * **sans compilateur**, par simple lecture : le contrôle de la CI n'a pas de Qt à sa disposition
 * et ne pourrait pas rejouer cette résolution.
 *
 * ## Le facteur d'agrandissement
 *
 * La feuille est figée à ×2, celui des maquettes à 1280 × 720. Le jeu, lui, recalcule le sien avec
 * la hauteur de la fenêtre : cette feuille-ci ne lui est JAMAIS destinée. Elle ne sert qu'à
 * Designer, et l'écran qui la porte l'efface aussitôt après `setupUi`.
 */

#include <QApplication>
#include <QByteArray>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QTextStream>

#include "HMI/Interface/ApplicationTheme.h"

namespace {

constexpr int IDENTITY_SCALE = 2;

/// Empreinte d'un fichier source, ou une chaîne vide s'il est illisible.
QString fingerprint(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&file)) {
        return {};
    }
    return QString::fromLatin1(hash.result().toHex());
}

}  // namespace

int main(int argc, char** argv) {
    // QApplication et non QCoreApplication : `hmi::applyFont` enregistre les fontes par
    // QFontDatabase, qui exige une application graphique.
    QApplication app(argc, argv);

    const QString repoRoot = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QStringLiteral(".");
    QTextStream err(stderr);

    const QDir root(repoRoot);
    const QString templatePath =
        root.filePath(QStringLiteral("Source/Elements/Themes/theme-identity.qss"));
    const QString tokensPath =
        root.filePath(QStringLiteral("Source/HMI/Interface/DesignTokens.cpp"));
    const QString outputPath =
        root.filePath(QStringLiteral("Source/Elements/Themes/theme-identity.resolved.qss"));

    const QString templateHash = fingerprint(templatePath);
    const QString tokensHash = fingerprint(tokensPath);
    if (templateHash.isEmpty() || tokensHash.isEmpty()) {
        err << "identity-qss : source illisible (" << templatePath << " ou " << tokensPath
            << ").\n";
        return 1;
    }

    // Les polices AVANT la résolution : cinq marqueurs nomment une famille, et sans cet appel
    // elles se résoudraient sur une police de repli — la feuille décrirait alors un jeu qui
    // n'existe pas.
    hmi::applyFont();
    hmi::setIdentityScale(IDENTITY_SCALE);

    const QString sheet = hmi::identityStyleSheet();
    if (sheet.isEmpty()) {
        err << "identity-qss : feuille vide — la ressource :/resources/theme-identity.qss n'a pas "
               "ete trouvee.\n";
        return 1;
    }

    QFile output(outputPath);
    // Sans QIODevice::Text : ce mode traduirait les fins de ligne en CRLF sous Windows, et
    // le fichier produit divergerait de la normalisation LF du depot (`.gitattributes`) a
    // chaque regeneration.
    if (!output.open(QIODevice::WriteOnly)) {
        err << "identity-qss : ecriture impossible (" << outputPath << ").\n";
        return 1;
    }

    QTextStream out(&output);
    out << "/* GENERE — ne pas editer a la main.\n"
        << "   Produit par Source/Tools/IdentityQss (LOT-85, EX-IHM-006) :\n"
        << "       powershell -File scripts/designer.ps1\n\n"
        << "   Cette feuille est la resolution de theme-identity.qss au facteur x2, celui des\n"
        << "   maquettes. Elle est destinee a Qt Designer, et a lui SEUL : le jeu recalcule son\n"
        << "   facteur avec la hauteur de la fenetre, et l'ecran efface cette feuille aussitot\n"
        << "   apres setupUi.\n\n"
        << "   Les empreintes ci-dessous rendent sa peremption detectable sans compilateur —\n"
        << "   scripts/check_ui_designer.py les recoupe, sur un runner qui n'a pas Qt.\n"
        << "   identity-scale: " << IDENTITY_SCALE << "\n"
        << "   source-template: " << templateHash << "\n"
        << "   source-tokens: " << tokensHash << "\n"
        << "*/\n\n"
        << sheet;
    out.flush();
    output.close();

    QTextStream(stdout) << "identity-qss : " << QFileInfo(outputPath).fileName() << " ecrit ("
                        << sheet.size() << " caracteres, facteur x" << IDENTITY_SCALE << ").\n";
    return 0;
}
