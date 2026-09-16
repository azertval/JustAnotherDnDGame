// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Core/Combat/TacticalTerrain.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/MapEntity.h"
#include "Core/World/EntityKinds.h"

/**
 * @file HMI/Editor/EditorDiagnostics.h
 * @brief Les avertissements que l'éditeur montre sur les entités d'une carte : références cassées
 *        et rencontres posées hors d'un terrain tactique valide (`LOT-11`).
 */

namespace hmi {

/// @brief La famille d'un avertissement : ce que l'entité référence, ou le terrain qu'elle occupe.
enum class EditorDiagnosticKind {
    Reference,
    Terrain,
};

/**
 * @brief Un avertissement prêt à traduire : une clé de `hmi::Localization` et ses arguments, dans
 *        l'ordre des `%1`, `%2`… du texte.
 *
 * Le `Core` rend des codes (`EX-NFR-011`) ; cette table les nomme, une fois, pour que le panneau et
 * les tests lisent la même chose.
 */
struct EditorDiagnostic {
    EditorDiagnosticKind kind = EditorDiagnosticKind::Reference;
    std::size_t entityIndex = 0;
    /// Case de l'entité, ou case en cause (combattant hors de la zone jouable).
    core::GridPosition cell;
    std::string key;
    std::vector<std::string> args;

    [[nodiscard]] bool operator==(const EditorDiagnostic&) const = default;
};

/// @return La clé de traduction de @p code.
[[nodiscard]] const char* entityIssueKey(core::EntityIssueCode code) noexcept;

/// @return La clé de traduction de @p code.
[[nodiscard]] const char* tacticalIssueKey(core::TacticalIssueCode code) noexcept;

/**
 * @brief Les avertissements de la carte, les références d'abord, puis le terrain, chacun dans
 *        l'ordre des entités.
 *
 * Arguments : pour une référence, le type d'entité, la propriété, puis la valeur ; pour un
 * combattant, la rencontre puis la créature ; pour une zone trop étroite, la rencontre, les cases
 * libres, puis les cases exigées.
 */
[[nodiscard]] std::vector<EditorDiagnostic> editorDiagnostics(
    const std::vector<core::MapEntity>& entities, const std::vector<core::EntityIssue>& issues,
    const std::vector<core::EncounterTerrain>& terrains);

}  // namespace hmi
