# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

# HMI owns the reusable non-QML presentation library and the standalone level editor.
# The game executable is intentionally elsewhere (Source/App) so that Jadg.Ui and Jadg.Runtime
# have clear consumers and no target needs to mutate another target after creation.

add_library(HmiLib STATIC
    Graphics/Camera2D.cpp
    Graphics/RoomGrid.cpp
    Graphics/FollowCamera.cpp
    Graphics/CameraZones.cpp
    Graphics/AssetPaths.cpp
    Graphics/TileVisuals.cpp
    Graphics/ProceduralAtlas.cpp
    Graphics/TextureAtlasRegions.cpp
    Graphics/AssetContract.cpp
    Graphics/RenderMode.cpp
    Graphics/TileAppearance.cpp
    Graphics/SkinCatalog.cpp
    Graphics/AnimationCatalog.cpp
    Graphics/PlayerSprite.cpp
    Graphics/TileAutotile.cpp
    Graphics/TileSilhouette.cpp
    Graphics/MissingTexture.cpp
    Graphics/ComposedScene.cpp
    Graphics/QuadRecorder.cpp
    Graphics/BackgroundRenderer.cpp
    Graphics/ShadowRenderer.cpp
    Graphics/ParticleRenderer.cpp
    Graphics/MechanismVisuals.cpp
    Graphics/PlaneVisuals.cpp
    Graphics/Parallax.cpp
    Graphics/ProceduralFont.cpp
    Graphics/TextRenderer.cpp
    Input/InputState.cpp
    Input/PlayerInputMapper.cpp
    Input/GameKeyBindings.cpp
    Input/EditorKeyBindings.cpp
    Input/KeyName.cpp
    Input/GamepadBindings.cpp
    Input/GamepadButtonName.cpp
    Localization/Localization.cpp
    Diagnostics/SessionLog.cpp
    Audio/SoundCatalog.cpp
    Audio/SoundTriggers.cpp
    Game/GameEvents.cpp
    Game/ExplorationMode.cpp
    Game/CombatMode.cpp
    Game/GameHud.cpp
    Game/DiagnosticsHud.cpp
    Editor/LevelNameValidation.cpp
    Editor/TileTaxonomy.cpp
    Editor/TaxonomyLabels.cpp
    Editor/SkinAssignments.cpp
    Editor/PaletteAppearance.cpp
    Editor/LevelFileOperations.cpp
    Editor/LinkGeometry.cpp
    Editor/LinkGesture.cpp
    Editor/TextureAssignGesture.cpp
    Editor/AssetLibrary.cpp
    Editor/AssetFileOperations.cpp
    Editor/AssetReferences.cpp
    Editor/MechanismAnimationAssignments.cpp
    Editor/EditorStatus.cpp
    Editor/PanelFocus.cpp
    Editor/ThumbnailGeometry.cpp
    Editor/PixelOperations.cpp
    Editor/PixelHistory.cpp
    Editor/PixelCanvasGeometry.cpp
    Editor/PlaneReference.cpp
    Editor/PlaneFileNaming.cpp
    Editor/PixelAssetIO.cpp
    Editor/PixelPalette.cpp
    Editor/PixelAutotilePreview.cpp
    Presentation/ScreenFlow.cpp
    Presentation/MenuBackdropGeometry.cpp
    Presentation/KeyHintText.cpp
    Presentation/RpgScreens.cpp
    Presentation/CharacterSheetValues.cpp
    Presentation/InventoryValues.cpp
    Interface/DesignTokens.cpp
    Interface/StyleSheetTemplate.cpp
    Interface/FontResolution.cpp
    Interface/ThemeResolution.cpp
    Interface/IconGeometry.cpp
    Presentation/IdentityScale.cpp
    Interface/EditorWorkspace.cpp
    Interface/ActionCatalog.cpp
    Platform/ExecutableDirectory.cpp
)

target_include_directories(HmiLib PUBLIC ${PROJECT_SOURCE_DIR}/Source)
target_link_libraries(HmiLib
    PUBLIC Core nlohmann_json::nlohmann_json
    PRIVATE project_warnings project_options
)
target_compile_definitions(HmiLib PUBLIC WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)

if(ENABLE_PCH)
    target_precompile_headers(HmiLib PRIVATE ${PROJECT_SOURCE_DIR}/Source/pch.h)
endif()

option(BUILD_EDITOR_QT "Build the standalone Qt level editor" ON)
if(NOT BUILD_EDITOR_QT)
    return()
endif()

set(QT_VERSION_MINIMUM "6.8.7")
set(_qt_hints "")
foreach(_qt_var QT_ROOT_DIR Qt6_ROOT QTDIR)
    if(DEFINED ENV{${_qt_var}})
        list(APPEND _qt_hints "$ENV{${_qt_var}}")
    endif()
endforeach()

if(WIN32)
    file(GLOB _qt_installs "$ENV{SystemDrive}/Qt/*/msvc*_64" "C:/Qt/*/msvc*_64" "D:/Qt/*/msvc*_64")
else()
    file(GLOB _qt_installs "$ENV{HOME}/Qt/*/gcc_64" "/opt/Qt/*/gcc_64")
endif()
list(SORT _qt_installs COMPARE NATURAL ORDER DESCENDING)
list(APPEND _qt_hints ${_qt_installs})
if(_qt_hints)
    list(REMOVE_DUPLICATES _qt_hints)
    list(APPEND CMAKE_PREFIX_PATH ${_qt_hints})
endif()

find_package(Qt6 ${QT_VERSION_MINIMUM} REQUIRED COMPONENTS
    Widgets Gui GuiPrivate Multimedia ShaderTools Quick Qml QuickControls2)

# Set only policies known by the Qt installation. In particular, do not request a newer QTP policy
# unconditionally: Qt 6.8.7 is the minimum supported version for this compatibility branch.
if(COMMAND qt_policy AND QT_KNOWN_POLICY_QTP0004)
    qt_policy(SET QTP0004 NEW)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(ELEMENTS_UI ${PROJECT_SOURCE_DIR}/Source/Elements/UI)

add_executable(LevelEditor
    ${PROJECT_SOURCE_DIR}/Source/App/Editor/Main.cpp
    ${PROJECT_SOURCE_DIR}/Source/App/Common/Bootstrap.cpp
    Input/GamepadPoller.cpp
    Input/QtKeyMap.cpp
    Graphics/SpriteBatch.cpp
    Graphics/SceneResources.cpp
    Graphics/TextureLoader.cpp
    Graphics/TextureAtlas.cpp
    Graphics/TextureCache.cpp
    Graphics/SpriteRenderer.cpp
    Graphics/DraftRenderer.cpp
    Graphics/BitmapFont.cpp
    Game/GameSession.cpp
    Game/GameViewport.cpp
    Audio/AudioEngine.cpp
    Interface/ApplicationTheme.cpp
    Interface/ThemeIcons.cpp
    Interface/EditorActions.cpp
    Interface/MainWindow.cpp
    Interface/KeybindingsWidget.cpp
    Interface/EditorKeybindingsWidget.cpp
    Interface/GamepadBindingsWidget.cpp
    Editor/PalettePanel.cpp
    Editor/LevelBrowserPanel.cpp
    Editor/LinkPanel.cpp
    Editor/TexturePanel.cpp
    Editor/AssetThumbnailView.cpp
    Editor/PixelCanvas.cpp
    Editor/PixelHistoryPanel.cpp
    Editor/PixelPalettePanel.cpp
    Editor/PlanesPanel.cpp
    ${ELEMENTS_UI}/resources.qrc
    ${ELEMENTS_UI}/MainWindow.ui
    ${ELEMENTS_UI}/TexturePanel.ui
    ${ELEMENTS_UI}/PlanesPanel.ui
    ${ELEMENTS_UI}/LevelBrowserPanel.ui
    ${ELEMENTS_UI}/AssetThumbnailView.ui
)

set_target_properties(LevelEditor PROPERTIES AUTOUIC_SEARCH_PATHS ${ELEMENTS_UI})
target_include_directories(LevelEditor PRIVATE ${PROJECT_SOURCE_DIR}/Source)
target_compile_definitions(LevelEditor PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)
target_link_libraries(LevelEditor PRIVATE
    HmiLib Qt6::Widgets Qt6::Gui Qt6::GuiPrivate Qt6::Multimedia xinput
    project_warnings project_options)

if(ENABLE_PCH)
    target_precompile_headers(LevelEditor PRIVATE ${PROJECT_SOURCE_DIR}/Source/pch.h)
endif()

qt6_add_shaders(LevelEditor jadg_shaders
    PREFIX "/shaders"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}/Graphics/Shaders"
    FILES Graphics/Shaders/sprite.vert Graphics/Shaders/sprite.frag)
