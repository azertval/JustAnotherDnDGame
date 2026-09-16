// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_layer_visibility.cpp
 * @brief Tests unitaires du jeu de visibilités par calque du mode d'inspection de l'éditeur
 * (`LOT-51`, `EX-EDIT-044`).
 */

#include <optional>

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Sprite.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/World.h"
#include "HMI/Graphics/ComposedScene.h"
#include "HMI/Graphics/LayerVisibility.h"
#include "HMI/Graphics/MissingTexture.h"
#include "HMI/Graphics/PlayerSpriteTag.h"
#include "HMI/Graphics/QuadRecorder.h"
#include "HMI/Graphics/RenderLayer.h"
#include "HMI/Graphics/TileSkinTag.h"

namespace {

// Textures factices : la composition ne fait que comparer des identites (cf. test_quad_recorder).
int textureStorage = 0;
int missingStorage = 0;
hmi::TextureHandle texture = &textureStorage;

/// Textures de reference des tests (atlas et damier de repli).
hmi::SceneTextures testTextures() {
    hmi::SceneTextures textures;
    textures.atlas = texture;
    textures.atlasWidth = 16;
    textures.atlasHeight = 16;
    textures.missing = &missingStorage;
    textures.missingWidth = hmi::MISSING_TEXTURE_SIZE;
    textures.missingHeight = hmi::MISSING_TEXTURE_SIZE;
    return textures;
}

/// Ajoute une entite tuile minimale (Transform + Sprite + TileSkinTag), calque par defaut
/// (RenderLayer::Tile). La marque d'habillage est ce qui rattache l'entite a l'axe "skin" du mode
/// d'inspection : sans elle, hmi::resolveTileAppearance la traite comme une entite non habillable
/// (personnage, aide d'edition) et l'axe ne la masque jamais.
void addTile(core::World& world) {
    const core::Entity entity = world.createEntity();
    world.addComponent(entity,
                       core::Transform{core::Vector2{0.0f, 0.0f}, core::Vector2{1.0f, 1.0f}, 0.0f});
    core::Sprite sprite;
    sprite.region = core::AtlasRegion{0, 0, 16, 16};
    world.addComponent(entity, sprite);
    world.addComponent(entity, hmi::TileSkinTag{core::TileType::Solid, 0, std::nullopt});
}

/// Ajoute une entite personnage minimale (RenderLayer::Player), comme
/// hmi::GameSession::spawnPlayer.
void addPlayer(core::World& world) {
    const core::Entity entity = world.createEntity();
    world.addComponent(entity,
                       core::Transform{core::Vector2{2.0f, 2.0f}, core::Vector2{0.4f, 0.8f}, 0.0f});
    core::Sprite sprite;
    sprite.region = core::AtlasRegion{0, 80, 16, 16};
    world.addComponent(entity, sprite);
    world.addComponent(entity, hmi::PlayerSpriteTag{});
    world.addComponent(entity, hmi::RenderLayerTag{hmi::RenderLayer::Player});
}

}  // namespace

/**
 * @brief Tous les calques sont visibles par défaut.
 * \castest{<b>Tous les calques sont visibles par defaut.</b><br/>
 * \tcat Unitaire · Visibilité par calque<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire un jeu de visibilités par defaut.<br/>2. Interroger chaque calque.<br/>
 * \tattendu Tous les calques répondent visibles.
 * }
 */
TEST(LayerVisibilityTest, TousLesCalquesSontVisiblesParDefaut) {
    const hmi::LayerVisibility visibility;

    EXPECT_TRUE(visibility.visible(hmi::RenderLayer::Background));
    EXPECT_TRUE(visibility.visible(hmi::RenderLayer::Plane));
    EXPECT_TRUE(visibility.visible(hmi::RenderLayer::Shadow));
    EXPECT_TRUE(visibility.visible(hmi::RenderLayer::Tile));
    EXPECT_TRUE(visibility.visible(hmi::RenderLayer::Object));
    EXPECT_TRUE(visibility.visible(hmi::RenderLayer::Player));
    EXPECT_TRUE(visibility.visible(hmi::RenderLayer::Foreground));
    EXPECT_TRUE(visibility.visible(hmi::RenderLayer::UI));
    EXPECT_TRUE(visibility.visible(hmi::RenderLayer::EditorOverlay));
}

/**
 * @brief Masquer un calque ne change que ce calque.
 * \castest{<b>Masquer un calque ne change que ce calque.</b><br/>
 * \tcat Unitaire · Visibilité par calque<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Masquer le calque Plans.<br/>2. Interroger Plans et Personnage.<br/>
 * \tattendu Décor est masqué, Personnage reste visible.
 * }
 */
TEST(LayerVisibilityTest, MasquerUnCalqueNeChangeQueCeCalque) {
    hmi::LayerVisibility visibility;
    visibility.setVisible(hmi::RenderLayer::Plane, false);

    EXPECT_FALSE(visibility.visible(hmi::RenderLayer::Plane));
    EXPECT_TRUE(visibility.visible(hmi::RenderLayer::Player));
}

/**
 * @brief showAll rétablit tous les calques, même après plusieurs masquages.
 * \castest{<b>showAll retablit tous les calques.</b><br/>
 * \tcat Unitaire · Visibilité par calque<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Masquer plusieurs calques.<br/>2. Appeler showAll.<br/>
 * \tattendu Tous les calques redeviennent visibles.
 * }
 */
TEST(LayerVisibilityTest, ShowAllRetablitTousLesCalques) {
    hmi::LayerVisibility visibility;
    visibility.setVisible(hmi::RenderLayer::Background, false);
    visibility.setVisible(hmi::RenderLayer::Player, false);
    visibility.setVisible(hmi::RenderLayer::Foreground, false);

    visibility.showAll();

    EXPECT_TRUE(visibility.visible(hmi::RenderLayer::Background));
    EXPECT_TRUE(visibility.visible(hmi::RenderLayer::Player));
    EXPECT_TRUE(visibility.visible(hmi::RenderLayer::Foreground));
}

/**
 * @brief Un calque masqué (Personnage) n'émet aucune primitive ; un autre calque (Tuiles) reste
 * inchangé.
 * \castest{<b>Un calque masque n'emet aucune primitive ; les autres sont inchanges.</b><br/>
 * \tcat Unitaire · Visibilité par calque<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Peupler une entite Personnage et une entite Tuile.<br/>2. Masquer Personnage et
 * composer en mode Texture.<br/>
 * \tattendu Aucune primitive sur Personnage ; une primitive sur Tuiles.
 * }
 */
TEST(LayerVisibilityTest, CalqueMasqueNEmetAucunePrimitiveLesAutresInchanges) {
    core::World world;
    addPlayer(world);
    addTile(world);

    hmi::LayerVisibility visibility;
    visibility.setVisible(hmi::RenderLayer::Player, false);

    hmi::ComposedScene scene;
    hmi::composeWorldSprites(scene, world, hmi::RenderMode::Texture, testTextures(), 0.0f,
                             visibility);

    hmi::QuadRecorder recorder;
    recorder.record(scene);

    EXPECT_EQ(recorder.countOnLayer(hmi::RenderLayer::Player), 0) << recorder.describe();
    EXPECT_EQ(recorder.countOnLayer(hmi::RenderLayer::Tile), 1) << recorder.describe();
}

/**
 * @brief Combinaisons de visibilité : deux calques visibles, un seul, aucun.
 * \castest{<b>Combinaisons de visibilite : deux, un seul, aucun.</b><br/>
 * \tcat Unitaire · Visibilité par calque<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Peupler une Tuile et un Personnage.<br/>2. Composer en mode Texture
 * avec les deux visibles, puis un seul, puis aucun.<br/>
 * \tattendu Le nombre de primitives suit exactement les calques visibles a chaque etape.
 * }
 */
TEST(LayerVisibilityTest, CombinaisonsDeuxUnSeulAucun) {
    core::World world;
    addTile(world);
    addPlayer(world);

    hmi::LayerVisibility visibility;

    // Deux calques visibles (defaut) : les deux entites sont composees.
    {
        hmi::ComposedScene scene;
        hmi::composeWorldSprites(scene, world, hmi::RenderMode::Texture, testTextures(), 0.0f,
                                 visibility);
        EXPECT_EQ(scene.size(), 2u);
    }

    // Un seul calque visible : une seule entite composee.
    visibility.setVisible(hmi::RenderLayer::Player, false);
    {
        hmi::ComposedScene scene;
        hmi::composeWorldSprites(scene, world, hmi::RenderMode::Texture, testTextures(), 0.0f,
                                 visibility);
        EXPECT_EQ(scene.size(), 1u);
        EXPECT_EQ(scene.quads().front().layer, hmi::RenderLayer::Tile);
    }

    // Aucun calque visible : rien n'est compose.
    visibility.setVisible(hmi::RenderLayer::Tile, false);
    {
        hmi::ComposedScene scene;
        hmi::composeWorldSprites(scene, world, hmi::RenderMode::Texture, testTextures(), 0.0f,
                                 visibility);
        EXPECT_EQ(scene.size(), 0u);
    }
}

/**
 * @brief Un jeu de visibilités par défaut ne change rien à la composition d'une scène — la valeur
 * par défaut passée implicitement par `hmi::GameSession` (qui ne fournit jamais cet argument) doit
 * rester strictement neutre.
 * \castest{<b>Un jeu de visibilite par defaut est neutre sur la composition.</b><br/>
 * \tcat Unitaire · Visibilité par calque<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Peupler une tuile et un personnage.<br/>2. Composer avec et sans jeu de visibilites
 * explicite, en mode Texture.<br/>
 * \tattendu Les deux scenes ont le meme nombre de primitives.
 * }
 */
TEST(LayerVisibilityTest, DefautNeutreSurLaCompositionDUneScene) {
    core::World world;
    addTile(world);
    addPlayer(world);

    hmi::ComposedScene withoutVisibility;
    hmi::composeWorldSprites(withoutVisibility, world, hmi::RenderMode::Texture, testTextures(),
                             0.0f);

    hmi::ComposedScene withDefaultVisibility;
    hmi::composeWorldSprites(withDefaultVisibility, world, hmi::RenderMode::Texture, testTextures(),
                             0.0f, hmi::LayerVisibility{});

    EXPECT_EQ(withoutVisibility.size(), withDefaultVisibility.size());
    EXPECT_EQ(withoutVisibility.size(), 2u);
}
