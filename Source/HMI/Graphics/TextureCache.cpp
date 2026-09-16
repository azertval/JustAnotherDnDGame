// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/TextureCache.h"

#include <utility>
#include <vector>

#include "Core/Levels/TileTypeName.h"
#include "Core/Resources/AssetMarker.h"
#include "HMI/Graphics/EntityMarkers.h"
#include "HMI/Graphics/GraphicsLog.h"
#include "HMI/Graphics/MissingTexture.h"

namespace hmi {

namespace {

// Decodage d'abord : la validation des dimensions (pure, EX-REN-007) doit s'intercaler entre
// le decodage et l'upload GPU, sinon un asset de travers est deja devenu une texture. Rend
// std::nullopt (avec journal) si l'image est illisible ou non conforme.
std::optional<DecodedImage> decodeValidatedImage(const std::filesystem::path& path,
                                                 const std::string& fileName, AssetFamily family) {
    std::optional<DecodedImage> image = decodeImageFile(path);
    if (!image) {
        GRAPHICS_LOG_WARNING("Asset " + fileName + " illisible ou format non supporte (" +
                             path.string() + ").");
        return std::nullopt;
    }

    const AssetValidation validation = validateAsset(family, fileName, image->width, image->height);
    if (!validation.valid) {
        GRAPHICS_LOG_WARNING(validation.message);
        return std::nullopt;
    }
    return image;
}

}  // namespace

// Construit un cache vide pour un device et un dossier d'assets donnes.
TextureCache::TextureCache(const RhiContext& context, AssetPaths paths)
    : _context(context), _paths(std::move(paths)) {}

// Charge et valide un asset depuis le disque, sans passer par le cache. Les trois causes d'echec
// (chemin introuvable, decodage impossible, dimensions non conformes) sont journalisees
// distinctement : le message doit dire quoi corriger, pas seulement que ca a rate.
std::optional<LoadedTexture> TextureCache::load(const std::string& fileName, AssetFamily family,
                                                std::optional<core::TileType> maskType) const {
    const std::optional<std::filesystem::path> path = _paths.resolve(fileName);
    if (!path) {
        GRAPHICS_LOG_WARNING(missingTextureWarning(fileName));
        return std::nullopt;
    }

    const std::optional<DecodedImage> image = decodeValidatedImage(*path, fileName, family);
    if (!image) {
        return std::nullopt;
    }

    // Detourage a la silhouette du type, entre validation et upload : l'artiste peint un carre
    // plein, le moteur le decoupe une fois pour toutes (LOT-42 TACHE-03). Sans effet pour un type
    // sans silhouette.
    std::vector<std::uint32_t> pixels = image->pixels;
    if (maskType) {
        applySilhouetteMask(*maskType, image->width, image->height, pixels);
    }

    std::optional<LoadedTexture> texture =
        createTexture(_context, image->width, image->height, pixels);
    if (!texture) {
        GRAPHICS_LOG_WARNING("Creation GPU de la texture " + fileName + " impossible.");
        return std::nullopt;
    }
    GRAPHICS_LOG_INFO("Texture chargee : " + fileName + " (" + std::to_string(image->width) + "x" +
                      std::to_string(image->height) + ").");
    return texture;
}

// Obtient la texture d'un asset, en la chargeant au premier acces.
// La texture chargee (propriete du cache), ou nullptr si l'asset est absent/illisible/non conforme.
const LoadedTexture* TextureCache::get(const std::string& fileName, AssetFamily family) {
    return getUnderKey(fileName, fileName, family, std::nullopt);
}

// Obtient la texture d'un fichier designe par son chemin, hors du dossier d'assets.
const LoadedTexture* TextureCache::getFromPath(const std::filesystem::path& path) {
    const std::string key = path.string();
    return _entries.getOrLoad(key, [this, &path, &key]() -> std::optional<LoadedTexture> {
        const std::optional<DecodedImage> image = decodeImageFile(path);
        if (!image) {
            GRAPHICS_LOG_WARNING("Image introuvable ou illisible : " + key + ".");
            return std::nullopt;
        }
        std::optional<LoadedTexture> texture =
            createTexture(_context, image->width, image->height, image->pixels);
        if (!texture) {
            GRAPHICS_LOG_WARNING("Creation GPU de la texture " + key + " impossible.");
            return std::nullopt;
        }
        GRAPHICS_LOG_INFO("Image chargee : " + key + " (" + std::to_string(image->width) + "x" +
                          std::to_string(image->height) + ").");
        return texture;
    });
}

// Obtient la variante d'un asset detouree a la silhouette d'un type de tuile.
const LoadedTexture* TextureCache::getMasked(const std::string& fileName, AssetFamily family,
                                             core::TileType type) {
    if (!hasSilhouette(type)) {
        return get(fileName, family);  // type carre : rien a detourer.
    }
    // Cle distincte de l'originale, et distincte d'un type a l'autre : un meme fichier peut etre
    // assigne a un type carre et a plusieurs pentes sans que les variantes se marchent dessus.
    return getUnderKey(fileName + "#" + core::tileTypeName(type), fileName, family, type);
}

// Obtient une entree du cache sous une cle donnee, en la chargeant au premier acces.
const LoadedTexture* TextureCache::getUnderKey(const std::string& cacheKey,
                                               const std::string& fileName, AssetFamily family,
                                               std::optional<core::TileType> maskType) {
    // La memoisation (et la memorisation d'un echec deja constate) est deleguee a CacheRegistry :
    // sans elle, un asset manquant relirait le disque et rejouerait son avertissement a chaque
    // image.
    return _entries.getOrLoad(cacheKey, [&] { return load(fileName, family, maskType); });
}

// Charge et valide la description d'animation d'un asset depuis le disque (sans cache).
std::optional<AnimationDescription> TextureCache::loadAnimation(const std::string& fileName,
                                                                int textureWidth,
                                                                int textureHeight) const {
    const std::string descriptorName = AnimationCatalog::descriptorFileName(fileName);
    const std::optional<std::filesystem::path> path = _paths.resolve(descriptorName);
    if (!path) {
        // Absence de fichier = image fixe : cas par defaut, silencieux (pas d'avertissement,
        // LOT-46 TACHE-03) -- a la difference d'un asset texture introuvable (load(), ci-dessus).
        return std::nullopt;
    }

    const AnimationDescriptionResult result = AnimationCatalog::loadFromFile(*path);
    if (!result.ok()) {
        GRAPHICS_LOG_WARNING("Animation " + fileName + " : " + result.error);
        return std::nullopt;
    }

    const AssetValidation validation = AnimationCatalog::validateAgainstTexture(
        *result.description, fileName, textureWidth, textureHeight);
    if (!validation.valid) {
        GRAPHICS_LOG_WARNING(validation.message);
        return std::nullopt;
    }

    GRAPHICS_LOG_INFO("Animation chargee : " + descriptorName + " (" +
                      std::to_string(result.description->clips.clipCount()) + " clip(s)).");
    return result.description;
}

// Obtient la description d'animation d'un asset, en la chargeant au premier acces.
const AnimationDescription* TextureCache::getAnimation(const std::string& fileName,
                                                       int textureWidth, int textureHeight) {
    return _animationEntries.getOrLoad(
        fileName, [&] { return loadAnimation(fileName, textureWidth, textureHeight); });
}

// Retire une entree du cache, de sorte que le prochain get relise le fichier. Invalidation
// conjointe (LOT-46 TACHE-03) : la description d'animation associee, sous la meme cle, est
// retiree en meme temps que la texture -- un seul point d'invalidation pour l'appelant.
void TextureCache::invalidate(const std::string& fileName) {
    _entries.invalidate(fileName);
    _animationEntries.invalidate(fileName);
}

// Retire toutes les entrees du cache (rechargement global), textures et animations.
void TextureCache::invalidateAll() {
    _entries.invalidateAll();
    _animationEntries.invalidateAll();
    _markerEntries.invalidateAll();
}

// Texture du marqueur genere d'une cle d'asset, creee une seule fois (voir en-tete).
const LoadedTexture* TextureCache::markerTexture(const std::string& key) {
    return _markerEntries.getOrLoad(key, [this, &key]() -> std::optional<LoadedTexture> {
        const core::MarkerImage image =
            core::assetMarker(key, ENTITY_MARKER_SIZE_PIXELS, ENTITY_MARKER_SIZE_PIXELS);
        if (image.isEmpty()) {
            // Cle malformee : core::assetMarker la refuse plutot que de lui inventer un marqueur.
            GRAPHICS_LOG_WARNING("Marqueur refuse : cle d'asset malformee '" + key + "'.");
            return std::nullopt;
        }
        std::optional<LoadedTexture> texture =
            createTexture(_context, image.width, image.height, markerPixelsRgba8(image));
        if (!texture) {
            GRAPHICS_LOG_WARNING("Creation GPU du marqueur " + key + " impossible.");
            return std::nullopt;
        }
        return texture;
    });
}

// Texture de repli en damier magenta, creee une seule fois a la demande.
const LoadedTexture* TextureCache::missingTexture() {
    if (!_missingTexture) {
        const ProceduralAtlasImage image = buildMissingTextureImage();
        _missingTexture = createTexture(_context, image.width, image.height, image.pixels);
        if (!_missingTexture) {
            GRAPHICS_LOG_ERROR("Creation de la texture de repli (damier magenta) impossible.");
            return nullptr;
        }
        GRAPHICS_LOG_INFO("Texture de repli (damier magenta) creee.");
    }
    return &*_missingTexture;
}

// Obtient la texture d'un asset, ou le repli en damier magenta a defaut (point unique).
// La texture de l'asset si disponible, sinon le damier ; nullptr si meme le damier a echoue.
const LoadedTexture* resolveOrPlaceholder(TextureCache& cache, const std::string& fileName,
                                          AssetFamily family) {
    if (const LoadedTexture* texture = cache.get(fileName, family)) {
        return texture;
    }
    // L'avertissement nommant l'asset a deja ete emis par `get` lors du premier echec ; le
    // repeter ici a chaque image noierait le journal (GraphicsLog : jamais dans un chemin par
    // image).
    return cache.missingTexture();
}

}  // namespace hmi
