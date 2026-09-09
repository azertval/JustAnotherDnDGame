// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Graphics/SceneResources.h"

#include <filesystem>
#include <utility>

#include "HMI/Graphics/AssetPaths.h"
#include "HMI/Graphics/BitmapFont.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TextureCache.h"
#include "HMI/Platform/ExecutableDirectory.h"

namespace hmi {

// Définis ici, et non `= default` dans l'en-tête : les membres sont des `unique_ptr` de types
// seulement DÉCLARÉS là-bas. Le destructeur implicite exigerait leur définition complète, et tout
// fichier incluant l'en-tête devrait alors tirer quatre en-têtes de rendu dont il n'a que faire.
SceneResources::SceneResources() = default;

SceneResources::~SceneResources() {
    release();
}

void SceneResources::create(QRhi* rhi, QRhiResourceUpdateBatch* updates) {
    _context.rhi = rhi;
    _context.updates = updates;

    _spriteBatch = std::make_unique<SpriteBatch>(_context.rhi);
    _atlas = std::make_unique<TextureAtlas>(_context);
    // Police bitmap du HUD (LOT-52), chargée une fois comme l'atlas (repli procédural intégré,
    // pas de damier de secours à gérer ici).
    _font = std::make_unique<BitmapFont>(_context);
    // Registre des textures nommées (LOT-40) : propriétaire du damier de repli du mode Texture,
    // et point d'entrée des skins depuis le LOT-42.
    _textureCache =
        std::make_unique<TextureCache>(_context, AssetPaths{executableDirectory() / "Assets"});

    // Catalogue des skins (LOT-42), lu à côté de l'exécutable comme les niveaux et les
    // traductions. Fichier absent ou illisible : catalogue vide, tout retombe sur le damier — un
    // état de départ légitime, pas une erreur bloquante (EX-NFR-040).
    SkinCatalogResult skins =
        SkinCatalog::loadFromFile(executableDirectory() / "Assets" / "skins.json");
    if (skins.ok()) {
        _skins = std::move(*skins.catalog);
    }
}

void SceneResources::release() noexcept {
    // L'ORDRE EST LA RAISON D'ÊTRE DE CETTE CLASSE. Ce qui tient une texture meurt avant elle, et
    // la texture avant le pipeline qui l'échantillonne. Libérer dans le désordre ne produit pas
    // une erreur nette mais un plantage à la fermeture, intermittent selon le pilote.
    _textureCache.reset();
    _font.reset();
    _atlas.reset();
    _spriteBatch.reset();
    _context.rhi = nullptr;
    _context.updates = nullptr;
}

}  // namespace hmi
