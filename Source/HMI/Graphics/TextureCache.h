// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <string>

#include "HMI/Graphics/CacheRegistry.h"
#include "HMI/Graphics/RhiContext.h"
#include "HMI/Graphics/TextureLoader.h"

/**
 * @file HMI/Graphics/TextureCache.h
 * @brief Registre des textures **générées** à la demande : les marqueurs d'entité (`LOT-39`).
 */

namespace hmi {

/**
 * @brief Crée et conserve les textures des marqueurs d'entité, une par clé d'asset.
 *
 * Un marqueur tient lieu d'illustration tant qu'aucune n'existe : il ne lit rien sur le disque, et
 * la même clé donne toujours la même image. Chaque texture est créée **au premier accès** puis
 * conservée — la mémoïsation, échec compris, est celle de `hmi::CacheRegistry`, testable sans
 * GPU. Les ressources sont détenues en RAII et libérées à la destruction du cache
 * (`EX-NFR-041`).
 */
class TextureCache {
public:
    /**
     * @param context Interface de rendu et lot de mises à jour de l'image courante (référencé, non
     *                copié : le lot change à chaque image).
     */
    explicit TextureCache(const RhiContext& context);

    TextureCache(const TextureCache&) = delete;
    TextureCache& operator=(const TextureCache&) = delete;

    /**
     * @brief Texture du **marqueur généré** d'une clé d'asset (`core::assetMarker`, `LOT-39`),
     *        créée une seule fois à la demande.
     *
     * Peint à `ENTITY_MARKER_SIZE_PIXELS` de côté (`HMI/Graphics/EntityMarkers.h`) — une case —
     * puisque l'éditeur le pose sur la case d'une entité.
     * @param key Clé d'asset (`famille/identifiant`, cf. `hmi::entityMarkerKey`).
     * @return La texture (propriété du cache), ou `nullptr` si la clé est malformée ou que la
     *         création GPU a échoué — jamais d'exception (`EX-NFR-040`).
     */
    [[nodiscard]] const LoadedTexture* markerTexture(const std::string& key);

    /// Retire toutes les entrées : elles seront recréées au prochain accès.
    void invalidateAll();

private:
    const RhiContext& _context;  // possédé par le propriétaire des ressources de scène
    CacheRegistry<LoadedTexture> _markerEntries;
};

}  // namespace hmi
