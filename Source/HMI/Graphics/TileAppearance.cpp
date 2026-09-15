// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/TileAppearance.h"

#include <optional>

#include "HMI/Graphics/MissingTexture.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TileAutotile.h"

namespace hmi {

// La geometrie composee doit etre identique dans les deux modes : le rendu bascule pour comparer
// l'habillage au physique, pas pour comparer deux tailles de tuiles. Cela n'est vrai que si le
// damier de repli occupe exactement une case.
static_assert(MISSING_TEXTURE_SIZE == TextureAtlas::TILE_SIZE,
              "Le damier de repli doit faire exactement une case, sinon basculer de mode change "
              "la taille des primitives composees.");

namespace {

// Damier magenta en entier : repli commun a toutes les branches qui n'aboutissent pas a un skin.
[[nodiscard]] TileAppearance missingAppearance() noexcept {
    return TileAppearance{
        .source = AppearanceSource::MissingTexture,
        .region =
            core::AtlasRegion{
                .x = 0, .y = 0, .width = MISSING_TEXTURE_SIZE, .height = MISSING_TEXTURE_SIZE},
        .skinIndex = -1};
}

// Region a echantillonner dans un skin, selon son mode de decoupage.
[[nodiscard]] core::AtlasRegion skinRegion(const SkinEntry& entry, const TileSkinTag& tag,
                                           const SkinTexture& texture) noexcept {
    constexpr int SIZE = TextureAtlas::TILE_SIZE;
    if (entry.mode == SkinMode::Bitmask16) {
        // La case depend du voisinage solide, calcule une fois a la construction de la scene.
        // Bitmask16 exclut l'animation (LOT-46 TACHE-05, limite assumee) : texture.animatedFrame
        // n'est jamais renseigne pour ce mode (voir hmi::sceneTextures), ignore ici de toute facon.
        const AutotileCell cell = autotileCell(tag.neighborMask);
        return core::AtlasRegion{
            .x = cell.column * SIZE, .y = cell.row * SIZE, .width = SIZE, .height = SIZE};
    }
    // Image courante PAR INSTANCE d'un mecanisme (LOT-47) : prioritaire sur l'horloge partagee par
    // asset ci-dessous, seule capable de distinguer deux tuiles du meme asset a des etats
    // differents (une porte ouverte a cote d'une porte fermee, par exemple).
    if (tag.animatedFrame) {
        return *tag.animatedFrame;
    }
    // Mode single anime (LOT-46 TACHE-05) : l'image COURANTE plutot que l'image entiere.
    if (texture.animatedFrame) {
        return *texture.animatedFrame;
    }
    // Mode single, non anime : l'image entiere, qui fait exactement une case (contrat d'asset
    // TileSkin).
    return core::AtlasRegion{.x = 0, .y = 0, .width = SIZE, .height = SIZE};
}

}  // namespace

// Resout l'apparence d'une entite affichee selon le mode de rendu courant (point d'appel unique).
std::optional<TileAppearance> resolveTileAppearance(RenderMode mode,
                                                    const core::AtlasRegion& physicalRegion,
                                                    const TileSkinTag* tag,
                                                    const SceneTextures& textures, bool skinVisible,
                                                    bool overrideVisible) noexcept {
    if (mode == RenderMode::Physique) {
        // Mode de reference : la region deja resolue par hmi::regionForTile a la construction de
        // la scene. Rien n'est recalcule ici -- c'est ce qui garantit l'absence de regression. Les
        // axes skin/surcharge (LOT-51) n'ont pas de sens en Physique : ignores.
        return TileAppearance{
            .source = AppearanceSource::Atlas, .region = physicalRegion, .skinIndex = -1};
    }

    // Entite non habillable (personnage, aides d'edition) : damier, quels que soient les axes
    // skin/surcharge (LOT-51) -- cette entite n'en releve pas.
    if (tag == nullptr) {
        return missingAppearance();
    }

    // Mode compose (LOT-45 inchange) : les deux axes sont visibles, c'est le seul cas ou une
    // resolution manquante retombe sur le damier plutot que de composer une primitive vide.
    const bool composeMode = skinVisible && overrideVisible;

    // Surcharge de texture par instance (EX-EDIT-043, LOT-45) : prioritaire sur le skin du type,
    // lui-meme prioritaire sur le damier -- verifiee avant tout acces au catalogue de skins.
    if (tag->overrideAsset) {
        if (!overrideVisible) {
            // Axe "surcharge" isole hors service (LOT-51) : cette entite n'appartient pas a l'axe
            // demande (skin), et l'isolement ne retombe jamais sur un autre axe -- rien a composer.
            return std::nullopt;
        }
        const int index = textures.objectIndexOf(*tag->overrideAsset);
        if (index < 0) {
            // Surcharge configuree mais asset introuvable : signal d'audit legitime meme isole
            // (c'est precisement ce qui est configure sur cet axe) -- jamais le skin du type.
            return missingAppearance();
        }
        // Image courante par instance (LOT-47), meme priorite que pour un skin ci-dessous ; a
        // defaut, l'image entiere (surcharge non animee, comportement inchange depuis LOT-45).
        constexpr int SIZE = TextureAtlas::TILE_SIZE;
        const core::AtlasRegion region =
            tag->animatedFrame ? *tag->animatedFrame
                               : core::AtlasRegion{.x = 0, .y = 0, .width = SIZE, .height = SIZE};
        return TileAppearance{
            .source = AppearanceSource::Override, .region = region, .skinIndex = index};
    }

    // Pas de surcharge : cette entite ne concerne l'axe "surcharge" en rien -- seul l'axe skin
    // decide de sa visibilite (LOT-51). Masque si l'audit isole precisement les surcharges.
    if (!skinVisible) {
        return std::nullopt;
    }

    if (textures.skinCatalog == nullptr) {
        return composeMode ? std::optional<TileAppearance>{missingAppearance()} : std::nullopt;
    }

    const std::optional<SkinEntry> entry =
        textures.skinCatalog->resolve(textures.skinSet, tag->type);
    if (!entry.has_value()) {
        // Type pas encore habille : etat normal (EX-NFR-040). En mode compose, le damier le
        // signale comme avant LOT-51 ; isole sur le skin, rien ne s'affiche -- c'est justement le
        // diagnostic recherche (« quels types manquent encore »).
        return composeMode ? std::optional<TileAppearance>{missingAppearance()} : std::nullopt;
    }

    // Skin assigne mais texture pas (encore) chargee -- fichier absent, illisible ou refuse par le
    // contrat d'asset : damier en mode compose (l'avertissement a deja ete journalise par le
    // TextureCache), rien en mode isole (meme raisonnement que ci-dessus).
    const int index = textures.skinIndexOf(entry->asset, tag->type);
    if (index < 0) {
        return composeMode ? std::optional<TileAppearance>{missingAppearance()} : std::nullopt;
    }

    return TileAppearance{
        .source = AppearanceSource::Skin,
        .region = skinRegion(*entry, *tag, textures.skins[static_cast<std::size_t>(index)]),
        .skinIndex = index};
}

// Indique si la combinaison (mode de skin, type de tuile) exclut l'animation (voir en-tete).
bool animationExcludedForTile(SkinMode mode, core::TileType type) noexcept {
    return mode == SkinMode::Bitmask16 || hasSilhouette(type);
}

}  // namespace hmi
