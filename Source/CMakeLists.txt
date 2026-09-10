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

# Jadg.Runtime doit être ajouté après HMI : le target JustAnotherDnDGame est alors disponible et
# le module peut déplacer ses sources C++ runtime hors de l'exécutable avant de créer son backing
# target QML. Jadg.Ui reste porté par l'exécutable et reste donc indépendant de ce module côté
# conception/Qt Design Studio.
add_subdirectory(HMI/Jadg/Runtime)
