// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <filesystem>
#include <string>
#include <vector>

/**
 * @file HMI/Editor/AssetLibrary.h
 * @brief Balayage et filtrage d'un dossier d'assets image, partagé par toutes les sections du
 *        panneau « Textures » (`EX-EDIT-026`).
 */

namespace hmi {

/**
 * @brief Liste les fichiers image d'un dossier, filtrés par un texte de recherche.
 *
 * Logique **pure** (aucune dépendance Qt/GPU), testable en isolation (`EX-NFR-010`) : c'est la
 * balayage qui alimente `hmi::AssetThumbnailView`, réutilisable tel quel par toutes les sections
 * du panneau « Textures » (Skins, puis Fond, Objets, Animations, Décors).
 * @param directory  Dossier balayé (typiquement un sous-dossier de `Assets/`).
 * @param filterText Sous-chaîne recherchée dans le nom de fichier, comparaison insensible à la
 *                   casse ; vide pour ne filtrer sur rien.
 * @return Les noms de fichiers image correspondants, triés par ordre alphabétique ; vide si le
 *         dossier est absent — un état de départ légitime, pas une erreur.
 */
[[nodiscard]] std::vector<std::string> listAssetFiles(const std::filesystem::path& directory,
                                                      const std::string& filterText = {});

}  // namespace hmi
