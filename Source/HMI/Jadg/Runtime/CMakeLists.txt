# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

# Module QML runtime : les types C++ consommes par les fichiers de cablage `.qml`.
#
# Ce module est volontairement separe de Jadg.Ui :
#   - Jadg.Ui       = composants et formulaires editables dans Qt Design Studio ;
#   - Jadg.Runtime  = types C++ generes/enregistres par qt_add_qml_module.
#
# Le target de l'application est cree par ../CMakeLists.txt avant l'arrivee ici. Les sources C++
# runtime etaient historiquement declarees directement dans l'executable ; elles sont retirees de
# celui-ci avant que qt_add_qml_module ne les ajoute au backing target du module. Cela evite une
# double compilation des Q_OBJECT/QML_ELEMENT tout en gardant l'executable et son module dans le
# meme build.

set(JADG_RUNTIME_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/../../Game/GameViewportItem.h
    ${CMAKE_CURRENT_LIST_DIR}/../../Game/GameViewportItem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../Presentation/CharacterSheetModel.h
    ${CMAKE_CURRENT_LIST_DIR}/../../Presentation/CharacterSheetModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../Presentation/InventoryModel.h
    ${CMAKE_CURRENT_LIST_DIR}/../../Presentation/InventoryModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../Presentation/OptionsModel.h
    ${CMAKE_CURRENT_LIST_DIR}/../../Presentation/OptionsModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../Presentation/PendingData.h
    ${CMAKE_CURRENT_LIST_DIR}/../../Presentation/PendingData.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../Presentation/ScreenRouter.h
    ${CMAKE_CURRENT_LIST_DIR}/../../Presentation/ScreenRouter.cpp
)

qt_add_qml_module(JadgRuntime
    URI Jadg.Runtime
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    SOURCES ${JADG_RUNTIME_SOURCES}
    DEPENDENCIES QtQml QtQuick
)

target_include_directories(JadgRuntime PUBLIC ${PROJECT_SOURCE_DIR}/Source)
target_link_libraries(JadgRuntime
    PRIVATE
        HmiLib
        Core
        Qt6::Qml
        Qt6::Quick
        Qt6::GuiPrivate
        project_warnings
        project_options
)

# Les fichiers runtime ne doivent plus etre compiles une seconde fois dans l'executable.
get_target_property(_jadg_game_sources JustAnotherDnDGame SOURCES)
foreach(_runtime_cpp IN ITEMS
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Game/GameViewportItem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Presentation/CharacterSheetModel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Presentation/InventoryModel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Presentation/OptionsModel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Presentation/PendingData.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Presentation/ScreenRouter.cpp"
    "Game/GameViewportItem.cpp"
    "Presentation/CharacterSheetModel.cpp"
    "Presentation/InventoryModel.cpp"
    "Presentation/OptionsModel.cpp"
    "Presentation/PendingData.cpp"
    "Presentation/ScreenRouter.cpp"
)
    list(REMOVE_ITEM _jadg_game_sources "${_runtime_cpp}")
endforeach()
set_property(TARGET JustAnotherDnDGame PROPERTY SOURCES "${_jadg_game_sources}")

# Le plugin du module est lie a l'application. C'est le modele recommande par Qt pour un module
# C++ supplementaire : le backing target porte les types et le plugin apporte l'enregistrement QML.
target_link_libraries(JustAnotherDnDGame PRIVATE JadgRuntimeplugin)
