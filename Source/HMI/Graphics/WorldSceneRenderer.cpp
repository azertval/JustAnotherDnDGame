// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/WorldSceneRenderer.h"

#include <algorithm>
#include <optional>
#include <utility>

#include <rhi/qrhi.h>

#include "HMI/Graphics/AnimationCatalog.h"
#include "HMI/Graphics/GraphicsLog.h"
#include "HMI/Graphics/MissingTexture.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/Graphics/SpriteRenderer.h"

namespace hmi {

namespace {

/// Le chemin du `.anim.json` d'une bande : `idle.png` -> `idle.anim.json`.
[[nodiscard]] std::filesystem::path animationDescriptionOf(const std::filesystem::path& band) {
    std::filesystem::path description = band;
    description.replace_extension();
    description += ".anim.json";
    return description;
}

}  // namespace

Camera2D worldCamera(const core::IsoProjection& projection, core::Vector2 focus, int pixelWidth,
                     int pixelHeight, float tilesAcross) {
    const int width = std::max(1, pixelWidth);
    const int height = std::max(1, pixelHeight);
    Camera2D camera(width, height);

    // L'echelle du jeu : un nombre de losanges en largeur, et non le cadrage entier de l'arene --
    // le Colisee ne tient pas dans un ecran (EX-LVL-006).
    const float visibleWidth = std::max(1.0F, tilesAcross) * projection.tileWidth();
    camera.setZoom(std::max(0.25F, static_cast<float>(width) / visibleWidth /
                                       Camera2D::PIXELS_PER_UNIT));

    // Le point suivi, ramene dans la scene : la vue ne montre pas le vide autour de la carte. Sur
    // un axe ou la scene est plus petite que la vue, elle reste centree.
    const core::Vector2 scene = projection.sceneSize();
    const core::Rect visible = camera.visibleBounds();
    const core::Vector2 origin = projection.origin();
    const float demiLargeur = visible.size.x / 2.0F;
    const float demiHauteur = visible.size.y / 2.0F;
    core::Vector2 centre = focus;
    if (scene.x <= visible.size.x) {
        centre.x = origin.x + (scene.x / 2.0F);
    } else {
        centre.x = std::clamp(centre.x, origin.x + demiLargeur, origin.x + scene.x - demiLargeur);
    }
    if (scene.y <= visible.size.y) {
        centre.y = origin.y + (scene.y / 2.0F);
    } else {
        centre.y = std::clamp(centre.y, origin.y + demiHauteur, origin.y + scene.y - demiHauteur);
    }
    camera.setCenter(centre);
    return camera;
}

WorldSceneRenderer::WorldSceneRenderer(std::filesystem::path assetsDirectory)
    : _directory(std::move(assetsDirectory)) {}

WorldSceneRenderer::~WorldSceneRenderer() {
    release();
}

bool WorldSceneRenderer::ensureResources(QRhi* rhi) {
    if (rhi == nullptr) {
        return false;
    }
    if (created() && _rhi == rhi) {
        return true;
    }
    // Une autre interface : tout ce qui a ete cree appartient a l'ancienne et ne doit plus servir.
    release();

    _rhi = rhi;
    _pendingUploads = rhi->nextResourceUpdateBatch();
    _resources.create(rhi, _pendingUploads);

    const ProceduralAtlasImage checker = buildMissingTextureImage();
    if (std::optional<LoadedTexture> missing =
            createTexture(_resources.context(), checker.width, checker.height, checker.pixels)) {
        _missing = std::move(*missing);
        _textures.missing = SceneTexture{
            .texture = _missing.handle(), .width = _missing.width, .height = _missing.height};
    }
    _resources.setFrameUpdates(nullptr);
    GRAPHICS_LOG_INFO("Lieu : ressources QRhi creees (" + std::string(rhi->backendName()) + ").");
    return true;
}

int WorldSceneRenderer::bandFrameWidth(const std::string& path) {
    const auto connue = _bandFrameWidths.find(path);
    if (connue != _bandFrameWidths.end()) {
        return connue->second;
    }
    // Seules les figurines ont un `.anim.json` ; une piece de decor n'en a pas, et son absence est
    // un cas legitime, pas une anomalie.
    const AnimationDescriptionResult lue =
        AnimationCatalog::loadFromFile(animationDescriptionOf(_directory / path));
    const int largeur = lue.ok() ? lue.description->frameWidth : 0;
    _bandFrameWidths.emplace(path, largeur);
    return largeur;
}

void WorldSceneRenderer::ensureTextures(const std::vector<std::string>& paths) {
    for (const std::string& path : paths) {
        // Deja tente : une piece absente ne doit pas etre redemandee a chaque image.
        if (!_requested.insert(path).second) {
            continue;
        }
        std::optional<LoadedTexture> texture =
            loadTextureFromFile(_resources.context(), _directory / path);
        if (!texture.has_value()) {
            // La composition retombe sur le damier : une piece manquante se voit, sans planter.
            GRAPHICS_LOG_WARNING(missingTextureWarning(path));
            continue;
        }
        _textures.byPath[path] = SceneTexture{.texture = texture->handle(),
                                              .width = texture->width,
                                              .height = texture->height,
                                              .frameWidth = bandFrameWidth(path)};
        _loaded.push_back(std::move(*texture));
    }
}

void WorldSceneRenderer::release() noexcept {
    // L'ordre : ce qui designe une texture, puis les textures, puis la grappe qui porte le
    // pipeline. Le lot de creation jamais soumis est rendu a QRhi.
    _composed.clear();
    _textures.byPath.clear();
    _textures.missing = SceneTexture{};
    _loaded.clear();
    _missing = LoadedTexture{};
    _requested.clear();
    if (_pendingUploads != nullptr) {
        _pendingUploads->release();
        _pendingUploads = nullptr;
    }
    _resources.release();
    _rhi = nullptr;
}

void WorldSceneRenderer::setSnapshot(WorldSceneSnapshot snapshot) {
    _snapshot = std::move(snapshot);
}

void WorldSceneRenderer::render(QRhiCommandBuffer* commandBuffer, QRhiRenderTarget* target,
                                const float* clear) {
    if (!created() || commandBuffer == nullptr || target == nullptr) {
        return;
    }
    // Le lot de cette image : celui de la creation s'il attend encore, sinon un neuf.
    QRhiResourceUpdateBatch* const updates = _pendingUploads != nullptr
                                                 ? std::exchange(_pendingUploads, nullptr)
                                                 : _rhi->nextResourceUpdateBatch();
    _resources.setFrameUpdates(updates);

    // Les textures du lieu ne sont pas connues d'avance : la carte change au passage d'un portail,
    // et ce sont ses pieces qui disent quoi charger.
    ensureTextures(worldTexturePaths(_snapshot));

    const core::IsoProjection projection(_snapshot.columns, _snapshot.rows);
    _composed.clear();
    composeWorldScene(_composed, _snapshot, projection, _textures);
    _composed.sort();

    const QSize pixels = target->pixelSize();
    const Camera2D camera = worldCamera(projection, projection.gridToWorld(_focus), pixels.width(),
                                        pixels.height(), WORLD_TILES_ACROSS);

    SpriteBatch& sprites = _resources.sprites();
    sprites.beginFrame();
    submitComposedScene(sprites, camera.projectionMatrix(), _composed);
    sprites.submit(commandBuffer, target, updates, clear);
    _resources.setFrameUpdates(nullptr);
}

}  // namespace hmi
