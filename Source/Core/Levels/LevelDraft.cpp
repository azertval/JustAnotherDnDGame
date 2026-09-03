// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Levels/LevelDraft.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "Core/Diagnostics/Assert.h"
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
    draft._entry = level.entry();
    draft._exit = level.exit();
    draft._mechanisms = level.mechanisms();
    draft._background = level.background();
    draft._skinSet = level.skinSet();
    draft._textureOverrides = level.textureOverrides();
    draft._cameraFraming = level.cameraFraming();
    // Plans picturaux et drapeau de parallaxe (LOT-69). INVARIANT DE CE CONSTRUCTEUR : fromLevel
    // recopie TOUS les champs de Level, sans exception. Un brouillon reenregistre doit etre
    // equivalent au niveau d'origine -- un champ non recopie ne fait echouer aucun appel, il
    // s'efface simplement au premier enregistrement. Ajouter un champ a Level, c'est l'ajouter ici.
    draft._planes = level.planes();
    draft._parallaxEnabled = level.parallaxEnabled();
    // Couches et entites (LOT-04), au titre du meme invariant : portees telles quelles jusqu'a
    // l'enregistrement, l'editeur ne sachant pas encore les manipuler (LOT-11).
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
    if (type == TileType::Exit) {
        setExitInternal(column, row);
        return;
    }

    const GridPosition position{.column = column, .row = row};
    if (_entry && *_entry == position) {
        _entry.reset();
    }
    if (_exit && *_exit == position) {
        _exit.reset();
    }
    // Reposer le meme type ne doit pas effacer un habillage (EX-EDIT-043) : un coup de pinceau
    // involontaire sur une case deja du bon type effacerait sinon un override de texture.
    const bool sameType = _tileMap.tile(column, row) == type;
    removeLinkedDataAt(position, /*keepTextureOverride=*/sameType);
    _tileMap.setTile(column, row, type);

    // Meme defaut que LevelLoader (EX-GP-026/EX-GP-051) : une plateforme ou un danger mobile
    // fraichement pose porte IMMEDIATEMENT sa configuration par defaut. Sans cette entree, l'outil
    // Parcours ne le trouve dans aucun des vecteurs qu'il parcourt (hmi::designatePathAt) tant que
    // le niveau n'a pas ete sauvegarde puis recharge (seul LevelLoader la creait jusqu'ici) : la
    // tuile reste invisible pour la selection, et son parcours impossible a commencer.
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
    removeLinkedDataAt(position);
    _tileMap.setTile(column, row, TileType::Entry);
    _entry = position;
}

void LevelDraft::setExit(int column, int row) {
    pushUndo();
    setExitInternal(column, row);
}

void LevelDraft::setExitInternal(int column, int row) {
    const GridPosition position{.column = column, .row = row};
    if (_exit && *_exit != position) {
        _tileMap.setTile(_exit->column, _exit->row, TileType::Empty);
    }
    removeLinkedDataAt(position);
    _tileMap.setTile(column, row, TileType::Exit);
    _exit = position;
}

void LevelDraft::linkMechanism(GridPosition switchPosition, GridPosition targetPosition) {
    // switchTile n'est lu que par l'assertion qui suit, laquelle disparait en Release : son
    // usage depend donc de la configuration, d'ou l'attribut.
    [[maybe_unused]] const TileType switchTile =
        _tileMap.inBounds(switchPosition.column, switchPosition.row)
            ? _tileMap.tile(switchPosition.column, switchPosition.row)
            : TileType::Empty;
    JADG_ASSERT(switchTile == TileType::Switch || switchTile == TileType::PressurePlate ||
                    switchTile == TileType::Key,
                "linkMechanism : la position source ne porte pas de declencheur "
                "(interrupteur, plaque de pression ou cle)");
    [[maybe_unused]] const TileType targetTile =
        _tileMap.inBounds(targetPosition.column, targetPosition.row)
            ? _tileMap.tile(targetPosition.column, targetPosition.row)
            : TileType::Empty;
    JADG_ASSERT(targetTile == TileType::Door || targetTile == TileType::LockedDoor,
                "linkMechanism : la position cible ne porte pas de porte "
                "ni de porte verrouillee");

    pushUndo();
    // Retrait direct (sans passer par unlinkMechanism, qui empilerait un second snapshot) :
    // lier remplace une eventuelle liaison existante en une seule action undoable.
    std::erase_if(_mechanisms, [targetPosition](const Mechanism& mechanism) {
        return mechanism.doorPosition == targetPosition;
    });
    // Meme vecteur pour Door ET LockedDoor (aucune notion de liaison dupliquee, LOT-63
    // TACHE-02) : core::MechanismController distingue leur comportement a la construction.
    _mechanisms.push_back(
        Mechanism{.switchPosition = switchPosition, .doorPosition = targetPosition});
}

void LevelDraft::unlinkMechanism(GridPosition targetPosition) {
    pushUndo();
    std::erase_if(_mechanisms, [targetPosition](const Mechanism& mechanism) {
        return mechanism.doorPosition == targetPosition;
    });
}

void LevelDraft::setTextureOverride(GridPosition position, std::string assetName) {
    pushUndo();
    std::erase_if(_textureOverrides, [position](const TileTextureOverride& override) {
        return override.position == position;
    });
    _textureOverrides.push_back(
        TileTextureOverride{.position = position, .assetName = std::move(assetName)});
}

void LevelDraft::removeTextureOverride(GridPosition position) {
    pushUndo();
    std::erase_if(_textureOverrides, [position](const TileTextureOverride& override) {
        return override.position == position;
    });
}

// --- Plans picturaux (EX-DEC-040, LOT-69) ---
//
// Discipline commune : un rang hors bornes ne fait RIEN et n'empile RIEN (sinon annuler un geste
// sans effet consommerait un pas d'historique), et une valeur refusee n'empile pas davantage. Le
// pushUndo() vient donc toujours APRES les validations, jamais avant.
//
// Le deplacement ne raisonne pas par couche, contrairement aux decors : le rang dans la liste EST
// l'ordre de superposition, la profondeur etant une propriete independante (EX-DEC-042).

void LevelDraft::addPlane(Plane plane) {
    pushUndo();
    _planes.push_back(std::move(plane));
}

void LevelDraft::removePlane(std::size_t index) {
    if (index >= _planes.size()) {
        return;
    }
    pushUndo();
    _planes.erase(_planes.begin() + static_cast<std::ptrdiff_t>(index));
}

bool LevelDraft::setPlaneDensity(std::size_t index, int pixelsPerUnit) {
    if (index >= _planes.size() || !isValidPlaneDensity(pixelsPerUnit)) {
        return false;
    }
    pushUndo();
    _planes[index].pixelsPerUnit = pixelsPerUnit;
    return true;
}

bool LevelDraft::setPlaneParallax(std::size_t index, float parallaxX, float parallaxY) {
    if (index >= _planes.size() || !std::isfinite(parallaxX) || !std::isfinite(parallaxY)) {
        return false;
    }
    pushUndo();
    _planes[index].parallaxX = parallaxX;
    _planes[index].parallaxY = parallaxY;
    return true;
}

bool LevelDraft::setPlaneOpacity(std::size_t index, float opacity) {
    // Comparaisons ecrites en positif pour rejeter aussi NaN, qui echoue toute comparaison.
    if (index >= _planes.size() || !(opacity >= 0.0F) || !(opacity <= 1.0F)) {
        return false;
    }
    pushUndo();
    _planes[index].opacity = opacity;
    return true;
}

bool LevelDraft::setPlaneDepth(std::size_t index, PlaneDepth depth) {
    if (index >= _planes.size()) {
        return false;
    }
    pushUndo();
    _planes[index].depth = depth;
    return true;
}

std::optional<std::size_t> LevelDraft::movePlaneForward(std::size_t index) {
    if (index >= _planes.size()) {
        return std::nullopt;
    }
    if (index + 1 == _planes.size()) {
        return index;  // deja le plus en avant : succes sans effet, rien d'empile.
    }
    pushUndo();
    std::swap(_planes[index], _planes[index + 1]);
    return index + 1;
}

std::optional<std::size_t> LevelDraft::movePlaneBackward(std::size_t index) {
    if (index >= _planes.size()) {
        return std::nullopt;
    }
    if (index == 0) {
        return index;  // deja le plus en arriere.
    }
    pushUndo();
    std::swap(_planes[index], _planes[index - 1]);
    return index - 1;
}

std::optional<std::size_t> LevelDraft::movePlaneToFront(std::size_t index) {
    if (index >= _planes.size()) {
        return std::nullopt;
    }
    const std::size_t last = _planes.size() - 1;
    if (index == last) {
        return index;
    }
    pushUndo();
    Plane moved = std::move(_planes[index]);
    _planes.erase(_planes.begin() + static_cast<std::ptrdiff_t>(index));
    _planes.push_back(std::move(moved));
    return last;
}

std::optional<std::size_t> LevelDraft::movePlaneToBack(std::size_t index) {
    if (index >= _planes.size()) {
        return std::nullopt;
    }
    if (index == 0) {
        return index;
    }
    pushUndo();
    Plane moved = std::move(_planes[index]);
    _planes.erase(_planes.begin() + static_cast<std::ptrdiff_t>(index));
    _planes.insert(_planes.begin(), std::move(moved));
    return 0;
}

void LevelDraft::setParallaxEnabled(bool enabled) {
    pushUndo();
    _parallaxEnabled = enabled;
}

void LevelDraft::setBackground(std::optional<std::string> background) {
    pushUndo();
    _background = std::move(background);
}

void LevelDraft::setSkinSet(std::optional<std::string> skinSet) {
    pushUndo();
    _skinSet = std::move(skinSet);
}

void LevelDraft::setCameraFraming(CameraFramingConfig cameraFraming) {
    pushUndo();
    _cameraFraming = cameraFraming;
}

void LevelDraft::addCameraZone(CameraZone zone) {
    pushUndo();
    _cameraFraming.zones.push_back(zone);
}

void LevelDraft::removeCameraZone(std::size_t index) {
    if (index >= _cameraFraming.zones.size()) {
        return;
    }
    pushUndo();
    _cameraFraming.zones.erase(_cameraFraming.zones.begin() + static_cast<std::ptrdiff_t>(index));
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
    if (_exit && !_tileMap.inBounds(_exit->column, _exit->row)) {
        _exit.reset();
    }
    std::erase_if(_mechanisms, [this](const Mechanism& mechanism) {
        return !_tileMap.inBounds(mechanism.switchPosition.column, mechanism.switchPosition.row) ||
               !_tileMap.inBounds(mechanism.doorPosition.column, mechanism.doorPosition.row);
    });
    std::erase_if(_textureOverrides, [this](const TileTextureOverride& override) {
        return !_tileMap.inBounds(override.position.column, override.position.row);
    });
    // Une entite est keyee par sa case, comme les mecanismes : hors de la nouvelle grille, elle
    // n'a plus de place ou exister, et la garder rendrait le niveau irrecuperable (EX-LVL-017).
    std::erase_if(_entities, [this](const MapEntity& entity) {
        return !_tileMap.inBounds(entity.position.column, entity.position.row);
    });
    // _decors n'est volontairement PAS filtre : contrairement aux autres donnees annexes (keyees
    // par case), un decor libre peut legitimement deborder du niveau (une branche qui depasse) --
    // le tronquer serait une perte de travail (TACHE-01).
}

bool LevelDraft::wouldResizeDropContent(int width, int height) const noexcept {
    const auto outOfBounds = [width, height](GridPosition position) {
        return position.column < 0 || position.column >= width || position.row < 0 ||
               position.row >= height;
    };
    if (_entry && outOfBounds(*_entry)) {
        return true;
    }
    if (_exit && outOfBounds(*_exit)) {
        return true;
    }
    for (const Mechanism& mechanism : _mechanisms) {
        if (outOfBounds(mechanism.switchPosition) || outOfBounds(mechanism.doorPosition)) {
            return true;
        }
    }
    for (const TileTextureOverride& override : _textureOverrides) {
        if (outOfBounds(override.position)) {
            return true;
        }
    }
    for (const MapEntity& entity : _entities) {
        if (outOfBounds(entity.position)) {
            return true;
        }
    }
    return false;
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
                 .exit = _exit,
                 .mechanisms = _mechanisms,
                 .layers = _layers,
                 .entities = _entities,
                 .background = _background,
                 .skinSet = _skinSet,
                 .textureOverrides = _textureOverrides,
                 .cameraFraming = _cameraFraming,
                 .planes = _planes,
                 .parallaxEnabled = _parallaxEnabled};
}

void LevelDraft::restore(State state) {
    _name = std::move(state.name);
    _tileMap = std::move(state.tileMap);
    _entry = state.entry;
    _exit = state.exit;
    _mechanisms = std::move(state.mechanisms);
    _layers = std::move(state.layers);
    _entities = std::move(state.entities);
    _background = std::move(state.background);
    _skinSet = std::move(state.skinSet);
    _textureOverrides = std::move(state.textureOverrides);
    _cameraFraming = state.cameraFraming;
    _planes = std::move(state.planes);
    _parallaxEnabled = state.parallaxEnabled;
}

void LevelDraft::pushUndo() {
    _undoHistory.push_back(snapshot());
    _redoHistory.clear();  // une nouvelle mutation invalide la branche de refaire
}

LevelLoadResult LevelDraft::toLevel() const {
    // La grille editee EST la couche de collision de la carte (LOT-04) : le brouillon n'en peint
    // qu'une, et laisser la couche de collision figee sur l'etat du fichier d'origine produirait
    // une carte ou l'on traverse un mur qu'on voit. Les autres couches -- sol, decor -- passent
    // telles quelles : l'editeur ne sait pas encore les toucher (LOT-11).
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
                                                              .mechanisms = _mechanisms,
                                                              .background = _background,
                                                              .skinSet = _skinSet,
                                                              .textureOverrides = _textureOverrides,
                                                              .cameraFraming = _cameraFraming,
                                                              .planes = _planes,
                                                              .parallaxEnabled = _parallaxEnabled});
    return LevelLoader::loadFromString(json);
}

void LevelDraft::removeLinkedDataAt(GridPosition position, bool keepTextureOverride) {
    std::erase_if(_mechanisms, [position](const Mechanism& mechanism) {
        return mechanism.switchPosition == position || mechanism.doorPosition == position;
    });
    if (!keepTextureOverride) {
        std::erase_if(_textureOverrides, [position](const TileTextureOverride& override) {
            return override.position == position;
        });
    }
}

}  // namespace core
