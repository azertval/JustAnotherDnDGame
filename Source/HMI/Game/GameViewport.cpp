// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Game/GameViewport.h"

#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPlatformSurfaceEvent>
#include <QResizeEvent>
#include <QSettings>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <optional>
#include <utility>

#include "Core/Ecs/Components/Sprite.h"  // core::Color
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Levels/LevelWriter.h"
#include "Core/Levels/TileMap.h"
#include "Core/Math/Vector2.h"
#include "HMI/Audio/AudioEngine.h"
#include "HMI/Audio/SoundTriggers.h"
#include "HMI/Editor/LevelFileOperations.h"
#include "HMI/Editor/LevelNameValidation.h"
#include "HMI/Editor/LinkGeometry.h"
#include "HMI/Editor/LinkGesture.h"
#include "HMI/Editor/TextureAssignGesture.h"
#include "HMI/Input/QtKeyMap.h"
// QRhi est une API privée de QtGui : l'en-tête vit sous rhi/, pas parmi les classes publiques.
#include <rhi/qrhi.h>

#include "HMI/Graphics/AssetContract.h"
#include "HMI/Graphics/AssetPaths.h"
#include "HMI/Graphics/BitmapFont.h"
#include "HMI/Graphics/DraftRenderer.h"
#include "HMI/Graphics/MissingTexture.h"
#include "HMI/Graphics/Parallax.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/Graphics/TextRenderer.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TextureCache.h"
#include "HMI/HmiLog.h"
#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Localization/Localization.h"
#include "HMI/Platform/ExecutableDirectory.h"

namespace hmi {

namespace {
// Cle de preference du mode de rendu, dans la meme portee QSettings que la langue et la
// disposition des docks (EX-IHM-011) : aucun nouveau mecanisme de persistance a inventer.
const char* const RENDER_MODE_SETTINGS_KEY = "render_mode";
}  // namespace

namespace {

[[nodiscard]] std::optional<hmi::MouseButton> mapQtMouseButton(Qt::MouseButton button) {
    switch (button) {
        case Qt::LeftButton:
            return hmi::MouseButton::Left;
        case Qt::RightButton:
            return hmi::MouseButton::Right;
        case Qt::MiddleButton:
            return hmi::MouseButton::Middle;
        default:
            return std::nullopt;
    }
}

[[nodiscard]] std::filesystem::path keybindingsPath() {
    return hmi::executableDirectory() / "Settings" / "keybindings.json";
}

}  // namespace

GameViewport::GameViewport(QWidget* parent)
    : QRhiWidget(parent),
      _gameBindings(hmi::GameKeyBindings::load(keybindingsPath())),
      _gamepadBindings(hmi::GamepadBindings::load(keybindingsPath())),
      _editorBindings(hmi::EditorKeyBindings::load(keybindingsPath())),
      _draft(core::LevelDraft::empty("Nouveau niveau", 24, 14)),
      _camera(1280, 720) {
    // Restaure le mode de rendu du dernier lancement (EX-IHM-011). Une preference absente, vide ou
    // corrompue retombe silencieusement sur le defaut (Texture) : perdre son mode d'affichage ne
    // doit jamais empecher de demarrer.
    _renderMode =
        renderModeFromName(QSettings()
                               .value(QString::fromLatin1(RENDER_MODE_SETTINGS_KEY), QString{})
                               .toString()
                               .toStdString());
}

GameViewport::~GameViewport() = default;

void GameViewport::setTool(hmi::EditorTool tool) {
    if (_tool == tool) {
        return;
    }
    _tool = tool;
    emit toolChanged(tool);
    if (_draftRenderer) {
        _draftRenderer->invalidate();  // le retour visuel des overrides depend de l'outil actif.
    }
}

// Choisit le mode de rendu et persiste le choix (EX-REN-046, EX-IHM-011).
void GameViewport::setRenderMode(RenderMode mode) {
    if (_renderMode == mode) {
        return;  // aucun changement : ni ecriture de preference, ni message d'etat inutile.
    }
    _renderMode = mode;
    // Ecriture a la bascule, jamais dans la boucle de rendu : QSettings bufferise, le cout est
    // marginal et le choix survit meme a une fermeture brutale.
    QSettings().setValue(QString::fromLatin1(RENDER_MODE_SETTINGS_KEY),
                         QString::fromLatin1(renderModeName(mode)));
    HMI_LOG_INFO(std::string{"Rendu : mode "} + renderModeName(mode) + ".");
    // Aucune invalidation du brouillon : la scene ECS est inchangee, seule la resolution de
    // l'apparence differe (LOT-41). La palette, elle, doit suivre pour rester fidele au canevas.
    emit renderModeChanged(mode);
}

int GameViewport::pixelWidth() const {
    return std::max(1, static_cast<int>(static_cast<qreal>(width()) * devicePixelRatio()));
}

int GameViewport::pixelHeight() const {
    return std::max(1, static_cast<int>(static_cast<qreal>(height()) * devicePixelRatio()));
}

void GameViewport::createResources() {
    HMI_LOG_INFO("Viewport : initialisation du rendu sur QRhi (" + std::to_string(pixelWidth()) +
                 "x" + std::to_string(pixelHeight()) + ").");

    // Le lot de sprites, l'atlas, la police, le cache de textures et le catalogue de skins sont
    // communs au jeu et a l'editeur : ils sont crees -- et surtout LIBERES dans le bon ordre --
    // par `hmi::SceneResources`, que l'element Qt Quick du jeu possede a l'identique (LOT-86).
    _scene.create(rhi(), _scene.context().updates);

    // Ce qui suit n'appartient qu'a l'editeur : le rendu de brouillon et le niveau de depart.
    _draftRenderer =
        std::make_unique<hmi::DraftRenderer>(_scene.sprites(), _scene.atlas(), _scene.textures());
    // Images des plans (LOT-69 TACHE-05) : a cote des niveaux, comme en jeu.
    _draftRenderer->setPlanesDirectory(hmi::executableDirectory() / "Levels" / "Plans");
    _draftRenderer->setSkins(&_scene.skins());

    // LOT-35 : ouvre un niveau de démonstration comme brouillon éditable. Échec récupérable : on
    // garde le brouillon vierge.
    const std::filesystem::path levelPath =
        hmi::executableDirectory() / "Levels" / "demo-deplacement.json";
    core::LevelLoadResult result = core::LevelLoader::loadFromFile(levelPath);
    if (result.ok()) {
        _draft = core::LevelDraft::fromLevel(*result.level);
        markDraftMutated();
    } else {
        HMI_LOG_WARNING("Editeur : echec du chargement du niveau de demo : " + result.error);
    }

    // MainWindow cable ses panneaux (palette, arbre de skins) a la construction, avant que la
    // fenetre ne soit exposee -- donc avant que ce catalogue ne soit charge. `resourcesReady`
    // leur donne l'occasion de se reconstruire une fois les skins reellement peuples, faute de
    // quoi l'editeur s'ouvre sans aucune texture jusqu'a la premiere bascule de mode.
    emit resourcesReady();
}

void GameViewport::updateEditCamera() {
    _camera.setViewportSize(pixelWidth(), pixelHeight());
    if (_manualCamera) {
        _camera.setZoom(_manualZoom);
        _camera.setCenter(_manualCenter);
        return;
    }
    const int levelWidth = _draft.tileMap().width();
    const int levelHeight = _draft.tileMap().height();
    _camera.setZoom(hmi::Camera2D::fitZoom(
        static_cast<float>(pixelWidth()), static_cast<float>(pixelHeight()),
        static_cast<float>(levelWidth), static_cast<float>(levelHeight), 0.92f));
    _camera.setCenter(core::Vector2{static_cast<float>(levelWidth) * 0.5f,
                                    static_cast<float>(levelHeight) * 0.5f});
}

core::Vector2 GameViewport::screenPosition(const QMouseEvent* event) const {
    const qreal ratio = devicePixelRatio();
    return core::Vector2{static_cast<float>(event->position().x() * ratio),
                         static_cast<float>(event->position().y() * ratio)};
}

float GameViewport::minManualZoom() const {
    return hmi::Camera2D::fitZoom(static_cast<float>(pixelWidth()),
                                  static_cast<float>(pixelHeight()),
                                  static_cast<float>(_draft.tileMap().width()),
                                  static_cast<float>(_draft.tileMap().height()), 0.92f);
}

float GameViewport::maxManualZoom() const {
    // Laisse au moins 4 cases visibles sur le plus petit axe de l'ecran : precision jugee
    // suffisante pour poser un bloc (ajustement post-livraison, LOT-15). Borne au minimum : un
    // niveau plus petit que 4 cases sur un axe rendrait sinon ce maximum inferieur au minimum
    // (std::clamp exige min <= max), verrouillant le zoom a l'ajustement automatique.
    constexpr float MINIMUM_VISIBLE_CELLS = 4.0f;
    const float smallerAxis = static_cast<float>((std::min)(pixelWidth(), pixelHeight()));
    const float rawMax = smallerAxis / (MINIMUM_VISIBLE_CELLS * hmi::Camera2D::PIXELS_PER_UNIT);
    return (std::max)(rawMax, minManualZoom());
}

std::optional<core::GridPosition> GameViewport::cellAt(const QMouseEvent* event) {
    updateEditCamera();  // s'assure que la conversion écran→monde utilise le cadrage courant.
    const qreal ratio = devicePixelRatio();
    const core::Vector2 world =
        _camera.screenToWorld(core::Vector2{static_cast<float>(event->position().x() * ratio),
                                            static_cast<float>(event->position().y() * ratio)});
    const int column = static_cast<int>(std::floor(world.x));
    const int row = static_cast<int>(std::floor(world.y));
    if (!_draft.tileMap().inBounds(column, row)) {
        return std::nullopt;
    }
    return core::GridPosition{column, row};
}

void GameViewport::paintAt(const QMouseEvent* event) {
    if (const std::optional<core::GridPosition> cell = cellAt(event)) {
        _draft.paintTile(cell->column, cell->row, _activeTile);
        _dirty = true;
        markDraftMutated();
    }
}

core::GridPosition GameViewport::clampedCell(const QMouseEvent* event) {
    updateEditCamera();
    const qreal ratio = devicePixelRatio();
    const core::Vector2 world =
        _camera.screenToWorld(core::Vector2{static_cast<float>(event->position().x() * ratio),
                                            static_cast<float>(event->position().y() * ratio)});
    const int width = _draft.tileMap().width();
    const int height = _draft.tileMap().height();
    return core::GridPosition{std::clamp(static_cast<int>(std::floor(world.x)), 0, width - 1),
                              std::clamp(static_cast<int>(std::floor(world.y)), 0, height - 1)};
}

void GameViewport::applyRectangle(core::GridPosition a, core::GridPosition b) {
    const int minColumn = std::min(a.column, b.column);
    const int maxColumn = std::max(a.column, b.column);
    const int minRow = std::min(a.row, b.row);
    const int maxRow = std::max(a.row, b.row);
    const std::vector<std::vector<core::TileType>> block(
        static_cast<std::size_t>(maxRow - minRow + 1),
        std::vector<core::TileType>(static_cast<std::size_t>(maxColumn - minColumn + 1),
                                    _activeTile));
    _draft.paintRegion(minColumn, minRow, block);  // un seul snapshot undo pour tout le rectangle
    _dirty = true;
    markDraftMutated();
}

// Ajoute une zone de camera dessinee a la main (outil CameraZone, EX-LVL-007, EX-EDIT-029) : meme
// decoupe min/max que applyRectangle ci-dessus, mais ajoutee au cadrage plutot que peinte sur la
// grille.
void GameViewport::addCameraZoneFromDrag(core::GridPosition a, core::GridPosition b) {
    const int minColumn = std::min(a.column, b.column);
    const int maxColumn = std::max(a.column, b.column);
    const int minRow = std::min(a.row, b.row);
    const int maxRow = std::max(a.row, b.row);
    _draft.addCameraZone(core::CameraZone{.x = minColumn,
                                          .y = minRow,
                                          .width = maxColumn - minColumn + 1,
                                          .height = maxRow - minRow + 1});
    _dirty = true;
    markDraftMutated();
}

void GameViewport::removeCameraZone(std::size_t index) {
    _draft.removeCameraZone(index);
    _dirty = true;
    markDraftMutated();
}

void GameViewport::copySelection() {
    if (!_selection) {
        return;
    }
    const core::GridPosition mn = _selection->first;
    const core::GridPosition mx = _selection->second;
    const core::TileMap& map = _draft.tileMap();
    _clipboard.clear();
    for (int row = mn.row; row <= mx.row; ++row) {
        std::vector<core::TileType> line;
        for (int column = mn.column; column <= mx.column; ++column) {
            line.push_back(map.tile(column, row));
        }
        _clipboard.push_back(std::move(line));
    }
    emit statusMessage(
        statusText("status.region_copied").arg(mx.column - mn.column + 1).arg(mx.row - mn.row + 1));
}

void GameViewport::pasteClipboard() {
    if (_clipboard.empty() || !_hoverCell) {
        return;
    }
    _draft.paintRegion(_hoverCell->column, _hoverCell->row, _clipboard);
    _dirty = true;
    markDraftMutated();
    emit statusMessage(statusText("status.region_pasted"));
}

std::optional<std::pair<core::GridPosition, core::GridPosition>> GameViewport::highlight() const {
    if (_dragging) {
        return std::make_pair(core::GridPosition{std::min(_dragStart.column, _dragCurrent.column),
                                                 std::min(_dragStart.row, _dragCurrent.row)},
                              core::GridPosition{std::max(_dragStart.column, _dragCurrent.column),
                                                 std::max(_dragStart.row, _dragCurrent.row)});
    }
    if (_highlightedOverride) {
        return std::make_pair(*_highlightedOverride, *_highlightedOverride);
    }
    return _selection;
}

void GameViewport::markDraftMutated() {
    if (_draftRenderer) {
        _draftRenderer->invalidate();
    }
    emit draftChanged();
}

void GameViewport::setHighlightedLink(
    std::optional<std::pair<core::GridPosition, core::GridPosition>> link) {
    _selectedLink = std::move(link);  // presentation seule : aucune mutation du brouillon.
    if (_draftRenderer) {
        _draftRenderer->invalidate();
    }
}

void GameViewport::unlinkMechanism(core::GridPosition targetPosition) {
    _draft.unlinkMechanism(targetPosition);
    _dirty = true;
    markDraftMutated();
}

void GameViewport::removeTextureOverride(core::GridPosition position) {
    _draft.removeTextureOverride(position);
    _dirty = true;
    markDraftMutated();
}

void GameViewport::setHighlightedTextureOverride(std::optional<core::GridPosition> position) {
    _highlightedOverride = position;  // presentation seule : aucune mutation du brouillon.
    if (_draftRenderer) {
        _draftRenderer->invalidate();
    }
}

void GameViewport::handleTextureAssignClick(const QMouseEvent* event, bool rightClick) {
    const std::optional<core::GridPosition> cell = cellAt(event);
    if (!cell) {
        return;
    }
    const core::TileType clickedType = _draft.tileMap().tile(cell->column, cell->row);
    std::optional<std::string> existingOverride;
    for (const core::TileTextureOverride& override : _draft.textureOverrides()) {
        if (override.position == *cell) {
            existingOverride = override.assetName;
            break;
        }
    }

    const hmi::TextureAssignDecision decision = hmi::resolveTextureAssignClick(
        *cell, clickedType, existingOverride, _activeTextureAsset, rightClick);
    switch (decision.action) {
        case hmi::TextureAssignAction::Ignore:
            // Cas silencieux le plus deroutant : cliquer pour assigner sans avoir choisi d'asset
            // dans la bibliotheque "Objets" ne fait rien -- le signaler, sinon l'action semble ne
            // pas marcher du tout.
            if (!rightClick && clickedType != core::TileType::Empty && !_activeTextureAsset) {
                emit statusMessage(statusText("status.texture_no_asset_selected"));
            }
            break;
        case hmi::TextureAssignAction::Assign:
            _draft.setTextureOverride(decision.cell, decision.assetName);
            _dirty = true;
            markDraftMutated();
            emit statusMessage(statusText("status.texture_assigned")
                                   .arg(QString::fromStdString(decision.assetName)));
            break;
        case hmi::TextureAssignAction::Remove:
            _draft.removeTextureOverride(decision.cell);
            _dirty = true;
            markDraftMutated();
            emit statusMessage(statusText("status.texture_removed"));
            break;
    }
}

core::Vector2 GameViewport::worldPositionAt(const QMouseEvent* event) {
    updateEditCamera();  // s'assure que la conversion écran→monde utilise le cadrage courant.
    const qreal ratio = devicePixelRatio();
    return _camera.screenToWorld(core::Vector2{static_cast<float>(event->position().x() * ratio),
                                               static_cast<float>(event->position().y() * ratio)});
}

void GameViewport::selectPlane(std::optional<std::size_t> index) {
    if (index && *index >= _draft.planes().size()) {
        return;
    }
    _selectedPlane = index;
    emit planeSelectionChanged(_selectedPlane);
}

void GameViewport::addPlane(core::Plane plane) {
    _draft.addPlane(std::move(plane));
    _selectedPlane = _draft.planes().size() - 1;  // le nouveau plan reste selectionne
    markDraftMutated();
    emit planeSelectionChanged(_selectedPlane);
}

void GameViewport::removePlane(std::size_t index) {
    if (index >= _draft.planes().size()) {
        return;
    }
    _draft.removePlane(index);
    // La selection suit le MEME plan quand c'est possible : retirer celui du dessus ne doit pas
    // faire glisser la selection sur son voisin sans qu'on l'ait demande.
    if (_selectedPlane == index) {
        _selectedPlane.reset();
    } else if (_selectedPlane && *_selectedPlane > index) {
        *_selectedPlane -= 1;
    }
    markDraftMutated();
    emit planeSelectionChanged(_selectedPlane);
}

void GameViewport::movePlane(std::size_t index, bool forward) {
    const std::optional<std::size_t> moved =
        forward ? _draft.movePlaneForward(index) : _draft.movePlaneBackward(index);
    if (!moved) {
        return;
    }
    // Le rang change : la selection suit le plan deplace, pas l'ancien rang qui designe desormais
    // son voisin.
    _selectedPlane = moved;
    markDraftMutated();
    emit planeSelectionChanged(_selectedPlane);
}

void GameViewport::setPlaneDepth(std::size_t index, core::PlaneDepth depth) {
    if (_draft.setPlaneDepth(index, depth)) {
        markDraftMutated();
    }
}

void GameViewport::setPlaneDensity(std::size_t index, int pixelsPerUnit) {
    if (_draft.setPlaneDensity(index, pixelsPerUnit)) {
        markDraftMutated();
    }
}

void GameViewport::setPlaneParallax(std::size_t index, float parallaxX, float parallaxY) {
    if (_draft.setPlaneParallax(index, parallaxX, parallaxY)) {
        markDraftMutated();
    }
}

void GameViewport::setPlaneOpacity(std::size_t index, float opacity) {
    if (_draft.setPlaneOpacity(index, opacity)) {
        markDraftMutated();
    }
}

void GameViewport::setLevelParallaxEnabled(bool enabled) {
    _draft.setParallaxEnabled(enabled);
    markDraftMutated();
}

void GameViewport::setPlaneVisible(std::size_t index, bool visible) {
    // Aide d'edition : aucun pas d'annulation, aucune ecriture dans le brouillon.
    _planeVisibility.setVisible(index, visible);
    if (_draftRenderer) {
        _draftRenderer->invalidate();
    }
}

void GameViewport::setPlaneIsolated(std::size_t index, bool isolate) {
    if (isolate) {
        _planeVisibility.isolate(index);
    } else {
        _planeVisibility.showAll();
    }
    if (_draftRenderer) {
        _draftRenderer->invalidate();
    }
}

void GameViewport::setLevelBackground(std::optional<std::string> background) {
    _draft.setBackground(std::move(background));
    _dirty = true;
    markDraftMutated();
}

void GameViewport::setLevelSkinSet(std::optional<std::string> skinSet) {
    _draft.setSkinSet(std::move(skinSet));
    _dirty = true;
    markDraftMutated();
}

void GameViewport::setLevelCameraFraming(core::CameraFramingConfig cameraFraming) {
    _draft.setCameraFraming(cameraFraming);
    _dirty = true;
    markDraftMutated();
}
void GameViewport::setSkinSet(const std::string& setName) {
    // Le catalogue est deja a jour (le panneau agit dessus directement) : il n'y a que le jeu
    // courant a propager, et l'image suivante montrera le resultat. Aucune scene a reconstruire.
    if (_draftRenderer) {
        _draftRenderer->setSkins(&_scene.skins(), setName);
    }
    if (_session) {
        _session->setSkins(&_scene.skins(), setName);
    }
    _skinSet = setName;
}

void GameViewport::reloadAssets() {
    // Invalidation avant relecture du catalogue : un asset renomme doit disparaitre du cache
    // AVANT que la nouvelle assignation ne pointe vers son remplacant.
    if (_scene.created()) {
        _scene.textures().invalidateAll();
    }
    // skins.json a pu changer hors de l'application (renommage/suppression d'un asset) : on le
    // relit entierement, comme au demarrage. Le catalogue garde son adresse (membre de la grappe de
    // ressources) : DraftRenderer et
    // la session en cours, qui n'en detiennent qu'un pointeur, voient le nouveau contenu sans
    // etre re-cables (LOT-42).
    hmi::SkinCatalogResult skins =
        hmi::SkinCatalog::loadFromFile(hmi::executableDirectory() / "Assets" / "skins.json");
    if (skins.ok()) {
        _scene.skins() = std::move(*skins.catalog);
    }
    // Aucune reconstruction de scene necessaire : l'apparence est resolue a la composition de
    // chaque image (hmi::DraftRenderer), donc l'image suivante montre deja le resultat (LOT-42).
    HMI_LOG_INFO("Viewport : rechargement a chaud des assets.");
}

void GameViewport::invalidateAsset(const std::string& fileName) {
    if (_scene.created()) {
        _scene.textures().invalidate(fileName);
    }
}

bool GameViewport::linkExists(core::GridPosition switchPosition,
                              core::GridPosition targetPosition) const {
    for (const core::Mechanism& mechanism : _draft.mechanisms()) {
        if (mechanism.switchPosition == switchPosition &&
            mechanism.doorPosition == targetPosition) {
            return true;
        }
    }
    return false;
}

void GameViewport::handleLinkClick(const QMouseEvent* event) {
    const std::optional<core::GridPosition> cell = cellAt(event);
    if (!cell) {
        return;
    }
    const core::TileType clickedType = _draft.tileMap().tile(cell->column, cell->row);

    std::optional<hmi::PendingLink> pending;
    if (_pendingLink) {
        pending = hmi::PendingLink{*_pendingLink,
                                   _draft.tileMap().tile(_pendingLink->column, _pendingLink->row)};
    }

    // N'a de sens que si l'attente est toujours un declencheur/une cible valide : resolu de la
    // meme facon que resolveLinkClick determinera lequel des deux est le declencheur.
    bool alreadyLinked = false;
    if (pending &&
        (hmi::isTriggerTile(pending->tileType) || hmi::isLinkTargetTile(pending->tileType))) {
        const bool pendingIsTrigger = hmi::isTriggerTile(pending->tileType);
        const core::GridPosition switchPosition = pendingIsTrigger ? pending->cell : *cell;
        const core::GridPosition targetPosition = pendingIsTrigger ? *cell : pending->cell;
        alreadyLinked = linkExists(switchPosition, targetPosition);
    }

    const hmi::LinkGestureDecision decision =
        hmi::resolveLinkClick(pending, *cell, clickedType, alreadyLinked);
    switch (decision.action) {
        case hmi::LinkGestureAction::Ignore:
            break;
        case hmi::LinkGestureAction::SetPending:
        case hmi::LinkGestureAction::ReplacePending:
            _pendingLink = decision.cell;
            if (_draftRenderer) {
                _draftRenderer->invalidate();  // affiche la nouvelle case en attente.
            }
            break;
        case hmi::LinkGestureAction::Link:
            _draft.linkMechanism(decision.switchPosition, decision.targetPosition);
            _pendingLink.reset();
            _dirty = true;
            markDraftMutated();
            break;
        case hmi::LinkGestureAction::Unlink:
            _draft.unlinkMechanism(decision.targetPosition);
            _pendingLink.reset();
            _dirty = true;
            markDraftMutated();
            break;
    }
}

void GameViewport::tick(float elapsedSeconds) {
    // Cadence de rendu (LOT-62 TACHE-02) : seulement quand le compteur est actif -- rien n'est
    // calculé au-delà du test, coût nul quand il est éteint.
    if (_diagnosticsEnabled) {
        _frameRateAverage.addSample(elapsedSeconds);
    }

    _gamepad.poll(_input);

    // Ouverture de la pause à la manette (bouton B, `EX-CTRL-012`), même geste que « retour »
    // ailleurs dans l'IHM -- seulement en partie réelle, jamais en essai depuis l'éditeur
    // (`LOT-59` TACHE-02).
    if (_gameMode && !_paused && _input.gamepadButtonPressed(GamepadButton::B)) {
        emit pauseRequested();
    }

    // Pause (LOT-59 TACHE-02, EX-GP-041) : l'accumulateur n'est PAS alimenté (pas un dt nul, un
    // appel simplement absent) -- aucun pas n'est consommé tant que la pause dure. Le rendu, lui,
    // continue ci-dessous : la scène reste dessinée derrière l'écran de pause.
    if (!_paused) {
        const int steps = _timestep.advance(elapsedSeconds);
        _lastSimulationSteps = steps;  // pas consommés à cette image (LOT-62 TACHE-02).
        const float fixedDelta = _timestep.fixedDeltaSeconds();
        for (int step = 0; step < steps; ++step) {
            core::LevelOutcome outcome = core::LevelOutcome::Playing;
            if (_session) {
                outcome = _session->update(_input, fixedDelta);
            }
            // Bilan du tableau (LOT-68) : compte au PAS, pour la meme raison que les sons
            // ci-dessous -- une mesure a l'image dependrait de la cadence de rendu.
            if (_session && _gameMode) {}
            // Sons de jeu (LOT-60 TACHE-03) : un evenement par pas, jamais par image de rendu --
            // lastStepEvents() reflete exactement CE pas, celui qui vient de s'executer.
            if (_session && _audioEngine) {
                for (const GameEvent gameEvent : _session->lastStepEvents()) {
                    if (const std::optional<std::string> soundId = soundForEvent(gameEvent)) {
                        _audioEngine->play(*soundId);
                    }
                }
            }
            if (_session && outcome == core::LevelOutcome::Won) {
                _input.beginFrame();
                if (_gameMode) {
                    // La sortie de la carte ne TERMINE plus rien (LOT-67) : il n'y a ni tableau
                    // suivant, ni ecran de fin. Elle ramene au menu, faute de destination -- et
                    // c'est un provisoire assume : la sortie redeviendra une TRANSITION, vers la
                    // carte que le graphe du LOT-09 designera. Y laisser un ecran de fin de
                    // sequence aurait ete plus spectaculaire et plus faux.
                    HMI_LOG_INFO(
                        "Jeu : sortie atteinte ; sans graphe de cartes (LOT-09), retour "
                        "au menu.");
                    _gameMode = false;
                    _session.reset();
                    emit exitToMenuRequested();
                } else {
                    stopPlaytest();  // essai éditeur : retour à l'édition
                }
                break;
            }
            _input.beginFrame();
        }
    } else {
        // Consomme les fronts (ex. bouton B tenu en ouvrant la pause) sans avancer la simulation
        // -- sinon un appui encore maintenu à la reprise ferait osciller entrée/sortie de pause
        // (piège documenté par TACHE-02).
        _input.beginFrame();
    }
}

// Crée (ou recrée) les ressources graphiques quand QRhiWidget fournit son interface de rendu.
void GameViewport::initialize(QRhiCommandBuffer* commandBuffer) {
    if (_scene.context().rhi == rhi()) {
        return;  // même interface : les ressources déjà créées restent valides.
    }
    // initialize() s'execute APRES startGame() dans le cas NOMINAL : le viewport n'est peint
    // qu'une fois affiche, donc la premiere image arrive alors qu'une session est deja demandee.
    // Or une session ne survit pas a la liberation ci-dessous -- elle tient l'atlas, la police et
    // le lot de sprites. D'ou la regle : memoriser ce qu'il faudra REMONTER avant de liberer, et
    // le remonter apres. Sans cela, l'etat affiche serait celui du brouillon d'edition et non
    // celui de la session demandee.
    const bool restorePlaytest = _session.has_value() && !_gameMode;
    // Changement d'interface (première image, ou widget passé sous une autre fenêtre de haut
    // niveau) : tout ce qui tient une texture est caduc. Ordre de libération : la session et les
    // rendus AVANT les textures qu'ils référencent.
    releaseResources();
    _scene.context().rhi = rhi();
    _scene.context().updates = _scene.context().rhi->nextResourceUpdateBatch();
    createResources();
    // Téléversements accumulés par la création des textures : soumis ici, hors de toute passe.
    commandBuffer->resourceUpdate(_scene.context().updates);
    _scene.context().updates = nullptr;
    // Remontage de la session, une fois les ressources disponibles. Le tableau repart de son
    // debut : l'avancee dans la salle n'est pas rejouable, et la reprendre a mi-course exigerait
    // de serialiser toute la simulation pour un cas qui ne se produit qu'a la premiere image ou
    // au changement d'interface de rendu.
    if (_gameMode) {
        loadGameMap();
    } else if (restorePlaytest) {
        startPlaytest();
    }
    _previousFrame = Clock::now();
}

// Libère les ressources graphiques quand QRhiWidget défait son interface de rendu.
void GameViewport::releaseResources() {
    // Ce qui tient une texture meurt avant elle. La session et le rendu de brouillon d'abord --
    // ils appartiennent a l'editeur ; `SceneResources::release` fixe ensuite l'ordre interne de
    // la grappe, que plus aucun appelant n'a a se rappeler.
    _session.reset();
    _draftRenderer.reset();
    _scene.release();
}

// Dessine une image : avance la simulation, compose la scène, puis la soumet.
void GameViewport::render(QRhiCommandBuffer* commandBuffer) {
    if (!_scene.created()) {
        return;  // initialize() n'a pas encore pu créer les ressources.
    }
    const Clock::time_point now = Clock::now();
    const float elapsedSeconds = std::chrono::duration<float>(now - _previousFrame).count();
    _previousFrame = now;

    // Lot de mises à jour de CETTE image : les textures chargées paresseusement pendant la
    // composition y déposent leurs pixels, et `submit` le soumet avant d'ouvrir sa passe.
    _scene.context().updates = _scene.context().rhi->nextResourceUpdateBatch();
    tick(elapsedSeconds);
    renderFrame(commandBuffer, elapsedSeconds);
    _scene.context().updates = nullptr;

    // Animation continue : la prochaine image est demandée dès celle-ci terminée, comme le faisait
    // `requestUpdate()` du temps de la fenêtre native.
    update();
}

void GameViewport::renderFrame(QRhiCommandBuffer* commandBuffer, float deltaSeconds) {
    _scene.sprites().beginFrame();
    // Fond derive des jetons (LOT-56) : portee variable (chassis d'edition, suivant le theme actif
    // de l'editeur, TACHE-06) en edition, portee invariante (identite du jeu) en jeu/essai -- seule
    // surface qui appartient tour a tour aux deux portees (hmi::viewportClearColor).
    const hmi::DesignColor clearColor =
        hmi::viewportClearColor(/*editorMode=*/!_session, hmi::currentEditorTokens());
    const float clear[4] = {static_cast<float>(clearColor.r) / 255.0f,
                            static_cast<float>(clearColor.g) / 255.0f,
                            static_cast<float>(clearColor.b) / 255.0f, 1.0f};
    if (_session) {
        _session->render(pixelWidth(), pixelHeight(), _renderMode, _timestep.interpolationAlpha());
        renderDiagnosticsOverlay(pixelWidth(), pixelHeight());
    } else {
        updateEditCamera();
        // Zoom courant pour la barre d'etat (LOT-57 TACHE-01) : pas de signal natif sur Camera2D,
        // comparaison au dernier zoom notifie plutot qu'une emission a chaque image.
        if (_camera.zoom() != _lastEmittedZoom) {
            _lastEmittedZoom = _camera.zoom();
            emit zoomChanged(_lastEmittedZoom);
        }
        hmi::LinkOverlayState linkOverlay;
        linkOverlay.hoveredCell = _hoverCell;
        linkOverlay.pendingLink = _pendingLink;
        linkOverlay.selectedLink = _selectedLink;
        // Retour visuel des cases habillees (LOT-45) : seulement quand l'outil dedie est actif,
        // sinon l'auteur ne sait pas ce qui est deja habille sans que ca encombre les autres
        // outils.
        const bool showTextureOverrides = _tool == hmi::EditorTool::TextureAssign;
        _draftRenderer->render(_draft, _camera, _showGrid, highlight(), linkOverlay, _renderMode,
                               showTextureOverrides, deltaSeconds, _layerVisibility,
                               _planeVisibility);
    }
    // Téléversement unique puis passe unique : c'est ici, et nulle part ailleurs, que le GPU voit
    // l'image (cf. `hmi::SpriteBatch`, enregistrement en deux phases).
    _scene.sprites().submit(commandBuffer, renderTarget(), _scene.context().updates, clear);
}

void GameViewport::renderDiagnosticsOverlay(int viewportWidth, int viewportHeight) {
    if (!_diagnosticsEnabled || !_session || _loc == nullptr) {
        return;  // eteint, hors session, ou localisation pas encore chargee (EX-NFR-040).
    }

    hmi::DiagnosticsMeasurements measurements;
    measurements.framesPerSecond = _frameRateAverage.framesPerSecond();
    measurements.sceneStatistics = _session->renderStatistics();
    measurements.simulationSteps = _lastSimulationSteps;
    const std::vector<std::string> lines = hmi::composeDiagnosticsHudLines(measurements, *_loc);

    // Meme habillage (ombre + texte) que hmi::GameSession::renderHud, coin haut-DROIT (LOT-62
    // TACHE-02) pour ne jamais recouvrir le HUD de jeu (budgets), ancre coin haut-gauche.
    constexpr float MARGIN = 8.0f;
    constexpr float SCALE = 1.0f;
    constexpr float LINE_SPACING = 2.0f;
    constexpr core::Color SHADOW_COLOR{0.0f, 0.0f, 0.0f, 0.75f};
    constexpr core::Color TEXT_COLOR{1.0f, 1.0f, 1.0f, 1.0f};
    constexpr hmi::TextAnchor ANCHOR{hmi::TextHorizontalAnchor::Right,
                                     hmi::TextVerticalAnchor::Top};

    _diagnosticsScene.clear();
    const float x = static_cast<float>(viewportWidth) - MARGIN;
    float lineY = MARGIN;
    for (const std::string& line : lines) {
        hmi::composeText(_diagnosticsScene, _scene.font(), line, x + 1.0f, lineY + 1.0f, SCALE,
                         SHADOW_COLOR, ANCHOR);
        hmi::composeText(_diagnosticsScene, _scene.font(), line, x, lineY, SCALE, TEXT_COLOR,
                         ANCHOR);
        lineY += static_cast<float>(_scene.font().metrics().lineHeight) * SCALE + LINE_SPACING;
    }
    _diagnosticsScene.sort();
    hmi::submitComposedScene(_scene.sprites(),
                             hmi::screenProjectionMatrix(viewportWidth, viewportHeight),
                             _diagnosticsScene);
}

bool GameViewport::event(QEvent* event) {
    switch (event->type()) {
        case QEvent::FocusOut:
            _input.releaseAll();
            break;
        case QEvent::Leave:
            // Le curseur quitte le viewport : la case survolee n'a plus de sens (LOT-57 TACHE-01,
            // barre d'etat).
            if (_hoverCell) {
                _hoverCell.reset();
                emit hoveredCellChanged(std::nullopt);
            }
            break;
        default:
            break;
    }
    return QRhiWidget::event(event);
}

void GameViewport::keyPressEvent(QKeyEvent* event) {
    // Mode jeu/essai : Échap ouvre la pause (partie réelle, LOT-59 TACHE-02) ou sort (essai) ;
    // les autres touches alimentent le jeu -- jamais pendant une pause, où le focus clavier
    // revient à l'écran de pause (MainWindow::applyScreenDressing), pas au viewport.
    if (_session) {
        if (event->key() == Qt::Key_Escape) {
            if (_paused) {
                return;  // ne devrait pas arriver (focus sur l'écran de pause) ; robustesse
            }
            if (_gameMode) {
                emit pauseRequested();
            } else {
                stopPlaytest();
            }
            return;
        }
        if (_paused) {
            return;
        }
        // Compteur de diagnostic (F9, LOT-62 TACHE-02) : touche dédiée non remappable, même statut
        // que F8 (bascule de rendu) -- jamais transmise au jeu comme entrée (return immédiat).
        if (!event->isAutoRepeat() && event->key() == Qt::Key_F9) {
            toggleDiagnosticsOverlay();
            return;
        }
        if (!event->isAutoRepeat()) {
            if (const std::optional<hmi::Key> key = qtKeyToHmiKey(event->key())) {
                _input.onKeyDown(*key);
            }
        }
        return;
    }

    // Mode édition : raccourcis (annuler/refaire, enregistrer, essai).
    if (event->key() == Qt::Key_Escape && _pendingLink) {
        // Annule la liaison en cours (outil Lien) : efface l'attente sans toucher au brouillon.
        _pendingLink.reset();
        if (_draftRenderer) {
            _draftRenderer->invalidate();
        }
        return;
    }
    // Annuler/refaire/enregistrer/essai/grille/recadrer/mode de rendu/copier/coller sont désormais
    // des actions Qt uniques (barre d'outils/menu, `hmi::EditorActions`, LOT-56 TACHE-04 et
    // LOT-57 TACHE-04) : plus de second traitement ici, sous peine de double déclenchement au même
    // appui de touche (annuler deux pas d'un coup, coller deux fois).
    if (event->isAutoRepeat()) {
        return;
    }
    if (const std::optional<hmi::Key> key = qtKeyToHmiKey(event->key());
        key && *key == _editorBindings.key(hmi::EditorAction::TextureAssignTool)) {
        setTool(hmi::EditorTool::TextureAssign);
        return;
    }
    if (const std::optional<hmi::Key> key = qtKeyToHmiKey(event->key())) {
        _input.onKeyDown(*key);
    }
}

void GameViewport::save() {
    const core::LevelLoadResult validated = _draft.toLevel();
    if (!validated.ok()) {
        HMI_LOG_WARNING("Editeur : enregistrement refuse (brouillon invalide) : " +
                        validated.error);
        emit statusMessage(
            statusText("status.save_failed").arg(QString::fromStdString(validated.error)));
        return;
    }
    const std::filesystem::path path =
        hmi::executableDirectory() / "Levels" / (_draft.name() + ".json");
    if (core::LevelWriter::saveToFile(*validated.level, path)) {
        _dirty = false;
        HMI_LOG_INFO("Editeur : niveau enregistre : " + path.string());
        emit statusMessage(
            statusText("status.level_saved").arg(QString::fromStdString(path.filename().string())));
    } else {
        HMI_LOG_ERROR("Editeur : echec d'ecriture du niveau : " + path.string());
        emit statusMessage(statusText("status.write_failed"));
    }
}

bool GameViewport::renameOpenLevel(const std::string& newName) {
    if (!hmi::isValidLevelName(newName)) {
        emit statusMessage(statusText("status.rename_failed"));
        return false;
    }
    const std::string trimmed = hmi::trimLevelName(newName);
    if (trimmed == _draft.name()) {
        return true;  // rien a faire.
    }
    const std::filesystem::path levelsDir = hmi::executableDirectory() / "Levels";
    const std::filesystem::path oldPath = levelsDir / (_draft.name() + ".json");
    if (std::filesystem::exists(oldPath)) {
        // Niveau deja enregistre au moins une fois : renomme le fichier sur disque, meme chemin que
        // LevelBrowserPanel::onRename -- le brouillon en memoire est mis a jour separement
        // ci-dessous (writeRenamed opere sur une copie chargee depuis le disque, pas sur _draft).
        const hmi::LevelFileOperations ops(levelsDir);
        const hmi::FileOperationResult result = ops.rename(oldPath, trimmed);
        if (!result.ok()) {
            HMI_LOG_WARNING("Editeur : renommage refuse : " + result.error);
            emit statusMessage(statusText("status.rename_failed_reason")
                                   .arg(QString::fromStdString(result.error)));
            return false;
        }
    }
    _draft.setName(trimmed);
    markDraftMutated();
    HMI_LOG_INFO("Editeur : niveau renomme en « " + trimmed + " ».");
    emit statusMessage(statusText("status.level_renamed").arg(QString::fromStdString(trimmed)));
    return true;
}

void GameViewport::openLevel(const std::filesystem::path& path) {
    core::LevelLoadResult loaded = core::LevelLoader::loadFromFile(path);
    if (!loaded.ok()) {
        HMI_LOG_WARNING("Editeur : ouverture impossible (" + path.string() + ") : " + loaded.error);
        emit statusMessage(
            statusText("status.open_failed").arg(QString::fromStdString(loaded.error)));
        return;
    }
    stopPlaytest();  // sort d'un éventuel essai en cours
    _draft = core::LevelDraft::fromLevel(*loaded.level);
    _dirty = false;
    _pendingLink.reset();
    _selectedLink.reset();
    _manualCamera =
        false;  // cadrage automatique sur le niveau ouvert (LOT-15), pas l'ancien pan/zoom.
    markDraftMutated();
    HMI_LOG_INFO("Editeur : niveau ouvert : " + path.string());
    emit statusMessage(
        statusText("status.level_opened").arg(QString::fromStdString(path.filename().string())));
}

void GameViewport::startPlaytest() {
    if (_session) {
        return;  // déjà en essai
    }
    const core::LevelLoadResult validated = _draft.toLevel();
    if (!validated.ok()) {
        HMI_LOG_WARNING("Editeur : essai refuse (brouillon invalide) : " + validated.error);
        emit statusMessage(
            statusText("status.playtest_failed").arg(QString::fromStdString(validated.error)));
        return;
    }
    if (!_scene.created()) {
        emit statusMessage(statusText("status.playtest_failed"));
        return;  // rendu pas encore initialise (aucune image dessinee) : rien a essayer.
    }
    _session.emplace(_scene.sprites(), _scene.atlas(), _scene.textures(), pixelWidth(),
                     pixelHeight(), std::move(*validated.level), _gameBindings, _gamepadBindings,
                     _scene.font(), _loc);
    // Meme habillage qu'en edition : l'essai doit montrer exactement le canevas de l'editeur.
    _session->setSkins(&_scene.skins(), _skinSet);
    HMI_LOG_INFO("Editeur : essai immediat demarre.");
    emit statusMessage(statusText("status.playtesting"));
}

void GameViewport::undo() {
    if (_draft.undo()) {
        _dirty = true;
        markDraftMutated();
    }
}

void GameViewport::redo() {
    if (_draft.redo()) {
        _dirty = true;
        markDraftMutated();
    }
}

void GameViewport::toggleGrid() noexcept {
    _showGrid = !_showGrid;
}

void GameViewport::resetCamera() noexcept {
    _manualCamera = false;
}

void GameViewport::toggleRenderMode() {
    setRenderMode(_renderMode == RenderMode::Physique ? RenderMode::Texture : RenderMode::Physique);
}

void GameViewport::setDiagnosticsOverlayEnabled(bool enabled) noexcept {
    if (_diagnosticsEnabled == enabled) {
        return;
    }
    toggleDiagnosticsOverlay();
}

void GameViewport::toggleDiagnosticsOverlay() noexcept {
    _diagnosticsEnabled = !_diagnosticsEnabled;
    _frameRateAverage.reset();
}

void GameViewport::stopPlaytest() {
    if (!_session) {
        return;
    }
    _session.reset();
    if (_draftRenderer) {
        _draftRenderer->invalidate();  // ré-affiche le brouillon (intact).
    }
    emit statusMessage(statusText("status.back_to_edit"));
}

void GameViewport::setVSync(bool enabled) noexcept {
    _vsync = enabled;
    // Depuis le portage QRhi (LOT-69 TACHE-02), la presentation appartient au compositeur de Qt :
    // le reglage est conserve et journalise, mais aucune swap chain du projet ne l'applique plus.
    HMI_LOG_INFO(std::string("Options : V-Sync ") + (enabled ? "activee." : "desactivee.") +
                 " La presentation est calee par Qt depuis le portage QRhi.");
}

void GameViewport::pauseSimulation() noexcept {
    _paused = true;
}

void GameViewport::resumeSimulation() {
    _paused = false;
    // Réarme l'horloge de référence sur l'instant courant : sans ça, le prochain tick() verrait
    // un elapsedSeconds egal a toute la duree de la pause, et FixedTimestep::advance rendrait
    // d'un coup tous les pas « manques » (le personnage traverserait le niveau, TACHE-02).
    _previousFrame = Clock::now();
}

void GameViewport::quitGame() noexcept {
    _gameMode = false;
    _paused = false;
    _session.reset();
}

void GameViewport::startGame(std::filesystem::path map) {
    _gameMap = std::move(map);
    _gameMode = true;
    HMI_LOG_INFO("Jeu : demarrage sur la carte " + _gameMap.filename().string() + ".");
    loadGameMap();
}

void GameViewport::loadGameMap() {
    core::LevelLoadResult loaded = core::LevelLoader::loadFromFile(_gameMap);
    if (!loaded.ok()) {
        // Plus de « passer a la carte suivante » : il n'y en a pas. Une carte illisible ramene au
        // menu en le disant (EX-NFR-040), plutot que d'ouvrir un ecran de jeu sans rien a jouer.
        HMI_LOG_WARNING("Jeu : carte illisible (" + _gameMap.string() + ") : " + loaded.error);
        _gameMode = false;
        _session.reset();
        emit exitToMenuRequested();
        return;
    }
    HMI_LOG_INFO("Jeu : carte chargee : " + _gameMap.filename().string());
    if (!_scene.created()) {
        // Les ressources de rendu n'existent pas encore : QRhiWidget ne les cree qu'a la premiere
        // image, et le viewport n'est peint qu'une fois affiche -- donc APRES ce chemin quand on
        // lance une partie depuis le menu. La carte reste designee (`_gameMode`, `_gameMap`) et
        // `initialize()` remonte la session des qu'il le peut. Emplacer ici lierait une reference
        // a un lot de sprites nul.
        return;
    }
    _session.emplace(_scene.sprites(), _scene.atlas(), _scene.textures(), pixelWidth(),
                     pixelHeight(), std::move(*loaded.level), _gameBindings, _gamepadBindings,
                     _scene.font(), _loc);
    _session->setSkins(&_scene.skins(), _skinSet);
}

void GameViewport::resizeLevel(int width, int height) {
    _draft.resize(width, height);
    _dirty = true;
    markDraftMutated();
    emit statusMessage(statusText("status.level_resized").arg(width).arg(height));
}

bool GameViewport::wouldResizeDrop(int width, int height) const {
    return _draft.wouldResizeDropContent(width, height);
}

int GameViewport::levelWidth() const {
    return _draft.tileMap().width();
}

int GameViewport::levelHeight() const {
    return _draft.tileMap().height();
}

QString GameViewport::statusText(const char* key) const {
    return _loc != nullptr ? QString::fromStdString(_loc->text(key)) : QString::fromLatin1(key);
}

void GameViewport::keyReleaseEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) {
        return;
    }
    if (const std::optional<hmi::Key> key = qtKeyToHmiKey(event->key())) {
        _input.onKeyUp(*key);
    }
}

void GameViewport::updateMousePosition(const QMouseEvent* event) {
    const qreal ratio = devicePixelRatio();
    _input.onMouseMove(static_cast<int>(event->position().x() * ratio),
                       static_cast<int>(event->position().y() * ratio));
}

void GameViewport::mousePressEvent(QMouseEvent* event) {
    updateMousePosition(event);
    if (const std::optional<hmi::MouseButton> button = mapQtMouseButton(event->button())) {
        _input.onMouseButtonDown(*button);
    }
    if (_session) {
        return;
    }
    if (event->button() == Qt::RightButton) {
        // Le bouton droit sert au pan (glisser) ET, pour l'outil « Texture par instance »
        // (LOT-45), au retrait explicite (clic sans glisser) -- la decision entre les deux est
        // prise a la relache, une fois qu'on sait si un glisser a eu lieu.
        _rightDragging = true;
        _cameraPanned = false;
        _rightDragLastScreen = screenPosition(event);
        return;
    }
    if (event->button() != Qt::LeftButton) {
        return;
    }
    // Mode édition : dispatch selon l'outil actif.
    switch (_tool) {
        case hmi::EditorTool::Paint:
            _painting = true;
            paintAt(event);
            break;
        case hmi::EditorTool::Rectangle:
        case hmi::EditorTool::Selection:
        case hmi::EditorTool::CameraZone:
            _dragging = true;
            _dragStart = clampedCell(event);
            _dragCurrent = _dragStart;
            break;
        case hmi::EditorTool::Link:
            handleLinkClick(event);
            break;
        case hmi::EditorTool::TextureAssign:
            handleTextureAssignClick(event, /*rightClick=*/false);
            break;
    }
}

void GameViewport::mouseReleaseEvent(QMouseEvent* event) {
    updateMousePosition(event);
    if (const std::optional<hmi::MouseButton> button = mapQtMouseButton(event->button())) {
        _input.onMouseButtonUp(*button);
    }
    if (event->button() == Qt::RightButton) {
        // Glisser droit sans mouvement significatif = un clic : l'outil « Texture par instance »
        // (LOT-45) y retire l'override de la case, les autres outils l'ignorent. Un glisser (pan)
        // ne declenche jamais d'action d'edition.
        if (_rightDragging && !_cameraPanned && _tool == hmi::EditorTool::TextureAssign) {
            handleTextureAssignClick(event, /*rightClick=*/true);
        }
        _rightDragging = false;
        _cameraPanned = false;
        return;
    }
    if (event->button() != Qt::LeftButton) {
        return;
    }
    if (_dragging) {
        _dragCurrent = clampedCell(event);
        if (_tool == hmi::EditorTool::Rectangle) {
            applyRectangle(_dragStart, _dragCurrent);
        } else if (_tool == hmi::EditorTool::Selection) {
            _selection =
                std::make_pair(core::GridPosition{std::min(_dragStart.column, _dragCurrent.column),
                                                  std::min(_dragStart.row, _dragCurrent.row)},
                               core::GridPosition{std::max(_dragStart.column, _dragCurrent.column),
                                                  std::max(_dragStart.row, _dragCurrent.row)});
        } else if (_tool == hmi::EditorTool::CameraZone) {
            addCameraZoneFromDrag(_dragStart, _dragCurrent);
        }
        _dragging = false;
    }
    _painting = false;
}

void GameViewport::mouseMoveEvent(QMouseEvent* event) {
    updateMousePosition(event);
    if (_session) {
        return;
    }
    if (_rightDragging) {
        constexpr float PAN_THRESHOLD_PIXELS = 3.0f;  // au-dela : un glisser, pas un clic.
        const core::Vector2 current = screenPosition(event);
        const core::Vector2 delta = current - _rightDragLastScreen;
        if (std::abs(delta.x) > PAN_THRESHOLD_PIXELS || std::abs(delta.y) > PAN_THRESHOLD_PIXELS) {
            _cameraPanned = true;
        }
        if (!_manualCamera) {
            // Premier glisser : demarre le cadrage manuel a partir du cadrage automatique
            // courant, pour ne pas faire sauter la vue.
            updateEditCamera();
            _manualZoom = _camera.zoom();
            _manualCenter = _camera.center();
            _manualCamera = true;
        }
        const float scale = hmi::Camera2D::PIXELS_PER_UNIT * _manualZoom;
        _manualCenter.x -= delta.x / scale;
        _manualCenter.y -= delta.y / scale;
        _rightDragLastScreen = current;
    }
    const std::optional<core::GridPosition> cell = cellAt(event);  // cible du collage (Ctrl+V)
    if (cell != _hoverCell) {
        _hoverCell = cell;
        emit hoveredCellChanged(_hoverCell);
    }
    if (_painting) {
        paintAt(event);  // glisser de peinture
    } else if (_dragging) {
        _dragCurrent = clampedCell(event);  // aperçu du rectangle/de la sélection
    }
}

void GameViewport::wheelEvent(QWheelEvent* event) {
    _input.onMouseWheel(event->angleDelta().y());
    if (_session) {
        return;  // essai/jeu : la molette n'alimente que l'input, aucun zoom manuel.
    }
    const int notches = event->angleDelta().y() / 120;  // 120 = un cran de molette (Qt/Win32).
    if (notches == 0) {
        return;
    }
    if (!_manualCamera) {
        // Premier cran : demarre le cadrage manuel a partir du cadrage automatique courant, pour
        // ne pas faire sauter la vue au premier zoom.
        updateEditCamera();
        _manualZoom = _camera.zoom();
        _manualCenter = _camera.center();
        _manualCamera = true;
    }
    _manualZoom =
        std::clamp(_manualZoom + static_cast<float>(notches), minManualZoom(), maxManualZoom());
}

}  // namespace hmi
