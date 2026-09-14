// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

/**
 * @file Core/Combat/Attack.h
 * @brief L'attaque : un jet de d20 contre la classe d'armure, amendable avant d'être figé, puis
 *        les dégâts (`LOT-21`, `EX-CBT-030`, `EX-CBT-031`, `EX-DND-003`).
 *
 * ## Ce que dit le Manuel, et où chaque règle vit
 *
 * Manuel des Joueurs, chapitre 9, « Effectuer une attaque » :
 *
 * - **choisir une cible** à distance d'attaque — l'allonge au corps à corps (`core::inReach`) ;
 * - **déterminer les modificateurs** — avantage, désavantage, bonus : `core::AttackRoll`, et les
 *   circonstances que la grille sait déjà dire (`core::attackCircumstances`) ;
 * - **résoudre** — le d20, puis les dés de dégâts si l'attaque touche (`core::resolveAttack`).
 *
 * « Faire 1 ou 20 » : un 20 au d20 **touche automatiquement**, quels que soient les modificateurs
 * et la CA, et c'est un coup critique ; un 1 **rate automatiquement**. Le critique double **les
 * dés** de dégâts, pas les modificateurs (`core::rollDamage`, `EX-CBT-031`).
 *
 * ## Un jet qui est un objet
 *
 * Les livres de Tanares lisent le d20 **brut** (*Omen*, *Augurs*), le relancent **avant** la
 * résolution, ajoutent un modificateur **après** avoir vu le résultat (*Future Guard*), substituent
 * un résultat stocké. Rien de cela ne s'écrit si le jet est un entier rendu par une fonction : le
 * jet est donc un `core::AttackRoll` que trois points d'insertion reçoivent, et l'issue n'est figée
 * qu'après le dernier. Chaque amendement s'inscrit dans le jet, pour que le journal le dise.
 *
 * ## Hors de ce fichier, nommément
 *
 * La portée à distance, la ligne de vue et l'abri (`LOT-22`) ; l'inconscience, les jets contre la
 * mort, la mort instantanée et le coup qui assomme (`LOT-72`, qui lit l'excédent et le critique que
 * ce lot rapporte) ; les sorts et les attaques de classe, ajoutés avec les classes.
 */

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "Core/Combat/CombatState.h"
#include "Core/Combat/Damage.h"
#include "Core/Math/DeterministicRandom.h"
#include "Core/Rpg/Check.h"

namespace core {

struct CharacterSheet;
struct Creature;
struct Weapon;

/// @brief Au corps à corps, ou à distance.
enum class AttackKind : std::uint8_t {
    Melee,
    Ranged,
};

/// @brief Les deux portées d'une attaque à distance, en cases.
struct AttackRange {
    int normal = 0;
    int maximum = 0;
};

/**
 * @brief Ce qu'un combattant sait frapper : une arme, une morsure, un coup à mains nues.
 *
 * Les modificateurs du jet portent **leur origine** (`EX-DND-003`) : « Force +3 », « maîtrise +2 »,
 * ou le bonus d'un bloc de bestiaire tel que le livre l'imprime. Les dégâts sont une liste de
 * clauses typées : une morsure venimeuse en a deux.
 */
struct AttackProfile {
    /// Le nom tel que le journal l'écrit : « Épée longue », « Morsure », « coup à mains nues ».
    std::string label;
    AttackKind kind = AttackKind::Melee;
    std::vector<Modifier> modifiers;
    std::vector<DamageClause> damage;
    /// Allonge au corps à corps, en cases — 1 pour 1,50 m, la plupart des créatures.
    int reach = 1;
    /**
     * @brief Les portées d'une attaque à distance, si la donnée les porte.
     *
     * Aucune arme ni aucune action du catalogue ne les porte encore autrement qu'en prose (« portée
     * 24/96 m ») : c'est le `LOT-22` qui les structure. En attendant, une attaque à distance sans
     * portée connue ne vise qu'au contact — avec le désavantage que le Manuel impose au tir au
     * contact d'un ennemi.
     */
    std::optional<AttackRange> range;
    /// Plus petit résultat du d20 qui fait un critique. 20 par défaut ; un Champion le baisse.
    int criticalThreshold = 20;
};

/// @brief Les attaques d'une créature, et les actions qu'on n'a pas su en tirer.
struct CreatureAttacks {
    std::vector<AttackProfile> attacks;
    /// Une action à bonus d'attaque dont les dégâts ne sont pas typés : jamais un type par défaut
    /// (`EX-CBT-032`), le nom de l'action et la raison.
    std::vector<std::string> refused;
};

/**
 * @brief Les attaques d'un bloc de bestiaire : chaque action qui a un bonus d'attaque et des
 * dégâts.
 *
 * Une action **avec** allonge est au corps à corps (allonge en cases, arrondie, au moins 1) ; une
 * action **sans** allonge est à distance. Le bonus du bloc est pris tel quel : le livre l'a déjà
 * calculé. Une action qui a un bonus mais aucun dégât (la toile de l'araignée géante) n'est pas une
 * attaque de ce lot — elle entrave, et c'est l'affaire des conditions (`LOT-72`).
 *
 * @note Le bestiaire ne type que la **première** clause de dégâts (`core::loadBestiary`) : le
 *       moteur sait en porter plusieurs, la donnée n'en fournit qu'une.
 */
[[nodiscard]] CreatureAttacks attacksFor(const Creature& creature);

/**
 * @brief L'attaque d'un personnage avec une arme, ou à mains nues si @p weapon est nul.
 *
 * Manuel, chapitre 9, « Modificateurs du jet » : Force au corps à corps, Dextérité à distance, la
 * meilleure des deux pour une arme de finesse (`core::weaponAttackAbility`) ; le bonus de maîtrise
 * si l'arme est maîtrisée ; le **même** modificateur de caractéristique aux dégâts. « Une attaque à
 * mains nues inflige une quantité de dégâts contondants égale à 1 + votre modificateur de Force.
 * Vous maîtrisez automatiquement ce type d'attaques. »
 *
 * @param sheet La fiche, pour ses modificateurs de Force et de Dextérité.
 * @param weapon L'arme en main, ou `nullptr` pour le coup à mains nues.
 * @param proficiencyBonus Le bonus de maîtrise au niveau de la fiche (`core::proficiencyBonus`).
 * @param proficient Faux si le personnage ne maîtrise pas l'arme. Les classes provisoires du
 *        `LOT-36` ne déclarent pas leurs maîtrises d'armes : l'appelant passe vrai jusqu'au socle
 *        de classe (`LOT-47`). Sans effet à mains nues.
 */
[[nodiscard]] AttackProfile weaponAttackFor(const CharacterSheet& sheet, const Weapon* weapon,
                                            int proficiencyBonus, bool proficient = true);

/**
 * @brief La distance en cases entre deux combattants, **emprises comprises** : zéro contact
 *        impossible, 1 pour deux emprises adjacentes, diagonale comprise.
 *
 * Manuel, « Jouer sur un quadrillage » : on compte les cases en partant d'une case adjacente à la
 * première créature et en finissant sur la case de la seconde. Un ogre de taille G sur 2 × 2 cases
 * touche donc à 1 tout ce qui borde son emprise, et non ce qui borde son ancre.
 * @return Vide si l'un des deux n'est pas sur la grille.
 */
[[nodiscard]] std::optional<int> gridDistance(const CombatState& combat, CombatantId from,
                                              CombatantId to);

/// @brief La même distance, @p mover supposé ancré en @p moverAnchor.
[[nodiscard]] std::optional<int> gridDistanceFrom(const CombatState& combat, CombatantId mover,
                                                  GridPosition moverAnchor, CombatantId other);

/**
 * @brief Vrai si @p target est à portée de l'attaque : allonge au corps à corps ; au contact, ou
 *        dans la portée maximale si elle est connue, à distance.
 */
[[nodiscard]] bool inReach(const CombatState& combat, CombatantId attacker, CombatantId target,
                           const AttackProfile& profile);

/// @brief Les sources nommées d'avantage et de désavantage d'un jet.
struct AttackCircumstances {
    std::vector<std::string> advantages;
    std::vector<std::string> disadvantages;
};

/**
 * @brief Ce que la grille sait dire des circonstances d'une attaque (Manuel, chapitre 9).
 *
 * - **Attaque à distance dans un combat au corps à corps** : désavantage si une créature hostile
 *   debout se trouve à une case de l'attaquant ;
 * - **au-delà de la portée normale** : désavantage.
 *
 * Voir sans être vu, l'abri et l'état de la cible sont au `LOT-22` et au `LOT-72`.
 */
[[nodiscard]] AttackCircumstances attackCircumstances(const CombatState& combat,
                                                      CombatantId attacker, CombatantId target,
                                                      const AttackProfile& profile);

/// @brief Les trois instants où un jet d'attaque se lit et s'amende, dans l'ordre.
enum class AttackRollStage : std::uint8_t {
    /// Avant les dés : ajouter une source d'avantage ou de désavantage, un bonus, changer la CA.
    BeforeRoll,
    /// Les d20 sont tombés : les lire bruts, en relancer un, en substituer un.
    DiceRolled,
    /// Le total est connu : ajouter un modificateur après l'avoir vu. Dernier instant avant
    /// l'issue.
    BeforeOutcome,
};

/**
 * @brief Un jet d'attaque : la demande, les dés, les amendements, et l'issue une fois figée.
 *
 * `check` suit le jet à chaque étape : dés lancés, dé retenu, modificateurs, total, seuil (la CA).
 */
struct AttackRoll {
    CombatantId attacker{};
    CombatantId target{};
    std::string label;
    int armorClass = 10;
    int criticalThreshold = 20;
    std::vector<std::string> advantages;
    std::vector<std::string> disadvantages;
    CheckResult check;
    /// Ce que les crochets ont changé, dans l'ordre : « relance (Chance) : 1 -> 14 ».
    std::vector<std::string> amendments;
    /// L'issue, figée après `BeforeOutcome`.
    bool hit = false;
    bool critical = false;

    /// @brief Relance le d20 d'indice @p die. Sans effet hors des dés lancés.
    void reroll(std::size_t die, DeterministicRandom& random, const std::string& source);
    /// @brief Remplace le d20 d'indice @p die par @p value (un résultat stocké, *Portent*).
    void substitute(std::size_t die, int value, const std::string& source);
    /// @brief Ajoute un modificateur, avec son origine.
    void addModifier(Modifier modifier);
    /// @brief Recalcule le dé retenu et le total depuis les dés et les modificateurs.
    void recompute();
};

/// @brief Un point d'insertion d'un jet d'attaque.
using AttackRollListener = std::function<void(AttackRoll&, DeterministicRandom&)>;

/// @brief Les greffons des jets d'attaque, par étape, dans l'ordre d'insertion.
class AttackHooks {
public:
    void insert(AttackRollStage stage, AttackRollListener listener);
    void run(AttackRollStage stage, AttackRoll& roll, DeterministicRandom& random) const;

private:
    std::vector<std::pair<AttackRollStage, AttackRollListener>> _listeners;
};

/**
 * @brief Jette le d20 d'une attaque : crochets, dés, amendements, puis l'issue.
 *
 * La posture se déduit des **nombres** de sources (`core::rollStance`, `EX-DND-002`). L'issue :
 * un 1 naturel rate ; un résultat au moins égal au seuil critique touche et est critique ; sinon,
 * le total atteint ou dépasse la CA.
 */
[[nodiscard]] AttackRoll rollAttack(AttackRoll request, const AttackHooks& hooks,
                                    DeterministicRandom& random);

/// @brief Une attaque résolue : le jet, les dégâts lancés, et ce que la cible en a reçu.
struct AttackOutcome {
    AttackRoll roll;
    std::string attackerName;
    std::string targetName;
    std::vector<RolledDamage> damage;
    std::optional<DamageReport> report;

    /**
     * @brief L'entrée de journal (`EX-DND-003`) : qui, quoi, le d20 et chaque modificateur avec
     *        son origine, la CA, l'issue, les dés de dégâts, chaque étape qui les a changés, et les
     *        points de vie avant et après.
     *
     * « 7 + 3 = 10 contre CA 15 : raté » est une information ; « tu as raté » n'en est pas une.
     */
    [[nodiscard]] std::string describe() const;
};

/// @brief Ce qu'une résolution consulte en plus du profil : greffons et circonstances.
struct AttackContext {
    const AttackHooks* hooks = nullptr;
    const DamagePipeline* pipeline = nullptr;
    /// Sources supplémentaires, que l'appelant connaît et la grille non (une esquive, une aide).
    AttackCircumstances circumstances;
};

/**
 * @brief Résout une attaque dans le combat : déclaration, jet, dégâts, points de vie.
 *
 * Annonce `CombatHook::AttackDeclared` **avant** le jet (`LOT-20`), ajoute les circonstances que la
 * grille dit, jette, et si l'attaque touche, fait traverser les dégâts au pipeline — qui se termine
 * dans `core::CombatState::applyDamage`. Ne dépense **aucune** ressource : une attaque de l'action
 * *attaquer* dépense l'action, une attaque d'opportunité la réaction, et c'est l'appelant qui sait
 * laquelle il joue. Ne vérifie pas la portée, pour la même raison (`core::inReach`).
 *
 * @return Vide si l'attaque ne peut pas être déclarée : l'un des deux n'est pas debout, ou le
 *         combat n'est pas en cours.
 */
[[nodiscard]] std::optional<AttackOutcome> resolveAttack(CombatState& combat, CombatantId attacker,
                                                         CombatantId target,
                                                         const AttackProfile& profile,
                                                         DeterministicRandom& random,
                                                         const AttackContext& context = {});

}  // namespace core
