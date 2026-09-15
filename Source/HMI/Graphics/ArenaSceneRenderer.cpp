// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/ArenaSceneRenderer.h"

#include <algorithm>
#include <optional>
#include <utility>

#include <rhi/qrhi.h>

#include "Core/Combat/IsoProjection.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/GraphicsLog.h"
#include "HMI/Graphics/MissingTexture.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/Graphics/SpriteRenderer.h"

namespace hmi {

namespace {

/// Marge du cadrage : la scène entière tient dans la surface, sans toucher ses bords.
constexpr float FRAMING_MARGIN = 0.95f;

}  // namespace

Camera2D arenaCamera(const core::IsoProjection& projection, int pixelWidth, int pixelHeight) {
    const int width = std::max(1, pixelWidth);
    const int height = std::max(1, pixelHeight);
    const core::Vector2 size = projection.sceneSize();
    Camera2D camera(width, height);
    camera.setCenter({size.x / 2.0f, size.y / 2.0f});
    camera.setZoom(Camera2D::fitZoom(static_cast<float>(width), static_cast<float>(height),
                                     std::max(size.x, 1.0f), std::max(size.y, 1.0f),
                                     FRAMING_MARGIN));
    return camera;
}

ArenaSceneRenderer::ArenaSceneRenderer(std::filesystem::path coliseumDirectory)
    : _directory(std::move(coliseumDirectory)) {
    // Lectures de fichiers, une fois : ni le catalogue ni les clips ne touchent au GPU, et une
    // recréation des ressources n'a pas à les relire.
    ArenaAppearanceCatalogResult catalog =
        ArenaAppearanceCatalog::loadFromFile(_directory / "manifest.json");
    if (catalog.ok()) {
        _catalog = std::move(*catalog.catalog);
    } else {
        GRAPHICS_LOG_WARNING("Arene : manifeste du Colisee illisible, " + catalog.error);
    }

    const auto declare = [this](const std::string& folder, const std::string& sheet) {
        ArenaFigureAnimationLoad load = loadArenaFigureAnimations(_directory / folder / sheet);
        for (const std::string& error : load.errors) {
            GRAPHICS_LOG_WARNING("Arene : animation de " + sheet + ", " + error);
        }
        _animation.setFigureAnimations(sheet, std::move(load.clips));
    };
    for (const std::string& hero : _catalog.heroes()) {
        declare("characters", hero);
    }
    for (const std::string& gladiator : _catalog.gladiators()) {
        declare("enemies", gladiator);
    }
}

ArenaSceneRenderer::~ArenaSceneRenderer() {
    release();
}

bool ArenaSceneRenderer::ensureResources(QRhi* rhi) {
    if (rhi == nullptr) {
        return false;
    }
    if (created() && _rhi == rhi) {
        return true;
    }
    // Une autre interface : tout ce qui a été créé appartient à l'ancienne et ne doit plus servir.
    release();

    _rhi = rhi;
    _pendingUploads = rhi->nextResourceUpdateBatch();
    _resources.create(rhi, _pendingUploads);
    loadTextures();
    _resources.setFrameUpdates(nullptr);
    GRAPHICS_LOG_INFO("Arene : ressources QRhi creees (" + std::string(rhi->backendName()) + ", " +
                      std::to_string(_textures.byPath.size()) + " textures).");
    return true;
}

void ArenaSceneRenderer::loadTextures() {
    const RhiContext& context = _resources.context();

    const ProceduralAtlasImage checker = buildMissingTextureImage();
    if (std::optional<LoadedTexture> missing =
            createTexture(context, checker.width, checker.height, checker.pixels)) {
        _missing = std::move(*missing);
        _textures.missing = ArenaTexture{_missing.handle(), _missing.width, _missing.height};
    }

    const std::vector<std::string> paths = arenaTexturePaths(_catalog);
    _loaded.reserve(paths.size());
    for (const std::string& path : paths) {
        std::optional<LoadedTexture> texture = loadTextureFromFile(context, _directory / path);
        if (!texture.has_value()) {
            // La composition retombe sur le damier : une pièce manquante se voit, sans planter.
            GRAPHICS_LOG_WARNING(missingTextureWarning(path));
            continue;
        }
        _textures.byPath[path] = ArenaTexture{texture->handle(), texture->width, texture->height};
        _loaded.push_back(std::move(*texture));
    }
}

void ArenaSceneRenderer::release() noexcept {
    // L'ordre : ce qui désigne une texture, puis les textures, puis la grappe qui porte le
    // pipeline. Le lot de création jamais soumis est rendu à QRhi, qui en garde un nombre borné.
    _composed.clear();
    _textures.byPath.clear();
    _textures.missing = ArenaTexture{};
    _loaded.clear();
    _missing = LoadedTexture{};
    if (_pendingUploads != nullptr) {
        _pendingUploads->release();
        _pendingUploads = nullptr;
    }
    _resources.release();
    _rhi = nullptr;
}

void ArenaSceneRenderer::setSnapshot(ArenaSceneSnapshot snapshot) {
    _snapshot = std::move(snapshot);

    std::set<core::CombatantId> present;
    for (const ArenaFigureSnapshot& figure : _snapshot.figures) {
        present.insert(figure.id);
        if (!_animated.contains(figure.id)) {
            // Seulement à l'arrivée : rejouer le repos à chaque instantané couperait une action en
            // cours. Déclencher attaque et coup d'après les événements est l'affaire de l'appelant.
            _animation.play(figure.id, _catalog.figureFor(figure.name, figure.side).sheet,
                            ArenaFigureAction::Idle);
        }
    }
    for (const core::CombatantId id : _animated) {
        if (!present.contains(id)) {
            _animation.remove(id);
        }
    }
    _animated = std::move(present);
}

void ArenaSceneRenderer::render(QRhiCommandBuffer* commandBuffer, QRhiRenderTarget* target,
                                float realDeltaSeconds, const float clear[4]) {
    if (!created() || commandBuffer == nullptr || target == nullptr) {
        return;
    }
    // Le lot de cette image : celui de la création s'il attend encore, sinon un neuf. `submit` le
    // soumet avec l'ouverture de sa passe, qui en prend la propriété.
    QRhiResourceUpdateBatch* const updates = _pendingUploads != nullptr
                                                 ? std::exchange(_pendingUploads, nullptr)
                                                 : _rhi->nextResourceUpdateBatch();
    _resources.setFrameUpdates(updates);

    _animation.advance(std::max(0.0f, realDeltaSeconds));
    const ArenaAnimationState animation = _animation.snapshot();
    const core::IsoProjection projection(_snapshot.columns, _snapshot.rows);

    _composed.clear();
    composeArenaScene(_composed, _snapshot, _catalog, animation, projection, _textures);
    _composed.sort();

    // Cadrage : la scène entière, centrée. La projection isométrique a déjà placé les pièces en
    // unités monde ; la caméra ne fait que déplacer et agrandir, en aval.
    const QSize pixels = target->pixelSize();
    const Camera2D camera = arenaCamera(projection, pixels.width(), pixels.height());

    SpriteBatch& sprites = _resources.sprites();
    sprites.beginFrame();
    submitComposedScene(sprites, camera.projectionMatrix(), _composed);
    sprites.submit(commandBuffer, target, updates, clear);
    _resources.setFrameUpdates(nullptr);
}

}  // namespace hmi
