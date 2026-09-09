// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file Tools/RpgScreenUi/Main.cpp
 * @brief Engendre UNE FOIS le `.ui` de chaque écran du RPG (`LOT-85`, `EX-IHM-006`).
 *
 * ## Ce que cet outil sert à faire, et à ne plus refaire
 *
 * Huit des neuf écrans du RPG n'avaient aucun `.ui` : `hmi::RpgScreenFrame` les bâtissait à
 * l'exécution depuis la table `hmi::rpgScreens()`. C'était le bon outil tant qu'aucune maquette
 * n'existait — un neuvième écran ne coûtait qu'une ligne de table — mais cela rendait ces écrans
 * **impossibles à retoucher** : il n'y avait rien à ouvrir dans Qt Designer.
 *
 * Cet outil produit, depuis cette même table, la planche de départ de chaque écran. Il s'exécute
 * **une fois** : les fichiers produits deviennent ensuite la propriété de qui les édite, et les
 * réexécuter écraserait ces retouches. Il refuse donc d'écraser un fichier existant sans
 * `--force`.
 *
 * ## Ce que le fichier porte, et ce qu'il ne porte pas
 *
 * Il porte la **structure**, les **rôles** (`rpgRole`, que le thème cible), les **clés de
 * traduction** (`rpgKey`) et les **identifiants de valeur** (`rpgValue`, ceux de la table, mot pour
 * mot).
 *
 * Il ne porte **aucune grandeur** : espacements, retraits et côtés de case se calculent depuis les
 * jetons multipliés par le facteur d'agrandissement, et les figer dans le fichier les arrêterait au
 * facteur du jour où l'outil a tourné. Le code les applique après chargement, **par rôle**.
 */

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QTextStream>

#include "HMI/Interface/RpgScreens.h"

namespace {

/// Un cran d'indentation, comme Qt Designer les écrit : un espace par niveau.
QString pas(int niveau) {
    return QString(niveau, QLatin1Char(' '));
}

QString echappe(const QString& brut) {
    QString sortie = brut;
    sortie.replace(QLatin1Char('&'), QStringLiteral("&amp;"));
    sortie.replace(QLatin1Char('<'), QStringLiteral("&lt;"));
    sortie.replace(QLatin1Char('>'), QStringLiteral("&gt;"));
    return sortie;
}

/// Une propriété dynamique — c'est par elles que le code retrouve ce que le fichier décrit.
QString propriete(int niveau, const char* nom, const QString& valeur) {
    return QStringLiteral("%1<property name=\"%2\" stdset=\"0\"><string>%3</string></property>\n")
        .arg(pas(niveau), QLatin1String(nom), echappe(valeur));
}

QString etiquette(int niveau, const QString& nom, const char* role, const QString& cleOuValeur,
                  bool estValeur) {
    QString sortie = QStringLiteral("%1<widget class=\"QLabel\" name=\"%2\">\n")
                         .arg(pas(niveau), nom);
    sortie += QStringLiteral("%1<property name=\"text\"><string/></property>\n").arg(pas(niveau + 1));
    sortie += propriete(niveau + 1, "rpgRole", QLatin1String(role));
    if (!cleOuValeur.isEmpty()) {
        sortie += propriete(niveau + 1, estValeur ? "rpgValue" : "rpgKey", cleOuValeur);
    }
    sortie += QStringLiteral("%1</widget>\n").arg(pas(niveau));
    return sortie;
}

/// Un cadre nu, dont le rôle dit au code quelle taille lui donner (case, jeton, action…).
QString cadre(int niveau, const QString& nom, const char* role) {
    QString sortie = QStringLiteral("%1<widget class=\"QFrame\" name=\"%2\">\n").arg(pas(niveau), nom);
    sortie += propriete(niveau + 1, "rpgRole", QLatin1String(role));
    sortie += QStringLiteral("%1</widget>\n").arg(pas(niveau));
    return sortie;
}

QString article(int niveau, const QString& contenu) {
    return QStringLiteral("%1<item>\n%2%1</item>\n").arg(pas(niveau), contenu);
}

/// Le corps d'un bloc, selon son genre. Reproduit ce que `hmi::RpgScreenFrame::buildBlock` peint.
///
/// Les genres qui rendent une DISPOSITION (`Fields`, `Grid`, `Track`, `ActionBar`) la rendent nue :
/// `corpsEnveloppe` la met dans son `<item>`, sans quoi `uic` refuse le fichier — un `<layout>`
/// posé directement dans un `<layout>` n'est pas du `.ui` valide.
QString corpsDuBloc(const hmi::RpgContentBlock& bloc, const QString& prefixe, int niveau);

/// Le corps d'un bloc, prêt à être posé dans la disposition d'un encart.
QString corpsEnveloppe(const hmi::RpgContentBlock& bloc, const QString& prefixe, int niveau) {
    switch (bloc.kind) {
        case hmi::RpgBlockKind::Fields:
        case hmi::RpgBlockKind::Grid:
        case hmi::RpgBlockKind::Track:
        case hmi::RpgBlockKind::ActionBar:
            return article(niveau, corpsDuBloc(bloc, prefixe, niveau + 1));
        default:
            return corpsDuBloc(bloc, prefixe, niveau);
    }
}

QString corpsDuBloc(const hmi::RpgContentBlock& bloc, const QString& prefixe, int niveau) {
    QString sortie;
    switch (bloc.kind) {
        case hmi::RpgBlockKind::Fields: {
            sortie += QStringLiteral("%1<layout class=\"QGridLayout\" name=\"%2Grid\">\n")
                          .arg(pas(niveau), prefixe);
            int ligne = 0;
            for (const hmi::RpgField& champ : bloc.fields) {
                const QString suffixe = QStringLiteral("%1Field%2").arg(prefixe).arg(ligne);
                sortie += QStringLiteral("%1<item row=\"%2\" column=\"0\">\n")
                              .arg(pas(niveau + 1))
                              .arg(ligne);
                sortie += etiquette(niveau + 2, suffixe + QStringLiteral("Label"), "field",
                                    QString::fromLatin1(champ.labelKey), false);
                sortie += QStringLiteral("%1</item>\n").arg(pas(niveau + 1));
                sortie += QStringLiteral("%1<item row=\"%2\" column=\"1\">\n")
                              .arg(pas(niveau + 1))
                              .arg(ligne);
                sortie += etiquette(niveau + 2, suffixe + QStringLiteral("Value"), "value",
                                    QString::fromLatin1(champ.valueId), true);
                sortie += QStringLiteral("%1</item>\n").arg(pas(niveau + 1));
                ++ligne;
            }
            sortie += QStringLiteral("%1</layout>\n").arg(pas(niveau));
            break;
        }
        case hmi::RpgBlockKind::Grid: {
            sortie += QStringLiteral("%1<layout class=\"QGridLayout\" name=\"%2Cells\">\n")
                          .arg(pas(niveau), prefixe);
            for (int ligne = 0; ligne < bloc.rows; ++ligne) {
                for (int colonne = 0; colonne < bloc.columns; ++colonne) {
                    sortie += QStringLiteral("%1<item row=\"%2\" column=\"%3\">\n")
                                  .arg(pas(niveau + 1))
                                  .arg(ligne)
                                  .arg(colonne);
                    sortie += cadre(niveau + 2,
                                    QStringLiteral("%1Cell%2_%3").arg(prefixe).arg(ligne).arg(colonne),
                                    "cell");
                    sortie += QStringLiteral("%1</item>\n").arg(pas(niveau + 1));
                }
            }
            sortie += QStringLiteral("%1</layout>\n").arg(pas(niveau));
            break;
        }
        case hmi::RpgBlockKind::List: {
            const int lignes = bloc.valueIds.empty() ? bloc.rows
                                                     : static_cast<int>(bloc.valueIds.size());
            for (int ligne = 0; ligne < lignes; ++ligne) {
                const QString nom = QStringLiteral("%1Row%2").arg(prefixe).arg(ligne);
                QString rangee = QStringLiteral("%1<widget class=\"QFrame\" name=\"%2\">\n")
                                     .arg(pas(niveau + 1), nom);
                rangee += propriete(niveau + 2, "rpgRole", QStringLiteral("row"));
                rangee += QStringLiteral("%1<layout class=\"QHBoxLayout\" name=\"%2Layout\">\n")
                              .arg(pas(niveau + 2), nom);
                const QString identifiant =
                    bloc.valueIds.empty() ? QString()
                                          : QString::fromLatin1(bloc.valueIds[static_cast<std::size_t>(ligne)]);
                rangee += article(niveau + 3,
                                  etiquette(niveau + 4, nom + QStringLiteral("Value"), "value",
                                            identifiant, true));
                rangee += QStringLiteral("%1</layout>\n").arg(pas(niveau + 2));
                rangee += QStringLiteral("%1</widget>\n").arg(pas(niveau + 1));
                sortie += article(niveau, rangee);
            }
            break;
        }
        case hmi::RpgBlockKind::Prose: {
            const QString identifiant =
                bloc.valueIds.empty() ? QString() : QString::fromLatin1(bloc.valueIds.front());
            QString prose = QStringLiteral("%1<widget class=\"QLabel\" name=\"%2Prose\">\n")
                                .arg(pas(niveau + 1), prefixe);
            prose += QStringLiteral("%1<property name=\"text\"><string/></property>\n")
                         .arg(pas(niveau + 2));
            prose += QStringLiteral("%1<property name=\"wordWrap\"><bool>true</bool></property>\n")
                         .arg(pas(niveau + 2));
            prose += QStringLiteral(
                         "%1<property name=\"alignment\"><set>Qt::AlignLeft|Qt::AlignTop</set>"
                         "</property>\n")
                         .arg(pas(niveau + 2));
            prose += propriete(niveau + 2, "rpgRole", QStringLiteral("prose"));
            if (!identifiant.isEmpty()) {
                prose += propriete(niveau + 2, "rpgValue", identifiant);
            }
            prose += QStringLiteral("%1</widget>\n").arg(pas(niveau + 1));
            sortie += article(niveau, prose);
            break;
        }
        case hmi::RpgBlockKind::Portrait: {
            sortie += article(niveau, cadre(niveau + 1, prefixe + QStringLiteral("Illustration"),
                                            "illustration"));
            break;
        }
        case hmi::RpgBlockKind::Track: {
            sortie += QStringLiteral("%1<layout class=\"QHBoxLayout\" name=\"%2Strip\">\n")
                          .arg(pas(niveau), prefixe);
            for (int jeton = 0; jeton < bloc.columns; ++jeton) {
                sortie += article(niveau + 1,
                                  cadre(niveau + 2,
                                        QStringLiteral("%1Chip%2").arg(prefixe).arg(jeton), "chip"));
            }
            sortie += QStringLiteral("%1<item>\n%2<spacer name=\"%3StripSpacer\">\n")
                          .arg(pas(niveau + 1), pas(niveau + 2), prefixe);
            sortie += QStringLiteral(
                          "%1<property name=\"orientation\"><enum>Qt::Horizontal</enum></property>\n")
                          .arg(pas(niveau + 3));
            sortie += QStringLiteral("%1</spacer>\n%2</item>\n").arg(pas(niveau + 2), pas(niveau + 1));
            sortie += QStringLiteral("%1</layout>\n").arg(pas(niveau));
            break;
        }
        case hmi::RpgBlockKind::ActionBar: {
            sortie += QStringLiteral("%1<layout class=\"QHBoxLayout\" name=\"%2Bar\">\n")
                          .arg(pas(niveau), prefixe);
            for (int action = 0; action < bloc.columns; ++action) {
                sortie += article(niveau + 1,
                                  cadre(niveau + 2,
                                        QStringLiteral("%1Action%2").arg(prefixe).arg(action),
                                        "action"));
            }
            sortie += QStringLiteral("%1</layout>\n").arg(pas(niveau));
            break;
        }
    }
    return sortie;
}

/// Un encart : le parchemin, son retrait, sa zone défilante, et le contenu du bloc.
QString encart(const hmi::RpgContentBlock& bloc, const QString& prefixe, int niveau) {
    QString interieur;
    if (bloc.titleKey[0] != '\0') {
        interieur += article(niveau + 5,
                             etiquette(niveau + 6, prefixe + QStringLiteral("Title"), "block",
                                       QString::fromLatin1(bloc.titleKey), false));
    }
    interieur += corpsEnveloppe(bloc, prefixe, niveau + 5);

    QString sortie = QStringLiteral("%1<widget class=\"hmi::ParchmentPanel\" name=\"%2Panel\">\n")
                         .arg(pas(niveau + 1), prefixe);
    sortie += QStringLiteral("%1<layout class=\"QVBoxLayout\" name=\"%2Inset\">\n")
                  .arg(pas(niveau + 2), prefixe);
    sortie += QStringLiteral("%1<item>\n").arg(pas(niveau + 3));
    sortie += QStringLiteral("%1<widget class=\"QScrollArea\" name=\"%2Scroll\">\n")
                  .arg(pas(niveau + 4), prefixe);
    sortie += QStringLiteral("%1<property name=\"widgetResizable\"><bool>true</bool></property>\n")
                  .arg(pas(niveau + 5));
    sortie += QStringLiteral(
                  "%1<property name=\"frameShape\"><enum>QFrame::NoFrame</enum></property>\n")
                  .arg(pas(niveau + 5));
    sortie += QStringLiteral("%1<widget class=\"QWidget\" name=\"%2Content\">\n")
                  .arg(pas(niveau + 5), prefixe);
    sortie += QStringLiteral("%1<layout class=\"QVBoxLayout\" name=\"%2Layout\">\n")
                  .arg(pas(niveau + 6), prefixe);
    sortie += interieur;
    sortie += QStringLiteral("%1</layout>\n").arg(pas(niveau + 6));
    sortie += QStringLiteral("%1</widget>\n").arg(pas(niveau + 5));
    sortie += QStringLiteral("%1</widget>\n").arg(pas(niveau + 4));
    sortie += QStringLiteral("%1</item>\n").arg(pas(niveau + 3));
    sortie += QStringLiteral("%1</layout>\n").arg(pas(niveau + 2));
    sortie += QStringLiteral("%1</widget>\n").arg(pas(niveau + 1));
    return article(niveau, sortie);
}

QString colonne(std::span<const hmi::RpgContentBlock> blocs, const QString& nom, int niveau) {
    if (blocs.empty()) {
        return {};
    }
    QString sortie = QStringLiteral("%1<widget class=\"QWidget\" name=\"%2\">\n").arg(pas(niveau + 1), nom);
    sortie += QStringLiteral("%1<layout class=\"QVBoxLayout\" name=\"%2Layout\">\n")
                  .arg(pas(niveau + 2), nom);
    int index = 0;
    for (const hmi::RpgContentBlock& bloc : blocs) {
        sortie += encart(bloc, QStringLiteral("%1Block%2").arg(nom).arg(index), niveau + 3);
        ++index;
    }
    sortie += QStringLiteral("%1<item>\n%2<spacer name=\"%3Stretch\">\n")
                  .arg(pas(niveau + 3), pas(niveau + 4), nom);
    sortie += QStringLiteral("%1<property name=\"orientation\"><enum>Qt::Vertical</enum></property>\n")
                  .arg(pas(niveau + 5));
    sortie += QStringLiteral("%1</spacer>\n%2</item>\n").arg(pas(niveau + 4), pas(niveau + 3));
    sortie += QStringLiteral("%1</layout>\n").arg(pas(niveau + 2));
    sortie += QStringLiteral("%1</widget>\n").arg(pas(niveau + 1));
    return article(niveau, sortie);
}

QString planche(const hmi::RpgScreenDescriptor& ecran) {
    const QString nom = QString::fromLatin1(ecran.objectName);

    QString sortie = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<ui version=\"4.0\">\n");
    sortie += QStringLiteral(" <class>%1</class>\n").arg(nom);
    sortie += QStringLiteral(" <widget class=\"QWidget\" name=\"%1\">\n").arg(nom);
    sortie += QStringLiteral("  <layout class=\"QVBoxLayout\" name=\"pageLayout\">\n");

    sortie += article(3, QStringLiteral(
                             "    <widget class=\"hmi::TitleBanner\" name=\"rpgTitle\">\n"
                             "     <property name=\"text\"><string/></property>\n"
                             "    </widget>\n"));

    QString corps = QStringLiteral("    <widget class=\"QWidget\" name=\"bodyHost\">\n");
    corps += QStringLiteral("     <layout class=\"QHBoxLayout\" name=\"bodyRow\">\n");
    corps += colonne(ecran.layout.leftColumn, QStringLiteral("leftColumn"), 6);
    corps += colonne(ecran.layout.rightColumn, QStringLiteral("rightColumn"), 6);
    corps += QStringLiteral("     </layout>\n    </widget>\n");
    sortie += article(3, corps);

    QString pied = QStringLiteral("    <layout class=\"QHBoxLayout\" name=\"footerRow\">\n");
    for (const char* bouton : {"closeButton", "previousButton", "nextButton"}) {
        pied += article(5, QStringLiteral("      <widget class=\"hmi::MenuEntryButton\" name=\"%1\">\n"
                                          "       <property name=\"text\"><string/></property>\n"
                                          "      </widget>\n")
                               .arg(QLatin1String(bouton)));
    }
    pied += article(5, QStringLiteral("      <widget class=\"QLabel\" name=\"hintsLabel\">\n"
                                      "       <property name=\"text\"><string/></property>\n"
                                      "      </widget>\n"));
    pied += QStringLiteral("    </layout>\n");
    sortie += article(3, pied);

    sortie += QStringLiteral("  </layout>\n </widget>\n");
    sortie += QStringLiteral(
        " <customwidgets>\n"
        "  <customwidget>\n"
        "   <class>hmi::TitleBanner</class>\n"
        "   <extends>QLabel</extends>\n"
        "   <header>HMI/Interface/TitleBanner.h</header>\n"
        "  </customwidget>\n"
        "  <customwidget>\n"
        "   <class>hmi::ParchmentPanel</class>\n"
        "   <extends>QWidget</extends>\n"
        "   <header>HMI/Interface/ParchmentPanel.h</header>\n"
        "   <container>1</container>\n"
        "  </customwidget>\n"
        "  <customwidget>\n"
        "   <class>hmi::MenuEntryButton</class>\n"
        "   <extends>QPushButton</extends>\n"
        "   <header>HMI/Interface/MenuEntryButton.h</header>\n"
        "  </customwidget>\n"
        " </customwidgets>\n"
        " <resources/>\n"
        " <connections/>\n"
        "</ui>\n");
    return sortie;
}

}  // namespace

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    const QStringList arguments = app.arguments();
    const bool force = arguments.contains(QStringLiteral("--force"));
    const QDir racine(arguments.size() > 1 && !arguments.at(1).startsWith(QLatin1Char('-'))
                          ? arguments.at(1)
                          : QStringLiteral("."));

    QTextStream out(stdout);
    QTextStream err(stderr);
    int ecrits = 0;
    int gardes = 0;

    for (const hmi::RpgScreenDescriptor& ecran : hmi::rpgScreens()) {
        // La fiche a déjà sa planche, écrite à la main : l'outil n'a rien à y faire.
        if (ecran.rendering == hmi::RpgRendering::DesignerPlate) {
            continue;
        }
        const QString chemin =
            racine.filePath(QStringLiteral("Source/Elements/UI/%1.ui")
                                .arg(QString::fromLatin1(ecran.objectName)));
        if (QFile::exists(chemin) && !force) {
            out << "  garde  " << QString::fromLatin1(ecran.objectName)
                << ".ui (existe deja ; --force pour ecraser)\n";
            ++gardes;
            continue;
        }
        QFile fichier(chemin);
        if (!fichier.open(QIODevice::WriteOnly)) {
            err << "rpg-screen-ui : ecriture impossible (" << chemin << ").\n";
            return 1;
        }
        QTextStream flux(&fichier);
        flux << planche(ecran);
        flux.flush();
        fichier.close();
        out << "  ecrit  " << QString::fromLatin1(ecran.objectName) << ".ui\n";
        ++ecrits;
    }

    out << "rpg-screen-ui : " << ecrits << " planche(s) engendree(s), " << gardes << " gardee(s).\n";
    return 0;
}
