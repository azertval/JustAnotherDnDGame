# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

# The game application owns the runtime wiring. Main.qml remains the application entry point, but it
# is deliberately part of Jadg.App rather than Jadg.Ui. Design Studio therefore never resolves
# Jadg.Runtime when opening a .ui.qml form.

add_executable(JustAnotherDnDGame
    Game/Main.cpp
    Common/Bootstrap.cpp
    Audio/AudioEngine.cpp
)

set(JADG_GAME_UI_IMAGE ${PROJECT_SOURCE_DIR}/Source/Elements/Assets/UI/world-map.jpg)
set_source_files_properties(${JADG_GAME_UI_IMAGE} PROPERTIES QT_RESOURCE_ALIAS assets/world-map.jpg)

qt_add_qml_module(JustAnotherDnDGame
    URI Jadg.App
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    IMPORTS
        Jadg.Ui
        Jadg.Runtime
    QML_FILES
        ${PROJECT_SOURCE_DIR}/Source/Ui/Main.qml
        ${PROJECT_SOURCE_DIR}/Source/Ui/Logic/ScreenStack.qml
        ${PROJECT_SOURCE_DIR}/Source/Ui/Logic/ScreenProbe.qml
        ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/CharacterSheet.qml
        ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/Inventory.qml
        ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/Journal.qml
        ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/WorldMap.qml
        ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/Dialogue.qml
        ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/Merchant.qml
        ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/GuildBoard.qml
        ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/CombatHud.qml
        ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/TeamSheet.qml
        ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/MainMenu.qml
        ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/Options.qml
        ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/Pause.qml
        ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/Credits.qml
        ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/GameView.qml
    RESOURCES
        ${JADG_GAME_UI_IMAGE}
)

qt_query_qml_module(JustAnotherDnDGame TYPEINFO JADG_APP_QMLTYPES)
message(STATUS "Jadg.App qmltypes: ${JADG_APP_QMLTYPES}")
add_custom_command(TARGET JustAnotherDnDGame POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -DEXPECTED_QMLTYPES="${JADG_APP_QMLTYPES}"
        -P "${PROJECT_SOURCE_DIR}/Source/HMI/Jadg/Runtime/VerifyQmlTypes.cmake"
    VERBATIM
)

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
