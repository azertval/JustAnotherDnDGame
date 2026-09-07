// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <span>

/**
 * @file HMI/Interface/RpgScreens.h
 * @brief Catalogue des écrans du RPG et **ossature** de leur contenu (`LOT-68`, `EX-IHM-090`,
 *        `EX-IHM-091`).
 *
 * Logique **pure** (aucune dépendance Qt), testable hors instance d'application (`EX-NFR-010`) --
 * même patron que `HMI/Interface/ScreenFlow.h` et `HMI/Interface/ActionCatalog.h`.
 *
 * ## Une table, et non huit écrans écrits à la main
 *
 * Les huit écrans que le RPG doit à terme porter -- fiche, inventaire, journal, carte, dialogue,
 * marchand, tableau de la Guilde, ATH de combat -- n'ont, à ce lot, aucun contenu à afficher : les
 * données viendront des lots qui les remplissent (`LOT-38`, `LOT-42`, `LOT-45`, `LOT-24`). Ce qui
 * est livré ici est ce qu'ils ont **en commun** : leur cadre, leur navigation, leur parcours de
 * focus, et leur **ossature**.
 *
 * Cette ossature est décrite en **données** plutôt qu'en code d'interface, pour la raison qu'écrit
 * `EX-IHM-090` : un neuvième écran doit coûter une **ligne de table**, jamais une retouche des huit
 * autres. Le rendu Qt (`hmi::RpgScreenFrame`) sait peindre les sept genres de blocs ci-dessous et
 * rien de plus ; il n'a aucune connaissance d'un écran en particulier.
 */

namespace hmi {

/// Les huit écrans du RPG. L'ordre est celui du **cycle** de navigation
/// (`nextRpgScreen`/`previousRpgScreen`) et celui de la table de `rpgScreens()`.
enum class RpgScreenId {
    CharacterSheet,  ///< Fiche de personnage (remplie par le `LOT-38`).
    Inventory,       ///< Inventaire et équipement (`LOT-14`).
    QuestJournal,    ///< Journal de quêtes (`LOT-16`).
    WorldMap,        ///< Carte du monde (`LOT-42`).
    Dialogue,        ///< Dialogue avec un PNJ (`LOT-15`).
    Merchant,        ///< Marchand (`LOT-26`).
    GuildBoard,      ///< Tableau de la Guilde des Aventuriers (`LOT-45`).
    CombatHud,       ///< Affichage tête haute de combat (`LOT-24`).
};

/// Nombre d'écrans du catalogue. Dérivé de la table, jamais écrit deux fois.
inline constexpr std::size_t RPG_SCREEN_COUNT = 8;

/**
 * @brief Règle de **superposition** d'un écran (`EX-IHM-091`) : ce qui met le jeu en pause et ce
 *        qui se consulte en marchant.
 *
 * Portée par la **description** de l'écran, jamais par le code qui l'ouvre : une règle décidée à
 * l'ouverture se contredit d'un point d'appel à l'autre, et personne ne peut la relire d'un coup
 * d'œil.
 */
enum class RpgSuperposition {
    /// La simulation est **suspendue** tant que l'écran est ouvert : on ne lit pas sa fiche en
    /// esquivant un gobelin.
    PausesGame,
    /// L'écran se consulte **en marchant** : la simulation continue derrière lui.
    WhileWalking,
};

/// Genres de blocs que le châssis sait peindre. Volontairement peu nombreux : sept formes
/// suffisent aux huit écrans, et un genre ajouté pour un seul écran est le premier pas vers huit
/// écrans écrits à la main.
enum class RpgBlockKind {
    Fields,     ///< Lignes « libellé → valeur ». `labelKeys` en donne les libellés.
    Grid,       ///< Grille de cases vides (sac, caractéristiques) : `columns` x `rows`.
    List,       ///< Liste d'entrées vides (quêtes, marchandises) : `rows` lignes.
    Prose,      ///< Bloc de texte suivi (réplique, description de contrat).
    Portrait,   ///< Cadre d'illustration carré (interlocuteur, blason).
    Track,      ///< Suite horizontale de jetons (ordre d'initiative) : `columns` jetons.
    ActionBar,  ///< Barre d'actions : `columns` cases larges, alignées en bas.
};

/// Un bloc de l'ossature d'un écran.
struct RpgContentBlock {
    const char* titleKey = "";  ///< Clé du titre du bloc ; vide pour un bloc sans titre.
    RpgBlockKind kind = RpgBlockKind::Fields;
    int columns = 0;  ///< `Grid`, `Track`, `ActionBar` : nombre de colonnes/jetons.
    int rows = 0;     ///< `Grid`, `List` : nombre de lignes.
    /// `Fields` : une clé de libellé par ligne. Vide pour les autres genres.
    std::span<const char* const> labelKeys{};
};

/// Ossature d'un écran : deux colonnes de blocs. La colonne droite peut être vide -- l'écran
/// occupe alors toute la largeur, ce qui est le cas du dialogue.
struct RpgScreenLayout {
    std::span<const RpgContentBlock> leftColumn{};
    std::span<const RpgContentBlock> rightColumn{};
};

/// Description complète d'un écran du RPG.
struct RpgScreenDescriptor {
    RpgScreenId id = RpgScreenId::CharacterSheet;
    /// `objectName` du widget, **ciblé par le thème** (`theme-identity.qss`). Un nom, jamais un
    /// indice : insérer un écran dans l'énumération ne doit pas déplacer l'habillage d'un autre.
    const char* objectName = "";
    const char* titleKey = "";  ///< Clé du titre de l'écran (catalogue de traduction).
    RpgSuperposition superposition = RpgSuperposition::PausesGame;
    RpgScreenLayout layout{};
};

/// @return La table des écrans, dans l'ordre du cycle de navigation.
[[nodiscard]] std::span<const RpgScreenDescriptor> rpgScreens() noexcept;

/// @return La description de @p screen.
[[nodiscard]] const RpgScreenDescriptor& rpgScreenDescriptor(RpgScreenId screen) noexcept;

/// @return L'écran suivant dans le cycle ; du dernier on revient au premier. C'est ce qui permet
///         d'atteindre n'importe quel écran depuis n'importe quel autre sans repasser par le menu
///         (`EX-IHM-090`).
[[nodiscard]] RpgScreenId nextRpgScreen(RpgScreenId screen) noexcept;

/// @return L'écran précédent dans le cycle ; du premier on revient au dernier.
[[nodiscard]] RpgScreenId previousRpgScreen(RpgScreenId screen) noexcept;

/// @return `true` si @p screen suspend la simulation (`EX-IHM-091`).
[[nodiscard]] bool pausesGame(RpgScreenId screen) noexcept;

}  // namespace hmi
