# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

# Jadg.Ui : les écrans du jeu, en QML déclaratif -- le territoire de la conception (LOT-87).
#
# Ce module ne contient PAS UNE LIGNE DE C++, et ce n'est pas un état de fait mais sa raison d'être.
# Qt Design Studio dessine avec son propre Qt (6.8.7, embarqué) et un marionnettiste (`qmlpuppet`)
# qui ne charge aucun plugin C++ du projet. Un module qui mêle formulaires et types C++ est donc
# résolvable par le jeu et pas par l'atelier ; un module de QML pur l'est par les deux, tel quel.
#
# Ce fichier vit DANS Source/Ui, à côté des fichiers qu'il liste. `qt_add_qml_module` calcule le
# chemin de ressource de chaque fichier relativement à ce répertoire : listé d'ailleurs, chaque
# chemin aurait dû être réécrit un par un (par un alias de ressource), et c'est cette plomberie qui
# a coûté 125 commits à la branche abandonnée. Aucun alias ici, ni nulle part.
#
# DESIGNER_SUPPORTED : sans ce mot dans le `qmldir`, Qt Design Studio ouvre les fichiers mais laisse
# la bibliothèque de composants VIDE -- chaque dossier y est listé « (vide) » -- et l'artiste ne
# peut poser aucune brique du projet sur un écran. C'est le constat de la phase 1 du LOT-87.

# Contrôles : les briques que la conception réutilise d'un écran à l'autre. Écrans : un FORMULAIRE
# `.ui.qml` par écran ; son jumeau de câblage `.qml` vit dans Source/App/Game/Qml/Screens/, module
# `Jadg.App`. Ajouter un écran = ajouter son formulaire ici et son jumeau là-bas.
set(JADG_UI_QML_FILES
    Theme/Tokens.qml
    Controls/SheetLine.ui.qml
    Controls/SheetBlock.ui.qml
    Controls/SheetList.ui.qml
    Controls/SheetProse.ui.qml
    Controls/SheetPortrait.ui.qml
    Controls/SheetTrack.ui.qml
    Controls/SheetActionBar.ui.qml
    Controls/RpgScreenFrame.ui.qml
    Controls/Cabochon.ui.qml
    Controls/ParchmentFrame.ui.qml
    Controls/TitleBanner.ui.qml
    Controls/FocusFleuron.ui.qml
    Controls/MenuEntry.ui.qml
    Controls/DiagnosticsOverlay.ui.qml
    Screens/CharacterSheetForm.ui.qml
    Screens/InventoryForm.ui.qml
    Screens/JournalForm.ui.qml
    Screens/WorldMapForm.ui.qml
    Screens/DialogueForm.ui.qml
    Screens/MerchantForm.ui.qml
    Screens/GuildBoardForm.ui.qml
    Screens/CombatHudForm.ui.qml
    Screens/TeamSheetForm.ui.qml
    Screens/MainMenuForm.ui.qml
    Screens/OptionsForm.ui.qml
    Screens/PauseForm.ui.qml
    Screens/CreditsForm.ui.qml
    Screens/GameViewForm.ui.qml
    # La galerie de l'atelier : ce que Qt Design Studio ouvre en premier (mainFile du .qmlproject).
    DesignStudio/Main.ui.qml
)

# Les singletons doivent être DÉCLARÉS : CMake ne déduit pas `pragma Singleton` du contenu du
# fichier. Non déclaré, le type se charge quand même, mais chaque `import` en construirait une
# instance neuve, et le facteur d'agrandissement posé par la fenêtre ne serait vu par aucun écran.
set(JADG_UI_QML_SINGLETONS
    Theme/Tokens.qml
)
set_source_files_properties(${JADG_UI_QML_SINGLETONS} PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

# Pas un seul Q_OBJECT ici, mais qt_add_qml_module extrait les métatypes de toute cible de module,
# et cette extraction exige AUTOMOC : sans lui, la configuration échoue sur « Metatype generation
# requires ... AUTOMOC », pour un module qui n'a rien à mocquer.
set(CMAKE_AUTOMOC ON)

qt_add_qml_module(JadgUi
    URI Jadg.Ui
    VERSION 1.0
    STATIC
    DESIGNER_SUPPORTED
    QML_FILES ${JADG_UI_QML_FILES}
)

# Illustrations extraites du corpus (EX-IHM-076) que les formulaires désignent par un chemin
# RELATIF À LEUR SOURCE : `../../Elements/Assets/UI/world-map.jpg` depuis Screens/. Le même chemin
# doit fonctionner dans l'atelier (qui lit Source/), depuis les sources (JADG_QML_FROM_SOURCE=1) et
# depuis la ressource : l'image est donc embarquée à l'endroit exact où ce chemin relatif la
# cherche depuis `/qt/qml/Jadg/Ui/Screens/`, c'est-à-dire `/qt/qml/Jadg/Elements/Assets/UI/`.
# `BASE` est le mécanisme prévu par qt_add_resources pour cela -- pas un alias par fichier.
qt_add_resources(JadgUi "JadgUiIllustrations"
    PREFIX "/qt/qml/Jadg/Elements/Assets/UI"
    BASE "${PROJECT_SOURCE_DIR}/Source/Elements/Assets/UI"
    FILES "${PROJECT_SOURCE_DIR}/Source/Elements/Assets/UI/world-map.jpg"
)

# --- Éditer un écran sans rien reconstruire (EX-IHM-100) --------------------------------------
#
# La ressource embarque les .qml : la moindre retouche de mise en page demanderait sinon de
# repasser par le build. Un second `qmldir` -- engendré depuis LA MÊME liste que la ressource,
# jamais tenu à la main -- désigne les fichiers SOURCES. Lancer le jeu avec JADG_QML_FROM_SOURCE=1
# place Source/Ui en tête des chemins d'import : le programme lit alors ces fichiers directement.
#
# Écrit SOUS Source/Ui (ignoré par git), pas dans le répertoire de construction : c'est ce qui
# permet au `.qmlproject` de l'artiste de déclarer `importPaths: ["."]` et de résoudre `Jadg.Ui`
# sans connaître le build. `designersupported` y figure pour la même raison que ci-dessus.
set(JADG_QML_DEV_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
set(_dev_module_dir ${CMAKE_CURRENT_SOURCE_DIR}/Jadg/Ui)
file(MAKE_DIRECTORY ${_dev_module_dir})
set(_dev_qmldir "module Jadg.Ui
designersupported
")
foreach(_qml ${JADG_UI_QML_FILES})
    get_filename_component(_qml_type ${_qml} NAME_WE)
    file(RELATIVE_PATH _qml_rel ${_dev_module_dir} ${CMAKE_CURRENT_SOURCE_DIR}/${_qml})
    if(${_qml} IN_LIST JADG_UI_QML_SINGLETONS)
        string(APPEND _dev_qmldir "singleton ${_qml_type} 1.0 ${_qml_rel}
")
    else()
        string(APPEND _dev_qmldir "${_qml_type} 1.0 ${_qml_rel}
")
    endif()
endforeach()
file(WRITE ${_dev_module_dir}/qmldir "${_dev_qmldir}")
