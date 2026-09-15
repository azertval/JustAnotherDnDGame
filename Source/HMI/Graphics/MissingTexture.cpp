// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/MissingTexture.h"

#include <algorithm>

namespace hmi {

namespace {
// Assemble une couleur RVBA (octets) en un pixel `R8G8B8A8_UNORM` (ordre memoire R,G,B,A) --
// meme convention que ProceduralAtlas.cpp, dont ce damier est la texture soeur.
constexpr std::uint32_t pack(std::uint8_t red, std::uint8_t green, std::uint8_t blue,
                             std::uint8_t alpha) {
    return static_cast<std::uint32_t>(red) | (static_cast<std::uint32_t>(green) << 8) |
           (static_cast<std::uint32_t>(blue) << 16) | (static_cast<std::uint32_t>(alpha) << 24);
}

// Magenta pur, opaque : couleur absente de toutes les palettes du jeu, donc jamais confondue avec
// un asset reel.
constexpr std::uint32_t MAGENTA = pack(255, 0, 255, 255);
// Noir opaque : contraste maximal avec le magenta, pour que le damier reste lisible en petit.
constexpr std::uint32_t BLACK = pack(0, 0, 0, 255);
}  // namespace

// Genere, en memoire, la texture de repli « asset manquant » : un damier magenta/noir.
// L'image generee (dimensions et pixels R8G8B8A8_UNORM).
ProceduralAtlasImage buildMissingTextureImage(int size) {
    const int side = (std::max)(size, 1);  // robustesse : une taille nulle n'a aucun sens ici
    ProceduralAtlasImage image;
    image.width = side;
    image.height = side;
    image.pixels.resize(static_cast<std::size_t>(side) * static_cast<std::size_t>(side));

    for (int y = 0; y < side; ++y) {
        for (int x = 0; x < side; ++x) {
            // Damier : la parite de l'indice de carreau (et non du pixel) alterne les couleurs.
            const int checkerX = x / MISSING_TEXTURE_CHECKER_SIZE;
            const int checkerY = y / MISSING_TEXTURE_CHECKER_SIZE;
            const bool magenta = ((checkerX + checkerY) % 2) == 0;
            image.pixels[(static_cast<std::size_t>(y) * static_cast<std::size_t>(side)) +
                         static_cast<std::size_t>(x)] = magenta ? MAGENTA : BLACK;
        }
    }
    return image;
}

// Message d'avertissement emis lorsqu'un asset attendu est introuvable ou refuse.
// Le message d'avertissement, en francais.
std::string missingTextureWarning(const std::string& fileName) {
    return "Texture attendue introuvable : " + fileName + " -- repli sur le damier magenta.";
}

}  // namespace hmi
