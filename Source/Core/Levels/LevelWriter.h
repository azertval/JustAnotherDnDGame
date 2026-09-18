// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "Core/Levels/Level.h"

/**
 * @file Core/Levels/LevelWriter.h
 * @brief Sérialisation d'un niveau vers le format JSON, symétrique à `LevelLoader`.
 */

namespace core {

class TileMap;

/**
 * @brief Sérialise une carte vers le format JSON défini par `EX-LVL-003`.
 *
 * Fonction **pure**, symétrique à `LevelLoader::loadFromString` : recharger la chaîne produite
 * reconstruit une carte équivalente (mêmes tuiles, couches, entités, pièces assignées),
 * `EX-EDIT-011`.
 */
class LevelWriter {
public:
    /**
     * @brief Sérialise un niveau déjà construit et validé.
     * @param level Niveau à sérialiser.
     * @return Le contenu JSON du niveau.
     */
    [[nodiscard]] static std::string toJsonString(const Level& level);

    /**
     * @brief Écrit un niveau au format JSON dans un fichier (`EX-EDIT-006`).
     *
     * Écriture directe (`std::ofstream`), sans exception vers l'appelant : un échec (chemin
     * invalide, permissions) est signalé par la valeur de retour, jamais par une exception
     * (`EX-NFR-040`), symétrique à `LevelLoader::loadFromFile`.
     * @param level Niveau à écrire (déjà validé).
     * @param path  Chemin du fichier de destination ; le dossier parent doit exister.
     * @return `true` si l'écriture a réussi.
     */
    [[nodiscard]] static bool saveToFile(const Level& level, const std::filesystem::path& path);

    /**
     * @brief Construit le JSON à partir des composantes d'un niveau (utilisé également par
     *        `LevelDraft::toLevel()`, qui n'a pas nécessairement de `Level` construit).
     *
     * Prend l'agrégat `core::LevelData` du `LOT-03` plutôt que ses composantes en **paramètres
     * positionnels**, dont plusieurs `std::vector` voisins s'intervertiraient sans que le
     * compilateur bronche.
     *
     * Champs consommés, et les conventions qui les gouvernent :
     * - `tileMap` est la **source de vérité** de l'entrée : une tuile `Entry` présente dans la
     *   grille est émise ; `entry` n'en est que la relecture, jamais écrit à part.
     * - `layers` et `entities` (`LOT-04`, `EX-LVL-016`/`EX-LVL-017`) sont émis dans les tableaux
     *   racine optionnels `"layers"` et `"entities"`, omis quand ils sont vides ou — pour les
     *   couches — quand la carte n'en porte qu'une, de rôle `Legacy` : une carte `version: 2`
     *   promue au chargement ressort **telle qu'elle est entrée**, sans couche apparue de nulle
     *   part.
     * - Les propriétés libres d'une couche ou d'une entité, y compris les clés que le chargeur
     *   n'a pas reconnues, sont réémises à côté des champs connus (`EX-LVL-018`).
     * - `textureOverrides` (`EX-EDIT-043`) est émis sur la tuile racine de sa case, champ
     *   `"texture"`.
     *
     * @param data Composantes de la carte à sérialiser.
     * @return Le contenu JSON correspondant.
     */
    [[nodiscard]] static std::string buildJson(const LevelData& data);
};

}  // namespace core
