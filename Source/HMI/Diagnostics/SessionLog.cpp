// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Diagnostics/SessionLog.h"

#include <fstream>
#include <system_error>

namespace hmi {

// Assemble les messages de log en un texte (une ligne par message).
std::string serializeSessionLog(const std::vector<core::MemoryLogSink::Entry>& entries) {
    std::string text;
    for (const core::MemoryLogSink::Entry& entry : entries) {
        text += entry.message;
        text += '\n';
    }
    return text;
}

// Enregistre les messages de log de la session dans un fichier.
bool saveSessionLog(const std::vector<core::MemoryLogSink::Entry>& entries,
                    const std::filesystem::path& path) {
    // Crée les dossiers parents si besoin (échec traité comme une erreur récupérable).
    std::error_code error;
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), error);
        if (error) {
            return false;
        }
    }

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) {
        return false;
    }
    file << serializeSessionLog(entries);
    return file.good();
}

}  // namespace hmi
