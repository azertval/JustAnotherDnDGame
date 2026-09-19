// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_map_render.cpp
 * @brief Tests de `LevelEditor --render` (`LOT-EDITOR-13`) : une carte livrée rendue hors écran,
 *        sans fenêtre, par le peintre du canevas.
 */

#include <QColor>
#include <QImage>
#include <cstdlib>
#include <filesystem>
#include <optional>
#include <string>

#include <gtest/gtest.h>

#include "Core/Levels/LevelLoader.h"
#include "Editor/Logic/CanvasScene.h"
#include "Editor/Ui/MapRender.h"

namespace {

[[nodiscard]] std::filesystem::path dataRoot() {
    return std::filesystem::path(JADG_LEVELS_DIR).parent_path();
}

/// La part des pixels de @p image qui ne sont pas le fond @p background.
[[nodiscard]] double peinte(const QImage& image, const QColor& background) {
    const QImage pixels = image.convertToFormat(QImage::Format_RGB32);
    long long painted = 0;
    for (int y = 0; y < pixels.height(); ++y) {
        for (int x = 0; x < pixels.width(); ++x) {
            if (pixels.pixelColor(x, y) != background) {
                ++painted;
            }
        }
    }
    return static_cast<double>(painted) /
           (static_cast<double>(pixels.width()) * static_cast<double>(pixels.height()));
}

}  // namespace

/**
 * @brief Martpart se rend en PNG, sans fenêtre ; la collision se peint par-dessus quand on la
 *        demande.
 * \castest{<b>--render peint une carte livrée hors écran.</b><br/>
 * \tcat Unitaire · Editeur · Sans fenetre<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Rendre Martpart au quart de l'échelle, bandes par défaut.<br/>
 *          2. La rendre avec la collision en plus.<br/>
 * \tattendu Une image de la taille du cadre (989 × 648), peinte sur plus du cinquième de sa
 *           surface (le losange de la carte en couvre la moitié, moins les îlots vides) ; la
 *           collision change l'image.
 * }
 */
TEST(MapRenderTest, MartpartSeRendSansFenetre) {
    const core::LevelLoadResult martpart =
        core::LevelLoader::loadFromFile(dataRoot() / "Levels" / "capital" / "martpart.json");
    ASSERT_TRUE(martpart.ok()) << martpart.error;

    hmi::MapRenderOptions options;
    options.scale = 0.25;
    const QImage lieu = hmi::renderMap(*martpart.level, dataRoot(), options);
    EXPECT_EQ(lieu.width(), 989);
    EXPECT_EQ(lieu.height(), 648);
    EXPECT_GT(peinte(lieu, options.background), 1.0 / 5.0);

    options.bands.collision = 1.0F;
    const QImage collision = hmi::renderMap(*martpart.level, dataRoot(), options);
    EXPECT_NE(collision, lieu);
}

/**
 * @brief Les bandes de `--layers` se lisent par leur nom ; un nom inconnu est refusé.
 * \castest{<b>--layers lit les bandes du canevas.</b><br/>
 * \tcat Unitaire · Editeur · Sans fenetre<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Lire `floors,collision`, puis `floors,toit`.<br/>
 * \tattendu Le sol et la collision seuls ; puis un refus.
 * }
 */
TEST(MapRenderTest, LesBandesSeLisentParLeurNom) {
    const std::optional<hmi::IsoBandOpacity> bands = hmi::parseRenderLayers("floors,collision");
    ASSERT_TRUE(bands.has_value());
    EXPECT_EQ(*bands, (hmi::IsoBandOpacity{
                          .floors = 1.0F, .relief = 0.0F, .figures = 0.0F, .collision = 1.0F}));
    EXPECT_FALSE(hmi::parseRenderLayers("floors,toit").has_value());
}

/**
 * @brief `--render` écrit une image par carte, nommée d'après son identifiant.
 * \castest{<b>--render écrit une image par carte.</b><br/>
 * \tcat Unitaire · Editeur · Sans fenetre<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. `--render capital/martpart coliseum --scale 0.125 --output <dossier>`.<br/>
 * \tattendu Code 0 ; `capital-martpart.png` et `coliseum.png` dans le dossier.
 * }
 */
TEST(MapRenderTest, RenderEcritUneImageParCarte) {
    const std::filesystem::path dossier =
        std::filesystem::temp_directory_path() / ("jadg-render-" + std::to_string(std::rand()));
    std::string sortie;
    const std::optional<int> code =
        hmi::runRenderCommand({"--render", "capital/martpart", "coliseum", "--scale", "0.125",
                               "--data", dataRoot().string(), "--output", dossier.string()},
                              {}, sortie);
    ASSERT_TRUE(code.has_value());
    EXPECT_EQ(*code, 0) << sortie;
    EXPECT_TRUE(std::filesystem::exists(dossier / "capital-martpart.png")) << sortie;
    EXPECT_TRUE(std::filesystem::exists(dossier / "coliseum.png")) << sortie;
    EXPECT_FALSE(hmi::runRenderCommand({"--check"}, {}, sortie).has_value());

    std::error_code ignore;
    std::filesystem::remove_all(dossier, ignore);
}
