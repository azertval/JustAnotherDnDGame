// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "Core/Combat/CombatState.h"
#include "Core/Levels/Level.h"
#include "Core/Math/DeterministicRandom.h"
#include "Core/Rpg/Bestiary.h"
#include "Core/Rpg/CharacterSheet.h"
#include "Core/Rpg/Check.h"
#include "Core/Rpg/Dice.h"

/**
 * @file Core/Combat/Arena.h
 * @brief Le Colisée : un affrontement qui se monte, se joue et se rejoue à graine fixée
 *        (`LOT-50`).
 *
 * ## Ce que l'arène est, et ce qu'elle n'est pas
 *
 * Les Arènes de Tanares sont une institution du monde : deux camps y règlent un litige par leurs
 * champions, **sans mort**, sous la protection d'un rituel de Marque Héroïque. C'est ce qui en
 * fait le banc d'essai naturel du combat — un affrontement s'y rejoue indéfiniment, et c'est la
 * fiction qui l'explique, pas une entorse aux règles.
 *
 * Tout ce qui est ici est du `Core` pur : une carte (`core::Level`), une composition
 * (`core::ArenaBout`), et une session (`core::ArenaSession`) qui tient la machine à états du
 * combat (`core::CombatState`, `LOT-20`) sur la grille de la carte (`core::BattleGrid`, `LOT-19`).
 * Rien ne dépend d'une fenêtre : l'écran de mise en place (`hmi::ArenaModel`) ne fait que
 * présenter et commander.
 *
 * ## Le coup d'essai, provisoire et dit comme tel
 *
 * Les attaques sont au `LOT-21`. Sans coup, un combat ne finit jamais, et un banc d'essai qui ne
 * finit jamais ne vérifie rien. `core::ArenaSession::strike` porte donc un **coup d'essai** : un
 * d20 plus le bonus d'attaque contre la classe d'armure, puis les dés de dégâts. Le kit
 * (`core::StrikeKit`) se lit du bloc de bestiaire — la première action qui frappe — ou de la
 * fiche — le coup à mains nues du SRD, 1 + Force. Le `LOT-21` remplacera ce coup par son
 * pipeline ; ce qu'il laisse en place est le **lieu** où l'on frappe, pas la façon.
 */

namespace core {

/// Type d'entité de carte qui marque un point d'entrée de l'arène (`Source/Elements/Levels/`).
inline constexpr std::string_view ARENA_ENTRY_ENTITY_TYPE = "arenaEntry";
/// Propriété d'un point d'entrée : le camp qu'il accueille, `"allies"` ou `"enemies"`.
inline constexpr std::string_view ARENA_SIDE_PROPERTY = "side";
/// Propriété d'un point d'entrée : son rang dans l'ordre d'appel. Deux rangs égaux se départagent
/// par la position (ligne, puis colonne) — l'ordre de la donnée, jamais celui de la mémoire.
inline constexpr std::string_view ARENA_RANK_PROPERTY = "rank";
/// La **troisième économie d'action** des Marques Héroïques (§4bis) : une ressource déclarée à
/// `core::ActionEconomy`, que le rituel de l'arène accorde à chaque combattant marqué.
inline constexpr std::string_view HEROIC_ACTION_RESOURCE = "heroicAction";

/// @brief Un point d'entrée de l'arène : une case, un camp, un rang d'appel.
struct ArenaEntryPoint {
    CombatSide side = CombatSide::Allies;
    int rank = 0;
    GridPosition position;

    [[nodiscard]] bool operator==(const ArenaEntryPoint&) const = default;
};

/**
 * @brief Les points d'entrée d'une carte, rangés par camp, rang, puis position.
 *
 * Une entité `arenaEntry` sans camp lisible est ignorée : la carte reste jouable avec les entrées
 * qu'elle déclare correctement, et c'est le montage qui dira qu'il manque une place.
 */
[[nodiscard]] std::vector<ArenaEntryPoint> arenaEntryPoints(const Level& level);

/// @brief Un rôle de Marque Héroïque : huit, tabulés par le Sourcebook.
struct HeroicMark {
    std::string id;
    std::string name;
    std::string text;
};

/// @brief Les huit Marques, et ce qui n'a pas pu être lu.
struct HeroicMarkCatalog {
    std::vector<HeroicMark> marks;
    std::vector<std::string> errors;

    [[nodiscard]] const HeroicMark* find(std::string_view id) const;
};

/// @brief Charge les Marques Héroïques (`Source/Elements/Rpg/rules/heroic-marks.json`).
[[nodiscard]] HeroicMarkCatalog loadHeroicMarks(const std::filesystem::path& file);

/**
 * @brief Une arène, telle que la donnée la décrit (`Source/Elements/World/arena/`).
 *
 * Les variantes régionales du Sourcebook — l'Arène du Futur non létale, Feargus la létale, les
 * Braves des débutants, le duel de baguettes de la Magocratie — sont des **données**, pas des
 * modes : ce qui les distingue tient dans trois champs.
 */
struct Arena {
    std::string id;
    std::string name;
    std::string source;
    /// Région de l'atlas (`LOT-37`) où l'arène se tient.
    std::string region;
    /// Nom du fichier de carte dans `Source/Elements/Levels/`, vide si l'arène n'a pas encore de
    /// carte : elle existe dans le monde, pas encore comme lieu jouable.
    std::string map;
    /// Vrai si l'on y meurt. Faux par défaut : c'est la règle des Arènes, et une arène létale est
    /// l'exception écrite dans la donnée.
    bool lethal = false;
    /// Vrai si le rituel de Marque Héroïque s'y pratique : les combattants sont relevés à la fin,
    /// et disposent de la troisième économie d'action.
    bool heroicMark = true;
};

/// @brief Le catalogue des arènes, et ce qui n'a pas pu être lu.
struct ArenaCatalog {
    std::vector<Arena> arenas;
    std::vector<std::string> errors;

    [[nodiscard]] const Arena* find(std::string_view id) const;
};

/// @brief Charge les arènes d'un dossier — un fichier JSON par arène, balayé et trié.
[[nodiscard]] ArenaCatalog loadArenas(const std::filesystem::path& directory);

/**
 * @brief Ce qu'un combattant sait frapper, en attendant les attaques du `LOT-21`.
 *
 * La classe d'armure est celle de la cible ; le bonus et les dés sont ceux du coup. Une allonge
 * n'y figure pas : le coup d'essai est au contact, une case, et c'est le `LOT-22` qui dira la
 * portée.
 */
struct StrikeKit {
    int armorClass = 10;
    int attackBonus = 0;
    Dice damage;
    /// Le nom du coup, tel que le journal l'écrit : « Cimeterre », « Coup à mains nues ».
    std::string label;
};

/// @brief Le kit d'une créature : sa classe d'armure et la **première** action qui frappe.
[[nodiscard]] StrikeKit strikeKitFor(const Creature& creature);

/**
 * @brief Le kit d'un personnage : sa classe d'armure et le coup à mains nues du SRD.
 *
 * Bonus d'attaque = maîtrise + modificateur de Force ; dégâts = 1 + modificateur de Force,
 * contondants. C'est la règle du livre, pas une valeur inventée ; lire l'arme équipée est
 * l'affaire du `LOT-21`, qui connaît les propriétés d'arme.
 * @param sheet            La fiche du personnage.
 * @param proficiencyBonus Le bonus de maîtrise au niveau de la fiche (`core::proficiencyBonus`).
 */
[[nodiscard]] StrikeKit strikeKitFor(const CharacterSheet& sheet, int proficiencyBonus);

/// @brief Un combattant tel que l'écran de mise en place le compose.
struct ArenaContestant {
    CombatantProfile profile;
    StrikeKit kit;
    /// Case demandée, ou absente : le prochain point d'entrée libre de son camp.
    std::optional<GridPosition> position;
    /// Rôle de Marque Héroïque revendiqué (`core::HeroicMark::id`), ou vide.
    std::string markId;
};

/// @brief Une composition d'affrontement : qui, contre qui, à quelle graine, sous quelle règle.
struct ArenaBout {
    std::vector<ArenaContestant> contestants;
    std::uint64_t seed = 0;
    bool lethal = false;
    bool heroicMark = true;
};

/// @brief Ce que le montage d'un affrontement a produit : les enrôlés par camp, et les refus.
struct ArenaMount {
    std::vector<CombatantId> allies;
    std::vector<CombatantId> enemies;
    std::vector<MountRefusal> refusals;
};

/// @brief Ce qu'un coup d'essai a donné.
enum class StrikeResult : std::uint8_t {
    Hit,
    Missed,
    /// Aucun tour actif : le combat n'a pas commencé, ou il est fini.
    NoActiveTurn,
    /// L'attaquant a déjà dépensé son action ce tour-ci.
    NoAction,
    /// La cible n'est pas à une case, ou n'est pas un ennemi debout.
    OutOfReach,
    InvalidTarget,
};

/// @brief Le détail d'un coup d'essai, restituable (`EX-DND-003`).
struct StrikeOutcome {
    StrikeResult result = StrikeResult::InvalidTarget;
    std::optional<CheckResult> roll;
    int damage = 0;
};

/**
 * @brief Une session d'arène : la carte, la composition, et le combat qui s'y joue.
 *
 * C'est le premier endroit où `core::CombatState` est **tenu** par quelque chose du jeu, et non
 * seulement par un test. La session garde la carte et la composition pour pouvoir **rejouer** :
 * `replay` remonte le même affrontement à la même graine, et deux exécutions donnent alors le
 * même journal — c'est ce qui permet de comparer deux versions d'une mécanique.
 *
 * Le journal est une liste de lignes lisibles : crochets du combat, pas, coups, issue. Il est la
 * matière du rejeu vérifié par test, et ce que l'écran affiche.
 */
class ArenaSession {
public:
    /// @param level La carte de l'arène, conservée pour chaque rejeu.
    explicit ArenaSession(Level level);

    /**
     * @brief Monte l'affrontement : enrôle chaque combattant à sa case ou au prochain point
     *        d'entrée libre de son camp, en nommant chaque refus.
     *
     * Un combattant sans case et sans point d'entrée restant est refusé avec `OutOfBounds` : la
     * carte n'a plus de place pour lui, et le dire vaut mieux que le poser dans un mur. Le rituel
     * de Marque Héroïque, s'il s'applique, déclare la troisième économie d'action à chacun.
     */
    ArenaMount mount(const ArenaBout& bout);

    /// @brief Jette l'initiative à la graine de la composition, et ouvre le premier tour.
    bool start();

    /// @brief Remonte la même composition à la même graine, journal vidé.
    ArenaMount replay();

    /// @return La machine à états du combat, montée ou non.
    [[nodiscard]] CombatState& combat() noexcept {
        return *_combat;
    }
    [[nodiscard]] const CombatState& combat() const noexcept {
        return *_combat;
    }

    /// @return La carte de l'arène.
    [[nodiscard]] const Level& level() const noexcept {
        return _level;
    }

    /// @return La composition montée.
    [[nodiscard]] const ArenaBout& bout() const noexcept {
        return _bout;
    }

    /// @return Le kit d'un combattant enrôlé, ou `nullptr`.
    [[nodiscard]] const StrikeKit* kit(CombatantId combatant) const;

    /**
     * @brief Le coup d'essai du combattant actif sur une cible au contact.
     *
     * Déclare l'attaque (`core::CombatHook::AttackDeclared`), dépense l'action, jette le d20
     * contre la classe d'armure de la cible, et applique les dégâts si le coup porte. Tout
     * passe par la suite aléatoire de la session : un rejeu redonne les mêmes coups.
     */
    StrikeOutcome strike(CombatantId target);

    /// @brief Déplace le combattant actif ; le chemin est payé sur son budget restant.
    MoveOutcome move(GridPosition destination);

    /// @brief Termine le tour actif.
    bool endTurn();

    /// @brief Le combattant actif quitte l'arène.
    WithdrawResult withdraw();

    /// @return L'issue du combat, absente tant qu'il court.
    [[nodiscard]] std::optional<CombatOutcome> outcome() const;

    /// @return Le journal de la partie en cours, dans l'ordre.
    [[nodiscard]] const std::vector<std::string>& journal() const noexcept {
        return _journal;
    }

private:
    void subscribe();
    void record(std::string line);
    /// Relève tout le monde : le rituel de la Marque Héroïque, qui fait des Arènes un lieu
    /// sans mort. Rien dans une arène létale.
    void restoreAll();

    Level _level;
    ArenaBout _bout;
    DeterministicRandom _random{0};
    std::unique_ptr<CombatState> _combat;
    std::map<CombatantId, StrikeKit> _kits;
    std::vector<std::string> _journal;
};

}  // namespace core
