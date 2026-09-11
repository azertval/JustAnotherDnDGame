# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.21)

# VERSION est la **source unique de vérité** du numéro de version : `core::Engine::version()` le
# reçoit par définition de compilation (Source/Core/CMakeLists.txt), et `scripts/build_docs.py`
# l'injecte dans le `PROJECT_NUMBER` de Doxygen à la génération (le Doxyfile versionné ne le porte
# pas). Bumper ici, et nulle part ailleurs, avant de poser le tag `vX.Y.Z`.
project(JustAnotherDnDGame
    VERSION 0.1.0
    DESCRIPTION "RPG 2D en vue de dessus, exploration temps reel et combat tactique d20, en C++/Qt QRhi"
    LANGUAGES CXX)

# --- Standard C++ ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Cohérence de l'environnement MSVC (générateurs hors Visual Studio) ---
# Les générateurs Ninja / Makefiles n'établissent pas l'environnement MSVC : ils héritent des
# variables LIB et INCLUDE du terminal appelant. Or une invite « Developer PowerShell/Command
# Prompt » démarre en **x86** par défaut. Le compilateur sélectionné par CMake est alors x64, mais
# l'éditeur de liens reçoit les bibliothèques x86 : le build échoue sur des centaines de « symbole
# externe non résolu » (LNK2001) que rien ne rattache à la cause réelle. On détecte l'incohérence
# à la configuration, seul endroit où le message peut être compris.
#
# Le générateur Visual Studio est exempt : MSBuild établit son propre environnement.
#
# Deux incohérences distinctes sont possibles, toutes deux silencieuses jusqu'à l'édition de liens :
#  - terminal x86 : CMake sélectionne le compilateur 32 bits et produit une build 32 bits
#    parfaitement cohérente… mais inutilisable, Qt (msvc*_64) et Direct3D 11 étant fournis en x64.
#    Symptôme observé : « Qt6 introuvable », donc aucun exécutable ;
#  - compilateur x64 imposé (cache d'un IDE) mais LIB héritée d'un terminal x86 : des centaines de
#    LNK2001 sans rapport apparent avec la cause.
option(SKIP_MSVC_ENV_CHECK "Désactiver la vérification d'architecture de l'environnement MSVC" OFF)
if(MSVC AND NOT SKIP_MSVC_ENV_CHECK AND NOT CMAKE_GENERATOR MATCHES "Visual Studio")
    set(_msvc_env_hint
        "Corriger en construisant via le script fourni, qui établit lui-même l'environnement :\n"
        "  pwsh scripts/build.ps1\n"
        "Ou en ouvrant un terminal x64 avant CMake (vcvars64.bat, ou « x64 Native Tools Command "
        "Prompt » — la « Developer PowerShell » démarre en x86).\n"
        "Passer -DSKIP_MSVC_ENV_CHECK=ON pour outrepasser cette vérification.")

    if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR
            "Environnement de compilation 32 bits — ce projet cible x64.\n"
            "  Compilateur : ${CMAKE_CXX_COMPILER}\n"
            "Le terminal appelant a été ouvert en x86, et les générateurs hors Visual Studio en "
            "héritent. La build serait cohérente mais inutilisable : Qt (msvc*_64) ne serait pas "
            "trouvé et aucun exécutable ne serait produit.\n"
            ${_msvc_env_hint})
    endif()

    # LIB est la variable que consulte l'éditeur de liens ; les chemins MSVC s'y terminent
    # invariablement par lib/<arch>.
    string(REPLACE "\\" "/" _msvc_lib_env "$ENV{LIB}")
    string(TOLOWER "${_msvc_lib_env}" _msvc_lib_env)
    if(NOT _msvc_lib_env MATCHES "/lib/x64(/|;|$)")
        message(FATAL_ERROR
            "Environnement MSVC incohérent : compilateur x64, bibliothèques non x64.\n"
            "  Compilateur  : ${CMAKE_CXX_COMPILER}\n"
            "  Variable LIB : $ENV{LIB}\n"
            "L'édition de liens échouerait sur des centaines de symboles externes non résolus "
            "(LNK2001) sans rapport apparent avec la cause.\n"
            ${_msvc_env_hint})
    endif()
endif()

# --- Dossiers de sortie ---
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# --- Options ---
option(BUILD_TESTING "Construire les tests (GoogleTest)" ON)
option(ENABLE_WARNINGS_AS_ERRORS "Traiter les avertissements comme des erreurs" ON)
option(ENABLE_ASAN "Activer AddressSanitizer" OFF)
option(ENABLE_PCH "Activer les en-têtes précompilés (Source/pch.h)" ON)

# --- Avertissements (appliqués uniquement à NOS cibles, pas aux dépendances) ---
add_library(project_warnings INTERFACE)
if(MSVC)
    # /external:* : les en-têtes inclus via <...> (STL, DirectX, GoogleTest) sont
    # traités comme externes et n'émettent pas d'avertissement — cohérent avec
    # notre règle « <...> = externe, "..." = projet ».
    #
    # C4702 (unreachable code) est désactivé séparément : c'est un faux positif connu du toolset
    # MSVC 14.44, déclenché lors de l'instanciation de templates Qt (qvariant.h, qjsengine.h) dans
    # NOS unités de compilation — /external:W0 ne le supprime pas car le diagnostic est émis côté
    # instanciation, pas côté parsing du header. Repéré sur le code généré par qmlcachegen pour la
    # logique QML réelle (Main.qml, ScreenProbe.qml, ...), absent des .ui.qml purement déclaratifs
    # de Jadg.Ui. Suivi upstream (non résolu à ce jour) : https://forum.qt.io/topic/164264
    target_compile_options(project_warnings INTERFACE
        /W4 /permissive- /external:anglebrackets /external:W0 /wd4702)
    if(ENABLE_WARNINGS_AS_ERRORS)
        target_compile_options(project_warnings INTERFACE /WX)
    endif()
else()
    target_compile_options(project_warnings INTERFACE -Wall -Wextra -Wpedantic)
    if(ENABLE_WARNINGS_AS_ERRORS)
        target_compile_options(project_warnings INTERFACE -Werror)
    endif()
endif()

# --- Options de compilation transverses (sanitizers) ---
add_library(project_options INTERFACE)
if(ENABLE_ASAN)
    # Portée GLOBALE (add_compile_options/add_link_options), pas seulement sur project_options :
    # sous MSVC, /fsanitize=address instrumente les conteneurs STL (annotate_string,
    # annotate_vector, annotate_optional...), et l'éditeur de liens exige que TOUS les objets liés
    # dans un même binaire partagent la même instrumentation (LNK2038 sinon). GoogleTest est
    # récupéré par FetchContent (add_subdirectory(External) ci-dessous) et ne lie pas
    # project_options : sans portée globale, gtest.lib serait compilé sans le sanitizer et
    # l'édition de liens de tout exécutable de test échouerait (« discordance détectée »).
    if(MSVC)
        add_compile_options(/fsanitize=address)
        target_compile_options(project_options INTERFACE /fsanitize=address)
    else()
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
        target_compile_options(project_options INTERFACE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(project_options INTERFACE -fsanitize=address)
    endif()
endif()

# --- Dépendances tierces (External/) ---
add_subdirectory(External)

# --- Code source ---
add_subdirectory(Source)

# --- Tests ---
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(Source/Test)
endif()
