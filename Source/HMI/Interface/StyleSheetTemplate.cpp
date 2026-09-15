// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Interface/StyleSheetTemplate.h"

namespace hmi {

StyleSheetSubstitutionResult substituteStyleSheetTemplate(
    const std::string& templateText, const std::unordered_map<std::string, std::string>& values) {
    StyleSheetSubstitutionResult result;
    result.text.reserve(templateText.size());

    std::size_t cursor = 0;
    while (cursor < templateText.size()) {
        const std::size_t markerStart = templateText.find("${", cursor);
        if (markerStart == std::string::npos) {
            result.text.append(templateText, cursor, std::string::npos);
            break;
        }
        result.text.append(templateText, cursor, markerStart - cursor);

        const std::size_t markerEnd = templateText.find('}', markerStart + 2);
        if (markerEnd == std::string::npos) {
            result.ok = false;
            result.error = "marqueur non ferme (aucun '}' apres la position " +
                           std::to_string(markerStart) + ")";
            return result;
        }

        const std::string name = templateText.substr(markerStart + 2, markerEnd - markerStart - 2);
        const auto found = values.find(name);
        if (found == values.end()) {
            result.ok = false;
            result.error = "marqueur inconnu : ${" + name + "}";
            return result;
        }
        result.text += found->second;
        cursor = markerEnd + 1;
    }

    result.ok = true;
    return result;
}

}  // namespace hmi
