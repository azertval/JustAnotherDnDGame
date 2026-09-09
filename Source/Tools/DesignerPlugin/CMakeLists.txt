# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

# Plugin Qt Designer : les widgets promus des `.ui` d'identité, peints dans Designer (LOT-85,
# EX-IHM-006). Sans lui, Designer ne connaît de ces widgets que leur classe de base et dessine des
# rectangles gris — une maquette qu'on ne voit pas ne s'édite pas.

# `UiPlugin` n'est pas dans qtbase : il vient du composant « Qt Designer » de l'installateur. La
# cible reste donc OPTIONNELLE, sur le même principe que BUILD_EDITOR_QT — un poste sans Designer
# construit le jeu sans avertissement inutile, il n'y édite simplement pas les planches.
find_package(Qt6 QUIET COMPONENTS UiPlugin)
if(NOT Qt6UiPlugin_FOUND)
    message(STATUS
        "Plugin Qt Designer : ignoré (composant Qt6 UiPlugin introuvable). "
        "Installer « Qt Designer » via l'installateur Qt pour éditer les .ui d'identité.")
    return()
endif()

add_library(JadgDesignerPlugin SHARED
    IdentityWidgetsPlugin.cpp
    IdentityWidgetsPlugin.h
    # La ressource embarquée, compilée ICI et non héritée : `hmi::slotIcon` lit les icônes
    # d'emplacement dans `:/resources/`, et une ressource portée par une bibliothèque STATIQUE
    # n'est pas initialisée de façon fiable. Sans cette ligne, les emplacements d'équipement
    # apparaîtraient vides dans Designer — ce qu'on lirait comme « rien d'équipé ».
    ${ELEMENTS_UI}/resources.qrc
)

target_link_libraries(JadgDesignerPlugin PRIVATE
    IdentityWidgetsLib
    Qt6::UiPlugin
    Qt6::Widgets
    Qt6::Gui
    Qt6::Svg
    project_options)

# `project_warnings` n'est PAS lié : les en-têtes de Qt Designer déclenchent des avertissements que
# ce dépôt traite en erreurs, et sur lesquels il n'a aucune prise.

# Designer cherche ses greffons dans un sous-dossier `designer/` du chemin de plugins. Produire la
# DLL directement à cet emplacement évite une étape de copie — et une copie périmée qui survivrait
# à une reconstruction.
set_target_properties(JadgDesignerPlugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qtplugins/designer
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qtplugins/designer)
