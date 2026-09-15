// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QColor>
#include <QPointer>
#include <QQuickRhiItem>
#include <QtQmlIntegration>

#include "HMI/Runtime/ArenaModel.h"

/**
 * @file HMI/Runtime/ArenaViewportItem.h
 * @brief La surface de rendu du **Colisée**, en Qt Quick (`LOT-86` Phase 5).
 */

namespace hmi {

/**
 * @brief Surface QRhi de la scène de combat du Colisée : la grille, l'enceinte et les figurines,
 *        dessinées par le pipeline 2D du jeu au lieu de quelques centaines de délégués QML.
 *
 * ## Pourquoi un item à part
 *
 * `hmi::GameViewportItem` dessinera le monde exploré ; celui-ci dessine une `core::ArenaSession`
 * en projection isométrique. Sessions et projections n'ont rien en commun : les faire cohabiter
 * dans une classe obligerait chacune à connaître l'autre.
 *
 * ## Ce qui traverse la frontière entre les fils
 *
 * Le modèle (`hmi::ArenaModel`) et sa session vivent sur le fil graphique ; le dessin, sur le fil
 * de rendu. Dans `synchronize()` — fil graphique bloqué — le peintre ne copie que des **valeurs** :
 * la couleur d'effacement, et, si la scène a changé depuis sa dernière copie, l'instantané
 * `hmi::ArenaSceneSnapshot`. Il ne garde aucun pointeur vers le modèle ni vers la session.
 *
 * « A changé » se compte ici : chaque `ArenaModel::changed` avance `sceneRevision`, et le peintre
 * ne reprend un instantané que si le numéro diffère du sien. Un pas de curseur (`cursorChanged`)
 * n'en provoque aucun — le curseur et le chemin restent dessinés en QML par-dessus (`LOT-24`).
 *
 * ## Ce que fait le peintre
 *
 * Il relaie `hmi::ArenaSceneRenderer`, qui porte le vrai travail (ressources, textures, animation,
 * passe) et se teste hors écran : `initialize()` → `ensureResources`, `synchronize()` →
 * `setSnapshot`, `render()` → `render`.
 */
class ArenaViewportItem : public QQuickRhiItem {
    Q_OBJECT
    QML_NAMED_ELEMENT(ArenaViewport)

    /// La vue-modèle du Colisée dont la session est dessinée. Nulle : seul le fond est dessiné.
    Q_PROPERTY(hmi::ArenaModel* model READ model WRITE setModel NOTIFY modelChanged)
    /// Couleur d'effacement, qui vient de `Tokens.qml` comme le reste de l'identité du jeu.
    Q_PROPERTY(QColor clearColor READ clearColor WRITE setClearColor NOTIFY clearColorChanged)

public:
    explicit ArenaViewportItem(QQuickItem* parent = nullptr);

    [[nodiscard]] ArenaModel* model() const noexcept {
        return _model.data();
    }
    void setModel(ArenaModel* model);

    [[nodiscard]] QColor clearColor() const noexcept {
        return _clearColor;
    }
    void setClearColor(const QColor& color);

    /// @return Le numéro de la scène : il avance à chaque changement du combat ou du modèle.
    [[nodiscard]] quint64 sceneRevision() const noexcept {
        return _sceneRevision;
    }

    [[nodiscard]] QQuickRhiItemRenderer* createRenderer() override;

signals:
    void modelChanged();
    void clearColorChanged();

private:
    /// La scène a changé : un nouvel instantané sera pris à la prochaine synchronisation.
    void invalidateScene();

    QPointer<ArenaModel> _model;
    QMetaObject::Connection _modelChangedConnection;
    QMetaObject::Connection _modelDestroyedConnection;
    /// Commence à 1 : un peintre neuf (à 0) prend toujours un premier instantané.
    quint64 _sceneRevision = 1;
    QColor _clearColor{0x1a, 0x14, 0x10};  ///< Pierre sombre, jusqu'à ce que le QML en décide.
};

}  // namespace hmi
