// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "Core/World/Atlas.h"

/**
 * @file HMI/Presentation/WorldMapRegions.h
 * @brief Les régions de l'atlas posées sur la carte du monde (`LOT-87`, T3.6).
 */

namespace hmi {

/// Position d'un repère sur la carte, en fraction de sa largeur et de sa hauteur (0 à 1).
using MapAnchor = std::pair<double, double>;

/// Les ancres lues dans `world-map-regions.json`, ou la raison de l'échec.
struct RegionAnchors {
    std::map<std::string, MapAnchor> anchors;  ///< Identifiant de région → position.
    std::string error;                         ///< Vide si la lecture a abouti.

    [[nodiscard]] bool ok() const {
        return error.empty();
    }
};

/// Une région telle que la carte la montre : son repère et sa fiche.
struct MapRegion {
    std::string id;
    std::string name;
    double x = 0.0;
    double y = 0.0;
    std::string government;
    std::vector<std::string> locations;  ///< Noms des lieux, dans l'ordre de l'atlas.
};

/**
 * @brief Lit les ancres des régions.
 *
 * **Pourquoi un fichier à part.** L'atlas du `LOT-37` est extrait du livre, qui ne donne aucune
 * coordonnée : ni les régions ni les lieux n'en portent. Les ancres sont **relevées sur la carte**
 * (le centre du nom imprimé de chaque région) et vivent à côté de l'atlas plutôt que dedans : la
 * chaîne d'extraction réécrit les régions, et effacerait un champ qu'elle n'a pas produit.
 *
 * Une ancre hors de [0, 1] ou mal formée fait échouer toute la lecture.
 */
[[nodiscard]] RegionAnchors readRegionAnchors(std::string_view json);

/**
 * @brief Les régions de l'atlas qui ont une ancre, dans l'ordre de l'atlas.
 *
 * @param atlas L'atlas du `LOT-37`, dont les régions sont filtrées et ordonnées.
 * @param anchors Les ancres lues par `readRegionAnchors`, qui donnent la position de chaque région.
 * @param missing Reçoit les identifiants des régions **sans** ancre, et ceux des ancres qui ne
 *        désignent aucune région : les deux écarts se voient, plutôt qu'une région qui manque en
 *        silence sur la carte.
 */
[[nodiscard]] std::vector<MapRegion> worldMapRegions(const core::Atlas& atlas,
                                                     const RegionAnchors& anchors,
                                                     std::vector<std::string>& missing);

}  // namespace hmi
