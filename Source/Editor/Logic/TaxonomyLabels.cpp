// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Editor/Logic/TaxonomyLabels.h"

#include <unordered_map>

#include "HMI/Localization/Localization.h"

namespace hmi {

std::string taxonomyLabelKey(const std::string& label) {
    static const std::unordered_map<std::string, std::string> keys = {
        {"Tuile", "palette.cat.tile"},
        {"Jalon", "palette.cat.marker"},
        {"Vide (gomme)", "palette.tile.empty"},
        {"Plein", "palette.tile.solid"},
        {"Entrée", "palette.tile.entry"},
        {"Sol", "palette.cat.ground"},
        {"Obstacle", "palette.cat.obstacle"},
        {"Passage", "palette.cat.crossing"},
        {"Herbe", "palette.tile.grass"},
        {"Terre", "palette.tile.dirt"},
        {"Sable", "palette.tile.sand"},
        {"Eau", "palette.tile.water"},
        {"Eau profonde", "palette.tile.deep_water"},
        {"Mur", "palette.tile.wall"},
        {"Falaise", "palette.tile.cliff"},
        {"Pont", "palette.tile.bridge"},
        {"Escalier", "palette.tile.stairs"},
    };

    const auto found = keys.find(label);
    return found == keys.end() ? std::string{} : found->second;
}

std::string localizedTaxonomyLabel(const Localization* loc, const std::string& label) {
    if (loc == nullptr) {
        return label;
    }
    const std::string key = taxonomyLabelKey(label);
    if (key.empty()) {
        return label;  // repli : libelle source (francais).
    }
    return loc->text(key);
}

}  // namespace hmi
