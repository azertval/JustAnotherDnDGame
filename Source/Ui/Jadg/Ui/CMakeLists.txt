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
set_source_files_properties(../../Theme/Tokens.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
    QT_RESOURCE_ALIAS Theme/Tokens.qml
)
set_source_files_properties(../../Controls/SheetLine.ui.qml PROPERTIES QT_RESOURCE_ALIAS Controls/SheetLine.ui.qml)
set_source_files_properties(../../Controls/SheetBlock.ui.qml PROPERTIES QT_RESOURCE_ALIAS Controls/SheetBlock.ui.qml)
set_source_files_properties(../../Controls/SheetList.ui.qml PROPERTIES QT_RESOURCE_ALIAS Controls/SheetList.ui.qml)
set_source_files_properties(../../Controls/SheetProse.ui.qml PROPERTIES QT_RESOURCE_ALIAS Controls/SheetProse.ui.qml)
set_source_files_properties(../../Controls/SheetPortrait.ui.qml PROPERTIES QT_RESOURCE_ALIAS Controls/SheetPortrait.ui.qml)
set_source_files_properties(../../Controls/SheetTrack.ui.qml PROPERTIES QT_RESOURCE_ALIAS Controls/SheetTrack.ui.qml)
set_source_files_properties(../../Controls/SheetActionBar.ui.qml PROPERTIES QT_RESOURCE_ALIAS Controls/SheetActionBar.ui.qml)
set_source_files_properties(../../Controls/RpgScreenFrame.ui.qml PROPERTIES QT_RESOURCE_ALIAS Controls/RpgScreenFrame.ui.qml)
set_source_files_properties(../../Controls/Cabochon.ui.qml PROPERTIES QT_RESOURCE_ALIAS Controls/Cabochon.ui.qml)
set_source_files_properties(../../Controls/ParchmentFrame.ui.qml PROPERTIES QT_RESOURCE_ALIAS Controls/ParchmentFrame.ui.qml)
set_source_files_properties(../../Controls/TitleBanner.ui.qml PROPERTIES QT_RESOURCE_ALIAS Controls/TitleBanner.ui.qml)
set_source_files_properties(../../Controls/FocusFleuron.ui.qml PROPERTIES QT_RESOURCE_ALIAS Controls/FocusFleuron.ui.qml)
set_source_files_properties(../../Controls/MenuEntry.ui.qml PROPERTIES QT_RESOURCE_ALIAS Controls/MenuEntry.ui.qml)
set_source_files_properties(../../Controls/DiagnosticsOverlay.ui.qml PROPERTIES QT_RESOURCE_ALIAS Controls/DiagnosticsOverlay.ui.qml)
set_source_files_properties(../../Screens/CharacterSheetForm.ui.qml PROPERTIES QT_RESOURCE_ALIAS Screens/CharacterSheetForm.ui.qml)
set_source_files_properties(../../Screens/InventoryForm.ui.qml PROPERTIES QT_RESOURCE_ALIAS Screens/InventoryForm.ui.qml)
set_source_files_properties(../../Screens/JournalForm.ui.qml PROPERTIES QT_RESOURCE_ALIAS Screens/JournalForm.ui.qml)
set_source_files_properties(../../Screens/WorldMapForm.ui.qml PROPERTIES QT_RESOURCE_ALIAS Screens/WorldMapForm.ui.qml)
set_source_files_properties(../../Screens/DialogueForm.ui.qml PROPERTIES QT_RESOURCE_ALIAS Screens/DialogueForm.ui.qml)
set_source_files_properties(../../Screens/MerchantForm.ui.qml PROPERTIES QT_RESOURCE_ALIAS Screens/MerchantForm.ui.qml)
set_source_files_properties(../../Screens/GuildBoardForm.ui.qml PROPERTIES QT_RESOURCE_ALIAS Screens/GuildBoardForm.ui.qml)
set_source_files_properties(../../Screens/CombatHudForm.ui.qml PROPERTIES QT_RESOURCE_ALIAS Screens/CombatHudForm.ui.qml)
set_source_files_properties(../../Screens/TeamSheetForm.ui.qml PROPERTIES QT_RESOURCE_ALIAS Screens/TeamSheetForm.ui.qml)
set_source_files_properties(../../Screens/MainMenuForm.ui.qml PROPERTIES QT_RESOURCE_ALIAS Screens/MainMenuForm.ui.qml)
set_source_files_properties(../../Screens/OptionsForm.ui.qml PROPERTIES QT_RESOURCE_ALIAS Screens/OptionsForm.ui.qml)
set_source_files_properties(../../Screens/PauseForm.ui.qml PROPERTIES QT_RESOURCE_ALIAS Screens/PauseForm.ui.qml)
set_source_files_properties(../../Screens/CreditsForm.ui.qml PROPERTIES QT_RESOURCE_ALIAS Screens/CreditsForm.ui.qml)
set_source_files_properties(../../Screens/GameViewForm.ui.qml PROPERTIES QT_RESOURCE_ALIAS Screens/GameViewForm.ui.qml)

add_library(JadgUi STATIC)

# The files are intentionally collected from sibling source directories and exposed through
# QT_RESOURCE_ALIAS. QTP0004's extra qmldirs do not understand those aliases, so disable only that
# generation here while retaining the normal module qmldir and qmltypes generation.
qt_add_qml_module(JadgUi
    URI Jadg.Ui
    VERSION 1.0
    CLASS_NAME JadgUiPlugin
    DESIGNER_SUPPORTED
    NO_GENERATE_EXTRA_QMLDIRS
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
