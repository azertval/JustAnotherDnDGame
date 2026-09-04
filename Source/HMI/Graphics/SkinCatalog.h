// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "Core/Levels/TileType.h"

/**
 * @file HMI/Graphics/SkinCatalog.h
 * @brief Association type de tuile → asset et mode de rendu, groupée en jeux nommés
 *        (`EX-EDIT-042`, `EX-EDIT-024`).
 */

#include "Core/Data/JsonDocument.h"

namespace hmi {

/**
 * @brief Manière dont l'asset d'un type de tuile est découpé à l'affichage (`EX-EDIT-025`).
 *
 * Le mode est porté **par type**, jamais par jeu : un même jeu contient des types à raccords (les
 * solides) et des types à image unique (dangers, mécanismes). Un mode global forcerait des
 * exceptions dans le code de rendu.
 */
enum class SkinMode {
    /// Image unique, utilisée telle quelle. Seul mode qui ait un sens pour un type dont le
    /// voisinage n'en a pas : dangers, interrupteurs, portes, blocs, pentes et arrondis.
    Single,
    /// Planche de 4×4 cases, dont celle affichée est choisie selon les quatre voisins solides
    /// (`hmi::autotileCell`). Dé-répétition et bords distincts de l'intérieur.
    Bitmask16,
};

/**
 * @brief Nom persisté d'un mode de skin (valeur écrite dans `skins.json`).
 * @param mode Mode à nommer.
 * @return `"single"` ou `"bitmask16"` (chaîne statique, jamais nulle).
 */
[[nodiscard]] const char* skinModeName(SkinMode mode) noexcept;

/**
 * @brief Mode de skin correspondant à un nom lu dans un fichier.
 * @param name Nom lu.
 * @return Le mode correspondant, ou `std::nullopt` si @p name n'en désigne aucun — un mode
 *         inconnu est une donnée invalide à signaler, jamais un mode deviné.
 */
[[nodiscard]] std::optional<SkinMode> skinModeFromName(std::string_view name) noexcept;

/// @brief Apparence assignée à un type de tuile : le fichier à utiliser et son découpage.
struct SkinEntry {
    /// Nom de fichier, relatif au dossier `Assets/Skins/`. Jamais un chemin absolu : la
    /// configuration doit rester valable sur un autre poste.
    std::string asset;
    SkinMode mode = SkinMode::Single;

    [[nodiscard]] friend bool operator==(const SkinEntry& lhs, const SkinEntry& rhs) {
        return lhs.asset == rhs.asset && lhs.mode == rhs.mode;
    }
};

/**
 * @brief Catégorie d'échec de lecture d'un `skins.json` (même esprit que
 *        `core::LevelValidationError`).
 *
 * Complète le message technique d'un code **programmatique**, pour qu'un appelant puisse réagir
 * sans analyser le texte du message — une comparaison de sous-chaînes se casserait à la première
 * reformulation.
 */
enum class SkinCatalogError {
    None,                ///< Pas d'erreur.
    FileNotFound,        ///< Fichier absent. Cas **légitime**, pas une anomalie (voir ci-dessous).
    ParseError,          ///< JSON malformé, ou racine qui n'est pas un objet.
    UnsupportedVersion,  ///< Numéro de version supérieur à `SkinCatalog::FORMAT_VERSION`.
    MalformedStructure,  ///< Structure inattendue (« jeux » absent, entrée sans « asset »…).
};

// Le resultat contient un SkinCatalog par valeur : il ne peut etre defini qu'apres la classe. Une
// declaration de fonction, elle, peut renvoyer un type incomplet — d'ou cette annonce.
struct SkinCatalogResult;

/**
 * @brief Catalogue des jeux de skins : `(jeu, type de tuile) → { asset, mode }`.
 *
 * **Configuration de présentation, jamais donnée de simulation.** Ce catalogue ne transite ni par
 * `core::Level`, ni par `core::LevelDraft` ni son undo/redo : un changement d'habillage n'est pas
 * une modification du niveau. Seule la *désignation* du jeu par un niveau relève du format de
 * niveau, et c'est l'objet du LOT-44.
 *
 * Logique **pure** (aucune dépendance GPU ni Qt), testable en isolation (`EX-NFR-010`). Aucune
 * lecture ne lève d'exception vers l'appelant (`EX-NFR-040`) : toute erreur est décrite dans un
 * `hmi::SkinCatalogResult`.
 */
class SkinCatalog {
public:
    /**
     * @brief Version du format écrite dans le fichier, et plus élevée qui soit lue.
     *
     * Le champ est présent dès la création du format : l'ajouter coûte une ligne aujourd'hui, une
     * migration une fois des fichiers dans la nature. Un fichier de version **supérieure** est
     * refusé plutôt que lu au mieux — c'est précisément ce que le champ sert à éviter.
     */
    static constexpr int FORMAT_VERSION = 1;

    /**
     * @brief Lit un catalogue depuis une chaîne JSON.
     * @param json Contenu JSON.
     * @return Le catalogue, ou une erreur décrite. Ne lève jamais.
     */
    [[nodiscard]] static SkinCatalogResult loadFromString(std::string_view json);

    /**
     * @brief Lit un catalogue depuis un fichier.
     *
     * Un fichier **absent** produit un échec de code `FileNotFound` : c'est à l'appelant de
     * décider qu'un catalogue vide est un état de départ légitime (ce qu'il est), car lui seul
     * sait s'il attendait un fichier livré ou une configuration utilisateur pas encore écrite.
     * @param path Chemin du fichier.
     * @return Le catalogue, ou une erreur décrite. Ne lève jamais.
     */
    [[nodiscard]] static SkinCatalogResult loadFromFile(const std::filesystem::path& path);

private:
    /**
     * @brief Construit le résultat depuis l'enveloppe déjà lue par `core::readJsonObject`.
     *
     * JSON bien formé, racine objet et garde de version sont vérifiés **avant** d'arriver ici :
     * c'est la brique partagée du `LOT-79` (`EX-CNT-012`), et ce catalogue n'a plus à les
     * réimplémenter pour son compte.
     */
    [[nodiscard]] static SkinCatalogResult fromDocument(const core::JsonDocument& document);

public:
    /// @return La représentation JSON du catalogue, relisible par `loadFromString`.
    [[nodiscard]] std::string toJsonString() const;

    /**
     * @brief Écrit le catalogue dans un fichier, en créant les dossiers parents au besoin.
     * @param path Chemin de destination (typiquement `executableDirectory()/Assets/skins.json`).
     * @return true si l'écriture a réussi. Ne lève jamais.
     */
    [[nodiscard]] bool saveToFile(const std::filesystem::path& path) const;

    /**
     * @brief Résout l'apparence d'un type de tuile dans un jeu donné.
     *
     * Repli sur le jeu par défaut si @p setName ne désigne aucun jeu : un niveau qui référence un
     * jeu supprimé doit rester affichable, dégradé plutôt qu'illisible (`EX-NFR-040`).
     * @param setName Nom du jeu voulu. Vide pour demander explicitement le jeu par défaut.
     * @param type Type de tuile.
     * @return L'entrée assignée, ou `std::nullopt` si le type n'est pas skinné — auquel cas
     *         l'appelant affiche le damier de repli (`hmi::MissingTexture`).
     */
    [[nodiscard]] std::optional<SkinEntry> resolve(std::string_view setName,
                                                   core::TileType type) const;

    /// @return Le nom du jeu par défaut (vide si le catalogue ne contient aucun jeu).
    [[nodiscard]] const std::string& defaultSetName() const noexcept {
        return _defaultSet;
    }

    /**
     * @brief Désigne le jeu par défaut.
     * @param setName Nom du jeu. Sans effet si aucun jeu ne porte ce nom.
     * @return true si le jeu existe et a été retenu.
     */
    [[nodiscard]] bool setDefaultSetName(const std::string& setName);

    /// @return Les noms des jeux, par ordre alphabétique (ordre stable d'affichage et d'écriture).
    [[nodiscard]] std::vector<std::string> setNames() const;

    /// @brief Crée un jeu vide s'il n'existe pas. @param setName Nom du jeu.
    void addSet(const std::string& setName);

    /**
     * @brief Assigne une apparence à un type de tuile dans un jeu, en créant le jeu au besoin.
     * @param setName Nom du jeu.
     * @param type Type de tuile.
     * @param entry Apparence à assigner.
     */
    void assign(const std::string& setName, core::TileType type, SkinEntry entry);

    /**
     * @brief Retire l'assignation d'un type de tuile dans un jeu (le type redevient non skinné).
     * @param setName Nom du jeu.
     * @param type Type de tuile.
     */
    void clearAssignment(const std::string& setName, core::TileType type);

    /**
     * @brief Assignations d'un jeu, sans repli sur le jeu par défaut.
     * @param setName Nom du jeu.
     * @return Les entrées du jeu, ordonnées par type ; vide si le jeu n'existe pas.
     */
    [[nodiscard]] std::map<core::TileType, SkinEntry> assignments(std::string_view setName) const;

private:
    // std::map (et non unordered_map) : l'ordre de parcours est deterministe, ce qui rend
    // l'ecriture du fichier stable d'une execution a l'autre — un diff de skins.json ne doit
    // montrer que ce que l'utilisateur a change.
    std::map<std::string, std::map<core::TileType, SkinEntry>, std::less<>> _sets;
    std::string _defaultSet;
};

/**
 * @brief Résultat d'une lecture de catalogue : soit un `SkinCatalog`, soit une **erreur** décrite.
 *
 * En cas de succès, `catalog` est renseigné, `error` est vide et `errorCode` vaut `None`. Même
 * patron que `core::LevelLoadResult` : aucune exception ne franchit la frontière de lecture
 * (`EX-NFR-040`).
 */
struct SkinCatalogResult {
    std::optional<SkinCatalog> catalog;
    std::string error;
    SkinCatalogError errorCode = SkinCatalogError::None;

    /// @return true si la lecture a réussi.
    [[nodiscard]] bool ok() const noexcept {
        return catalog.has_value();
    }
};

}  // namespace hmi
