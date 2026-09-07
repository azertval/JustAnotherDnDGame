// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/RpgScreens.h"

#include <array>

namespace hmi {
namespace {

// --- La FICHE DE PERSONNAGE n'a pas d'ossature : elle a une PLANCHE (LOT-38) ------------------
//
// Ses champs, leur place et leurs intitulés vivent dans la table
// `Source/Elements/Assets/UI/character-sheet-plate.json`, relevée sur la GRAVURE de la planche du
// livre — la même table qui a servi à retirer de cette gravure son lettrage anglais, pour que
// l'écran puisse le reposer traduit.
//
// Rien n'est donc décrit deux fois. Une ossature laissée ici « pour mémoire » aurait été une
// seconde description du même écran, et c'est toujours la copie oubliée qu'on lit six mois plus
// tard.

// --- Ossature des sept autres écrans (LOT-68) -------------------------------------------------
//
// Aucun ne porte de DONNÉE : ce lot-là livrait le châssis, pas le contenu (LOT-42 pour la carte,
// LOT-45 pour la guilde, LOT-24 pour le combat). Leurs identifiants de valeur restent donc vides,
// et leurs cases au tiret cadratin.

constexpr std::array LOAD_FIELDS = {
    RpgField{.labelKey = "rpg.field.carried"},
    RpgField{.labelKey = "rpg.field.capacity"},
    RpgField{.labelKey = "rpg.field.gold"},
};

constexpr std::array EQUIPMENT_SLOTS = {
    RpgField{.labelKey = "rpg.slot.head"},      RpgField{.labelKey = "rpg.slot.torso"},
    RpgField{.labelKey = "rpg.slot.hands"},     RpgField{.labelKey = "rpg.slot.feet"},
    RpgField{.labelKey = "rpg.slot.main_hand"}, RpgField{.labelKey = "rpg.slot.off_hand"},
    RpgField{.labelKey = "rpg.slot.amulet"},    RpgField{.labelKey = "rpg.slot.ring"},
};

constexpr std::array PLACE_FIELDS = {
    RpgField{.labelKey = "rpg.field.region"},
    RpgField{.labelKey = "rpg.field.place_type"},
    RpgField{.labelKey = "rpg.field.danger"},
};

constexpr std::array SPEAKER_FIELDS = {
    RpgField{.labelKey = "rpg.field.name"},
    RpgField{.labelKey = "rpg.field.attitude"},
};

constexpr std::array PURSE_FIELDS = {RpgField{.labelKey = "rpg.field.gold"}};

constexpr std::array CONTRACT_FIELDS = {
    RpgField{.labelKey = "rpg.field.giver"},
    RpgField{.labelKey = "rpg.field.rank"},
    RpgField{.labelKey = "rpg.field.reward"},
};

constexpr std::array TARGET_FIELDS = {
    RpgField{.labelKey = "rpg.field.name"},
    RpgField{.labelKey = "rpg.field.hit_points"},
    RpgField{.labelKey = "rpg.field.armor_class"},
    RpgField{.labelKey = "rpg.field.conditions"},
};

constexpr std::array INVENTORY_LEFT = {
    RpgContentBlock{
        .titleKey = "rpg.block.equipment", .kind = RpgBlockKind::Fields, .fields = EQUIPMENT_SLOTS},
    RpgContentBlock{
        .titleKey = "rpg.block.load", .kind = RpgBlockKind::Fields, .fields = LOAD_FIELDS},
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
        .titleKey = "rpg.block.place", .kind = RpgBlockKind::Fields, .fields = PLACE_FIELDS},
};
constexpr std::array WORLD_MAP_RIGHT = {
    RpgContentBlock{.titleKey = "rpg.block.map", .kind = RpgBlockKind::Portrait},
};

constexpr std::array DIALOGUE_LEFT = {
    RpgContentBlock{.titleKey = "rpg.block.speaker", .kind = RpgBlockKind::Portrait},
    RpgContentBlock{.titleKey = "", .kind = RpgBlockKind::Fields, .fields = SPEAKER_FIELDS},
};
constexpr std::array DIALOGUE_RIGHT = {
    RpgContentBlock{.titleKey = "rpg.block.line", .kind = RpgBlockKind::Prose},
    RpgContentBlock{.titleKey = "rpg.block.replies", .kind = RpgBlockKind::List, .rows = 4},
};

constexpr std::array MERCHANT_LEFT = {
    RpgContentBlock{.titleKey = "rpg.block.goods", .kind = RpgBlockKind::List, .rows = 8},
    RpgContentBlock{
        .titleKey = "rpg.block.purse", .kind = RpgBlockKind::Fields, .fields = PURSE_FIELDS},
};
constexpr std::array MERCHANT_RIGHT = {
    RpgContentBlock{.titleKey = "rpg.block.your_bag", .kind = RpgBlockKind::List, .rows = 8},
};

constexpr std::array GUILD_BOARD_LEFT = {
    RpgContentBlock{.titleKey = "rpg.block.contracts", .kind = RpgBlockKind::List, .rows = 6},
};
constexpr std::array GUILD_BOARD_RIGHT = {
    RpgContentBlock{.titleKey = "rpg.block.contract", .kind = RpgBlockKind::Prose},
    RpgContentBlock{.titleKey = "", .kind = RpgBlockKind::Fields, .fields = CONTRACT_FIELDS},
};

constexpr std::array COMBAT_HUD_LEFT = {
    RpgContentBlock{.titleKey = "rpg.block.initiative", .kind = RpgBlockKind::Track, .columns = 6},
    RpgContentBlock{
        .titleKey = "rpg.block.target", .kind = RpgBlockKind::Fields, .fields = TARGET_FIELDS},
    RpgContentBlock{.titleKey = "rpg.block.actions", .kind = RpgBlockKind::ActionBar, .columns = 6},
};

// --- Ossature de la FEUILLE D'ÉQUIPE (planche 5, LOT-38) --------------------------------------
//
// La cinquième planche n'est pas la fiche d'un personnage : c'est celle de son ÉQUIPE — renommée,
// blason, quartier général, mécénat. La ranger dans la fiche aurait mêlé deux sujets sur un même
// écran ; elle en a donc un à elle.
//
// Cet écran est aussi la preuve de ce que le `LOT-68` affirmait : il s'ajoute par une entrée de
// table et ses clés de traduction, sans qu'aucun des huit autres, ni la feuille de style, ni le
// châssis, n'aient été touchés (`EX-IHM-090`).

constexpr std::array TEAM_FIELDS = {
    RpgField{.labelKey = "rpg.field.team_name"},
    RpgField{.labelKey = "rpg.field.career_points"},
    RpgField{.labelKey = "rpg.field.fame"},
    RpgField{.labelKey = "rpg.field.prestige"},
    RpgField{.labelKey = "rpg.field.beneficiary"},
    RpgField{.labelKey = "rpg.field.style"},
    RpgField{.labelKey = "rpg.field.specialization"},
};

constexpr std::array TEAM_SHEET_LEFT = {
    RpgContentBlock{
        .titleKey = "rpg.block.team", .kind = RpgBlockKind::Fields, .fields = TEAM_FIELDS},
    RpgContentBlock{.titleKey = "rpg.block.team_members", .kind = RpgBlockKind::List, .rows = 4},
    RpgContentBlock{.titleKey = "rpg.block.relations", .kind = RpgBlockKind::Prose},
};
constexpr std::array TEAM_SHEET_RIGHT = {
    RpgContentBlock{.titleKey = "rpg.block.coat_of_arms", .kind = RpgBlockKind::Portrait},
    RpgContentBlock{.titleKey = "rpg.block.dream", .kind = RpgBlockKind::Prose},
    RpgContentBlock{.titleKey = "rpg.block.hidden_agenda", .kind = RpgBlockKind::Prose},
    RpgContentBlock{.titleKey = "rpg.block.legendary_rewards", .kind = RpgBlockKind::Prose},
    // Huit installations au quartier général de la planche.
    RpgContentBlock{.titleKey = "rpg.block.headquarters", .kind = RpgBlockKind::List, .rows = 8},
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
     .rendering = RpgRendering::Plate},
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
    // Le neuvième (LOT-38) : une équipe se consulte à l'arrêt, comme une fiche.
    {.id = RpgScreenId::TeamSheet,
     .objectName = "RpgTeamSheetScreen",
     .titleKey = "rpg.team_sheet.title",
     .superposition = RpgSuperposition::PausesGame,
     .layout = {.leftColumn = TEAM_SHEET_LEFT, .rightColumn = TEAM_SHEET_RIGHT}},
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
