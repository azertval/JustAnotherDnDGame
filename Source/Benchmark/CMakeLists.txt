# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

# Mesures de performance (refonte de la CI, phase 3).
#
# Le déplacement, la ligne de vue et l'IA tactique sont des algorithmes dont le coût peut doubler
# sans qu'aucun test ne rougisse. Le job `benchmarks` de .github/workflows/nightly.yml exécute ces
# mesures en Release chaque nuit et en conserve l'historique ; une régression au-delà du seuil y est
# signalée. Construites uniquement avec -DBUILD_BENCHMARKS=ON, sans Qt.
#
# Rejouer sur le poste (Release, sinon les chiffres ne disent rien) :
#   cmake -S . -B build/bench -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARKS=ON
#         -DBUILD_TESTING=OFF -DBUILD_EDITOR_QT=OFF
#   cmake --build build/bench --target Benchmarks
#   build/bench/bin/Benchmarks.exe

add_executable(Benchmarks
    bench_combat.cpp
    bench_levels.cpp
)
target_link_libraries(Benchmarks PRIVATE Core benchmark::benchmark_main project_warnings)
target_compile_definitions(Benchmarks PRIVATE
    JADG_RPG_RULES_DIR="${PROJECT_SOURCE_DIR}/Source/Elements/Rpg/rules"
    JADG_LEVELS_DIR="${PROJECT_SOURCE_DIR}/Source/Elements/Levels")
