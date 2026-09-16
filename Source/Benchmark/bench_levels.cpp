// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file bench_levels.cpp
 * @brief Mesure du chargement d'un niveau livré, du texte JSON au niveau validé.
 *
 * Le fichier est lu une fois hors de la boucle : c'est l'analyse et la validation qu'on mesure, pas
 * le disque du runner.
 */

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#include <benchmark/benchmark.h>

#include "Core/Levels/LevelLoader.h"

/// Analyse et validation de `arena-of-the-future.json`.
static void LoadShippedLevel(benchmark::State& state) {
    std::ifstream fichier(std::filesystem::path(JADG_LEVELS_DIR) / "arena-of-the-future.json",
                          std::ios::binary);
    const std::string texte{std::istreambuf_iterator<char>(fichier),
                            std::istreambuf_iterator<char>()};
    if (texte.empty()) {
        state.SkipWithError("arena-of-the-future.json illisible");
        return;
    }
    for (auto _ : state) {
        benchmark::DoNotOptimize(core::LevelLoader::loadFromString(texte));
    }
    state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(texte.size()));
}
BENCHMARK(LoadShippedLevel);
