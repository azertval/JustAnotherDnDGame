// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Graphics/ArenaAnimationDriver.h"

#include <utility>

#include "HMI/Graphics/AnimationCatalog.h"

namespace hmi {

namespace {

// Nom de fichier associe a chaque action, sous le dossier de la figurine (manifest.json, champ
// « animations »).
[[nodiscard]] const char* actionFileName(ArenaFigureAction action) {
    switch (action) {
        case ArenaFigureAction::Idle:
            return "idle.anim.json";
        case ArenaFigureAction::Walk:
            return "walk.anim.json";
        case ArenaFigureAction::Attack:
            return "attack.anim.json";
        case ArenaFigureAction::Hit:
            return "hit.anim.json";
        case ArenaFigureAction::Death:
            return "death.anim.json";
    }
    return "idle.anim.json";
}

// Lit UN fichier optionnel du dossier de planche. Absent : nul, silencieusement. Present mais
// invalide : nul, et le message rejoint errors.
[[nodiscard]] std::shared_ptr<const core::ClipSet> loadOptionalClips(
    const std::filesystem::path& sheetDirectory, ArenaFigureAction action,
    std::vector<std::string>& errors) {
    const std::filesystem::path path = sheetDirectory / actionFileName(action);
    const AnimationDescriptionResult result = AnimationCatalog::loadFromFile(path);
    if (result.ok()) {
        return std::make_shared<const core::ClipSet>(std::move(result.description->clips));
    }
    if (result.errorCode != AnimationCatalogError::FileNotFound) {
        errors.push_back(path.string() + " : " + result.error);
    }
    return nullptr;
}

}  // namespace

const std::shared_ptr<const core::ClipSet>& ArenaFigureAnimationSet::forAction(
    ArenaFigureAction action) const {
    switch (action) {
        case ArenaFigureAction::Idle:
            return idle;
        case ArenaFigureAction::Walk:
            return walk;
        case ArenaFigureAction::Attack:
            return attack;
        case ArenaFigureAction::Hit:
            return hit;
        case ArenaFigureAction::Death:
            return death;
    }
    return idle;
}

ArenaFigureAnimationLoad loadArenaFigureAnimations(const std::filesystem::path& sheetDirectory) {
    ArenaFigureAnimationLoad load;
    load.clips.idle = loadOptionalClips(sheetDirectory, ArenaFigureAction::Idle, load.errors);
    load.clips.walk = loadOptionalClips(sheetDirectory, ArenaFigureAction::Walk, load.errors);
    load.clips.attack = loadOptionalClips(sheetDirectory, ArenaFigureAction::Attack, load.errors);
    load.clips.hit = loadOptionalClips(sheetDirectory, ArenaFigureAction::Hit, load.errors);
    load.clips.death = loadOptionalClips(sheetDirectory, ArenaFigureAction::Death, load.errors);
    return load;
}

int ArenaCombatantAnimation::frame() const {
    if (!clips || clips->clipCount() == 0) {
        return 0;
    }
    const core::AnimationClip& clip = clips->clipAt(0);
    if (clip.frames.empty()) {
        return 0;
    }
    const bool inBounds =
        frameIndex >= 0 && static_cast<std::size_t>(frameIndex) < clip.frames.size();
    return clip.frames[static_cast<std::size_t>(inBounds ? frameIndex : 0)];
}

void advanceArenaAnimation(ArenaCombatantAnimation& state, float realDeltaSeconds) {
    if (!state.clips || state.clips->clipCount() == 0) {
        return;
    }
    const core::AnimationClip& clip = state.clips->clipAt(0);
    if (clip.frames.size() <= 1 || clip.frameDuration <= 0.0F) {
        return;  // pose unique : jamais animee, aucun temps accumule (EX-NFR-040).
    }

    state.elapsed += realDeltaSeconds;
    while (state.elapsed >= clip.frameDuration) {
        state.elapsed -= clip.frameDuration;
        const int frameCount = static_cast<int>(clip.frames.size());
        if (state.frameIndex + 1 < frameCount) {
            ++state.frameIndex;
            continue;
        }
        if (clip.endMode == core::ClipEndMode::Loop) {
            state.frameIndex = 0;
            continue;
        }
        // Ponctuel (attaque, coup, mort) : reste net sur la derniere image, sans derive.
        state.elapsed = 0.0F;
        break;
    }
}

void ArenaAnimationDriver::setFigureAnimations(std::string sheet, ArenaFigureAnimationSet clips) {
    _figures[std::move(sheet)] = std::move(clips);
}

void ArenaAnimationDriver::play(core::CombatantId combatant, std::string_view sheet,
                                 ArenaFigureAction action) {
    ArenaCombatantAnimation& state = _states[combatant];
    if (state.action == ArenaFigureAction::Death) {
        return;  // un mort ne se relance jamais.
    }

    const auto found = _figures.find(sheet);
    std::shared_ptr<const core::ClipSet> clips =
        found != _figures.end() ? found->second.forAction(action) : nullptr;
    if (!clips) {
        // Action non declaree pour cette planche (ex. l'attaque d'un ennemi sans attack.png) :
        // repli silencieux sur Idle (EX-NFR-040).
        action = ArenaFigureAction::Idle;
        clips = found != _figures.end() ? found->second.idle : nullptr;
    }

    const bool continuous =
        action == ArenaFigureAction::Idle || action == ArenaFigureAction::Walk;
    if (continuous && state.action == action && state.clips == clips) {
        return;  // deja en cours : ne pas repartir de l'image 0.
    }

    state.sheet.assign(sheet);
    state.action = action;
    state.clips = std::move(clips);
    state.frameIndex = 0;
    state.elapsed = 0.0F;
}

void ArenaAnimationDriver::remove(core::CombatantId combatant) {
    _states.erase(combatant);
}

void ArenaAnimationDriver::advance(float realDeltaSeconds) {
    for (auto& [id, state] : _states) {
        advanceArenaAnimation(state, realDeltaSeconds);

        const bool oneShotAction =
            state.action == ArenaFigureAction::Attack || state.action == ArenaFigureAction::Hit;
        if (!oneShotAction || !state.clips || state.clips->clipCount() == 0) {
            continue;
        }
        const core::AnimationClip& clip = state.clips->clipAt(0);
        const bool onLastFrame = state.frameIndex + 1 >= static_cast<int>(clip.frames.size());
        if (clip.endMode != core::ClipEndMode::OneShot || !onLastFrame || state.elapsed != 0.0F) {
            continue;
        }
        // Termine : retour sur Idle. Copie du nom de planche avant l'appel, qui reecrit ce meme
        // champ sur ce meme etat -- eviter une auto-affectation depuis une vue sur son propre
        // tampon (state.sheet serait a la fois source et destination).
        const std::string sheet = state.sheet;
        play(id, sheet, ArenaFigureAction::Idle);
    }
}

ArenaAnimationState ArenaAnimationDriver::snapshot() const {
    ArenaAnimationState result;
    for (const auto& [id, state] : _states) {
        result.figures[id] = ArenaFigureAnimation{.frame = state.frame()};
    }
    return result;
}

ArenaFigureAction ArenaAnimationDriver::actionOf(core::CombatantId combatant) const {
    const auto found = _states.find(combatant);
    return found != _states.end() ? found->second.action : ArenaFigureAction::Idle;
}

}  // namespace hmi
