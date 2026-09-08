# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

# Couche de présentation HMI : **application Qt** (`JustAnotherDnDGame`).
#
# Depuis le LOT-38, l'IHM « maison » et l'exécutable historique `JustAnotherDnDGame` ont été retirés :
# l'unique application est l'éditeur Qt, qui embarque le rendu Direct3D 11 du jeu dans un viewport et
# l'entoure de panneaux d'édition. Le code est réparti par domaine, au plus près de l'architecture
# d'origine du projet :
#   Platform/     provisionnement bas niveau (répertoire exécutable) ;
#   Input/        entrées (état, mapping, remappage clavier/manette, pont Qt -> Key) ;
#   Graphics/     rendu Direct3D 11 réutilisable (device, sprites, caméra, brouillon d'édition) ;
#   Game/         simulation d'un niveau (GameSession) et viewport Qt jeu/édition ;
#   Localization/ catalogue de traduction ;
#   Interface/    fenêtre principale, menu, options, remappage (widgets Qt) ;
#   Editor/       périmètre éditeur de niveau (palette, outils, navigateur, logique pure).
# Les fichiers déclaratifs Qt (.ui, .qrc) et le thème (.qss) vivent dans Source/Elements (assets).

# --- HmiLib : la couche de présentation **sans Qt Widgets** (LOT-02) ---
#
# Ces sources sont de la logique pure : géométrie de caméra, composition de scène (sans GPU),
# catalogues, entrées, persistance JSON, géométrie d'éditeur. Elles ne dérivent d'aucun QObject et
# n'incluent aucun en-tête Qt — c'est exactement ce que les tests unitaires consomment.
#
# Avant ce lot, `Source/Test/CMakeLists.txt` **relistait ces 79 chemins un par un** pour les
# recompiler dans `UnitTests` : chaque fichier ajouté à l'IHM devait l'être à deux endroits, et un
# oubli ne se voyait qu'à l'édition de liens. Une bibliothèque nomme cette frontière au lieu de la
# maintenir à la main dans un commentaire.
#
# Déclarée **avant** la garde `BUILD_EDITOR_QT`/Qt ci-dessus, et volontairement libre de Qt : sur un
# poste sans Qt, l'application est ignorée mais `UnitTests` doit continuer de se construire — c'est
# la raison d'être de l'option. AUTOMOC n'est pas encore actif à ce point du fichier, ce qui est
# correct : aucune de ces sources n'a de `Q_OBJECT`.
add_library(HmiLib STATIC
    # Graphismes : logique de rendu sans GPU (composition, catalogues, géométrie de caméra).
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
    # Entrées : état, mapping, remappage persistant.
    Input/InputState.cpp
    Input/PlayerInputMapper.cpp
    Input/GameKeyBindings.cpp
    Input/EditorKeyBindings.cpp
    Input/KeyName.cpp
    Input/GamepadBindings.cpp
    Input/GamepadButtonName.cpp
    # Localisation, journal de session.
    Localization/Localization.cpp
    Diagnostics/SessionLog.cpp
    # Audio : catalogue et table événement -> son (le moteur lui-même dépend de Qt Multimedia).
    Audio/SoundCatalog.cpp
    Audio/SoundTriggers.cpp
    # Jeu : événements, affichage tête haute, progression, bilan de tableau.
    Game/GameEvents.cpp
    # Ordre des passes du pas fixe (LOT-05) : logique pure, sans Qt ni GPU -- c'est ce qui la rend
    # testable sans fenetre, alors que GameSession, son orchestrateur, ne l'est pas.
    Game/ExplorationMode.cpp
    Game/CombatMode.cpp
    Game/GameHud.cpp
    Game/DiagnosticsHud.cpp
    # Éditeur : validation, taxonomie, gestes et géométrie, bibliothèque d'assets, atelier pixel.
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
    # Interface : logique d'écrans et de thème, sans widget.
    Interface/ScreenFlow.cpp
    Interface/RpgScreens.cpp
    Interface/CharacterSheetValues.cpp
    Interface/InventoryValues.cpp
    Interface/DesignTokens.cpp
    Interface/StyleSheetTemplate.cpp
    Interface/FontResolution.cpp
    Interface/ThemeResolution.cpp
    Interface/IconGeometry.cpp
    Interface/IdentityScale.cpp
    Interface/EditorWorkspace.cpp
    Interface/ParchmentFrame.cpp
    Interface/ParchmentOrnaments.cpp
    Interface/AbilityWheelGeometry.cpp
    Interface/MenuBackdropGeometry.cpp
    Interface/KeyHintText.cpp
    Interface/ActionCatalog.cpp
    # Plateforme.
    Platform/ExecutableDirectory.cpp
)

# `PUBLIC` sur la racine d'inclusion et sur Core : tout consommateur de HmiLib (l'application, les
# tests) écrit `#include "HMI/..."` et manipule des types de `core::`.
target_include_directories(HmiLib PUBLIC ${PROJECT_SOURCE_DIR}/Source)
target_link_libraries(HmiLib
    PUBLIC Core
    PRIVATE project_warnings project_options
    # Les *KeyBindings et les catalogues persistent en JSON. Core le propage depuis le LOT-79,
    # mais HmiLib le redemande explicitement : ses en-tetes en dependent pour leur compte.
    PUBLIC nlohmann_json::nlohmann_json)

target_compile_definitions(HmiLib PUBLIC WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)

if(ENABLE_PCH)
    target_precompile_headers(HmiLib PRIVATE ${PROJECT_SOURCE_DIR}/Source/pch.h)
endif()

# La cible reste **optionnelle** : sur un poste sans Qt (ex. CI n'exécutant que les tests unitaires,
# qui compilent les sources pures directement), la configuration n'échoue pas — l'application est
# simplement ignorée. Fournir Qt via CMAKE_PREFIX_PATH (ou l'action CI install-qt-action) pour
# l'activer, ou passer -DBUILD_EDITOR_QT=OFF pour l'ignorer explicitement.
option(BUILD_EDITOR_QT "Construire l'éditeur Qt (JustAnotherDnDGame). Nécessite Qt6." ON)
if(NOT BUILD_EDITOR_QT)
    message(STATUS "JustAnotherDnDGame : désactivé (BUILD_EDITOR_QT=OFF).")
    return()
endif()

# --- Version de référence (EX-BUILD-010) ---
# Seul endroit où la version de Qt est écrite dans le CMake : la CI (`.github/workflows/ci.yml`,
# `release.yml`) l'installe explicitement à cette même valeur (`env.QT_VERSION`), vérifié
# automatiquement par `scripts/check_qt_version_pin.py` (job `lint-exigences`) pour que les deux
# ne divergent jamais silencieusement.
set(QT_VERSION_MINIMUM "6.11.2")

# --- Découverte de Qt sans configuration manuelle ---
# find_package() ne consulte que CMAKE_PREFIX_PATH. Or l'installateur officiel Qt place ses
# versions dans <racine>/<version>/<compilateur> — un emplacement que CMake ne connaît pas. Sur un
# poste neuf, la configuration ne trouvait donc rien et la cible était ignorée *en silence* : le
# build « réussissait » sans jamais produire d'exécutable. On complète la recherche par les
# emplacements conventionnels, en les ajoutant **en fin** de liste pour qu'un choix explicite
# (-DCMAKE_PREFIX_PATH, -DQt6_DIR) garde toujours la priorité.
set(_qt_hints "")

# Variables d'environnement usuelles. QT_ROOT_DIR est exporté par install-qt-action (CI) ;
# QTDIR est la convention historique de l'installateur.
foreach(_qt_var QT_ROOT_DIR Qt6_ROOT QTDIR)
    if(DEFINED ENV{${_qt_var}})
        list(APPEND _qt_hints "$ENV{${_qt_var}}")
    endif()
endforeach()

# Arborescences de l'installateur officiel. Le tri décroissant naturel place la version la plus
# récente en tête (6.10 avant 6.8, ce qu'un tri lexicographique ferait à l'envers).
if(WIN32)
    file(GLOB _qt_installs
        "$ENV{SystemDrive}/Qt/*/msvc*_64"
        "C:/Qt/*/msvc*_64"
        "D:/Qt/*/msvc*_64")
else()
    file(GLOB _qt_installs
        "$ENV{HOME}/Qt/*/gcc_64"
        "/opt/Qt/*/gcc_64")
endif()
list(SORT _qt_installs COMPARE NATURAL ORDER DESCENDING)
list(APPEND _qt_hints ${_qt_installs})

if(_qt_hints)
    list(REMOVE_DUPLICATES _qt_hints)
    list(APPEND CMAKE_PREFIX_PATH ${_qt_hints})
endif()

find_package(Qt6 ${QT_VERSION_MINIMUM} QUIET COMPONENTS Widgets Gui GuiPrivate Multimedia ShaderTools)
if(NOT Qt6_FOUND)
    # Deuxième essai, sans contrainte de version, seulement pour distinguer dans le message
    # « rien d'installé » de « une version installée, mais trop ancienne » — sinon les deux
    # produisent le même NOT Qt6_FOUND et le second cas serait diagnostiqué à tort comme le premier.
    find_package(Qt6 QUIET COMPONENTS Widgets Gui GuiPrivate Multimedia ShaderTools)
    # Avertissement et non simple statut : sans Qt il n'y a **aucun exécutable**, seulement les
    # tests. Passer inaperçu est précisément ce qui rendait l'absence d'exe difficile à diagnostiquer.
    if(Qt6_FOUND)
        message(WARNING
            "JustAnotherDnDGame : Qt6 ${Qt6_VERSION} trouvé (${Qt6_DIR}), mais inférieur à la version "
            "minimale requise ${QT_VERSION_MINIMUM} — cible ignorée, AUCUN EXÉCUTABLE ne sera produit.\n"
            "Installer au moins Qt ${QT_VERSION_MINIMUM} (ou désigner une autre installation via "
            "-DCMAKE_PREFIX_PATH), ou passer -DBUILD_EDITOR_QT=OFF pour ne construire que les tests.")
    else()
        # Troisième cas, distinct des deux précédents : Qt est bien installé, mais il lui manque un
        # MODULE (typiquement Multimedia, un composant additionnel que l'installateur officiel ne
        # coche pas par défaut). find_package échoue alors en bloc, et annoncer « Qt6 introuvable »
        # envoie chercher au mauvais endroit — le cas a réellement coûté une session de diagnostic.
        find_package(Qt6 QUIET COMPONENTS Widgets Gui)
        if(Qt6_FOUND)
            set(_qt_missing "")
            foreach(_qt_component Widgets Gui GuiPrivate Multimedia ShaderTools)
                find_package(Qt6${_qt_component} QUIET)
                if(NOT Qt6${_qt_component}_FOUND)
                    list(APPEND _qt_missing ${_qt_component})
                endif()
            endforeach()
            message(WARNING
                "JustAnotherDnDGame : Qt6 ${Qt6_VERSION} est bien installé (${Qt6_DIR}), mais il manque "
                "le(s) module(s) : ${_qt_missing} — cible ignorée, AUCUN EXÉCUTABLE ne sera produit.\n"
                "ShaderTools (l'outil `qsb`) et GuiPrivate (les en-têtes QRhi) sont exigés depuis le "
                "LOT-69 TACHE-02 : le rendu passe par QRhi, plus par Direct3D 11 en direct.\n"
                "Ajouter le(s) module(s) manquant(s) avec l'outil de maintenance Qt "
                "(MaintenanceTool.exe, à la racine de l'installation), puis reconfigurer.\n"
                "Passer -DBUILD_EDITOR_QT=OFF pour ne construire que les tests, sans cet avertissement.")
        else()
            message(WARNING
                "JustAnotherDnDGame : Qt6 introuvable, cible ignorée — AUCUN EXÉCUTABLE ne sera produit.\n"
                "Version minimale requise : ${QT_VERSION_MINIMUM}.\n"
                "Emplacements examinés : ${_qt_hints}\n"
                "Corriger en installant Qt via l'installateur officiel, ou en désignant l'installation :\n"
                "  cmake --preset ninja -DCMAKE_PREFIX_PATH=<chemin>/msvc2022_64\n"
                "Passer -DBUILD_EDITOR_QT=OFF pour ne construire que les tests, sans cet avertissement.")
        endif()
    endif()
    return()
endif()

if(NOT Qt6_VERSION VERSION_EQUAL QT_VERSION_MINIMUM)
    # Avertissement, pas erreur (cf. TACHE-01) : interdire de construire sur une version voisine
    # coûterait plus qu'il ne rapporterait. Le but est qu'un écart soit visible, pas interdit.
    message(WARNING
        "JustAnotherDnDGame : Qt6 ${Qt6_VERSION} trouvé (${Qt6_DIR}), différent de la version de "
        "référence ${QT_VERSION_MINIMUM} (celle validée par la CI) — le build peut diverger de la "
        "CI sans que rien d'autre ne le signale.")
endif()

message(STATUS "JustAnotherDnDGame : Qt6 ${Qt6_VERSION} trouvé (${Qt6_DIR}).")

# Traitement automatique des métacompilateurs Qt, restreint à cette cible (moc/uic/rcc).
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Assets Qt déclaratifs (hors code) : mises en page Designer + ressource embarquée, dans Elements.
set(ELEMENTS_UI ${PROJECT_SOURCE_DIR}/Source/Elements/UI)

add_executable(JustAnotherDnDGame
    Main.cpp
    # Plateforme
    # Entrées (dont le pont Qt -> Key)
    Input/GamepadPoller.cpp
    Input/QtKeyMap.cpp
    # Rendu 2D sur QRhi (LOT-69 TACHE-02) — cible Direct3D 11 sous Windows (EX-REN-002),
    # atteinte au travers de QRhi plutôt qu'en appelant l'API directement.
    Graphics/SpriteBatch.cpp
    Graphics/TextureLoader.cpp
    Graphics/TextureAtlas.cpp
    Graphics/TextureCache.cpp
    Graphics/SpriteRenderer.cpp
    Graphics/DraftRenderer.cpp
    Graphics/BitmapFont.cpp
    # Jeu (simulation d'un niveau + viewport Qt jeu/édition)
    Game/GameSession.cpp
    Game/GameViewport.cpp
    # Traduction requete -> surcharges (LOT-73, EX-IHM-083) : logique PURE, compilee aussi
    # dans UnitTests (cf. Source/Test/CMakeLists.txt) -- c'est la ou des reglages se perdaient.
    # Localisation
    # Audio (bruitages, LOT-60)
    Audio/AudioEngine.cpp
    # Diagnostics (enregistrement des journaux de session)
    # Interface (fenêtre, menu, options, remappage)
    Interface/ApplicationTheme.cpp
    # Habillage pixel art des ecrans du jeu (LOT-68) : logique pure, compilee aussi
    # dans UnitTests (cf. Source/Test/CMakeLists.txt).
    # Widgets Qt de l'habillage pixel art (LOT-68) : peintres au-dessus de la geometrie pure
    # ci-dessus, promus dans les .ui. Non compiles dans UnitTests (dependent de Qt).
    Interface/ParchmentPanel.cpp
    Interface/OrnamentPainter.cpp
    Interface/TitleBanner.cpp
    Interface/AbilityWheel.cpp
    Interface/SheetWidgets.cpp
    Interface/RpgCharacterSheetPlate.cpp
    Interface/FocusMarker.cpp
    Interface/MenuEntryButton.cpp
    Interface/ThemeIcons.cpp
    Interface/EditorActions.cpp
    # Enveloppe defilante de toute page d ecran (LOT-73, EX-IHM-080) : aucun ecran ne
    # contraint la taille de la fenetre.
    Interface/ScreenPageHost.cpp
    Interface/MainWindow.cpp
    Interface/MainMenu.cpp
    Interface/PauseScreen.cpp
    Interface/CreditsScreen.cpp
    Interface/RpgScreenFrame.cpp
    Interface/RpgScreenHost.cpp
    Interface/OptionsPage.cpp
    Interface/KeybindingsWidget.cpp
    Interface/EditorKeybindingsWidget.cpp
    Interface/GamepadBindingsWidget.cpp
    # Éditeur de niveau
    Editor/PalettePanel.cpp
    Editor/LevelBrowserPanel.cpp
    Editor/LinkPanel.cpp
    Editor/TexturePanel.cpp
    Editor/AssetThumbnailView.cpp
    Editor/PixelCanvas.cpp
    Editor/PixelHistoryPanel.cpp
    Editor/PixelPalettePanel.cpp
    Editor/PlanesPanel.cpp
    # Ressource embarquée (thème .qss) — traitée par AUTORCC.
    ${ELEMENTS_UI}/resources.qrc
    # Mises en page Qt Designer — traitées par AUTOUIC (génèrent ui_*.h).
    ${ELEMENTS_UI}/MainWindow.ui
    ${ELEMENTS_UI}/MainMenu.ui
    ${ELEMENTS_UI}/PauseScreen.ui
    ${ELEMENTS_UI}/RpgCharacterSheetPlate.ui
    ${ELEMENTS_UI}/CreditsScreen.ui
    ${ELEMENTS_UI}/OptionsPage.ui
    ${ELEMENTS_UI}/TexturePanel.ui
    ${ELEMENTS_UI}/PlanesPanel.ui
    ${ELEMENTS_UI}/LevelBrowserPanel.ui
    ${ELEMENTS_UI}/AssetThumbnailView.ui
)

# Shaders du pipeline 2D, compilés en `.qsb` par `qsb` (module ShaderTools) et embarqués comme
# ressource Qt. Un `.qsb` porte plusieurs variantes (SPIR-V, HLSL, MSL, GLSL) : c'est ce qui permet
# à QRhi de choisir son backend à l'exécution sans que le projet ait à livrer un shader par API.
qt6_add_shaders(JustAnotherDnDGame "jadg_shaders"
    PREFIX "/shaders"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}/Graphics/Shaders"
    FILES
        Graphics/Shaders/sprite.vert
        Graphics/Shaders/sprite.frag
)

# Racine d'inclusion Source/ (pour les includes "HMI/..." et "Core/...").
target_include_directories(JustAnotherDnDGame PRIVATE ${PROJECT_SOURCE_DIR}/Source)

# Les mises en page .ui vivent dans Source/Elements/UI (assets), pas à côté des widgets : AUTOUIC
# doit y chercher le .ui correspondant à chaque include "ui_*.h".
set_target_properties(JustAnotherDnDGame PROPERTIES AUTOUIC_SEARCH_PATHS ${ELEMENTS_UI})

target_link_libraries(JustAnotherDnDGame PRIVATE
    # La couche sans Qt Widgets (LOT-02) : elle propage Core, la racine d'inclusion Source/ et
    # nlohmann_json, que l'application n'a donc plus à redemander.
    HmiLib
    Qt6::Widgets
    Qt6::Gui
    # QRhi vit dans les en-têtes privés de QtGui : c'est une API privée, stable en pratique mais
    # non couverte par la garantie de compatibilité binaire — assumé, QRhiWidget n'existe pas sans.
    Qt6::GuiPrivate
    Qt6::Multimedia
    xinput
    project_warnings
    project_options
)

# Réduit la surface de Windows.h, évite les macros min/max, API Unicode.
target_compile_definitions(JustAnotherDnDGame PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    UNICODE
    _UNICODE
)

if(ENABLE_PCH)
    target_precompile_headers(JustAnotherDnDGame PRIVATE ${PROJECT_SOURCE_DIR}/Source/pch.h)
endif()

# Copie des assets à côté de l'exécutable, pour que l'éditeur charge niveaux et traductions
# relativement à son binaire.
#
# Cible séparée (et non des POST_BUILD sur JustAnotherDnDGame) : sous Ninja, un POST_BUILD est fondu
# dans l'arête de build de l'exécutable et n'est rejoué que si celle-ci l'est — c'est-à-dire quand
# un .obj change. Modifier uniquement un fichier sous Source/Elements/{Levels,Assets,...} ne
# touche aucun .obj, donc l'exécutable reste « à jour » aux yeux de Ninja et la copie est silencieusement
# sautée, même via un `cmake --build` complet, sans cible restreinte. Une cible dédiée sans sortie
# déclarée est toujours considérée sale par Ninja et s'exécute donc à chaque build.
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
    # Catalogues RPG (LOT-38) : especes, classes, historiques, competences, regles et fiches. Le
    # premier consommateur a l'execution est l'ecran de fiche ; jusque-la, ces donnees ne vivaient
    # que pour les tests, qui les lisent dans l'arborescence source.
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/Source/Elements/Rpg
        $<TARGET_FILE_DIR:JustAnotherDnDGame>/Rpg
    COMMENT "Copie des données de jeu (traductions, cartes, assets, audio, catalogues RPG)")
add_dependencies(JustAnotherDnDGame CopyGameData)

# Déploiement Qt : copie les DLL Qt (plateforme, styles) à côté de l'exécutable pour que
# l'utilisateur final n'ait **aucune bibliothèque à installer** — un simple dossier exécutable
# autonome. windeployqt est fourni par l'installation Qt (même répertoire bin que qmake).
get_target_property(_qmake_exe Qt6::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_exe}" DIRECTORY)
find_program(WINDEPLOYQT_EXE windeployqt HINTS "${_qt_bin_dir}")
if(WINDEPLOYQT_EXE)
    add_custom_command(TARGET JustAnotherDnDGame POST_BUILD
        COMMAND "${WINDEPLOYQT_EXE}"
            --no-translations --compiler-runtime
            "$<TARGET_FILE:JustAnotherDnDGame>"
        COMMENT "Déploiement des DLL Qt + runtime à côté de l'exécutable (windeployqt)")

    # `--no-translations` ci-dessus reste le bon réglage : windeployqt déploierait la trentaine de
    # catalogues de Qt, alors que l'IHM du projet a le sien. Mais **les boutons standard des boîtes
    # de dialogue** (« Oui »/« Non »/« Annuler ») viennent de Qt, pas du catalogue du projet, et
    # restent en anglais sans son `qtbase_<langue>.qm` — une boîte française à boutons anglais, ce
    # qui s'est vu à l'essai (`LOT-69`). On copie donc **les deux seuls** catalogues dont l'IHM
    # propose la langue. Fichier absent : la copie est silencieusement ignorée (`copy_if_different`
    # sur une liste vide), et le programme retombe sur l'anglais (`EX-NFR-040`).
    get_target_property(_qt_core_location Qt6::Core IMPORTED_LOCATION_RELEASE)
    if(NOT _qt_core_location)
        get_target_property(_qt_core_location Qt6::Core IMPORTED_LOCATION)
    endif()
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
