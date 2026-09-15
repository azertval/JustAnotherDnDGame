// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/AssetReferences.h"

#include <map>

#include "Core/Levels/TileTypeName.h"

namespace hmi {

std::vector<AssetReference> findSkinCatalogReferences(const SkinCatalog& catalog,
                                                      const std::string& fileName) {
    std::vector<AssetReference> references;
    for (const std::string& setName : catalog.setNames()) {
        for (const auto& [type, entry] : catalog.assignments(setName)) {
            // Comparaison exacte : un nom proche mais different (ex. « mur.png » vs
            // « mur2.png ») ne doit jamais declencher un avertissement injustifie.
            if (entry.asset == fileName) {
                references.push_back(
                    AssetReference{.setName = setName, .typeName = core::tileTypeName(type)});
            }
        }
    }
    return references;
}

std::string describeReferences(const std::vector<AssetReference>& references) {
    if (references.empty()) {
        return {};
    }
    // Regroupe par jeu pour un message compact : « foret : solid, bloc ; grotte : solid ».
    std::map<std::string, std::vector<std::string>> bySet;
    for (const AssetReference& reference : references) {
        bySet[reference.setName].push_back(reference.typeName);
    }

    std::string message;
    for (const auto& [setName, typeNames] : bySet) {
        if (!message.empty()) {
            message += " ; ";
        }
        message += setName + " : ";
        for (std::size_t index = 0; index < typeNames.size(); ++index) {
            if (index > 0) {
                message += ", ";
            }
            message += typeNames[index];
        }
    }
    return message;
}

}  // namespace hmi
