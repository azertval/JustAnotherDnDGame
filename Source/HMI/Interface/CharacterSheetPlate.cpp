// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/CharacterSheetPlate.h"

#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

namespace hmi {
namespace {

/// Les rôles, tels que la table les nomme. Un rôle inconnu retombe sur `Band` plutôt que de faire
/// échouer la lecture : un champ mal étiqueté s'affiche de travers, ce qui se voit et se corrige ;
/// une planche entière refusée ne se voit que comme un écran vide.
[[nodiscard]] PlateRole roleFromName(const std::string& name) {
    static const std::unordered_map<std::string, PlateRole> ROLES = {
        {"name", PlateRole::Name},       {"band", PlateRole::Band},
        {"dial", PlateRole::Dial},       {"plaque", PlateRole::Plaque},
        {"ability", PlateRole::Ability}, {"score", PlateRole::Score},
        {"skill", PlateRole::Skill},     {"save", PlateRole::Save},
        {"banner", PlateRole::Banner},   {"prose", PlateRole::Prose},
        {"table", PlateRole::Table},     {"portrait", PlateRole::Portrait},
    };
    const auto found = ROLES.find(name);
    return found == ROLES.end() ? PlateRole::Band : found->second;
}

/// La table écrit un rectangle `[x0, y0, x1, y1]` — deux COINS, comme les relevés sur la maquette.
/// La structure, elle, porte une origine et une taille : c'est ce dont le placement a besoin, et
/// convertir une fois ici évite de le refaire à chaque peinture.
[[nodiscard]] PlateRect toRect(const nlohmann::json& node) {
    if (!node.is_array() || node.size() != 4) {
        return {};
    }
    const double x0 = node[0].get<double>();
    const double y0 = node[1].get<double>();
    const double x1 = node[2].get<double>();
    const double y1 = node[3].get<double>();
    return {.x = x0, .y = y0, .width = x1 - x0, .height = y1 - y0};
}

}  // namespace

CharacterSheetPlateLayout parseCharacterSheetPlate(const std::string& json) {
    CharacterSheetPlateLayout layout;
    const nlohmann::json document = nlohmann::json::parse(json, nullptr, false);
    if (document.is_discarded() || !document.is_object()) {
        return layout;
    }

    if (const auto page = document.find("page");
        page != document.end() && page->is_array() && page->size() == 2) {
        layout.pageWidth = (*page)[0].get<double>();
        layout.pageHeight = (*page)[1].get<double>();
    }

    const auto fields = document.find("fields");
    if (fields == document.end() || !fields->is_array()) {
        return layout;
    }

    for (const nlohmann::json& node : *fields) {
        if (!node.is_object()) {
            continue;
        }
        PlateField field;
        field.id = node.value("id", std::string{});
        if (field.id.empty()) {
            continue;
        }
        field.labelKey = node.value("labelKey", std::string{});
        field.valueId = node.value("valueId", std::string{});
        field.role = roleFromName(node.value("role", std::string{"band"}));
        field.align = node.value("align", std::string{"center"}) == "left" ? PlateAlign::Left
                                                                           : PlateAlign::Center;
        if (const auto label = node.find("label"); label != node.end()) {
            field.label = toRect(*label);
        }
        if (const auto value = node.find("value"); value != node.end()) {
            field.value = toRect(*value);
        }
        if (const auto cover = node.find("cover"); cover != node.end() && cover->is_array()) {
            for (const nlohmann::json& rectangle : *cover) {
                const PlateRect converted = toRect(rectangle);
                if (!converted.isEmpty()) {
                    field.cover.push_back(converted);
                }
            }
        }
        layout.fields.push_back(std::move(field));
    }
    return layout;
}

}  // namespace hmi
