// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_player_sprite.cpp
 * @brief Tests unitaires de l'habillage du personnage : ancrage image/hitbox, repli entre clips
 *        et retournement horizontal (`LOT-48`).
 */

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Sprite.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/World.h"
#include "Core/Physics/PlayerSpawn.h"
#include "HMI/Graphics/ComposedScene.h"
#include "HMI/Graphics/PlayerSprite.h"
#include "HMI/Graphics/PlayerSpriteTag.h"
#include "HMI/Graphics/QuadRecorder.h"
#include "HMI/Graphics/RenderMode.h"

namespace {
int textureStorage = 0;
int otherTextureStorage = 0;
hmi::TextureHandle atlasTexture = &textureStorage;
hmi::TextureHandle characterSheetTexture = &otherTextureStorage;
}  // namespace

/**
 * @brief Une image de la même taille que la hitbox produit un quad sans décalage.
 * \castest{<b>Image de la taille de la hitbox : quad sans decalage.</b><br/>
 * \tcat Unitaire · Player Sprite<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Calculer le quad pour une image 16x16 px (1x1 unite) et une hitbox 1x1 unite.<br/>
 * \tattendu La taille du quad vaut 1x1 unite (16 px = 1 unite), decalage nul (hitbox et image de
 * meme taille : l'ancrage centre-bas coincide avec le coin haut-gauche).
 * }
 */
TEST(PlayerSpriteTest, AncrageImageDeMemeTailleQueLaHitbox) {
    const core::Vector2 hitboxSize{16.0f / 16.0f, 16.0f / 16.0f};  // 1x1 unite = 16x16 px.
    const hmi::PlayerSpriteQuad quad =
        hmi::computePlayerSpriteQuad(core::Vector2{16.0f, 16.0f}, hitboxSize);

    EXPECT_FLOAT_EQ(quad.size.x, 1.0f);
    EXPECT_FLOAT_EQ(quad.size.y, 1.0f);
    EXPECT_FLOAT_EQ(quad.offset.x, 0.0f);
    EXPECT_FLOAT_EQ(quad.offset.y, 0.0f);
}

/**
 * @brief Une image plus grande que la hitbox est centrée horizontalement et alignée par le bas :
 *        elle déborde symétriquement de chaque côté, jamais vers le bas.
 * \castest{<b>Image plus grande que la hitbox : centree horizontalement, alignee par le
 * bas.</b><br/> \tcat Unitaire · Player Sprite<br/> \tcrit Critique<br/> \tetapes 1. Calculer le
 * quad pour une image 32x32 px et la hitbox reelle du personnage
 * (`core::playerSize()`).<br/>
 * \tattendu Le decalage horizontal est negatif de la moitie du surplus de largeur ; le bas de
 * l'image (offset.y + size.y) coincide avec le bas de la hitbox (hitboxSize.y).
 * }
 */
TEST(PlayerSpriteTest, ImagePlusGrandeCentreeEtAncreeParLeBas) {
    const core::Vector2 hitboxSize = core::playerSize();  // 0,4 x 0,8 unite.
    const hmi::PlayerSpriteQuad quad =
        hmi::computePlayerSpriteQuad(core::Vector2{32.0f, 32.0f}, hitboxSize);

    const float imageWidthWorld = 32.0f / 16.0f;   // 2 unites.
    const float imageHeightWorld = 32.0f / 16.0f;  // 2 unites.
    EXPECT_FLOAT_EQ(quad.size.x, imageWidthWorld);
    EXPECT_FLOAT_EQ(quad.size.y, imageHeightWorld);
    // Centrage horizontal : la hitbox est plus etroite, le decalage est negatif et symetrique.
    EXPECT_FLOAT_EQ(quad.offset.x, (hitboxSize.x - imageWidthWorld) * 0.5f);
    EXPECT_LT(quad.offset.x, 0.0f);
    // Ancrage par le bas : le bas de l'image coincide EXACTEMENT avec le bas de la hitbox.
    EXPECT_FLOAT_EQ(quad.offset.y + quad.size.y, hitboxSize.y);
}

/**
 * @brief Une image plus grande que la hitbox ne déplace jamais celle-ci : le calcul du quad ne lit
 *        ni ne modifie la position simulée, seulement des tailles.
 * \castest{<b>Le calcul du quad ne modifie jamais la hitbox (fonction pure).</b><br/>
 * \tcat Unitaire · Player Sprite<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Calculer le quad pour plusieurs tailles d'image, avec la meme hitbox.<br/>
 * \tattendu hitboxSize n'est jamais lu autrement qu'en entree : deux tailles d'image differentes
 * ne changent pas hitboxSize (verification par construction, cf. signature pure de la fonction).
 * }
 */
TEST(PlayerSpriteTest, ImagePlusGrandeNeDeplacePasLaHitbox) {
    const core::Vector2 hitboxSize = core::playerSize();
    const hmi::PlayerSpriteQuad small =
        hmi::computePlayerSpriteQuad(core::Vector2{16.0f, 16.0f}, hitboxSize);
    const hmi::PlayerSpriteQuad large =
        hmi::computePlayerSpriteQuad(core::Vector2{48.0f, 48.0f}, hitboxSize);

    // Les deux appels partagent la MEME hitboxSize passee par valeur : rien dans la fonction ne
    // pourrait la modifier meme si elle avait ete passee par reference (EX-ARCH-012).
    EXPECT_NE(small.size.x, large.size.x);
    EXPECT_EQ(hitboxSize.x, core::playerSize().x);
    EXPECT_EQ(hitboxSize.y, core::playerSize().y);
}

/**
 * @brief L'ancrage est symétrique horizontalement : une image plus large que haute reste centrée,
 *        propriété nécessaire pour que le retournement (TACHE-03) n'ait besoin d'aucune correction
 *        de position.
 * \castest{<b>L'ancrage horizontal est symetrique (prealable au retournement).</b><br/>
 * \tcat Unitaire · Player Sprite<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Calculer le quad d'une image plus large que la hitbox.<br/>
 * \tattendu Le debord a gauche egale le debord a droite.
 * }
 */
TEST(PlayerSpriteTest, AncrageHorizontalSymetrique) {
    const core::Vector2 hitboxSize = core::playerSize();
    const hmi::PlayerSpriteQuad quad =
        hmi::computePlayerSpriteQuad(core::Vector2{64.0f, 16.0f}, hitboxSize);

    const float overflowLeft = -quad.offset.x;
    const float overflowRight = (quad.offset.x + quad.size.x) - hitboxSize.x;
    EXPECT_FLOAT_EQ(overflowLeft, overflowRight);
}

/**
 * @brief Un clip déclaré est utilisé tel quel, sans repli.
 * \castest{<b>Un clip declare est utilise tel quel.</b><br/>
 * \tcat Unitaire · Player Sprite<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Resoudre "run" dans un jeu qui le declare.<br/>
 * \tattendu Le nom resolu est "run".
 * }
 */
TEST(PlayerSpriteTest, ClipDeclareUtiliseTelQuel) {
    const std::vector<std::string> declared{"idle", "run", "jump", "fall"};
    EXPECT_EQ(hmi::resolveDeclaredPlayerClip(declared, "fall"), "fall");
}

/**
 * @brief Un clip absent retombe sur le plus proche déclaré (chaîne de repli documentée).
 * \castest{<b>Chute/atterrissage/glissade/dash retombent sur le clip le plus proche.</b><br/>
 * \tcat Unitaire · Player Sprite<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Resoudre fall/land/wallslide/dash dans le jeu procedural (idle/run/jump
 * seulement).<br/>
 * \tattendu fall -> jump, land -> idle, wallslide -> jump, dash -> run.
 * }
 */
TEST(PlayerSpriteTest, ClipsLot48RepliSurLAtlasProcedural) {
    const std::vector<std::string>& declared = hmi::proceduralPlayerClipNames();
    EXPECT_EQ(hmi::resolveDeclaredPlayerClip(declared, "fall"), "jump");
    EXPECT_EQ(hmi::resolveDeclaredPlayerClip(declared, "land"), "idle");
    EXPECT_EQ(hmi::resolveDeclaredPlayerClip(declared, "wallslide"), "jump");
    EXPECT_EQ(hmi::resolveDeclaredPlayerClip(declared, "dash"), "run");
}

/**
 * @brief Une chaîne de repli complète (aucun clip déclaré) retombe finalement sur "idle", jamais
 *        sur une chaîne vide ni une boucle infinie.
 * \castest{<b>Aucun clip declare : repli final sur idle.</b><br/>
 * \tcat Unitaire · Player Sprite<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Resoudre "dash" dans un jeu totalement vide.<br/>
 * \tattendu Le nom resolu est "idle".
 * }
 */
TEST(PlayerSpriteTest, AucunClipDeclareRetombeSurIdle) {
    const std::vector<std::string> declared{};
    EXPECT_EQ(hmi::resolveDeclaredPlayerClip(declared, "dash"), "idle");
}

/**
 * @brief L'atlas procédural ne déclare que trois clips (idle/run/jump), seule source de vérité
 *        partagée avec `hmi::PlayerClipKind`.
 * \castest{<b>L'atlas procedural ne declare que idle/run/jump.</b><br/>
 * \tcat Unitaire · Player Sprite<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Lire proceduralPlayerClipNames().<br/>
 * \tattendu Exactement {"idle", "run", "jump"}.
 * }
 */
TEST(PlayerSpriteTest, NomsProceduralsSontIdleRunJump) {
    const std::vector<std::string> expected{"idle", "run", "jump"};
    EXPECT_EQ(hmi::proceduralPlayerClipNames(), expected);
}

namespace {
// Monde a une entite personnage (Transform + Sprite + PlayerSpriteTag), pour verifier la
// composition (hmi::composeWorldSprites) sans GPU.
core::Entity spawnPlayerEntity(core::World& world, const hmi::PlayerSpriteTag& tag) {
    const core::Entity entity = world.createEntity();
    world.addComponent(entity,
                       core::Transform{core::Vector2{5.0f, 5.0f}, core::Vector2{0.4f, 0.8f}, 0.0f});
    core::Sprite sprite;
    sprite.region = core::AtlasRegion{0, 80, 16, 16};  // region procedurale (Physique).
    world.addComponent(entity, sprite);
    world.addComponent(entity, tag);
    return entity;
}

hmi::SceneTextures playerTestTextures() {
    hmi::SceneTextures textures;
    textures.atlas = atlasTexture;
    textures.atlasWidth = 96;
    textures.atlasHeight = 112;
    textures.characterSheet = characterSheetTexture;
    textures.characterSheetWidth = 64;
    textures.characterSheetHeight = 32;
    return textures;
}
}  // namespace

/**
 * @brief En `RenderMode::Physique`, un personnage portant `PlayerSpriteTag` est dessiné par le
 *        chemin générique (`Sprite::region` + `Transform::scale`), sans effet du tag : non
 *        régression du mode diagnostic (`LOT-48` TACHE-01).
 * \castest{<b>RenderMode::Physique ignore PlayerSpriteTag.</b><br/>
 * \tcat Unitaire · Player Sprite<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer un personnage taggue en RenderMode::Physique.<br/>
 * \tattendu Le quad soumis vient de la region du sprite et de l'echelle du transform (0,4x0,8
 * unite), pas du tag.
 * }
 */
TEST(PlayerSpriteTest, ModePhysiqueIgnoreLeTag) {
    core::World world;
    hmi::PlayerSpriteTag tag;
    tag.quadOffset = core::Vector2{-5.0f, -5.0f};  // tres different, pour detecter une fuite.
    tag.quadSize = core::Vector2{9.0f, 9.0f};
    spawnPlayerEntity(world, tag);

    hmi::ComposedScene scene;
    hmi::composeWorldSprites(scene, world, hmi::RenderMode::Physique, playerTestTextures(), 0.0f);

    hmi::QuadRecorder recorder;
    recorder.record(scene);
    ASSERT_EQ(recorder.size(), 1u);
    // Position/taille du chemin generique : Transform::position (5,5), region 16x16 px * scale
    // 0,4x0,8 = 0,4x0,8 unite -- rien du tag n'a ete applique.
    EXPECT_TRUE(recorder.containsSpriteAt(5.0f, 5.0f)) << recorder.describe();
}

/**
 * @brief En `RenderMode::Texture`, le quad d'un personnage taggué vient du tag (taille et
 *        décalage), pas de `Transform::scale`.
 * \castest{<b>RenderMode::Texture applique le quad du tag.</b><br/>
 * \tcat Unitaire · Player Sprite<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer un personnage taggue (quad plus grand que la hitbox) en
 * RenderMode::Texture.<br/>
 * \tattendu Le quad soumis est decale/dimensionne selon le tag, a la position Transform +
 * decalage.
 * }
 */
TEST(PlayerSpriteTest, ModeTextureAppliqueLeQuadDuTag) {
    core::World world;
    hmi::PlayerSpriteTag tag;
    tag.textureRegion = core::AtlasRegion{0, 0, 32, 32};
    tag.usesCharacterSheet = true;
    tag.quadOffset = core::Vector2{-0.5f, -1.0f};
    tag.quadSize = core::Vector2{2.0f, 2.0f};
    spawnPlayerEntity(world, tag);

    hmi::ComposedScene scene;
    hmi::composeWorldSprites(scene, world, hmi::RenderMode::Texture, playerTestTextures(), 0.0f);

    ASSERT_EQ(scene.quads().size(), 1u);
    const hmi::SpriteQuad& quad = scene.quads().front().sprite;
    EXPECT_FLOAT_EQ(quad.x, 5.0f - 0.5f);
    EXPECT_FLOAT_EQ(quad.y, 5.0f - 1.0f);
    EXPECT_FLOAT_EQ(quad.width, 2.0f);
    EXPECT_FLOAT_EQ(quad.height, 2.0f);
    EXPECT_EQ(scene.quads().front().texture, characterSheetTexture);
}

/**
 * @brief Sans spritesheet externe chargée (`usesCharacterSheet == false`), le tag lie l'atlas :
 *        repli procédural en `RenderMode::Texture` (`LOT-48` AC#2).
 * \castest{<b>Sans spritesheet, RenderMode::Texture lie l'atlas.</b><br/>
 * \tcat Unitaire · Player Sprite<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Composer un personnage taggue avec usesCharacterSheet=false.<br/>
 * \tattendu La texture liee est l'atlas, pas la spritesheet.
 * }
 */
TEST(PlayerSpriteTest, ReplinProceduralLieLAtlasEnModeTexture) {
    core::World world;
    hmi::PlayerSpriteTag tag;
    tag.usesCharacterSheet = false;
    tag.textureRegion = core::AtlasRegion{0, 80, 16, 16};
    tag.quadSize = core::Vector2{0.4f, 0.8f};
    spawnPlayerEntity(world, tag);

    hmi::ComposedScene scene;
    hmi::composeWorldSprites(scene, world, hmi::RenderMode::Texture, playerTestTextures(), 0.0f);

    ASSERT_EQ(scene.quads().size(), 1u);
    EXPECT_EQ(scene.quads().front().texture, atlasTexture);
}

/**
 * @brief Le retournement horizontal échange les coordonnées de texture `u0`/`u1`, sans modifier la
 *        position ni la taille du quad (ancrage symétrique, `LOT-48` TACHE-03).
 * \castest{<b>Le retournement echange u0/u1 sans deplacer le quad.</b><br/>
 * \tcat Unitaire · Player Sprite<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer le meme personnage, flipHorizontal=false puis true.<br/>
 * \tattendu u0 et u1 sont echanges ; x/y/width/height identiques dans les deux cas.
 * }
 */
TEST(PlayerSpriteTest, RetournementEchangeUvSansDeplacerLeQuad) {
    hmi::PlayerSpriteTag baseTag;
    baseTag.textureRegion = core::AtlasRegion{0, 0, 32, 32};
    baseTag.usesCharacterSheet = true;
    baseTag.quadSize = core::Vector2{2.0f, 2.0f};

    core::World worldFacingRight;
    hmi::PlayerSpriteTag facingRight = baseTag;
    facingRight.flipHorizontal = false;
    spawnPlayerEntity(worldFacingRight, facingRight);
    hmi::ComposedScene sceneRight;
    hmi::composeWorldSprites(sceneRight, worldFacingRight, hmi::RenderMode::Texture,
                             playerTestTextures(), 0.0f);

    core::World worldFacingLeft;
    hmi::PlayerSpriteTag facingLeft = baseTag;
    facingLeft.flipHorizontal = true;
    spawnPlayerEntity(worldFacingLeft, facingLeft);
    hmi::ComposedScene sceneLeft;
    hmi::composeWorldSprites(sceneLeft, worldFacingLeft, hmi::RenderMode::Texture,
                             playerTestTextures(), 0.0f);

    const hmi::SpriteQuad& right = sceneRight.quads().front().sprite;
    const hmi::SpriteQuad& left = sceneLeft.quads().front().sprite;

    EXPECT_FLOAT_EQ(right.u0, left.u1);
    EXPECT_FLOAT_EQ(right.u1, left.u0);
    EXPECT_FLOAT_EQ(right.x, left.x);
    EXPECT_FLOAT_EQ(right.y, left.y);
    EXPECT_FLOAT_EQ(right.width, left.width);
    EXPECT_FLOAT_EQ(right.height, left.height);
}
