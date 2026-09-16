// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/CameraZones.h"

namespace hmi {

// Indice de la zone active a une position (voir en-tete) : premiere zone de la liste qui contient
// la position (ordre = priorite).
std::optional<std::size_t> activeCameraZoneIndex(const std::vector<core::CameraZone>& zones,
                                                 core::GridPosition position) noexcept {
    for (std::size_t index = 0; index < zones.size(); ++index) {
        const core::CameraZone& zone = zones[index];
        const bool inside = position.column >= zone.x && position.column < zone.x + zone.width &&
                            position.row >= zone.y && position.row < zone.y + zone.height;
        if (inside) {
            return index;
        }
    }
    return std::nullopt;
}

}  // namespace hmi
