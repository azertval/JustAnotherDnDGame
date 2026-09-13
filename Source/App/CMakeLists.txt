# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

# JustAnotherDnDGame : le jeu, en Qt Quick (LOT-86), et son module QML `Jadg.App` (LOT-87).
#
# L'exécutable porte le troisième module : la fenêtre (`Main.qml`), la pile d'écrans (`Logic/`) et
# les JUMEAUX de câblage des écrans (`Screens/<Écran>.qml`), un par formulaire de `Jadg.Ui`. C'est
# le module du DÉVELOPPEUR : il importe `Jadg.Runtime` (les types C++) et `Jadg.Ui` (les
# formulaires), et personne ne l'importe en retour. Les fichiers vivent sous Game/Qml/, et ce
# CMakeLists.txt est celui de l'exécutable, donc les chemins de ressource se calculent d'ici sans
# alias -- cf. Source/CMakeLists.txt.
#
# L'éditeur de niveaux (LevelEditor, Qt Widgets) est déclaré dans Source/HMI/CMakeLists.txt : ses
# sources sont celles de HMI, et il n'a pas de QML.

set(CMAKE_AUTOMOC ON)

add_executable(JustAnotherDnDGame
    Game/Main.cpp
    Common/Bootstrap.cpp
    # Le son : le volume des options doit atteindre un moteur reel (EX-IHM-083).
    ${PROJECT_SOURCE_DIR}/Source/HMI/Audio/AudioEngine.cpp
)

# Les deux modules statiques ET leurs plugins : le `qmldir` embarqué de chaque module désigne son
# plugin (`optional plugin`, `linktarget`), et c'est le plugin lié qui enregistre les types quand
# l'engine rencontre `import Jadg.Runtime`. Aucune macro d'import dans le C++ : Qt le fait à
# l'édition de liens.
target_link_libraries(JustAnotherDnDGame PRIVATE
    HmiLib
    JadgRuntime
    JadgRuntimeplugin
    JadgUi
    JadgUiplugin
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

# Écrans : chaque jumeau porte le nom du formulaire sans le suffixe `Form`, dans le même
# sous-dossier. `python scripts/list_pending_bindings.py` donne l'inventaire des liaisons en
# attente, dérivé de ces fichiers.
qt_add_qml_module(JustAnotherDnDGame
    URI Jadg.App
    VERSION 1.0
    QML_FILES
        Game/Qml/Main.qml
        # La pile d'écrans : l'état publié par ScreenRouter, traduit en écran affiché. Elle vit en
        # QML pour que la conception puisse renommer ses fichiers sans qu'une ligne de C++ le sache.
        Game/Qml/Logic/ScreenStack.qml
        # Le sélecteur d'écrans de développement, lié à ScreenRouter.developerBuild.
        Game/Qml/Logic/ScreenProbe.qml
        Game/Qml/Screens/CharacterSheet.qml
        Game/Qml/Screens/Inventory.qml
        Game/Qml/Screens/Journal.qml
        Game/Qml/Screens/WorldMap.qml
        Game/Qml/Screens/Dialogue.qml
        Game/Qml/Screens/Merchant.qml
        Game/Qml/Screens/GuildBoard.qml
        Game/Qml/Screens/CombatHud.qml
        Game/Qml/Screens/TeamSheet.qml
        Game/Qml/Screens/MainMenu.qml
        Game/Qml/Screens/Options.qml
        Game/Qml/Screens/Pause.qml
        Game/Qml/Screens/Credits.qml
        Game/Qml/Screens/GameView.qml
)

# `qmlcachegen` compile le QML A L'AVANCE en C++, et ce C++ ENGENDRÉ déclenche C4702 (« code
# inatteignable ») depuis les en-têtes de Qt eux-mêmes. `/external:W0` ne l'attrape pas : la
# construction fautive est instanciée dans l'unité engendrée, pas dans l'en-tête. `/WX` en fait
# alors une erreur, sur du code que le projet n'écrit pas et ne peut pas corriger.
#
# Neutralisé pour les cibles QML seulement, jamais pour le dépôt : `/W4 /WX` reste la règle
# (EX-NFR-013), et C4702 continue d'être une erreur partout ailleurs, y compris dans l'éditeur.
foreach(_qml_target JustAnotherDnDGame JadgUi JadgRuntime)
    target_compile_options(${_qml_target} PRIVATE /wd4702)
endforeach()

# --- Traductions du JEU (LOT-86) --------------------------------------------------------------
#
# Les ecrans du jeu ecrivent leurs textes EN FRANCAIS dans le fichier QML (`qsTr("Nouvelle
# partie")`) plutot que par une cle. C'est ce qui permet a la conception de juger une mise en page
# dans Qt Design Studio : une cle technique ne se lit pas. Le francais est donc la langue SOURCE, et
# n'a pas de catalogue -- sans traducteur installe, `qsTr` rend sa source.
#
# `lupdate` extrait, `lrelease` compile, et le .qm est embarque dans la ressource. La mise a jour du
# .ts se declenche a la main (cible `update_translations`) et non a chaque construction : lupdate
# reecrit alors le fichier, et le voir bouger sous soi a chaque build rend toute relecture penible.
#
# OPTIONNEL, et il faut qu'il le soit : LinguistTools n'est pas garanti present sur une machine ou
# dans une image d'integration continue. Absent, le jeu se construit et parle francais -- ce qui est
# un repli honnete -- et le message dit pourquoi, plutot que de laisser chercher.
find_package(Qt6 ${QT_VERSION_MINIMUM} QUIET COMPONENTS LinguistTools)
if(Qt6LinguistTools_FOUND)
    # Les textes à traduire vivent dans les DEUX modules : les formulaires (Jadg.Ui) et les jumeaux
    # (Jadg.App). Un seul catalogue, extrait des deux cibles.
    qt_add_translations(JustAnotherDnDGame
        SOURCE_TARGETS JustAnotherDnDGame JadgUi
        TS_FILES ${PROJECT_SOURCE_DIR}/Source/Elements/Localization/jadg_en.ts
        RESOURCE_PREFIX "/i18n"
    )
else()
    message(STATUS "JustAnotherDnDGame : LinguistTools absent, le jeu ne parlera que francais "
        "(sa langue source). Installer le module qttools pour produire les catalogues.")
endif()

# Fichier de configuration du serveur de langage QML : ce qui donne complétion et diagnostics dans
# un éditeur, sans que la conception ait à décrire les chemins d'import à la main.
set_target_properties(JustAnotherDnDGame PROPERTIES QT_QML_GENERATE_QMLLS_INI ON)

# Éditer un écran sans rien reconstruire (EX-IHM-100) : le chemin que `JADG_QML_FROM_SOURCE=1` place
# en tête des imports. Posé par Source/Ui/CMakeLists.txt, qui y engendre le `qmldir` de dev.
target_compile_definitions(JustAnotherDnDGame PRIVATE JADG_QML_DEV_IMPORT_PATH="${JADG_QML_DEV_DIR}")

# Racine d'inclusion Source/ (pour les includes "App/...", "HMI/..." et "Core/...").
target_include_directories(JustAnotherDnDGame PRIVATE ${PROJECT_SOURCE_DIR}/Source)
# Réduit la surface de Windows.h, évite les macros min/max, API Unicode.
target_compile_definitions(JustAnotherDnDGame PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)
if(ENABLE_PCH)
    target_precompile_headers(JustAnotherDnDGame PRIVATE ${PROJECT_SOURCE_DIR}/Source/pch.h)
endif()


# Données de jeu à côté de l'exécutable : la cible est déclarée dans Source/HMI (les deux binaires
# sortent dans le même répertoire, une seule copie les sert).
add_dependencies(JustAnotherDnDGame CopyGameData)

# Déploiement Qt : copie les DLL Qt (plateforme, styles, plugins QML) à côté du jeu pour que
# l'utilisateur final n'ait **aucune bibliothèque à installer**. `--qmldir` : sans lui, windeployqt
# ne sait pas quels modules QML embarquer, et le jeu se lance sans son interface, sans message. Les
# deux répertoires QML du jeu sont scannés : les formulaires et les jumeaux.
get_target_property(_qmake_exe Qt6::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_exe}" DIRECTORY)
find_program(WINDEPLOYQT_EXE windeployqt HINTS "${_qt_bin_dir}")
if(WINDEPLOYQT_EXE)
    add_custom_command(TARGET JustAnotherDnDGame POST_BUILD
        COMMAND "${WINDEPLOYQT_EXE}"
            --no-translations --compiler-runtime
            --qmldir "${PROJECT_SOURCE_DIR}/Source/Ui"
            --qmldir "${CMAKE_CURRENT_SOURCE_DIR}/Game/Qml"
            "$<TARGET_FILE:JustAnotherDnDGame>"
        COMMENT "Déploiement des DLL Qt + modules QML à côté du jeu (windeployqt)")

    # --no-translations ci-dessus reste le bon réglage : windeployqt déploierait la trentaine de
    # catalogues de Qt, alors que l'IHM du projet a le sien. Mais **les boutons standard des boîtes
    # de dialogue** (« Oui »/« Non »/« Annuler ») viennent de Qt, pas du catalogue du projet, et
    # restent en anglais sans son qtbase_<langue>.qm — une boîte française à boutons anglais, ce
    # qui s'est vu à l'essai (LOT-69). On copie donc **les deux seuls** catalogues dont l'IHM
    # propose la langue. Fichier absent : la copie est silencieusement ignorée, et le programme
    # retombe sur l'anglais (EX-NFR-040).
    get_filename_component(_qt_prefix "${_qt_bin_dir}" DIRECTORY)
    set(_qt_translations_dir "${_qt_prefix}/translations")
    foreach(_lang fr en)
        if(EXISTS "${_qt_translations_dir}/qtbase_${_lang}.qm")
            add_custom_command(TARGET JustAnotherDnDGame POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${_qt_translations_dir}/qtbase_${_lang}.qm"
                    "$<TARGET_FILE_DIR:JustAnotherDnDGame>/Translations/qtbase_${_lang}.qm"
                COMMENT "Copie du catalogue Qt qtbase_${_lang}.qm")
        endif()
    endforeach()
else()
    message(STATUS "JustAnotherDnDGame : windeployqt introuvable, DLL Qt non déployées "
        "automatiquement (exécution possible depuis un environnement Qt).")
endif()
