// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Core/Levels/CameraFraming.h"

#include <cstddef>

namespace core {

// Nom JSON du mode (voir en-tete).
std::string_view cameraFramingModeName(CameraFramingMode mode) noexcept {
    switch (mode) {
        case CameraFramingMode::WholeLevel:
            return "wholeLevel";
        case CameraFramingMode::PerRoom:
            return "perRoom";
        case CameraFramingMode::Follow:
            return "follow";
    }
    return "wholeLevel";
}

// Reconnait le nom JSON d'un mode (voir en-tete).
std::optional<CameraFramingMode> parseCameraFramingMode(std::string_view name) noexcept {
    if (name == "wholeLevel") {
        return CameraFramingMode::WholeLevel;
    }
    if (name == "perRoom") {
        return CameraFramingMode::PerRoom;
    }
    if (name == "follow") {
        return CameraFramingMode::Follow;
    }
    return std::nullopt;
}

// Regle de repli (EX-LVL-006, voir en-tete) : reproduit EXACTEMENT la regle historique, deduite
// de la comparaison entre les dimensions du niveau et la taille de salle par defaut -- jamais une
// salle personnalisee, puisqu'un niveau sans champ declare n'en a par definition pas.
CameraFramingConfig resolveCameraFraming(const std::optional<CameraFramingConfig>& declared,
                                         int levelWidth, int levelHeight) noexcept {
    if (declared) {
        return *declared;
    }
    if (levelWidth <= DEFAULT_ROOM_WIDTH_TILES && levelHeight <= DEFAULT_ROOM_HEIGHT_TILES) {
        return CameraFramingConfig{.mode = CameraFramingMode::WholeLevel,
                                   .roomWidthTiles = std::nullopt,
                                   .roomHeightTiles = std::nullopt,
                                   .zones = {}};
    }
    return CameraFramingConfig{.mode = CameraFramingMode::PerRoom,
                               .roomWidthTiles = std::nullopt,
                               .roomHeightTiles = std::nullopt,
                               .zones = {}};
}

namespace {

// Une zone dessinee a la main doit etre entierement dans le niveau et de taille non nulle.
std::optional<std::string> validateCameraZone(const CameraZone& zone, std::size_t index,
                                              int levelWidth, int levelHeight) {
    const std::string prefix = "cameraFraming.zones[" + std::to_string(index) + "]";
    if (zone.width <= 0) {
        return prefix + ".width doit etre superieur a zero";
    }
    if (zone.height <= 0) {
        return prefix + ".height doit etre superieur a zero";
    }
    if (zone.x < 0 || zone.y < 0) {
        return prefix + " a une position negative";
    }
    if (zone.x + zone.width > levelWidth) {
        return prefix + " depasse la largeur du niveau";
    }
    if (zone.y + zone.height > levelHeight) {
        return prefix + " depasse la hauteur du niveau";
    }
    return std::nullopt;
}

}  // namespace

// Valide un cadrage declare (EX-LVL-004, voir en-tete).
std::optional<std::string> validateCameraFramingConfig(const CameraFramingConfig& config,
                                                       int levelWidth, int levelHeight) {
    // Zones dessinees a la main : mode PerRoom uniquement, chacune entierement dans le niveau et
    // de taille non nulle.
    if (config.mode != CameraFramingMode::PerRoom && !config.zones.empty()) {
        return "cameraFraming.zones renseigne pour un mode qui n'est pas 'perRoom'";
    }
    for (std::size_t index = 0; index < config.zones.size(); ++index) {
        if (std::optional<std::string> error =
                validateCameraZone(config.zones[index], index, levelWidth, levelHeight)) {
            return error;
        }
    }

    // Taille de vue : mode PerRoom (salle de la grille automatique) ou Follow (zone de suivi).
    if (config.mode != CameraFramingMode::PerRoom && config.mode != CameraFramingMode::Follow) {
        if (config.roomWidthTiles) {
            return "cameraFraming.roomWidthTiles renseigne pour un mode qui n'est ni 'perRoom' ni "
                   "'follow'";
        }
        if (config.roomHeightTiles) {
            return "cameraFraming.roomHeightTiles renseigne pour un mode qui n'est ni 'perRoom' "
                   "ni 'follow'";
        }
        return std::nullopt;
    }
    if (config.roomWidthTiles) {
        if (*config.roomWidthTiles <= 0) {
            return "cameraFraming.roomWidthTiles doit etre superieur a zero";
        }
        if (*config.roomWidthTiles > levelWidth) {
            return "cameraFraming.roomWidthTiles depasse la largeur du niveau";
        }
    }
    if (config.roomHeightTiles) {
        if (*config.roomHeightTiles <= 0) {
            return "cameraFraming.roomHeightTiles doit etre superieur a zero";
        }
        if (*config.roomHeightTiles > levelHeight) {
            return "cameraFraming.roomHeightTiles depasse la hauteur du niveau";
        }
    }
    return std::nullopt;
}

}  // namespace core
