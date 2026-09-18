// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <QImage>
#include <filesystem>

#include "Core/Math/Vector2.h"
#include "HMI/Graphics/WorldSceneComposer.h"

namespace core {
class IsoProjection;
struct CityBlock;
}  // namespace core

/**
 * @file HMI/Graphics/CityBlockRender.h
 * @brief L'**îlot** vu sur le plan : la carte du quartier telle que le jeu la dessine, cadrée sur
 *        un rectangle nommé (`LOT-96`).
 *
 * *Décision de l'auteur, 18 septembre 2026* : l'îlot n'a pas d'image à lui. L'écran « Carte » le
 * montre par le **même** rendu que le lieu qu'on parcourt (`hmi::WorldSceneRenderer`), hors écran,
 * sur un `QRhi` sans fenêtre — les mêmes planches, les mêmes marqueurs, le héros à sa case. Rien à
 * peindre, et un plan qui ne peut pas diverger du terrain.
 */

namespace hmi {

/// @brief Le cadrage d'un îlot : le point à suivre, et la taille d'image qui le contient.
struct CityBlockFraming {
    /// Le point, en cases, que le rendu suit (`WorldSceneRenderer::setFocus`).
    core::Vector2 focus{};
    int pixelWidth = 0;
    int pixelHeight = 0;
};

/**
 * @brief Cadre l'îlot @p block de la carte que @p projection projette.
 *
 * L'image couvre le losange englobant de l'îlot, à l'agrandissement 1 du lieu
 * (`Camera2D::PIXELS_PER_UNIT`), plus, en haut, la hauteur des pièces les plus hautes de l'atelier :
 * un mur posé au fond de l'îlot se dresse au-dessus de sa case, et le couper ferait un plan
 * décapité.
 */
[[nodiscard]] CityBlockFraming cityBlockFraming(const core::IsoProjection& projection,
                                                const core::CityBlock& block);

/**
 * @brief Dessine @p snapshot cadré sur @p block, hors écran.
 *
 * @param assetsDirectory Le dossier des assets, où les chemins de l'instantané se résolvent.
 * @param snapshot L'instantané du lieu à peindre, tel que `WorldSceneRenderer` le produit.
 * @param block L'îlot sur lequel le rendu se cadre.
 * @return L'image, ou une image nulle si aucune interface QRhi n'est disponible — l'écran le dit
 *         plutôt que de planter (`EX-NFR-040`).
 */
[[nodiscard]] QImage renderCityBlock(const std::filesystem::path& assetsDirectory,
                                     const WorldSceneSnapshot& snapshot,
                                     const core::CityBlock& block);

}  // namespace hmi
