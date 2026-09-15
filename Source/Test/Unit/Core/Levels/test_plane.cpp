// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_plane.cpp
 * @brief Tests unitaires du plan pictural : modèle, format et brouillon annulable
 *        (LOT-69 TACHE-03, `EX-DEC-040` à `EX-DEC-044`, `EX-LVL-009`).
 */

#include <cmath>
#include <limits>
#include <string>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "Core/Levels/Level.h"
#include "Core/Levels/LevelDraft.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelWriter.h"
#include "Core/Levels/Plane.h"

namespace {

using core::LevelDraft;
using core::Plane;
using core::PlaneDepth;

/// Niveau minimal valide, sans plan : socle des tests de format.
constexpr const char* BARE_LEVEL = R"({
  "name": "P",
  "width": 4,
  "height": 3,
  "tiles": [
    { "x": 1, "y": 1, "type": "entry" },
    { "x": 3, "y": 2, "type": "exit" }
  ]
})";

/// Même niveau, avec deux plans dont un entièrement par défaut.
constexpr const char* LEVEL_WITH_PLANES = R"({
  "name": "P",
  "width": 4,
  "height": 3,
  "planes": [
    { "file": "ciel.png", "pixelsPerUnit": 4, "parallaxX": 0.25 },
    { "file": "mur.png" },
    { "file": "grille.png", "parallaxX": 1.2, "parallaxY": 1.0, "opacity": 0.5, "depth": "front" }
  ],
  "tiles": [
    { "x": 1, "y": 1, "type": "entry" },
    { "x": 3, "y": 2, "type": "exit" }
  ]
})";

/// Niveau portant l'ancien champ racine `decors` (LOT-49/LOT-50), retiré au LOT-69.
constexpr const char* LEVEL_WITH_OBSOLETE_DECORS = R"({
  "name": "P",
  "width": 4,
  "height": 3,
  "decors": [
    { "asset": "tree.png", "x": 1.5, "y": 2.0, "layer": "background" },
    { "asset": "rock.png", "x": 2.5, "y": 1.0 }
  ],
  "tiles": [
    { "x": 1, "y": 1, "type": "entry" },
    { "x": 3, "y": 2, "type": "exit" }
  ]
})";

/// Construit un brouillon portant @p count plans nommés `p0.png`, `p1.png`, …
[[nodiscard]] LevelDraft draftWithPlanes(std::size_t count) {
    LevelDraft draft = LevelDraft::empty("N", 8, 6);
    for (std::size_t index = 0; index < count; ++index) {
        Plane plane;
        plane.fileName = "p" + std::to_string(index) + ".png";
        draft.addPlane(std::move(plane));
    }
    return draft;
}

}  // namespace

/**
 * @brief Un plan par défaut est natif, solidaire du niveau, opaque et en arrière-plan.
 * \castest{<b>Un plan par défaut est natif, solidaire, opaque et derrière.</b><br/>
 * \tcat Unitaire · Plan<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un plan par défaut est natif, solidaire du niveau, opaque et en arrière-plan.
 * }
 */
TEST(PlaneTest, ValeursParDefaut) {
    const Plane plane;

    EXPECT_TRUE(plane.fileName.empty());
    EXPECT_EQ(plane.pixelsPerUnit, core::PLANE_NATIVE_PIXELS_PER_UNIT);
    EXPECT_FLOAT_EQ(plane.parallaxX, 1.0f);
    EXPECT_FLOAT_EQ(plane.parallaxY, 1.0f);
    EXPECT_FLOAT_EQ(plane.opacity, 1.0f);
    EXPECT_EQ(plane.depth, PlaneDepth::Behind);
}

/**
 * @brief Seules les densités 4, 8 et 16 sont acceptées.
 * \castest{<b>Seules les densités 4, 8 et 16 sont acceptées.</b><br/>
 * \tcat Unitaire · Plan<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Seules les densités 4, 8 et 16 sont acceptées.
 * }
 */
TEST(PlaneTest, DensitesValides) {
    EXPECT_TRUE(core::isValidPlaneDensity(4));
    EXPECT_TRUE(core::isValidPlaneDensity(8));
    EXPECT_TRUE(core::isValidPlaneDensity(16));

    EXPECT_FALSE(core::isValidPlaneDensity(0));
    EXPECT_FALSE(core::isValidPlaneDensity(12));
    EXPECT_FALSE(core::isValidPlaneDensity(32));
    EXPECT_FALSE(core::isValidPlaneDensity(-16));
}

/**
 * @brief addPlane ajoute en fin de liste ; l'ordre d'ajout est l'ordre de superposition.
 * \castest{<b>addPlane ajoute en fin de liste, préservant l'ordre.</b><br/>
 * \tcat Unitaire · Plan<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu addPlane ajoute en fin de liste ; l'ordre d'ajout est l'ordre de superposition.
 * }
 */
TEST(PlaneTest, AddPlaneAjouteEnFinDeListe) {
    const LevelDraft draft = draftWithPlanes(3);

    ASSERT_EQ(draft.planes().size(), 3u);
    EXPECT_EQ(draft.planes()[0].fileName, "p0.png");
    EXPECT_EQ(draft.planes()[2].fileName, "p2.png");
}

/**
 * @brief removePlane retire au rang donné et reste sans effet hors bornes.
 * \castest{<b>removePlane retire au rang donné, sans effet hors bornes.</b><br/>
 * \tcat Unitaire · Plan<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu removePlane retire au rang donné et reste sans effet hors bornes.
 * }
 */
TEST(PlaneTest, RemovePlaneRetireAuRangDonne) {
    LevelDraft draft = draftWithPlanes(3);

    draft.removePlane(1);

    ASSERT_EQ(draft.planes().size(), 2u);
    EXPECT_EQ(draft.planes()[0].fileName, "p0.png");
    EXPECT_EQ(draft.planes()[1].fileName, "p2.png");

    draft.removePlane(42);
    EXPECT_EQ(draft.planes().size(), 2u);
}

/**
 * @brief Chaque mutateur de plan est annulable **et** refaisable en un seul pas.
 * \castest{<b>Chaque mutateur de plan est annulable et refaisable en un pas.</b><br/>
 * \tcat Unitaire · Plan<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Chaque mutateur de plan est annulable et refaisable en un seul pas.
 * }
 */
TEST(PlaneTest, ChaqueMutateurEstAnnulableEtRefaisable) {
    LevelDraft draft = draftWithPlanes(2);

    // REGLE VERIFIEE ICI : tout mutateur de plan empile un pushUndo(). Le vecteur ci-dessous les
    // enumere EXHAUSTIVEMENT -- ajouter un mutateur sans l'ajouter ici laisserait une mutation
    // non annulable, invisible autrement qu'a l'usage.
    draft.setPlaneDensity(0, 8);
    EXPECT_EQ(draft.planes()[0].pixelsPerUnit, 8);
    ASSERT_TRUE(draft.canUndo());
    draft.undo();
    EXPECT_EQ(draft.planes()[0].pixelsPerUnit, 16);
    draft.redo();
    EXPECT_EQ(draft.planes()[0].pixelsPerUnit, 8);

    draft.setPlaneParallax(0, 0.5f, 0.75f);
    draft.undo();
    EXPECT_FLOAT_EQ(draft.planes()[0].parallaxX, 1.0f);
    draft.redo();
    EXPECT_FLOAT_EQ(draft.planes()[0].parallaxY, 0.75f);

    draft.setPlaneOpacity(1, 0.25f);
    draft.undo();
    EXPECT_FLOAT_EQ(draft.planes()[1].opacity, 1.0f);
    draft.redo();
    EXPECT_FLOAT_EQ(draft.planes()[1].opacity, 0.25f);

    draft.setPlaneDepth(1, PlaneDepth::Front);
    draft.undo();
    EXPECT_EQ(draft.planes()[1].depth, PlaneDepth::Behind);
    draft.redo();
    EXPECT_EQ(draft.planes()[1].depth, PlaneDepth::Front);

    draft.setParallaxEnabled(false);
    EXPECT_FALSE(draft.parallaxEnabled());
    draft.undo();
    EXPECT_TRUE(draft.parallaxEnabled());

    draft.removePlane(0);
    EXPECT_EQ(draft.planes().size(), 1u);
    draft.undo();
    EXPECT_EQ(draft.planes().size(), 2u);
}

/**
 * @brief Un mutateur refusé ne modifie rien et n'empile aucun pas d'annulation.
 * \castest{<b>Un mutateur refusé n'empile aucun pas d'annulation.</b><br/>
 * \tcat Unitaire · Plan<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un mutateur refusé ne modifie rien et n'empile aucun pas d'annulation.
 * }
 */
TEST(PlaneTest, MutateurRefuseNEmpileRien) {
    LevelDraft draft = draftWithPlanes(1);
    while (draft.canUndo()) {
        draft.undo();  // repartir d'un historique vide apres les addPlane
    }
    while (draft.canRedo()) {
        draft.redo();
    }
    const bool hadUndo = draft.canUndo();

    EXPECT_FALSE(draft.setPlaneDensity(0, 12));    // densite invalide
    EXPECT_FALSE(draft.setPlaneDensity(9, 8));     // rang hors bornes
    EXPECT_FALSE(draft.setPlaneOpacity(0, 1.5f));  // hors de [0,1]
    EXPECT_FALSE(draft.setPlaneOpacity(0, -0.1f));
    EXPECT_FALSE(draft.setPlaneOpacity(0, std::numeric_limits<float>::quiet_NaN()));
    EXPECT_FALSE(draft.setPlaneParallax(0, std::numeric_limits<float>::infinity(), 1.0f));
    EXPECT_FALSE(draft.setPlaneDepth(9, PlaneDepth::Front));
    EXPECT_FALSE(draft.movePlaneForward(9).has_value());
    EXPECT_FALSE(draft.movePlaneToBack(9).has_value());

    EXPECT_EQ(draft.canUndo(), hadUndo) << "un refus ne doit jamais consommer un pas d'historique";
    EXPECT_EQ(draft.planes()[0].pixelsPerUnit, 16);
    EXPECT_FLOAT_EQ(draft.planes()[0].opacity, 1.0f);
}

/**
 * @brief Le réordonnancement déplace le plan et renvoie son nouveau rang.
 * \castest{<b>Le réordonnancement déplace le plan et renvoie son nouveau rang.</b><br/>
 * \tcat Unitaire · Plan<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le réordonnancement déplace le plan et renvoie son nouveau rang.
 * }
 */
TEST(PlaneTest, ReordonnancementDesPlans) {
    LevelDraft draft = draftWithPlanes(4);  // p0 p1 p2 p3

    EXPECT_EQ(draft.movePlaneForward(0), 1u);
    EXPECT_EQ(draft.planes()[1].fileName, "p0.png");  // p1 p0 p2 p3

    EXPECT_EQ(draft.movePlaneBackward(1), 0u);
    EXPECT_EQ(draft.planes()[0].fileName, "p0.png");  // p0 p1 p2 p3

    EXPECT_EQ(draft.movePlaneToFront(0), 3u);
    EXPECT_EQ(draft.planes()[3].fileName, "p0.png");  // p1 p2 p3 p0

    EXPECT_EQ(draft.movePlaneToBack(3), 0u);
    EXPECT_EQ(draft.planes()[0].fileName, "p0.png");  // p0 p1 p2 p3

    // Aux extremites : succes sans effet, et l'ordre reste intact.
    EXPECT_EQ(draft.movePlaneBackward(0), 0u);
    EXPECT_EQ(draft.movePlaneForward(3), 3u);
    EXPECT_EQ(draft.planes()[3].fileName, "p3.png");
}

/**
 * @brief Les plans sont lus dans l'ordre déclaré, avec leurs valeurs par défaut.
 * \castest{<b>Les plans sont lus dans l'ordre déclaré, avec leurs défauts.</b><br/>
 * \tcat Unitaire · Plan<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les plans sont lus dans l'ordre déclaré, avec leurs valeurs par défaut.
 * }
 */
TEST(PlaneTest, ChargementDesPlans) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL_WITH_PLANES);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::vector<Plane>& planes = loaded.level->planes();
    ASSERT_EQ(planes.size(), 3u);

    EXPECT_EQ(planes[0].fileName, "ciel.png");
    EXPECT_EQ(planes[0].pixelsPerUnit, 4);
    EXPECT_FLOAT_EQ(planes[0].parallaxX, 0.25f);
    // parallaxY absent : retombe sur parallaxX, pas sur 1.0.
    EXPECT_FLOAT_EQ(planes[0].parallaxY, 0.25f);
    EXPECT_EQ(planes[0].depth, PlaneDepth::Behind);

    EXPECT_EQ(planes[1].fileName, "mur.png");
    EXPECT_EQ(planes[1].pixelsPerUnit, 16);
    EXPECT_FLOAT_EQ(planes[1].parallaxX, 1.0f);

    EXPECT_EQ(planes[2].fileName, "grille.png");
    EXPECT_FLOAT_EQ(planes[2].parallaxY, 1.0f);
    EXPECT_FLOAT_EQ(planes[2].opacity, 0.5f);
    EXPECT_EQ(planes[2].depth, PlaneDepth::Front);

    // Drapeau absent : la parallaxe est active par defaut.
    EXPECT_TRUE(loaded.level->parallaxEnabled());
}

/**
 * @brief Le champ obsolète `decors` est **ignoré**, jamais rejeté : un niveau personnel écrit
 * avant le `LOT-69` reste lisible (`EX-LVL-005`), et un simple charger-puis-enregistrer le migre.
 * \castest{<b>Le champ obsolete decors est ignore, et disparait a la reecriture.</b><br/>
 * \tcat Unitaire · Plan<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Charger un niveau portant encore le champ racine decors.<br/>2. Reecrire le niveau
 * charge.<br/>
 * \tattendu Le chargement reussit, planes() est vide, et le JSON reecrit ne contient plus decors.
 * }
 */
TEST(PlaneTest, ChampObsoleteDecorsIgnoreEtNonReecrit) {
    const core::LevelLoadResult loaded =
        core::LevelLoader::loadFromString(LEVEL_WITH_OBSOLETE_DECORS);

    // Un champ obsolete n'est pas une donnee INVALIDE (EX-LVL-004 vise la validite) : rejeter
    // rendrait illisible tout niveau personnel anterieur, a rebours de EX-LVL-005.
    ASSERT_TRUE(loaded.ok()) << loaded.error;
    EXPECT_TRUE(loaded.level->planes().empty());

    // La reecriture migre le fichier : le champ disparait, sans que l'auteur ait rien a faire.
    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    EXPECT_EQ(json.find("\"decors\""), std::string::npos);
}

/**
 * @brief Un niveau sans plan reste valide et n'écrit pas le champ "planes".
 * \castest{<b>Un niveau sans plan n'écrit pas le champ planes.</b><br/>
 * \tcat Unitaire · Plan<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un niveau sans plan reste valide et n'écrit pas le champ "planes".
 * }
 */
TEST(PlaneTest, NiveauSansPlanNEcritPasLeChamp) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(BARE_LEVEL);
    ASSERT_TRUE(loaded.ok()) << loaded.error;
    EXPECT_TRUE(loaded.level->planes().empty());

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    EXPECT_EQ(json.find("\"planes\""), std::string::npos);
    // Le drapeau vaut true par defaut : il ne doit pas etre ecrit non plus.
    EXPECT_EQ(json.find("\"parallax\""), std::string::npos);
}

/**
 * @brief Le round-trip préserve les plans et omet tout champ à sa valeur par défaut.
 * \castest{<b>Le round-trip préserve les plans et omet les défauts.</b><br/>
 * \tcat Unitaire · Plan<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le round-trip préserve les plans et omet tout champ à sa valeur par défaut.
 * }
 */
TEST(PlaneTest, RoundTripDesPlans) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL_WITH_PLANES);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    ASSERT_EQ(reloaded.level->planes().size(), 3u);
    for (std::size_t index = 0; index < 3; ++index) {
        const Plane& before = loaded.level->planes()[index];
        const Plane& after = reloaded.level->planes()[index];
        EXPECT_EQ(after.fileName, before.fileName);
        EXPECT_EQ(after.pixelsPerUnit, before.pixelsPerUnit);
        EXPECT_FLOAT_EQ(after.parallaxX, before.parallaxX);
        EXPECT_FLOAT_EQ(after.parallaxY, before.parallaxY);
        EXPECT_FLOAT_EQ(after.opacity, before.opacity);
        EXPECT_EQ(after.depth, before.depth);
    }

    // "mur.png" est entierement par defaut : il ne produit que son nom de fichier. Verifie sur le
    // JSON reanalyse plutot que sur sa mise en forme, qui n'est pas ce que le test garantit.
    const nlohmann::json parsed = nlohmann::json::parse(json);
    const nlohmann::json* defaultPlane = nullptr;
    for (const nlohmann::json& plane : parsed.at("planes")) {
        if (plane.at("file").get<std::string>() == "mur.png") {
            defaultPlane = &plane;
        }
    }
    ASSERT_NE(defaultPlane, nullptr) << json;
    EXPECT_EQ(defaultPlane->size(), 1u)
        << "un plan par defaut ne doit ecrire que son champ 'file' -- JSON : " << json;
}

/**
 * @brief Le drapeau de parallaxe n'est écrit que lorsqu'il est désactivé.
 * \castest{<b>Le drapeau de parallaxe n'est écrit que désactivé.</b><br/>
 * \tcat Unitaire · Plan<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le drapeau de parallaxe n'est écrit que lorsqu'il est désactivé.
 * }
 */
TEST(PlaneTest, DrapeauDeParallaxeDesactive) {
    const std::string source = R"({
      "name": "P", "width": 4, "height": 3, "parallax": false,
      "planes": [ { "file": "ciel.png" } ],
      "tiles": [ { "x": 1, "y": 1, "type": "entry" }, { "x": 3, "y": 2, "type": "exit" } ]
    })";

    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(source);
    ASSERT_TRUE(loaded.ok()) << loaded.error;
    EXPECT_FALSE(loaded.level->parallaxEnabled());

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    EXPECT_FALSE(nlohmann::json::parse(json).at("parallax").get<bool>()) << json;

    // Le drapeau survit au round-trip : c'est ce qui compte pour le level designer.
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;
    EXPECT_FALSE(reloaded.level->parallaxEnabled());
}

/**
 * @brief Une densité de plan invalide fait échouer le chargement.
 * \castest{<b>Une densité de plan invalide fait échouer le chargement.</b><br/>
 * \tcat Unitaire · Plan<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une densité de plan invalide fait échouer le chargement.
 * }
 */
TEST(PlaneTest, DensiteInvalideRejetee) {
    const std::string json = R"({
      "name": "P", "width": 4, "height": 3,
      "planes": [ { "file": "x.png", "pixelsPerUnit": 12 } ],
      "tiles": [ { "x": 1, "y": 1, "type": "entry" }, { "x": 3, "y": 2, "type": "exit" } ]
    })";

    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(json);

    EXPECT_FALSE(loaded.ok());
    EXPECT_EQ(loaded.errorCode, core::LevelValidationError::ParseError);
}

/**
 * @brief Le nombre de plans est plafonné au chargement (garde-fou de coût).
 * \castest{<b>Le nombre de plans est plafonné au chargement.</b><br/>
 * \tcat Unitaire · Plan<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le nombre de plans est plafonné au chargement (garde-fou de coût).
 * }
 */
TEST(PlaneTest, TropDePlansRejete) {
    std::string planes;
    for (std::size_t index = 0; index <= core::MAX_PLANES_PER_LEVEL; ++index) {
        planes += (index == 0 ? "" : ",");
        planes += R"({"file":"p)" + std::to_string(index) + R"(.png"})";
    }
    const std::string json = R"({"name":"P","width":4,"height":3,"planes":[)" + planes +
                             R"(],"tiles":[{"x":1,"y":1,"type":"entry"},)"
                             R"({"x":3,"y":2,"type":"exit"}]})";

    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(json);

    EXPECT_FALSE(loaded.ok());
    EXPECT_EQ(loaded.errorCode, core::LevelValidationError::ParseError);
}

/**
 * @brief Un plan dont la texture dépasserait la limite matérielle est refusé.
 * \castest{<b>Un plan dépassant la limite de texture est refusé.</b><br/>
 * \tcat Unitaire · Plan<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un plan dont la texture dépasserait la limite matérielle est refusé.
 * }
 */
TEST(PlaneTest, PlanTropGrandRejete) {
    // 2000 cases x 16 px/unite = 32000 px, au-dela de MAX_PLANE_TEXTURE_EXTENT (16384).
    // C'est la COMBINAISON taille x densite qui est refusee : la densite seule est valide.
    const std::string json = R"({
      "name": "P", "width": 2000, "height": 3,
      "planes": [ { "file": "x.png", "pixelsPerUnit": 16 } ],
      "tiles": [ { "x": 1, "y": 1, "type": "entry" }, { "x": 3, "y": 2, "type": "exit" } ]
    })";

    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(json);

    EXPECT_FALSE(loaded.ok());
    EXPECT_EQ(loaded.errorCode, core::LevelValidationError::ParseError);

    // La meme largeur a 4 px/unite passe : 8000 px reste sous la limite.
    const std::string smaller = R"({
      "name": "P", "width": 2000, "height": 3,
      "planes": [ { "file": "x.png", "pixelsPerUnit": 4 } ],
      "tiles": [ { "x": 1, "y": 1, "type": "entry" }, { "x": 3, "y": 2, "type": "exit" } ]
    })";
    EXPECT_TRUE(core::LevelLoader::loadFromString(smaller).ok());
}
