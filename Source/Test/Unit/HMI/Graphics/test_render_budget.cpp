// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_render_budget.cpp
 * @brief Test de non-régression du volume de primitives et de l'efficacité du culling (`LOT-62`
 *        TACHE-01, `EX-NFR-005`, `EX-NFR-004`).
 */

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Sprite.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/World.h"
#include "Core/Levels/CameraFraming.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelScene.h"
#include "Core/Levels/Plane.h"
#include "Core/Levels/TileMap.h"
#include "Core/Physics/PlayerSpawn.h"
#include "HMI/Editor/PlaneFileNaming.h"
#include "HMI/Graphics/BackgroundRenderer.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/CameraZones.h"
#include "HMI/Graphics/ComposedScene.h"
#include "HMI/Graphics/ParticleRenderer.h"
#include "HMI/Graphics/PlaneVisuals.h"
#include "HMI/Graphics/PlayerSpriteTag.h"
#include "HMI/Graphics/QuadRecorder.h"
#include "HMI/Graphics/RenderLayer.h"
#include "HMI/Graphics/RenderMode.h"
#include "HMI/Graphics/RoomGrid.h"
#include "HMI/Graphics/ShadowRenderer.h"
#include "HMI/Graphics/TileAutotile.h"
#include "HMI/Graphics/TileSkinTag.h"
#include "HMI/Graphics/TileVisuals.h"

namespace {

// Textures factices : la composition ne fait que comparer des identites (cf. test_quad_recorder).
// Aucun skin n'est charge : tout retombe sur l'atlas (Physique) ou le damier (Texture,
// EX-NFR-040) -- comportement normal du programme d'habillage, sans consequence sur le VOLUME de
// primitives, seule chose que ce test mesure.
int atlasStorage = 0;
int missingStorage = 0;
int backgroundStorage = 0;
hmi::TextureHandle atlasTexture = &atlasStorage;
hmi::TextureHandle missingTexture = &missingStorage;
hmi::TextureHandle backgroundTexture = &backgroundStorage;

/// Textures de reference des tests : atlas 5x5 cases de 16px (meme geometrie que
/// hmi::TextureAtlas, cf. test_shadow_render.cpp) et damier de repli.
hmi::SceneTextures testTextures() {
    hmi::SceneTextures textures;
    textures.atlas = atlasTexture;
    textures.atlasWidth = 80;
    textures.atlasHeight = 80;
    textures.missing = missingTexture;
    textures.missingWidth = 16;
    textures.missingHeight = 16;
    return textures;
}

/// Charge un niveau livre depuis son fichier JSON (JADG_LEVELS_DIR).
core::Level loadDeliveredLevel(const std::string& fileName) {
    const std::filesystem::path path = std::filesystem::path(JADG_LEVELS_DIR) / fileName;
    const core::LevelLoadResult result = core::LevelLoader::loadFromFile(path);
    if (!result.ok()) {
        ADD_FAILURE() << "Echec de chargement de " << fileName << " : " << result.error;
        return core::Level{core::LevelData{.name = "invalide", .tileMap = core::TileMap{1, 1}}};
    }
    return *result.level;
}

/// Peuple un monde a partir d'un niveau, EXACTEMENT comme hmi::GameSession::loadLevel (tuiles +
/// tuiles + personnage a l'entree), sans aucune dependance GPU : c'est ce qui rend la scene
/// composee representative de ce que le joueur voit reellement.
core::World buildWorld(const core::Level& level) {
    core::World world;
    const core::TileMap& map = level.tileMap();
    core::buildLevelScene(
        world, level, [](core::TileType type) { return hmi::regionForTile(type); },
        [&](core::Entity entity, core::LayerKind, core::TileType type, int column, int row) {
            world.addComponent(
                entity, hmi::TileSkinTag{type, hmi::solidNeighborMask(map, column, row),
                                         hmi::textureOverrideAt(level.textureOverrides(),
                                                                core::GridPosition{column, row})});
        });

    // Personnage a l'entree, meme construction que hmi::GameSession::spawnPlayer (les champs
    // d'animation/collision n'affectent pas la composition, omis ici).
    const core::Entity player = world.createEntity();
    const core::Vector2 size = core::playerSize();
    world.addComponent(
        player, core::Transform{core::playerSpawnPosition(level.entry().column, level.entry().row),
                                size, 0.0f});
    core::Sprite sprite;
    sprite.region = core::AtlasRegion{0, 80, 16, 16};
    world.addComponent(player, sprite);
    hmi::PlayerSpriteTag playerTag;
    playerTag.quadSize = size;  // approximation suffisante : seul le VOLUME est mesure ici.
    world.addComponent(player, playerTag);
    world.addComponent(player, hmi::RenderLayerTag{hmi::RenderLayer::Player});
    return world;
}

/// Camera de reference, reproductible : cadree sur la salle de l'ENTREE du niveau, comme
/// hmi::GameSession::render (LOT-32, EX-REN-015) -- jamais un etat par defaut susceptible de
/// changer (cf. points d'attention de la tache).
///
/// Respecte les zones de camera DESSINEES A LA MAIN quand le niveau en declare (EX-LVL-007) --
/// meme resolution que `GameSession::applyCameraFraming`/`updateCurrentCameraZone` : sans cela, un
/// niveau `perRoom` a zones (comme `demo-final`) serait cadre sur le decoupage automatique en
/// grille uniforme au lieu de la zone que son auteur a dessinee, un ecart silencieux entre ce test
/// et le rendu reel (LOT-71).
hmi::Camera2D referenceCamera(const core::Level& level) {
    constexpr int VIEWPORT_WIDTH = 1280;
    constexpr int VIEWPORT_HEIGHT = 720;
    constexpr float ROOM_ZOOM_MARGIN = 0.92f;  // meme marge que GameSession::render.

    const core::CameraFramingConfig& framing = level.cameraFraming();
    float centerX = 0.0f;
    float centerY = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    const std::optional<std::size_t> zoneIndex =
        framing.mode == core::CameraFramingMode::PerRoom
            ? hmi::activeCameraZoneIndex(framing.zones, level.entry())
            : std::nullopt;
    if (zoneIndex) {
        const core::CameraZone& zone = framing.zones[*zoneIndex];
        centerX = static_cast<float>(zone.x) + static_cast<float>(zone.width) * 0.5f;
        centerY = static_cast<float>(zone.y) + static_cast<float>(zone.height) * 0.5f;
        width = static_cast<float>(zone.width);
        height = static_cast<float>(zone.height);
    } else {
        const core::TileMap& map = level.tileMap();
        const hmi::RoomGrid rooms(map.width(), map.height());
        const hmi::RoomBounds bounds = rooms.roomBounds(rooms.roomIndexAt(level.entry()));
        centerX = static_cast<float>(bounds.column) + static_cast<float>(bounds.width) * 0.5f;
        centerY = static_cast<float>(bounds.row) + static_cast<float>(bounds.height) * 0.5f;
        width = static_cast<float>(bounds.width);
        height = static_cast<float>(bounds.height);
    }

    hmi::Camera2D camera(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    camera.setCenter(core::Vector2{centerX, centerY});
    camera.setZoom(hmi::Camera2D::fitZoom(static_cast<float>(VIEWPORT_WIDTH),
                                          static_cast<float>(VIEWPORT_HEIGHT), width, height,
                                          ROOM_ZOOM_MARGIN));
    return camera;
}

/// Compose UNE PASSE d'image complete d'un niveau dans @p scene (jamais videe : la composition est
/// cumulative, meme contrat que hmi::composeWorldSprites), au meme patron que
/// hmi::SpriteRenderer::render mais sans aucune soumission GPU : fond, ombres, sprites du monde,
/// particules (aucune ici). Appelable plusieurs fois de suite sur la meme scene pour simuler une
/// emission en double (cf. le test negatif).
void composeIntoScene(hmi::ComposedScene& scene, core::World& world, const core::Level& level,
                      hmi::RenderMode mode) {
    const hmi::SceneTextures textures = testTextures();

    hmi::BackgroundTexture background;
    if (level.background().has_value()) {
        background = hmi::BackgroundTexture{backgroundTexture, 64, 64};
    }
    hmi::composeBackground(scene, background, level.tileMap().width(), level.tileMap().height(),
                           mode);
    hmi::composeShadows(scene, world, mode, textures, 0.0f);
    hmi::composeWorldSprites(scene, world, mode, textures, 0.0f);
    hmi::composeParticles(scene, world, mode, textures);
}

/// Compose une image complete d'un niveau (une seule passe, triee) : voir composeIntoScene.
hmi::ComposedScene composeLevelScene(core::World& world, const core::Level& level,
                                     hmi::RenderMode mode, const hmi::Camera2D& camera) {
    hmi::ComposedScene scene;
    scene.setVisibleBounds(camera.visibleBounds());
    composeIntoScene(scene, world, level, mode);
    scene.sort();
    return scene;
}

/// Plafond nommé du volume de primitives d'un niveau livré, dans les deux modes de rendu
/// (`EX-NFR-005`). Valeurs larges (cf. points d'attention de la tâche) : elles attrapent un facteur
/// deux accidentel, pas un chiffre exact — voir la mesure de référence datée du `LOT-62`
/// (`Documentation/Guide/guide-rendu.md`).
struct LevelBudget {
    std::string fileName;
    /// Plafond des primitives EXAMINEES (avant culling) : proportionnel au contenu total du
    /// niveau, pas a la salle visible seule.
    int consideredCeiling;
    /// Plafond des primitives SOUMISES (apres culling) : borne par la taille d'une salle
    /// (`hmi::RoomGrid::ROOM_WIDTH_TILES` x `ROOM_HEIGHT_TILES`), plus marge/fond/personnage.
    int submittedCeiling;
};

/// Les niveaux livrés (`Source/Elements/Levels/sequence-demo.json`, `LOT-25`, refondus `LOT-65`),
/// avec leur plafond. Un plafond par niveau, jamais global (`demo-final` et `demo-deplacement`
/// n'ont rien de comparable) — voir `epic.md`.
const std::vector<LevelBudget>& deliveredLevelBudgets() {
    static const std::vector<LevelBudget> budgets = {
        {"demo-deplacement.json", 160, 160},
        {"demo-saut.json", 210, 210},
        {"demo-double-saut.json", 140, 140},
        {"demo-wall-jump.json", 90, 90},
        {"demo-dash.json", 260, 260},
        {"demo-mouvement.json", 300, 300},
        {"demo-interrupteur.json", 175, 175},
        {"demo-plaque-pression.json", 235, 235},
        {"demo-cle.json", 190, 190},
        {"demo-bloc.json", 190, 190},
        {"demo-budget.json", 230, 160},
        {"demo-pente.json", 190, 190},
        {"demo-pente-gauche.json", 190, 190},
        {"demo-concave.json", 260, 260},
        {"demo-plafond.json", 270, 270},
        {"demo-bloc-reduit.json", 190, 190},
        {"demo-bloc-quart.json", 215, 215},
        {"demo-plateforme.json", 140, 140},
        {"demo-dangers-avances.json", 340, 230},
        {"demo-dangers-directionnels.json", 380, 270},
        // LOT-71 : demo-final reprend le tracé dense de Test-IA (24×24, cadrage `follow`) --
        // plus petit que l'ancien tracé (50×26) mais bien plus dense (~307 tuiles contre 234),
        // et la salle de suivi par défaut (24×14) y montre presque tout le niveau à la fois,
        // d'où un plafond soumis nettement relevé par rapport à l'ancien cadrage `perRoom`.
        {"demo-final.json", 650, 520},
    };
    return budgets;
}

/**
 * Plafond de **memoire de texture des plans** par niveau livre (EX-NFR-043, LOT-69 TACHE-09).
 *
 * Second axe du budget, et il ne se decline PAS par niveau comme celui des primitives : le volume
 * de primitives depend du contenu pose par l'auteur (chaque niveau a le sien), la memoire de plans
 * ne depend que de la taille du niveau et des densites declarees -- deux reglages, pas du contenu.
 * Un seul plafond dit donc exactement la meme chose que vingt et un plafonds identiques.
 *
 * 16 Mio : sur le plus grand niveau du depot (50x26), un plan a densite native pese 800x416x4, soit
 * 1,27 Mio. Le plafond en laisse donc passer DOUZE, la ou le format en autorise seize
 * (core::MAX_PLANES_PER_LEVEL) -- il refuse avant la limite de format, ce qui est le seul moyen
 * qu'il refuse quoi que ce soit un jour.
 */
constexpr std::size_t MAX_PLANE_TEXTURE_BYTES_PER_LEVEL = 16u * 1024u * 1024u;

/// Textures factices d'un plan, aux dimensions que sa densite impose -- resolvePlaneTextures
/// demande un TextureCache, donc un GPU (EX-NFR-004).
std::vector<hmi::PlaneTexture> fakePlaneTextures(const std::vector<core::Plane>& planes,
                                                 int levelWidth, int levelHeight) {
    static std::vector<int> storage(64);
    std::vector<hmi::PlaneTexture> textures;
    for (std::size_t rank = 0; rank < planes.size(); ++rank) {
        const hmi::PlanePixelSize size =
            hmi::planePixelSize(levelWidth, levelHeight, planes[rank].pixelsPerUnit);
        // Une identite DISTINCTE par plan : c'est ce qui rend chaque plan sa propre passe.
        textures.push_back(
            hmi::PlaneTexture{&storage[rank % storage.size()], size.width, size.height});
    }
    return textures;
}

/// @return @p count plans a densite native, tous distincts par leur nom de fichier.
std::vector<core::Plane> nativePlanes(std::size_t count) {
    std::vector<core::Plane> planes;
    for (std::size_t rank = 0; rank < count; ++rank) {
        core::Plane plane;
        plane.fileName = "plan-" + std::to_string(rank) + ".png";
        plane.pixelsPerUnit = core::PLANE_NATIVE_PIXELS_PER_UNIT;
        planes.push_back(std::move(plane));
    }
    return planes;
}

}  // namespace

/**
 * @brief Un plan supplémentaire à densité native sur un niveau **au plafond** le fait dépasser :
 *        le garde-fou est vérifié dans le sens qui compte — celui du refus. Un plafond qu'on n'a
 *        jamais vu refuser quoi que ce soit ne prouve rien.
 * \castest{<b>Un plan de plus sur un niveau au plafond fait echouer le budget de memoire.</b><br/>
 * \tcat Unitaire · Budget de rendu<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire un niveau de la taille du plus grand niveau livre, charge de plans a
 * densite native jusqu'au plafond.<br/>2. Ajouter un plan de plus, a la meme densite.<br/>
 * \tattendu Le total passe de « sous le plafond » a « au-dessus », sans que le format lui-meme
 * soit atteint (le nombre de plans reste sous core::MAX_PLANES_PER_LEVEL).
 * }
 */
TEST(RenderBudgetTest, UnPlanDePlusSurUnNiveauAuPlafondDepasseLeBudgetDeMemoire) {
    constexpr int WIDTH = 50;
    constexpr int HEIGHT = 26;

    // Nombre de plans natifs que le plafond laisse passer sur ce niveau -- calcule, jamais ecrit
    // en dur : le test doit rester juste si le plafond ou la densite native changent.
    const std::size_t perPlane = hmi::planeTextureMemoryBytes(core::Plane{}, WIDTH, HEIGHT);
    ASSERT_GT(perPlane, 0u);
    const std::size_t atCeiling = MAX_PLANE_TEXTURE_BYTES_PER_LEVEL / perPlane;
    ASSERT_GT(atCeiling, 0u);
    ASSERT_LT(atCeiling, core::MAX_PLANES_PER_LEVEL)
        << "Le plafond de memoire doit refuser AVANT la limite de format, sinon il ne refuse "
           "jamais rien.";

    const std::size_t bytesAtCeiling =
        hmi::planesTextureMemoryBytes(nativePlanes(atCeiling), WIDTH, HEIGHT);
    EXPECT_LE(bytesAtCeiling, MAX_PLANE_TEXTURE_BYTES_PER_LEVEL);

    const std::size_t bytesOneMore =
        hmi::planesTextureMemoryBytes(nativePlanes(atCeiling + 1), WIDTH, HEIGHT);
    EXPECT_GT(bytesOneMore, MAX_PLANE_TEXTURE_BYTES_PER_LEVEL)
        << "Un plan natif de plus doit faire echouer le budget : " << bytesOneMore << " octets.";
}

/**
 * @brief Le coût de composition des plans est **invariant en taille de niveau** : c'est ce qui les
 *        distingue des tuiles, et la raison pour laquelle un plafond en primitives ne les verrait
 *        pas grossir.
 * \castest{<b>Le cout en primitives des plans ne depend pas de la taille du niveau.</b><br/>
 * \tcat Unitaire · Budget de rendu<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer quatre plans sur un niveau, puis sur un niveau deux fois plus grand sur
 * chaque axe.<br/>
 * \tattendu Memes primitives examinees, soumises et passes ; la memoire, elle, quadruple.
 * }
 */
TEST(RenderBudgetTest, LeCoutDesPlansEstInvariantEnTailleDeNiveau) {
    constexpr int SMALL_WIDTH = 40;
    constexpr int SMALL_HEIGHT = 20;
    const std::vector<core::Plane> planes = nativePlanes(4);

    const auto compose = [&planes](int width, int height) {
        hmi::ComposedScene scene;
        hmi::composePlanes(scene, planes, fakePlaneTextures(planes, width, height), width, height,
                           hmi::RenderMode::Texture);
        scene.sort();
        return scene.statistics();
    };

    const hmi::SceneStatistics small = compose(SMALL_WIDTH, SMALL_HEIGHT);
    const hmi::SceneStatistics large = compose(SMALL_WIDTH * 2, SMALL_HEIGHT * 2);

    EXPECT_EQ(small.considered, large.considered);
    EXPECT_EQ(small.submitted, large.submitted);
    EXPECT_EQ(small.batches, large.batches);
    // La memoire, elle, suit la surface : c'est precisement l'axe que le budget en primitives ne
    // mesure pas.
    EXPECT_EQ(large.textureBytes, small.textureBytes * 4);
}

/**
 * @brief `N` plans coûtent exactement `+N` primitives soumises et `+N` passes, **aucune** écartée
 *        par le culling : un plan couvre le niveau entier, il est toujours à l'écran.
 * \castest{<b>N plans coutent N primitives soumises, N passes et aucune ecartee.</b><br/>
 * \tcat Unitaire · Budget de rendu<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Composer quatre plans dans une scene cadree sur une salle du niveau.<br/>
 * \tattendu Quatre primitives examinees, quatre soumises, quatre passes, zero ecartee.
 * }
 */
TEST(RenderBudgetTest, ChaquePlanCouteUnePrimitiveEtUnePasseJamaisEcartee) {
    constexpr int WIDTH = 50;
    constexpr int HEIGHT = 26;
    constexpr int PLANE_COUNT = 4;
    const std::vector<core::Plane> planes = nativePlanes(PLANE_COUNT);

    hmi::ComposedScene scene;
    // Cadrage sur une salle seulement : le culling est ACTIF, et n'ecarte pourtant aucun plan.
    scene.setVisibleBounds(core::Rect{core::Vector2{0.0f, 0.0f}, core::Vector2{20.0f, 12.0f}});
    hmi::composePlanes(scene, planes, fakePlaneTextures(planes, WIDTH, HEIGHT), WIDTH, HEIGHT,
                       hmi::RenderMode::Texture);
    scene.sort();

    const hmi::SceneStatistics stats = scene.statistics();
    EXPECT_EQ(stats.considered, PLANE_COUNT);
    EXPECT_EQ(stats.submitted, PLANE_COUNT);
    EXPECT_EQ(stats.culled, 0);
    EXPECT_EQ(stats.batches, PLANE_COUNT);
}
