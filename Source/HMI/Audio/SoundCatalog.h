// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "Core/Data/JsonDocument.h"

/**
 * @file HMI/Audio/SoundCatalog.h
 * @brief Association nom logique d'événement → fichier de son (`EX-REN-047`).
 */

namespace hmi {

/**
 * @brief Catégorie d'échec de lecture d'un `sounds.json` (même patron que
 *        `hmi::SkinCatalogError`/`hmi::AnimationCatalogError`).
 */
enum class SoundCatalogError {
    None,                ///< Pas d'erreur.
    FileNotFound,        ///< Fichier absent. Cas **légitime** (catalogue vide), pas une anomalie.
    ParseError,          ///< JSON malformé, ou racine qui n'est pas un objet.
    UnsupportedVersion,  ///< Numéro de version supérieur à `SoundCatalog::FORMAT_VERSION`.
    MalformedStructure,  ///< Structure inattendue (« sons » absent, entrée invalide…).
};

// Le resultat contient un SoundCatalog par valeur : il ne peut etre defini qu'apres la classe.
struct SoundCatalogResult;

/**
 * @brief Catalogue des sons : `nom d'événement → fichier` (relatif à `Source/Elements/Audio/`).
 *
 * Logique **pure** (aucune dépendance Qt/périphérique), testable en isolation (`EX-NFR-010`) —
 * la résolution de chemin est séparée de la lecture, qui appartient à `hmi::AudioEngine`. Aucune
 * lecture ne lève d'exception vers l'appelant (`EX-NFR-040`) : toute erreur est décrite dans un
 * `hmi::SoundCatalogResult`.
 *
 * Même patron que `hmi::SkinCatalog` (`LOT-42`) : un fichier absent produit un catalogue vide
 * (cas légitime), une entrée malformée fait échouer le chargement **entier** plutôt que d'être
 * devinée ou silencieusement ignorée.
 */
class SoundCatalog {
public:
    /// @brief Version du format écrite dans le fichier, et plus élevée qui soit lue.
    static constexpr int FORMAT_VERSION = 1;

    /**
     * @brief Lit un catalogue depuis une chaîne JSON.
     * @param json Contenu JSON.
     * @return Le catalogue, ou une erreur décrite. Ne lève jamais.
     */
    [[nodiscard]] static SoundCatalogResult loadFromString(std::string_view json);

    /**
     * @brief Lit un catalogue depuis un fichier.
     *
     * Un fichier **absent** produit un échec de code `FileNotFound` : c'est à l'appelant de
     * décider qu'un catalogue vide est un état de départ légitime.
     * @param path Chemin du fichier.
     * @return Le catalogue, ou une erreur décrite. Ne lève jamais.
     */
    [[nodiscard]] static SoundCatalogResult loadFromFile(const std::filesystem::path& path);

private:
    /**
     * @brief Construit le catalogue depuis l'enveloppe déjà lue par `core::readJsonObject`.
     *
     * Le JSON bien formé, la racine objet et la garde de version sont vérifiés **avant** d'arriver
     * ici : c'est la brique partagée du `LOT-79` qui s'en charge (`EX-CNT-012`), et ce catalogue
     * n'a plus à les réimplémenter pour son compte.
     */
    [[nodiscard]] static SoundCatalogResult fromDocument(const core::JsonDocument& document);

public:
    /**
     * @brief Résout le fichier assigné à un événement.
     * @param eventId Identifiant logique de l'événement (ex. "saut").
     * @return Le nom de fichier (relatif à `Source/Elements/Audio/`), ou `std::nullopt` si
     *         l'événement n'est pas dans le catalogue — l'appelant reste silencieux
     *         (`EX-NFR-040`), il n'affiche ni ne journalise rien à chaque déclenchement.
     */
    [[nodiscard]] std::optional<std::string> resolve(std::string_view eventId) const;

    /// @return Les identifiants d'événement connus, par ordre alphabétique (ordre stable).
    [[nodiscard]] std::vector<std::string> eventIds() const;

    /// @brief Assigne un fichier à un événement, en écrasant une assignation existante.
    void assign(const std::string& eventId, std::string file);

private:
    // std::map (et non unordered_map) : ordre de parcours deterministe, comme hmi::SkinCatalog.
    std::map<std::string, std::string, std::less<>> _sounds;
};

/**
 * @brief Résultat d'une lecture de catalogue : soit un `SoundCatalog`, soit une **erreur** décrite.
 *
 * Même patron que `hmi::SkinCatalogResult` : aucune exception ne franchit la frontière de lecture
 * (`EX-NFR-040`).
 */
struct SoundCatalogResult {
    std::optional<SoundCatalog> catalog;
    std::string error;
    SoundCatalogError errorCode = SoundCatalogError::None;

    /// @return true si la lecture a réussi.
    [[nodiscard]] bool ok() const noexcept {
        return catalog.has_value();
    }
};

}  // namespace hmi
