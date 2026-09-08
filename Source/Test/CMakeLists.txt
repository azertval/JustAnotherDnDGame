# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

# Tests unitaires (GoogleTest).
include(GoogleTest)

add_executable(UnitTests
    # L'arborescence des tests unitaires reflète celle des sources (Source/) :
    # le test de Source/<Module>/<X> vit sous Unit/<Module>/test_<x>.cpp.
    Unit/Core/test_core.cpp
    Unit/Core/Math/test_vector2.cpp
    Unit/Core/Math/test_rect.cpp
    Unit/Core/Math/test_math_utils.cpp
    Unit/Core/Math/test_deterministic_random.cpp
    Unit/Core/Ecs/test_entity_manager.cpp
    Unit/Core/Ecs/test_animation_clip.cpp
    Unit/Core/Ecs/test_component_pool.cpp
    Unit/Core/Ecs/test_view.cpp
    Unit/Core/Ecs/test_world.cpp
    Unit/Core/Ecs/test_sprite.cpp
    Unit/Core/Ecs/test_particle_system.cpp
    Unit/Core/Ecs/test_top_down_movement.cpp
    Unit/Core/Ecs/test_particle_triggers.cpp
    Unit/Core/Data/test_json_document.cpp
    Unit/Core/Levels/test_level.cpp
    Unit/Core/Levels/test_camera_framing.cpp
    Unit/Core/Levels/test_level_loader.cpp
    Unit/Core/Levels/test_level_writer.cpp
    Unit/Core/Levels/test_map_layers.cpp
    Unit/Core/Levels/test_level_draft.cpp
    Unit/Core/Levels/test_plane.cpp
    Unit/Core/Levels/test_level_outcome.cpp
    Unit/Core/Levels/test_tile_type_name.cpp
    Unit/Core/Rpg/test_check.cpp
    Unit/Core/Rpg/test_dice.cpp
    Unit/Core/Rpg/test_equipment.cpp
    Unit/Core/Rpg/test_inventory.cpp
    Unit/Core/Combat/test_encounter.cpp
    Unit/Core/Resources/test_asset_keys.cpp
    Unit/Core/Rpg/test_rpg_enums.cpp
    Unit/Core/Rpg/test_multiclassing.cpp
    Unit/Core/Rpg/test_bestiary.cpp
    Unit/Core/World/test_atlas.cpp
    Unit/Core/Rpg/test_character_options.cpp
    Unit/Core/Rpg/test_character_sheet.cpp
    Unit/Core/Levels/test_rpg_terrain.cpp
    Unit/Core/Levels/test_tile_texture_override.cpp
    Unit/Core/Physics/test_swept_collision.cpp
    Unit/Core/Physics/test_aabb_vs_aabb.cpp
    Unit/Core/World/test_grid_distance_field.cpp
    Unit/Core/Gameplay/test_interaction.cpp
    Unit/Core/Gameplay/test_mechanism_controller.cpp
    Unit/Core/Time/test_fixed_timestep.cpp
    Unit/Core/Diagnostics/test_logger.cpp
    Unit/Core/Diagnostics/test_scoped_log_level.cpp
    Unit/Core/Diagnostics/test_sinks.cpp
    Unit/Core/Diagnostics/test_file_log_sink.cpp
    Unit/Core/Diagnostics/test_log_format.cpp
    Unit/Core/Diagnostics/test_assert.cpp
    Unit/Core/Diagnostics/test_log_level_parse.cpp
    Unit/HMI/Input/test_input_state.cpp
    Unit/HMI/Input/test_player_input_mapper.cpp
    Unit/HMI/Input/test_game_key_bindings.cpp
    Unit/HMI/Input/test_editor_key_bindings.cpp
    Unit/HMI/Input/test_key_name.cpp
    Unit/HMI/Input/test_gamepad_bindings.cpp
    Unit/HMI/Input/test_gamepad_probe.cpp
    Unit/HMI/Input/test_gamepad_button_name.cpp
    Unit/HMI/Graphics/test_camera2d.cpp
    Unit/HMI/Graphics/test_room_grid.cpp
    Unit/HMI/Graphics/test_follow_camera.cpp
    Unit/HMI/Graphics/test_camera_zones.cpp
    Unit/HMI/Graphics/test_asset_paths.cpp
    Unit/HMI/Graphics/test_procedural_atlas.cpp
    Unit/HMI/Graphics/test_tile_visuals.cpp
    Unit/HMI/Graphics/test_texture_atlas.cpp
    Unit/HMI/Graphics/test_animation_catalog.cpp
    Unit/HMI/Graphics/test_animated_tiles.cpp
    Unit/HMI/Graphics/test_player_sprite.cpp
    Unit/HMI/Graphics/test_asset_contract.cpp
    Unit/HMI/Graphics/test_missing_texture.cpp
    Unit/HMI/Graphics/test_quad_recorder.cpp
    Unit/HMI/Graphics/test_depth_sort.cpp
    Unit/HMI/Graphics/test_render_culling.cpp
    Unit/HMI/Graphics/test_background_fit.cpp
    Unit/HMI/Graphics/test_render_mode.cpp
    Unit/HMI/Graphics/test_skin_catalog.cpp
    Unit/HMI/Graphics/test_tile_autotile.cpp
    Unit/HMI/Graphics/test_tile_skin_resolution.cpp
    Unit/HMI/Graphics/test_texture_resolution.cpp
    Unit/HMI/Graphics/test_cache_registry.cpp
    Unit/HMI/Graphics/test_mechanism_visuals.cpp
    Unit/HMI/Graphics/test_mechanism_transitions.cpp
    Unit/HMI/Graphics/test_parallax.cpp
    Unit/HMI/Graphics/test_layer_visibility.cpp
    Unit/HMI/Graphics/test_plane_render.cpp
    Unit/HMI/Graphics/test_bitmap_font.cpp
    Unit/HMI/Graphics/test_text_renderer.cpp
    Unit/HMI/Graphics/test_shadow_render.cpp
    Unit/HMI/Graphics/test_render_budget.cpp
    Unit/HMI/Graphics/test_particle_render.cpp
    Unit/HMI/Graphics/test_screen_shake.cpp
    Unit/HMI/Game/test_game_hud.cpp
    Unit/HMI/Game/test_diagnostics_hud.cpp
    Unit/HMI/Game/test_game_mode.cpp
    Unit/HMI/Game/test_game_events.cpp
    Unit/HMI/Audio/test_sound_triggers.cpp
    Unit/HMI/Localization/test_localization.cpp
    Unit/HMI/Diagnostics/test_session_log.cpp
    Unit/HMI/Audio/test_sound_catalog.cpp
    Unit/HMI/Interface/test_design_tokens.cpp
    Unit/HMI/Interface/test_application_theme.cpp
    Unit/HMI/Interface/test_theme_teardown_guards.cpp
    Unit/HMI/Interface/test_editor_actions.cpp
    Unit/HMI/Interface/test_screen_flow.cpp
    Unit/HMI/Interface/test_rpg_screens.cpp
    Unit/HMI/Interface/test_character_sheet_values.cpp
    Unit/HMI/Interface/test_identity_scale.cpp
    Unit/HMI/Interface/test_editor_workspace.cpp
    Unit/HMI/Interface/test_parchment_frame.cpp
    Unit/HMI/Interface/test_parchment_ornaments.cpp
    Unit/HMI/Editor/test_thumbnail_geometry.cpp
    Unit/HMI/Editor/test_level_name_validation.cpp
    Unit/HMI/Editor/test_tile_taxonomy.cpp
    Unit/HMI/Editor/test_skin_assignments.cpp
    Unit/HMI/Editor/test_palette_appearance.cpp
    Unit/HMI/Editor/test_level_file_operations.cpp
    Unit/HMI/Editor/test_link_geometry.cpp
    Unit/HMI/Editor/test_link_gesture.cpp
    Unit/HMI/Editor/test_texture_assign_gesture.cpp
    Unit/HMI/Editor/test_asset_library.cpp
    Unit/HMI/Editor/test_asset_file_operations.cpp
    Unit/HMI/Editor/test_asset_references.cpp
    Unit/HMI/Editor/test_mechanism_animation_assignments.cpp
    Unit/HMI/Editor/test_editor_status.cpp
    Unit/HMI/Editor/test_panel_focus.cpp
    Unit/HMI/Editor/test_pixel_operations.cpp
    Unit/HMI/Editor/test_pixel_history.cpp
    Unit/HMI/Editor/test_pixel_canvas_geometry.cpp
    Unit/HMI/Editor/test_plane_reference.cpp
    Unit/HMI/Editor/test_plane_file_naming.cpp
    Unit/HMI/Editor/test_pixel_asset_io.cpp
    Unit/HMI/Editor/test_pixel_palette.cpp
    Unit/HMI/Editor/test_pixel_autotile_preview.cpp
)

# Encodage/decodage d'image (LOT-54 TACHE-01) : QImage (Qt) lit et ecrit le PNG, mais aucun device
# Direct3D n'est requis -- seul TextureLoader.cpp touche a Qt parmi les sources de UnitTests.
# Ajoute conditionnellement : sur un poste sans Qt, UnitTests doit continuer a se configurer et
# construire (meme garde que BUILD_EDITOR_QT dans Source/HMI/CMakeLists.txt, qui a deja resolu
# Qt6_DIR en cache si l'application a ete traitee avant ce dossier). Qt est toujours installe en
# CI (cf. ci.yml) : le test s'y execute donc systematiquement.
# GuiPrivate en plus de Gui depuis le LOT-69 TACHE-02 : TextureLoader.cpp cree desormais des
# QRhiTexture, et <rhi/qrhi.h> vit dans les en-tetes prives de QtGui. Le test, lui, ne touche que
# le decodage/encodage d'image -- aucune ressource GPU n'est creee sans QRhi.
find_package(Qt6 QUIET COMPONENTS Gui GuiPrivate)
if(Qt6_FOUND)
    target_sources(UnitTests PRIVATE
        Unit/HMI/Graphics/test_image_encode.cpp
        # Non-regression VISUELLE du portage QRhi (LOT-69 TACHE-02) : rendu hors ecran d'un motif
        # temoin, relu pixel par pixel. Se saute proprement si la machine n'offre aucune interface
        # QRhi -- c'est le seul test qui protege la nettete du pixel art.
        Unit/HMI/Graphics/test_rhi_offscreen.cpp
        # Table de correspondance Qt <-> Win32 (LOT-57 TACHE-04) : logique PURE, mais qui lit les
        # constantes de <qnamespace.h> -- elle depend des en-tetes Qt, jamais d'une QApplication.
        # Elle vit donc ici plutot qu'avec les autres sources HMI/Input compilees sans condition.
        Unit/HMI/Input/test_qt_key_map.cpp
        ../HMI/Input/QtKeyMap.cpp
        ../HMI/Graphics/TextureLoader.cpp
        ../HMI/Graphics/SpriteBatch.cpp)
    target_link_libraries(UnitTests PRIVATE Qt6::Gui Qt6::GuiPrivate)
    # Les shaders du pipeline sont embarques dans UnitTests comme dans l'executable : le test de
    # rendu hors ecran construit un vrai hmi::SpriteBatch, qui les charge depuis les ressources.
    qt6_add_shaders(UnitTests "unittests_shaders"
        PREFIX "/shaders"
        BASE "${PROJECT_SOURCE_DIR}/Source/HMI/Graphics/Shaders"
        FILES
            ${PROJECT_SOURCE_DIR}/Source/HMI/Graphics/Shaders/sprite.vert
            ${PROJECT_SOURCE_DIR}/Source/HMI/Graphics/Shaders/sprite.frag
    )
else()
    message(STATUS
        "UnitTests : Qt6 introuvable -- test d'encodage/decodage d'image (LOT-54) ignore.")
endif()

# Le moteur audio (LOT-60 TACHE-01) enveloppe QSoundEffect (Qt Multimedia) : sa dependance a la
# bibliotheque n'est testable qu'avec le module present. Ajoute conditionnellement, meme garde que
# Qt6::Gui/TextureLoader.cpp ci-dessus -- sur un poste sans le module, UnitTests continue de se
# configurer et de construire (DoD de la tache-01 : testable sans Qt6::Multimedia).
find_package(Qt6 QUIET COMPONENTS Multimedia)
if(Qt6_FOUND AND TARGET Qt6::Multimedia)
    target_sources(UnitTests PRIVATE
        Unit/HMI/Audio/test_audio_engine.cpp
        ../HMI/Audio/AudioEngine.cpp)
    target_link_libraries(UnitTests PRIVATE Qt6::Multimedia)
else()
    message(STATUS
        "UnitTests : Qt6::Multimedia introuvable -- test du moteur audio (LOT-60) ignore.")
endif()

target_link_libraries(UnitTests PRIVATE
    Core
    # Couche de présentation sans Qt Widgets (LOT-02) : remplace les 79 chemins `../HMI/…` que
    # cette cible relistait pour les recompiler. HmiLib propage Core, la racine Source/ et
    # nlohmann_json.
    HmiLib
    GTest::gtest_main
    project_warnings
    project_options
    # GameKeyBindings.cpp/EditorKeyBindings.cpp (ci-dessus) sont compiles directement dans cette
    # cible (comme PlayerInputMapper.cpp) et persistent en JSON. Core le propage depuis le LOT-79,
    # mais la dependance reste declaree ici : ces fichiers en dependent pour leur compte.
    nlohmann_json::nlohmann_json
)

# Chemins (source) des assets, pour vérifier que le contenu livré (niveaux, catalogues) se charge.
target_compile_definitions(UnitTests PRIVATE
    # Dossier de fixtures parcouru par les tests parametres (LOT-79) : un catalogue se teste en
    # balayant un dossier, pas en ecrivant un test par entree.
    JADG_TEST_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Fixtures"
    JADG_LEVELS_DIR="${PROJECT_SOURCE_DIR}/Source/Elements/Levels"
    JADG_LOCALIZATION_DIR="${PROJECT_SOURCE_DIR}/Source/Elements/Localization"
    # Schemas RPG (LOT-32) : le test d'enumerations lit la donnee LIVREE, pas une copie -- une
    # copie divergerait, et le test cesserait de prouver quoi que ce soit exactement quand il
    # compterait (EX-CNT-011).
    JADG_RPG_SCHEMA_DIR="${PROJECT_SOURCE_DIR}/Source/Elements/Rpg/schema"
    # Regles RPG livrees (LOT-43) : le test du multiclassage lit la table
    # d'emplacements REELLE, celle que le jeu chargera.
    JADG_RPG_RULES_DIR="${PROJECT_SOURCE_DIR}/Source/Elements/Rpg/rules"
    # Creatures livrees (LOT-33) : le test rejoue dix blocs contre des valeurs recopiees a la main
    # du PDF. Il lit les 94 fichiers REELS -- une fixture serait une copie, et une copie ne prouve
    # plus rien le jour ou la generation change.
    JADG_RPG_CREATURES_DIR="${PROJECT_SOURCE_DIR}/Source/Elements/Rpg/creatures"
    # Racine des catalogues RPG (LOT-36) : le test des options de personnage lit especes,
    # historiques et classes, ET balaie tout l'arbre pour verifier qu'aucune donnee definitive ne
    # reference une classe provisoire.
    JADG_RPG_DIR="${PROJECT_SOURCE_DIR}/Source/Elements/Rpg"
    # Atlas livre (LOT-37) : le test des regions lit les 13 regions et 94 lieux REELS, ceux que le
    # jeu chargera. Une fixture serait une copie, et une copie ne prouve plus rien le jour ou
    # l'extraction change.
    JADG_WORLD_DIR="${PROJECT_SOURCE_DIR}/Source/Elements/World"
    JADG_ASSETS_DIR="${PROJECT_SOURCE_DIR}/Source/Elements/Assets"
    JADG_AUDIO_DIR="${PROJECT_SOURCE_DIR}/Source/Elements/Audio"
    # Deux portees depuis le LOT-73 (EX-IHM-082) : les tests de theme lisent les DEUX, pour que
    # leurs garde-fous continuent de couvrir la totalite du style livre.
    JADG_THEME_IDENTITY_PATH="${PROJECT_SOURCE_DIR}/Source/Elements/Themes/theme-identity.qss"
    JADG_THEME_EDITOR_PATH="${PROJECT_SOURCE_DIR}/Source/Elements/Themes/theme-editor.qss"
    JADG_MAIN_MENU_UI_PATH="${PROJECT_SOURCE_DIR}/Source/Elements/UI/MainMenu.ui"
    JADG_OPTIONS_PAGE_UI_PATH="${PROJECT_SOURCE_DIR}/Source/Elements/UI/OptionsPage.ui"
    JADG_FR_LANG_PATH="${PROJECT_SOURCE_DIR}/Source/Elements/Localization/fr.lang"
    JADG_EN_LANG_PATH="${PROJECT_SOURCE_DIR}/Source/Elements/Localization/en.lang")
# La planche de la fiche (LOT-38) se lit desormais sous JADG_ASSETS_DIR : depuis la reprise, sa mise
# en page n'est plus un `.ui` mais la TABLE que l'ecran et le script de gravure partagent
# (Assets/UI/character-sheet-plate.json). Plus de chemin dedie, donc : c'est un asset livre.

# Tests d'intégration : plusieurs briques assemblées collaborant de bout en bout.
add_executable(IntegrationTests
    Integration/test_ecs_mouvement.cpp
    Integration/test_boucle_simulation.cpp
    Integration/test_niveau_ecs.cpp
    Integration/test_animation_personnage.cpp
)

target_link_libraries(IntegrationTests PRIVATE
    Core
    GTest::gtest_main
    project_warnings
    project_options
)

# Chemin (source) des niveaux, pour le test d'intégration « fichier livré → ECS ».
target_compile_definitions(IntegrationTests PRIVATE
    JADG_LEVELS_DIR="${PROJECT_SOURCE_DIR}/Source/Elements/Levels")

# Tests système : un parcours de jeu complet sur le contenu livré, sans la couche GPU.
add_executable(SystemTests
    Systeme/test_parcours_edition.cpp
)

target_link_libraries(SystemTests PRIVATE
    Core
    GTest::gtest_main
    project_warnings
    project_options
)

# Chemin (source) des niveaux : le parcours complet rejoue les niveaux livrés.
target_compile_definitions(SystemTests PRIVATE
    JADG_LEVELS_DIR="${PROJECT_SOURCE_DIR}/Source/Elements/Levels")

# Enregistre chaque TEST() aupres de CTest.
#
# LABELS : l'etage d'appartenance, pour pouvoir ne rejouer qu'une famille --
# `ctest --preset vs -L unitaire` pendant une boucle de developpement, les trois en CI.
# Les proprietes de `gtest_discover_tests` s'appliquent a TOUTE la cible : un etiquetage plus fin
# (les quelques tests d'entrainement IA qui pesent des dizaines de secondes) se fait par nom, ces
# tests etant tous nommes d'apres leur sujet -- p. ex.
# `ctest --preset vs -E 'Trainer|Comparator|CrossLevel|NonRegression'`.
#
# TIMEOUT : une borne de TERMINAISON, pas un objectif de performance. Sans elle, un test parti en
# boucle bloque le job jusqu'au delai global du runner, sans dire lequel. Le seuil laisse une marge
# large au test le plus lourd connu (~42 s en Debug) : il n'attrape qu'un veritable emballement.
#
# DISCOVERY_MODE PRE_TEST : la decouverte a lieu au moment du `ctest`, pas a la fin du build.
#
# En mode POST_BUILD (le defaut), CMake execute l'executable de test des qu'il est lie, pour en
# lister les TEST(). Sous Ninja, qui parallelise agressivement, cette execution tombe dans la
# meme fenetre que le `windeployqt` de l'application : les deux visent `bin/`, l'un ecrit les DLL
# Qt pendant que l'autre les charge. Resultat, un STATUS_SHARING_VIOLATION (0xc0000043) au
# lancement -- « Error running test executable », sans le moindre rapport avec le code teste.
#
# La course preexistait ; le LOT-02 l'a rendue visible en decalant l'ordre d'edition de liens
# (UnitTests attend desormais HmiLib). Un simple reordonnancement ne ferait que la recacher :
# PRE_TEST la supprime, la decouverte n'ayant plus lieu pendant le build. Le generateur Visual
# Studio n'etait pas touche, ses cibles ne se recouvrant pas de la meme facon -- exactement le
# genre d'ecart entre generateurs que le job `build-ninja` existe pour attraper.
gtest_discover_tests(UnitTests DISCOVERY_MODE PRE_TEST PROPERTIES LABELS unitaire TIMEOUT 300)
gtest_discover_tests(IntegrationTests DISCOVERY_MODE PRE_TEST
                     PROPERTIES LABELS integration TIMEOUT 300)
gtest_discover_tests(SystemTests DISCOVERY_MODE PRE_TEST PROPERTIES LABELS systeme TIMEOUT 900)
