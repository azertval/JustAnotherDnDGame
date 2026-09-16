// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <memory>

#include "Core/Ecs/Components/Sprite.h"
#include "HMI/Graphics/ProceduralAtlas.h"
#include "HMI/Graphics/RenderLayer.h"

class QRhiTexture;

/**
 * @file HMI/Graphics/TextureAtlas.h
 * @brief Atlas de textures (spritesheet) et ses régions de tuiles.
 */

namespace hmi {

struct RhiContext;

/**
 * @brief Texture d'atlas et table de régions, chargée depuis un **fichier image** avec repli
 *        **procédural** (`EX-REN-041`/`EX-REN-042`).
 *
 * À la construction, l'atlas essaie de charger `Assets/atlas.png` (à côté de l'exécutable,
 * `hmi::AssetPaths`/`hmi::TextureLoader`) : une grille de tuiles 16×16, complétée d'une **grille
 * d'images du personnage** (silhouette animée — repos, course, saut, `EX-REN-011`/`EX-REN-012`)
 * sous la grille. Si l'asset est absent ou illisible, l'atlas retombe sans plantage
 * (`EX-NFR-040`) sur une génération **procédurale** équivalente (`hmi::buildProceduralAtlasImage`,
 * couleurs distinctes par tuile, dont une avec des zones **transparentes** pour valider le rendu
 * alpha). Chaque image de personnage reste **carrée** (16×16, comme une tuile) : le rendu
 * (`SpriteRenderer`) la multiplie par `Transform::scale`, déjà non uniforme
 * (`core::playerSize()`), qui donne à elle seule la silhouette sa proportion finale deux fois plus
 * haute que large — une région déjà non carrée doublerait cet effet. L'**interface** (vue de
 * texture, dimensions, régions) est stable, indépendante du chemin (fichier ou procédural) qui
 * l'a produite. Ressources Direct3D en RAII (`ComPtr`).
 */
class TextureAtlas {
public:
    /// Nom du fichier d'atlas attendu dans le dossier d'assets (`Assets/atlas.png`).
    static constexpr const char* ATLAS_FILE_NAME = "atlas.png";

    /// Côté d'une tuile, en pixels (`EX-ARCH-021`).
    static constexpr int TILE_SIZE = 16;
    /// Nombre de tuiles par ligne et par colonne dans la grille de tuiles générée. `6` depuis
    /// l'ajout de la clé/porte verrouillée/plateforme mobile (`EX-GP-023`/`EX-GP-026`, `LOT-63`) :
    /// `5` (25 cases, depuis l'ajout des pentes/arrondis de plafond, `EX-GP-006`) ne laissait plus
    /// qu'une case libre après la clé et la porte verrouillée, insuffisant pour la plateforme.
    static constexpr int TILES_PER_SIDE = 6;
    /// Côté d'une image d'animation du personnage, en pixels. **Carrée** (= `TILE_SIZE`) : le
    /// ratio 1:2 final vient de `Transform::scale` (`core::playerSize`), pas de la région.
    static constexpr int PLAYER_FRAME_SIZE = TILE_SIZE;
    /// Nombre de colonnes de la grille d'images du personnage (largeur de la grille de tuiles,
    /// réutilisée telle quelle).
    static constexpr int PLAYER_FRAME_COLUMNS = TILES_PER_SIDE;

    /**
     * @brief Charge l'atlas (fichier, avec repli procédural) et crée la texture GPU associée.
     * @param context Interface de rendu et lot de mises à jour de l'image courante.
     */
    explicit TextureAtlas(const RhiContext& context);

    /// @return L'identité opaque de la texture d'atlas (non possédée par l'appelant).
    [[nodiscard]] TextureHandle textureHandle() const {
        return _texture.get();
    }

    /// @return Largeur de l'atlas, en pixels.
    [[nodiscard]] int width() const {
        return _width;
    }

    /// @return Hauteur de l'atlas, en pixels.
    [[nodiscard]] int height() const {
        return _height;
    }

    /**
     * @brief Région (en pixels) de la tuile à une position de la grille.
     *
     * Pure arithmétique de grille (aucun état d'instance) : `static`, testable sans GPU
     * (`EX-NFR-010`) et sans dépendre de l'origine (fichier ou procédurale) de l'atlas.
     * @param column Colonne de la tuile (0 à TILES_PER_SIDE-1).
     * @param row    Ligne de la tuile (0 à TILES_PER_SIDE-1).
     * @return La région d'atlas correspondante, en pixels.
     */
    [[nodiscard]] static core::AtlasRegion tile(int column, int row);

    /**
     * @brief Région (en pixels) d'une image d'animation du personnage, sous la grille de tuiles.
     *
     * Pure arithmétique de grille (aucun état d'instance) : `static`, testable sans GPU
     * (`EX-NFR-010`).
     * @param clip       Clip d'animation du personnage (`hmi::PlayerClipKind`, `EX-REN-012`).
     * @param frameIndex Index de l'image dans le clip (0-based).
     * @return La région d'atlas (16×16, carrée) de cette image (`EX-REN-011`).
     */
    [[nodiscard]] static core::AtlasRegion playerFrameRegion(PlayerClipKind clip, int frameIndex);

private:
    /// Essaie de charger `Assets/atlas.png`. @return true si la texture a été créée avec succès.
    bool loadFromFile(const RhiContext& context);
    /// Génère l'atlas procédural (`hmi::buildProceduralAtlasImage`) et crée la texture associée.
    void generateProcedural(const RhiContext& context);

    int _width = 0;
    int _height = 0;
    std::shared_ptr<QRhiTexture> _texture;
};

}  // namespace hmi
