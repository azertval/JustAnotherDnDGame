// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "Core/Combat/BattleGrid.h"  // core::CombatantId
#include "Core/Ecs/AnimationClip.h"  // core::ClipSet
#include "HMI/Graphics/ArenaAnimationState.h"

/**
 * @file HMI/Graphics/ArenaAnimationDriver.h
 * @brief Fait vivre les figurines du Colisée au temps réel du rendu (`LOT-86` Phase 4) : quelle
 *        image de quel fichier de la planche chaque combattant montre, indépendamment de tout pas
 *        fixe ou de l'ECS du monde exploré.
 *
 * ## Une action, un fichier
 *
 * Contrairement au personnage exploré (`core::playerClipSet()`, plusieurs clips dans **une**
 * spritesheet), une figurine du Colisée a **un fichier par action** — `idle.png`, `walk.png`,
 * `attack.png`, `hit.png`, `death.png` (`manifest.json`, champ « animations ») — chacun décrit par
 * son propre `.anim.json` à un seul clip. `ArenaFigureAnimationSet` porte ces cinq jeux de clips
 * (nuls pour une action sans fichier : les ennemis n'ont aujourd'hui que `idle.png`) ;
 * `loadArenaFigureAnimations` les lit depuis un dossier de planche en réutilisant
 * `hmi::AnimationCatalog`.
 *
 * ## Le pilotage
 *
 * `ArenaAnimationDriver` ne lit jamais `core::ArenaSession` (`EX-ARCH-012`, même frontière que
 * `hmi::composeArenaScene`) : c'est l'appelant qui décide, d'après les événements de combat, quand
 * appeler `play()`. Le pilote se contente de faire avancer chaque animation en cours
 * (`advance(realDeltaSeconds)`) et de revenir sur `Idle` une fois une action ponctuelle
 * (`Attack`/`Hit`) terminée — sauf `Death`, où le combattant reste.
 */

namespace hmi {

/// @brief Action jouée par la figurine d'un combattant, une par fichier de sa planche.
enum class ArenaFigureAction : std::uint8_t {
    Idle,
    Walk,
    Attack,
    Hit,
    Death,
};

/**
 * @brief Le jeu de clips d'UNE figurine, un par action existante sur sa planche.
 *
 * Chaque jeu (quand il existe) porte un unique clip — le fichier `<action>.anim.json` ne décrit
 * jamais qu'une bande. Un pointeur nul signifie qu'aucun fichier ne porte cette action pour cette
 * figurine : `ArenaAnimationDriver::play` y retombe alors sur `Idle`, jamais une erreur
 * (`EX-NFR-040`) — le cas normal des ennemis, qui n'ont qu'`idle.png` (`LOT-50`).
 */
struct ArenaFigureAnimationSet {
    std::shared_ptr<const core::ClipSet> idle;
    std::shared_ptr<const core::ClipSet> walk;
    std::shared_ptr<const core::ClipSet> attack;
    std::shared_ptr<const core::ClipSet> hit;
    std::shared_ptr<const core::ClipSet> death;

    /// @return Le jeu de clips de @p action, nul si cette figurine n'a pas ce fichier.
    [[nodiscard]] const std::shared_ptr<const core::ClipSet>& forAction(
        ArenaFigureAction action) const;
};

/**
 * @brief Résultat de la lecture des `.anim.json` d'un dossier de planche.
 *
 * Un fichier absent (`AnimationCatalogError::FileNotFound`) laisse simplement l'action correspon-
 * dante sans clips dans `clips` — jamais dans `errors`, qui ne porte que les fichiers **présents
 * mais invalides** (`EX-NFR-040`, même distinction que `hmi::AnimationCatalog::loadFromFile`).
 */
struct ArenaFigureAnimationLoad {
    ArenaFigureAnimationSet clips;
    std::vector<std::string> errors;
};

/**
 * @brief Lit le jeu de clips d'une figurine depuis les `.anim.json` de son dossier de planche.
 *
 * Réutilise `hmi::AnimationCatalog::loadFromFile` pour chacun des cinq fichiers possibles
 * (`idle.anim.json`, `walk.anim.json`, `attack.anim.json`, `hit.anim.json`, `death.anim.json`) :
 * cette fonction ne parse rien elle-même, elle ne fait qu'assembler les cinq lectures.
 * @param sheetDirectory Dossier de la figurine (ex. `Coliseum/characters/bram`).
 */
[[nodiscard]] ArenaFigureAnimationLoad loadArenaFigureAnimations(
    const std::filesystem::path& sheetDirectory);

/**
 * @brief L'état d'animation d'UN combattant : son action, le clip qui la joue, l'image et le temps
 *        écoulé depuis le début de cette image.
 *
 * Donnée pure (`EX-ARCH-011`) : `advanceArenaAnimation` est la seule à la faire avancer.
 */
struct ArenaCombatantAnimation {
    /// Planche du combattant (`ArenaAppearanceCatalog::FigureAppearance::sheet`), reprise pour
    /// retrouver son `Idle` sans redemander l'appelant quand une action ponctuelle se termine.
    std::string sheet;
    ArenaFigureAction action = ArenaFigureAction::Idle;
    /// Jeu de clips de l'action courante (un seul clip dedans, voir `ArenaFigureAnimationSet`).
    std::shared_ptr<const core::ClipSet> clips;
    /// Indice de l'image courante **dans le clip**, pas dans la bande — voir `frame()`.
    int frameIndex = 0;
    float elapsed = 0.0f;

    /// @return L'indice d'image dans la bande, prêt pour `hmi::ArenaFigureAnimation::frame`.
    [[nodiscard]] int frame() const;
};

/**
 * @brief Fait avancer l'animation d'UN combattant d'un pas de temps réel.
 *
 * Fonction pure (`EX-ARCH-011`, même patron que `core::advanceAnimation`, dont elle reprend la
 * logique de bouclage/arrêt pour un jeu à un seul clip) : une pose unique ou sans clips n'avance
 * jamais (aucune dérive de `elapsed`, `EX-NFR-040`) ; un clip bouclé revient à la première image ;
 * un clip ponctuel (`loop: false`) s'arrête net sur la dernière et n'accumule plus de temps —
 * `ArenaAnimationDriver::advance` décide ensuite s'il faut y répondre.
 * @param state            État à faire avancer.
 * @param realDeltaSeconds Temps écoulé depuis le dernier appel, en secondes.
 */
void advanceArenaAnimation(ArenaCombatantAnimation& state, float realDeltaSeconds);

/**
 * @brief Pilote l'animation des figurines du Colisée, par combattant, au temps réel du rendu.
 *
 * Ni ECS ni pas fixe : une entrée par combattant suivi, avancée par `advance(realDeltaSeconds)`.
 * Ne lit jamais `core::ArenaSession` (`EX-ARCH-012`) — c'est à l'appelant de déclencher `play()`
 * d'après les événements de combat qu'il observe par ailleurs.
 */
class ArenaAnimationDriver {
public:
    /// Déclare le jeu de clips d'une figurine, adressée par le nom de son dossier de planche
    /// (`"bram"`, `"archer"`…). Un appel répété avec le même nom remplace la déclaration.
    void setFigureAnimations(std::string sheet, ArenaFigureAnimationSet clips);

    /**
     * @brief Démarre (ou relance) une action pour un combattant.
     *
     * `Idle`/`Walk` ne relancent pas depuis la première image s'ils sont déjà en cours (boucle
     * continue) ; `Attack`/`Hit` relancent toujours, même appelés coup sur coup ; `Death` ne se
     * relance jamais une fois atteint — un mort le reste, quel que soit l'appel suivant
     * (`EX-NFR-040` : pas de résurrection par un appel erroné). Une action sans fichier pour cette
     * planche (ex. l'attaque d'un ennemi) retombe silencieusement sur `Idle`.
     * @param combatant Combattant concerné.
     * @param sheet     Sa planche (`ArenaAppearanceCatalog::FigureAppearance::sheet`).
     * @param action    Action désirée.
     */
    void play(core::CombatantId combatant, std::string_view sheet, ArenaFigureAction action);

    /// Retire un combattant du pilotage (sorti de la grille, `CombatantStatus::Withdrawn`).
    void remove(core::CombatantId combatant);

    /// Fait avancer toutes les animations suivies d'un pas de temps réel, et ramène sur `Idle`
    /// toute action ponctuelle (`Attack`/`Hit`) arrivée à sa dernière image.
    void advance(float realDeltaSeconds);

    /// @return L'image courante de chaque combattant suivi, prête pour `hmi::composeArenaScene`.
    [[nodiscard]] ArenaAnimationState snapshot() const;

    /// @return L'action en cours d'un combattant, `Idle` s'il n'est pas (encore) suivi.
    [[nodiscard]] ArenaFigureAction actionOf(core::CombatantId combatant) const;

private:
    std::map<std::string, ArenaFigureAnimationSet, std::less<>> _figures;
    std::map<core::CombatantId, ArenaCombatantAnimation> _states;
};

}  // namespace hmi
