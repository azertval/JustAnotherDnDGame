// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "Core/Levels/CameraFraming.h"
#include "Core/Levels/Level.h"

/**
 * @file Core/Levels/LevelWriter.h
 * @brief Sérialisation d'un niveau vers le format JSON, symétrique à `LevelLoader`.
 */

namespace core {

class TileMap;

/**
 * @brief Sérialise un niveau vers le format JSON défini par `EX-LVL-003`.
 *
 * Fonction **pure**, symétrique à `LevelLoader::loadFromString` : recharger la chaîne produite
 * reconstruit un niveau équivalent (mêmes tuiles, entrée/sortie, mécanismes, budgets),
 * `EX-EDIT-011`. Les identifiants d'interrupteurs (`id`/`opensWith`) ne sont pas conservés du
 * fichier d'origine — ni `Level` ni `LevelDraft` ne les retiennent après chargement — ils sont
 * **régénérés** de façon déterministe (balayage de la grille ligne par ligne), sans effet sur la
 * sémantique du niveau rechargé.
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
     * Prend l'agrégat `core::LevelData` du `LOT-03` plutôt que ses neuf composantes en
     * **paramètres positionnels** : les couches et les entités du `LOT-04` les auraient portées à
     * onze, dont trois `std::vector` voisins qui s'intervertissent sans que le compilateur
     * bronche. C'est exactement la dette que le `LOT-03` a soldée du côté du constructeur de
     * `Level` ; l'écrivain n'avait pas de raison de la garder.
     *
     * Champs consommés, et les conventions qui les gouvernent :
     * - `tileMap` est la **source de vérité** des positions entrée/sortie/mécanismes : une tuile
     *   `Entry`/`Exit`/`Switch`/`Door` présente dans la grille est émise, qu'elle soit ou non
     *   reliée/complète. `entry` et `exit` n'en sont que la relecture, jamais écrits à part.
     * - `layers` et `entities` (`LOT-04`, `EX-LVL-016`/`EX-LVL-017`) sont émis dans les tableaux
     *   racine optionnels `"layers"` et `"entities"`, omis quand ils sont vides ou — pour les
     *   couches — quand la carte n'en porte qu'une, de rôle `Legacy` : une carte `version: 2`
     *   promue au chargement ressort **telle qu'elle est entrée**, sans couche apparue de nulle
     *   part.
     * - Les propriétés libres d'une couche ou d'une entité, y compris les clés que le chargeur
     *   n'a pas reconnues, sont réémises à côté des champs connus (`EX-LVL-018`).
     * - `cameraFraming` (`EX-LVL-006`) n'est émis que s'il **diverge** de ce que la règle de repli
     *   (`resolveCameraFraming`) recalculerait pour ces dimensions : un fichier **sans** le champ,
     *   dont le cadrage résolu coïncide avec le repli, ressort **sans** le champ.
     * - `background`, `skinSet`, `planes` et `parallaxEnabled` suivent la même convention
     *   « omis à sa valeur par défaut » (`LOT-67`).
     *
     * @param data Composantes du niveau à sérialiser.
     * @return Le contenu JSON correspondant.
     */
    [[nodiscard]] static std::string buildJson(const LevelData& data);
};

}  // namespace core
