// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

/**
 * @file Core/Resources/ScenePieceManifest.h
 * @brief Le **manifeste des pièces** d'un lieu : ce que l'atelier des textures (`LOT-92`) déclare
 *        de chaque pièce de sa planche — classe, emprise, ancre, taille, miroir.
 *
 * L'atelier écrit, à côté des images d'un lieu, un `Assets/Scene/<lieu>/manifest.json`. Jusqu'au
 * `LOT-EDITOR-02`, seule la galerie des assets le lisait, dans `HMI` (constat A9 de la feuille de
 * route de l'éditeur) : aucune règle de carte ne pouvait donc s'appuyer sur l'emprise d'une pièce.
 * La lecture descend ici, sans Qt ni GPU, pour que `Core` puisse un jour en déduire l'occupation et
 * la collision (`LOT-EDITOR-12`) ; `HMI` n'en garde que les images.
 *
 * Le manifeste nomme une pièce par une clé d'atelier (`scene/martpart/wall-left`). Une carte, elle,
 * ne connaît que le **nom court** (`wall-left`), celui que la table d'apparence et l'assignation de
 * texture écrivent : le lecteur rend les deux.
 */

namespace core {

struct JsonDocument;

/// @brief Classe d'une pièce, telle que l'atelier la range.
enum class ScenePieceClass : std::uint8_t {
    /// Un losange de sol, posé à plat.
    Floor,
    /// Une pièce debout d'une case : mur, porte, torche.
    Tall,
    /// Une pièce debout de plusieurs cases : façade, étal, gradin.
    Wide,
    /// Une classe que ce lecteur ne connaît pas : gardée par son nom, jamais refusée.
    Other,
};

/// @brief Une pièce de la planche d'un lieu.
struct ScenePiece {
    /// Nom court (`wall-left`) : celui que les cartes écrivent.
    std::string name;
    /// Clé de l'atelier (`scene/martpart/wall-left`).
    std::string key;
    /// Fichier image, relatif au dossier du lieu (`wall-left.png`).
    std::string file;
    ScenePieceClass pieceClass = ScenePieceClass::Other;
    /// Classe telle qu'écrite (`floor`, `tall`, `wide`…), pour une classe inconnue.
    std::string className;
    /// Emprise en cases : colonnes, puis lignes. Au moins 1 × 1.
    int footprintColumns = 1;
    int footprintRows = 1;
    /// Taille de l'image, en pixels d'art ; 0 si le manifeste ne la donne pas.
    int width = 0;
    int height = 0;
    /// Ancre (sommet haut du losange de l'emprise), en pixels d'art ; -1 si non donnée.
    int anchorX = -1;
    int anchorY = -1;
    /// Nom court de la pièce dont celle-ci est le miroir, vide sinon. L'image miroir est livrée
    /// telle quelle par l'atelier : le rendu n'a rien à retourner.
    std::string mirrorOf;

    [[nodiscard]] bool operator==(const ScenePiece&) const = default;
};

/// @brief Catégorie d'échec de lecture.
enum class ScenePieceManifestError : std::uint8_t {
    None,
    FileNotFound,
    ParseError,
    UnsupportedVersion,
    MalformedStructure,
};

struct ScenePieceManifestResult;

/**
 * @brief Les pièces d'un lieu, dans l'ordre du manifeste.
 *
 * Logique pure. Aucune lecture ne lève (`EX-NFR-040`) ; une entrée mal formée (sans `file`) est
 * ignorée plutôt que de faire perdre les autres, comme la galerie l'a toujours fait.
 */
class ScenePieceManifest {
public:
    static constexpr int FORMAT_VERSION = 1;

    [[nodiscard]] static ScenePieceManifestResult loadFromString(std::string_view json);
    [[nodiscard]] static ScenePieceManifestResult loadFromFile(const std::filesystem::path& path);

    /// @return Le lieu (`disposition` du manifeste : `martpart`), vide s'il n'est pas donné.
    [[nodiscard]] const std::string& place() const noexcept {
        return _place;
    }

    /// @return Les pièces, dans l'ordre où le manifeste les écrit.
    [[nodiscard]] const std::vector<ScenePiece>& pieces() const noexcept {
        return _pieces;
    }

    /// @return La pièce de nom court @p name, `nullptr` si le lieu ne la déclare pas.
    [[nodiscard]] const ScenePiece* find(std::string_view name) const noexcept;

private:
    [[nodiscard]] static ScenePieceManifestResult fromDocument(const JsonDocument& document);

    std::string _place;
    std::vector<ScenePiece> _pieces;
};

/// @brief Résultat d'une lecture : le manifeste, et ce qui a échoué.
struct ScenePieceManifestResult {
    ScenePieceManifest manifest;
    ScenePieceManifestError error = ScenePieceManifestError::None;
    /// Message technique, vide en cas de succès.
    std::string message;

    [[nodiscard]] bool ok() const noexcept {
        return error == ScenePieceManifestError::None;
    }
};

/// @return La classe nommée @p name (`floor`, `tall`, `wide`), `Other` pour toute autre.
[[nodiscard]] ScenePieceClass parseScenePieceClass(std::string_view name) noexcept;

/// @return Le nom court d'une clé d'atelier : ce qui suit la dernière barre.
[[nodiscard]] std::string_view scenePieceShortName(std::string_view key) noexcept;

}  // namespace core
