// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>

#include "HMI/Graphics/RhiContext.h"
#include "HMI/Graphics/SkinCatalog.h"

class QRhi;
class QRhiResourceUpdateBatch;

/**
 * @file HMI/Graphics/SceneResources.h
 * @brief Les ressources QRhi que **toute** surface de rendu du projet possède (`LOT-86`).
 */

namespace hmi {

class SpriteBatch;
class TextureAtlas;
class TextureCache;
class BitmapFont;

/**
 * @brief Lot de sprites, atlas, police bitmap, registre de textures et catalogue de skins —
 *        créés ensemble, libérés ensemble, dans le bon ordre.
 *
 * **Pourquoi cette classe.** Depuis le `LOT-86`, deux surfaces dessinent la même scène : le
 * viewport à widget de l'éditeur (`hmi::GameViewport`, un `QRhiWidget`) et l'élément Qt Quick du
 * jeu (`hmi::GameViewportItem`, un `QQuickRhiItem`). Elles n'ont ni le même hôte, ni la même
 * boucle, ni les mêmes entrées — mais elles créent **exactement** les mêmes ressources graphiques.
 * Les écrire deux fois aurait suffi à les faire diverger : c'est ce qui arrive toujours, et ça ne
 * se voit qu'à l'exécution, sur une seule des deux.
 *
 * **L'ordre de libération est la raison d'être du regroupement.** Ce qui tient une texture doit
 * mourir avant la texture, et la texture avant le pipeline qui l'échantillonne. Libérer dans le
 * désordre ne produit pas une erreur nette mais un plantage à la fermeture, intermittent selon le
 * pilote. `release()` fixe cet ordre une fois pour toutes ; aucun appelant n'a plus à s'en
 * souvenir.
 *
 * **Ce qui n'est pas ici** : le brouillon d'édition, le `DraftRenderer`, la caméra, les plans, la
 * session de jeu. Ils appartiennent à un seul des deux hôtes — les remonter ici rendrait la classe
 * dépendante de l'éditeur, et le jeu paierait pour ce dont il ne se sert pas.
 */
class SceneResources {
public:
    SceneResources();
    ~SceneResources();

    SceneResources(const SceneResources&) = delete;
    SceneResources& operator=(const SceneResources&) = delete;

    /**
     * @brief Crée les ressources sur @p rhi, en déposant les téléversements dans @p updates.
     *
     * @p updates est le lot de la première image : les textures se chargent paresseusement et
     * leurs pixels y transitent. L'appelant le soumet ensuite, **hors** de toute passe de rendu —
     * c'est la contrainte de QRhi que `hmi::RhiContext` documente.
     *
     * Le catalogue de skins est lu à côté de l'exécutable. Fichier absent ou illisible : catalogue
     * vide, tout retombe sur le damier de repli — un état de départ légitime, pas une erreur
     * bloquante (`EX-NFR-040`).
     */
    void create(QRhi* rhi, QRhiResourceUpdateBatch* updates);

    /// Libère tout, dans l'ordre imposé par les dépendances entre ressources.
    void release() noexcept;

    /// @return `true` si `create` a réussi et que rien n'a été libéré depuis.
    [[nodiscard]] bool created() const noexcept {
        return _spriteBatch != nullptr;
    }

    /// Déclare le lot de téléversements de l'image en cours (`nullptr` pour le retirer en fin
    /// d'image). Les propriétaires de textures y déposent leurs pixels pendant la composition.
    void setFrameUpdates(QRhiResourceUpdateBatch* updates) noexcept {
        _context.updates = updates;
    }

    [[nodiscard]] RhiContext& context() noexcept {
        return _context;
    }
    [[nodiscard]] SpriteBatch& sprites() noexcept {
        return *_spriteBatch;
    }
    [[nodiscard]] TextureAtlas& atlas() noexcept {
        return *_atlas;
    }
    [[nodiscard]] BitmapFont& font() noexcept {
        return *_font;
    }
    [[nodiscard]] TextureCache& textures() noexcept {
        return *_textureCache;
    }
    [[nodiscard]] SkinCatalog& skins() noexcept {
        return _skins;
    }

private:
    RhiContext _context;
    std::unique_ptr<SpriteBatch> _spriteBatch;
    std::unique_ptr<TextureAtlas> _atlas;
    std::unique_ptr<BitmapFont> _font;
    std::unique_ptr<TextureCache> _textureCache;
    SkinCatalog _skins;
};

}  // namespace hmi
