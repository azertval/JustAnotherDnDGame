// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/ApplicationTheme.h"

#include <QApplication>
#include <QColor>
#include <QFile>
#include <QFont>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleHints>
#include <filesystem>
#include <unordered_map>

#include "HMI/HmiLog.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/FontResolution.h"
#include "HMI/Interface/StyleSheetTemplate.h"
#include "HMI/Platform/ExecutableDirectory.h"

namespace hmi {

namespace {

// Cle de preference du theme de l'editeur, meme portee QSettings que la langue et le mode de rendu
// (EX-IHM-011) : aucun nouveau mecanisme de persistance a inventer.
const char* const THEME_SETTINGS_KEY = "editor_theme";

[[nodiscard]] QColor toQColor(DesignColor color) {
    return {color.r, color.g, color.b, color.a};
}

// Applique les rôles communs aux trois groupes de palette pour un jeu de jetons donné.
void setCommonRoles(QPalette& palette, QPalette::ColorGroup group, const ColorTokens& color) {
    palette.setColor(group, QPalette::Window, toQColor(color.background));
    palette.setColor(group, QPalette::WindowText, toQColor(color.text));
    palette.setColor(group, QPalette::Base, toQColor(color.surface));
    palette.setColor(group, QPalette::AlternateBase, toQColor(color.surfaceAlt));
    palette.setColor(group, QPalette::ToolTipBase, toQColor(color.surface));
    palette.setColor(group, QPalette::ToolTipText, toQColor(color.text));
    palette.setColor(group, QPalette::Text, toQColor(color.text));
    palette.setColor(group, QPalette::Button, toQColor(color.surface));
    palette.setColor(group, QPalette::ButtonText, toQColor(color.text));
    palette.setColor(group, QPalette::BrightText, toQColor(color.error));
    palette.setColor(group, QPalette::Highlight, toQColor(color.accent));
    // Texte de sélection lu sur l'accent : noir ou blanc, celui des deux qui contraste le mieux
    // avec l'accent -- fonctionne aussi bien pour l'accent clair du thème sombre que pour l'accent
    // assombri du thème clair (LOT-56 TACHE-06), sans étude de cas par thème.
    constexpr DesignColor BLACK{.r = 0, .g = 0, .b = 0};
    constexpr DesignColor WHITE{.r = 255, .g = 255, .b = 255};
    const DesignColor highlightedText =
        contrastRatio(color.accent, BLACK) >= contrastRatio(color.accent, WHITE) ? BLACK : WHITE;
    palette.setColor(group, QPalette::HighlightedText, toQColor(highlightedText));
    palette.setColor(group, QPalette::Link, toQColor(color.accent));
    palette.setColor(group, QPalette::PlaceholderText, toQColor(color.textMuted));
}

[[nodiscard]] EditorThemeSetting parseThemeSetting(const QString& value) {
    if (value == QLatin1String("light")) {
        return EditorThemeSetting::Light;
    }
    if (value == QLatin1String("dark")) {
        return EditorThemeSetting::Dark;
    }
    return EditorThemeSetting::System;
}

[[nodiscard]] const char* themeSettingName(EditorThemeSetting setting) {
    switch (setting) {
        case EditorThemeSetting::Light:
            return "light";
        case EditorThemeSetting::Dark:
            return "dark";
        case EditorThemeSetting::System:
            return "system";
    }
    return "system";
}

}  // namespace

void applyApplicationStyle() {
    // Fusion : seul style Qt fourni sur toutes les plate-formes qui honore intégralement une
    // QPalette et une feuille de style personnalisées -- les styles natifs dessinent une partie de
    // leurs contrôles via l'API du système et ignorent le reste (cf. epic.md, constat du lot).
    if (QStyle* const style = QStyleFactory::create(QStringLiteral("Fusion"))) {
        QApplication::setStyle(style);
    }
}

QPalette buildApplicationPalette(const DesignTokens& tokens) {
    QPalette palette;
    setCommonRoles(palette, QPalette::Active, tokens.color);
    setCommonRoles(palette, QPalette::Inactive, tokens.color);

    // Groupe désactivé : texte ramené au jeton atténué sur les trois rôles de texte, faute de quoi
    // Qt se rabat sur un gris calculé automatiquement, jamais garanti lisible sur le fond posé ici.
    setCommonRoles(palette, QPalette::Disabled, tokens.color);
    const QColor muted = toQColor(tokens.color.textMuted);
    palette.setColor(QPalette::Disabled, QPalette::WindowText, muted);
    palette.setColor(QPalette::Disabled, QPalette::Text, muted);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, muted);

    return palette;
}

namespace {

// Familles REELLEMENT enregistrees par applyFont(), relues par applyStyleSheet(). Vides tant que
// applyFont() n'a pas tourne, ou si Qt a refuse le fichier : la feuille de style bascule alors sur
// un mot-cle CSS generique. Cet etat est le seul moyen de faire descendre un nom resolu par Qt
// dans buildStyleSheetValues, qui est une fonction PURE et doit le rester.
struct ResolvedFamilies {
    std::string ui;
    std::string identityBody;
    std::string identityTitle;
};

ResolvedFamilies& resolvedFamilies() {
    static ResolvedFamilies families;
    return families;
}

// Facteur d'agrandissement courant des ecrans du jeu (LOT-68). Meme raison d'etre que les familles
// resolues ci-dessus : buildStyleSheetValues est pur et ne connait pas la fenetre.
int& identityScaleState() {
    static int scale = 1;
    return scale;
}

// Enregistre une famille et retourne le nom rapporte par Qt, ou une chaine vide. Les graisses
// supplementaires sont enregistrees pour que Qt puisse les selectionner, mais seul le nom de la
// graisse reguliere fait foi -- c'est lui qui nomme la famille.
[[nodiscard]] std::string registerFamily(const std::filesystem::path& regular,
                                         const std::filesystem::path& bold) {
    const int regularId =
        QFontDatabase::addApplicationFont(QString::fromStdString(regular.string()));
    const bool boldOk = bold.empty() || QFontDatabase::addApplicationFont(
                                            QString::fromStdString(bold.string())) != -1;

    std::string family;
    if (regularId != -1) {
        const QStringList families = QFontDatabase::applicationFontFamilies(regularId);
        if (!families.isEmpty()) {
            family = families.first().toStdString();
        }
    }
    const FontFamilyResolution resolution =
        resolveFontFamily(regularId != -1 && boldOk && !family.empty(), family);
    return resolution.useEmbeddedFamily ? resolution.embeddedFamily : std::string{};
}

// Nom de famille a ecrire dans la feuille de style : le nom resolu s'il existe, sinon un mot-cle
// CSS GENERIQUE. Jamais un second nom de police litteral (EX-IHM-052), et jamais celui d'un autre
// role -- une police d'ecran manquante ne doit pas faire retomber le jeu sur celle de l'editeur.
[[nodiscard]] std::string cssFamily(const std::string& resolved, FontRole role) {
    const char* const generic = genericCssFamily(role);
    if (resolved.empty()) {
        return generic;
    }
    return "\"" + resolved + "\", " + generic;
}

}  // namespace

std::string resolvedFontFamily(FontRole role) {
    return role == FontRole::Identity ? resolvedFamilies().identityBody : resolvedFamilies().ui;
}

namespace {

// Charge une feuille embarquee et y substitue les jetons. Chaine VIDE en cas d'echec : l'appelant
// laisse alors en place la feuille precedente plutot que d'en poser une a moitie substituee.
// Repli explicite journalise, jamais une exception (EX-NFR-040).
[[nodiscard]] QString loadStyleSheet(const QString& resourcePath,
                                     const DesignTokens& editorTokens) {
    QFile themeFile(resourcePath);
    if (!themeFile.open(QFile::ReadOnly | QFile::Text)) {
        HMI_LOG_WARNING("Theme d'interface introuvable (" + resourcePath.toStdString() +
                        ") : style par defaut.");
        return {};
    }
    const std::string templateText = QString::fromUtf8(themeFile.readAll()).toStdString();
    // Les familles de police sont ajoutees ICI et non dans buildStyleSheetValues : ce dernier est
    // pur (compile dans UnitTests, sans Qt) et ne peut pas connaitre un nom resolu par
    // QFontDatabase.
    std::unordered_map<std::string, std::string> values =
        buildStyleSheetValues(editorTokens, identityScaleState());
    const ResolvedFamilies& families = resolvedFamilies();
    // Seule la portee identite nomme sa famille dans la feuille de style : celle du chassis
    // est deja la police PAR DEFAUT de l'application (applyFont), et la reposer en QSS
    // ecraserait les polices que certains widgets se donnent eux-memes.
    values["identity.font.body"] = cssFamily(families.identityBody, FontRole::Identity);
    values["identity.font.title"] = cssFamily(families.identityTitle, FontRole::Identity);
    const StyleSheetSubstitutionResult substituted =
        substituteStyleSheetTemplate(templateText, values);
    if (!substituted.ok) {
        HMI_LOG_WARNING("Theme d'interface invalide (" + resourcePath.toStdString() + " : " +
                        substituted.error + ") : style par defaut.");
        return {};
    }
    return QString::fromStdString(substituted.text);
}

}  // namespace

void applyStyleSheet(const DesignTokens& editorTokens) {
    // PORTEE APPLICATION : le chassis d'edition SEUL (LOT-73, EX-IHM-082). La portee identite est
    // posee a part sur la pile d'ecrans par MainWindow -- l'y laisser faisait payer un rejeu
    // applicatif complet (862 widgets, cinq secondes en Debug) au moindre changement de facteur
    // d'agrandissement, qui ne concerne pourtant que les ecrans du jeu.
    const QString sheet =
        loadStyleSheet(QStringLiteral(":/resources/theme-editor.qss"), editorTokens);
    if (sheet.isEmpty()) {
        return;
    }
    qApp->setStyleSheet(sheet);
}

void applyFont() {
    const std::filesystem::path fonts = executableDirectory() / "Assets" / "Fonts";
    ResolvedFamilies& families = resolvedFamilies();
    families.ui = registerFamily(fonts / "Inter-Regular.ttf", fonts / "Inter-Bold.ttf");
    // Portee identite (LOT-68) : corps en Pixelify Sans, titres en Press Start 2P. Les deux sont
    // enregistrees independamment -- l'echec de l'une n'entraine pas l'autre.
    families.identityBody =
        registerFamily(fonts / "PixelifySans-Regular.ttf", fonts / "PixelifySans-Bold.ttf");
    families.identityTitle = registerFamily(fonts / "PressStart2P-Regular.ttf", {});

    if (families.ui.empty()) {
        HMI_LOG_WARNING(
            "Police du chassis introuvable ou invalide (Assets/Fonts/Inter-*.ttf) : famille "
            "generique.");
    }
    if (families.identityBody.empty()) {
        HMI_LOG_WARNING(
            "Police des ecrans du jeu introuvable ou invalide "
            "(Assets/Fonts/PixelifySans-*.ttf) : famille generique.");
    }
    if (families.identityTitle.empty()) {
        HMI_LOG_WARNING(
            "Police des titres d'ecran introuvable ou invalide "
            "(Assets/Fonts/PressStart2P-Regular.ttf) : famille generique.");
    }

    // Police PAR DEFAUT de l'application = celle du chassis d'edition. Les ecrans du jeu recoivent
    // la leur par la feuille de style (portee identite, cadree par objectName) : c'est la seule
    // facon de ne pas repandre la police pixel dans les tables et les arbres denses de l'editeur.
    QFont font;
    if (families.ui.empty()) {
        // Famille generique demandee a Qt : jamais un second nom de police code en dur (voir
        // hmi::resolveFontFamily).
        font.setStyleHint(QFont::SansSerif);
    } else {
        font = QFont(QString::fromStdString(families.ui));
    }
    font.setPointSize(identityTokens().typography.body.pointSize);
    QApplication::setFont(font);
}

EditorThemeSetting editorThemeSetting() {
    return parseThemeSetting(
        QSettings()
            .value(QString::fromLatin1(THEME_SETTINGS_KEY), QStringLiteral("system"))
            .toString());
}

void setEditorThemeSetting(EditorThemeSetting setting) {
    QSettings().setValue(QString::fromLatin1(THEME_SETTINGS_KEY),
                         QString::fromLatin1(themeSettingName(setting)));
}

bool systemPrefersDarkTheme() {
    return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
}

const DesignTokens& currentEditorTokens() {
    const EditorThemeMode mode =
        resolveEffectiveEditorTheme(editorThemeSetting(), systemPrefersDarkTheme());
    return mode == EditorThemeMode::Light ? editorLightTokens() : editorDarkTokens();
}

void applyEditorTheme() {
    applyFont();
    reapplyEditorTheme();
}

void reapplyEditorTheme() {
    const DesignTokens& tokens = currentEditorTokens();
    QApplication::setPalette(buildApplicationPalette(tokens));
    applyStyleSheet(tokens);
}

}  // namespace hmi
