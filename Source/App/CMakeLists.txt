# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

# The game application owns the runtime wiring.  Its entry point remains Main.qml, but Main.qml is
# deliberately not part of Jadg.Ui: Design Studio must never have to resolve Jadg.Runtime.

add_executable(JustAnotherDnDGame
    Game/Main.cpp
    Common/Bootstrap.cpp
    Game/GameViewportItem.cpp
    Audio/AudioEngine.cpp
    project_placeholder.cpp
)

# Replace the placeholder source with the real C++ QML-facing models.  They are compiled only by
# Jadg.Runtime, never by the executable itself; the runtime module is linked below.
set_source_files_properties(project_placeholder.cpp PROPERTIES HEADER_FILE_ONLY TRUE)

qt_add_qml_module(JustAnotherDnDGame
    URI Jadg.App
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
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
)

target_include_directories(JustAnotherDnDGame PRIVATE ${PROJECT_SOURCE_DIR}/Source)
target_compile_definitions(JustAnotherDnDGame PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    UNICODE
    _UNICODE
)

target_link_libraries(JustAnotherDnDGame PRIVATE
    HmiLib
    JadgUi
    JadgRuntime
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

# Keep source-mode development independent of generated CMake qmldirs.  The runtime QML module is
# still supplied by the built JadgRuntime target, while Jadg.Ui can be resolved from Source/Ui.
target_compile_definitions(JustAnotherDnDGame PRIVATE
    JADG_QML_SOURCE_ROOT="${PROJECT_SOURCE_DIR}/Source/Ui"
)

# The QML module is deliberately the application entry point.  C++ loads Jadg.App/Main rather than
# reaching into Jadg.Ui, so the design module remains reusable by Qt Design Studio.
