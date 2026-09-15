// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <DirectXMath.h>

#include "Core/Math/Rect.h"
#include "Core/Math/Vector2.h"

/**
 * @file HMI/Graphics/Camera2D.h
 * @brief Caméra 2D : conversion entre unités monde et pixels écran.
 */

namespace hmi {

/// Amplitude conservatrice de la secousse d'écran à un atterrissage lourd (`LOT-53` TACHE-03) :
/// discrète, un jeu de plateforme précis devient injouable si l'écran bouge trop.
inline constexpr float LANDING_SHAKE_AMPLITUDE_PIXELS = 2.0f;
/// Amplitude de la secousse à la mort du personnage : légèrement plus marquée que l'atterrissage.
inline constexpr float DEATH_SHAKE_AMPLITUDE_PIXELS = 3.0f;
/// Durée de toute secousse d'écran, en secondes -- brève par construction.
inline constexpr float SCREEN_SHAKE_DURATION = 0.2f;

/**
 * @brief État déterministe d'une secousse d'écran en cours de décroissance vers zéro.
 *
 * Donnée pure (`EX-NFR-002`) : aucune horloge système, seulement un temps écoulé avancé au pas
 * fixe par `advanceScreenShake`. `elapsed >= duration` signifie « terminée » -- état initial
 * (agrégat par défaut) légitimement déjà terminé, sans secousse active.
 */
struct ScreenShakeState {
    /// Amplitude au déclenchement, en pixels écran.
    float amplitudePixels = 0.0f;
    /// Temps écoulé depuis le déclenchement, en secondes.
    float elapsed = 0.0f;
    /// Durée totale de la secousse, en secondes.
    float duration = 0.0f;
};

/**
 * @brief Déclenche (ou relance) une secousse d'écran.
 * @param state         État à réarmer.
 * @param amplitudePixels Amplitude au déclenchement, en pixels écran (constante nommée côté
 *                        appelant, ex. `LANDING_SHAKE_AMPLITUDE_PIXELS`).
 * @param duration      Durée totale, en secondes (ex. `SCREEN_SHAKE_DURATION`).
 */
void triggerScreenShake(ScreenShakeState& state, float amplitudePixels, float duration) noexcept;

/**
 * @brief Avance une secousse d'écran d'un pas fixe.
 * @param state      État à avancer ; sans effet si déjà terminée (`elapsed >= duration`).
 * @param fixedDelta Durée du pas, en secondes.
 */
void advanceScreenShake(ScreenShakeState& state, float fixedDelta) noexcept;

/**
 * @brief Décalage courant d'une secousse d'écran, en pixels **entiers** (`EX-ARCH-022`).
 *
 * Décroît **linéairement** de `amplitudePixels` à zéro sur `duration`, sur l'axe vertical
 * uniquement -- une discrétion volontaire (cf. `epic.md`) plutôt qu'une oscillation qui
 * ajouterait un axe de plus à lire. Nul une fois la secousse terminée.
 * @param state Secousse dont on lit le décalage courant.
 * @return Le décalage à appliquer à la **projection de rendu uniquement** (jamais à `_center`, ni
 *         à la logique de cadrage par salle, ni à une position simulée).
 */
[[nodiscard]] core::Vector2 screenShakeOffset(const ScreenShakeState& state) noexcept;

/**
 * @brief Caméra 2D orthographique : projette le monde vers l'écran pour le rendu.
 *
 * La caméra est centrée sur une position **en unités monde** et applique l'échelle
 * **16 pixels/unité** (`EX-ARCH-021`), avec l'origine écran en haut-gauche et l'axe Y vers
 * le bas (convention du projet). Un facteur de **zoom** (de préférence entier pour la
 * netteté du pixel art, `EX-ARCH-022`) multiplie cette échelle. La caméra est un objet de
 * **présentation** : elle lit des positions monde mais ne modifie jamais l'ECS.
 */
class Camera2D {
public:
    /// Nombre de pixels par unité monde (`EX-ARCH-021`).
    static constexpr float PIXELS_PER_UNIT = 16.0f;

    /**
     * @brief Construit une caméra pour une surface de rendu donnée.
     * @param viewportWidth  Largeur de la surface de rendu, en pixels.
     * @param viewportHeight Hauteur de la surface de rendu, en pixels.
     */
    Camera2D(int viewportWidth, int viewportHeight);

    /**
     * @brief Met à jour les dimensions de la surface de rendu (redimensionnement).
     * @param viewportWidth  Nouvelle largeur, en pixels.
     * @param viewportHeight Nouvelle hauteur, en pixels.
     */
    void setViewportSize(int viewportWidth, int viewportHeight);

    /**
     * @brief Place le centre de la caméra.
     * @param worldCenter Position visée, au centre de l'écran, en unités monde.
     */
    void setCenter(const core::Vector2& worldCenter);

    /**
     * @brief Règle le facteur de zoom.
     * @param zoom Multiplicateur d'échelle (> 0 ; entier recommandé).
     */
    void setZoom(float zoom);

    /**
     * @brief Décalage de secousse d'écran (`LOT-53` TACHE-03), en pixels **entiers**.
     *
     * Appliqué uniquement à `projectionMatrix()` (rendu) -- jamais à `_center`, donc sans effet
     * sur `worldToScreen`/`screenToWorld`/`visibleBounds` : le culling et la logique de cadrage
     * par salle (`hmi::GameSession::updateCurrentRoom`, piloté par la position du personnage, pas
     * par la caméra) restent structurellement insensibles à la secousse.
     * @param offsetPixels Décalage courant (`hmi::screenShakeOffset`), nul hors secousse.
     */
    void setShakeOffsetPixels(const core::Vector2& offsetPixels) {
        _shakeOffsetPixels = offsetPixels;
    }

    /// @return Le centre courant de la caméra, en unités monde.
    [[nodiscard]] const core::Vector2& center() const noexcept {
        return _center;
    }

    /// @return Le facteur de zoom courant.
    [[nodiscard]] float zoom() const noexcept {
        return _zoom;
    }

    /**
     * @brief Matrice de projection monde → clip, pour le vertex shader.
     * @return La matrice (ligne-major DirectXMath) transformant une position monde en clip.
     */
    [[nodiscard]] DirectX::XMFLOAT4X4 projectionMatrix() const;

    /**
     * @brief Convertit une position monde en pixels écran.
     * @param world Position en unités monde.
     * @return Position en pixels (origine haut-gauche, Y-bas).
     */
    [[nodiscard]] core::Vector2 worldToScreen(const core::Vector2& world) const;

    /**
     * @brief Convertit une position écran (pixels) en unités monde.
     * @param screen Position en pixels (origine haut-gauche, Y-bas).
     * @return Position en unités monde.
     */
    [[nodiscard]] core::Vector2 screenToWorld(const core::Vector2& screen) const;

    /**
     * @brief Rectangle du monde effectivement cadré par la caméra.
     *
     * Base du **culling** (`EX-NFR-005`) : une primitive dont la boîte englobante n'intersecte pas
     * ce rectangle (élargi d'une marge, cf. `hmi::ComposedScene::CULLING_MARGIN_UNITS`) n'a aucune
     * raison d'être soumise. Dérivé de `screenToWorld` : aucune notion de cadrage nouvelle n'est
     * introduite, la caméra reste la seule source de vérité.
     * @return Le rectangle visible, en unités monde (coin haut-gauche + dimensions).
     */
    [[nodiscard]] core::Rect visibleBounds() const;

    /**
     * @brief Facteur de zoom ajustant un contenu à une surface disponible, sans zone hors champ.
     *
     * Entier (`std::floor`) tant que le facteur brut est supérieur ou égal à 1 — netteté pixel
     * art, `EX-ARCH-022` (préférence, pas une règle absolue) ; **fractionnaire** uniquement
     * lorsque nécessaire pour que le contenu entier tienne dans la surface disponible
     * (`EX-REN-013`, `EX-EDIT-013`) : sans cette exception, un contenu plus grand que la surface
     * resterait partiellement hors champ, faute de pouvoir descendre sous le zoom ×1.
     *
     * Fonction **pure**, partagée par le cadrage automatique de l'éditeur et celui du jeu (aucune
     * règle dupliquée entre les deux écrans, LOT-16).
     * @param availableWidth  Largeur disponible, en pixels (> 0).
     * @param availableHeight Hauteur disponible, en pixels (> 0).
     * @param contentWidth    Largeur du contenu à cadrer, en unités monde (> 0).
     * @param contentHeight   Hauteur du contenu à cadrer, en unités monde (> 0).
     * @param margin          Facteur multiplicatif appliqué avant l'arrondi (ex. `0.85` pour
     *                        laisser une marge visuelle) ; `1.0` par défaut (aucune marge).
     * @return Le facteur de zoom à appliquer via `setZoom`.
     */
    [[nodiscard]] static float fitZoom(float availableWidth, float availableHeight,
                                       float contentWidth, float contentHeight,
                                       float margin = 1.0f);

private:
    /// @return L'échelle effective, en pixels par unité monde (PIXELS_PER_UNIT × zoom).
    [[nodiscard]] float scale() const;

    int _viewportWidth;
    int _viewportHeight;
    core::Vector2 _center{};
    float _zoom = 1.0f;
    /// Décalage de secousse d'écran courant, en pixels (`setShakeOffsetPixels`) ; nul par défaut.
    core::Vector2 _shakeOffsetPixels{};
};

}  // namespace hmi
