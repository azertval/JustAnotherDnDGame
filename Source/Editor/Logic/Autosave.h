// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

/**
 * @file Editor/Logic/Autosave.h
 * @brief Sauvegarde automatique des brouillons et reprise après plantage (`EX-EDIT-056`).
 *
 * L'éditeur écrit le brouillon d'une carte modifiée dans un dossier du poste (`%LOCALAPPDATA%`),
 * jamais à côté de la carte : une sauvegarde automatique n'est pas un enregistrement, et le dossier
 * des cartes est versionné. Un enregistrement, ou une fermeture qui abandonne les modifications,
 * retire le fichier. **Un fichier qui reste au démarrage est donc la trace d'une session qui ne
 * s'est pas terminée** : l'éditeur propose de le reprendre.
 *
 * Logique pure, sans Qt : la fenêtre choisit le dossier et le moment, ce module écrit et relit.
 */

namespace hmi {

/// Un brouillon sauvegardé automatiquement.
struct AutosaveRecord {
    /// Identifiant de la carte (`capital/martpart`) : c'est lui qui nomme le fichier.
    std::string mapId;
    /// Fichier de la carte que le brouillon remplacera à l'enregistrement.
    std::filesystem::path levelPath;
    /// Le brouillon, tel que `core::LevelDraft::toJson` le rend (non validé : un brouillon
    /// incomplet se sauvegarde aussi).
    std::string draftJson;

    [[nodiscard]] bool operator==(const AutosaveRecord&) const = default;
};

/// Version du format des fichiers de reprise. Un fichier d'une autre version est ignoré, jamais
/// effacé : il reste lisible à la main.
inline constexpr int AUTOSAVE_FORMAT = 1;

/// @return Le nom de fichier de reprise de @p mapId : sous-dossiers aplatis (`capital~martpart`),
///         suffixe `.autosave.json`. Un identifiant vide donne `untitled`.
[[nodiscard]] std::string autosaveFileName(std::string_view mapId);

/// @return Le contenu d'un fichier de reprise pour @p record (JSON, format `AUTOSAVE_FORMAT`).
[[nodiscard]] std::string serializeAutosave(const AutosaveRecord& record);

/// @return Le brouillon porté par @p text, ou `std::nullopt` si le texte n'est pas un fichier de
///         reprise lisible de ce format.
[[nodiscard]] std::optional<AutosaveRecord> parseAutosave(std::string_view text);

/**
 * @brief Le dossier des brouillons sauvegardés automatiquement.
 *
 * Chaque écriture passe par un fichier temporaire renommé ensuite : un plantage pendant l'écriture
 * laisse l'ancien brouillon intact, jamais un fichier tronqué.
 */
class AutosaveStore {
public:
    /// @param directory Dossier des fichiers de reprise ; créé à la première écriture.
    explicit AutosaveStore(std::filesystem::path directory);

    [[nodiscard]] const std::filesystem::path& directory() const noexcept {
        return _directory;
    }

    /// @return Le fichier de reprise de @p mapId.
    [[nodiscard]] std::filesystem::path pathFor(std::string_view mapId) const;

    /// Écrit (ou remplace) le brouillon de @p record.
    /// @return `false` si le fichier n'a pas pu être écrit ; l'ancien reste alors en place.
    [[nodiscard]] bool write(const AutosaveRecord& record) const;

    /// @return Les brouillons en attente de reprise, triés par identifiant de carte. Un fichier
    ///         illisible est ignoré (et laissé en place).
    [[nodiscard]] std::vector<AutosaveRecord> pending() const;

    /// Retire le brouillon de @p mapId, s'il y en a un.
    void discard(std::string_view mapId) const;

    /**
     * @brief Met de côté une version d'une carte qu'un choix de l'auteur va écarter : le fichier
     *        changé sur disque qu'il garde de côté, ou le brouillon qu'il abandonne pour recharger.
     *
     * Rien n'est jamais perdu : la copie reste sous `conflicts/` jusqu'à ce que l'auteur la retire.
     * @param mapId   Carte concernée.
     * @param label   Nature de la copie (`disk`, `draft`), dans le nom du fichier.
     * @param stamp   Horodatage du nom (`20260918-142501`), fourni par l'appelant.
     * @param content Contenu à conserver.
     * @return Le chemin écrit, ou `std::nullopt` en cas d'échec.
     */
    [[nodiscard]] std::optional<std::filesystem::path> keepAside(std::string_view mapId,
                                                                 std::string_view label,
                                                                 std::string_view stamp,
                                                                 std::string_view content) const;

private:
    std::filesystem::path _directory;
};

}  // namespace hmi
