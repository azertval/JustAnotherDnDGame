// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <string>
#include <vector>

/**
 * @file HMI/Editor/PlaneFileNaming.h
 * @brief Nommage et dimensions des fichiers de plan (`EX-EDIT-047`, `EX-DEC-041`, LOT-69
 *        TACHE-08).
 *
 * Logique **pure** (aucune dépendance Qt ni disque), testable hors instance d'application
 * (`EX-NFR-010`) — même patron que `hmi::isValidLevelName` (`LevelNameValidation.h`), dont ce
 * fichier reprend délibérément les règles de caractères : un nom de plan est dérivé d'un nom de
 * niveau, il ne peut pas être plus permissif que lui.
 */

namespace hmi {

/// Extension des fichiers de plan. Un plan est une image, toujours écrite en PNG.
inline const std::string PLANE_FILE_EXTENSION = ".png";

/**
 * @brief Dimensions, en pixels, du PNG d'un plan.
 *
 * Le format borne déjà le produit `taille × densité` au chargement (`EX-DEC-044`) : cette fonction
 * ne re-valide rien, elle **dérive**. Sa raison d'être est qu'un seul endroit décide de ces
 * dimensions — la création du fichier, le changement de densité et le contrôle de cohérence
 * (`TACHE-10`) doivent tomber d'accord, et trois calculs séparés finiraient par diverger.
 * @param widthUnits    Largeur du niveau, en cases.
 * @param heightUnits   Hauteur du niveau, en cases.
 * @param pixelsPerUnit Densité du plan (4, 8 ou 16).
 * @return Les dimensions attendues ; `{0, 0}` si un paramètre est invalide.
 */
struct PlanePixelSize {
    int width = 0;
    int height = 0;

    [[nodiscard]] friend bool operator==(const PlanePixelSize&,
                                         const PlanePixelSize&) noexcept = default;
};

/// @copydoc PlanePixelSize
[[nodiscard]] PlanePixelSize planePixelSize(int widthUnits, int heightUnits,
                                            int pixelsPerUnit) noexcept;

/**
 * @brief Compose un nom de fichier de plan **unique** pour un niveau donné.
 *
 * Le nom dérive de celui du niveau, suffixé d'un numéro : `foret.png`, `foret-2.png`, … Le suffixe
 * est incrémenté jusqu'à sortir de @p existing — jamais un identifiant aléatoire, pour qu'un
 * dossier de plans reste lisible à l'œil et qu'un plan se retrouve sans ouvrir l'éditeur.
 * @param levelName Nom du niveau (sans extension), tel que saisi.
 * @param existing  Noms déjà pris dans le dossier des plans, extension comprise.
 * @return Le nom de fichier à créer, ou une chaîne vide si @p levelName ne donne aucun nom
 *         utilisable (vide, ou fait uniquement de caractères refusés).
 */
[[nodiscard]] std::string uniquePlaneFileName(const std::string& levelName,
                                              const std::vector<std::string>& existing);

/**
 * @brief Réduit un nom libre à ce qu'un nom de fichier de plan accepte.
 *
 * Mêmes caractères que les noms de niveau (`hmi::isValidLevelName`) : lettres, chiffres, tiret,
 * tiret bas et espace, ce dernier ramené au tiret. Tout le reste est **retiré** plutôt que
 * remplacé — un nom qui perd ses accents reste lisible, un nom truffé de tirets de substitution ne
 * l'est plus.
 * @param name Nom libre.
 * @return Le nom assaini, éventuellement vide.
 */
[[nodiscard]] std::string sanitizePlaneBaseName(const std::string& name);

}  // namespace hmi
