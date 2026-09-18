# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

# Couche de présentation HMI : ce que le jeu (`JustAnotherRpgGame`, Qt Quick) et l'éditeur
# (`LevelEditor`, Qt Widgets) partagent, puis l'éditeur lui-même. Le code est réparti par domaine :
#   Platform/     provisionnement bas niveau (répertoire exécutable, minidump) ;
#   Input/        entrées (état, manette, raccourcis de l'éditeur) ;
#   Graphics/     rendu 2D sur QRhi (sprites, caméra, lieu, arène, brouillon d'édition) ;
#   Game/         la carte qu'on parcourt, partagée par le jeu et l'essai de l'éditeur ;
#   Presentation/ logique de présentation pure (table de transitions, échelle, valeurs de fiche) ;
#   Runtime/      les types C++ que les écrans du jeu voient -- module QML `Jadg.Runtime` (LOT-87) ;
#   Localization/ catalogue de traduction ;
#   Interface/    fenêtre de l'éditeur, actions, thème (widgets Qt) ;
#   Editor/       périmètre éditeur de cartes (canevas, palette, navigateur, logique pure).
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
    Graphics/TileVisuals.cpp
    Graphics/ProceduralAtlas.cpp
    Graphics/TextureAtlasRegions.cpp
    Graphics/ArenaAppearanceCatalog.cpp
    Graphics/ArenaSceneComposer.cpp
    # Le lieu qu'on parcourt : table d'apparence du lieu, et composition (LOT-09).
    Graphics/PlaceAppearance.cpp
    Graphics/WorldSceneComposer.cpp
    # La carte qu'on parcourt et sa mise en scene, partagees par le jeu et l'essai de l'editeur.
    Game/WorldPlay.cpp
    Graphics/AssetGallery.cpp
    Graphics/ArenaAnimationDriver.cpp
    Graphics/AnimationCatalog.cpp
    Graphics/MissingTexture.cpp
    Graphics/ComposedScene.cpp
    Graphics/QuadRecorder.cpp
    # Marqueurs d'entite de carte (LOT-11) : cle d'asset et pixels, sans GPU.
    Graphics/EntityMarkers.cpp
    # Entrées : état, mapping, remappage persistant.
    Input/InputState.cpp
    Input/EditorKeyBindings.cpp
    # Localisation, journal de session.
    Localization/Localization.cpp
    # Éditeur : validation, taxonomie, gestes et géométrie, diagnostics des entités.
    Editor/LevelNameValidation.cpp
    Editor/TileTaxonomy.cpp
    Editor/TaxonomyLabels.cpp
    Editor/LevelFileOperations.cpp
    Editor/EditorStatus.cpp
    Editor/PanelFocus.cpp
    Editor/ThumbnailGeometry.cpp
    Editor/LayerView.cpp
    Editor/EntityGesture.cpp
    Editor/EntityReferences.cpp
    Editor/EditorDiagnostics.cpp
    Editor/WorldGraphLayout.cpp
    # Interface : logique d'écrans et de thème, sans widget.
    Presentation/ScreenFlow.cpp
    Presentation/RpgScreens.cpp
    Presentation/CharacterSheetValues.cpp
    Presentation/InventoryValues.cpp
    Presentation/DialogueScreen.cpp
    Presentation/CreditsCatalog.cpp
    Presentation/InventoryScreen.cpp
    Presentation/WorldMaps.cpp
    Interface/DesignTokens.cpp
    Interface/StyleSheetTemplate.cpp
    Interface/FontResolution.cpp
    Interface/ThemeResolution.cpp
    Interface/IconGeometry.cpp
    Presentation/IdentityScale.cpp
    Interface/ActionCatalog.cpp
    # Plateforme.
    Platform/ExecutableDirectory.cpp
    Platform/CrashDump.cpp
)

# `PUBLIC` sur la racine d'inclusion et sur Core : tout consommateur de HmiLib (l'application, les
# tests) écrit `#include "HMI/..."` et manipule des types de `core::`.
target_include_directories(HmiLib PUBLIC ${PROJECT_SOURCE_DIR}/Source)
target_link_libraries(HmiLib
    PUBLIC Core
    PRIVATE project_warnings project_options
    # MiniDumpWriteDump (Platform/CrashDump.cpp) : bibliothèque système, présente sur tout Windows.
    PRIVATE dbghelp
    # Les *KeyBindings et les catalogues persistent en JSON. Core le propage depuis le LOT-79,
    # mais HmiLib le redemande explicitement : ses en-tetes en dependent pour leur compte.
    PUBLIC nlohmann_json::nlohmann_json)

target_compile_definitions(HmiLib PUBLIC WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)

if(ENABLE_PCH)
    target_precompile_headers(HmiLib PRIVATE ${PROJECT_SOURCE_DIR}/Source/pch.h)
endif()

# Tout ce qui suit demande Qt. La découverte de Qt et l'option BUILD_EDITOR_QT vivent dans
# Source/CMakeLists.txt depuis le LOT-87 : trois répertoires frères (HMI, Ui, App) en dépendent, et
# les cibles importées d'un find_package ne sont visibles que sous le répertoire qui l'a appelé.
if(NOT JADG_GAME_ENABLED)
    return()
endif()


# =============================================================================================
# Deux applications (LOT-86)
#
#   JustAnotherRpgGame  le JEU      -- Qt Quick, QGuiApplication, ne lie PAS Qt6::Widgets ;
#   LevelEditor         l'ÉDITEUR   -- Qt Widgets, QApplication, docks et arbres.
#
# Elles partagent Core, HmiLib, Graphics, Game, Input, Audio et l'amorçage (App/Common) — tout ce
# qui n'est pas de la présentation. Elles ne partagent AUCUNE technologie d'IHM, et c'est le point :
# un widget ne peut pas réapparaître dans le jeu par inadvertance, l'édition de liens échouerait.
#
# Pourquoi séparer. L'éditeur est un outil d'auteur : docks détachables, arbres, disposition
# persistée (EX-IHM-010/011) — Qt Widgets y est le bon outil, et QML n'y apporterait rien. Les
# écrans du jeu sont l'inverse : une image agrandie d'un facteur entier, dont l'apparence doit
# pouvoir changer sans compiler. Les tenir dans une seule application obligeait à choisir une
# technologie pour deux besoins opposés — c'est de là que venaient les 2 472 lignes de
# MainWindow.cpp.
# =============================================================================================

# Traitement automatique des métacompilateurs Qt (moc/uic/rcc), pour les deux cibles.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Mises en page Designer + ressource embarquée de l'éditeur : avec le reste de l'interface, sous Source/Ui.
set(EDITOR_UI ${PROJECT_SOURCE_DIR}/Source/Ui/Editor)
set(APP_DIR ${PROJECT_SOURCE_DIR}/Source/App)

# ---------------------------------------------------------------------------------------------
# LevelEditor : l'outil d'auteur, en Qt Widgets
# ---------------------------------------------------------------------------------------------
add_executable(LevelEditor
    ${APP_DIR}/Editor/Main.cpp
    ${APP_DIR}/Common/Bootstrap.cpp
    # Pont Qt -> Key : les raccourcis remappables de l'éditeur.
    Input/QtKeyMap.cpp
    # Rendu 2D sur QRhi — cible Direct3D 11 sous Windows (EX-REN-002),
    # atteinte au travers de QRhi plutôt qu'en appelant l'API directement.
    Graphics/SpriteBatch.cpp
    Graphics/SceneResources.cpp
    Graphics/TextureLoader.cpp
    Graphics/TextureAtlas.cpp
    Graphics/TextureCache.cpp
    Graphics/SpriteRenderer.cpp
    Graphics/DraftRenderer.cpp
    # Le lieu joué par l'essai immédiat : le même rendu que le jeu (hmi::WorldSceneRenderer).
    Graphics/WorldSceneRenderer.cpp
    # Interface : le châssis d'édition (fenêtre, actions, thème) — Widgets.
    Interface/ApplicationTheme.cpp
    Interface/ThemeIcons.cpp
    Interface/EditorActions.cpp
    Interface/MainWindow.cpp
    # Éditeur de cartes
    Editor/EditorViewport.cpp
    Editor/PalettePanel.cpp
    Editor/LevelBrowserPanel.cpp
    Editor/LayersPanel.cpp
    Editor/EntityPanel.cpp
    Editor/WorldGraphView.cpp
    # Ressource embarquée (thème .qss) — traitée par AUTORCC.
    ${EDITOR_UI}/resources.qrc
    # Mises en page Qt Designer — traitées par AUTOUIC (génèrent ui_*.h).
    ${EDITOR_UI}/MainWindow.ui
    ${EDITOR_UI}/LevelBrowserPanel.ui
    ${EDITOR_UI}/LayersPanel.ui
    ${EDITOR_UI}/EntityPanel.ui
)

# Les mises en page .ui vivent dans Source/Ui/Editor, pas à côté des widgets : AUTOUIC
# doit y chercher le .ui correspondant à chaque include "ui_*.h".
set_target_properties(LevelEditor PROPERTIES AUTOUIC_SEARCH_PATHS ${EDITOR_UI})

target_link_libraries(LevelEditor PRIVATE
    HmiLib
    Qt6::Widgets
    Qt6::Gui
    # QRhi vit dans les en-têtes privés de QtGui : API privée, stable en pratique mais non couverte
    # par la garantie de compatibilité binaire — assumé, QRhiWidget n'existe pas sans.
    Qt6::GuiPrivate
    project_warnings
    project_options
)

# Racine d'inclusion Source/ (pour les includes "App/...", "HMI/..." et "Core/...").
target_include_directories(LevelEditor PRIVATE ${PROJECT_SOURCE_DIR}/Source)
# Réduit la surface de Windows.h, évite les macros min/max, API Unicode.
target_compile_definitions(LevelEditor PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)
if(ENABLE_PCH)
    target_precompile_headers(LevelEditor PRIVATE ${PROJECT_SOURCE_DIR}/Source/pch.h)
endif()

# Les types C++ que les écrans du JEU voient (module QML `Jadg.Runtime`, LOT-87) : après AUTOMOC,
# qu'ils exigent, et après HmiLib, qu'ils lient. Le jeu lui-même est dans Source/App.
add_subdirectory(Runtime)

# Shaders du pipeline 2D, compilés en .qsb par qsb (module ShaderTools) et embarqués comme
# ressource Qt. Un .qsb porte plusieurs variantes (SPIR-V, HLSL, MSL, GLSL) : c'est ce qui permet
# à QRhi de choisir son backend à l'exécution sans que le projet ait à livrer un shader par API.
#
# Sur l'éditeur seulement pour l'instant : le jeu n'a pas encore de viewport (LOT-86 étape 2, où
# SceneRenderer sera partagé et les shaders suivront sur les deux cibles).
qt6_add_shaders(LevelEditor "jadg_shaders"
    PREFIX "/shaders"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}/Graphics/Shaders"
    FILES
        Graphics/Shaders/sprite.vert
        Graphics/Shaders/sprite.frag
)

# Copie des assets à côté des exécutables, pour que le jeu et l'éditeur chargent niveaux,
# traductions et catalogues relativement à leur binaire. Les deux cibles sortent dans le même
# répertoire : une seule copie les sert toutes les deux.
#
# Cible séparée (et non des POST_BUILD) : sous Ninja, un POST_BUILD est fondu dans l'arête de build
# de l'exécutable et n'est rejoué que si celle-ci l'est — c'est-à-dire quand un .obj change.
# Modifier uniquement un fichier sous Source/Elements/ n'en touche aucun, donc l'exécutable reste
# « à jour » aux yeux de Ninja et la copie est silencieusement sautée, même via un cmake --build
# complet. Une cible dédiée sans sortie déclarée est toujours considérée sale et s'exécute donc à
# chaque build.
add_custom_target(CopyGameData ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/Source/Elements/Localization
        $<TARGET_FILE_DIR:JustAnotherRpgGame>/Localization
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/Source/Elements/Levels
        $<TARGET_FILE_DIR:JustAnotherRpgGame>/Levels
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/Source/Elements/Assets
        $<TARGET_FILE_DIR:JustAnotherRpgGame>/Assets
    # Catalogues RPG (LOT-38) : espèces, classes, historiques, compétences, règles et fiches.
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/Source/Elements/Rpg
        $<TARGET_FILE_DIR:JustAnotherRpgGame>/Rpg
    # Atlas du monde (LOT-37) et positions relevees sur les cartes de l'auteur (LOT-94, LOT-95).
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/Source/Elements/World
        $<TARGET_FILE_DIR:JustAnotherRpgGame>/World
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/Source/Elements/Maps
        $<TARGET_FILE_DIR:JustAnotherRpgGame>/Maps
    COMMENT "Copie des données de jeu (traductions, cartes, assets, catalogues RPG)")
add_dependencies(LevelEditor CopyGameData)

# Déploiement Qt : copie les DLL Qt (plateforme, styles) à côté de l'éditeur pour que
# l'utilisateur final n'ait **aucune bibliothèque à installer**. windeployqt est fourni par
# l'installation Qt (même répertoire bin que qmake). Le jeu a le sien, dans Source/App : lui seul
# a des modules QML à embarquer.
get_target_property(_qmake_exe Qt6::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_exe}" DIRECTORY)
find_program(WINDEPLOYQT_EXE windeployqt HINTS "${_qt_bin_dir}")
if(WINDEPLOYQT_EXE)
    add_custom_command(TARGET LevelEditor POST_BUILD
        COMMAND "${WINDEPLOYQT_EXE}"
            --no-translations --compiler-runtime
            "$<TARGET_FILE:LevelEditor>"
        COMMENT "Déploiement des DLL Qt + runtime à côté de l'éditeur (windeployqt)")
    # Le déploiement du jeu vise le même `bin/` : jamais en même temps (Source/CMakeLists.txt).
    set_property(TARGET LevelEditor PROPERTY JOB_POOL_LINK deploiement_qt)
else()
    message(STATUS "LevelEditor : windeployqt introuvable, DLL Qt non déployées "
        "automatiquement (exécution possible depuis un environnement Qt).")
endif()
