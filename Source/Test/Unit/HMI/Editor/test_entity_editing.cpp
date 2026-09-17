// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_entity_editing.cpp
 * @brief Tests unitaires de la logique d'édition des couches et des entités de l'éditeur
 *        (`LOT-11`) : lignes du panneau « Couches », réglages d'affichage, geste de l'outil
 *        « Entité », références proposées et avertissements.
 */

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <limits>
#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Combat/Arena.h"
#include "Core/Combat/TacticalTerrain.h"
#include "Core/Levels/LevelDraft.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/MapEntity.h"
#include "Core/Levels/TileLayer.h"
#include "Core/World/EntityKinds.h"
#include "HMI/Editor/EditorDiagnostics.h"
#include "HMI/Editor/EntityGesture.h"
#include "HMI/Editor/EntityReferences.h"
#include "HMI/Editor/LayerView.h"
#include "HMI/Localization/Localization.h"

namespace {

constexpr const char* MAP = R"({
  "version": 3,
  "name": "Village",
  "width": 5,
  "height": 4,
  "tiles": [
    { "x": 0, "y": 0, "type": "entry" },
    { "x": 4, "y": 3, "type": "exit" }
  ],
  "layers": [
    { "name": "sol", "kind": "ground", "tiles": [] },
    { "name": "arbres", "kind": "decor", "tiles": [] }
  ],
  "entities": [
    { "type": "chest", "x": 2, "y": 1 },
    { "type": "sign", "x": 2, "y": 1 }
  ]
})";

[[nodiscard]] core::LevelDraft draft() {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(MAP);
    EXPECT_TRUE(loaded.ok()) << loaded.error;
    return core::LevelDraft::fromLevel(*loaded.level);
}

[[nodiscard]] core::TileLayer layer(std::string name, core::LayerKind kind) {
    return core::TileLayer{
        .name = std::move(name), .kind = kind, .tiles = core::TileMap(2, 2), .properties = {}};
}

// Racine des elements livres : le dossier parent des niveaux.
[[nodiscard]] std::filesystem::path elementsRoot() {
    return std::filesystem::path{JADG_LEVELS_DIR}.parent_path();
}

}  // namespace

/**
 * @brief La grille racine a une ligne, en tête, dont le rôle dit si elle est l'image ou la
 * collision ; les couches visuelles suivent dans l'ordre de dessin.
 * \castest{<b>Les lignes du panneau Couches suivent l'ordre de dessin.</b><br/>
 * \tcat Unitaire · Edition de couches<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire les lignes d'une carte sans couche visuelle.<br/>2. Puis d'une carte a
 * collision, sol et decor.<br/>
 * \tattendu Une ligne Legacy seule ; puis Collision, sol (rang 1), decor (rang 2).
 * }
 */
TEST(EditionEntitesTest, LignesDuPanneauCouchesSuiventLOrdreDeDessin) {
    EXPECT_EQ(hmi::layerRows({layer({}, core::LayerKind::Legacy)}),
              (std::vector<hmi::LayerRow>{
                  {.slot = std::nullopt, .kind = core::LayerKind::Legacy, .name = {}}}));

    const std::vector<core::TileLayer> layers = {layer({}, core::LayerKind::Collision),
                                                 layer("sol", core::LayerKind::Ground),
                                                 layer("arbres", core::LayerKind::Decor)};
    EXPECT_EQ(hmi::layerRows(layers),
              (std::vector<hmi::LayerRow>{
                  {.slot = std::nullopt, .kind = core::LayerKind::Collision, .name = {}},
                  {.slot = 1, .kind = core::LayerKind::Ground, .name = "sol"},
                  {.slot = 2, .kind = core::LayerKind::Decor, .name = "arbres"}}));
}

/**
 * @brief Une couche active qui n'existe plus, ou n'est plus visuelle, retombe sur la grille racine.
 * \castest{<b>La couche active invalide retombe sur la collision.</b><br/>
 * \tcat Unitaire · Edition de couches<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Valider le rang 2, puis 5, puis 0 (collision) sur trois couches.<br/>
 * \tattendu 2 est garde ; 5 et 0 retombent sur la grille racine.
 * }
 */
TEST(EditionEntitesTest, CoucheActiveInvalideRetombeSurLaRacine) {
    const std::vector<core::TileLayer> layers = {layer({}, core::LayerKind::Collision),
                                                 layer("sol", core::LayerKind::Ground),
                                                 layer("arbres", core::LayerKind::Decor)};
    EXPECT_EQ(hmi::validActiveLayer(layers, 2), hmi::LayerSlot{2});
    EXPECT_EQ(hmi::validActiveLayer(layers, 5), hmi::LayerSlot{});
    EXPECT_EQ(hmi::validActiveLayer(layers, 0), hmi::LayerSlot{});
}

/**
 * @brief La collision se montre à demi par-dessus des couches visuelles, pleine sur une carte à
 * grille unique ; les réglages se bornent et suivent un déplacement.
 * \castest{<b>Les reglages d'affichage des couches.</b><br/>
 * \tcat Unitaire · Edition de couches<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Lire l'opacite de la racine avec et sans couche visuelle.<br/>2. Regler une opacite
 * hors bornes et une non finie.<br/>3. Masquer le rang 1 puis l'echanger avec le rang 2.<br/>
 * \tattendu 0,55 puis 1 ; opacite bornee a 1, la non finie ignoree ; le masquage suit la couche.
 * }
 */
TEST(EditionEntitesTest, ReglagesDAffichageDesCouches) {
    hmi::LayerViewState view;
    view.sync(3);
    EXPECT_FLOAT_EQ(view.display(std::nullopt, true).opacity,
                    hmi::DEFAULT_COLLISION_OVERLAY_OPACITY);
    EXPECT_FLOAT_EQ(view.display(std::nullopt, false).opacity, 1.0F);

    view.setOpacity(1, 3.0F);
    EXPECT_FLOAT_EQ(view.display(1, true).opacity, 1.0F);
    view.setOpacity(1, 0.25F);
    view.setOpacity(1, std::numeric_limits<float>::quiet_NaN());
    EXPECT_FLOAT_EQ(view.display(1, true).opacity, 0.25F);

    view.setVisible(1, false);
    view.swap(1, 2);
    EXPECT_TRUE(view.display(1, true).visible);
    EXPECT_FALSE(view.display(2, true).visible);
    EXPECT_FLOAT_EQ(view.display(2, true).opacity, 0.25F);

    view.reset();
    EXPECT_EQ(view.display(2, true), hmi::LayerDisplay{});
}

/**
 * @brief L'appui de l'outil Entité : une case occupée sélectionne l'entité du dessus, une case
 * libre pose, Ctrl pose sur une case occupée, et sans famille une case libre désélectionne.
 * \castest{<b>Le geste de l'outil Entite.</b><br/>
 * \tcat Unitaire · Edition d'entites<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Appuyer sur la case du coffre et du panneau, avec et sans Ctrl.<br/>2. Appuyer sur
 * une case libre, avec et sans famille choisie.<br/>3. Appuyer hors de la grille.<br/>
 * \tattendu Selection du panneau (rang 1) ; pose avec Ctrl ; pose ; deselection ; rien.
 * }
 */
TEST(EditionEntitesTest, GesteDeLOutilEntite) {
    const core::LevelDraft map = draft();
    const core::GridPosition shared{.column = 2, .row = 1};
    const core::GridPosition libre{.column = 0, .row = 3};

    EXPECT_EQ(hmi::resolveEntityPress(map, shared, "npc", false),
              (hmi::EntityGestureDecision{
                  .action = hmi::EntityGestureAction::Select, .entityIndex = 1, .cell = shared}));
    EXPECT_EQ(hmi::resolveEntityPress(map, shared, "npc", true).action,
              hmi::EntityGestureAction::Place);
    EXPECT_EQ(hmi::resolveEntityPress(map, libre, "npc", false),
              (hmi::EntityGestureDecision{
                  .action = hmi::EntityGestureAction::Place, .entityIndex = 0, .cell = libre}));
    EXPECT_EQ(hmi::resolveEntityPress(map, libre, "", false).action,
              hmi::EntityGestureAction::Deselect);
    EXPECT_EQ(hmi::resolveEntityPress(map, {.column = 9, .row = 9}, "npc", false).action,
              hmi::EntityGestureAction::Ignore);
}

/**
 * @brief Relâcher un glisser déplace l'entité saisie seulement si la case a changé.
 * \castest{<b>Un glisser deplace l'entite saisie.</b><br/>
 * \tcat Unitaire · Edition d'entites<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Relacher sur une autre case, sur la meme, sans entite saisie.<br/>
 * \tattendu Deplacement ; rien ; rien.
 * }
 */
TEST(EditionEntitesTest, GlisserDeplaceLEntiteSaisie) {
    const core::GridPosition from{.column = 2, .row = 1};
    const core::GridPosition to{.column = 3, .row = 1};
    EXPECT_EQ(hmi::resolveEntityRelease(0, from, to),
              (hmi::EntityGestureDecision{
                  .action = hmi::EntityGestureAction::Move, .entityIndex = 0, .cell = to}));
    EXPECT_EQ(hmi::resolveEntityRelease(0, from, from).action, hmi::EntityGestureAction::Ignore);
    EXPECT_EQ(hmi::resolveEntityRelease(std::nullopt, from, to).action,
              hmi::EntityGestureAction::Ignore);
}

/**
 * @brief Les choix proposés : liste fixe, catalogues, cartes, et points d'arrivée de la carte
 * nommée par l'entité — ceux du brouillon pour la carte éditée.
 * \castest{<b>Les choix proposes par le panneau Entites.</b><br/>
 * \tcat Unitaire · Edition d'entites<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un contexte : une carte « foret » au point « lisiere », la carte editee
 * « village » dont le brouillon pose le point « puits ».<br/>2. Demander les choix du camp d'une
 * entree d'arene, des cartes, et des points d'arrivee vers le village puis sans cible.<br/>
 * \tattendu allies/enemies ; foret et village ; puits ; aucun.
 * }
 */
TEST(EditionEntitesTest, ChoixProposesParLePanneau) {
    hmi::EditorReferences references;
    references.world = core::buildWorldGraph(
        {core::WorldMapInput{
             .mapId = "foret",
             .name = "Forêt",
             .entities = {core::MapEntity{.type = "spawnPoint",
                                          .position = {},
                                          .properties = {{"name", std::string{"lisiere"}}}}},
             .loadError = {}},
         core::WorldMapInput{
             .mapId = "village", .name = "Village", .entities = {}, .loadError = {}}});
    const std::vector<core::MapEntity> edited = {core::MapEntity{
        .type = "spawnPoint", .position = {}, .properties = {{"name", std::string{"puits"}}}}};
    const core::EntityReferenceContext context =
        hmi::referenceContext(references, "village", edited);

    const core::EntityKind* const arena = core::findEntityKind(core::ARENA_ENTRY_ENTITY_TYPE);
    const core::EntityKind* const portal = core::findEntityKind(core::PORTAL_ENTITY_TYPE);
    ASSERT_NE(arena, nullptr);
    ASSERT_NE(portal, nullptr);
    core::MapEntity gate = core::makeEntity(*portal, {});

    EXPECT_EQ(hmi::entityChoices(*arena->find("side"), gate, context),
              (std::vector<std::string>{"allies", "enemies"}));
    EXPECT_EQ(hmi::entityChoices(*portal->find("targetMap"), gate, context),
              (std::vector<std::string>{"foret", "village"}));
    EXPECT_TRUE(hmi::entityChoices(*portal->find("arrival"), gate, context).empty());
    gate.properties["targetMap"] = std::string{"village"};
    EXPECT_EQ(hmi::entityChoices(*portal->find("arrival"), gate, context),
              (std::vector<std::string>{"puits"}));
}

/**
 * @brief Les catalogues livrés se lisent, et le Colisée livré ne lève aucun avertissement.
 * \castest{<b>Les catalogues livres alimentent l'editeur.</b><br/>
 * \tcat Unitaire · Edition d'entites<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Lire les references sous Source/Elements.<br/>2. Valider les entites de l'arene du
 * futur.<br/>
 * \tattendu Le dialogue du heraut, la rencontre du Colisee et l'arene sont connus ; aucune entite de
 * l'arene n'est signalee.
 * }
 */
TEST(EditionEntitesTest, CataloguesLivresAlimententLEditeur) {
    const hmi::EditorReferences references = hmi::loadEditorReferences(elementsRoot());
    EXPECT_NE(std::ranges::find(references.dialogues, "heraut-colisee"),
              references.dialogues.end());
    EXPECT_NE(references.encounters.find("colisee-fauves"), nullptr);
    ASSERT_NE(references.world.find("coliseum"), nullptr);

    const core::LevelLoadResult arena =
        core::LevelLoader::loadFromFile(elementsRoot() / "Levels" / "coliseum.json");
    ASSERT_TRUE(arena.ok()) << arena.error;
    const core::EntityReferenceContext context =
        hmi::referenceContext(references, "coliseum", arena.level->entities());
    EXPECT_TRUE(core::validateMapEntities(arena.level->entities(), context).empty());
}

/**
 * @brief Les avertissements nomment l'entité, la propriété et la valeur, puis le terrain.
 * \castest{<b>Les avertissements de l'editeur.</b><br/>
 * \tcat Unitaire · Edition d'entites<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Traduire un dialogue inconnu, un combattant dans un mur et une zone trop
 * etroite.<br/>
 * \tattendu Trois lignes, references d'abord, avec leurs cles et arguments.
 * }
 */
TEST(EditionEntitesTest, AvertissementsDeLEditeur) {
    const std::vector<core::MapEntity> entities = {
        core::MapEntity{.type = "npc", .position = {.column = 1, .row = 1}, .properties = {}},
        core::MapEntity{
            .type = "encounter", .position = {.column = 3, .row = 2}, .properties = {}}};
    const std::vector<core::EntityIssue> issues = {
        core::EntityIssue{.entityIndex = 0,
                          .code = core::EntityIssueCode::UnknownDialogue,
                          .key = "dialogue",
                          .value = "absent"}};
    core::EncounterTerrain terrain;
    terrain.entityIndex = 1;
    terrain.encounterId = "colisee-fauves";
    terrain.area.resize(5);
    terrain.requiredCells = 24;
    terrain.issues = {core::TacticalIssue{.code = core::TacticalIssueCode::CombatantObstructed,
                                          .creatureId = "rat",
                                          .cell = {.column = 4, .row = 2}},
                      core::TacticalIssue{.code = core::TacticalIssueCode::AreaTooNarrow,
                                          .creatureId = {},
                                          .cell = {.column = 3, .row = 2}}};

    const std::vector<hmi::EditorDiagnostic> lines =
        hmi::editorDiagnostics(entities, issues, {terrain});
    ASSERT_EQ(lines.size(), 3U);
    EXPECT_EQ(lines[0], (hmi::EditorDiagnostic{.kind = hmi::EditorDiagnosticKind::Reference,
                                               .entityIndex = 0,
                                               .cell = {.column = 1, .row = 1},
                                               .key = "diagnostic.unknown_dialogue",
                                               .args = {"npc", "dialogue", "absent"}}));
    EXPECT_EQ(lines[1].key, "diagnostic.terrain_obstructed");
    EXPECT_EQ(lines[1].cell, (core::GridPosition{.column = 4, .row = 2}));
    EXPECT_EQ(lines[1].args, (std::vector<std::string>{"colisee-fauves", "rat"}));
    EXPECT_EQ(lines[2].args, (std::vector<std::string>{"colisee-fauves", "5", "24"}));
}

/**
 * @brief Chaque clé de l'édition de couches et d'entités existe dans les deux catalogues.
 * \castest{<b>Les cles de l'edition de couches et d'entites sont traduites.</b><br/>
 * \tcat Unitaire · Edition d'entites<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger fr.lang puis en.lang.<br/>2. Resoudre chaque cle d'avertissement, chaque
 * famille et chaque propriete de la table.<br/>
 * \tattendu Chaque cle resout vers un texte distinct de la cle.
 * }
 */
TEST(EditionEntitesTest, ClesTraduitesDansLesDeuxCatalogues) {
    std::vector<std::string> keys;
    for (int code = 0; code <= static_cast<int>(core::EntityIssueCode::DuplicateArrivalPoint);
         ++code) {
        keys.emplace_back(hmi::entityIssueKey(static_cast<core::EntityIssueCode>(code)));
    }
    for (int code = 0; code <= static_cast<int>(core::TacticalIssueCode::AreaTooNarrow); ++code) {
        keys.emplace_back(hmi::tacticalIssueKey(static_cast<core::TacticalIssueCode>(code)));
    }
    for (const core::EntityKind& kind : core::knownEntityKinds()) {
        keys.push_back("entities.kind." + std::string{kind.type});
        for (const core::EntityPropertySpec& spec : kind.properties) {
            keys.push_back("entities.property." + std::string{spec.key});
        }
    }
    for (const char* const key :
         {"dock.layers", "dock.entities", "tool.entity", "status.help_entity", "layers.root_legacy",
          "layers.root_collision", "layers.kind.ground", "layers.kind.decor"}) {
        keys.emplace_back(key);
    }
    for (const std::string& language : {"fr", "en"}) {
        hmi::Localization localization(std::filesystem::path{JADG_LOCALIZATION_DIR});
        ASSERT_TRUE(localization.loadDefaultLanguage(language)) << language;
        for (const std::string& key : keys) {
            EXPECT_NE(localization.text(key), key) << language << " / " << key;
        }
    }
}
