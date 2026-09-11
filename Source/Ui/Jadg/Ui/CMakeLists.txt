# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

# Design-time QML module. The CMake file intentionally lives under the URI-matching directory so
# qt_add_qml_module() has the same source and module target path.
set(JADG_UI_QML_FILES
    ../../Theme/Tokens.qml
    ../../Controls/SheetLine.ui.qml
    ../../Controls/SheetBlock.ui.qml
    ../../Controls/SheetList.ui.qml
    ../../Controls/SheetProse.ui.qml
    ../../Controls/SheetPortrait.ui.qml
    ../../Controls/SheetTrack.ui.qml
    ../../Controls/SheetActionBar.ui.qml
    ../../Controls/RpgScreenFrame.ui.qml
    ../../Controls/Cabochon.ui.qml
    ../../Controls/ParchmentFrame.ui.qml
    ../../Controls/TitleBanner.ui.qml
    ../../Controls/FocusFleuron.ui.qml
    ../../Controls/MenuEntry.ui.qml
    ../../Controls/DiagnosticsOverlay.ui.qml
    ../../Screens/CharacterSheetForm.ui.qml
    ../../Screens/InventoryForm.ui.qml
    ../../Screens/JournalForm.ui.qml
    ../../Screens/WorldMapForm.ui.qml
    ../../Screens/DialogueForm.ui.qml
    ../../Screens/MerchantForm.ui.qml
    ../../Screens/GuildBoardForm.ui.qml
    ../../Screens/CombatHudForm.ui.qml
    ../../Screens/TeamSheetForm.ui.qml
    ../../Screens/MainMenuForm.ui.qml
    ../../Screens/OptionsForm.ui.qml
    ../../Screens/PauseForm.ui.qml
    ../../Screens/CreditsForm.ui.qml
    ../../Screens/GameViewForm.ui.qml
)

# The QML files live outside this CMake directory. Keep their source paths relative and assign
# stable resource aliases so the generated QML module remains independent of the filesystem layout.
set_source_files_properties(
    ../../Theme/Tokens.qml
    PROPERTIES
        QT_QML_SINGLETON_TYPE TRUE
        QT_RESOURCE_ALIAS Theme/Tokens.qml
)

set(_jadg_ui_aliases
    "../../Controls/SheetLine.ui.qml;Controls/SheetLine.ui.qml"
    "../../Controls/SheetBlock.ui.qml;Controls/SheetBlock.ui.qml"
    "../../Controls/SheetList.ui.qml;Controls/SheetList.ui.qml"
    "../../Controls/SheetProse.ui.qml;Controls/SheetProse.ui.qml"
    "../../Controls/SheetPortrait.ui.qml;Controls/SheetPortrait.ui.qml"
    "../../Controls/SheetTrack.ui.qml;Controls/SheetTrack.ui.qml"
    "../../Controls/SheetActionBar.ui.qml;Controls/SheetActionBar.ui.qml"
    "../../Controls/RpgScreenFrame.ui.qml;Controls/RpgScreenFrame.ui.qml"
    "../../Controls/Cabochon.ui.qml;Controls/Cabochon.ui.qml"
    "../../Controls/ParchmentFrame.ui.qml;Controls/ParchmentFrame.ui.qml"
    "../../Controls/TitleBanner.ui.qml;Controls/TitleBanner.ui.qml"
    "../../Controls/FocusFleuron.ui.qml;Controls/FocusFleuron.ui.qml"
    "../../Controls/MenuEntry.ui.qml;Controls/MenuEntry.ui.qml"
    "../../Controls/DiagnosticsOverlay.ui.qml;Controls/DiagnosticsOverlay.ui.qml"
    "../../Screens/CharacterSheetForm.ui.qml;Screens/CharacterSheetForm.ui.qml"
    "../../Screens/InventoryForm.ui.qml;Screens/InventoryForm.ui.qml"
    "../../Screens/JournalForm.ui.qml;Screens/JournalForm.ui.qml"
    "../../Screens/WorldMapForm.ui.qml;Screens/WorldMapForm.ui.qml"
    "../../Screens/DialogueForm.ui.qml;Screens/DialogueForm.ui.qml"
    "../../Screens/MerchantForm.ui.qml;Screens/MerchantForm.ui.qml"
    "../../Screens/GuildBoardForm.ui.qml;Screens/GuildBoardForm.ui.qml"
    "../../Screens/CombatHudForm.ui.qml;Screens/CombatHudForm.ui.qml"
    "../../Screens/TeamSheetForm.ui.qml;Screens/TeamSheetForm.ui.qml"
    "../../Screens/MainMenuForm.ui.qml;Screens/MainMenuForm.ui.qml"
    "../../Screens/OptionsForm.ui.qml;Screens/OptionsForm.ui.qml"
    "../../Screens/PauseForm.ui.qml;Screens/PauseForm.ui.qml"
    "../../Screens/CreditsForm.ui.qml;Screens/CreditsForm.ui.qml"
    "../../Screens/GameViewForm.ui.qml;Screens/GameViewForm.ui.qml"
)
foreach(_entry IN LISTS _jadg_ui_aliases)
    string(REPLACE ";" ";" _alias_pair "${_entry}")
    list(GET _alias_pair 0 _source)
    list(GET _alias_pair 1 _alias)
    set_source_files_properties(${_source} PROPERTIES QT_RESOURCE_ALIAS "${_alias}")
endforeach()

add_library(JadgUi STATIC)
qt_add_qml_module(JadgUi
    URI Jadg.Ui
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    CLASS_NAME JadgUiPlugin
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
