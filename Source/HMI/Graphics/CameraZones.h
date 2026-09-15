// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "Core/Levels/CameraFraming.h"
#include "Core/Levels/GridPosition.h"

/**
 * @file HMI/Graphics/CameraZones.h
 * @brief Résolution de la zone de caméra active parmi des zones dessinées à la main (mode *par
 *        salle*, `core::CameraZone`).
 */

namespace hmi {

/**
 * @brief Indice de la zone active de @p zones à @p position.
 *
 * Fonction **pure**, testable sans GPU (`EX-NFR-004`), même patron que `hmi::RoomGrid` pour le
 * découpage automatique. La **première** zone de la liste qui contient @p position est retenue :
 * l'ordre porte donc la priorité en cas de chevauchement — pas de règle de résolution
 * supplémentaire à inventer.
 * @param zones    Zones de caméra du niveau (`core::CameraFramingConfig::zones`).
 * @param position Position (case) dont on cherche la zone de caméra active.
 * @return L'indice dans @p zones de la première zone contenant @p position ; `std::nullopt` si
 *         aucune zone ne la contient (repli sur le cadrage niveau entier, côté appelant).
 */
[[nodiscard]] std::optional<std::size_t> activeCameraZoneIndex(
    const std::vector<core::CameraZone>& zones, core::GridPosition position) noexcept;

}  // namespace hmi
