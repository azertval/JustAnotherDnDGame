// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/PlayerSprite.h"

#include <algorithm>

namespace hmi {

namespace {
// Chaine de repli d'un clip demande vers le plus proche declare (voir
// hmi::resolveDeclaredPlayerClip). nullptr = "idle", dernier recours (jamais absent en pratique,
// procedural comme externe).
[[nodiscard]] const char* fallbackFor(std::string_view name) noexcept {
    if (name == "fall") {
        return "jump";
    }
    if (name == "land") {
        return "idle";
    }
    if (name == "wallslide") {
        return "jump";
    }
    if (name == "dash") {
        return "run";
    }
    if (name == "run") {
        return "idle";
    }
    if (name == "jump") {
        return "idle";
    }
    return nullptr;
}

[[nodiscard]] bool isDeclared(const std::vector<std::string>& declaredNames,
                              std::string_view name) noexcept {
    return std::ranges::any_of(declaredNames,
                               [&](const std::string& declared) { return declared == name; });
}
}  // namespace

// Calcule le quad d'affichage du personnage par ancrage centre-bas (voir en-tete).
PlayerSpriteQuad computePlayerSpriteQuad(core::Vector2 imageSizePixels, core::Vector2 hitboxSize) {
    constexpr float PIXELS_PER_UNIT = 16.0F;  // hmi::Camera2D::PIXELS_PER_UNIT (EX-ARCH-021).
    const core::Vector2 imageSizeWorld{imageSizePixels.x / PIXELS_PER_UNIT,
                                       imageSizePixels.y / PIXELS_PER_UNIT};
    // Centre-bas de l'image aligne sur le centre-bas de la hitbox : decalage horizontal centre
    // (symetrique, LOT-48 TACHE-03), decalage vertical tel que le bas de l'image coincide avec le
    // bas de la hitbox.
    const core::Vector2 offset{(hitboxSize.x - imageSizeWorld.x) * 0.5F,
                               hitboxSize.y - imageSizeWorld.y};
    return PlayerSpriteQuad{.offset = offset, .size = imageSizeWorld};
}

// Noms des clips que l'atlas procedural sait dessiner (voir en-tete).
const std::vector<std::string>& proceduralPlayerClipNames() {
    static const std::vector<std::string> names{"idle", "run", "jump"};
    return names;
}

// Resout un nom de clip demande vers le plus proche declare (voir en-tete).
std::string resolveDeclaredPlayerClip(const std::vector<std::string>& declaredNames,
                                      std::string_view requested) {
    std::string current{requested};
    // Borne la chaine de repli : strictement plus que le nombre de clips connus (7), pour ne
    // jamais boucler meme si fallbackFor() etait mal chainee par erreur future.
    for (int guard = 0; guard < 8; ++guard) {
        if (isDeclared(declaredNames, current)) {
            return current;
        }
        const char* next = fallbackFor(current);
        if (next == nullptr) {
            break;
        }
        current = next;
    }
    return "idle";  // dernier recours.
}

}  // namespace hmi
