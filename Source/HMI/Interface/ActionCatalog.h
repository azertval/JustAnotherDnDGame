// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>
#include <optional>

#include "HMI/Editor/EditorTool.h"
#include "HMI/Editor/PixelTool.h"
#include "HMI/Input/EditorKeyBindings.h"
#include "HMI/Interface/IconGeometry.h"

/**
 * @file HMI/Interface/ActionCatalog.h
 * @brief Catalogue des actions de l'éditeur : outils et commandes principales (`LOT-56` TACHE-04,
 *        `EX-IHM-055`).
 *
 * Logique **pure** (aucune dépendance Qt/GPU), testable hors instance d'application
 * (`EX-NFR-010`) — compilée à la fois dans `JustAnotherDnDGame` et directement dans `UnitTests`. La
 * construction Qt des `QAction` (`HMI/Interface/EditorActions.h`) est produite depuis ce
 * catalogue, pour qu'aucune commande n'ait deux définitions (menu, barre d'outils, raccourci).
 */

namespace hmi {

/// Groupe d'exclusivité d'une action : les outils d'édition forment un groupe **exclusif**
/// (un seul actif à la fois) ; les commandes n'appartiennent à aucun groupe. `PixelTools` (`LOT-54`
/// TACHE-04) est un second groupe exclusif, **distinct** de `LevelTools` : les outils du canevas
/// pixel art et ceux du niveau ne s'excluent jamais entre eux, seulement au sein de leur propre
/// groupe. `PixelCommands` (`LOT-54` TACHE-05) n'est pas exclusif (comme `None`) mais reste tagué
/// séparément : ces commandes vivent dans la barre d'outils **du canevas**, jamais dans celle du
/// niveau.
enum class EditorActionGroup { None, LevelTools, PixelTools, PixelCommands };

/// Surface d'une action (`LOT-68`, `EX-IHM-074`). `EX-IHM-055` posait qu'une commande placée à
/// plusieurs endroits reste une seule définition ; il restait à arbitrer **lesquelles** méritent
/// une place permanente à l'écran.
///
/// La barre d'outils ne porte que ce qui se déclenche au fil du geste : sélection d'outil,
/// enregistrement, annulation, essai. Tout le reste vit au menu — atteignable, découvrable
/// (`EX-EDIT-015`), mais sans occuper l'écran en permanence. C'est la même `QAction` dans les deux
/// cas : le champ décide de sa *présentation*, jamais de son existence.
enum class ActionSurface {
    ToolBarAndMenu,  ///< Barre d'outils **et** menu : usage continu.
    MenuOnly,        ///< Menu seul (plus son raccourci) : usage ponctuel.
};

/// Description d'une action, indépendante de Qt : de quoi construire un `QAction` complet (icône,
/// libellé, raccourci, caractère cochable) sans dupliquer sa définition ailleurs.
struct EditorActionSpec {
    IconId id;
    const char* labelKey;
    /// Séquence de raccourci au format `QKeySequence` (ex. `"Ctrl+S"`), chaîne vide si aucun.
    const char* shortcut;
    bool checkable;
    EditorActionGroup group;
    /// Où l'action apparaît. Porté par le catalogue, et non décidé par le code de la fenêtre :
    /// une répartition implicite ne se relit pas, et dérive au premier ajout.
    ActionSurface surface;
};

/// Nombre total d'actions du catalogue (six outils de niveau, cinq outils de canevas pixel art,
/// onze commandes principales, quatre commandes de fichier de l'atelier, quatre commandes de
/// région de l'atelier).
constexpr int EDITOR_ACTION_CATALOG_COUNT = 30;

/// @return Le catalogue complet, dans l'ordre d'affichage voulu de la barre d'outils : les huit
///         outils de niveau (ordre de la palette/du panneau Outils historique), les outils
///         de canevas pixel art (`LOT-54` TACHE-04), puis les commandes.
[[nodiscard]] const std::array<EditorActionSpec, EDITOR_ACTION_CATALOG_COUNT>&
editorActionCatalog();

/// @return La spécification de l'action @p id.
[[nodiscard]] const EditorActionSpec& editorActionSpec(IconId id);

/// Nombre d'actions **hors sélection d'outil** admises dans une barre d'outils (`LOT-68`). Sert de
/// plafond vérifié par test : c'est le nombre au-delà duquel la barre redevient le fourre-tout que
/// ce lot supprime, et un plafond qu'on relève sans y penser ne protège de rien.
constexpr int TOOLBAR_COMMAND_BUDGET = 5;

/// @return L'outil associé à l'action @p id, si elle appartient au groupe `LevelTools` ;
///         `std::nullopt` pour une commande ou un outil de canevas pixel art.
[[nodiscard]] std::optional<EditorTool> editorActionTool(IconId id);

/// @return L'identifiant d'action portant l'outil @p tool.
[[nodiscard]] IconId editorActionForTool(EditorTool tool);

/// @return L'outil de canevas pixel art associé à l'action @p id, si elle appartient au groupe
///         `PixelTools` ; `std::nullopt` sinon (`LOT-54` TACHE-04).
[[nodiscard]] std::optional<PixelTool> editorActionPixelTool(IconId id);

/// @return L'identifiant d'action portant l'outil de canevas pixel art @p tool.
[[nodiscard]] IconId editorActionForPixelTool(PixelTool tool);

/**
 * @brief Correspondance entre une action d'éditeur remappable (`hmi::EditorAction`,
 *        `EditorKeyBindings.h`) et l'identifiant d'action du catalogue qui la rend effective
 *        (`LOT-57` TACHE-04, `EX-IHM-062`).
 *
 * `EditorAction::TextureAssignTool` en est volontairement absente : elle sélectionne un outil
 * (groupe `LevelTools`), pas une commande, et reste lue directement par `hmi::GameViewport` comme
 * un raccourci brut (cf. commentaire à son site d'appel).
 */
struct KeyBindingIconEntry {
    EditorAction action;
    IconId id;
};

/// Nombre d'actions d'éditeur remappables ayant une commande effective (`EDITOR_ACTION_COUNT`
/// moins `TextureAssignTool`).
constexpr int KEY_BINDING_ICON_COUNT = 9;

/// @return La table complète action remappable -> commande du catalogue.
[[nodiscard]] const std::array<KeyBindingIconEntry, KEY_BINDING_ICON_COUNT>&
keyBindingIconCatalog();

/// @return L'identifiant de commande rendant @p action effective.
[[nodiscard]] IconId iconForKeyBindingAction(EditorAction action);

/// @return L'action remappable dont @p id est la commande effective, si elle en a une.
[[nodiscard]] std::optional<EditorAction> keyBindingActionForIcon(IconId id);

}  // namespace hmi
