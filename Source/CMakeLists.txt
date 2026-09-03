# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

# Modules de code source
#
# Core     : logique / moteur (bibliothèque)
# HMI      : application/éditeur Qt (JustAnotherDnDGame) + rendu via QRhi (Direct3D 11 par défaut
#            sous Windows) — nécessite Qt6
# Elements : assets statiques non compilés (niveaux, localisation, UI Qt, thèmes, textures…)

add_subdirectory(Core)
add_subdirectory(HMI)
