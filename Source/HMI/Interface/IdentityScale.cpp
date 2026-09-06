// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/IdentityScale.h"

#include <algorithm>

namespace hmi {

int identityScaleFor(int windowLogicalHeight) noexcept {
    // Division ENTIERE, jamais un arrondi : arrondir ferait passer une fenetre de 700 px a
    // l'echelle 2, pour laquelle il manque 20 px de hauteur -- la derniere entree du menu
    // disparaitrait sous le bord.
    const int fitted = windowLogicalHeight / IDENTITY_BASE_HEIGHT;
    return std::clamp(fitted, 1, IDENTITY_MAX_SCALE);
}

int identityScaleForDisplay(int windowLogicalHeight, int availableLogicalHeight) noexcept {
    const int fromWindow = identityScaleFor(windowLogicalHeight);
    // Zone disponible inconnue : aucune borne a appliquer. Le cas se produit avant que la fenetre
    // ne soit rattachee a un ecran, ou sur une plate-forme qui ne sait pas le dire.
    if (availableLogicalHeight <= 0) {
        return fromWindow;
    }
    // Le MINIMUM des deux, jamais la seule zone disponible : une fenetre petite sur un grand ecran
    // doit rester a son facteur, sans quoi elle rendrait une maquette plus grande qu'elle.
    return std::min(fromWindow, identityScaleFor(availableLogicalHeight));
}

}  // namespace hmi
