// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <filesystem>
#include <string>
#include <vector>

/**
 * @file Editor/Logic/DataRoot.h
 * @brief La racine des données que l'éditeur ouvre : celle des cartes, des planches et des
 *        catalogues (`LOT-EDITOR-06`).
 *
 * Jusqu'au `LOT-EDITOR-06`, la fenêtre ouvrait les cartes **à côté de l'exécutable**, dans la copie
 * que la construction y refait à chaque fois (`CopyGameData`) : ce qu'on enregistrait à la souris
 * n'atteignait jamais le dépôt, et la construction suivante l'écrasait. L'éditeur faisant foi pour
 * les cartes (décision D4), il ouvre désormais **l'arbre des sources** qui l'a construit
 * (`Source/Elements`), et la fenêtre comme les commandes sans fenêtre partagent la même racine.
 */

namespace hmi {

/**
 * @brief Choisit la racine des données.
 *
 * Dans l'ordre : la valeur de `--data`, si la ligne de commande en donne une ; sinon
 * @p sourceData, le `Source/Elements` de l'arbre qui a construit l'éditeur, s'il existe sur ce
 * poste ; sinon @p executableDirectory, où la construction recopie les données.
 *
 * @param arguments           Les arguments de la ligne de commande, sans le programme.
 * @param executableDirectory Le dossier de l'exécutable.
 * @param sourceData          Le dossier des données de l'arbre des sources ; vide s'il est
 *                            inconnu.
 */
[[nodiscard]] std::filesystem::path resolveDataRoot(
    const std::vector<std::string>& arguments, const std::filesystem::path& executableDirectory,
    const std::filesystem::path& sourceData);

/// @brief Fixe la racine des données de la fenêtre, une fois, au lancement.
void setEditorDataRoot(std::filesystem::path root);

/// @return La racine des données de la fenêtre : `Levels/`, `Assets/`, `World/`… y vivent.
[[nodiscard]] const std::filesystem::path& editorDataRoot();

}  // namespace hmi
