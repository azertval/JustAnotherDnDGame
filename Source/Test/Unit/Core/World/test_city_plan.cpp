// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_city_plan.cpp
 * @brief Le graphe d'une ville jouable (LOT-96) : la lecture, ses refus, et la Capitale livree.
 */

#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "Core/World/CityPlan.h"
#include "Core/World/WorldTravel.h"

namespace {

const std::filesystem::path MONDE{JADG_WORLD_DIR};
const std::filesystem::path NIVEAUX{JADG_LEVELS_DIR};

// Fournit un dossier temporaire vierge par test (cree/supprime automatiquement).
class CityPlanFileTest : public ::testing::Test {
protected:
    std::filesystem::path dir;

    void SetUp() override {
        dir = std::filesystem::temp_directory_path() /
              ("jadg_city_plan_" +
               std::string{::testing::UnitTest::GetInstance()->current_test_info()->name()});
        std::filesystem::remove_all(dir);
        std::filesystem::create_directories(dir);
    }

    void TearDown() override {
        std::filesystem::remove_all(dir);
    }

    [[nodiscard]] std::filesystem::path ecrire(const std::string& contenu) const {
        const std::filesystem::path chemin = dir / "ville.json";
        std::ofstream(chemin, std::ios::binary) << contenu;
        return chemin;
    }
};

}  // namespace

/**
 * @brief La Capitale livree se lit : douze quartiers, deux cartes, dix portes gardees.
 * \castest{<b>La Capitale livree se lit, et sa porte de depart s'ouvre.</b><br/>
 * \tcat Unitaire · Graphe de la ville<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Lire `Source/Elements/World/cities/capital.json`.<br/>
 * 2. Entrer a sa porte de depart par le chargeur du jeu.<br/>
 * \tattendu Douze quartiers, dont Martpart et Arenarea avec leur carte et dix fermes ; le depart
 * est la porte de l'Est de Martpart, et la carte s'y ouvre.
 * }
 */
TEST(CityPlanTest, LaCapitaleLivreeSeLit) {
    const core::CityPlanResult lue = core::loadCityPlan(MONDE / "cities" / "capital.json");
    ASSERT_TRUE(lue.ok()) << lue.error;
    const core::CityPlan& capitale = lue.plan;

    EXPECT_EQ(capitale.location, "central-empire-the-capital-city");
    ASSERT_EQ(capitale.districts.size(), 12U);
    int cartes = 0;
    int gardees = 0;
    for (const core::CityDistrict& quartier : capitale.districts) {
        (quartier.hasMap() ? cartes : gardees) += 1;
    }
    EXPECT_EQ(cartes, 2);
    EXPECT_EQ(gardees, 10);

    EXPECT_EQ(capitale.startMap(), "capital/martpart");
    EXPECT_EQ(capitale.startArrival, "porte-est");
    const core::CityDistrict* const arenarea = capitale.districtOfMap("capital/arenarea");
    ASSERT_NE(arenarea, nullptr);
    EXPECT_EQ(arenarea->id, "central-empire-the-capital-city-arenarea");
    EXPECT_EQ(capitale.districtOfMap("coliseum"), nullptr);

    core::WorldTravel voyage{core::WorldTravel::directoryLoader(NIVEAUX)};
    EXPECT_EQ(voyage.enter(capitale.startMap(), capitale.startArrival), core::TravelResult::Moved);
}

/**
 * @brief Un quartier qui a a la fois une carte et une porte gardee est refuse.
 * \castest{<b>Un quartier doit avoir soit une carte, soit une porte gardee.</b><br/>
 * \tcat Unitaire · Graphe de la ville<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Lire une ville dont un quartier porte `map` et `guard`, puis une autre dont un
 * quartier ne porte ni l'un ni l'autre.<br/>
 * \tattendu Les deux lectures echouent, en nommant le quartier.
 * }
 */
TEST_F(CityPlanFileTest, UnQuartierDoitAvoirSoitUneCarteSoitUnePorteGardee) {
    const core::CityPlanResult lesDeux = core::loadCityPlan(ecrire(R"({
        "id": "v", "name": "V", "location": "v",
        "start": {"district": "a", "arrival": "porte"},
        "districts": [{"id": "a", "map": "v/a", "guard": {"map": "v/a"}}]})"));
    EXPECT_FALSE(lesDeux.ok());
    EXPECT_NE(lesDeux.error.find("« a »"), std::string::npos) << lesDeux.error;

    const core::CityPlanResult aucun = core::loadCityPlan(ecrire(R"({
        "id": "v", "name": "V", "location": "v",
        "start": {"district": "a", "arrival": "porte"},
        "districts": [{"id": "a", "map": "v/a"}, {"id": "b"}]})"));
    EXPECT_FALSE(aucun.ok());
    EXPECT_NE(aucun.error.find("« b »"), std::string::npos) << aucun.error;
}

/**
 * @brief Un depart dans un quartier sans carte est refuse.
 * \castest{<b>« Nouvelle partie » doit pouvoir poser le heros : le depart a une carte.</b><br/>
 * \tcat Unitaire · Graphe de la ville<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Lire une ville dont le quartier de depart est ferme par une porte gardee.<br/>
 * 2. Lire un fichier absent.<br/>
 * \tattendu Les deux lectures echouent sans lever (EX-NFR-040).
 * }
 */
TEST_F(CityPlanFileTest, UnDepartDansUnQuartierSansCarteEstRefuse) {
    const core::CityPlanResult ferme = core::loadCityPlan(ecrire(R"({
        "id": "v", "name": "V", "location": "v",
        "start": {"district": "b", "arrival": "porte"},
        "districts": [{"id": "a", "map": "v/a"}, {"id": "b", "guard": {"map": "v/a"}}]})"));
    EXPECT_FALSE(ferme.ok());
    EXPECT_TRUE(ferme.plan.startMap().empty());

    core::CityPlanResult absent;
    EXPECT_NO_THROW(absent = core::loadCityPlan(dir / "inexistant.json"));
    EXPECT_FALSE(absent.ok());
}
