// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file bench_canvas.cpp
 * @brief Mesure de la composition d'un lieu : ce que le canevas de l'éditeur refait à chaque geste
 *        et le jeu à chaque image (`LOT-EDITOR-02`, feuille de route de l'éditeur, §5 règle 5).
 *
 * Martpart, la carte la plus chargée (48 × 40 cases, 429 pièces de relief). Les textures sont des
 * identités sans image, à la taille que déclare le manifeste : on mesure l'instantané et la
 * composition triée, pas le disque ni le GPU.
 */

#include <cstdint>
#include <filesystem>
#include <vector>

#include <benchmark/benchmark.h>

#include "Core/Combat/IsoProjection.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Resources/ScenePieceManifest.h"
#include "HMI/Graphics/ComposedScene.h"
#include "HMI/Graphics/PlaceAppearance.h"
#include "HMI/Graphics/WorldSceneComposer.h"

/// Instantané puis composition triée de Martpart, comme le canevas après un coup de pinceau.
static void ComposeMartpart(benchmark::State& state) {
    const std::filesystem::path assets(JADG_ASSETS_DIR);
    const core::LevelLoadResult map = core::LevelLoader::loadFromFile(
        std::filesystem::path(JADG_LEVELS_DIR) / "capital" / "martpart.json");
    const hmi::PlaceAppearanceResult appearance =
        hmi::PlaceAppearance::loadFromFile(assets / "Scene" / "martpart" / "appearance.json");
    const core::ScenePieceManifestResult manifest =
        core::ScenePieceManifest::loadFromFile(assets / "Scene" / "martpart" / "manifest.json");
    if (!map.ok() || !appearance.ok() || !manifest.ok()) {
        state.SkipWithError("Martpart illisible");
        return;
    }
    hmi::ScenePieceTextures textures;
    std::vector<std::uint8_t> identities(manifest.manifest.pieces().size());
    for (std::size_t index = 0; index < identities.size(); ++index) {
        const core::ScenePiece& piece = manifest.manifest.pieces()[index];
        textures.byPath["Scene/martpart/" + piece.file] = hmi::SceneTexture{
            .texture = &identities[index], .width = piece.width, .height = piece.height};
    }
    hmi::ComposedScene scene;
    for (auto _ : state) {
        const hmi::WorldSceneSnapshot snapshot = hmi::snapshotWorldScene(
            *map.level, appearance.appearance, hmi::npcFigures(map.level->entities(), 0));
        scene.clear();
        hmi::composeWorldScene(scene, snapshot,
                               core::IsoProjection(snapshot.columns, snapshot.rows), textures);
        scene.sort();
        benchmark::DoNotOptimize(scene.size());
    }
    state.counters["primitives"] = static_cast<double>(scene.size());
}
BENCHMARK(ComposeMartpart)->Unit(benchmark::kMicrosecond);
