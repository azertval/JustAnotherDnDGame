# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

# Bibliothèque Core : logique et moteur du jeu.
add_library(Core STATIC
    Core.cpp
    Data/JsonDocument.cpp
    Math/Vector2.cpp
    Math/Rect.cpp
    Ecs/EntityManager.cpp
    Ecs/World.cpp
    Ecs/AnimationClip.cpp
    Ecs/Systems/MovementSystem.cpp
    Ecs/Systems/TopDownMovementSystem.cpp
    Ecs/Systems/AnimationSystem.cpp
    Ecs/Systems/ParticleSystem.cpp
    World/GridDistanceField.cpp
    Gameplay/MechanismController.cpp
    Rpg/RpgEnumNames.cpp
    Rpg/Multiclassing.cpp
    Levels/TileMap.cpp
    Levels/TileTypeName.cpp
    Levels/CameraFraming.cpp
    Levels/LevelLoader.cpp
    Levels/LevelSequence.cpp
    Levels/LevelWriter.cpp
    Levels/LevelDraft.cpp
    Levels/LevelScene.cpp
    Levels/LevelOutcome.cpp
    Physics/SweptCollision.cpp
    Physics/AabbVsAabb.cpp
    Time/FixedTimestep.cpp
    Diagnostics/LogFormat.cpp
    Diagnostics/Logger.cpp
    Diagnostics/ConsoleLogSink.cpp
    Diagnostics/FileLogSink.cpp
    Diagnostics/Assert.cpp
)

# Racine d'inclusion = Source/ : les en-têtes s'incluent par chemin complet
# depuis la racine (ex. "Core/Core.h"), jamais par nom seul.
target_include_directories(Core PUBLIC ${PROJECT_SOURCE_DIR}/Source)

# Numéro de version : injecté depuis `project(... VERSION ...)`, seul endroit où il est écrit.
# PRIVATE suffit — Core.cpp est l'unique consommateur, et un en-tête généré obligerait tous les
# dépendants à connaître un répertoire d'inclusion supplémentaire pour une seule chaîne.
target_compile_definitions(Core PRIVATE JADG_VERSION="${PROJECT_VERSION}")

# `nlohmann_json` est PUBLIC depuis le LOT-79 : `Core/Data/JsonDocument.h` expose l'arbre parse a
# ses appelants, donc la bibliotheque fait partie de l'interface de Core et non de son
# implementation. C'est le prix -- assume -- d'une brique de lecture unique : la seule facon de
# l'eviter serait de reecrire une facade typee par-dessus nlohmann, soit un second modele d'arbre
# a maintenir pour ne rien gagner d'observable.
target_link_libraries(Core PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(Core PRIVATE project_warnings project_options)

if(ENABLE_PCH)
    target_precompile_headers(Core PRIVATE ${PROJECT_SOURCE_DIR}/Source/pch.h)
endif()
