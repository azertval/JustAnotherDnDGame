# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

# The game application owns the runtime wiring. Main.qml remains the application entry point, but it
# is deliberately part of Jadg.App rather than Jadg.Ui. Design Studio therefore never resolves
# Jadg.Runtime when opening a .ui.qml form.

# AudioEngine is already part of HmiLib. Do not compile it a second time into the executable: doing
# so would duplicate its symbols when the static HMI library is linked.
add_executable(JustAnotherDnDGame
    Game/Main.cpp
    Common/Bootstrap.cpp
)

set(JADG_GAME_UI_IMAGE ${PROJECT_SOURCE_DIR}/Source/Elements/Assets/UI/world-map.jpg)
set_source_files_properties(${JADG_GAME_UI_IMAGE} PROPERTIES QT_RESOURCE_ALIAS assets/world-map.jpg)

set(JADG_APP_QML_FILES
    ../Ui/Main.qml
    ../Ui/Logic/ScreenStack.qml
    ../Ui/Logic/ScreenProbe.qml
    ../Ui/Screens/CharacterSheet.qml
    ../Ui/Screens/Inventory.qml
    ../Ui/Screens/Journal.qml
    ../Ui/Screens/WorldMap.qml
    ../Ui/Screens/Dialogue.qml
    ../Ui/Screens/Merchant.qml
    ../Ui/Screens/GuildBoard.qml
    ../Ui/Screens/CombatHud.qml
    ../Ui/Screens/TeamSheet.qml
    ../Ui/Screens/MainMenu.qml
    ../Ui/Screens/Options.qml
    ../Ui/Screens/Pause.qml
    ../Ui/Screens/Credits.qml
    ../Ui/Screens/GameView.qml
)

# These QML files live outside Source/App. Stable aliases keep the application resource layout
# independent from the physical source tree.
set_source_files_properties(../Ui/Main.qml PROPERTIES QT_RESOURCE_ALIAS Main.qml)
set_source_files_properties(../Ui/Logic/ScreenStack.qml PROPERTIES QT_RESOURCE_ALIAS Logic/ScreenStack.qml)
set_source_files_properties(../Ui/Logic/ScreenProbe.qml PROPERTIES QT_RESOURCE_ALIAS Logic/ScreenProbe.qml)
set_source_files_properties(../Ui/Screens/CharacterSheet.qml PROPERTIES QT_RESOURCE_ALIAS Screens/CharacterSheet.qml)
set_source_files_properties(../Ui/Screens/Inventory.qml PROPERTIES QT_RESOURCE_ALIAS Screens/Inventory.qml)
set_source_files_properties(../Ui/Screens/Journal.qml PROPERTIES QT_RESOURCE_ALIAS Screens/Journal.qml)
set_source_files_properties(../Ui/Screens/WorldMap.qml PROPERTIES QT_RESOURCE_ALIAS Screens/WorldMap.qml)
set_source_files_properties(../Ui/Screens/Dialogue.qml PROPERTIES QT_RESOURCE_ALIAS Screens/Dialogue.qml)
set_source_files_properties(../Ui/Screens/Merchant.qml PROPERTIES QT_RESOURCE_ALIAS Screens/Merchant.qml)
set_source_files_properties(../Ui/Screens/GuildBoard.qml PROPERTIES QT_RESOURCE_ALIAS Screens/GuildBoard.qml)
set_source_files_properties(../Ui/Screens/CombatHud.qml PROPERTIES QT_RESOURCE_ALIAS Screens/CombatHud.qml)
set_source_files_properties(../Ui/Screens/TeamSheet.qml PROPERTIES QT_RESOURCE_ALIAS Screens/TeamSheet.qml)
set_source_files_properties(../Ui/Screens/MainMenu.qml PROPERTIES QT_RESOURCE_ALIAS Screens/MainMenu.qml)
set_source_files_properties(../Ui/Screens/Options.qml PROPERTIES QT_RESOURCE_ALIAS Screens/Options.qml)
set_source_files_properties(../Ui/Screens/Pause.qml PROPERTIES QT_RESOURCE_ALIAS Screens/Pause.qml)
set_source_files_properties(../Ui/Screens/Credits.qml PROPERTIES QT_RESOURCE_ALIAS Screens/Credits.qml)
set_source_files_properties(../Ui/Screens/GameView.qml PROPERTIES QT_RESOURCE_ALIAS Screens/GameView.qml)

# QTP0001 already provides the canonical /qt/qml resource prefix. Keep the aliases above because
# the source tree and the QML module tree intentionally differ.
# QTP0004 extra qmldirs cannot account for QT_RESOURCE_ALIAS paths, so disable them for this
# deliberately flattened/aliased application module.
qt_add_qml_module(JustAnotherDnDGame
    URI Jadg.App
    VERSION 1.0
    NO_GENERATE_EXTRA_QMLDIRS
    IMPORTS
        TARGET JadgUi
        TARGET JadgRuntime
    QML_FILES ${JADG_APP_QML_FILES}
    RESOURCES
        ${JADG_GAME_UI_IMAGE}
)

qt_query_qml_module(JustAnotherDnDGame TYPEINFO JADG_APP_QMLTYPES)
message(STATUS "Jadg.App qmltypes: ${JADG_APP_QMLTYPES}")

# See the equivalent fix in Source/Ui/Jadg/Ui/CMakeLists.txt: `TARGET ... POST_BUILD` only orders
# this after JustAnotherDnDGame's own link step, not after the qmltypes file itself is produced by
# qt_add_qml_module()'s internal custom command. Depend on the actual file path instead so Ninja
# orders it correctly.
add_custom_target(JustAnotherDnDGame_verify_qmltypes ALL
    DEPENDS "${JADG_APP_QMLTYPES}"
    COMMAND ${CMAKE_COMMAND}
        -DMODULE_LABEL="Jadg.App"
        -DEXPECTED_QMLTYPES="${JADG_APP_QMLTYPES}"
        -P "${PROJECT_SOURCE_DIR}/Source/HMI/Jadg/Runtime/VerifyQmlTypes.cmake"
    VERBATIM
)
add_dependencies(JustAnotherDnDGame_verify_qmltypes JustAnotherDnDGame)

target_include_directories(JustAnotherDnDGame PRIVATE ${PROJECT_SOURCE_DIR}/Source)
target_compile_definitions(JustAnotherDnDGame PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    UNICODE
    _UNICODE
    JADG_QML_DEV_IMPORT_PATH="${PROJECT_SOURCE_DIR}/Source/Ui"
)

target_link_libraries(JustAnotherDnDGame PRIVATE
    HmiLib
    JadgUi
    JadgUiplugin
    JadgRuntime
    JadgRuntimeplugin
    Qt6::Quick
    Qt6::Qml
    Qt6::QuickControls2
    Qt6::Gui
    Qt6::GuiPrivate
    Qt6::Multimedia
    xinput
    project_warnings
    project_options
)

if(ENABLE_PCH)
    target_precompile_headers(JustAnotherDnDGame PRIVATE ${PROJECT_SOURCE_DIR}/Source/pch.h)
endif()

# Keep all runtime data beside the game executable. This is a real build target rather than a
# POST_BUILD source mutation, so asset changes are deterministic and visible to the build graph.
add_custom_target(CopyGameData ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/Source/Elements/Localization
        $<TARGET_FILE_DIR:JustAnotherDnDGame>/Localization
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/Source/Elements/Levels
        $<TARGET_FILE_DIR:JustAnotherDnDGame>/Levels
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/Source/Elements/Assets
        $<TARGET_FILE_DIR:JustAnotherDnDGame>/Assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/Source/Elements/Audio
        $<TARGET_FILE_DIR:JustAnotherDnDGame>/Audio
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/Source/Elements/Rpg
        $<TARGET_FILE_DIR:JustAnotherDnDGame>/Rpg
    COMMENT "Copy game data")
add_dependencies(JustAnotherDnDGame CopyGameData)

find_package(Qt6 QUIET COMPONENTS LinguistTools)
if(Qt6LinguistTools_FOUND)
    qt_add_translations(JustAnotherDnDGame
        TS_FILES ${PROJECT_SOURCE_DIR}/Source/Elements/Localization/jadg_en.ts
        RESOURCE_PREFIX "/i18n")
endif()

# The QML module is the application entry point. C++ loads Jadg.App/Main, while the design module
# remains independent and reusable by Qt Design Studio.
