// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_editor_actions.cpp
 * @brief Tests unitaires du catalogue d'actions et de la géométrie des icônes de l'éditeur
 *        (`LOT-56` TACHE-04, `EX-IHM-055`).
 */

#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>

#include <gtest/gtest.h>

#include "HMI/Interface/ActionCatalog.h"
#include "HMI/Interface/IconGeometry.h"
#include "HMI/Localization/Localization.h"

namespace {

[[nodiscard]] std::unordered_map<std::string, std::string> readCatalog(const char* path) {
    std::ifstream file(path);
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return hmi::Localization::parseCatalog(buffer.str());
}

}  // namespace

/**
 * @brief Les outils du catalogue forment une bijection exacte avec `hmi::EditorTool` : un outil,
 *        une action, aucun manquant ni dupliqué. C'est ce qui garantit l'exclusivité réelle (le
 *        groupe Qt `QActionGroup`, construit à partir de ce même catalogue, EditorActions.cpp) et,
 *        surtout, la **complétude** : `hmi::MainWindow` dérive de ce catalogue les connexions
 *        action → `GameViewport::setTool`, donc un outil absent du groupe `LevelTools` serait
 *        cochable dans la barre d'outils sans jamais devenir l'outil actif. Le cas s'est produit
 *        avec l'outil « Parcours » (`LOT-67`), qu'une liste écrite à la main avait laissé de côté.
 *
 * La liste des outils est **dérivée du catalogue**, jamais recopiée ici : la recopier reproduirait
 * exactement l'erreur que ce test doit détecter. `EDITOR_TOOL_COUNT`, déclaré à côté de
 * l'énumération, ferme la boucle — autant d'actions distinctes que d'outils implique, par
 * tiroirs, que chaque outil en a exactement une.
 * \castest{<b>Les outils du catalogue forment une bijection avec EditorTool.</b><br/>
 * \tcat Unitaire · Actions de l'editeur<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Parcourir les actions du groupe LevelTools du catalogue.<br/>2. Convertir chacune en
 * outil puis reconvertir vers l'action.<br/>
 * \tattendu Chaque action du groupe designe un outil distinct, l'aller-retour est fidele, et le
 * nombre d'outils couverts vaut EDITOR_TOOL_COUNT : aucun outil n'est laisse sans action.
 * }
 */
TEST(EditorActionsTest, LesOutilsDuCatalogueFormentUneBijectionAvecEditorTool) {
    std::set<hmi::IconId> seenIcons;
    std::set<hmi::EditorTool> seenTools;
    for (const hmi::EditorActionSpec& spec : hmi::editorActionCatalog()) {
        if (spec.group != hmi::EditorActionGroup::LevelTools) {
            continue;
        }
        const std::optional<hmi::EditorTool> tool = hmi::editorActionTool(spec.id);
        ASSERT_TRUE(tool.has_value()) << "une action du groupe LevelTools ne designe aucun outil";
        EXPECT_TRUE(seenTools.insert(*tool).second) << "deux actions pour le meme outil";
        EXPECT_TRUE(seenIcons.insert(spec.id).second) << "action dupliquee dans le catalogue";
        EXPECT_EQ(hmi::editorActionForTool(*tool), spec.id);
    }
    EXPECT_EQ(seenTools.size(), hmi::EDITOR_TOOL_COUNT)
        << "un outil de EditorTool n'a pas d'action dans le groupe LevelTools : il serait "
           "cochable sans jamais devenir l'outil actif";
}

/**
 * @brief Les quatre outils du canevas pixel art forment une bijection exacte avec `hmi::PixelTool`
 *        (`LOT-54` TACHE-04) — même garantie que pour les six outils de niveau, et le groupe
 *        `PixelTools` reste **distinct** de `LevelTools` (aucun outil de canevas n'appartient au
 *        groupe des outils de niveau).
 * \castest{<b>Les quatre outils de canevas forment une bijection avec PixelTool.</b><br/>
 * \tcat Unitaire · Actions de l'editeur<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Pour chaque hmi::PixelTool, resoudre l'action puis reconvertir vers l'outil.<br/>
 * 2. Verifier que l'action appartient au groupe PixelTools.<br/>
 * \tattendu L'aller-retour restitue l'outil d'origine pour chacun des cinq, et chaque action
 * appartient au groupe PixelTools.
 * }
 */
TEST(EditorActionsTest, LesCinqOutilsDeCanevasFormentUneBijectionAvecPixelTool) {
    constexpr hmi::PixelTool tools[] = {hmi::PixelTool::Brush, hmi::PixelTool::Eraser,
                                        hmi::PixelTool::Fill, hmi::PixelTool::Eyedropper,
                                        hmi::PixelTool::Selection};
    std::set<hmi::IconId> seen;
    for (hmi::PixelTool tool : tools) {
        const hmi::IconId id = hmi::editorActionForPixelTool(tool);
        EXPECT_TRUE(seen.insert(id).second) << "action dupliquee pour plusieurs outils";
        EXPECT_EQ(hmi::editorActionSpec(id).group, hmi::EditorActionGroup::PixelTools);
        EXPECT_FALSE(hmi::editorActionTool(id).has_value())
            << "un outil de canevas ne doit pas appartenir au groupe des outils de niveau";
        const std::optional<hmi::PixelTool> roundTrip = hmi::editorActionPixelTool(id);
        ASSERT_TRUE(roundTrip.has_value());
        EXPECT_EQ(*roundTrip, tool);
    }
    EXPECT_EQ(seen.size(), 5u);
}

/**
 * @brief Chaque commande n'a qu'une seule définition de raccourci : aucun raccourci non vide n'est
 *        attribué deux fois dans le catalogue.
 * \castest{<b>Aucun raccourci n'est attribue a deux actions.</b><br/>
 * \tcat Unitaire · Actions de l'editeur<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Parcourir le catalogue, ignorer les raccourcis vides.<br/>2. Verifier qu'aucune
 * valeur n'apparait deux fois.<br/>
 * \tattendu Chaque raccourci non vide est unique.
 * }
 */
TEST(EditorActionsTest, DefinitionUniqueDeRaccourci) {
    std::set<std::string> shortcuts;
    for (const hmi::EditorActionSpec& spec : hmi::editorActionCatalog()) {
        const std::string shortcut = spec.shortcut;
        if (shortcut.empty()) {
            continue;
        }
        EXPECT_TRUE(shortcuts.insert(shortcut).second)
            << "raccourci attribue deux fois : " << shortcut;
    }
}

/**
 * @brief La géométrie de chaque icône du catalogue est non vide et reste dans l'espace normalisé
 *        `[0,1] x [0,1]` : une icône hors de ce cadre déborderait de sa vignette à toute taille.
 * \castest{<b>La geometrie de chaque icone est non vide et dans le cadre normalise.</b><br/>
 * \tcat Unitaire · Actions de l'editeur<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Pour chaque action du catalogue, produire sa geometrie.<br/>2. Verifier qu'elle a au
 * moins un trait, et que chaque point est dans [0,1].<br/>
 * \tattendu Aucune icone n'est vide ni ne deborde du cadre.
 * }
 */
TEST(EditorActionsTest, GeometrieDesIconesNonVideEtDansLeCadre) {
    for (const hmi::EditorActionSpec& spec : hmi::editorActionCatalog()) {
        const hmi::IconGeometry geometry = hmi::iconGeometry(spec.id);
        EXPECT_FALSE(geometry.strokes.empty()) << "icone vide pour " << spec.labelKey;
        for (const hmi::IconStroke& stroke : geometry.strokes) {
            EXPECT_FALSE(stroke.points.empty());
            for (const hmi::IconPoint& point : stroke.points) {
                EXPECT_GE(point.x, 0.0f);
                EXPECT_LE(point.x, 1.0f);
                EXPECT_GE(point.y, 0.0f);
                EXPECT_LE(point.y, 1.0f);
            }
        }
    }
}

/**
 * @brief Chaque clé de libellé utilisée par le catalogue d'actions existe dans les deux
 *        catalogues de traduction livrés (`EX-REN-033`).
 * \castest{<b>Chaque cle de libelle du catalogue d'actions existe en francais et en
 * anglais.</b><br/> \tcat Unitaire · Actions de l'editeur<br/> \tcrit Majeur<br/> \tetapes 1. Lire
 * fr.lang et en.lang.<br/>2. Verifier que chaque labelKey du catalogue y figure.<br/> \tattendu
 * Aucune cle n'est absente de l'un ou l'autre catalogue.
 * }
 */
TEST(EditorActionsTest, ChaqueLibelleExisteDansLesDeuxLangues) {
    const std::unordered_map<std::string, std::string> fr = readCatalog(JADG_FR_LANG_PATH);
    const std::unordered_map<std::string, std::string> en = readCatalog(JADG_EN_LANG_PATH);
    ASSERT_FALSE(fr.empty());
    ASSERT_FALSE(en.empty());

    for (const hmi::EditorActionSpec& spec : hmi::editorActionCatalog()) {
        EXPECT_TRUE(fr.count(spec.labelKey) > 0) << "cle absente de fr.lang : " << spec.labelKey;
        EXPECT_TRUE(en.count(spec.labelKey) > 0) << "cle absente de en.lang : " << spec.labelKey;
    }
}

/**
 * @brief Garde-fou « aucune action orpheline » (`LOT-57` TACHE-04) : chaque action d'éditeur
 *        remappable, hors sélection d'outil, correspond à une commande effective du catalogue.
 *        Ce test casse si une action est ajoutée à `EditorKeyBindings` sans être branchée ici —
 *        exactement le défaut que cette tâche corrige (neuf actions définies, une seule lue).
 * \castest{<b>Chaque action d'editeur remappable a une commande effective.</b><br/>
 * \tcat Unitaire · Actions de l'editeur<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Comparer le nombre d'actions remappables (hors outil) au nombre d'entrees de la
 * table de correspondance.<br/>2. Verifier que chaque entree pointe vers une commande reelle du
 * catalogue et que l'aller-retour restitue l'action d'origine.<br/>
 * \tattendu Les deux comptes sont egaux ; chaque commande existe et l'aller-retour est fidele.
 * }
 */
TEST(EditorActionsTest, AucuneActionRemappableOrpheline) {
    // EDITOR_ACTION_COUNT - 1 : TextureAssignTool selectionne un outil, pas une commande (exclue
    // par construction de keyBindingIconCatalog). Une action ajoutee sans etre wiree ferait
    // diverger ce compte de la taille de la table.
    EXPECT_EQ(hmi::KEY_BINDING_ICON_COUNT, hmi::EDITOR_ACTION_COUNT - 1);

    std::set<hmi::IconId> seen;
    for (const hmi::KeyBindingIconEntry& entry : hmi::keyBindingIconCatalog()) {
        EXPECT_NO_THROW(static_cast<void>(hmi::editorActionSpec(entry.id)))
            << "action remappable sans commande reelle dans le catalogue";
        EXPECT_TRUE(seen.insert(entry.id).second) << "commande partagee par deux actions";
        const std::optional<hmi::EditorAction> roundTrip = hmi::keyBindingActionForIcon(entry.id);
        ASSERT_TRUE(roundTrip.has_value());
        EXPECT_EQ(*roundTrip, entry.action);
    }
}

/**
 * @brief La barre d'outils ne porte que la **sélection d'outil** et un petit nombre de commandes à
 *        usage continu (`LOT-68`, `EX-IHM-074`). Elle en portait onze, dont neuf figuraient déjà au
 *        menu : c'est cette accumulation, et non une duplication de définition, qui la rendait
 *        illisible.
 *
 * Le plafond `TOOLBAR_COMMAND_BUDGET` est vérifié **par groupe d'outils** : une barre commune aux
 * deux espaces ne dirait rien de ce que chacun affiche réellement.
 * \castest{<b>La barre d'outils ne porte que les outils et un petit nombre de commandes.</b><br/>
 * \tcat Unitaire · Actions de l'editeur<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Compter les actions de surface ToolBarAndMenu qui ne sont pas des outils, pour
 * l'espace de niveau puis pour l'atelier.<br/>
 * \tattendu Chaque compte reste sous TOOLBAR_COMMAND_BUDGET, et tous les outils des deux familles
 * restent en barre d'outils.
 * }
 */
TEST(EditorActionsTest, LaBarreDOutilsNePorteQueLEssentiel) {
    int levelCommands = 0;
    int pixelCommands = 0;
    int toolsInToolBar = 0;
    for (const hmi::EditorActionSpec& spec : hmi::editorActionCatalog()) {
        const bool isTool = spec.group == hmi::EditorActionGroup::LevelTools ||
                            spec.group == hmi::EditorActionGroup::PixelTools;
        if (spec.surface != hmi::ActionSurface::ToolBarAndMenu) {
            EXPECT_FALSE(isTool) << "un outil doit rester selectionnable a la barre d'outils";
            continue;
        }
        if (isTool) {
            ++toolsInToolBar;
        } else if (spec.group == hmi::EditorActionGroup::PixelCommands) {
            ++pixelCommands;
        } else {
            ++levelCommands;
        }
    }
    EXPECT_LE(levelCommands, hmi::TOOLBAR_COMMAND_BUDGET)
        << "la barre d'outils du niveau redevient un fourre-tout";
    EXPECT_LE(pixelCommands, hmi::TOOLBAR_COMMAND_BUDGET)
        << "la barre d'outils de l'atelier redevient un fourre-tout";
    EXPECT_EQ(toolsInToolBar, static_cast<int>(hmi::EDITOR_TOOL_COUNT) + 5)
        << "les huit outils de niveau et les cinq de canevas doivent tous y figurer";
}

/**
 * @brief Aucune action n'est **orpheline de surface** : chacune apparaît au moins au menu. Une
 *        action absente des deux surfaces n'aurait plus que son raccourci — donc, en pratique,
 *        n'existerait plus pour qui ne le connaît pas (`EX-EDIT-015`).
 * \castest{<b>Chaque action du catalogue reste atteignable par au moins une surface.</b><br/>
 * \tcat Unitaire · Actions de l'editeur<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Parcourir le catalogue et lire la surface de chaque action.<br/>
 * \tattendu Chaque action vaut ToolBarAndMenu ou MenuOnly ; les deux mènent au menu.
 * }
 */
TEST(EditorActionsTest, AucuneActionOrphelineDeSurface) {
    for (const hmi::EditorActionSpec& spec : hmi::editorActionCatalog()) {
        EXPECT_TRUE(spec.surface == hmi::ActionSurface::ToolBarAndMenu ||
                    spec.surface == hmi::ActionSurface::MenuOnly)
            << "action sans surface : " << spec.labelKey;
    }
}
