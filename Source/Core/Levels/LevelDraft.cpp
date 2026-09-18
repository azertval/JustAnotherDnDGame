// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Core/Levels/LevelDraft.h"

#include <algorithm>
#include <utility>

#include "Core/Levels/LevelWriter.h"

namespace core {

namespace {

// Copie @p source dans une grille de @p width x @p height, tronquee aux bords. Extrait de
// resize() : les couches du LOT-04 subissent exactement le meme sort que la grille racine, une
// couche restee aux anciennes dimensions rendrait le niveau irrecuperable a l'enregistrement (le
// chargeur refuse une tuile hors bornes).
[[nodiscard]] TileMap resizedCopy(const TileMap& source, int width, int height) {
    TileMap resized(width, height);
    const int copyWidth = (std::min)(width, source.width());
    const int copyHeight = (std::min)(height, source.height());
    for (int row = 0; row < copyHeight; ++row) {
        for (int column = 0; column < copyWidth; ++column) {
            resized.setTile(column, row, source.tile(column, row));
        }
    }
    return resized;
}

}  // namespace

LevelDraft::LevelDraft(std::string name, TileMap tileMap)
    : _name(std::move(name)), _tileMap(std::move(tileMap)) {}

LevelDraft LevelDraft::empty(std::string name, int width, int height) {
    return {std::move(name), TileMap(width, height)};
}

LevelDraft LevelDraft::fromLevel(const Level& level) {
    LevelDraft draft(level.name(), level.tileMap());
    // INVARIANT DE CE CONSTRUCTEUR : fromLevel recopie TOUS les champs de Level, sans exception.
    // Un brouillon reenregistre doit etre equivalent a la carte d'origine -- un champ non recopie
    // ne fait echouer aucun appel, il s'efface simplement au premier enregistrement. Ajouter un
    // champ a Level, c'est l'ajouter ici.
    draft._entry = level.entry();
    draft._textureOverrides = level.textureOverrides();
    draft._layers = level.layers();
    draft._entities = level.entities();
    return draft;
}

void LevelDraft::paintTile(int column, int row, TileType type) {
    pushUndo();
    paintTileInternal(column, row, type);
}

void LevelDraft::paintRegion(int originColumn, int originRow,
                             const std::vector<std::vector<TileType>>& block) {
    if (block.empty()) {
        return;
    }
    pushUndo();
    for (std::size_t rowOffset = 0; rowOffset < block.size(); ++rowOffset) {
        const std::vector<TileType>& rowTiles = block[rowOffset];
        for (std::size_t columnOffset = 0; columnOffset < rowTiles.size(); ++columnOffset) {
            const int column = originColumn + static_cast<int>(columnOffset);
            const int row = originRow + static_cast<int>(rowOffset);
            if (!_tileMap.inBounds(column, row)) {
                continue;  // decoupe silencieuse aux bords, meme principe que resize()
            }
            paintTileInternal(column, row, rowTiles[columnOffset]);
        }
    }
}

void LevelDraft::paintTileInternal(int column, int row, TileType type) {
    if (type == TileType::Entry) {
        setEntryInternal(column, row);
        return;
    }

    const GridPosition position{.column = column, .row = row};
    if (_entry && *_entry == position) {
        _entry.reset();
    }
    // Reposer le meme type ne doit pas effacer une piece assignee (EX-EDIT-043) : un coup de
    // pinceau involontaire sur une case deja du bon type l'effacerait sinon.
    if (_tileMap.tile(column, row) != type) {
        removeTextureOverrideAt(position);
    }
    _tileMap.setTile(column, row, type);
}

void LevelDraft::setEntry(int column, int row) {
    pushUndo();
    setEntryInternal(column, row);
}

void LevelDraft::setEntryInternal(int column, int row) {
    const GridPosition position{.column = column, .row = row};
    if (_entry && *_entry != position) {
        _tileMap.setTile(_entry->column, _entry->row, TileType::Empty);
    }
    removeTextureOverrideAt(position);
    _tileMap.setTile(column, row, TileType::Entry);
    _entry = position;
}

// --- Couches de tuiles (LOT-11) ---
//
// Les validations d'abord, le pushUndo() ensuite : jamais un pas d'historique pour un geste refuse
// ou sans effet.

namespace {

// Vrai si @p layers porte au moins une couche visuelle.
[[nodiscard]] bool hasVisualLayer(const std::vector<TileLayer>& layers) {
    return std::ranges::any_of(
        layers, [](const TileLayer& layer) { return isVisualLayerKind(layer.kind); });
}

// Aligne l'entree de la grille racine sur ce que ferait le chargeur a la relecture : `Collision` en
// tete des qu'une couche visuelle existe, `Legacy` sinon. Sans cet alignement, le brouillon
// divergerait du niveau qu'il produit -- et l'editeur afficherait l'une quand le jeu lirait
// l'autre.
void alignRootLayer(std::vector<TileLayer>& layers, const TileMap& root) {
    const auto rootEntry = std::ranges::find_if(layers, [](const TileLayer& layer) {
        return layer.kind == LayerKind::Collision || layer.kind == LayerKind::Legacy;
    });
    if (!hasVisualLayer(layers)) {
        if (rootEntry != layers.end()) {
            rootEntry->kind = LayerKind::Legacy;
        }
        return;
    }
    if (rootEntry != layers.end()) {
        rootEntry->kind = LayerKind::Collision;
        return;
    }
    layers.insert(
        layers.begin(),
        TileLayer{.name = {}, .kind = LayerKind::Collision, .tiles = root, .properties = {}});
}

// Vrai si chaque type de @p block se peint sur une couche visuelle.
[[nodiscard]] bool blockIsVisual(const std::vector<std::vector<TileType>>& block) {
    return std::ranges::all_of(block, [](const std::vector<TileType>& row) {
        return std::ranges::all_of(row, isVisualLayerTileType);
    });
}

}  // namespace

bool LevelDraft::isVisualLayerIndex(std::size_t index) const noexcept {
    return index < _layers.size() && isVisualLayerKind(_layers[index].kind);
}

std::optional<std::size_t> LevelDraft::addLayer(LayerKind kind, std::string name) {
    if (!isVisualLayerKind(kind)) {
        return std::nullopt;
    }
    pushUndo();
    TileMap tiles(_tileMap.width(), _tileMap.height());
    if (!hasVisualLayer(_layers)) {
        // Promotion (voir l'en-tete) : la premiere couche visuelle reprend l'image de la grille
        // unique, sans les types qui n'ont de sens qu'en collision.
        for (int row = 0; row < _tileMap.height(); ++row) {
            for (int column = 0; column < _tileMap.width(); ++column) {
                const TileType type = _tileMap.tile(column, row);
                if (isVisualLayerTileType(type)) {
                    tiles.setTile(column, row, type);
                }
            }
        }
    }
    _layers.push_back(TileLayer{
        .name = std::move(name), .kind = kind, .tiles = std::move(tiles), .properties = {}});
    alignRootLayer(_layers, _tileMap);
    // L'alignement peut avoir insere l'entree de collision en tete : la couche creee est toujours
    // la derniere.
    return _layers.size() - 1;
}

bool LevelDraft::removeLayer(std::size_t index) {
    if (!isVisualLayerIndex(index)) {
        return false;
    }
    pushUndo();
    _layers.erase(_layers.begin() + static_cast<std::ptrdiff_t>(index));
    alignRootLayer(_layers, _tileMap);
    return true;
}

bool LevelDraft::renameLayer(std::size_t index, std::string name) {
    if (!isVisualLayerIndex(index) || _layers[index].name == name) {
        return false;
    }
    pushUndo();
    _layers[index].name = std::move(name);
    return true;
}

bool LevelDraft::setLayerKind(std::size_t index, LayerKind kind) {
    if (!isVisualLayerIndex(index) || !isVisualLayerKind(kind) || _layers[index].kind == kind) {
        return false;
    }
    pushUndo();
    _layers[index].kind = kind;
    return true;
}

std::optional<std::size_t> LevelDraft::moveLayer(std::size_t index, bool forward) {
    if (!isVisualLayerIndex(index)) {
        return std::nullopt;
    }
    const bool atEnd = forward ? index + 1 >= _layers.size() : index == 0;
    if (atEnd) {
        return index;
    }
    const std::size_t neighbour = forward ? index + 1 : index - 1;
    if (!isVisualLayerIndex(neighbour)) {
        return index;  // l'entree de collision ne se franchit pas.
    }
    pushUndo();
    std::swap(_layers[index], _layers[neighbour]);
    return neighbour;
}

bool LevelDraft::paintLayerTile(std::size_t index, int column, int row, TileType type) {
    if (!isVisualLayerIndex(index) || !isVisualLayerTileType(type)) {
        return false;
    }
    TileMap& tiles = _layers[index].tiles;
    if (!tiles.inBounds(column, row) || tiles.tile(column, row) == type) {
        return false;
    }
    pushUndo();
    _layers[index].tiles.setTile(column, row, type);
    return true;
}

bool LevelDraft::paintLayerRegion(std::size_t index, int originColumn, int originRow,
                                  const std::vector<std::vector<TileType>>& block) {
    if (!isVisualLayerIndex(index) || block.empty() || !blockIsVisual(block)) {
        return false;
    }
    pushUndo();
    TileMap& tiles = _layers[index].tiles;
    for (std::size_t rowOffset = 0; rowOffset < block.size(); ++rowOffset) {
        const std::vector<TileType>& rowTiles = block[rowOffset];
        for (std::size_t columnOffset = 0; columnOffset < rowTiles.size(); ++columnOffset) {
            const int column = originColumn + static_cast<int>(columnOffset);
            const int row = originRow + static_cast<int>(rowOffset);
            if (tiles.inBounds(column, row)) {
                tiles.setTile(column, row, rowTiles[columnOffset]);
            }
        }
    }
    return true;
}

// --- Entites de carte (LOT-11) ---

std::optional<std::size_t> LevelDraft::placeEntity(MapEntity entity) {
    if (!_tileMap.inBounds(entity.position.column, entity.position.row)) {
        return std::nullopt;
    }
    pushUndo();
    _entities.push_back(std::move(entity));
    return _entities.size() - 1;
}

bool LevelDraft::moveEntity(std::size_t index, GridPosition position) {
    if (!isEntityIndex(index) || !_tileMap.inBounds(position.column, position.row) ||
        _entities[index].position == position) {
        return false;
    }
    pushUndo();
    _entities[index].position = position;
    return true;
}

bool LevelDraft::removeEntity(std::size_t index) {
    if (!isEntityIndex(index)) {
        return false;
    }
    pushUndo();
    _entities.erase(_entities.begin() + static_cast<std::ptrdiff_t>(index));
    return true;
}

bool LevelDraft::setEntityProperty(std::size_t index, const std::string& key, PropertyValue value) {
    if (!isEntityIndex(index) || key.empty()) {
        return false;
    }
    const PropertyMap& properties = _entities[index].properties;
    if (const auto found = properties.find(key);
        found != properties.end() && found->second == value) {
        return false;
    }
    pushUndo();
    _entities[index].properties[key] = std::move(value);
    return true;
}

bool LevelDraft::removeEntityProperty(std::size_t index, const std::string& key) {
    if (!isEntityIndex(index) || !_entities[index].properties.contains(key)) {
        return false;
    }
    pushUndo();
    _entities[index].properties.erase(key);
    return true;
}

std::optional<std::size_t> LevelDraft::entityAt(GridPosition position) const {
    for (std::size_t index = _entities.size(); index > 0; --index) {
        if (_entities[index - 1].position == position) {
            return index - 1;
        }
    }
    return std::nullopt;
}

void LevelDraft::resize(int width, int height) {
    pushUndo();
    _tileMap = resizedCopy(_tileMap, width, height);
    // Les couches suivent la grille racine (LOT-04) : toutes les couches d'une carte partagent ses
    // dimensions, c'est ce que le chargeur verifie a la relecture.
    for (TileLayer& layer : _layers) {
        layer.tiles = resizedCopy(layer.tiles, width, height);
    }

    if (_entry && !_tileMap.inBounds(_entry->column, _entry->row)) {
        _entry.reset();
    }
    std::erase_if(_textureOverrides, [this](const TileTextureOverride& override) {
        return !_tileMap.inBounds(override.position.column, override.position.row);
    });
    // Une entite est keyee par sa case : hors de la nouvelle grille, elle n'a plus de place ou
    // exister, et la garder rendrait la carte irrecuperable (EX-LVL-017).
    std::erase_if(_entities, [this](const MapEntity& entity) {
        return !_tileMap.inBounds(entity.position.column, entity.position.row);
    });
}

bool LevelDraft::wouldResizeDropContent(int width, int height) const noexcept {
    const auto outOfBounds = [width, height](GridPosition position) {
        return position.column < 0 || position.column >= width || position.row < 0 ||
               position.row >= height;
    };
    if (_entry && outOfBounds(*_entry)) {
        return true;
    }
    for (const TileTextureOverride& override : _textureOverrides) {
        if (outOfBounds(override.position)) {
            return true;
        }
    }
    return std::ranges::any_of(_entities, [&outOfBounds](const MapEntity& entity) {
        return outOfBounds(entity.position);
    });
}

bool LevelDraft::undo() {
    if (_undoHistory.empty()) {
        return false;
    }
    _redoHistory.push_back(snapshot());
    restore(std::move(_undoHistory.back()));
    _undoHistory.pop_back();
    return true;
}

bool LevelDraft::redo() {
    if (_redoHistory.empty()) {
        return false;
    }
    _undoHistory.push_back(snapshot());
    restore(std::move(_redoHistory.back()));
    _redoHistory.pop_back();
    return true;
}

LevelDraft::State LevelDraft::snapshot() const {
    return State{.name = _name,
                 .tileMap = _tileMap,
                 .entry = _entry,
                 .layers = _layers,
                 .entities = _entities,
                 .textureOverrides = _textureOverrides};
}

void LevelDraft::restore(State state) {
    _name = std::move(state.name);
    _tileMap = std::move(state.tileMap);
    _entry = state.entry;
    _layers = std::move(state.layers);
    _entities = std::move(state.entities);
    _textureOverrides = std::move(state.textureOverrides);
}

void LevelDraft::pushUndo() {
    _undoHistory.push_back(snapshot());
    _redoHistory.clear();  // une nouvelle mutation invalide la branche de refaire
}

LevelLoadResult LevelDraft::toLevel() const {
    // La grille editee EST la couche de collision de la carte (LOT-04) : le brouillon n'en peint
    // qu'une, et laisser la couche de collision figee sur l'etat du fichier d'origine produirait
    // une carte ou l'on traverse un mur qu'on voit. Les couches visuelles -- sol, decor -- sont
    // celles que les mutateurs de couches ont peintes (LOT-11).
    std::vector<TileLayer> layers = _layers;
    for (TileLayer& layer : layers) {
        if (layer.kind == LayerKind::Collision || layer.kind == LayerKind::Legacy) {
            layer.tiles = _tileMap;
        }
    }
    const std::string json = LevelWriter::buildJson(LevelData{.name = _name,
                                                              .tileMap = _tileMap,
                                                              .layers = std::move(layers),
                                                              .entities = _entities,
                                                              .textureOverrides = _textureOverrides});
    return LevelLoader::loadFromString(json);
}

void LevelDraft::removeTextureOverrideAt(GridPosition position) {
    std::erase_if(_textureOverrides, [position](const TileTextureOverride& override) {
        return override.position == position;
    });
}

}  // namespace core
