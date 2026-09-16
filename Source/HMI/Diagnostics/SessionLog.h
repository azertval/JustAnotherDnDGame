// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "Core/Diagnostics/MemoryLogSink.h"

/**
 * @file HMI/Diagnostics/SessionLog.h
 * @brief Sérialisation et enregistrement des messages de log de la session.
 */

namespace hmi {

/**
 * @brief Assemble les messages de log en un texte (une ligne par message).
 * @param entries Messages accumulés (chacun déjà formaté par le journaliseur).
 * @return Le texte complet, terminé par un saut de ligne (vide si aucun message).
 */
[[nodiscard]] std::string serializeSessionLog(
    const std::vector<core::MemoryLogSink::Entry>& entries);

/**
 * @brief Enregistre les messages de log de la session dans un fichier.
 * @param entries Messages accumulés.
 * @param path    Chemin du fichier de destination (les dossiers parents sont créés).
 * @return true si le fichier a été écrit ; false (récupérable) en cas d'échec d'écriture.
 */
[[nodiscard]] bool saveSessionLog(const std::vector<core::MemoryLogSink::Entry>& entries,
                                  const std::filesystem::path& path);

}  // namespace hmi
