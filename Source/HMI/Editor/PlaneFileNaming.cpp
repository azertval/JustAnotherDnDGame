// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/PlaneFileNaming.h"

#include <algorithm>
#include <cctype>

#include "Core/Levels/Plane.h"

namespace hmi {

namespace {

// Un caractere accepte dans un nom de fichier de plan : memes regles que les noms de niveau.
bool isAccepted(unsigned char character) noexcept {
    return (std::isalnum(character) != 0) || character == '-' || character == '_';
}

}  // namespace

// Dimensions, en pixels, du PNG d'un plan.
PlanePixelSize planePixelSize(int widthUnits, int heightUnits, int pixelsPerUnit) noexcept {
    if (widthUnits <= 0 || heightUnits <= 0 || !core::isValidPlaneDensity(pixelsPerUnit)) {
        return PlanePixelSize{};
    }
    return PlanePixelSize{widthUnits * pixelsPerUnit, heightUnits * pixelsPerUnit};
}

// Reduit un nom libre a ce qu'un nom de fichier de plan accepte.
std::string sanitizePlaneBaseName(const std::string& name) {
    std::string result;
    result.reserve(name.size());
    for (const char character : name) {
        const auto raw = static_cast<unsigned char>(character);
        if (raw == ' ') {
            // Espace -> tiret : un nom de fichier a espaces complique toute manipulation en ligne
            // de commande, et le tiret reste lisible.
            result.push_back('-');
            continue;
        }
        if (isAccepted(raw)) {
            result.push_back(character);
        }
        // Tout le reste est RETIRE, pas remplace : un nom qui perd ses accents reste lisible, un
        // nom truffe de tirets de substitution ne l'est plus.
    }
    // Tirets de bord : sans interet, et un nom commencant par un tiret se confond avec une option
    // en ligne de commande.
    const auto notDash = [](char character) { return character != '-'; };
    const auto first = std::find_if(result.begin(), result.end(), notDash);
    const auto last = std::find_if(result.rbegin(), result.rend(), notDash).base();
    if (first >= last) {
        return {};
    }
    return std::string(first, last);
}

// Compose un nom de fichier de plan unique pour un niveau donne.
std::string uniquePlaneFileName(const std::string& levelName,
                                const std::vector<std::string>& existing) {
    const std::string base = sanitizePlaneBaseName(levelName);
    if (base.empty()) {
        return {};
    }
    const auto taken = [&existing](const std::string& candidate) {
        return std::find(existing.begin(), existing.end(), candidate) != existing.end();
    };

    std::string candidate = base + PLANE_FILE_EXTENSION;
    if (!taken(candidate)) {
        return candidate;
    }
    // Suffixe numerique croissant, jamais un identifiant aleatoire : un dossier de plans doit
    // rester lisible a l'oeil, et un plan se retrouver sans ouvrir l'editeur.
    for (std::size_t index = 2; index <= existing.size() + 2; ++index) {
        candidate = base + '-' + std::to_string(index) + PLANE_FILE_EXTENSION;
        if (!taken(candidate)) {
            return candidate;
        }
    }
    return {};  // inatteignable : la borne depasse le nombre de noms pris.
}

}  // namespace hmi
