// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/AssetContract.h"

#include "HMI/Graphics/TextureAtlas.h"

namespace hmi {

// Nom lisible d'une famille d'assets, utilise dans les messages de journalisation.
const char* assetFamilyName(AssetFamily family) noexcept {
    switch (family) {
        case AssetFamily::Atlas:
            return "Atlas";
        case AssetFamily::TileSkin:
            return "Skin de tuile";
        case AssetFamily::AutotileSheet:
            return "Planche a raccords";
        case AssetFamily::Background:
            return "Fond de niveau";
        case AssetFamily::Object:
            return "Objet interactif";
        case AssetFamily::CharacterSheet:
            return "Spritesheet de personnage";
        case AssetFamily::Font:
            return "Police bitmap";
    }
    return "Inconnu";
}

// Contrat de dimensions d'une famille d'assets. Les contraintes sont exprimees en **cases**
// (TextureAtlas::TILE_SIZE) plutot qu'en pixels partout ou l'asset est destine a etre decoupe en
// grille : c'est le decoupage, pas une taille absolue, qui rend un asset utilisable.
AssetDimensionContract assetDimensionContract(AssetFamily family) noexcept {
    AssetDimensionContract contract;
    switch (family) {
        case AssetFamily::Atlas:
            // Grille de tuiles (TILES_PER_SIDE de cote) + lignes d'images du personnage sous la
            // grille : la hauteur depend du nombre d'images, seul le decoupage en cases est exige.
            contract.multipleOfTileSize = true;
            contract.minimumTiles = TextureAtlas::TILES_PER_SIDE;
            return contract;
        case AssetFamily::TileSkin:
            // Une case de haut, un multiple de case en largeur : le cas historique (une seule
            // case, LOT-42) est le cas particulier a une image ; un skin ANIME (LOT-46) est une
            // spritesheet horizontale sur ce meme rang, plusieurs images de large. La coherence
            // fine avec un eventuel fichier d'animation (nombre exact d'images, indices
            // references) releve de hmi::AnimationCatalog::validateAgainstTexture, en aval.
            contract.exactHeight = TextureAtlas::TILE_SIZE;
            contract.multipleOfTileSize = true;
            contract.minimumTiles = 1;
            return contract;
        case AssetFamily::AutotileSheet:
            // Planche a raccords bitmask (LOT-42) : au moins 4x4 cases pour couvrir les 16
            // combinaisons de voisinage cardinal.
            contract.multipleOfTileSize = true;
            contract.minimumTiles = 4;
            return contract;
        case AssetFamily::Object:
        case AssetFamily::CharacterSheet:
            // Grille d'images de la taille d'une case : le decoupage impose le multiple, le
            // nombre d'images reste libre.
            contract.multipleOfTileSize = true;
            contract.minimumTiles = 1;
            return contract;
        case AssetFamily::Background:
        case AssetFamily::Font:
            // Dimensions libres : le fond est etire sur les bornes du niveau (LOT-44) et un atlas
            // de police est decoupe par ses metriques plutot que par une grille de cases
            // (LOT-52). Seule la positivite est exigee.
            return contract;
    }
    return contract;
}

// Description en francais des dimensions attendues, telle qu'elle apparait dans le message d'un
// asset refuse. Derivee du contrat (jamais redigee a la main cote appelant) pour que verdict et
// message ne puissent pas diverger.
std::string describeContract(const AssetDimensionContract& contract) {
    if (contract.exactWidth > 0 && contract.exactHeight > 0) {
        return std::to_string(contract.exactWidth) + "x" + std::to_string(contract.exactHeight) +
               " px";
    }
    if (contract.multipleOfTileSize) {
        const int minimumSide = contract.minimumTiles * TextureAtlas::TILE_SIZE;
        return "des dimensions multiples de " + std::to_string(TextureAtlas::TILE_SIZE) +
               " px, au moins " + std::to_string(contract.minimumTiles) + "x" +
               std::to_string(contract.minimumTiles) + " cases (" + std::to_string(minimumSide) +
               "x" + std::to_string(minimumSide) + " px)";
    }
    return "des dimensions strictement positives";
}

// Valide les dimensions d'un asset decode contre le contrat de sa famille (EX-REN-007).
// Le verdict, accompagne d'un message nommant le fichier, le trouve et l'attendu si refuse.
AssetValidation validateAsset(AssetFamily family, const std::string& fileName, int width,
                              int height) {
    const AssetDimensionContract contract = assetDimensionContract(family);
    const int tileSize = TextureAtlas::TILE_SIZE;

    // Un seul point de decision : chaque regle du contrat est evaluee ici, dans l'ordre du plus
    // grossier (positivite) au plus fin (nombre minimal de cases).
    bool conforming = width > 0 && height > 0;
    if (conforming && contract.exactWidth > 0) {
        conforming = width == contract.exactWidth;
    }
    if (conforming && contract.exactHeight > 0) {
        conforming = height == contract.exactHeight;
    }
    if (conforming && contract.multipleOfTileSize) {
        conforming = width % tileSize == 0 && height % tileSize == 0 &&
                     width / tileSize >= contract.minimumTiles &&
                     height / tileSize >= contract.minimumTiles;
    }

    if (conforming) {
        return AssetValidation{.valid = true, .message = std::string{}};
    }
    return AssetValidation{.valid = false,
                           .message = "Asset " + fileName + " (" + assetFamilyName(family) +
                                      ") refuse : dimensions " + std::to_string(width) + "x" +
                                      std::to_string(height) + " px, attendu " +
                                      describeContract(contract) + "."};
}

}  // namespace hmi
