// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/SpriteRenderer.h"

#include "Core/Ecs/Systems/AnimationSystem.h"
#include "Core/Ecs/World.h"
#include "Core/Levels/Level.h"
#include "HMI/Graphics/AnimationCatalog.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/GraphicsLog.h"
#include "HMI/Graphics/ParticleRenderer.h"
#include "HMI/Graphics/PlaneVisuals.h"
#include "HMI/Graphics/PlayerSprite.h"
#include "HMI/Graphics/ShadowRenderer.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TextureCache.h"

namespace hmi {

// Soumet une scene composee au pipeline de dessin, une passe par groupe de texture.
void submitComposedScene(SpriteBatch& batch, const DirectX::XMFLOAT4X4& projection,
                         const ComposedScene& scene) {
    const std::vector<ComposedQuad>& quads = scene.quads();
    TextureHandle current = nullptr;
    bool open = false;

    for (const ComposedQuad& composed : quads) {
        if (composed.texture == nullptr) {
            continue;  // primitive sans texture liee : rien a dessiner (robustesse).
        }
        if (!open || composed.texture != current) {
            if (open) {
                batch.end();
            }
            // L'identite opaque traverse la chaine telle quelle : ni la composition ni la
            // soumission ne connaissent le type reel de la texture (cf. hmi::TextureHandle).
            batch.begin(projection, composed.texture);
            current = composed.texture;
            open = true;
        }
        if (composed.kind == QuadKind::Sprite) {
            batch.draw(composed.sprite);
        } else {
            batch.draw(composed.line);
        }
    }

    if (open) {
        batch.end();
    }
}

// Resout les images des plans d'un niveau (acces disque/GPU), dans l'ordre de la liste.
// Implemente ICI et non dans PlaneVisuals.cpp, exactement comme resolveBackgroundTexture vis-a-vis
// de composeBackground : la composition doit rester compilable et testable sans GPU (EX-NFR-004),
// ce qu'un appel au cache de textures interdirait.
std::vector<PlaneTexture> resolvePlaneTextures(TextureCache& cache,
                                               const std::filesystem::path& directory,
                                               const std::vector<core::Plane>& planes) {
    std::vector<PlaneTexture> textures;
    textures.reserve(planes.size());
    for (const core::Plane& plane : planes) {
        const LoadedTexture* loaded = cache.getFromPath(directory / plane.fileName);
        if (loaded == nullptr) {
            // Image absente ou illisible : le damier partage rend le manque VISIBLE plutot que
            // silencieux (EX-NFR-040). L'avertissement a deja ete journalise par le cache.
            loaded = cache.missingTexture();
        }
        if (loaded == nullptr) {
            textures.push_back(PlaneTexture{});  // meme le damier a echoue : plan muet.
            continue;
        }
        textures.push_back(PlaneTexture{
            .texture = loaded->handle(), .width = loaded->width, .height = loaded->height});
    }
    return textures;
}

// Textures liables par la composition d'une scene : atlas, damier de repli et skins (point unique).
SceneTextures sceneTextures(
    const TextureAtlas& atlas, TextureCache& cache, const SkinCatalog* skins,
    const std::string& skinSet, const std::vector<core::TileTextureOverride>& textureOverrides,
    const std::unordered_map<std::string, core::Animation>& tileAnimations) {
    SceneTextures textures;
    textures.atlas = atlas.textureHandle();
    textures.atlasWidth = atlas.width();
    textures.atlasHeight = atlas.height();
    // Resolution a la demande : en mode Physique, le damier n'est jamais cree.
    if (const LoadedTexture* missing = cache.missingTexture()) {
        textures.missing = missing->handle();
        textures.missingWidth = missing->width;
        textures.missingHeight = missing->height;
    }

    // Spritesheet externe du personnage (LOT-48), independamment du catalogue de skins : le
    // personnage n'est pas une tuile, sa resolution vit hors de hmi::resolveTileAppearance
    // (GameSession::refreshPlayerSprite compose PlayerSpriteTag a partir de cette texture, avec
    // repli sur l'atlas si absente/invalide -- meme avertissement deja journalise par le cache).
    if (const LoadedTexture* sheet =
            cache.get(PLAYER_SUBDIRECTORY + PLAYER_SHEET_FILE_NAME, AssetFamily::CharacterSheet)) {
        textures.characterSheet = sheet->handle();
        textures.characterSheetWidth = sheet->width;
        textures.characterSheetHeight = sheet->height;
    }

    // Charge les surcharges de texture par instance (EX-EDIT-043, LOT-45), independamment du
    // catalogue de skins : plusieurs cases peuvent partager le meme asset, charge une seule fois.
    for (const core::TileTextureOverride& override : textureOverrides) {
        if (textures.objectIndexOf(override.assetName) >= 0) {
            continue;  // asset deja charge.
        }
        const LoadedTexture* loaded =
            cache.get(OBJECTS_SUBDIRECTORY + override.assetName, AssetFamily::Object);
        if (loaded == nullptr) {
            continue;  // absent/illisible/refuse : la resolution retombera sur le damier.
        }
        textures.objects.push_back(SkinTexture{.asset = override.assetName,
                                               .maskType = std::nullopt,
                                               .texture = loaded->handle(),
                                               .width = loaded->width,
                                               .height = loaded->height});
    }

    textures.skinCatalog = skins;
    textures.skinSet = skinSet;
    if (skins == nullptr) {
        return textures;
    }

    // Charge les skins du jeu courant. Le TextureCache ne relit le disque qu'au premier acces et
    // memorise aussi les echecs : cet appel par image reste donc une recherche en table. Un asset
    // absent ou refuse par le contrat n'est simplement pas ajoute -- la resolution le verra
    // manquant et retombera sur le damier, apres l'unique avertissement deja journalise.
    const std::string& effectiveSet = skinSet.empty() ? skins->defaultSetName() : skinSet;
    for (const auto& [type, entry] : skins->assignments(effectiveSet)) {
        if (textures.skinIndexOf(entry.asset, type) >= 0) {
            continue;  // variante deja chargee : plusieurs types carres peuvent partager un asset.
        }
        const AssetFamily family =
            entry.mode == SkinMode::Bitmask16 ? AssetFamily::AutotileSheet : AssetFamily::TileSkin;
        // Un type a silhouette recoit l'image detouree, distincte de l'originale et calculee une
        // seule fois (LOT-42 TACHE-03).
        const LoadedTexture* loaded =
            cache.getMasked(SKINS_SUBDIRECTORY + entry.asset, family, type);
        if (loaded == nullptr) {
            continue;
        }
        const std::optional<core::TileType> maskType =
            hasSilhouette(type) ? std::optional<core::TileType>{type} : std::nullopt;

        // Image COURANTE d'un asset anime (LOT-46 TACHE-05), uniquement en mode Single sans
        // silhouette : bitmask16 et le detourage de silhouette excluent l'animation (limite
        // assumee, cf. GameSession::updateTileAnimations qui la signale). L'horloge partagee est
        // deja avancee au pas fixe ; ici on ne fait QUE traduire son etat courant en region.
        std::optional<core::AtlasRegion> animatedFrame;
        if (!animationExcludedForTile(entry.mode, type)) {
            const auto animationEntry = tileAnimations.find(entry.asset);
            if (animationEntry != tileAnimations.end()) {
                if (const AnimationDescription* description = cache.getAnimation(
                        SKINS_SUBDIRECTORY + entry.asset, loaded->width, loaded->height)) {
                    animatedFrame =
                        AnimationCatalog::currentFrameRegion(*description, animationEntry->second);
                }
            }
        }

        textures.skins.push_back(SkinTexture{.asset = entry.asset,
                                             .maskType = maskType,
                                             .texture = loaded->handle(),
                                             .width = loaded->width,
                                             .height = loaded->height,
                                             .animatedFrame = animatedFrame});
    }
    return textures;
}

// Resout la texture de fond d'un niveau (acces TextureCache/GPU) ; nullptr si aucun fond n'est
// designe -- distinct du repli en damier, deja gere par resolveOrPlaceholder si un fond est
// designe mais introuvable.
BackgroundTexture resolveBackgroundTexture(const std::optional<std::string>& background,
                                           TextureCache& cache) {
    if (!background) {
        return {};
    }
    const LoadedTexture* texture = resolveOrPlaceholder(
        cache, BACKGROUNDS_SUBDIRECTORY + *background, AssetFamily::Background);
    if (texture == nullptr) {
        return {};  // meme le damier de repli n'a pas pu etre cree (device perdu).
    }
    return BackgroundTexture{
        .texture = texture->handle(), .width = texture->width, .height = texture->height};
}

// Avance l'horloge d'animation partagee des tuiles animees d'un jeu de skins courant (voir
// en-tete).
void advanceTileAnimations(const SkinCatalog* skins, const std::string& skinSet,
                           TextureCache& cache, float deltaSeconds,
                           std::unordered_map<std::string, core::Animation>& tileAnimations,
                           std::set<std::string>& warnedExclusions) {
    if (skins == nullptr) {
        return;
    }
    const std::string& effectiveSet = skinSet.empty() ? skins->defaultSetName() : skinSet;
    for (const auto& [type, entry] : skins->assignments(effectiveSet)) {
        const AssetFamily family =
            entry.mode == SkinMode::Bitmask16 ? AssetFamily::AutotileSheet : AssetFamily::TileSkin;
        const LoadedTexture* loaded = cache.get(SKINS_SUBDIRECTORY + entry.asset, family);
        if (loaded == nullptr) {
            continue;  // asset absent/illisible/refuse : deja journalise par le TextureCache.
        }
        const AnimationDescription* description =
            cache.getAnimation(SKINS_SUBDIRECTORY + entry.asset, loaded->width, loaded->height);
        if (description == nullptr) {
            continue;  // pas de fichier d'animation : image fixe, cas par defaut silencieux.
        }

        // bitmask16 et silhouette detouree excluent l'animation (limite assumee, epic LOT-46
        // TACHE-05) : signale UNE fois par asset plutot que silencieusement ignore.
        if (animationExcludedForTile(entry.mode, type)) {
            if (warnedExclusions.insert(entry.asset).second) {
                GRAPHICS_LOG_WARNING(
                    "Animation de '" + entry.asset + "' ignoree : combinaison non supportee (" +
                    std::string(entry.mode == SkinMode::Bitmask16 ? "mode bitmask16"
                                                                  : "silhouette detouree") +
                    " + animation, LOT-46).");
            }
            continue;
        }

        core::Animation& animation = tileAnimations[entry.asset];
        if (!animation.clips) {
            // Premiere rencontre de cet asset : associe son jeu de clips (copie immuable,
            // partagee par toutes les tuiles de ce type via sceneTextures -- LOT-46 TACHE-05).
            animation.clips = std::make_shared<core::ClipSet>(description->clips);
        }
        core::advanceAnimation(animation, deltaSeconds);
    }
}

// Construit le rendu de sprites.
SpriteRenderer::SpriteRenderer(SpriteBatch& batch, const TextureAtlas& atlas, TextureCache& cache)
    : _batch(&batch), _atlas(&atlas), _cache(&cache) {}

// Dessine toutes les entites affichables du monde, vues par la camera.
void SpriteRenderer::render(core::World& world, const Camera2D& camera, RenderMode mode,
                            float interpolationAlpha, const std::optional<std::string>& background,
                            int levelWidth, int levelHeight,
                            const std::vector<core::TileTextureOverride>& textureOverrides,
                            const std::unordered_map<std::string, core::Animation>& tileAnimations,
                            const std::vector<core::Plane>& planes, bool planeParallax,
                            const core::TileMap* doorCollision) {
    _scene.clear();
    _scene.setVisibleBounds(camera.visibleBounds());
    const SceneTextures textures =
        sceneTextures(*_atlas, *_cache, _skins, _skinSet, textureOverrides, tileAnimations);
    composeBackground(_scene, resolveBackgroundTexture(background, *_cache), levelWidth,
                      levelHeight, mode);
    // Plans picturaux AVANT tout le reste et dans l'ordre declare (LOT-69 TACHE-05) : c'est cet
    // ordre de composition qui porte leur ordre de dessin, le tri intercalant le rang de premiere
    // apparition de texture entre le calque et le sortOrder. Propriete figee par un test.
    PlaneParallax parallax;
    parallax.active = planeParallax;
    parallax.cameraBounds = camera.visibleBounds();
    parallax.pixelsPerWorldUnit = Camera2D::PIXELS_PER_UNIT * camera.zoom();
    composePlanes(_scene, planes, resolvePlaneTextures(*_cache, _planesDirectory, planes),
                  levelWidth, levelHeight, mode, PlaneVisibility{}, parallax);
    composeShadows(_scene, world, mode, textures, interpolationAlpha, doorCollision);
    composeWorldSprites(_scene, world, mode, textures, interpolationAlpha);
    // Particules du personnage (LOT-53 TACHE-03) : meme scene, apres les sprites -- l'ordre de
    // composition n'importe pas, ComposedScene::sort() reordonne par calque/texture/sortOrder.
    composeParticles(_scene, world, mode, textures);
    _scene.sort();
    logStatisticsIfChanged();
    submitComposedScene(*_batch, camera.projectionMatrix(), _scene);
}

// Journalise les compteurs de l'image seulement s'ils ont change depuis la precedente.
void SpriteRenderer::logStatisticsIfChanged() {
    const SceneStatistics statistics = _scene.statistics();
    const bool changed = statistics.considered != _loggedStatistics.considered ||
                         statistics.culled != _loggedStatistics.culled ||
                         statistics.submitted != _loggedStatistics.submitted ||
                         statistics.batches != _loggedStatistics.batches;
    if (!changed) {
        return;
    }
    _loggedStatistics = statistics;
    GRAPHICS_LOG_TRACE(formatSceneStatistics(statistics));
}

}  // namespace hmi
