# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

# Résolution de la feuille d'identité pour Qt Designer (LOT-85, EX-IHM-006).
# Un exécutable et non un script : la substitution des marqueurs vit dans le code du jeu, et la
# réécrire ailleurs en aurait fait une seconde vérité.
add_executable(IdentityQss
    Main.cpp
    # `identityStyleSheet()` lit :/resources/theme-identity.qss. Compilée ici pour la raison déjà
    # dite au plugin : une ressource portée par une bibliothèque STATIQUE n'est pas initialisée.
    ${ELEMENTS_UI}/resources.qrc
)

target_link_libraries(IdentityQss PRIVATE
    IdentityWidgetsLib
    Qt6::Widgets
    Qt6::Gui
    project_warnings
    project_options)

# Les fontes se lisent dans `Assets/Fonts` À CÔTÉ de l'exécutable (`hmi::applyFont`). Laisser cet
# outil dans le même `bin/` que le jeu est donc ce qui lui fait résoudre les MÊMES familles —
# ailleurs, il écrirait une feuille nommant des polices de repli.
