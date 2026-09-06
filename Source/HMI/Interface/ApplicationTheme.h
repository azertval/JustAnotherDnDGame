// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPalette>
#include <QString>
#include <string>

#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/ThemeResolution.h"

/**
 * @file HMI/Interface/ApplicationTheme.h
 * @brief Application du système de design de l'IHM Qt (`LOT-56`) : style, palettes et thème.
 *
 * Couche **Qt** au-dessus des jetons purs (`HMI/Interface/DesignTokens.h`) : construit la
 * `QPalette` de l'application et choisit son style, avant la création du moindre widget
 * (`EX-IHM-050`). Non compilée dans `UnitTests` (dépend de Qt), à l'inverse de `DesignTokens.cpp`.
 */

namespace hmi {

struct DesignTokens;

/// Choisit explicitement le style Qt de l'application (non natif, condition pour que la palette et
/// la feuille de style s'appliquent à l'ensemble des contrôles). À appeler **avant** la création du
/// moindre widget : un style appliqué après ne se propage pas aux widgets déjà construits.
void applyApplicationStyle();

/// Construit une `QPalette` complète — groupes actif, inactif et désactivé — à partir d'un jeu de
/// jetons. Le groupe désactivé est explicitement dérivé du texte atténué : c'est celui qu'on
/// oublie le plus souvent, et celui qui trahit le plus vite un thème incomplet.
[[nodiscard]] QPalette buildApplicationPalette(const DesignTokens& tokens);

/// Charge le modèle de feuille de style du **châssis d'édition** (`:/resources/theme-editor.qss`),
/// le substitue avec @p editorTokens et l'applique à l'**application**. Repli explicite : fichier
/// absent/illisible ou marqueur inconnu -> avertissement journalisé, l'application reste utilisable
/// sans feuille de style (comportement historique de `main.cpp`, non régressé).
///
/// Ne porte **plus** la portée identité depuis le `LOT-73` (`EX-IHM-082`) : celle-ci se pose sur la
/// pile d'écrans (`identityStyleSheet`). Les deux vivaient dans la même feuille applicative, si
/// bien qu'un changement de facteur d'agrandissement — préoccupation des seuls écrans du jeu —
/// repolissait les 862 widgets de l'application, cinq secondes durant en Debug.
void applyStyleSheet(const DesignTokens& editorTokens);

/// Feuille de style de la **portée identité** (`:/resources/theme-identity.qss`), substituée avec
/// les jetons effectifs et le facteur d'agrandissement courant (`identityScale`).
///
/// Rendue plutôt qu'appliquée : c'est l'appelant qui sait **où** la poser — sur la pile d'écrans,
/// et non sur l'application (`EX-IHM-082`). Chaîne vide si la feuille est absente ou invalide ;
/// l'appelant laisse alors en place la précédente.
[[nodiscard]] QString identityStyleSheet();

/// Enregistre les polices embarquées auprès de Qt et applique la famille du **châssis
/// d'édition** comme police par défaut de l'application (`Assets/Fonts/`) :
/// - `Inter-{Regular,Bold}.ttf` — `FontRole::Ui`, le châssis d'édition ;
/// - `PixelifySans-{Regular,Bold}.ttf` et `PressStart2P-Regular.ttf` — `FontRole::Identity`, les
///   écrans du jeu (`LOT-68`, `EX-IHM-070`), la seconde étant réservée aux titres d'écran.
///
/// Repli explicite et **par famille** si un fichier est absent ou refusé par Qt : famille
/// **générique** (`QFont::StyleHint` ici, mot-clé CSS générique dans la feuille de style), jamais
/// un second nom de police codé en dur, et jamais la famille d'un autre rôle — une police d'écran
/// manquante ne doit pas faire retomber le jeu sur la police de l'éditeur. Avertissement
/// journalisé.
void applyFont();

/// Nom de famille **effectivement** enregistré pour la police de corps de @p role, ou une chaîne
/// vide si l'enregistrement a échoué (l'appelant emploie alors une famille générique). Renseigné
/// par `applyFont()` ; vide tant qu'elle n'a pas été appelée.
[[nodiscard]] std::string resolvedFontFamily(FontRole role);

/// Nom de famille des **titres d'écran** de la portée identité (`Press Start 2P`), ou une chaîne
/// vide en cas d'échec. Distincte de `resolvedFontFamily(FontRole::Identity)` : une police de
/// titre très typée serait illisible en corps de texte.
[[nodiscard]] std::string resolvedIdentityTitleFamily();

/// Fixe le facteur d'agrandissement **entier** des écrans du jeu (`hmi::identityScaleFor`), relu
/// par `applyStyleSheet` (`LOT-68`, `EX-IHM-070`). Un réglage, pas un calcul : c'est la fenêtre qui
/// connaît sa hauteur, et elle seule.
/// @return `true` si la valeur a changé — l'appelant sait alors qu'il doit rejouer le thème.
bool setIdentityScale(int scale);

/// @return Le facteur d'agrandissement courant des écrans du jeu (1 tant qu'aucune fenêtre ne l'a
/// fixé).
[[nodiscard]] int identityScale();

/// @return Le réglage de thème persisté (`QSettings`), `Système` par défaut.
[[nodiscard]] EditorThemeSetting editorThemeSetting();

/// Persiste @p setting (`QSettings`), relu au démarrage suivant.
void setEditorThemeSetting(EditorThemeSetting setting);

/// @return `true` si le système d'exploitation est actuellement réglé sur un thème sombre
/// (`QStyleHints::colorScheme`).
[[nodiscard]] bool systemPrefersDarkTheme();

/// @return Les jetons du châssis d'édition **actuellement effectifs** : résolution du réglage
/// persisté et, si `Système`, du réglage courant du système d'exploitation
/// (`hmi::resolveEffectiveEditorTheme`).
[[nodiscard]] const DesignTokens& currentEditorTokens();

/// Applique le thème complet du châssis d'édition (portée **variable**) : police, palette puis
/// feuille de style, tous dérivés des jetons **actuellement effectifs**
/// (`currentEditorTokens()`). À appeler avant la construction de `MainWindow`.
void applyEditorTheme();

/// Réapplique la palette et la feuille de style (mais pas la police, inchangée par le thème)
/// depuis `currentEditorTokens()` — bascule à chaud (`LOT-56` TACHE-06), sans reconstruire les
/// widgets. Les icônes et vignettes ne suivent **pas** automatiquement : leurs propriétaires
/// (`EditorActions::refreshIcons`, caches de vignettes) doivent être resynchronisés séparément.
void reapplyEditorTheme();

}  // namespace hmi
