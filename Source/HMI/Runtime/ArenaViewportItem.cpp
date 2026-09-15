// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Runtime/ArenaViewportItem.h"

#include <algorithm>
#include <chrono>

#include <rhi/qrhi.h>

#include "HMI/Graphics/ArenaSceneComposer.h"
#include "HMI/Graphics/ArenaSceneRenderer.h"
#include "HMI/HmiLog.h"
#include "HMI/Platform/ExecutableDirectory.h"

namespace hmi {
namespace {

/// Au-delà, une image en retard (fenêtre déplacée, point d'arrêt) ne fait pas sauter l'animation.
constexpr float MAXIMUM_FRAME_SECONDS = 0.25f;

/**
 * @brief Le peintre, côté **fil de rendu**.
 *
 * Il ne partage aucun état avec l'élément : tout ce dont il a besoin lui est remis par copie dans
 * `synchronize()`. Qt Quick le détruit sur le fil de rendu quand le graphe de scène de l'élément
 * est libéré (élément retiré, fenêtre changée, graphe invalidé), l'interface QRhi encore vivante :
 * le destructeur de `ArenaSceneRenderer` libère alors tout, dans l'ordre.
 */
class ArenaViewportRenderer : public QQuickRhiItemRenderer {
public:
    ArenaViewportRenderer() : _arena(executableDirectory() / "Assets" / "Coliseum") {}

    void initialize(QRhiCommandBuffer* commandBuffer) override;
    void synchronize(QQuickRhiItem* item) override;
    void render(QRhiCommandBuffer* commandBuffer) override;

private:
    using Clock = std::chrono::steady_clock;

    ArenaSceneRenderer _arena;
    /// Numéro de la scène copiée : 0 tant qu'aucun instantané n'a été pris.
    quint64 _sceneRevision = 0;
    QColor _clearColor;
    Clock::time_point _previousFrame = Clock::now();
};

void ArenaViewportRenderer::initialize(QRhiCommandBuffer* /*commandBuffer*/) {
    // Appelée à chaque synchronisation où la texture d'appui a pu changer — souvent pour une simple
    // taille. `ensureResources` ne recrée que si l'interface QRhi n'est plus la même.
    const QRhi* const previous = _arena.rhi();
    if (_arena.ensureResources(rhi()) && previous != rhi()) {
        HMI_LOG_INFO(std::string("Viewport de l'arene : interface de rendu QRhi initialisee (") +
                     rhi()->backendName() + ").");
    }
}

void ArenaViewportRenderer::synchronize(QQuickRhiItem* item) {
    // Le SEUL instant où les deux fils se parlent : le fil graphique est bloqué. Rien de ce qui est
    // lu ici n'est gardé par référence.
    auto* const viewport = static_cast<ArenaViewportItem*>(item);
    _clearColor = viewport->clearColor();

    if (viewport->sceneRevision() == _sceneRevision) {
        return;
    }
    _sceneRevision = viewport->sceneRevision();
    const ArenaModel* const model = viewport->model();
    const core::ArenaSession* const session = model != nullptr ? model->session() : nullptr;
    _arena.setSnapshot(session != nullptr ? snapshotArenaScene(*session) : ArenaSceneSnapshot{});
}

void ArenaViewportRenderer::render(QRhiCommandBuffer* commandBuffer) {
    const Clock::time_point now = Clock::now();
    const float elapsed =
        std::min(std::chrono::duration<float>(now - _previousFrame).count(), MAXIMUM_FRAME_SECONDS);
    _previousFrame = now;

    const float clear[4] = {_clearColor.redF(), _clearColor.greenF(), _clearColor.blueF(), 1.0f};
    _arena.render(commandBuffer, renderTarget(), elapsed, clear);

    // Des figurines à l'écran : elles respirent, l'image suivante est demandée. Sans elles, la
    // surface ne se redessine qu'à un changement de scène, de taille ou de couleur.
    if (_arena.animating()) {
        update();
    }
}

}  // namespace

ArenaViewportItem::ArenaViewportItem(QQuickItem* parent) : QQuickRhiItem(parent) {}

void ArenaViewportItem::setModel(ArenaModel* model) {
    if (_model == model) {
        return;
    }
    disconnect(_modelChangedConnection);
    disconnect(_modelDestroyedConnection);
    _model = model;
    if (model != nullptr) {
        // `combatSceneChanged`, pas `changed` : un geste de composition (enrôler, retirer, marquer,
        // graine, IA) ne mute encore aucune grille, et ne doit pas faire reprendre un instantané.
        _modelChangedConnection =
            connect(model, &ArenaModel::combatSceneChanged, this, &ArenaViewportItem::invalidateScene);
        // Le QPointer se vide seul ; il reste à redessiner une scène vide.
        _modelDestroyedConnection =
            connect(model, &QObject::destroyed, this, &ArenaViewportItem::invalidateScene);
    }
    invalidateScene();
    emit modelChanged();
}

void ArenaViewportItem::setClearColor(const QColor& color) {
    if (_clearColor == color) {
        return;
    }
    _clearColor = color;
    emit clearColorChanged();
    update();  // sans quoi la couleur ne parvient au fil de rendu qu'à la prochaine image demandée.
}

void ArenaViewportItem::invalidateScene() {
    ++_sceneRevision;
    update();
}

QQuickRhiItemRenderer* ArenaViewportItem::createRenderer() {
    return new ArenaViewportRenderer;
}

}  // namespace hmi
