// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Presentation/CreditsCatalog.h"

#include <optional>
#include <utility>

#include <nlohmann/json.hpp>

#include "Core/Data/JsonDocument.h"

namespace hmi {
namespace {

constexpr int FORMAT_VERSION = 1;
constexpr const char* SOURCE_LANGUAGE = "fr";

/// Un libellé traduit : la langue demandée, sinon la langue source ; rien si ni l'une ni l'autre.
[[nodiscard]] std::optional<std::string> translated(const nlohmann::json& label,
                                                    std::string_view language) {
    if (!label.is_object()) {
        return std::nullopt;
    }
    for (const std::string& key : {std::string(language), std::string(SOURCE_LANGUAGE)}) {
        const auto found = label.find(key);
        if (found != label.end() && found->is_string() && !found->get<std::string>().empty()) {
            return found->get<std::string>();
        }
    }
    return std::nullopt;
}

[[nodiscard]] CreditsResult failure(std::string message) {
    return CreditsResult{.sections = {}, .error = "credits.json : " + std::move(message)};
}

}  // namespace

CreditsResult readCredits(std::string_view json, std::string_view language) {
    const core::JsonDocument document = core::readJsonObject(json, FORMAT_VERSION, "credits.json");
    if (!document.ok()) {
        return failure(document.message);
    }
    const auto sections = document.root.find("sections");
    if (sections == document.root.end() || !sections->is_array()) {
        return failure("le champ « sections » manque ou n'est pas une liste.");
    }

    CreditsResult result;
    for (const nlohmann::json& sectionJson : *sections) {
        CreditSection section;
        if (!sectionJson.is_object() || !sectionJson.value("id", nlohmann::json()).is_string()) {
            return failure("une section n'a pas d'identifiant.");
        }
        section.id = sectionJson["id"].get<std::string>();
        const nlohmann::json column = sectionJson.value("column", nlohmann::json(0));
        if (!column.is_number_integer() || column.get<int>() < 0 || column.get<int>() > 1) {
            return failure("section « " + section.id + " » : « column » vaut 0 ou 1.");
        }
        section.column = column.get<int>();
        const std::optional<std::string> title =
            translated(sectionJson.value("title", nlohmann::json()), language);
        if (!title) {
            return failure("section « " + section.id + " » sans titre.");
        }
        section.title = *title;

        const nlohmann::json lines = sectionJson.value("lines", nlohmann::json());
        if (!lines.is_array() || lines.empty()) {
            return failure("section « " + section.id + " » sans ligne.");
        }
        for (const nlohmann::json& lineJson : lines) {
            CreditLine line;
            const std::optional<std::string> role =
                lineJson.is_object()
                    ? translated(lineJson.value("role", nlohmann::json()), language)
                    : std::nullopt;
            if (!role) {
                return failure("section « " + section.id + " » : une ligne n'a pas de rôle.");
            }
            line.role = *role;
            const nlohmann::json names = lineJson.value("names", nlohmann::json());
            if (!names.is_array() || names.empty()) {
                return failure("section « " + section.id + " », rôle « " + line.role +
                               " » : aucun nom.");
            }
            for (const nlohmann::json& name : names) {
                // Un nom propre s'écrit tel quel ; une mention (« fan game non officiel ») se
                // traduit comme un rôle.
                std::optional<std::string> text = translated(name, language);
                if (name.is_string() && !name.get<std::string>().empty()) {
                    text = name.get<std::string>();
                }
                if (!text) {
                    return failure("section « " + section.id + " », rôle « " + line.role +
                                   " » : un nom vide.");
                }
                line.names.push_back(std::move(*text));
            }
            section.lines.push_back(std::move(line));
        }
        result.sections.push_back(std::move(section));
    }
    return result;
}

}  // namespace hmi
