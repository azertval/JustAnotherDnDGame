// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_arena_scene_renderer.cpp
 * @brief Le rendu QRhi de la scène du Colisée (`LOT-86` Phase 5), **hors écran** : cycle de vie
 *        des ressources et une vraie image de la scène composée.
 *
 * `hmi::ArenaViewportItem` n'est qu'un hôte Qt Quick ; ce qui peut fuir ou planter — créer,
 * libérer, recréer sur une autre interface QRhi — vit dans `hmi::ArenaSceneRenderer`, que ce test
 * fait tourner sur un `QRhi` Direct3D 11 sans fenêtre, avec les pièces **livrées** de la planche.
 * Se saute proprement si la machine n'offre aucune interface QRhi (`EX-NFR-004`).
 */

#include <QImage>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>

#include <gtest/gtest.h>
#include <rhi/qrhi.h>

#include "Core/Combat/Arena.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/TileMap.h"
#include "HMI/Graphics/ArenaSceneComposer.h"
#include "HMI/Graphics/ArenaSceneRenderer.h"

namespace {

using core::CombatSide;

/// Côté, en pixels, de la cible de rendu.
constexpr int TARGET_SIZE = 256;

/// Fond franc, qu'aucune pièce de la planche ne reproduit à l'identique.
constexpr float CLEAR[4] = {1.0f, 0.0f, 1.0f, 1.0f};

std::filesystem::path coliseum() {
    return std::filesystem::path(JADG_ASSETS_DIR) / "Coliseum";
}

std::unique_ptr<QRhi> createOffscreenRhi() {
#ifdef Q_OS_WIN
    QRhiD3D11InitParams params;
    if (QRhi* const rhi = QRhi::create(QRhi::D3D11, &params)) {
        return std::unique_ptr<QRhi>(rhi);
    }
#endif
    return nullptr;
}

/// Une cible de rendu relisible, et ce qui la tient.
struct OffscreenTarget {
    std::unique_ptr<QRhiTexture> texture;
    std::unique_ptr<QRhiTextureRenderTarget> renderTarget;
    std::unique_ptr<QRhiRenderPassDescriptor> pass;

    explicit OffscreenTarget(QRhi& rhi)
        : texture(rhi.newTexture(QRhiTexture::RGBA8, QSize(TARGET_SIZE, TARGET_SIZE), 1,
                                 QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource)) {
        EXPECT_TRUE(texture->create());
        renderTarget.reset(rhi.newTextureRenderTarget({{texture.get()}}));
        pass.reset(renderTarget->newCompatibleRenderPassDescriptor());
        renderTarget->setRenderPassDescriptor(pass.get());
        EXPECT_TRUE(renderTarget->create());
    }
};

/// Dessine une image de @p renderer dans @p target, et la relit.
QImage renderFrame(QRhi& rhi, hmi::ArenaSceneRenderer& renderer, OffscreenTarget& target) {
    QRhiCommandBuffer* commandBuffer = nullptr;
    if (rhi.beginOffscreenFrame(&commandBuffer) != QRhi::FrameOpSuccess) {
        ADD_FAILURE() << "beginOffscreenFrame";
        return {};
    }
    renderer.render(commandBuffer, target.renderTarget.get(), 1.0f / 60.0f, CLEAR);

    QRhiReadbackResult readback;
    QRhiResourceUpdateBatch* const readbackBatch = rhi.nextResourceUpdateBatch();
    readbackBatch->readBackTexture({target.texture.get()}, &readback);
    commandBuffer->resourceUpdate(readbackBatch);
    if (rhi.endOffscreenFrame() != QRhi::FrameOpSuccess) {
        ADD_FAILURE() << "endOffscreenFrame";
        return {};
    }
    return QImage(reinterpret_cast<const uchar*>(readback.data.constData()),
                  readback.pixelSize.width(), readback.pixelSize.height(), QImage::Format_RGBA8888)
        .copy();
}

/// Nombre de pixels qui ne sont pas le fond d'effacement.
std::size_t paintedPixels(const QImage& image) {
    std::size_t painted = 0;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            const QColor pixel = image.pixelColor(x, y);
            if (pixel.red() != 255 || pixel.green() != 0 || pixel.blue() != 255) {
                ++painted;
            }
        }
    }
    return painted;
}

/// La piste 5x4 ceinte de murs du test du composeur, une porte en (0, 2).
core::Level piste() {
    core::TileMap carte(5, 4);
    for (int x = 0; x < 5; ++x) {
        carte.setTile(x, 0, core::TileType::Wall);
        carte.setTile(x, 3, core::TileType::Wall);
    }
    for (int y = 0; y < 4; ++y) {
        carte.setTile(0, y, core::TileType::Wall);
        carte.setTile(4, y, core::TileType::Wall);
    }
    carte.setTile(0, 2, core::TileType::Empty);
    return core::Level(core::LevelData{.name = "piste",
                                       .tileMap = std::move(carte),
                                       .entities = {},
                                       .entry = {1, 1},
                                       .exit = {3, 2}});
}

core::ArenaContestant concurrent(const std::string& nom, CombatSide camp, int colonne, int ligne) {
    const core::CombatantProfile profil{
        .name = nom, .side = camp, .maximumHitPoints = 10, .currentHitPoints = 10, .movement = 6};
    return {.profile = profil,
            .attacks = {},
            .position = core::GridPosition{.column = colonne, .row = ligne},
            .markId = {}};
}

/// L'instantané de la piste, deux allies et deux ennemis montés. La session meurt ici : le rendu
/// ne dessine que l'instantané.
hmi::ArenaSceneSnapshot snapshotDePiste() {
    core::ArenaSession session{piste()};
    core::ArenaBout bout{.seed = 7, .lethal = false, .heroicMark = false};
    bout.contestants.push_back(concurrent("Bram", CombatSide::Allies, 1, 1));
    bout.contestants.push_back(concurrent("Eve", CombatSide::Allies, 3, 1));
    bout.contestants.push_back(concurrent("Orc", CombatSide::Enemies, 1, 2));
    bout.contestants.push_back(concurrent("Rat", CombatSide::Enemies, 2, 2));
    const core::ArenaMount mount = session.mount(bout);
    EXPECT_TRUE(mount.refusals.empty());
    return hmi::snapshotArenaScene(session);
}

/// Sols (20), enceinte (15) et figurines (4) de la piste.
constexpr std::size_t PISTE_QUADS = 20 + 15 + 4;

}  // namespace

/**
 * @brief Créer, libérer deux fois, recréer : les ressources suivent, sans fuite de lot ni plantage.
 * \castest{<b>Le cycle de vie des ressources QRhi de l'arene est sur.</b><br/>
 * \tcat Unitaire · Rendu QRhi de l'arene<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Creer les ressources sur une interface QRhi hors ecran, sans jamais dessiner.<br/>
 *          2. Liberer, puis liberer encore.<br/>
 *          3. Recreer, dessiner une image, detruire le rendu avant l'interface.<br/>
 * \tattendu Toutes les pieces livrees sont chargees et le damier existe ; apres liberation plus
 *           rien n'est cree ; la recreation dessine une image valide.
 * }
 */
TEST(ArenaSceneRendererTest, CreationLiberationRecreation) {
    const std::unique_ptr<QRhi> rhi = createOffscreenRhi();
    if (!rhi) {
        GTEST_SKIP() << "Aucune interface QRhi disponible sur cette machine.";
    }
    OffscreenTarget target(*rhi);
    {
        hmi::ArenaSceneRenderer renderer(coliseum());
        ASSERT_FALSE(renderer.catalog().heroes().empty()) << "manifeste du Colisee illisible";
        EXPECT_FALSE(renderer.ensureResources(nullptr));
        EXPECT_FALSE(renderer.created());

        ASSERT_TRUE(renderer.ensureResources(rhi.get()));
        EXPECT_TRUE(renderer.created());
        EXPECT_EQ(renderer.rhi(), rhi.get());
        EXPECT_EQ(renderer.textures().byPath.size(),
                  hmi::arenaTexturePaths(renderer.catalog()).size())
            << "une piece livree n'a pas pu etre chargee";
        EXPECT_NE(renderer.textures().missing.texture, nullptr);
        // Idempotent sur la meme interface.
        const hmi::TextureHandle sand = renderer.textures().resolve("terrain/sand.png").texture;
        EXPECT_TRUE(renderer.ensureResources(rhi.get()));
        EXPECT_EQ(renderer.textures().resolve("terrain/sand.png").texture, sand);

        // Liberee sans avoir jamais dessine : le lot de creation doit etre rendu, pas perdu.
        renderer.release();
        EXPECT_FALSE(renderer.created());
        EXPECT_EQ(renderer.rhi(), nullptr);
        EXPECT_TRUE(renderer.textures().byPath.empty());
        renderer.release();

        ASSERT_TRUE(renderer.ensureResources(rhi.get()));
        renderer.setSnapshot(snapshotDePiste());
        const QImage image = renderFrame(*rhi, renderer, target);
        EXPECT_EQ(image.size(), QSize(TARGET_SIZE, TARGET_SIZE));
    }
    // Le rendu est detruit ici, l'interface encore vivante : c'est l'ordre de Qt Quick.
}

/**
 * @brief Une grille de test donne une scène composée complète, et une image qui n'est pas que le
 *        fond.
 * \castest{<b>La scene de l'arene devient des pixels.</b><br/>
 * \tcat Unitaire · Rendu QRhi de l'arene<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Tirer l'instantane de la piste 5x4 (quatre combattants), puis detruire la
 *             session.<br/>2. Dessiner une image hors ecran et la relire.<br/>
 * \tattendu 39 quads composes, tous sur une piece chargee (aucun damier) ; une part notable de
 *           l'image est peinte, et le fond subsiste dans les coins.
 * }
 */
TEST(ArenaSceneRendererTest, SceneNonVideSurUneGrilleDeTest) {
    const std::unique_ptr<QRhi> rhi = createOffscreenRhi();
    if (!rhi) {
        GTEST_SKIP() << "Aucune interface QRhi disponible sur cette machine.";
    }
    OffscreenTarget target(*rhi);
    hmi::ArenaSceneRenderer renderer(coliseum());
    ASSERT_TRUE(renderer.ensureResources(rhi.get()));

    renderer.setSnapshot(snapshotDePiste());
    EXPECT_TRUE(renderer.animating());
    const QImage image = renderFrame(*rhi, renderer, target);
    ASSERT_EQ(image.size(), QSize(TARGET_SIZE, TARGET_SIZE));

    ASSERT_EQ(renderer.composed().size(), PISTE_QUADS);
    for (const hmi::ComposedQuad& quad : renderer.composed().quads()) {
        EXPECT_NE(quad.texture, nullptr);
        EXPECT_NE(quad.texture, renderer.textures().missing.texture)
            << "piece tombee sur le damier";
    }

    const std::size_t painted = paintedPixels(image);
    EXPECT_GT(painted, static_cast<std::size_t>(TARGET_SIZE * TARGET_SIZE / 5))
        << "la scene cadree devrait couvrir une part notable de la cible";
    EXPECT_LT(painted, static_cast<std::size_t>(TARGET_SIZE * TARGET_SIZE))
        << "le losange de la scene laisse le fond visible dans les coins";

    // Une scene vide ne dessine que le fond, et n'appelle plus d'image.
    renderer.setSnapshot({});
    EXPECT_FALSE(renderer.animating());
    const QImage empty = renderFrame(*rhi, renderer, target);
    EXPECT_EQ(renderer.composed().size(), 0U);
    EXPECT_EQ(paintedPixels(empty), 0U);
}

/**
 * @brief L'interface QRhi change (fenêtre changée) : tout est libéré puis recréé sur la nouvelle,
 * et le dessin reprend.
 * \castest{<b>Le rendu de l'arene se recree sur une nouvelle interface QRhi.</b><br/>
 * \tcat Unitaire · Rendu QRhi de l'arene<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Creer et dessiner sur une premiere interface.<br/>2. Appeler ensureResources avec une
 *          seconde interface, puis dessiner dessus.<br/>
 * \tattendu Le rendu designe la seconde interface, ses textures sont neuves, et l'image est peinte.
 * }
 */
TEST(ArenaSceneRendererTest, RecreationSurUneAutreInterface) {
    const std::unique_ptr<QRhi> first = createOffscreenRhi();
    const std::unique_ptr<QRhi> second = createOffscreenRhi();
    if (!first || !second) {
        GTEST_SKIP() << "Deux interfaces QRhi hors ecran indisponibles sur cette machine.";
    }
    OffscreenTarget firstTarget(*first);
    OffscreenTarget secondTarget(*second);

    hmi::ArenaSceneRenderer renderer(coliseum());
    renderer.setSnapshot(snapshotDePiste());
    ASSERT_TRUE(renderer.ensureResources(first.get()));
    EXPECT_GT(paintedPixels(renderFrame(*first, renderer, firstTarget)), 0U);

    ASSERT_TRUE(renderer.ensureResources(second.get()));
    EXPECT_EQ(renderer.rhi(), second.get());
    EXPECT_EQ(renderer.textures().byPath.size(), hmi::arenaTexturePaths(renderer.catalog()).size());
    EXPECT_GT(paintedPixels(renderFrame(*second, renderer, secondTarget)), 0U);
    EXPECT_EQ(renderer.composed().size(), PISTE_QUADS);
}
