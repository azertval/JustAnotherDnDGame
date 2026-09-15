// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "HMI/Graphics/TextureLoader.h"

/**
 * @file HMI/Editor/PixelOperations.h
 * @brief Opérations pures sur un tampon de pixels RGBA (atelier pixel art, LOT-54 TACHE-02).
 *
 * Fonctions **libres**, sans état, sans Qt ni GPU (`EX-NFR-010`) : ce que fait chaque outil du
 * canevas (TACHE-03) sur `hmi::DecodedImage` — le même type que `decodeImageFile`/`encodeImageFile`
 * (TACHE-01), pour que le canevas édite, charge et enregistre le même tampon sans conversion.
 */

namespace hmi {

/**
 * @brief Rectangle de pixels modifié par une opération (bornes incluses).
 *
 * Sert à borner le coût mémoire de l'historique (`hmi::PixelHistory`) : une entrée mémorise le
 * contenu de cette région avant/après plutôt qu'une copie complète de l'image.
 */
struct PixelRegion {
    int minX = 0;
    int minY = 0;
    int maxX = -1;  ///< `maxX < minX` (valeur par défaut) : région vide.
    int maxY = -1;

    /// @return `true` si la région ne couvre aucun pixel.
    [[nodiscard]] constexpr bool empty() const noexcept {
        return maxX < minX || maxY < minY;
    }
    /// @return Largeur de la région, `0` si vide.
    [[nodiscard]] constexpr int width() const noexcept {
        return empty() ? 0 : (maxX - minX + 1);
    }
    /// @return Hauteur de la région, `0` si vide.
    [[nodiscard]] constexpr int height() const noexcept {
        return empty() ? 0 : (maxY - minY + 1);
    }

    /// @return `true` si les deux régions couvrent exactement les mêmes pixels.
    [[nodiscard]] constexpr bool operator==(const PixelRegion& other) const noexcept {
        return (empty() && other.empty()) || (minX == other.minX && minY == other.minY &&
                                              maxX == other.maxX && maxY == other.maxY);
    }
};

/// @return `true` si `(x, y)` est dans les bornes d'une image `width` x `height`.
[[nodiscard]] constexpr bool pixelInBounds(int width, int height, int x, int y) noexcept {
    return x >= 0 && y >= 0 && x < width && y < height;
}

/**
 * @brief Union de deux régions (la plus petite région couvrant les deux).
 *
 * Une région vide n'affecte pas l'union : sert à accumuler la région touchée par un geste complet
 * (`hmi::PixelCanvas`, TACHE-03) à partir des régions renvoyées par chacune de ses sous-opérations
 * (ex. plusieurs `drawLine` successifs pendant un glisser).
 */
[[nodiscard]] constexpr PixelRegion unionPixelRegion(const PixelRegion& a,
                                                     const PixelRegion& b) noexcept {
    if (a.empty()) {
        return b;
    }
    if (b.empty()) {
        return a;
    }
    return PixelRegion{
        a.minX < b.minX ? a.minX : b.minX,
        a.minY < b.minY ? a.minY : b.minY,
        a.maxX > b.maxX ? a.maxX : b.maxX,
        a.maxY > b.maxY ? a.maxY : b.maxY,
    };
}

/**
 * @brief Pose un pixel d'une couleur donnée.
 * @param image Image modifiée en place.
 * @param x     Colonne (0-based).
 * @param y     Ligne (0-based).
 * @param color Couleur `R8G8B8A8_UNORM` posée.
 * @return La région modifiée (le seul pixel `(x, y)`), vide si `(x, y)` est hors bornes.
 */
PixelRegion setPixel(DecodedImage& image, int x, int y, std::uint32_t color);

/**
 * @brief Efface un pixel (alpha nul), sans changer sa couleur RVB.
 *
 * Alpha nul, RVB **conservé** plutôt que mis à zéro : un pixel effacé puis re-coloré sans jamais
 * changer sa teinte reproduit l'attente naturelle d'une gomme.
 * @param image Image modifiée en place.
 * @param x     Colonne (0-based).
 * @param y     Ligne (0-based).
 * @return La région modifiée (le seul pixel `(x, y)`), vide si `(x, y)` est hors bornes.
 */
PixelRegion erasePixel(DecodedImage& image, int x, int y);

/**
 * @brief Trace une ligne pleine entre deux positions (algorithme de Bresenham).
 *
 * Sans elle, un glisser rapide de souris entre deux positions successives d'un même geste laisse
 * des pixels isolés au lieu d'un trait continu — le défaut le plus visible d'un canevas pixel art
 * fait naïvement.
 * @param image Image modifiée en place.
 * @param x0    Colonne de départ.
 * @param y0    Ligne de départ.
 * @param x1    Colonne d'arrivée.
 * @param y1    Ligne d'arrivée.
 * @param color Couleur posée sur chaque pixel du tracé.
 * @return La région englobante des pixels effectivement posés (vide si tout le segment est hors
 *         bornes).
 */
PixelRegion drawLine(DecodedImage& image, int x0, int y0, int x1, int y1, std::uint32_t color);

/**
 * @brief Efface une ligne pleine entre deux positions (même tracé que `drawLine`, alpha nul plutôt
 *        qu'une couleur posée).
 *
 * Pendant de `drawLine` pour la gomme : un glisser rapide avec la gomme doit laisser une trace
 * continue effacée, pas des pixels effacés isolés — même défaut, même correction.
 * @param image Image modifiée en place.
 * @param x0    Colonne de départ.
 * @param y0    Ligne de départ.
 * @param x1    Colonne d'arrivée.
 * @param y1    Ligne d'arrivée.
 * @return La région englobante des pixels effectivement effacés (vide si tout le segment est hors
 *         bornes).
 */
PixelRegion eraseLine(DecodedImage& image, int x0, int y0, int x1, int y1);

/**
 * @brief Remplit, de proche en proche, la zone connexe (4-connexité) de même couleur que le pixel
 *        de départ.
 *
 * Itératif (file explicite), **jamais récursif** : une récursion sur une grande zone déborderait
 * la pile. Sans effet — et sans parcourir quoi que ce soit — si `(x, y)` est hors bornes ou si la
 * couleur de départ est déjà @p color, ce qui évite à la fois une opération d'historique vide et
 * une boucle sans fin sur une image déjà uniforme.
 * @param image Image modifiée en place.
 * @param x     Colonne du point de départ.
 * @param y     Ligne du point de départ.
 * @param color Couleur de remplissage.
 * @return La région englobante des pixels effectivement remplis (vide si sans effet).
 */
PixelRegion floodFill(DecodedImage& image, int x, int y, std::uint32_t color);

/**
 * @brief Couleur du pixel à une position donnée (outil pipette).
 * @param image Image consultée.
 * @param x     Colonne.
 * @param y     Ligne.
 * @return La couleur du pixel, ou `std::nullopt` si `(x, y)` est hors bornes.
 */
[[nodiscard]] std::optional<std::uint32_t> pickColor(const DecodedImage& image, int x,
                                                     int y) noexcept;

/**
 * @brief Copie les pixels d'une région, ligne par ligne — capture le « avant » ou l'« après »
 *        d'une entrée d'historique (`hmi::PixelHistory`).
 * @param image  Image consultée.
 * @param region Région à copier (doit être contenue dans les bornes de @p image).
 * @return Les pixels de la région, taille `region.width() * region.height()` ; vide si
 *         @p region est vide.
 */
[[nodiscard]] std::vector<std::uint32_t> readRegion(const DecodedImage& image,
                                                    const PixelRegion& region);

/**
 * @brief Écrit des pixels dans une région — inverse de `readRegion`, utilisé par
 *        `hmi::PixelHistory` pour restaurer un instantané annulé/rétabli.
 * @param image  Image modifiée en place.
 * @param region Région ciblée (doit être contenue dans les bornes de @p image).
 * @param data   Pixels à écrire, taille attendue `region.width() * region.height()`.
 */
void writeRegion(DecodedImage& image, const PixelRegion& region,
                 const std::vector<std::uint32_t>& data);

/**
 * @brief Contenu copié depuis un tampon (presse-papiers de l'atelier, `LOT-54` TACHE-06).
 *
 * **Distinct** du presse-papiers de l'éditeur de niveaux : deux tampons séparés, visés selon le
 * contexte d'édition actif (`hmi::PixelCanvas`, comme Annuler — `EX-IHM-062`).
 */
struct PixelClipboard {
    int width = 0;
    int height = 0;
    std::vector<std::uint32_t> pixels;

    /// @return `true` si le presse-papiers ne porte aucun contenu.
    [[nodiscard]] bool empty() const noexcept {
        return width <= 0 || height <= 0;
    }
};

/**
 * @brief Retourne une région horizontalement (miroir gauche-droite), en place.
 *
 * Involutive : appliquer deux fois de suite restitue exactement le contenu d'origine.
 * @param image  Image modifiée en place.
 * @param region Région retournée ; sans effet si vide.
 * @return @p region telle quelle si non vide, sinon une région vide — jamais d'entrée d'historique
 *         pour une région vide (à la charge de l'appelant de ne pas en pousser une).
 */
PixelRegion flipHorizontal(DecodedImage& image, const PixelRegion& region);

/// Retourne une région verticalement (miroir haut-bas), en place. Involutive, comme
/// `flipHorizontal`.
PixelRegion flipVertical(DecodedImage& image, const PixelRegion& region);

/**
 * @brief Pivote le contenu d'une région d'un quart de tour, sens horaire.
 *
 * La région d'origine est effacée (alpha nul), puis le contenu pivoté (largeur/hauteur échangées)
 * est écrit ancré au même coin haut-gauche — sur une région **non carrée**, le nouveau contour
 * déborde donc du contour d'origine ; ce débordement est tronqué au cadre de **l'image**, jamais
 * agrandi (`setPixel` ignore silencieusement tout pixel hors bornes).
 * @param image  Image modifiée en place.
 * @param region Région pivotée ; sans effet si vide.
 * @return L'union de la région effacée et de la région écrite (pour l'historique).
 */
PixelRegion rotateClockwise(DecodedImage& image, const PixelRegion& region);

/// Pivote le contenu d'une région d'un quart de tour, sens antihoraire (trois quarts de tour
/// horaires, pour ne pas dupliquer la logique de rotation). Mêmes règles de troncature que
/// `rotateClockwise`.
PixelRegion rotateCounterClockwise(DecodedImage& image, const PixelRegion& region);

/**
 * @brief Déplace le contenu d'une région d'un décalage donné.
 *
 * La région de départ devient transparente ; le contenu est reposé au décalage donné, tronqué au
 * cadre de l'image. Un déplacement entièrement hors cadre efface la région de départ sans rien
 * reposer de visible — jamais d'écriture hors bornes, jamais de tampon corrompu.
 * @param image  Image modifiée en place.
 * @param region Région déplacée ; sans effet si vide.
 * @param dx     Décalage horizontal, en pixels.
 * @param dy     Décalage vertical, en pixels.
 * @return L'union de la région quittée et de la région reposée (pour l'historique).
 */
PixelRegion moveRegion(DecodedImage& image, const PixelRegion& region, int dx, int dy);

/**
 * @brief Copie les pixels d'une région dans un presse-papiers autonome (ne référence plus @p
 * image).
 * @param image  Image consultée.
 * @param region Région copiée ; un presse-papiers vide si @p region est vide.
 */
[[nodiscard]] PixelClipboard copyRegion(const DecodedImage& image, const PixelRegion& region);

/**
 * @brief Colle un presse-papiers dans l'image, coin haut-gauche à `(x, y)`.
 *
 * Tronqué au cadre de l'image : coller près d'un bord n'écrit jamais hors bornes (chaque pixel
 * passe par `setPixel`, qui ignore silencieusement les positions hors bornes).
 * @param image     Image modifiée en place.
 * @param clipboard Contenu à coller ; sans effet si vide.
 * @param x         Colonne du coin haut-gauche du contenu collé.
 * @param y         Ligne du coin haut-gauche du contenu collé.
 * @return La région effectivement écrite (peut être plus petite que @p clipboard si tronquée).
 */
PixelRegion pasteClipboard(DecodedImage& image, const PixelClipboard& clipboard, int x, int y);

}  // namespace hmi
