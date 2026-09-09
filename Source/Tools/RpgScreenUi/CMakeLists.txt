# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

# Engendre UNE FOIS la planche de chaque ecran du RPG depuis la table (LOT-85, EX-IHM-006).
# Un outil et non un script : la table vit en C++, et la relire en Python en aurait fait une
# seconde verite -- meme raison que pour IdentityQss.
#
# Il ne se lance pas a la construction : les fichiers produits deviennent la propriete de qui les
# edite, et les regenerer ecraserait ces retouches. Il refuse d'ailleurs d'ecraser sans --force.
add_executable(RpgScreenUi Main.cpp)

target_link_libraries(RpgScreenUi PRIVATE
    HmiLib
    Qt6::Core
    project_warnings
    project_options)
