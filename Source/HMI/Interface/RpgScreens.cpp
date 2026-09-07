// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/RpgScreens.h"

#include <array>

namespace hmi {
namespace {

// --- Libellés des blocs « Fields » ---------------------------------------------------------
//
// Les lignes de ces blocs ne sont pas des exemples : elles sont relevées sur les MODÈLES DÉJÀ
// LIVRÉS -- `core::CharacterSheet` (LOT-13) pour la fiche, `core::Ability` (LOT-12) pour les six
// caractéristiques, `core::Equipment` (LOT-34) pour les emplacements. Une ossature qui annonce des
// champs que le modèle ne porte pas promet ce que le jeu ne pourra pas tenir.

constexpr std::array IDENTITY_FIELDS = {"rpg.field.name", "rpg.field.species", "rpg.field.class",
                                        "rpg.field.background"};

constexpr std::array COMBAT_FIELDS = {"rpg.field.hit_points", "rpg.field.armor_class",
                                      "rpg.field.initiative", "rpg.field.speed",
                                      "rpg.field.proficiency_bonus"};

// Le NIVEAU d'un personnage et son expérience (LOT-74). À ne pas confondre avec le « niveau »
// retiré au LOT-67 : celui-là était un TABLEAU de jeu de plateforme, et c'est cette notion-là,
// discrète et ordonnée, qui a quitté le programme. Un personnage, lui, progresse.
constexpr std::array PROGRESSION_FIELDS = {"rpg.field.level", "rpg.field.experience"};

constexpr std::array EQUIPMENT_SLOTS = {
    "rpg.slot.head",      "rpg.slot.torso",    "rpg.slot.hands",  "rpg.slot.feet",
    "rpg.slot.main_hand", "rpg.slot.off_hand", "rpg.slot.amulet", "rpg.slot.ring"};

constexpr std::array LOAD_FIELDS = {"rpg.field.carried", "rpg.field.capacity", "rpg.field.gold"};

constexpr std::array PLACE_FIELDS = {"rpg.field.region", "rpg.field.place_type",
                                     "rpg.field.danger"};

constexpr std::array SPEAKER_FIELDS = {"rpg.field.name", "rpg.field.attitude"};

constexpr std::array PURSE_FIELDS = {"rpg.field.gold"};

constexpr std::array CONTRACT_FIELDS = {"rpg.field.giver", "rpg.field.rank", "rpg.field.reward"};

constexpr std::array TARGET_FIELDS = {"rpg.field.name", "rpg.field.hit_points",
                                      "rpg.field.armor_class", "rpg.field.conditions"};

// --- Ossature de chacun des huit écrans ------------------------------------------------------
//
// Aucun de ces blocs ne porte de DONNÉE : ce lot livre le châssis, pas le contenu (LOT-38 pour la
// fiche, LOT-42 pour la carte, LOT-45 pour la guilde, LOT-24 pour le combat). Les cases et les
// lignes sont donc vides, et le restent -- une valeur d'exemple posée ici se lirait comme un état
// du jeu et mentirait à la première lecture (EX-IHM-072).

constexpr std::array CHARACTER_SHEET_LEFT = {
    RpgContentBlock{.titleKey = "rpg.block.identity",
                    .kind = RpgBlockKind::Fields,
                    .labelKeys = IDENTITY_FIELDS},
    RpgContentBlock{.titleKey = "rpg.block.progression",
                    .kind = RpgBlockKind::Fields,
                    .labelKeys = PROGRESSION_FIELDS},
    // Six cases sur deux rangs de trois : la disposition des feuilles du corpus.
    RpgContentBlock{
        .titleKey = "rpg.block.abilities", .kind = RpgBlockKind::Grid, .columns = 3, .rows = 2},
};
constexpr std::array CHARACTER_SHEET_RIGHT = {
    RpgContentBlock{
        .titleKey = "rpg.block.combat", .kind = RpgBlockKind::Fields, .labelKeys = COMBAT_FIELDS},
    // Six jets de sauvegarde, un par caractéristique (core::CharacterSheet::savingThrows).
    RpgContentBlock{.titleKey = "rpg.block.saving_throws", .kind = RpgBlockKind::List, .rows = 6},
    // Le catalogue en porte dix-huit (LOT-43) ; l'ossature en montre huit et défile.
    RpgContentBlock{.titleKey = "rpg.block.skills", .kind = RpgBlockKind::List, .rows = 8},
};

constexpr std::array INVENTORY_LEFT = {
    RpgContentBlock{.titleKey = "rpg.block.equipment",
                    .kind = RpgBlockKind::Fields,
                    .labelKeys = EQUIPMENT_SLOTS},
    RpgContentBlock{
        .titleKey = "rpg.block.load", .kind = RpgBlockKind::Fields, .labelKeys = LOAD_FIELDS},
};
constexpr std::array INVENTORY_RIGHT = {
    RpgContentBlock{
        .titleKey = "rpg.block.bag", .kind = RpgBlockKind::Grid, .columns = 6, .rows = 5},
};

constexpr std::array JOURNAL_LEFT = {
    RpgContentBlock{.titleKey = "rpg.block.quests", .kind = RpgBlockKind::List, .rows = 8},
};
constexpr std::array JOURNAL_RIGHT = {
    RpgContentBlock{.titleKey = "rpg.block.quest_detail", .kind = RpgBlockKind::Prose},
    RpgContentBlock{.titleKey = "rpg.block.objectives", .kind = RpgBlockKind::List, .rows = 4},
};

constexpr std::array WORLD_MAP_LEFT = {
    // Treize régions à l'atlas (LOT-37) ; la liste en montre six et défile.
    RpgContentBlock{.titleKey = "rpg.block.regions", .kind = RpgBlockKind::List, .rows = 6},
    RpgContentBlock{
        .titleKey = "rpg.block.place", .kind = RpgBlockKind::Fields, .labelKeys = PLACE_FIELDS},
};
constexpr std::array WORLD_MAP_RIGHT = {
    RpgContentBlock{.titleKey = "rpg.block.map", .kind = RpgBlockKind::Portrait},
};

constexpr std::array DIALOGUE_LEFT = {
    RpgContentBlock{.titleKey = "rpg.block.speaker", .kind = RpgBlockKind::Portrait},
    RpgContentBlock{.titleKey = "", .kind = RpgBlockKind::Fields, .labelKeys = SPEAKER_FIELDS},
};
constexpr std::array DIALOGUE_RIGHT = {
    RpgContentBlock{.titleKey = "rpg.block.line", .kind = RpgBlockKind::Prose},
    RpgContentBlock{.titleKey = "rpg.block.replies", .kind = RpgBlockKind::List, .rows = 4},
};

constexpr std::array MERCHANT_LEFT = {
    RpgContentBlock{.titleKey = "rpg.block.goods", .kind = RpgBlockKind::List, .rows = 8},
    RpgContentBlock{
        .titleKey = "rpg.block.purse", .kind = RpgBlockKind::Fields, .labelKeys = PURSE_FIELDS},
};
constexpr std::array MERCHANT_RIGHT = {
    RpgContentBlock{.titleKey = "rpg.block.your_bag", .kind = RpgBlockKind::List, .rows = 8},
};

constexpr std::array GUILD_BOARD_LEFT = {
    RpgContentBlock{.titleKey = "rpg.block.contracts", .kind = RpgBlockKind::List, .rows = 6},
};
constexpr std::array GUILD_BOARD_RIGHT = {
    RpgContentBlock{.titleKey = "rpg.block.contract", .kind = RpgBlockKind::Prose},
    RpgContentBlock{.titleKey = "", .kind = RpgBlockKind::Fields, .labelKeys = CONTRACT_FIELDS},
};

constexpr std::array COMBAT_HUD_LEFT = {
    RpgContentBlock{.titleKey = "rpg.block.initiative", .kind = RpgBlockKind::Track, .columns = 6},
    RpgContentBlock{
        .titleKey = "rpg.block.target", .kind = RpgBlockKind::Fields, .labelKeys = TARGET_FIELDS},
    RpgContentBlock{.titleKey = "rpg.block.actions", .kind = RpgBlockKind::ActionBar, .columns = 6},
};

// --- La table ---------------------------------------------------------------------------------
//
// C'est ELLE, et rien d'autre, qu'un neuvième écran vient allonger (EX-IHM-090) : le châssis Qt ne
// connaît aucun écran par son nom, il peint ce que cette table décrit.
constexpr std::array<RpgScreenDescriptor, RPG_SCREEN_COUNT> SCREENS = {{
    {.id = RpgScreenId::CharacterSheet,
     .objectName = "RpgCharacterSheetScreen",
     .titleKey = "rpg.character_sheet.title",
     .superposition = RpgSuperposition::PausesGame,
     .layout = {.leftColumn = CHARACTER_SHEET_LEFT, .rightColumn = CHARACTER_SHEET_RIGHT}},
    {.id = RpgScreenId::Inventory,
     .objectName = "RpgInventoryScreen",
     .titleKey = "rpg.inventory.title",
     .superposition = RpgSuperposition::PausesGame,
     .layout = {.leftColumn = INVENTORY_LEFT, .rightColumn = INVENTORY_RIGHT}},
    {.id = RpgScreenId::QuestJournal,
     .objectName = "RpgJournalScreen",
     .titleKey = "rpg.journal.title",
     .superposition = RpgSuperposition::PausesGame,
     .layout = {.leftColumn = JOURNAL_LEFT, .rightColumn = JOURNAL_RIGHT}},
    // La carte se consulte EN MARCHANT : c'est ce pour quoi on l'ouvre -- savoir où l'on va sans
    // s'arrêter. Suspendre la simulation en ferait un écran de bilan.
    {.id = RpgScreenId::WorldMap,
     .objectName = "RpgWorldMapScreen",
     .titleKey = "rpg.world_map.title",
     .superposition = RpgSuperposition::WhileWalking,
     .layout = {.leftColumn = WORLD_MAP_LEFT, .rightColumn = WORLD_MAP_RIGHT}},
    // Un dialogue suspend : l'interlocuteur attend une réponse, il ne la reçoit pas en courant.
    {.id = RpgScreenId::Dialogue,
     .objectName = "RpgDialogueScreen",
     .titleKey = "rpg.dialogue.title",
     .superposition = RpgSuperposition::PausesGame,
     .layout = {.leftColumn = DIALOGUE_LEFT, .rightColumn = DIALOGUE_RIGHT}},
    {.id = RpgScreenId::Merchant,
     .objectName = "RpgMerchantScreen",
     .titleKey = "rpg.merchant.title",
     .superposition = RpgSuperposition::PausesGame,
     .layout = {.leftColumn = MERCHANT_LEFT, .rightColumn = MERCHANT_RIGHT}},
    {.id = RpgScreenId::GuildBoard,
     .objectName = "RpgGuildBoardScreen",
     .titleKey = "rpg.guild_board.title",
     .superposition = RpgSuperposition::PausesGame,
     .layout = {.leftColumn = GUILD_BOARD_LEFT, .rightColumn = GUILD_BOARD_RIGHT}},
    // L'ATH de combat est le seul écran qui ne s'ouvre PAS par-dessus le jeu : il EST le jeu
    // pendant un combat. Il ne suspend donc rien, et le tour par tour du LOT-20 décidera de son
    // rythme -- pas cette table.
    {.id = RpgScreenId::CombatHud,
     .objectName = "RpgCombatHudScreen",
     .titleKey = "rpg.combat_hud.title",
     .superposition = RpgSuperposition::WhileWalking,
     .layout = {.leftColumn = COMBAT_HUD_LEFT}},
}};

/// @return Le rang de @p screen dans la table.
[[nodiscard]] std::size_t indexOf(RpgScreenId screen) noexcept {
    for (std::size_t rank = 0; rank < SCREENS.size(); ++rank) {
        if (SCREENS[rank].id == screen) {
            return rank;
        }
    }
    return 0;
}

}  // namespace

std::span<const RpgScreenDescriptor> rpgScreens() noexcept {
    return SCREENS;
}

const RpgScreenDescriptor& rpgScreenDescriptor(RpgScreenId screen) noexcept {
    return SCREENS[indexOf(screen)];
}

RpgScreenId nextRpgScreen(RpgScreenId screen) noexcept {
    return SCREENS[(indexOf(screen) + 1) % SCREENS.size()].id;
}

RpgScreenId previousRpgScreen(RpgScreenId screen) noexcept {
    return SCREENS[(indexOf(screen) + SCREENS.size() - 1) % SCREENS.size()].id;
}

bool pausesGame(RpgScreenId screen) noexcept {
    return rpgScreenDescriptor(screen).superposition == RpgSuperposition::PausesGame;
}

}  // namespace hmi
