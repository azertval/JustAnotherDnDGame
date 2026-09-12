# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

# Jadg.Runtime : ce que le jeu SAIT DIRE, exposé au QML (LOT-87).
#
# Les vues-modèles (fiche, inventaire, réglages, navigation, données en attente) et la surface de
# rendu Qt Quick. C'est le seul module QML du projet qui contienne du C++, et c'est précisément
# pour cela qu'il est à part : Qt Design Studio ne charge aucun plugin C++ dans son marionnettiste
# (`qmlpuppet`), donc tout type C++ mêlé à un module de formulaires rend ce module irrésolvable dans
# l'atelier. Ici, les formulaires (`Jadg.Ui`) n'importent jamais ce module ; seuls les jumeaux de
# câblage (`Jadg.App`) le font, et l'atelier le remplace par des doublures (Source/Ui/Mocks/).
#
# Bibliothèque STATIQUE liée par le jeu. Les types s'enregistrent par le fichier engendré par
# `qmltyperegistrar` (les macros QML_ELEMENT / QML_SINGLETON des en-têtes) : rien n'est enregistré
# à la main, et ajouter un type se fait en écrivant la macro dans son en-tête.
#
# Les vues-modèles ne connaissent ni Quick ni Widgets (EX-IHM-101, `check_ui_layers.py`) ; seule
# `GameViewportItem`, la surface de rendu, lie Quick -- c'est un item de scène, pas un modèle.

qt_add_qml_module(JadgRuntime
    URI Jadg.Runtime
    VERSION 1.0
    STATIC
    SOURCES
        OptionsModel.h OptionsModel.cpp
        ScreenRouter.h ScreenRouter.cpp
        PendingData.h PendingData.cpp
        CharacterSheetModel.h CharacterSheetModel.cpp
        InventoryModel.h InventoryModel.cpp
        SheetRowModel.h SheetRowModel.cpp
        RuleLabels.h RuleLabels.cpp
        DemonstrationCharacter.h DemonstrationCharacter.cpp
        GameViewportItem.h GameViewportItem.cpp
)

# Le fichier d'enregistrement des types est ENGENDRÉ et inclut les en-têtes par NOM DE BASE
# (`#if __has_include(<OptionsModel.h>)`), pas par le chemin « HMI/Runtime/… » qu'emploie le reste
# du projet. Sans ce répertoire dans les chemins d'inclusion, le `__has_include` échoue en silence
# et la compilation du fichier engendré échoue sur un identificateur que personne n'a écrit.
target_include_directories(JadgRuntime
    PUBLIC ${PROJECT_SOURCE_DIR}/Source
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(JadgRuntime PUBLIC WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)
target_link_libraries(JadgRuntime
    PUBLIC HmiLib Qt6::Qml Qt6::Quick Qt6::Gui Qt6::GuiPrivate
    PRIVATE project_warnings project_options)
