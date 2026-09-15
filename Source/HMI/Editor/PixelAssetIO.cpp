// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/PixelAssetIO.h"

#include "HMI/Graphics/TextureAtlas.h"

namespace hmi {

namespace {

// Progression courte (au plus quatre valeurs) de nombres de cases, en partant du minimum du
// contrat et en doublant -- assez pour couvrir les usages courants sans proposer une liste sans
// fin.
std::vector<int> tileCountProgression(int minimumTiles) {
    std::vector<int> counts;
    constexpr int MAX_TILES =
        64;  // plafond arbitraire : au-dela, une planche est deja tres grande.
    for (int tiles = minimumTiles; tiles <= MAX_TILES && counts.size() < 4; tiles *= 2) {
        counts.push_back(tiles);
    }
    if (counts.empty()) {
        counts.push_back(minimumTiles);
    }
    return counts;
}

}  // namespace

std::vector<std::pair<int, int>> validAssetSizes(AssetFamily family) {
    const AssetDimensionContract contract = assetDimensionContract(family);
    const int tileSize = TextureAtlas::TILE_SIZE;

    if (contract.exactWidth > 0 && contract.exactHeight > 0) {
        return {{contract.exactWidth, contract.exactHeight}};
    }
    if (contract.exactHeight > 0 && contract.multipleOfTileSize) {
        // Ex. TileSkin : hauteur d'une case exactement, largeur un multiple de case (skin anime).
        std::vector<std::pair<int, int>> sizes;
        for (const int tiles : tileCountProgression(contract.minimumTiles)) {
            sizes.emplace_back(tiles * tileSize, contract.exactHeight);
        }
        return sizes;
    }
    if (contract.multipleOfTileSize) {
        // Ex. AutotileSheet/Object/CharacterSheet : planche carree, un multiple de case sur les
        // deux axes.
        std::vector<std::pair<int, int>> sizes;
        for (const int tiles : tileCountProgression(contract.minimumTiles)) {
            sizes.emplace_back(tiles * tileSize, tiles * tileSize);
        }
        return sizes;
    }
    return {};  // dimensions libres (fond, police) : saisie libre a la charge de l'appelant.
}

}  // namespace hmi
