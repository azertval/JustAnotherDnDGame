# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

# Design-time QML module. The CMake file intentionally lives under the URI-matching directory so
# qt_add_qml_module() has the same source and module target path.

set(JADG_UI_QML_FILES
    ${PROJECT_SOURCE_DIR}/Source/Ui/Theme/Tokens.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Controls/SheetLine.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Controls/SheetBlock.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Controls/SheetList.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Controls/SheetProse.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Controls/SheetPortrait.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Controls/SheetTrack.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Controls/SheetActionBar.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Controls/RpgScreenFrame.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Controls/Cabochon.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Controls/ParchmentFrame.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Controls/TitleBanner.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Controls/FocusFleuron.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Controls/MenuEntry.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Controls/DiagnosticsOverlay.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/CharacterSheetForm.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/InventoryForm.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/JournalForm.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/WorldMapForm.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/DialogueForm.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/MerchantForm.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/GuildBoardForm.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/CombatHudForm.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/TeamSheetForm.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/MainMenuForm.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/OptionsForm.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/PauseForm.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/CreditsForm.ui.qml
    ${PROJECT_SOURCE_DIR}/Source/Ui/Screens/GameViewForm.ui.qml
)

set_source_files_properties(
    ${PROJECT_SOURCE_DIR}/Source/Ui/Theme/Tokens.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)

add_library(JadgUi STATIC)
qt_add_qml_module(JadgUi
    URI Jadg.Ui
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    NO_PLUGIN
    DESIGNER_SUPPORTED
    QML_FILES ${JADG_UI_QML_FILES}
)

target_link_libraries(JadgUi
    PRIVATE
        Qt6::Quick
        Qt6::Qml
        project_warnings
        project_options
)

qt_query_qml_module(JadgUi TYPEINFO JADG_UI_QMLTYPES)
message(STATUS "Jadg.Ui qmltypes: ${JADG_UI_QMLTYPES}")
add_custom_command(TARGET JadgUi POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -DEXPECTED_QMLTYPES="${JADG_UI_QMLTYPES}"
        -P "${PROJECT_SOURCE_DIR}/Source/HMI/Jadg/Runtime/VerifyQmlTypes.cmake"
    VERBATIM
)
