// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Editor/TaxonomyLabels.h"

#include <unordered_map>

#include "HMI/Localization/Localization.h"

namespace hmi {

std::string taxonomyLabelKey(const std::string& label) {
    static const std::unordered_map<std::string, std::string> keys = {
        {"Tuile", "palette.cat.tile"},
        {"Interactif", "palette.cat.interactive"},
        {"Piège", "palette.cat.trap"},
        {"Jalon", "palette.cat.marker"},
        {"Bloc poussable", "palette.tile.block"},
        {"Vide (gomme)", "palette.tile.empty"},
        {"Plein", "palette.tile.solid"},
        {"Interrupteur", "palette.tile.switch"},
        {"Plaque de pression", "palette.tile.plate"},
        {"Porte", "palette.tile.door"},
        {"Clé", "palette.tile.key"},
        {"Porte verrouillée", "palette.tile.locked_door"},
        {"Danger", "palette.tile.danger"},
        {"Entrée", "palette.tile.entry"},
        {"Sortie", "palette.tile.exit"},
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
