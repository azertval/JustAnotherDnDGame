// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Graphics/MechanismVisuals.h"

#include <memory>

#include "Core/Ecs/Systems/AnimationSystem.h"
#include "HMI/Graphics/GraphicsLog.h"

namespace hmi {

namespace {

// Bascule l'animation d'un mecanisme vers preferredClip s'il existe dans clips ; sinon replie sur
// fallbackClip (clip d'etat cible) s'il existe ; sinon reste sur le premier clip du jeu (repli
// lisible, LOT-47 TACHE-02) et journalise UNE fois l'asset et le clip manquants -- jamais pour un
// simple clip de TRANSITION absent (bascule directe, cas legitime, silencieux). Meme patron que
// core::MechanismController, qui journalise deja depuis une fonction par ailleurs testee (LOT-19).
void setMechanismClip(core::Animation& animation, const core::ClipSet& clips,
                      const std::string& preferredClip, const std::string& fallbackClip,
                      const std::string& assetKey, std::set<std::string>& warnedMissingClips) {
    int index = clips.indexOf(preferredClip);
    if (index < 0 && preferredClip != fallbackClip) {
        index = clips.indexOf(fallbackClip);  // transition absente : bascule directe, silencieuse.
    }
    if (index < 0) {
        // Le clip d'ETAT cible lui-meme est introuvable (pas seulement une transition absente) :
        // repli lisible + un seul message par (asset, clip d'etat).
        const std::string key = assetKey + "|" + fallbackClip;
        if (warnedMissingClips.insert(key).second) {
            GRAPHICS_LOG_WARNING("Mecanisme : clip d'etat '" + fallbackClip +
                                 "' introuvable dans '" + assetKey +
                                 "' -- repli sur le premier clip disponible (LOT-47).");
        }
        index = 0;  // repli lisible : ClipSet::clipAt borne deja les index invalides/jeu vide.
    }
    if (index == animation.clipIndex) {
        return;  // deja sur ce clip (ex. deja replie sur l'etat cible) : ne pas redemarrer l'image.
    }
    animation.clipIndex = index;
    animation.frameIndex = 0;
    animation.elapsed = 0.0F;
}

}  // namespace

bool isStatefulMechanism(core::TileType type) noexcept {
    switch (type) {
        case core::TileType::Door:
        case core::TileType::Switch:
        case core::TileType::PressurePlate:
        case core::TileType::Key:
        case core::TileType::LockedDoor:
            return true;
        default:
            return false;
    }
}

std::optional<std::string> mechanismTargetClip(core::TileType type, bool active) noexcept {
    switch (type) {
        case core::TileType::Door:
            return active ? MECHANISM_CLIP_DOOR_OPEN : MECHANISM_CLIP_DOOR_CLOSED;
        case core::TileType::Switch:
            return active ? MECHANISM_CLIP_SWITCH_ACTIVE : MECHANISM_CLIP_SWITCH_INACTIVE;
        case core::TileType::PressurePlate:
            return active ? MECHANISM_CLIP_PLATE_PRESSED : MECHANISM_CLIP_PLATE_RELEASED;
        case core::TileType::Key:
            // active == cle ramassee (meme booleen que la porte verrouillee liee, EX-GP-023) :
            // collected masque la cle, present l'affiche.
            return active ? MECHANISM_CLIP_KEY_COLLECTED : MECHANISM_CLIP_KEY_PRESENT;
        case core::TileType::LockedDoor:
            return active ? MECHANISM_CLIP_LOCKED_DOOR_OPEN : MECHANISM_CLIP_LOCKED_DOOR_CLOSED;
        default:
            return std::nullopt;  // type sans etat : aucune demande de clip.
    }
}

std::optional<std::string> mechanismTransitionClip(core::TileType type, bool wasActive,
                                                   bool isActive) noexcept {
    if (type != core::TileType::Door || wasActive == isActive) {
        // Seule la porte transitionne visiblement (LOT-47 TACHE-02) ; les autres familles basculent
        // directement sur leur clip d'etat cible. Aucun changement : rien a declencher.
        return std::nullopt;
    }
    return isActive ? MECHANISM_CLIP_DOOR_OPENING : MECHANISM_CLIP_DOOR_CLOSING;
}

std::vector<std::string> mechanismExpectedClips(core::TileType type) {
    switch (type) {
        case core::TileType::Door:
            return {MECHANISM_CLIP_DOOR_CLOSED, MECHANISM_CLIP_DOOR_OPENING,
                    MECHANISM_CLIP_DOOR_OPEN, MECHANISM_CLIP_DOOR_CLOSING};
        case core::TileType::Switch:
            return {MECHANISM_CLIP_SWITCH_INACTIVE, MECHANISM_CLIP_SWITCH_ACTIVE};
        case core::TileType::PressurePlate:
            return {MECHANISM_CLIP_PLATE_RELEASED, MECHANISM_CLIP_PLATE_PRESSED};
        case core::TileType::Key:
            return {MECHANISM_CLIP_KEY_PRESENT, MECHANISM_CLIP_KEY_COLLECTED};
        case core::TileType::LockedDoor:
            return {MECHANISM_CLIP_LOCKED_DOOR_CLOSED, MECHANISM_CLIP_LOCKED_DOOR_OPEN};
        default:
            return {};
    }
}

core::AtlasRegion advanceMechanismVisual(MechanismVisualState& state,
                                         const AnimationDescription& description,
                                         core::TileType type, bool active,
                                         const std::string& assetKey, float fixedDelta,
                                         std::set<std::string>& warnedMissingClips) {
    const bool changed = !state.initialized || active != state.previousActive;
    if (!state.animation.clips) {
        // Premiere rencontre de cet asset pour cette instance : copie immuable du jeu de clips
        // (meme patron que advanceTileAnimations, SpriteRenderer.cpp, LOT-46 TACHE-05).
        state.animation.clips = std::make_shared<core::ClipSet>(description.clips);
    }
    if (changed) {
        const std::string targetClip = mechanismTargetClip(type, active).value_or(std::string{});
        std::string preferredClip = targetClip;
        if (state.initialized) {
            // Pas le tout premier calcul (chargement/rechargement de niveau) : une transition est
            // possible. Au premier calcul, on rejoint l'etat cible directement, sans transition.
            if (const std::optional<std::string> transition =
                    mechanismTransitionClip(type, state.previousActive, active)) {
                preferredClip = *transition;
            }
        }
        setMechanismClip(state.animation, *state.animation.clips, preferredClip, targetClip,
                         assetKey, warnedMissingClips);
        state.previousActive = active;
        state.initialized = true;
    }
    core::advanceAnimation(state.animation, fixedDelta);
    return AnimationCatalog::currentFrameRegion(description, state.animation);
}

float mechanismDiagnosticAlpha(RenderMode mode, bool active, float activeAlpha,
                               float inactiveAlpha) noexcept {
    if (mode != RenderMode::Physique) {
        return 1.0F;  // mode Texture : l'etat se voit au clip, jamais a l'alpha (LOT-47 TACHE-03).
    }
    return active ? activeAlpha : inactiveAlpha;
}

}  // namespace hmi
