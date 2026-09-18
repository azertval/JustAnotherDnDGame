// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstdint>
#include <filesystem>
#include <string_view>

/**
 * @file Editor/Logic/DiskGuard.h
 * @brief Garde de fichier modifié sur disque (`EX-EDIT-057`).
 *
 * Une carte ouverte peut changer sous l'éditeur : un script la régénère, Claude la retouche, un
 * `git checkout` la remplace. L'éditeur retient l'**empreinte** du fichier qu'il a lu ou écrit, et
 * la compare à celle du disque quand le fichier est signalé changé, quand la fenêtre reprend la
 * main et avant chaque enregistrement. Une carte n'est jamais écrasée en silence.
 *
 * L'empreinte porte sur le **contenu**, pas sur la date : un fichier réécrit à l'identique n'est
 * pas un changement, et une date qui recule (restauration) n'en cache pas un.
 */

namespace hmi {

/// Empreinte d'un fichier : existence, taille et condensé du contenu.
struct FileFingerprint {
    bool exists = false;
    std::uintmax_t size = 0;
    std::uint64_t hash = 0;

    [[nodiscard]] bool operator==(const FileFingerprint&) const = default;
};

/// @return L'empreinte de @p content, comme s'il était le contenu d'un fichier existant.
[[nodiscard]] FileFingerprint fingerprintOf(std::string_view content) noexcept;

/// @return L'empreinte du fichier @p path ; `exists == false` s'il est absent ou illisible.
[[nodiscard]] FileFingerprint fingerprintFile(const std::filesystem::path& path);

/// Ce qui a changé sur disque depuis la dernière lecture ou écriture de l'éditeur.
enum class DiskChange {
    None,      ///< Même contenu.
    Modified,  ///< Le fichier existe et son contenu diffère (ou il est apparu).
    Deleted,   ///< Le fichier a disparu.
};

/// @return Le changement entre l'empreinte @p known et l'empreinte @p current.
[[nodiscard]] DiskChange compareFingerprints(const FileFingerprint& known,
                                             const FileFingerprint& current) noexcept;

/// Ce que l'éditeur fait d'un changement sur disque.
enum class DiskReaction {
    Ignore,           ///< Rien n'a changé.
    ReloadQuietly,    ///< Brouillon intact : relire le fichier, rien n'est perdu.
    AskReloadOrKeep,  ///< Brouillon modifié : l'auteur choisit, l'autre version est mise de côté.
    WarnDeleted,      ///< Fichier disparu : le brouillon reste, l'enregistrement le recréera.
};

/// @return La réaction à @p change selon que le brouillon porte des modifications (@p
/// draftModified).
[[nodiscard]] DiskReaction reactToDiskChange(DiskChange change, bool draftModified) noexcept;

}  // namespace hmi
