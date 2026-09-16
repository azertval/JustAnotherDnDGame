// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/RenderMode.h"

#include <algorithm>
#include <cctype>

namespace hmi {

// Nom persiste d'un mode de rendu (valeur ecrite dans les preferences).
const char* renderModeName(RenderMode mode) noexcept {
    switch (mode) {
        case RenderMode::Physique:
            return "physique";
        case RenderMode::Texture:
            return "texture";
    }
    return renderModeName(DEFAULT_RENDER_MODE);
}

// Mode de rendu correspondant a un nom persiste. Une valeur absente, vide ou corrompue retombe sur
// le defaut : une preference illisible ne doit jamais empecher l'application de demarrer.
RenderMode renderModeFromName(const std::string& name) noexcept {
    std::string lowered = name;
    std::ranges::transform(lowered, lowered.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    if (lowered == renderModeName(RenderMode::Physique)) {
        return RenderMode::Physique;
    }
    if (lowered == renderModeName(RenderMode::Texture)) {
        return RenderMode::Texture;
    }
    return DEFAULT_RENDER_MODE;
}

}  // namespace hmi
