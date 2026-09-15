// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <optional>
#include <utility>

#include "HMI/Editor/ThumbnailGeometry.h"

/**
 * @file HMI/Editor/PixelCanvasGeometry.h
 * @brief Conversions pures vue ↔ image du canevas pixel art (LOT-54 TACHE-03, `EX-EDIT-045`).
 *
 * Aucune dépendance Qt/GPU (`EX-NFR-010`) : `hmi::PixelCanvas` (widget) se contente d'appeler ces
 * fonctions, qui restent seules responsables de la justesse à tout zoom, décalage de vue et
 * échelle d'affichage — c'est elle qui détermine le pixel survolé affiché dans la barre d'état
 * (TACHE-04) et doit donc rester juste dans tous les cas.
 */

namespace hmi {

/// Zoom minimal (1 pixel image = 1 pixel écran) et maximal admis par les commandes de zoom du
/// canevas — toujours **entier**, pour que les pixels restent carrés (`EX-ARCH-022`).
inline constexpr int PIXEL_CANVAS_MIN_ZOOM = 1;
inline constexpr int PIXEL_CANVAS_MAX_ZOOM = 32;

/// Facteur de zoom à partir duquel la grille de pixels est dessinée : en-deçà, elle ne ferait que
/// du bruit visuel sur des pixels trop petits pour la distinguer du contenu.
inline constexpr int PIXEL_CANVAS_GRID_VISIBLE_MIN_ZOOM = 4;

/// État de la vue du canevas : zoom entier et décalage de défilement, en pixels **image**.
struct PixelCanvasView {
    /// **Numérateur** du zoom. Nom conservé depuis le `LOT-54` : à dénominateur 1 (le défaut), la
    /// sémantique est strictement celle d'avant, et tout le code qui l'ignore continue de valoir.
    int zoom = 8;
    int panX = 0;  ///< Colonne de l'image affichée au coin haut-gauche du viewport.
    int panY = 0;  ///< Ligne de l'image affichée au coin haut-gauche du viewport.
    /// **Dénominateur** du zoom (`LOT-69` TACHE-07) : 1, 2, 4 ou 8. Permet de descendre **sous**
    /// le 1:1 pour embrasser un plan à l'échelle du niveau entier, ce qu'un zoom entier ne pouvait
    /// pas exprimer. Placé en dernier, et à 1 par défaut, pour que l'atelier pixel art ne bouge pas
    /// d'un pixel : c'est une extension, pas un remplacement.
    int zoomDenominator = 1;

    [[nodiscard]] friend bool operator==(const PixelCanvasView&,
                                         const PixelCanvasView&) noexcept = default;
};

/// Rectangle écran (pixels **logiques**, relatifs au coin haut-gauche du widget) d'un pixel image.
struct PixelScreenRect {
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;
};

/**
 * @brief Rectangle écran du pixel image `(x, y)`, au zoom et décalage de vue donnés.
 *
 * Fonction **pure** : ne dépend que de @p view, jamais des bornes de l'image — c'est elle qui
 * garantit que la grille, le pixel survolé et le tracé de sélection (TACHE-06) s'accordent
 * toujours exactement, puisqu'ils l'appellent tous.
 */
[[nodiscard]] PixelScreenRect imagePixelScreenRect(const PixelCanvasView& view, int x,
                                                   int y) noexcept;

/**
 * @brief Pixel image sous une position écran donnée.
 * @param view        Vue courante (zoom, décalage).
 * @param imageWidth  Largeur de l'image, en pixels.
 * @param imageHeight Hauteur de l'image, en pixels.
 * @param screenX     Position écran, en pixels logiques (ex. `QMouseEvent::position().x()`).
 * @param screenY     Position écran, en pixels logiques.
 * @return La position `(colonne, ligne)`, ou `std::nullopt` si elle tombe hors de l'image —
 *         jamais un indice tronqué au bord.
 */
[[nodiscard]] std::optional<std::pair<int, int>> screenToImagePixel(const PixelCanvasView& view,
                                                                    int imageWidth, int imageHeight,
                                                                    double screenX,
                                                                    double screenY) noexcept;

/// Zoom borné à `[PIXEL_CANVAS_MIN_ZOOM, PIXEL_CANVAS_MAX_ZOOM]`.
[[nodiscard]] constexpr int clampPixelCanvasZoom(int zoom) noexcept {
    if (zoom < PIXEL_CANVAS_MIN_ZOOM) {
        return PIXEL_CANVAS_MIN_ZOOM;
    }
    if (zoom > PIXEL_CANVAS_MAX_ZOOM) {
        return PIXEL_CANVAS_MAX_ZOOM;
    }
    return zoom;
}

/// Zoom suivant (plus grand), borné — toujours entier.
[[nodiscard]] constexpr int pixelCanvasZoomIn(int zoom) noexcept {
    return clampPixelCanvasZoom(zoom + 1);
}

/// Zoom précédent (plus petit), borné — toujours entier.
[[nodiscard]] constexpr int pixelCanvasZoomOut(int zoom) noexcept {
    return clampPixelCanvasZoom(zoom - 1);
}

/// `true` si la grille de pixels doit être dessinée à ce zoom
/// (`PIXEL_CANVAS_GRID_VISIBLE_MIN_ZOOM`).
[[nodiscard]] constexpr bool pixelCanvasGridVisible(int zoom) noexcept {
    return zoom >= PIXEL_CANVAS_GRID_VISIBLE_MIN_ZOOM;
}

/// Dénominateur de zoom maximal : au-delà, un pixel image occuperait moins d'un huitième de
/// pixel écran, et le repère cesserait d'être lisible.
inline constexpr int PIXEL_CANVAS_MAX_DENOMINATOR = 8;

/// @return `true` si @p denominator est une valeur acceptée (1, 2, 4 ou 8).
[[nodiscard]] constexpr bool isValidPixelCanvasDenominator(int denominator) noexcept {
    return denominator == 1 || denominator == 2 || denominator == 4 ||
           denominator == PIXEL_CANVAS_MAX_DENOMINATOR;
}

/**
 * @brief Échelle effective d'une vue : numérateur / dénominateur.
 *
 * Les pixels restent **carrés** par construction — une seule échelle, appliquée aux deux axes
 * (`EX-ARCH-022`). Il n'y a jamais eu, et il ne peut pas y avoir, d'échelle par axe ici.
 * @param view Vue courante.
 * @return Le nombre de pixels écran par pixel image ; `0` si la vue est dégénérée.
 */
[[nodiscard]] constexpr double pixelCanvasScale(const PixelCanvasView& view) noexcept {
    if (view.zoom <= 0 || view.zoomDenominator <= 0) {
        return 0.0;
    }
    return static_cast<double>(view.zoom) / static_cast<double>(view.zoomDenominator);
}

/**
 * @brief Vue **agrandie** d'un cran, en remontant l'échelle rationnelle.
 *
 * L'échelle parcourt `1/8, 1/4, 1/2, 1, 2, 3, …` : on remonte d'abord les dénominateurs jusqu'à 1,
 * puis les numérateurs. Le décalage de vue est conservé tel quel.
 * @param view Vue courante.
 * @return La vue au cran suivant.
 */
[[nodiscard]] constexpr PixelCanvasView pixelCanvasZoomIn(const PixelCanvasView& view) noexcept {
    PixelCanvasView next = view;
    if (next.zoomDenominator > 1) {
        next.zoomDenominator /= 2;
        return next;
    }
    next.zoom = clampPixelCanvasZoom(next.zoom + 1);
    return next;
}

/**
 * @brief Vue **réduite** d'un cran, en descendant l'échelle rationnelle — sous le 1:1 si besoin.
 * @param view Vue courante.
 * @return La vue au cran précédent.
 */
[[nodiscard]] constexpr PixelCanvasView pixelCanvasZoomOut(const PixelCanvasView& view) noexcept {
    PixelCanvasView next = view;
    if (next.zoom > PIXEL_CANVAS_MIN_ZOOM) {
        next.zoom = clampPixelCanvasZoom(next.zoom - 1);
        return next;
    }
    if (next.zoomDenominator < PIXEL_CANVAS_MAX_DENOMINATOR) {
        next.zoomDenominator *= 2;
    }
    return next;
}

/// `true` si la grille de pixels doit être dessinée pour cette vue : jamais **sous** le 1:1, où
/// elle couvrirait plus de surface que les pixels qu'elle sépare.
[[nodiscard]] constexpr bool pixelCanvasGridVisible(const PixelCanvasView& view) noexcept {
    return view.zoomDenominator == 1 && view.zoom >= PIXEL_CANVAS_GRID_VISIBLE_MIN_ZOOM;
}

/// Taille en pixels **réels** de l'image entière affichée (`largeur/hauteur image × zoom`, à
/// l'échelle d'affichage réelle).
struct PixelCanvasRealSize {
    int width = 0;
    int height = 0;
};

/**
 * @brief Taille en pixels réels du canevas entier à un facteur d'échelle d'affichage donné.
 *
 * Réutilise **directement** `hmi::thumbnailPixelSize` (`LOT-56` TACHE-05) — jamais réécrite :
 * c'est la même fonction qui dimensionne les vignettes de la bibliothèque et du panneau Textures,
 * pour que le canevas ne redevienne pas flou à 125 %/150 % le jour où elle évolue sans lui.
 * @param imageWidth  Largeur de l'image, en pixels.
 * @param imageHeight Hauteur de l'image, en pixels.
 * @param zoom        Facteur de zoom entier du canevas.
 * @param scaleFactor Facteur d'échelle du support d'affichage (`devicePixelRatio`).
 */
[[nodiscard]] PixelCanvasRealSize pixelCanvasRealSize(int imageWidth, int imageHeight, int zoom,
                                                      double scaleFactor) noexcept;

/**
 * @brief Taille en pixels réels du canevas entier, à l'échelle **rationnelle** d'une vue.
 * @param imageWidth  Largeur de l'image, en pixels.
 * @param imageHeight Hauteur de l'image, en pixels.
 * @param view        Vue courante (numérateur, dénominateur).
 * @param scaleFactor Facteur d'échelle du support d'affichage (`devicePixelRatio`).
 */
[[nodiscard]] PixelCanvasRealSize pixelCanvasRealSize(int imageWidth, int imageHeight,
                                                      const PixelCanvasView& view,
                                                      double scaleFactor) noexcept;

}  // namespace hmi
