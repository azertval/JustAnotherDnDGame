# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

# Dépendances tierces
#
# Deux stratégies possibles :
#  - FetchContent : la dépendance est téléchargée à la configuration (cas GoogleTest).
#  - Vendored     : la dépendance est copiée dans un sous-dossier de External/ et
#                   ajoutée ici via add_subdirectory().

include(FetchContent)

# --- GoogleTest (framework de test) ---
if(BUILD_TESTING)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.15.2
    )
    # Utilise la même CRT que le reste du projet sous MSVC.
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

# --- nlohmann/json (parsing JSON des fichiers de niveaux) ---
# Header-only, épinglée (EX-NFR-031) ; utilisée par Core (chargement de niveaux).
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)
set(JSON_BuildTests OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(nlohmann_json)

# --- Google Benchmark (mesures de performance, refonte de la CI phase 3) ---
# Seulement avec BUILD_BENCHMARKS : ni le jeu ni les tests n'en dépendent. Ses propres tests et son
# -Werror sont coupés : on mesure notre code, on ne vérifie pas la bibliothèque.
if(BUILD_BENCHMARKS)
    FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG        v1.9.5
        GIT_SHALLOW    TRUE
    )
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(benchmark)
endif()
