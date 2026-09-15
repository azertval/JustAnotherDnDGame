// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstdint>
#include <vector>

#include "Core/Levels/LevelDraft.h"
#include "HMI/Graphics/TextureLoader.h"

/**
 * @file HMI/Editor/PlaneReference.h
 * @brief Repères géométriques du mode création : pelure d'oignon des tuiles, aplatissement des
 *        plans voisins et rééchantillonnage (`EX-EDIT-046`, `EX-DEC-045`, LOT-69 TACHE-07).
 *
 * **Sans Qt ni GPU** (`EX-NFR-010`) : tout est calculé sur des `hmi::DecodedImage`, donc testable
 * exhaustivement hors instance d'application.
 *
 * La référence est un **repère géométrique, pas un aperçu**. Ni raccords automatiques, ni skins, ni
 * objets animés : elle dit *où* sont les choses, pas *à quoi elles ressembleront*. Le dire ici et
 * dans la documentation utilisateur évite une attente déçue — l'aperçu fidèle reste l'essai.
 */

namespace hmi {

/// Un plan à aplatir sous (ou sur) celui qu'on peint.
struct PlaneLayer {
    /// Image du plan, à sa propre densité. `nullptr` : couche ignorée.
    const DecodedImage* image = nullptr;
    /// Densité de @ref image, en pixels par unité monde.
    int pixelsPerUnit = 16;
    /// Opacité de composition, dans `[0, 1]`.
    float opacity = 1.0f;
    /// Couche masquée dans l'éditeur : ignorée, comme si elle n'existait pas.
    bool visible = true;
};

/**
 * @brief Rééchantillonne une image d'une densité à une autre, par ratio **entier**.
 *
 * Jamais d'interpolation : 16 → 8 garde **un pixel sur deux**, 8 → 16 en **duplique** chacun. Un
 * filtrage moyennerait des couleurs voisines et introduirait des teintes que l'artiste n'a pas
 * posées — inacceptable en pixel art, et contraire à `EX-ARCH-022`.
 *
 * Sert deux besoins : aplatir des plans de densités différentes sur un même repère
 * (`flattenPlanes`) et **changer la densité** d'un plan depuis le panneau (`TACHE-08`), auquel cas
 * la perte est réelle et assumée — descendre puis remonter ne restitue pas l'original.
 * @param image             Image source.
 * @param fromPixelsPerUnit Densité de @p image (> 0).
 * @param toPixelsPerUnit   Densité voulue (> 0).
 * @return L'image rééchantillonnée ; une copie de @p image si les deux densités sont égales, une
 *         image vide si l'une est invalide ou si leur rapport n'est pas entier dans un sens ou
 *         dans l'autre.
 */
[[nodiscard]] DecodedImage resamplePlane(const DecodedImage& image, int fromPixelsPerUnit,
                                         int toPixelsPerUnit);

/**
 * @brief Construit la **pelure d'oignon** des tuiles : une couleur plate par type, aux dimensions
 *        d'un plan de la densité donnée.
 *
 * Réutilise la palette du **mode Physique** (`hmi::buildProceduralAtlasImage`), qui est déjà « la
 * lecture des collisions sans distraction » : inventer un second jeu de couleurs pour l'éditeur
 * ferait diverger deux vues du même niveau. Les cases vides restent **transparentes** — c'est un
 * repère qu'on peint par-dessus, pas un fond.
 * @param draft          Brouillon dont on lit la grille de tuiles.
 * @param pixelsPerUnit  Densité du plan peint (4, 8 ou 16).
 * @return L'image du repère, vide si @p pixelsPerUnit est invalide ou la grille dégénérée.
 */
[[nodiscard]] DecodedImage buildTileOnionSkin(const core::LevelDraft& draft, int pixelsPerUnit);

/**
 * @brief Aplatit des plans en une seule image, par **alpha-over** dans l'ordre donné.
 *
 * Les couches masquées sont ignorées — c'est ce qui rend l'isolement (`hmi::PlaneVisibility`)
 * effectif dans le repère aussi, et pas seulement à l'écran de jeu. Chaque couche est ramenée à la
 * densité cible par `resamplePlane` avant d'être composée : une couche de densité incompatible est
 * ignorée plutôt que déformée.
 * @param layers        Couches à aplatir, de la plus lointaine à la plus proche.
 * @param pixelsPerUnit Densité cible (celle du plan peint).
 * @param widthUnits    Largeur du niveau, en unités monde.
 * @param heightUnits   Hauteur du niveau, en unités monde.
 * @return L'image aplatie, entièrement transparente si aucune couche visible ne subsiste.
 */
[[nodiscard]] DecodedImage flattenPlanes(const std::vector<PlaneLayer>& layers, int pixelsPerUnit,
                                         int widthUnits, int heightUnits);

/**
 * @brief Compose un pixel **au-dessus** d'un autre (alpha-over, alpha non prémultiplié).
 *
 * Exposée pour être testée directement : l'associativité de l'alpha-over est ce qui garantit
 * qu'aplatir trois plans d'un coup donne le même résultat que deux fois deux.
 * @param below Pixel de dessous (RGBA, R en poids faible).
 * @param above Pixel de dessus.
 * @return Le pixel composé.
 */
[[nodiscard]] std::uint32_t alphaOver(std::uint32_t below, std::uint32_t above) noexcept;

}  // namespace hmi
