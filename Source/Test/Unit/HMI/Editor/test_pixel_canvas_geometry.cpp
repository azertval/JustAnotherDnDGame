// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_pixel_canvas_geometry.cpp
 * @brief Tests unitaires des conversions pures vue ↔ image du canevas pixel art
 *        (LOT-54 TACHE-03, `EX-EDIT-045`).
 */

#include <gtest/gtest.h>

#include "HMI/Editor/PixelCanvasGeometry.h"
#include "HMI/Editor/ThumbnailGeometry.h"

/**
 * @brief Pour une combinaison de zoom et de décalage de vue, la position de souris donne le bon
 *        pixel image. La conversion ne dépend jamais de l'échelle d'affichage : Qt livre déjà les
 *        positions de souris en pixels **logiques**, indépendamment du `devicePixelRatio`.
 * \castest{<b>screenToImagePixel donne le bon pixel pour diverses combinaisons de zoom et de
 * decalage.</b><br/>
 * \tcat Unitaire · Geometrie du canevas pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire plusieurs vues (zoom, decalage) differentes.<br/>2. Convertir une
 * position ecran connue vers un pixel image.<br/>
 * \tattendu Le pixel obtenu correspond a la case ecran attendue dans chaque cas.
 * }
 */
TEST(PixelCanvasGeometryTest, ScreenToImagePixelPourDiversesVues) {
    const hmi::PixelCanvasView view{/*zoom=*/8, /*panX=*/0, /*panY=*/0};
    EXPECT_EQ(hmi::screenToImagePixel(view, 16, 16, 0.0, 0.0), std::make_pair(0, 0));
    EXPECT_EQ(hmi::screenToImagePixel(view, 16, 16, 20.0, 12.0), std::make_pair(2, 1));

    const hmi::PixelCanvasView zoomed{/*zoom=*/16, /*panX=*/0, /*panY=*/0};
    EXPECT_EQ(hmi::screenToImagePixel(zoomed, 16, 16, 20.0, 12.0), std::make_pair(1, 0));

    const hmi::PixelCanvasView panned{/*zoom=*/8, /*panX=*/3, /*panY=*/1};
    EXPECT_EQ(hmi::screenToImagePixel(panned, 16, 16, 0.0, 0.0), std::make_pair(3, 1));
}

/**
 * @brief Une position hors de l'image produit une absence de pixel, jamais un indice tronqué au
 *        bord — y compris pour une position écran négative.
 * \castest{<b>screenToImagePixel renvoie nullopt hors de l'image, y compris en negatif.</b><br/>
 * \tcat Unitaire · Geometrie du canevas pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Convertir une position ecran negative.<br/>2. Convertir une position au-dela du
 * bord droit/bas de l'image.<br/>
 * \tattendu Les deux conversions renvoient nullopt.
 * }
 */
TEST(PixelCanvasGeometryTest, ScreenToImagePixelHorsBornesRenvoieAbsence) {
    const hmi::PixelCanvasView view{/*zoom=*/8, /*panX=*/0, /*panY=*/0};

    EXPECT_FALSE(hmi::screenToImagePixel(view, 4, 4, -1.0, 0.0).has_value());
    EXPECT_FALSE(hmi::screenToImagePixel(view, 4, 4, 0.0, -8.0).has_value());
    EXPECT_FALSE(hmi::screenToImagePixel(view, 4, 4, 32.0, 0.0).has_value());
}

/**
 * @brief Le centre du rectangle écran d'un pixel, reconverti, donne exactement ce même pixel, à
 *        plusieurs zooms et décalages de vue.
 * \castest{<b>Le centre du rectangle d'un pixel reconverti donne ce meme pixel, a tout zoom.</b>
 * <br/>
 * \tcat Unitaire · Geometrie du canevas pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Pour plusieurs zooms et decalages, calculer le rectangle ecran d'un pixel.<br/>
 * 2. Reconvertir le centre de ce rectangle.<br/>
 * \tattendu Le pixel obtenu est identique au pixel d'origine, dans tous les cas testes.
 * }
 */
TEST(PixelCanvasGeometryTest, AllerRetourRectangleVersPixel) {
    const int zooms[] = {1, 4, 8, 16, 32};
    const hmi::PixelCanvasView pans[] = {
        hmi::PixelCanvasView{1, 0, 0},
        hmi::PixelCanvasView{1, 5, 2},
    };
    for (const int zoom : zooms) {
        for (hmi::PixelCanvasView view : pans) {
            view.zoom = zoom;
            for (int y = 0; y < 4; ++y) {
                for (int x = 0; x < 4; ++x) {
                    const hmi::PixelScreenRect rect = hmi::imagePixelScreenRect(view, x, y);
                    const auto roundTrip = hmi::screenToImagePixel(
                        view, 10, 10, rect.x + rect.width / 2.0, rect.y + rect.height / 2.0);
                    ASSERT_TRUE(roundTrip.has_value())
                        << "zoom=" << zoom << " x=" << x << " y=" << y;
                    EXPECT_EQ(*roundTrip, std::make_pair(x, y))
                        << "zoom=" << zoom << " x=" << x << " y=" << y;
                }
            }
        }
    }
}

/**
 * @brief `pixelCanvasRealSize` réutilise `hmi::thumbnailPixelSize` sans la redéfinir : elle
 *        produit exactement les mêmes valeurs à plusieurs facteurs d'échelle d'affichage.
 * \castest{<b>pixelCanvasRealSize reutilise thumbnailPixelSize (LOT-56) sans la
 * redefinir.</b><br/>
 * \tcat Unitaire · Geometrie du canevas pixel<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Calculer la taille reelle du canevas a plusieurs facteurs d'echelle.<br/>2. Calculer
 * independamment thumbnailPixelSize sur les memes tailles logiques.<br/>
 * \tattendu Les deux resultats sont identiques a chaque facteur.
 * }
 */
TEST(PixelCanvasGeometryTest, PixelCanvasRealSizeReutiliseThumbnailPixelSize) {
    const double factors[] = {1.0, 1.25, 1.5, 2.0};
    constexpr int IMAGE_WIDTH = 16;
    constexpr int IMAGE_HEIGHT = 16;
    constexpr int ZOOM = 8;
    for (const double factor : factors) {
        const hmi::PixelCanvasRealSize real =
            hmi::pixelCanvasRealSize(IMAGE_WIDTH, IMAGE_HEIGHT, ZOOM, factor);
        EXPECT_EQ(real.width, hmi::thumbnailPixelSize(IMAGE_WIDTH * ZOOM, factor)) << factor;
        EXPECT_EQ(real.height, hmi::thumbnailPixelSize(IMAGE_HEIGHT * ZOOM, factor)) << factor;
    }
}

/**
 * @brief Les commandes de zoom avant/arrière restent toujours dans les bornes entières admises,
 *        sans jamais dépasser le zoom maximal ni descendre sous le minimal.
 * \castest{<b>Les commandes de zoom restent bornees et entieres.</b><br/>
 * \tcat Unitaire · Geometrie du canevas pixel<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Zoomer avant a repetition au-dela du maximum.<br/>2. Zoomer arriere a repetition
 * en-deca du minimum.<br/>
 * \tattendu Le zoom reste plafonne a PIXEL_CANVAS_MAX_ZOOM, puis revient et se plafonne a
 * PIXEL_CANVAS_MIN_ZOOM.
 * }
 */
TEST(PixelCanvasGeometryTest, CommandesDeZoomRestentBorneesEtEntieres) {
    int zoom = hmi::PIXEL_CANVAS_MIN_ZOOM;
    for (int i = 0; i < hmi::PIXEL_CANVAS_MAX_ZOOM + 10; ++i) {
        zoom = hmi::pixelCanvasZoomIn(zoom);
    }
    EXPECT_EQ(zoom, hmi::PIXEL_CANVAS_MAX_ZOOM);

    for (int i = 0; i < hmi::PIXEL_CANVAS_MAX_ZOOM + 10; ++i) {
        zoom = hmi::pixelCanvasZoomOut(zoom);
    }
    EXPECT_EQ(zoom, hmi::PIXEL_CANVAS_MIN_ZOOM);
}

/**
 * @brief La grille de pixels n'est visible qu'à partir du zoom seuil, pas en-deçà.
 * \castest{<b>pixelCanvasGridVisible bascule au seuil attendu.</b><br/>
 * \tcat Unitaire · Geometrie du canevas pixel<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Evaluer pixelCanvasGridVisible juste en-deca et juste au niveau du seuil.<br/>
 * \tattendu Faux en-deca, vrai au seuil et au-dela.
 * }
 */
TEST(PixelCanvasGeometryTest, GrilleVisibleAuSeuilAttendu) {
    EXPECT_FALSE(hmi::pixelCanvasGridVisible(hmi::PIXEL_CANVAS_GRID_VISIBLE_MIN_ZOOM - 1));
    EXPECT_TRUE(hmi::pixelCanvasGridVisible(hmi::PIXEL_CANVAS_GRID_VISIBLE_MIN_ZOOM));
}

/**
 * @brief **Zoom rationnel** (`LOT-69` TACHE-07) : l'aller-retour vue ↔ image reste exact aux
 * dénominateurs 2 et 4, c'est-à-dire **sous** le 1:1.
 *
 * L'extension est compatible par construction — le dénominateur vaut 1 par défaut, et tous les cas
 * ci-dessus, écrits avant elle, valent tels quels. Ce test-ci couvre le terrain nouveau.
 * \castest{<b>L'aller-retour vue vers image reste exact sous le 1:1.</b><br/>
 * \tcat Unitaire · Geometrie du canevas pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Pour les denominateurs 2 et 4, convertir chaque pixel image en rectangle ecran.<br/>
 * 2. Reconvertir le centre de ce rectangle en pixel image.<br/>
 * \tattendu Le pixel de depart est retrouve a chaque fois.
 * }
 */
TEST(PixelCanvasGeometryTest, AllerRetourExactSousLeUnPourUn) {
    for (const int denominator : {2, 4}) {
        hmi::PixelCanvasView view{/*zoom=*/8, /*panX=*/0, /*panY=*/0};
        view.zoomDenominator = denominator;
        ASSERT_TRUE(hmi::isValidPixelCanvasDenominator(denominator));

        for (int y = 0; y < 6; ++y) {
            for (int x = 0; x < 6; ++x) {
                const hmi::PixelScreenRect rect = hmi::imagePixelScreenRect(view, x, y);
                const std::optional<std::pair<int, int>> back = hmi::screenToImagePixel(
                    view, 16, 16, rect.x + rect.width / 2.0, rect.y + rect.height / 2.0);
                ASSERT_TRUE(back.has_value()) << "denominateur " << denominator;
                EXPECT_EQ(back->first, x);
                EXPECT_EQ(back->second, y);
            }
        }
    }
}

/**
 * @brief Les pixels restent **carrés** à tout zoom, dénominateur compris (`EX-ARCH-022`).
 * \castest{<b>Les pixels restent carres a tout zoom.</b><br/>
 * \tcat Unitaire · Geometrie du canevas pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Parcourir plusieurs numerateurs et les quatre denominateurs admis.<br/>2. Comparer
 * largeur et hauteur du rectangle ecran d'un pixel.<br/>
 * \tattendu Largeur et hauteur sont egales dans tous les cas.
 * }
 */
TEST(PixelCanvasGeometryTest, PixelsCarresATousLesZooms) {
    for (const int numerator : {1, 3, 8, 32}) {
        for (const int denominator : {1, 2, 4, 8}) {
            hmi::PixelCanvasView view{numerator, 0, 0};
            view.zoomDenominator = denominator;
            const hmi::PixelScreenRect rect = hmi::imagePixelScreenRect(view, 2, 3);
            EXPECT_DOUBLE_EQ(rect.width, rect.height) << numerator << '/' << denominator;
            EXPECT_DOUBLE_EQ(hmi::pixelCanvasScale(view),
                             static_cast<double>(numerator) / denominator);
        }
    }
}

/**
 * @brief L'échelle descend **sous** le 1:1 puis y remonte : la course de zoom parcourt les
 * dénominateurs avant les numérateurs, et se borne aux deux extrémités.
 * \castest{<b>La course de zoom descend sous le 1:1 et y remonte.</b><br/>
 * \tcat Unitaire · Geometrie du canevas pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Reduire depuis le 1:1 jusqu'a la borne basse.<br/>2. Agrandir jusqu'a repasser au
 * 1:1.<br/>
 * \tattendu L'echelle atteint 1/8 sans aller plus bas, puis revient exactement a 1.
 * }
 */
TEST(PixelCanvasGeometryTest, CourseDeZoomRationnelleBornee) {
    hmi::PixelCanvasView view{hmi::PIXEL_CANVAS_MIN_ZOOM, 0, 0};

    for (int step = 0; step < 10; ++step) {
        view = hmi::pixelCanvasZoomOut(view);
    }
    EXPECT_EQ(view.zoomDenominator, hmi::PIXEL_CANVAS_MAX_DENOMINATOR);
    EXPECT_DOUBLE_EQ(hmi::pixelCanvasScale(view), 1.0 / hmi::PIXEL_CANVAS_MAX_DENOMINATOR);
    // La grille de pixels n'a plus de sens sous le 1:1 : elle couvrirait plus que les pixels
    // qu'elle separe.
    EXPECT_FALSE(hmi::pixelCanvasGridVisible(view));

    for (int step = 0; step < 3; ++step) {
        view = hmi::pixelCanvasZoomIn(view);
    }
    EXPECT_EQ(view.zoomDenominator, 1);
    EXPECT_DOUBLE_EQ(hmi::pixelCanvasScale(view), 1.0);
}
