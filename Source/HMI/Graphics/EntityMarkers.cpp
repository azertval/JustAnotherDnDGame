// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/EntityMarkers.h"

#include "Core/Resources/AssetMarker.h"

namespace hmi {

namespace {

[[nodiscard]] constexpr bool isLower(char c) noexcept {
    return c >= 'a' && c <= 'z';
}
[[nodiscard]] constexpr bool isUpper(char c) noexcept {
    return c >= 'A' && c <= 'Z';
}
[[nodiscard]] constexpr bool isDigit(char c) noexcept {
    return c >= '0' && c <= '9';
}

}  // namespace

std::string entityMarkerKey(std::string_view entityType) {
    std::string id;
    // Vrai quand un separateur a ete vu depuis le dernier caractere ecrit : le tiret n'est pose
    // qu'au caractere SUIVANT, ce qui elimine d'office les tirets de tete, de fin et repetes.
    bool pendingSeparator = false;
    // Dernier caractere RETENU, dans sa casse d'origine : un caractere ignore entre deux lettres ne
    // doit pas decider de la coupure (`a.B` se lit comme `aB`).
    char previous = '\0';
    for (std::size_t index = 0; index < entityType.size(); ++index) {
        const char c = entityType[index];
        if (c == '-' || c == '_' || c == ' ') {
            pendingSeparator = true;
            continue;
        }
        const bool upper = isUpper(c);
        if (!upper && !isLower(c) && !isDigit(c)) {
            continue;  // ponctuation, non-ASCII : ignore.
        }
        if (upper && !id.empty()) {
            const bool afterLowerOrDigit = isLower(previous) || isDigit(previous);
            // Fin d'un acronyme : `NPCGuard` -> la majuscule G ouvre un mot parce qu'une minuscule
            // la suit.
            const bool endsAcronym = isUpper(previous) && index + 1 < entityType.size() &&
                                     isLower(entityType[index + 1]);
            if (afterLowerOrDigit || endsAcronym) {
                pendingSeparator = true;
            }
        }
        if (pendingSeparator && !id.empty()) {
            id.push_back('-');
        }
        pendingSeparator = false;
        previous = c;
        id.push_back(upper ? static_cast<char>(c - 'A' + 'a') : c);
    }
    if (id.empty()) {
        id = ENTITY_MARKER_UNKNOWN_ID;
    }
    return std::string{ENTITY_MARKER_FAMILY} + "/" + id;
}

std::vector<std::uint32_t> markerPixelsRgba8(const core::MarkerImage& image) {
    std::vector<std::uint32_t> pixels;
    if (image.isEmpty()) {
        return pixels;
    }
    pixels.reserve(image.pixels.size());
    for (const core::MarkerColor& color : image.pixels) {
        pixels.push_back(static_cast<std::uint32_t>(color.r) |
                         (static_cast<std::uint32_t>(color.g) << 8) |
                         (static_cast<std::uint32_t>(color.b) << 16) |
                         (static_cast<std::uint32_t>(color.a) << 24));
    }
    return pixels;
}

}  // namespace hmi
