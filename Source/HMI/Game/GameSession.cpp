// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Game/GameSession.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Core/Ecs/Components/Actor.h"
#include "Core/Ecs/Components/Animation.h"
#include "Core/Ecs/Components/Collider.h"
#include "Core/Ecs/Components/Sprite.h"  // core::AtlasRegion, core::Color
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Ecs/Systems/TopDownMovementSystem.h"
#include "Core/Levels/LevelScene.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Rect.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/AabbVsAabb.h"
#include "Core/Physics/PlayerInput.h"
#include "Core/Physics/PlayerSpawn.h"
#include "HMI/Game/ExplorationMode.h"
#include "HMI/Game/GameHud.h"
#include "HMI/Graphics/AnimationCatalog.h"
#include "HMI/Graphics/BitmapFont.h"
#include "HMI/Graphics/MechanismVisuals.h"
#include "HMI/Graphics/Parallax.h"
#include "HMI/Graphics/PlayerSprite.h"
#include "HMI/Graphics/PlayerSpriteTag.h"
#include "HMI/Graphics/PreviousPosition.h"
#include "HMI/Graphics/RenderLayer.h"
#include "HMI/Graphics/TextRenderer.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TextureCache.h"
#include "HMI/Graphics/TileAppearance.h"
#include "HMI/Graphics/TileAutotile.h"
#include "HMI/Graphics/TileSkinTag.h"
#include "HMI/Graphics/TileVisuals.h"
#include "HMI/HmiLog.h"
#include "HMI/Input/InputState.h"
#include "HMI/Input/PlayerInputMapper.h"
#include "HMI/Localization/Localization.h"
#include "HMI/Platform/ExecutableDirectory.h"

namespace hmi {

namespace {
// Nombre d'images procedurales d'un PlayerClipKind (ProceduralAtlas.h) : seule source de verite
// deja etablie par LOT-18/LOT-46, reprise ici pour borner animation.frameIndex (Core) au cycle
// procedural, quel que soit le nombre d'images du clip Core resolu (LOT-48).
int proceduralFrameCount(PlayerClipKind kind) {
    switch (kind) {
        case PlayerClipKind::Idle:
            return PLAYER_IDLE_FRAME_COUNT;
        case PlayerClipKind::Run:
            return PLAYER_RUN_FRAME_COUNT;
        case PlayerClipKind::Jump:
            return PLAYER_JUMP_FRAME_COUNT;
    }
    return 1;
}

// Traduit un NOM de clip resolu par Core (core::playerClipSet()->clipAt(...).name) en identite de
// clip cote presentation (hmi::PlayerClipKind, ProceduralAtlas.h). L'atlas procedural ne sait
// dessiner que trois poses (Idle/Run/Jump, LOT-18) : les quatre clips LOT-48 (fall/land/wallslide/
// dash) retombent sur le plus proche via la MEME chaine de repli qu'une spritesheet externe
// partielle (hmi::resolveDeclaredPlayerClip, hmi::proceduralPlayerClipNames) -- l'atlas procedural
// est traite comme une spritesheet qui n'en declare que trois.
PlayerClipKind proceduralClipKindFor(const std::string& clipName) {
    const std::string resolved = resolveDeclaredPlayerClip(proceduralPlayerClipNames(), clipName);
    if (resolved == "run") {
        return PlayerClipKind::Run;
    }
    if (resolved == "jump") {
        return PlayerClipKind::Jump;
    }
    return PlayerClipKind::Idle;
}

// Zoom et marge visuelle communs aux trois modes de cadrage (LOT-64) : une marge UNIQUE, la meme
// quel que soit le mode -- aucune raison visuelle de la faire varier, et une marge par mode se
// verrait comme un saut a chaque changement de cadrage.
constexpr float CAMERA_FIT_MARGIN = 0.92f;
}  // namespace

GameSession::GameSession(SpriteBatch& batch, const TextureAtlas& atlas, TextureCache& cache,
                         int viewportWidth, int viewportHeight, core::Level level,
                         const GameKeyBindings& gameBindings,
                         const GamepadBindings& gamepadBindings, const BitmapFont& font,
                         const Localization* localization)
    : _atlas(atlas),
      _cache(cache),
      _gameBindings(gameBindings),
      _gamepadBindings(gamepadBindings),
      _batch(batch),
      _font(font),
      _localization(localization),
      _camera(viewportWidth, viewportHeight),
      _renderer(batch, atlas, cache) {
    // Images des plans (LOT-69 TACHE-05) : a cote des niveaux, pas sous Assets/ -- un plan est une
    // donnee de niveau, jamais un asset reutilisable.
    _renderer.setPlanesDirectory(executableDirectory() / "Levels" / "Plans");
    // Mode de jeu par defaut (LOT-05) : l'exploration, seul mode existant. Pose AVANT le
    // chargement, pour que son onLoad() voie un niveau pret.
    _mode = std::make_unique<ExplorationMode>();
    loadLevel(std::move(level));
    _mode->onLoad(*this);
}

// (Re)construit la scene pour un niveau deja charge et valide : monde neuf + grille + personnage
// a l'entree. Coeur commun a la construction et aux rechargements (echec).
void GameSession::loadLevel(core::Level level) {
    // Particules (LOT-53 TACHE-02) : videes AVANT de reinitialiser _world -- les entites qu'elles
    // detiennent encore appartiennent a l'ANCIEN monde, sur le point d'etre remplace ci-dessous.
    // Un ordre inverse laisserait des handles perimes que le nouveau monde (index/generation
    // repartis a zero) pourrait faire coincider avec de toutes autres entites.
    _particles.clear(_world);
    _screenShake = ScreenShakeState{};  // aucune secousse residuelle sur le niveau rechargé
    _world = core::World{};  // repart d'un monde vierge (aucune entite du niveau precedent)
    _loadError.clear();
    // Detection d'evenements (LOT-60 TACHE-03) : un (re)chargement change l'etat sous les pieds de
    // la detection (personnage/mecanismes remis) -- sans ce reset, le premier pas du niveau
    // rechargé se comparerait a l'etat du niveau precedent et fabriquerait de faux evenements.
    // NE PAS vider _lastStepEvents ici : reload() est appele DEPUIS update(), apres que
    // l'evenement Died y a ete pousse -- le vider ici l'effacerait avant que l'appelant ne l'ait
    // jamais vu (mort et rechargement se suivent dans le meme pas).
    _gameEventsInitialized = false;

    _level = std::move(level);              // conserve le niveau pour la simulation et le reset
    const core::Level& levelRef = *_level;  // level est deplace : plus lu au-dela de cette ligne
    _levelWidth = levelRef.tileMap().width();
    _levelHeight = levelRef.tileMap().height();
    // Cadrage de camera (LOT-64) : copie une fois, resolu par LevelLoader -- jamais recalcule
    // ici, la regle de repli vit a un seul endroit (core::resolveCameraFraming).
    _cameraFraming = levelRef.cameraFraming();
    // Partition en salles (LOT-32), a la taille RESOLUE du niveau (LOT-64 : reglable, valeurs par
    // defaut sinon) : reconstruite pour ce niveau, camera immediatement cadree sur la salle de
    // l'ENTREE (pas de salle "en retard" d'une frame apres un (re)chargement) -- meme si le mode
    // retenu n'est pas "par salle", la partition reste bon marche a construire et sert de repere
    // a l'editeur (TACHE-03).
    _roomGrid.emplace(_levelWidth, _levelHeight,
                      _cameraFraming.roomWidthTiles.value_or(core::DEFAULT_ROOM_WIDTH_TILES),
                      _cameraFraming.roomHeightTiles.value_or(core::DEFAULT_ROOM_HEIGHT_TILES));
    _currentRoomIndex = _roomGrid->roomIndexAt(levelRef.entry());
    // Caméra de suivi (LOT-64 TACHE-02) : réinitialisée à chaque chargement, elle démarrera sur le
    // personnage à l'entrée dès le premier `update()` (état non initialisé).
    _followCameraState = FollowCameraState{};
    _previousFollowCenter = core::Vector2{static_cast<float>(levelRef.entry().column) + 0.5f,
                                          static_cast<float>(levelRef.entry().row) + 0.5f};
    // Zone de camera active (mode par salle avec zones dessinees a la main, EX-LVL-007) : resolue
    // ici comme _currentRoomIndex ci-dessus, pour ne pas dependre d'un premier update() avant le
    // premier rendu.
    _currentZoneIndex = _cameraFraming.zones.empty()
                            ? std::nullopt
                            : activeCameraZoneIndex(_cameraFraming.zones, levelRef.entry());
    switch (_cameraFraming.mode) {
        case core::CameraFramingMode::WholeLevel:
            centerCameraOnWholeLevel();
            break;
        case core::CameraFramingMode::PerRoom:
            if (!_cameraFraming.zones.empty()) {
                if (_currentZoneIndex) {
                    centerCameraOnZone(_cameraFraming.zones[*_currentZoneIndex]);
                } else {
                    centerCameraOnWholeLevel();
                }
            } else {
                centerCameraOnRoom(_currentRoomIndex);
            }
            break;
        case core::CameraFramingMode::Follow:
            // Amorcé au premier update() ; place un centre raisonnable (l'entrée) pour un
            // éventuel rendu avant le premier pas fixe (chargement à mi-frame).
            _camera.setCenter(_previousFollowCenter);
            break;
    }
    // La correspondance type -> region d'atlas (rendu) est injectee dans la projection pure, et
    // chaque entite tuile recoit sa marque d'habillage (type + voisinage solide, LOT-42). Le
    // masque ne depend que de la grille du niveau : le calculer ici, une fois, evite de le
    // refaire a chaque image sans rendre la scene dependante du mode de rendu.
    const core::TileMap& sceneMap = levelRef.tileMap();
    core::buildLevelScene(
        _world, levelRef, [this](core::TileType type) { return regionForTile(type); },
        [this, &sceneMap, &levelRef](core::Entity entity, core::TileType type, int column,
                                     int row) {
            _world.addComponent(entity,
                                TileSkinTag{type, solidNeighborMask(sceneMap, column, row),
                                            textureOverrideAt(levelRef.textureOverrides(),
                                                              core::GridPosition{column, row})});
        });
    // Mecanismes : etat interrupteurs/portes + grille de collision (portes fermees = solides).
    _mechanisms.emplace(levelRef);
    // Repere l'entite-tuile de chaque porte (avant le spawn du perso) pour le retour visuel d'etat.
    _doorEntities.clear();
    for (const core::Mechanism& mechanism : _mechanisms->mechanisms()) {
        core::Entity doorEntity{};
        bool found = false;
        _world.view<core::Transform, core::Sprite>().each(
            [&](core::Entity entity, core::Transform& transform, core::Sprite&) {
                if (!found &&
                    static_cast<int>(transform.position.x) == mechanism.doorPosition.column &&
                    static_cast<int>(transform.position.y) == mechanism.doorPosition.row) {
                    doorEntity = entity;
                    found = true;
                }
            });
        _doorEntities.push_back(doorEntity);
    }
    _doorVisuals.assign(_doorEntities.size(), MechanismVisualState{});
    // Repere l'entite-tuile du DECLENCHEUR (interrupteur/plaque de pression) de chaque mecanisme,
    // meme ordre que _doorEntities (LOT-47 : le declencheur change aussi d'apparence).
    _switchEntities.clear();
    for (const core::Mechanism& mechanism : _mechanisms->mechanisms()) {
        core::Entity switchEntity{};
        bool found = false;
        _world.view<core::Transform, core::Sprite>().each(
            [&](core::Entity entity, core::Transform& transform, core::Sprite&) {
                if (!found &&
                    static_cast<int>(transform.position.x) == mechanism.switchPosition.column &&
                    static_cast<int>(transform.position.y) == mechanism.switchPosition.row) {
                    switchEntity = entity;
                    found = true;
                }
            });
        _switchEntities.push_back(switchEntity);
    }
    _switchVisuals.assign(_switchEntities.size(), MechanismVisualState{});
    spawnPlayer(levelRef.entry());
    HMI_LOG_INFO("Niveau charge : " + levelRef.name() + " (" + std::to_string(_levelWidth) + "x" +
                 std::to_string(_levelHeight) + ")");
}

void GameSession::setGameMode(std::unique_ptr<IGameMode> mode) {
    if (!mode) {
        // Une session sans mode n'aurait plus d'ordre de passes du tout : on garde le mode
        // courant plutot que de se retrouver sans (EX-NFR-040).
        HMI_LOG_WARNING("Mode de jeu nul refuse : le mode courant est conserve");
        return;
    }
    _mode->onUnload(*this);
    _mode = std::move(mode);
    _mode->onLoad(*this);
    HMI_LOG_INFO("Mode de jeu : " + std::string(_mode->name()));
}

void GameSession::reload() {
    if (_level) {
        loadLevel(*_level);  // recharge depuis le Level en memoire (perso a l'entree, etat remis)
    }
}

// Fait apparaitre le personnage humanoide (0,4x0,8), centre dans la tuile d'entree.
void GameSession::spawnPlayer(core::GridPosition entry) {
    _player = _world.createEntity();
    const core::Vector2 size = core::playerSize();  // collision ET rendu partagent la meme taille
    _world.addComponent(
        _player, core::Transform{core::playerSpawnPosition(entry.column, entry.row), size, 0.0f});
    _world.addComponent(_player, core::Velocity{});
    _world.addComponent(_player, core::Collider{size});
    _world.addComponent(_player, core::Actor{});
    core::Animation animation;
    animation.clips = core::playerClipSet();
    animation.clipIndex = core::PLAYER_CLIP_IDLE;
    _world.addComponent(_player, animation);
    core::Sprite sprite;
    sprite.region = _atlas.playerFrameRegion(PlayerClipKind::Idle, 0);
    sprite.tint = core::Color{1.0f, 1.0f, 1.0f, 1.0f};
    _world.addComponent(_player, sprite);
    // Habillage du personnage (LOT-48) : resolu chaque image par refreshPlayerSprite, valeurs
    // par defaut sans effet tant qu'un premier appel n'a pas eu lieu (RenderMode::Physique inchange
    // entre-temps, puisqu'il ne consulte que core::Sprite::region ci-dessus).
    _world.addComponent(_player, PlayerSpriteTag{});
    // Calque de dessin nomme (LOT-40, EX-REN-014) : le personnage passe devant les tuiles parce
    // qu'il est sur RenderLayer::Player, plus parce qu'on lui aurait attribue un entier plus grand.
    _world.addComponent(_player, RenderLayerTag{RenderLayer::Player});
    _world.addComponent(_player,
                        PreviousPosition{core::playerSpawnPosition(entry.column, entry.row)});
}

// Applique la correspondance etat -> clip a UNE entite-tuile de mecanisme (voir en-tete).
void GameSession::applyMechanismVisual(core::Entity entity, bool active,
                                       MechanismVisualState& state, const SceneTextures& textures,
                                       float fixedDelta) {
    if (!_world.hasComponent<TileSkinTag>(entity) || !_world.hasComponent<core::Sprite>(entity)) {
        return;  // entite-tuile non reperee (robustesse) : rien a faire, meme garde que le reste.
    }
    TileSkinTag& tag = _world.getComponent<TileSkinTag>(entity);
    // Par defaut : pas d'image par instance -- repli sur l'horloge partagee par asset (LOT-46) ou
    // sur l'image entiere, selon ce que resolveTileAppearance decide plus bas au rendu.
    tag.animatedFrame.reset();

    if (!isStatefulMechanism(tag.type)) {
        return;
    }

    // Asset effectivement lie a CETTE tuile, via le point de resolution UNIQUE (LOT-41) -- jamais
    // duplique ici : la hierarchie surcharge (LOT-45) > skin de type (LOT-42) > damier reste celle
    // du rendu. On ignore la region retournee (calculee sans connaitre l'etat), seuls la source et
    // l'index servent a retrouver l'asset et ses dimensions.
    const core::Sprite& sprite = _world.getComponent<core::Sprite>(entity);
    // Axes skin/surcharge (LOT-51) tous deux visibles par defaut : en mode compose, une valeur est
    // TOUJOURS renvoyee. GameSession n'expose pas ces axes -- ce sont des outils d'inspection de
    // l'editeur, sans objet en jeu.
    const TileAppearance appearance =
        resolveTileAppearance(RenderMode::Texture, sprite.region, &tag, textures).value();

    std::string assetPath;
    int width = 0;
    int height = 0;
    if (appearance.source == AppearanceSource::Skin) {
        const SkinTexture& skin = textures.skins[static_cast<std::size_t>(appearance.skinIndex)];
        assetPath = SKINS_SUBDIRECTORY + skin.asset;
        width = skin.width;
        height = skin.height;
    } else if (appearance.source == AppearanceSource::Override) {
        const SkinTexture& object =
            textures.objects[static_cast<std::size_t>(appearance.skinIndex)];
        assetPath = OBJECTS_SUBDIRECTORY + object.asset;
        width = object.width;
        height = object.height;
    } else {
        return;  // damier de repli (aucun asset assigne/charge) : rien a animer, deja journalise.
    }

    const AnimationDescription* description = _cache.getAnimation(assetPath, width, height);
    if (description == nullptr) {
        return;  // pas de fichier d'animation : image fixe, cas legitime et silencieux (LOT-46).
    }

    // Decision (transition/etat cible/repli) et progression : logique pure, testee hors GPU
    // (hmi::MechanismVisuals, LOT-47 TACHE-02) -- cette fonction ne fait que lui fournir l'asset
    // effectivement lie et ecrire le resultat sur la marque de presentation de la tuile.
    tag.animatedFrame = advanceMechanismVisual(state, *description, tag.type, active, assetPath,
                                               fixedDelta, _warnedMissingMechanismClips);
}

// Apparence des mecanismes pilotee par leur etat logique, au pas fixe (voir en-tete).
void GameSession::updateMechanismVisuals(float fixedDelta) {
    // Textures resolues UNE fois pour ce pas : c'est precisement ce pas qui calcule l'image par
    // instance de chaque mecanisme (tileAnimations vide -- l'horloge partagee par asset, LOT-46,
    // n'a rien a apporter ici, seule la resolution asset/dimensions de sceneTextures sert).
    const SceneTextures textures =
        sceneTextures(_atlas, _cache, _tileSkins, _tileSkinSet, _level->textureOverrides(), {});

    for (std::size_t index = 0; index < _doorEntities.size(); ++index) {
        applyMechanismVisual(_doorEntities[index], _mechanisms->isDoorOpen(index),
                             _doorVisuals[index], textures, fixedDelta);
    }
    for (std::size_t index = 0; index < _switchEntities.size(); ++index) {
        applyMechanismVisual(_switchEntities[index], _mechanisms->isDoorOpen(index),
                             _switchVisuals[index], textures, fixedDelta);
    }
}

// Modulation d'opacite de diagnostic, reservee au mode Physique (voir en-tete).
void GameSession::refreshMechanismDiagnosticTint(RenderMode mode) {
    constexpr float DOOR_OPEN_ALPHA = 0.25f;
    constexpr float DOOR_CLOSED_ALPHA = 1.0f;

    for (std::size_t index = 0; index < _doorEntities.size(); ++index) {
        const core::Entity door = _doorEntities[index];
        if (!_world.hasComponent<core::Sprite>(door)) {
            continue;  // porte non reperee (robustesse) : rien a faire.
        }
        const float alpha = mechanismDiagnosticAlpha(mode, _mechanisms->isDoorOpen(index),
                                                     DOOR_OPEN_ALPHA, DOOR_CLOSED_ALPHA);
        _world.getComponent<core::Sprite>(door).tint = core::Color{1.0f, 1.0f, 1.0f, alpha};
    }
}

std::vector<core::Aabb> GameSession::collectActiveDangerBoxes() {
    std::vector<core::Aabb> boxes;
    // Ecrasement par une PORTE qui se referme (EX-GP-021, LOT-65 TACHE-06) : mortel, traduit en
    // boite de danger supplementaire. Sans cela, le personnage reste encastre dans un mur sans
    // echec possible -- la « situation sans issue » que la conception des niveaux interdit.
    if (_world.hasComponent<core::Actor>(_player) && _mechanisms && _mechanisms->crushedPlayer()) {
        const core::Transform& squishedTransform = _world.getComponent<core::Transform>(_player);
        const core::Collider& squishedCollider = _world.getComponent<core::Collider>(_player);
        boxes.push_back(
            core::Aabb::fromTopLeftSize(squishedTransform.position, squishedCollider.size));
    }
    return boxes;
}

// Resout l'apparence du personnage pour l'image courante : region procedurale (Physique, et repli
// de Texture) + habillage Texture (spritesheet externe si chargee, LOT-48).
void GameSession::refreshPlayerSprite() {
    const core::Animation& animation = _world.getComponent<core::Animation>(_player);
    const core::Actor& actor = _world.getComponent<core::Actor>(_player);
    core::Sprite& sprite = _world.getComponent<core::Sprite>(_player);
    PlayerSpriteTag& tag = _world.getComponent<PlayerSpriteTag>(_player);

    const std::string clipName =
        animation.clips ? std::string(animation.clips->clipAt(animation.clipIndex).name) : "idle";

    // Region PROCEDURALE : comportement de RenderMode::Physique strictement inchange depuis avant
    // LOT-48 (core::Sprite::region), et repli de RenderMode::Texture en l'absence de spritesheet
    // externe (AC#2 du lot) -- calculee une seule fois, partagee par les deux usages.
    const PlayerClipKind proceduralKind = proceduralClipKindFor(clipName);
    const int proceduralFrame = animation.frameIndex % proceduralFrameCount(proceduralKind);
    const core::AtlasRegion proceduralRegion =
        _atlas.playerFrameRegion(proceduralKind, proceduralFrame);
    sprite.region = proceduralRegion;

    core::AtlasRegion textureRegion = proceduralRegion;
    bool usesCharacterSheet = false;
    core::Vector2 imageSizePixels{static_cast<float>(TextureAtlas::PLAYER_FRAME_SIZE),
                                  static_cast<float>(TextureAtlas::PLAYER_FRAME_SIZE)};

    if (const LoadedTexture* sheet =
            _cache.get(PLAYER_SUBDIRECTORY + PLAYER_SHEET_FILE_NAME, AssetFamily::CharacterSheet)) {
        if (const AnimationDescription* description = _cache.getAnimation(
                PLAYER_SUBDIRECTORY + PLAYER_SHEET_FILE_NAME, sheet->width, sheet->height)) {
            // Noms effectivement declares par CETTE spritesheet (peut-etre partielle) : le repli
            // (chute -> saut, atterrissage -> repos, ...) est le meme mecanisme que pour l'atlas
            // procedural ci-dessus, seul l'ensemble declare differe.
            std::vector<std::string> declaredNames;
            declaredNames.reserve(static_cast<std::size_t>(description->clips.clipCount()));
            for (int index = 0; index < description->clips.clipCount(); ++index) {
                declaredNames.emplace_back(description->clips.clipAt(index).name);
            }
            const std::string resolvedName = resolveDeclaredPlayerClip(declaredNames, clipName);
            const int resolvedIndex = description->clips.indexOf(resolvedName);
            const core::AnimationClip& sheetClip =
                description->clips.clipAt(resolvedIndex >= 0 ? resolvedIndex : 0);
            const int frameSheetIndex =
                sheetClip.frames.empty()
                    ? 0
                    : sheetClip.frames[static_cast<std::size_t>(animation.frameIndex) %
                                       sheetClip.frames.size()];
            textureRegion = AnimationCatalog::frameRegion(*description, frameSheetIndex);
            usesCharacterSheet = true;
            imageSizePixels = core::Vector2{static_cast<float>(description->frameWidth),
                                            static_cast<float>(description->frameHeight)};
        }
    }

    const PlayerSpriteQuad quad = computePlayerSpriteQuad(imageSizePixels, core::playerSize());
    tag.textureRegion = textureRegion;
    tag.usesCharacterSheet = usesCharacterSheet;
    tag.quadOffset = quad.offset;
    tag.quadSize = quad.size;
    // Orientation (LOT-48 TACHE-03) : derniere direction de marche, maintenue par le
    // deplacement (core::Actor::facing), sans que le rendu n'ait a la recalculer. Seule sa
    // composante horizontale se lit en miroir -- le vocabulaire de sprites RPG du LOT-08 dira quoi
    // faire des quatre orientations.
    tag.flipHorizontal = actor.facing.x < 0.0f;
}

// Avance l'horloge d'animation partagee des tuiles animees, au pas fixe (LOT-46 TACHE-05).
void GameSession::updateTileAnimations(float fixedDelta) {
    advanceTileAnimations(_tileSkins, _tileSkinSet, _cache, fixedDelta, _tileAnimations,
                          _warnedExcludedAnimations);
}

void GameSession::snapshotPreviousPositions() {
    _world.view<core::Transform, PreviousPosition>().each(
        [](core::Entity, const core::Transform& transform, PreviousPosition& previous) {
            previous.value = transform.position;
        });
    // Meme principe pour la camera de suivi (LOT-64 TACHE-02) : fige le centre du pas PRECEDENT
    // avant que ce pas ne le fasse avancer (updateFollowCamera), pour que render() interpole entre
    // les deux comme il le fait deja pour chaque PreviousPosition.
    _previousFollowCenter = _followCameraState.center;
}

void GameSession::centerCameraOnRoom(core::GridPosition roomIndex) {
    const RoomBounds bounds = _roomGrid->roomBounds(roomIndex);
    _camera.setCenter(
        core::Vector2{static_cast<float>(bounds.column) + static_cast<float>(bounds.width) * 0.5f,
                      static_cast<float>(bounds.row) + static_cast<float>(bounds.height) * 0.5f});
}

void GameSession::updateCurrentRoom() {
    const core::Transform& transform = _world.getComponent<core::Transform>(_player);
    const core::Collider& collider = _world.getComponent<core::Collider>(_player);
    const core::Vector2 center = transform.position + collider.size * 0.5f;
    const core::GridPosition tile{static_cast<int>(std::floor(center.x)),
                                  static_cast<int>(std::floor(center.y))};
    const core::GridPosition roomIndex = _roomGrid->roomIndexAt(tile);
    if (roomIndex != _currentRoomIndex) {
        _currentRoomIndex = roomIndex;
        centerCameraOnRoom(_currentRoomIndex);
    }
}

// Centre la camera sur le niveau entier (mode WholeLevel, LOT-64) : centre fixe, pose une fois au
// chargement -- symetrique a centerCameraOnRoom, jamais recalcule au pas fixe (voir en-tete).
void GameSession::centerCameraOnWholeLevel() {
    _camera.setCenter(core::Vector2{static_cast<float>(_levelWidth) * 0.5f,
                                    static_cast<float>(_levelHeight) * 0.5f});
}

// Centre la camera sur une zone dessinee a la main (EX-LVL-007, voir en-tete).
void GameSession::centerCameraOnZone(const core::CameraZone& zone) {
    _camera.setCenter(
        core::Vector2{static_cast<float>(zone.x) + static_cast<float>(zone.width) * 0.5f,
                      static_cast<float>(zone.y) + static_cast<float>(zone.height) * 0.5f});
}

// Equivalent de updateCurrentRoom pour les zones dessinees a la main (EX-LVL-007, voir en-tete).
void GameSession::updateCurrentCameraZone() {
    const core::Transform& transform = _world.getComponent<core::Transform>(_player);
    const core::Collider& collider = _world.getComponent<core::Collider>(_player);
    const core::Vector2 center = transform.position + collider.size * 0.5f;
    const core::GridPosition tile{static_cast<int>(std::floor(center.x)),
                                  static_cast<int>(std::floor(center.y))};
    const std::optional<std::size_t> zoneIndex = activeCameraZoneIndex(_cameraFraming.zones, tile);
    if (zoneIndex == _currentZoneIndex) {
        return;
    }
    _currentZoneIndex = zoneIndex;
    if (zoneIndex) {
        centerCameraOnZone(_cameraFraming.zones[*zoneIndex]);
    } else {
        // Aucune zone ne contient le personnage (trou entre les zones dessinees par l'auteur) :
        // repli sur le niveau entier, jamais un etat indefini.
        centerCameraOnWholeLevel();
    }
}

// Avance la camera de suivi d'un pas fixe (mode Follow, LOT-64 TACHE-02, voir en-tete).
void GameSession::updateFollowCamera(float fixedDelta) {
    const core::Transform& transform = _world.getComponent<core::Transform>(_player);
    const core::Collider& collider = _world.getComponent<core::Collider>(_player);
    // Position SIMULEE du personnage (pas interpolee) : le suivi (zone morte, anticipation,
    // lissage) doit rester deterministe au pas fixe (EX-NFR-002) -- c'est render() qui interpole
    // le CENTRE DE CAMERA resultant entre deux pas, exactement comme il interpole deja la position
    // affichee du personnage (PreviousPosition). Melanger les deux ici desynchroniserait le suivi
    // de la simulation qu'il est cense suivre.
    const core::Vector2 characterCenter = transform.position + collider.size * 0.5f;
    const core::Actor& actor = _world.getComponent<core::Actor>(_player);
    const core::Rect levelBounds{
        core::Vector2{0.0f, 0.0f},
        core::Vector2{static_cast<float>(_levelWidth), static_cast<float>(_levelHeight)}};
    // Cadrage de la camera de suivi : taille reglable par niveau (EX-REN-017, memes champs que la
    // taille de salle du mode par salle -- valeur par defaut si non declaree).
    const core::Vector2 viewHalfExtent{
        static_cast<float>(_cameraFraming.roomWidthTiles.value_or(core::DEFAULT_ROOM_WIDTH_TILES)) *
            0.5f,
        static_cast<float>(
            _cameraFraming.roomHeightTiles.value_or(core::DEFAULT_ROOM_HEIGHT_TILES)) *
            0.5f};
    // L'anticipation de la camera ne connait qu'un sens horizontal : elle regarde devant le
    // personnage le long de l'axe des x. Le suivi en huit directions (anticiper aussi vers le haut
    // et vers le bas) est une decision de cadrage, pas une consequence du deplacement -- elle
    // appartient au lot qui refera la camera pour la vue de dessus.
    _followCameraState = advanceFollowCamera(_followCameraState, characterCenter, actor.facing.x,
                                             levelBounds, viewHalfExtent, fixedDelta);
}

// Selectionne le zoom et le centre effectifs de _camera selon le mode de cadrage resolu (LOT-64,
// voir en-tete). Point d'application UNIQUE du cadrage : les trois modes y sont traites a plat,
// aucune regle de cadrage n'est dupliquee ailleurs dans render().
void GameSession::applyCameraFraming(int viewportWidth, int viewportHeight,
                                     float interpolationAlpha) {
    switch (_cameraFraming.mode) {
        case core::CameraFramingMode::WholeLevel: {
            const float zoom = Camera2D::fitZoom(
                static_cast<float>(viewportWidth), static_cast<float>(viewportHeight),
                static_cast<float>(_levelWidth), static_cast<float>(_levelHeight),
                CAMERA_FIT_MARGIN);
            _camera.setZoom(zoom);
            // Centre deja pose une fois au chargement (centerCameraOnWholeLevel) : fixe, rien a
            // refaire ici.
            break;
        }
        case core::CameraFramingMode::PerRoom: {
            // Rectangle a cadrer : la zone dessinee a la main active (EX-LVL-007), le niveau entier
            // en repli (aucune zone ne contient le personnage), ou la salle de la grille
            // automatique -- selon que _cameraFraming.zones est non vide ou non. Le centre, lui,
            // est deja pose par updateCurrentRoom/updateCurrentCameraZone au franchissement d'une
            // frontiere : rien a refaire ici.
            int width = 0;
            int height = 0;
            if (!_cameraFraming.zones.empty()) {
                if (_currentZoneIndex) {
                    const core::CameraZone& zone = _cameraFraming.zones[*_currentZoneIndex];
                    width = zone.width;
                    height = zone.height;
                } else {
                    width = _levelWidth;
                    height = _levelHeight;
                }
            } else {
                const RoomBounds roomBounds = _roomGrid->roomBounds(_currentRoomIndex);
                width = roomBounds.width;
                height = roomBounds.height;
            }
            const float zoom = Camera2D::fitZoom(
                static_cast<float>(viewportWidth), static_cast<float>(viewportHeight),
                static_cast<float>(width), static_cast<float>(height), CAMERA_FIT_MARGIN);
            _camera.setZoom(zoom);
            break;
        }
        case core::CameraFramingMode::Follow: {
            const float zoom = Camera2D::fitZoom(
                static_cast<float>(viewportWidth), static_cast<float>(viewportHeight),
                static_cast<float>(
                    _cameraFraming.roomWidthTiles.value_or(core::DEFAULT_ROOM_WIDTH_TILES)),
                static_cast<float>(
                    _cameraFraming.roomHeightTiles.value_or(core::DEFAULT_ROOM_HEIGHT_TILES)),
                CAMERA_FIT_MARGIN);
            _camera.setZoom(zoom);
            // Interpole entre le centre du pas fixe PRECEDENT et celui du pas COURANT, comme
            // chaque entite interpole entre PreviousPosition et sa position simulee (EX-ARCH-031)
            // -- sans quoi le personnage, rendu lisse, tremblerait par rapport au contenu cale
            // sur un centre qui ne bouge que par sauts discrets (tache-02, piege documente).
            const core::Vector2 interpolated =
                _previousFollowCenter +
                (_followCameraState.center - _previousFollowCenter) * interpolationAlpha;
            // Alignement pixel (EX-ARCH-022) : sans lui, un centre de camera fractionnaire
            // echantillonne chaque texture entre deux texels et rend tout le pixel art flou
            // (hmi::roundToScreenPixel).
            const float pixelsPerWorldUnit = Camera2D::PIXELS_PER_UNIT * zoom;
            _camera.setCenter(hmi::roundToScreenPixel(interpolated, pixelsPerWorldUnit));
            break;
        }
    }
}

// Simule le personnage d'un pas fixe (mecanismes + physique + animation), puis statue sur l'issue.
core::LevelOutcome GameSession::update(const InputState& input, float fixedDelta) {
    return update(toPlayerInput(input, _gameBindings, _gamepadBindings), fixedDelta);
}

core::LevelOutcome GameSession::update(const core::PlayerInput& intent, float fixedDelta) {
    if (!_level) {
        return core::LevelOutcome::Playing;  // chargement echoue : rien a simuler (etat neutre)
    }
    // L'ORDRE des passes appartient au mode, jamais a la session (LOT-05, EX-ARCH-002) : ici, plus
    // aucun choix -- ni sequence codee en dur, ni `if (mode == ...)`, seulement une delegation.
    return _mode->step(*this, intent, fixedDelta);
}

// --- Passes du pas fixe (hmi::IGameModePasses) --------------------------------------------------
//
// Chacune ne fait que ce que son nom dit. Le corps qu'elles se partagent est exactement celui que
// GameSession::update enchainait avant le LOT-05, decoupe sans rien y changer.

void GameSession::advanceParticles(float fixedDelta) {
    _particles.update(_world, fixedDelta);
}

void GameSession::advanceScreenShake(float fixedDelta) {
    hmi::advanceScreenShake(_screenShake, fixedDelta);
}

void GameSession::moveCharacter(const core::PlayerInput& input, float fixedDelta) {
    // Deplacement libre en 8 directions (LOT-06), contre la grille de COLLISION du niveau
    // (EX-LVL-016) : c'est elle, et pas le decor, qui arrete le personnage.
    core::updateTopDownMovement(_world, _player, input, _level->tileMap(), _movementConfig,
                                fixedDelta);
}

void GameSession::advanceAnimations(float fixedDelta) {
    // Animation (EX-REN-012) et tuiles animees (LOT-46 TACHE-05) : meme pas fixe que tout ce qui
    // precede, jamais le rythme du rendu (EX-NFR-002).
    _animation.update(_world, fixedDelta);
    updateTileAnimations(fixedDelta);
}

void GameSession::updateCamera(float fixedDelta) {
    switch (_cameraFraming.mode) {
        case core::CameraFramingMode::WholeLevel:
            break;
        case core::CameraFramingMode::PerRoom:
            if (!_cameraFraming.zones.empty()) {
                updateCurrentCameraZone();
            } else {
                updateCurrentRoom();
            }
            break;
        case core::CameraFramingMode::Follow:
            updateFollowCamera(fixedDelta);
            break;
    }
}

core::Aabb GameSession::playerBox() {
    const core::Transform& transform = _world.getComponent<core::Transform>(_player);
    const core::Collider& collider = _world.getComponent<core::Collider>(_player);
    return core::Aabb::fromTopLeftSize(transform.position, collider.size);
}

void GameSession::updateMechanisms(const core::PlayerInput& input) {
    // Contact interrupteurs (front) / poids sur plaque (continu) -> etat des portes. La liste de
    // poids est vide : les blocs poussables reviendront avec leur controleur top-down, seul le
    // personnage pese aujourd'hui sur une plaque.
    const float playerMass = _world.getComponent<core::Actor>(_player).mass;
    _mechanisms->update(playerBox(), playerMass, input.interactPressed, {});
}

void GameSession::detectEvents() {
    // Detection d'evenements (LOT-60 TACHE-03) : mecanismes a jour, une seule fois par pas fixe
    // (jamais par image de rendu, EX-REN-021). Plus d'evenements de PERSONNAGE depuis le LOT-06 :
    // saut, atterrissage et glissade murale etaient ceux d'un jeu de plateforme, et le deplacement
    // en vue de dessus n'en produit aucun equivalent -- le combat (LOT-21) dira ce qui les
    // remplace.
    const MechanismEventState currentMechanismEventState =
        MechanismEventState::capture(*_mechanisms);
    _lastStepEvents.clear();
    if (_gameEventsInitialized) {
        std::vector<bool> continuousMechanisms;
        continuousMechanisms.reserve(_mechanisms->mechanisms().size());
        for (std::size_t index = 0; index < _mechanisms->mechanisms().size(); ++index) {
            continuousMechanisms.push_back(_mechanisms->isContinuous(index));
        }
        const std::vector<GameEvent> mechanismEvents = detectMechanismEvents(
            _previousMechanismEventState, currentMechanismEventState, continuousMechanisms);
        _lastStepEvents.insert(_lastStepEvents.end(), mechanismEvents.begin(),
                               mechanismEvents.end());
    } else {
        // Tout premier pas apres un (re)chargement : rejoint l'etat courant sans transition, meme
        // principe que MechanismVisualState::initialized (LOT-47).
        _gameEventsInitialized = true;
    }
    _previousMechanismEventState = currentMechanismEventState;
}

core::LevelOutcome GameSession::evaluateOutcome() {
    const core::LevelOutcome outcome =
        core::evaluateOutcome(playerBox(), *_level, collectActiveDangerBoxes());
    // Evenement d'issue (LOT-60 TACHE-03) : pousse ICI, avant que onLevelLost() ne remette le
    // personnage et les mecanismes a l'etat d'entree -- apres, "Died" ne serait plus observable.
    if (const std::optional<GameEvent> outcomeEvent = detectOutcomeEvent(outcome)) {
        _lastStepEvents.push_back(*outcomeEvent);
    }
    return outcome;
}

void GameSession::onLevelLost() {
    // Eclatement a la mort (LOT-53 TACHE-02) : emis AVANT reload(), qui remet le personnage a
    // l'entree -- apres, l'eclatement partirait du point d'apparition.
    const core::Aabb box = playerBox();
    _particles.emitDeath(_world, (box.min + box.max) * 0.5f);
    triggerScreenShake(_screenShake, DEATH_SHAKE_AMPLITUDE_PIXELS, SCREEN_SHAKE_DURATION);
    reload();
}

// Dessine le niveau charge (rien si le chargement a echoue : l'appelant gere l'affichage d'erreur).
void GameSession::render(int viewportWidth, int viewportHeight, RenderMode mode,
                         float interpolationAlpha) {
    if (!_level) {
        return;
    }
    refreshPlayerSprite();
    // Modulation d'opacite de diagnostic, mode Physique uniquement (LOT-47 TACHE-03) : decision
    // purement visuelle et dependante du mode courant, elle vit ici plutot qu'au pas fixe.
    refreshMechanismDiagnosticTint(mode);

    _camera.setViewportSize(viewportWidth, viewportHeight);
    applyCameraFraming(viewportWidth, viewportHeight, interpolationAlpha);
    // Secousse d'ecran (LOT-53 TACHE-03) : decalage courant applique a la CAMERA DE RENDU
    // uniquement (Camera2D::setShakeOffsetPixels), jamais a _center -- sans effet sur le culling
    // ni sur la logique de cadrage (par salle ou de suivi, toutes deux pilotees par la position du
    // personnage, jamais par la secousse).
    _camera.setShakeOffsetPixels(screenShakeOffset(_screenShake));

    // Interpolation de rendu (EX-ARCH-031) entre le pas precedent et le pas courant. La grille de
    // collision des mecanismes (portes) tranche l'ombre d'une porte d'apres son etat COURANT
    // (LOT-55) : TileSkinTag::type reste TileType::Door quel que soit cet etat.
    _renderer.render(_world, _camera, mode, interpolationAlpha, _level->background(), _levelWidth,
                     _levelHeight, _level->textureOverrides(), _tileAnimations, _level->planes(),
                     hmi::planeParallaxActive(_cameraFraming.mode, _level->parallaxEnabled()),
                     _mechanisms ? &_mechanisms->collisionMap() : nullptr);

    renderHud(viewportWidth, viewportHeight);
}

// Compose et soumet l'affichage tete haute, en espace ecran (voir en-tete).
void GameSession::renderHud(int viewportWidth, int viewportHeight) {
    if (_localization == nullptr) {
        return;  // catalogue pas encore charge (demarrage) : pas de HUD plutot qu'un plantage.
    }

    constexpr float HUD_MARGIN = 8.0f;
    constexpr float HUD_SCALE = 1.0f;
    constexpr float HUD_LINE_SPACING = 2.0f;
    // Ombre portee (decalage d'un pixel, noir semi-opaque) : contraste suffisant sur un fond
    // clair comme sur un fond sombre, le fond de niveau etant libre (TACHE-03).
    constexpr core::Color HUD_SHADOW_COLOR{0.0f, 0.0f, 0.0f, 0.75f};
    constexpr core::Color HUD_TEXT_COLOR{1.0f, 1.0f, 1.0f, 1.0f};

    // Le personnage touche-t-il une cle non ramassee (EX-GP-023, LOT-65 TACHE-07) ? La porte
    // qu'ouvre une cle reste FERMEE tant que celle-ci n'est pas ramassee : `isDoorOpen` vaut donc
    // « cle deja prise », et une cle consommee ne doit plus rien afficher.
    const core::Transform& hudTransform = _world.getComponent<core::Transform>(_player);
    const core::Collider& hudCollider = _world.getComponent<core::Collider>(_player);
    const core::Aabb hudBox = core::Aabb::fromTopLeftSize(hudTransform.position, hudCollider.size);
    bool overlappingKey = false;
    for (std::size_t index = 0; index < _mechanisms->mechanisms().size(); ++index) {
        if (!_mechanisms->isKey(index) || _mechanisms->isDoorOpen(index)) {
            continue;
        }
        const core::GridPosition cell = _mechanisms->mechanisms()[index].switchPosition;
        const auto left = static_cast<float>(cell.column);
        const auto top = static_cast<float>(cell.row);
        if (hudBox.min.x < left + 1.0f && hudBox.max.x > left && hudBox.min.y < top + 1.0f &&
            hudBox.max.y > top) {
            overlappingKey = true;
            break;
        }
    }

    const std::vector<std::string> lines =
        gameHudLines(_level->name(), *_localization, overlappingKey);

    _hudScene.clear();
    float lineY = HUD_MARGIN;
    for (const std::string& line : lines) {
        composeText(_hudScene, _font, line, HUD_MARGIN + 1.0f, lineY + 1.0f, HUD_SCALE,
                    HUD_SHADOW_COLOR);
        composeText(_hudScene, _font, line, HUD_MARGIN, lineY, HUD_SCALE, HUD_TEXT_COLOR);
        lineY += static_cast<float>(_font.metrics().lineHeight) * HUD_SCALE + HUD_LINE_SPACING;
    }
    _hudScene.sort();
    submitComposedScene(_batch, screenProjectionMatrix(viewportWidth, viewportHeight), _hudScene);
}

}  // namespace hmi
