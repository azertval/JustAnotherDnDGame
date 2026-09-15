# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

# Cibles libFuzzer des lecteurs de données (refonte de la CI, phase 3).
#
# Chaque exécutable passe à un lecteur des octets arbitraires, des millions de fois, en mutant ceux
# qui ouvrent de la couverture nouvelle. Les 600 JSON du dépôt sont lus par du C++ qui promet de ne
# jamais lever ; le fuzzer cherche le fichier qui fait mentir cette promesse : exception qui
# s'échappe, lecture hors bornes (AddressSanitizer), allocation démesurée. Exécutées la nuit par le
# job `fuzz` de .github/workflows/nightly.yml, jamais en PR.
#
# Construites uniquement avec -DBUILD_FUZZERS=ON sous clang-cl (garde du CMakeLists racine).

# CMake lie par lld-link/link.exe directement, pas par le pilote clang-cl : les bibliothèques que
# `clang-cl -fsanitize=fuzzer,address` ajouterait seul à l'édition de liens doivent donc être
# nommées ici, avec les mêmes options que le pilote (MSVCToolChain de LLVM 20). Elles vivent dans le
# dossier des runtimes du compilateur, que clang-cl indique lui-même.
execute_process(
    COMMAND "${CMAKE_CXX_COMPILER}" /clang:-print-runtime-dir
    OUTPUT_VARIABLE _jadg_clang_runtime_dir
    OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(
    COMMAND "${CMAKE_CXX_COMPILER}" /clang:-print-resource-dir
    OUTPUT_VARIABLE _jadg_clang_resource_dir
    OUTPUT_STRIP_TRAILING_WHITESPACE)
set(_jadg_clang_rt_hints "${_jadg_clang_runtime_dir}" "${_jadg_clang_resource_dir}/lib/windows")

find_library(JADG_CLANG_RT_FUZZER
    NAMES clang_rt.fuzzer-x86_64 clang_rt.fuzzer
    HINTS ${_jadg_clang_rt_hints} NO_DEFAULT_PATH)
find_library(JADG_CLANG_RT_ASAN
    NAMES clang_rt.asan_dynamic-x86_64 clang_rt.asan_dynamic
    HINTS ${_jadg_clang_rt_hints} NO_DEFAULT_PATH)
find_library(JADG_CLANG_RT_ASAN_THUNK
    NAMES clang_rt.asan_dynamic_runtime_thunk-x86_64 clang_rt.asan_dynamic_runtime_thunk
    HINTS ${_jadg_clang_rt_hints} NO_DEFAULT_PATH)
find_file(JADG_CLANG_RT_ASAN_DLL
    NAMES clang_rt.asan_dynamic-x86_64.dll clang_rt.asan_dynamic.dll
    HINTS ${_jadg_clang_rt_hints} "${_jadg_clang_resource_dir}/../../../bin" NO_DEFAULT_PATH)

foreach(_jadg_rt JADG_CLANG_RT_FUZZER JADG_CLANG_RT_ASAN JADG_CLANG_RT_ASAN_THUNK JADG_CLANG_RT_ASAN_DLL)
    if(NOT ${_jadg_rt})
        message(FATAL_ERROR
            "${_jadg_rt} introuvable dans : ${_jadg_clang_rt_hints}\n"
            "L'installation LLVM de ${CMAKE_CXX_COMPILER} ne fournit pas les runtimes libFuzzer/ASan.")
    endif()
endforeach()

# Données d'amorce : le fuzzer part de fichiers réels plutôt que de rien, et atteint ainsi dès les
# premières secondes le code qui suit l'enveloppe JSON. Le job de nuit copie ces dossiers dans le
# corpus qu'il conserve d'une nuit à l'autre.
set(JADG_FUZZ_SEEDS_json "${PROJECT_SOURCE_DIR}/Source/Test/Fixtures/Json;${PROJECT_SOURCE_DIR}/Source/Elements/Rpg/rules")
set(JADG_FUZZ_SEEDS_level "${PROJECT_SOURCE_DIR}/Source/Elements/Levels")
set(JADG_FUZZ_SEEDS_dialogue "${PROJECT_SOURCE_DIR}/Source/Elements/World/dialogues")
set(JADG_FUZZ_SEEDS_skins "${PROJECT_SOURCE_DIR}/Source/Elements/Assets")
set(JADG_FUZZ_SEEDS_localization "${PROJECT_SOURCE_DIR}/Source/Elements/Localization")

function(jadg_add_fuzzer name)
    add_executable(fuzz_${name} fuzz_${name}.cpp)
    target_link_libraries(fuzz_${name} PRIVATE ${ARGN} project_warnings)
    # `fuzzer` (et non plus `fuzzer-no-link`) : le harnais fournit LLVMFuzzerTestOneInput, libFuzzer
    # fournit main.
    target_compile_options(fuzz_${name} PRIVATE -fsanitize=fuzzer)
    target_link_options(fuzz_${name} PRIVATE
        "/WHOLEARCHIVE:${JADG_CLANG_RT_FUZZER}"
        "${JADG_CLANG_RT_ASAN}"
        "/INCLUDE:__asan_seh_interceptor"
        "/WHOLEARCHIVE:${JADG_CLANG_RT_ASAN_THUNK}"
        /DEBUG /INCREMENTAL:NO)
    # La DLL d'ASan à côté de l'exécutable : il se lance sans toucher au PATH.
    add_custom_command(TARGET fuzz_${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${JADG_CLANG_RT_ASAN_DLL}" "$<TARGET_FILE_DIR:fuzz_${name}>")
    string(REPLACE ";" "\n" _jadg_seeds "${JADG_FUZZ_SEEDS_${name}}")
    file(WRITE "${CMAKE_BINARY_DIR}/fuzz/${name}.seeds" "${_jadg_seeds}\n")
endfunction()

jadg_add_fuzzer(json Core)
jadg_add_fuzzer(level Core)
jadg_add_fuzzer(dialogue Core)
jadg_add_fuzzer(skins HmiLib)
jadg_add_fuzzer(localization HmiLib)
