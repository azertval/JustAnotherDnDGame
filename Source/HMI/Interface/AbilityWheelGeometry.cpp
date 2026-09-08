// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/AbilityWheelGeometry.h"

#include <cmath>
#include <numbers>

namespace hmi {
namespace {

// --- Les rapports de la planche ---------------------------------------------------------------
//
// Tous relevés sur la feuille vierge du corpus, rapportés au côté de la roue. Aucun n'est arrondi
// à un compte rond : la planche ne l'est pas, et arrondir décalerait les médaillons de leur arc.

/// Centre de la roue : à mi-largeur, mais au-dessus de la mi-hauteur. La roue n'est pas centrée
/// verticalement dans son carré, et c'est voulu — ce qu'elle laisse dessous est l'ouverture de
/// l'arc, où le nom se pose.
constexpr float CENTER_X_RATIO = 0.5F;
constexpr float CENTER_Y_RATIO = 0.446428571F;

/// Rayon de l'arc portant les centres des six médaillons.
constexpr float ARC_RADIUS_RATIO = 0.348214286F;
/// Rayon d'un médaillon.
constexpr float SEAT_RADIUS_RATIO = 0.082142857F;
/// Rayon du portrait, au centre.
constexpr float PORTRAIT_RADIUS_RATIO = 0.205357143F;
/// Rayon de l'anneau d'entrelacs, entre le portrait et les médaillons.
constexpr float RING_RADIUS_RATIO = 0.221428571F;

/// Premier siège, en degrés depuis le haut, sens horaire. Négatif : à gauche du sommet.
constexpr float ARC_START_DEGREES = -125.0F;
/// Ouverture totale balayée par les six. Les 110 degrés restants sont l'ouverture du bas.
constexpr float ARC_SWEEP_DEGREES = 250.0F;

/// Écart angulaire entre deux sièges voisins : l'ouverture divisée par les cinq intervalles.
constexpr float SEAT_STEP_DEGREES = ARC_SWEEP_DEGREES / static_cast<float>(ABILITY_SEAT_COUNT - 1);

[[nodiscard]] float toRadians(float degrees) noexcept {
    return degrees * std::numbers::pi_v<float> / 180.0F;
}

}  // namespace

AbilityWheelLayout abilityWheelLayout(int side) noexcept {
    AbilityWheelLayout layout{};
    if (side < ABILITY_WHEEL_MINIMUM_SIZE) {
        return layout;  // `valid` reste faux : rien à peindre, et l'appelant le sait.
    }

    const float extent = static_cast<float>(side);
    const float centerX = extent * CENTER_X_RATIO;
    const float centerY = extent * CENTER_Y_RATIO;

    layout.portrait = WheelDisc{centerX, centerY, extent * PORTRAIT_RADIUS_RATIO};
    layout.ringRadius = extent * RING_RADIUS_RATIO;
    layout.arcRadius = extent * ARC_RADIUS_RATIO;
    layout.arcStartDegrees = ARC_START_DEGREES;
    layout.arcSweepDegrees = ARC_SWEEP_DEGREES;

    const float seatRadius = extent * SEAT_RADIUS_RATIO;
    for (std::size_t index = 0; index < ABILITY_SEAT_COUNT; ++index) {
        // Depuis le HAUT et en sens HORAIRE : d'où le sinus en abscisse et le cosinus retranché en
        // ordonnée. La convention de Qt (depuis trois heures, sens inverse) est celle du peintre ;
        // la table garde celle de la lecture.
        const float angle =
            toRadians(ARC_START_DEGREES + SEAT_STEP_DEGREES * static_cast<float>(index));
        layout.seats.at(index) =
            WheelDisc{centerX + layout.arcRadius * std::sin(angle),
                      centerY - layout.arcRadius * std::cos(angle), seatRadius};
    }

    layout.valid = true;
    return layout;
}

WheelSeat wheelSeatAt(std::size_t index) noexcept {
    return static_cast<WheelSeat>(index % ABILITY_SEAT_COUNT);
}

}  // namespace hmi
