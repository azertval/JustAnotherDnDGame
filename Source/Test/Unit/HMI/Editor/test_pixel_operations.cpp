// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_pixel_operations.cpp
 * @brief Tests unitaires des opérations pures sur tampon de pixels (LOT-54 TACHE-02,
 *        `EX-EDIT-045`).
 */

#include <cstdint>
#include <optional>
#include <vector>

#include <gtest/gtest.h>

#include "HMI/Editor/PixelOperations.h"

namespace {

constexpr std::uint32_t RED = 0xFF0000FFu;
constexpr std::uint32_t GREEN = 0xFF00FF00u;
constexpr std::uint32_t BLUE = 0xFFFF0000u;
constexpr std::uint32_t TRANSPARENT_BLACK = 0x00000000u;

// Image width x height uniformement de couleur color.
hmi::DecodedImage uniformImage(int width, int height, std::uint32_t color) {
    hmi::DecodedImage image;
    image.width = width;
    image.height = height;
    image.pixels.assign(static_cast<std::size_t>(width) * static_cast<std::size_t>(height), color);
    return image;
}

}  // namespace

/**
 * @brief Poser un pixel dans les bornes change sa couleur et renvoie ce seul pixel comme région
 *        modifiée ; hors bornes, l'image n'est pas touchée et la région renvoyée est vide.
 * \castest{<b>setPixel change la couleur du pixel vise et renvoie la region attendue.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Poser un pixel dans les bornes d'une image.<br/>2. Poser un pixel hors bornes.<br/>
 * \tattendu Le premier change la couleur et renvoie la region (x,y)-(x,y) ; le second ne change
 * rien et renvoie une region vide.
 * }
 */
TEST(PixelOperationsTest, SetPixelChangeLaCouleurEtRenvoieLaRegion) {
    hmi::DecodedImage image = uniformImage(4, 4, RED);

    const hmi::PixelRegion region = hmi::setPixel(image, 1, 2, GREEN);
    EXPECT_FALSE(region.empty());
    EXPECT_EQ(region.minX, 1);
    EXPECT_EQ(region.maxX, 1);
    EXPECT_EQ(region.minY, 2);
    EXPECT_EQ(region.maxY, 2);
    EXPECT_EQ(hmi::pickColor(image, 1, 2), GREEN);

    const hmi::PixelRegion outOfBounds = hmi::setPixel(image, 10, 10, GREEN);
    EXPECT_TRUE(outOfBounds.empty());
}

/**
 * @brief Effacer un pixel met son alpha à zéro sans changer sa teinte RVB.
 * \castest{<b>erasePixel met l'alpha a zero sans changer la teinte RVB.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Effacer un pixel colore.<br/>2. Lire sa couleur.<br/>
 * \tattendu Le canal alpha est nul, les canaux RVB sont inchanges.
 * }
 */
TEST(PixelOperationsTest, ErasePixelPreserveLaTeinteRvb) {
    hmi::DecodedImage image = uniformImage(2, 2, RED);

    hmi::erasePixel(image, 0, 0);
    const std::optional<std::uint32_t> erased = hmi::pickColor(image, 0, 0);
    ASSERT_TRUE(erased.has_value());
    EXPECT_EQ(*erased & 0xFF000000u, 0u) << "alpha doit etre nul";
    EXPECT_EQ(*erased & 0x00FFFFFFu, RED & 0x00FFFFFFu) << "la teinte RVB doit etre conservee";
}

/**
 * @brief La pipette renvoie la couleur du pixel visé, ou `nullopt` hors bornes.
 * \castest{<b>pickColor renvoie la couleur du pixel ou nullopt hors bornes.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Poser une couleur connue sur un pixel.<br/>2. La prelever avec pickColor.<br/>
 * 3. Prelever hors bornes.<br/>
 * \tattendu La couleur prelevee correspond ; hors bornes, le resultat est absent.
 * }
 */
TEST(PixelOperationsTest, PickColorRenvoieLaCouleurOuAbsentHorsBornes) {
    hmi::DecodedImage image = uniformImage(3, 3, BLUE);

    EXPECT_EQ(hmi::pickColor(image, 1, 1), BLUE);
    EXPECT_FALSE(hmi::pickColor(image, -1, 0).has_value());
    EXPECT_FALSE(hmi::pickColor(image, 3, 0).has_value());
}

/**
 * @brief Un glisser rapide (diagonale) ne laisse aucun trou : tous les pixels du segment sont
 *        peints.
 * \castest{<b>drawLine peint un segment diagonal sans trou.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Tracer une ligne diagonale entre deux positions eloignees.<br/>2. Lire chaque pixel
 * du chemin attendu.<br/>
 * \tattendu Tous les pixels du chemin diagonal sont de la couleur tracee.
 * }
 */
TEST(PixelOperationsTest, DrawLineDiagonaleSansTrou) {
    hmi::DecodedImage image = uniformImage(6, 6, TRANSPARENT_BLACK);

    const hmi::PixelRegion region = hmi::drawLine(image, 0, 0, 5, 5, GREEN);
    EXPECT_EQ(region.minX, 0);
    EXPECT_EQ(region.minY, 0);
    EXPECT_EQ(region.maxX, 5);
    EXPECT_EQ(region.maxY, 5);
    for (int i = 0; i < 6; ++i) {
        EXPECT_EQ(hmi::pickColor(image, i, i), GREEN) << "pixel diagonal " << i;
    }
}

/**
 * @brief Un tracé purement vertical peint chaque ligne du segment, sans trou.
 * \castest{<b>drawLine peint un segment purement vertical sans trou.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Tracer une ligne verticale.<br/>2. Lire chaque pixel de la colonne.<br/>
 * \tattendu Tous les pixels de la colonne sont de la couleur tracee.
 * }
 */
TEST(PixelOperationsTest, DrawLineVerticaleSansTrou) {
    hmi::DecodedImage image = uniformImage(3, 6, TRANSPARENT_BLACK);

    hmi::drawLine(image, 1, 0, 1, 5, RED);
    for (int y = 0; y < 6; ++y) {
        EXPECT_EQ(hmi::pickColor(image, 1, y), RED) << "ligne " << y;
    }
    EXPECT_EQ(hmi::pickColor(image, 0, 3), TRANSPARENT_BLACK) << "colonne voisine non touchee";
}

/**
 * @brief Un glisser rapide avec la gomme efface toute la ligne sans laisser de trou, en préservant
 *        la teinte RVB des pixels effacés (même contrat que `erasePixel`).
 * \castest{<b>eraseLine efface un segment complet sans trou.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Effacer une ligne diagonale sur une image coloree.<br/>2. Lire chaque pixel du
 * chemin.<br/>
 * \tattendu Chaque pixel du chemin a un alpha nul et sa teinte RVB d'origine.
 * }
 */
TEST(PixelOperationsTest, EraseLineEffaceSansTrou) {
    hmi::DecodedImage image = uniformImage(5, 5, RED);

    const hmi::PixelRegion region = hmi::eraseLine(image, 0, 0, 4, 4);
    EXPECT_EQ(region, (hmi::PixelRegion{0, 0, 4, 4}));
    for (int i = 0; i < 5; ++i) {
        const std::optional<std::uint32_t> pixel = hmi::pickColor(image, i, i);
        ASSERT_TRUE(pixel.has_value());
        EXPECT_EQ(*pixel & 0xFF000000u, 0u) << "pixel " << i;
        EXPECT_EQ(*pixel & 0x00FFFFFFu, RED & 0x00FFFFFFu) << "pixel " << i;
    }
}

/**
 * @brief Le remplissage d'une zone fermée colore exactement cette zone, sans déborder au-delà de
 *        sa frontière.
 * \castest{<b>floodFill remplit une zone fermee sans deborder.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire une image avec un carre interieur cerne d'une bordure d'une autre
 * couleur.<br/>2. Remplir depuis le centre du carre.<br/>
 * \tattendu Seul l'interieur change de couleur ; la bordure reste intacte.
 * }
 */
TEST(PixelOperationsTest, FloodFillZoneFermeeNeDeborde) {
    hmi::DecodedImage image = uniformImage(5, 5, BLUE);
    // Bordure bleue conservee ; interieur 3x3 rouge, rempli en vert depuis son centre.
    for (int y = 1; y <= 3; ++y) {
        for (int x = 1; x <= 3; ++x) {
            hmi::setPixel(image, x, y, RED);
        }
    }

    hmi::floodFill(image, 2, 2, GREEN);

    for (int y = 1; y <= 3; ++y) {
        for (int x = 1; x <= 3; ++x) {
            EXPECT_EQ(hmi::pickColor(image, x, y), GREEN) << x << "," << y;
        }
    }
    // La bordure (tout pixel a la frontiere du carre 5x5) reste bleue.
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(hmi::pickColor(image, i, 0), BLUE);
        EXPECT_EQ(hmi::pickColor(image, i, 4), BLUE);
        EXPECT_EQ(hmi::pickColor(image, 0, i), BLUE);
        EXPECT_EQ(hmi::pickColor(image, 4, i), BLUE);
    }
}

/**
 * @brief Une zone ouverte débouchant sur un bord de l'image est remplie jusqu'aux bornes, sans
 *        écriture hors limites.
 * \castest{<b>floodFill remplit une zone ouverte jusqu'au bord sans deborder du tampon.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire une image entierement d'une couleur (zone "ouverte" par construction :
 * aucune bordure).<br/>2. Remplir depuis un coin.<br/>
 * \tattendu Toute l'image change de couleur, sans exception ni ecriture hors bornes.
 * }
 */
TEST(PixelOperationsTest, FloodFillZoneOuverteRempliJusquAuBord) {
    hmi::DecodedImage image = uniformImage(4, 3, RED);

    const hmi::PixelRegion region = hmi::floodFill(image, 0, 0, GREEN);

    EXPECT_EQ(region.minX, 0);
    EXPECT_EQ(region.minY, 0);
    EXPECT_EQ(region.maxX, 3);
    EXPECT_EQ(region.maxY, 2);
    for (const std::uint32_t pixel : image.pixels) {
        EXPECT_EQ(pixel, GREEN);
    }
}

/**
 * @brief Remplir sur la couleur déjà présente ne change rien et ne boucle jamais.
 * \castest{<b>floodFill sur la couleur deja presente est un no-op sans boucle infinie.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire une grande image uniforme.<br/>2. La remplir avec sa propre
 * couleur.<br/>
 * \tattendu Le test se termine (pas de boucle infinie), l'image est inchangee et la region
 * renvoyee est vide.
 * }
 */
TEST(PixelOperationsTest, FloodFillCouleurDejaPresenteNeChangeRien) {
    hmi::DecodedImage image = uniformImage(50, 50, BLUE);

    const hmi::PixelRegion region = hmi::floodFill(image, 25, 25, BLUE);

    EXPECT_TRUE(region.empty());
    for (const std::uint32_t pixel : image.pixels) {
        EXPECT_EQ(pixel, BLUE);
    }
}

/**
 * @brief Le remplissage fonctionne sur une image d'un seul pixel.
 * \castest{<b>floodFill fonctionne sur une image d'un seul pixel.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Construire une image 1x1.<br/>2. La remplir d'une autre couleur.<br/>
 * \tattendu L'unique pixel change de couleur, region = (0,0)-(0,0).
 * }
 */
TEST(PixelOperationsTest, FloodFillImageUnPixel) {
    hmi::DecodedImage image = uniformImage(1, 1, RED);

    const hmi::PixelRegion region = hmi::floodFill(image, 0, 0, GREEN);

    EXPECT_EQ(region, (hmi::PixelRegion{0, 0, 0, 0}));
    EXPECT_EQ(hmi::pickColor(image, 0, 0), GREEN);
}

/**
 * @brief Le remplissage d'une image entièrement d'une seule couleur la recolore intégralement.
 * \castest{<b>floodFill sur une image entierement uniforme la recolore integralement.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une image uniforme.<br/>2. La remplir d'une autre couleur depuis un
 * pixel quelconque.<br/>
 * \tattendu Tous les pixels sont de la nouvelle couleur.
 * }
 */
TEST(PixelOperationsTest, FloodFillImageEntierementUniformeRecoloreTout) {
    hmi::DecodedImage image = uniformImage(8, 8, RED);

    hmi::floodFill(image, 4, 4, GREEN);

    for (const std::uint32_t pixel : image.pixels) {
        EXPECT_EQ(pixel, GREEN);
    }
}

/**
 * @brief L'union de deux régions donne la plus petite région couvrant les deux ; une région vide
 *        n'affecte pas l'union.
 * \castest{<b>unionPixelRegion couvre les deux regions, une region vide etant neutre.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Unir deux regions disjointes.<br/>2. Unir une region avec une region vide.<br/>
 * \tattendu Le premier resultat englobe les deux ; le second est identique a la region non vide.
 * }
 */
TEST(PixelOperationsTest, UnionPixelRegionCouvreLesDeuxRegions) {
    const hmi::PixelRegion a{0, 0, 2, 2};
    const hmi::PixelRegion b{5, 5, 7, 7};

    EXPECT_EQ(hmi::unionPixelRegion(a, b), (hmi::PixelRegion{0, 0, 7, 7}));
    EXPECT_EQ(hmi::unionPixelRegion(a, hmi::PixelRegion{}), a);
    EXPECT_EQ(hmi::unionPixelRegion(hmi::PixelRegion{}, b), b);
}

// Image 4x3 de valeurs distinctes (index de pixel encode en couleur), pour verifier une
// transformation geometrique pixel par pixel sans ambiguite.
namespace {
hmi::DecodedImage indexedImage(int width, int height) {
    hmi::DecodedImage image;
    image.width = width;
    image.height = height;
    image.pixels.resize(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));
    for (std::size_t i = 0; i < image.pixels.size(); ++i) {
        image.pixels[i] = 0xFF000000u | static_cast<std::uint32_t>(i);
    }
    return image;
}
}  // namespace

/**
 * @brief Deux symétries horizontales successives restituent exactement l'image d'origine, avec et
 *        sans sélection active (région partielle et image entière).
 * \castest{<b>Deux symetries horizontales restituent l'image d'origine.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Appliquer flipHorizontal deux fois sur une sous-region.<br/>2. Repeter sur l'image
 * entiere.<br/>
 * \tattendu Les pixels sont identiques a l'original dans les deux cas.
 * }
 */
TEST(PixelOperationsTest, DeuxSymetriesHorizontalesRestituentLOrigine) {
    const hmi::DecodedImage original = indexedImage(6, 4);

    hmi::DecodedImage partial = original;
    const hmi::PixelRegion subRegion{1, 1, 4, 2};
    hmi::flipHorizontal(partial, subRegion);
    hmi::flipHorizontal(partial, subRegion);
    EXPECT_EQ(partial.pixels, original.pixels) << "sous-region (avec selection)";

    hmi::DecodedImage whole = original;
    const hmi::PixelRegion wholeRegion{0, 0, 5, 3};
    hmi::flipHorizontal(whole, wholeRegion);
    hmi::flipHorizontal(whole, wholeRegion);
    EXPECT_EQ(whole.pixels, original.pixels) << "image entiere (sans selection)";
}

/**
 * @brief Deux symétries verticales successives restituent exactement l'image d'origine.
 * \castest{<b>Deux symetries verticales restituent l'image d'origine.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Appliquer flipVertical deux fois sur une sous-region.<br/>
 * \tattendu Les pixels sont identiques a l'original.
 * }
 */
TEST(PixelOperationsTest, DeuxSymetriesVerticalesRestituentLOrigine) {
    const hmi::DecodedImage original = indexedImage(5, 6);
    hmi::DecodedImage image = original;
    const hmi::PixelRegion region{0, 1, 4, 4};

    hmi::flipVertical(image, region);
    hmi::flipVertical(image, region);

    EXPECT_EQ(image.pixels, original.pixels);
}

/**
 * @brief Quatre rotations d'un quart de tour dans le même sens restituent l'image d'origine sur
 *        une région carrée.
 * \castest{<b>Quatre rotations restituent l'image d'origine sur une region carree.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Appliquer rotateClockwise quatre fois sur une region carree.<br/>
 * \tattendu Les pixels de la region sont identiques a l'original apres le cycle complet.
 * }
 */
TEST(PixelOperationsTest, QuatreRotationsRestituentLOrigineSurRegionCarree) {
    const hmi::DecodedImage original = indexedImage(6, 6);
    hmi::DecodedImage image = original;
    const hmi::PixelRegion region{1, 1, 4, 4};  // 4x4, carree.

    for (int i = 0; i < 4; ++i) {
        hmi::rotateClockwise(image, region);
    }

    EXPECT_EQ(image.pixels, original.pixels);
}

/**
 * @brief Sur une région non carrée, la rotation opère dans le rectangle englobant et le
 *        débordement est tronqué au cadre de l'image, jamais écrit hors bornes.
 * \castest{<b>Rotation non carree : le debordement est tronque au cadre.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Pivoter une region rectangulaire proche du bord de l'image.<br/>2. Verifier que
 * l'image garde sa taille et qu'aucune ecriture n'a debord du tampon.<br/>
 * \tattendu Le vecteur de pixels garde exactement sa taille d'origine (aucun redimensionnement,
 * aucun crash).
 * }
 */
TEST(PixelOperationsTest, RotationNonCarreeTronqueLeDebordement) {
    hmi::DecodedImage image = indexedImage(5, 5);
    const std::size_t originalSize = image.pixels.size();
    // Region 4 (largeur) x 2 (hauteur) collee au bord bas : pivotee, elle devient 2 (largeur) x 4
    // (hauteur), ancree au meme coin haut-gauche (0,3) -- la nouvelle hauteur (4) deborderait du
    // cadre 5x5 des la ligne 5 (lignes valides 0..4).
    const hmi::PixelRegion region{0, 3, 3, 4};

    EXPECT_NO_THROW(hmi::rotateClockwise(image, region));

    EXPECT_EQ(image.pixels.size(), originalSize);
    EXPECT_EQ(image.width, 5);
    EXPECT_EQ(image.height, 5);
}

/**
 * @brief Coller un presse-papiers près d'un bord tronque au cadre sans écriture hors limites.
 * \castest{<b>Coller pres d'un bord tronque sans ecriture hors limites.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Copier une region.<br/>2. La coller a une position debordant du cadre.<br/>
 * \tattendu Aucune exception ; la taille du tampon est inchangee ; les pixels dans les bornes sont
 * ecrits.
 * }
 */
TEST(PixelOperationsTest, CollerPresDUnBordTronqueSansDeborder) {
    hmi::DecodedImage image = indexedImage(4, 4);
    const std::size_t originalSize = image.pixels.size();
    const hmi::PixelClipboard clip = hmi::copyRegion(image, hmi::PixelRegion{0, 0, 1, 1});

    hmi::PixelRegion touched{};
    EXPECT_NO_THROW(touched = hmi::pasteClipboard(image, clip, 3, 3));

    EXPECT_EQ(image.pixels.size(), originalSize);
    // Seul le pixel (3,3) tombe dans les bornes ; le reste du 2x2 colle deborderait.
    EXPECT_EQ(touched, (hmi::PixelRegion{3, 3, 3, 3}));
}

/**
 * @brief Une transformation sans région (région vide) est sans effet et ne produit pas d'entrée
 *        d'historique (région renvoyée vide).
 * \castest{<b>Une transformation sur une region vide est sans effet.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Appliquer flipHorizontal, rotateClockwise et moveRegion sur une region vide.<br/>
 * \tattendu Chaque appel renvoie une region vide ; l'image reste inchangee.
 * }
 */
TEST(PixelOperationsTest, TransformationSurRegionVideEstSansEffet) {
    const hmi::DecodedImage original = indexedImage(3, 3);
    hmi::DecodedImage image = original;
    const hmi::PixelRegion empty{};

    EXPECT_TRUE(hmi::flipHorizontal(image, empty).empty());
    EXPECT_TRUE(hmi::rotateClockwise(image, empty).empty());
    EXPECT_TRUE(hmi::moveRegion(image, empty, 1, 1).empty());
    EXPECT_EQ(image.pixels, original.pixels);
}

/**
 * @brief Déplacer une région laisse la zone quittée transparente ; un déplacement entièrement hors
 *        cadre efface la zone de départ sans rien reposer de visible, sans tampon corrompu.
 * \castest{<b>Deplacer une region laisse la zone quittee transparente.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Deplacer une region a l'interieur du cadre.<br/>2. Verifier la zone quittee et la
 * zone reposee.<br/>3. Deplacer une region entierement hors cadre.<br/>
 * \tattendu La zone quittee est transparente ; le deplacement hors cadre n'ecrit rien de visible et
 * ne leve pas d'exception.
 * }
 */
TEST(PixelOperationsTest, DeplacerUneRegionLaisseLaZoneQuitteeTransparente) {
    hmi::DecodedImage image = uniformImage(6, 6, RED);
    hmi::setPixel(image, 1, 1, GREEN);

    hmi::moveRegion(image, hmi::PixelRegion{1, 1, 1, 1}, 3, 3);

    EXPECT_EQ(hmi::pickColor(image, 1, 1).value() & 0xFF000000u, 0u) << "zone quittee transparente";
    EXPECT_EQ(hmi::pickColor(image, 4, 4), GREEN) << "contenu repose au decalage";

    hmi::DecodedImage farImage = uniformImage(4, 4, RED);
    EXPECT_NO_THROW(hmi::moveRegion(farImage, hmi::PixelRegion{0, 0, 1, 1}, 100, 100));
    EXPECT_EQ(farImage.pixels.size(), 16U);
}

/**
 * @brief `readRegion` puis `writeRegion` restituent exactement le contenu d'origine — l'aller-
 *        retour dont dépend `hmi::PixelHistory` pour annuler/refaire.
 * \castest{<b>readRegion puis writeRegion restituent le contenu d'origine.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Lire une region d'une image.<br/>2. Modifier l'image dans cette region.<br/>
 * 3. Reecrire les pixels lus a l'etape 1.<br/>
 * \tattendu L'image retrouve exactement son contenu d'origine dans la region.
 * }
 */
TEST(PixelOperationsTest, ReadRegionPuisWriteRegionRestitueLeContenu) {
    hmi::DecodedImage image = uniformImage(5, 5, RED);
    const hmi::PixelRegion region{1, 1, 3, 3};

    const std::vector<std::uint32_t> before = hmi::readRegion(image, region);
    EXPECT_EQ(before.size(), 9U);

    hmi::floodFill(image, 2, 2, GREEN);
    ASSERT_EQ(hmi::pickColor(image, 2, 2), GREEN);

    hmi::writeRegion(image, region, before);
    for (int y = region.minY; y <= region.maxY; ++y) {
        for (int x = region.minX; x <= region.maxX; ++x) {
            EXPECT_EQ(hmi::pickColor(image, x, y), RED) << x << "," << y;
        }
    }
}
