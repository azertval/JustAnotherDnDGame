// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <deque>
#include <string>
#include <vector>

#include "HMI/Graphics/ComposedScene.h"

/**
 * @file HMI/Game/DiagnosticsHud.h
 * @brief Compteur de cadence et de primitives, affichable en jeu (`LOT-62` TACHE-02,
 *        `EX-NFR-001`, `EX-NFR-005`).
 */

namespace hmi {

class Localization;

/// Fenêtre de la moyenne glissante de cadence, en secondes : assez courte pour rester réactive à
/// une vraie dégradation, assez longue pour lisser le bruit d'une image à l'autre. Une cadence
/// **instantanée** (`1 / dt` sur une seule image) saute dans tous les sens et n'informerait sur
/// rien (cf. `epic.md`, points d'attention).
inline constexpr float DIAGNOSTICS_FPS_WINDOW_SECONDS = 0.5f;

/**
 * @brief Moyenne glissante de la cadence de rendu, sur une fenêtre courte.
 *
 * Accumule les durées d'image **réelles** (temps écoulé entre deux rendus, indépendant du pas fixe
 * de simulation, `LOT-33`) et n'en retient que celles tombées dans les
 * `DIAGNOSTICS_FPS_WINDOW_SECONDS` les plus récentes. Logique **pure** (`EX-NFR-010`), testable
 * sans horloge système : l'appelant fournit lui-même les durées.
 */
class FrameRateAverage {
public:
    /**
     * @brief Ajoute la durée d'une image écoulée et purge les échantillons sortis de la fenêtre.
     * @param deltaSeconds Durée écoulée depuis l'image précédente, en secondes (ignorée si
     *                     négative ou nulle : un appelant ne doit jamais en produire, robustesse).
     */
    void addSample(float deltaSeconds) noexcept;

    /**
     * @brief Cadence moyenne sur la fenêtre courante.
     * @return Le nombre d'images par seconde, ou `0` si aucun échantillon n'a encore été ajouté
     *         (division par zéro évitée : état de départ légitime, avant la première image).
     */
    [[nodiscard]] float framesPerSecond() const noexcept;

    /// Oublie tous les échantillons (bascule affichage éteint → allumé, TACHE-02 : repartir d'une
    /// fenêtre vide plutôt que de mélanger un temps accumulé pendant que rien n'était mesuré).
    void reset() noexcept;

private:
    std::deque<float> _recentDeltas;
    float _accumulatedSeconds = 0.0f;
};

/// Valeurs mesurées pour une image de diagnostic (`LOT-62` TACHE-02).
struct DiagnosticsMeasurements {
    /// Cadence de **rendu**, moyennée (`FrameRateAverage`) — distincte du nombre de pas de
    /// simulation ci-dessous depuis le `LOT-33` : les deux cadences divergent dès que le rendu
    /// dépasse (ou rattrape) le pas fixe.
    float framesPerSecond = 0.0f;
    /// Compteurs de la dernière image composée (`hmi::SpriteRenderer::lastScene`, `EX-NFR-005`).
    SceneStatistics sceneStatistics{};
    /// Nombre de pas de simulation consommés à la dernière image (`core::FixedTimestep::advance`) :
    /// révèle immédiatement une boucle qui rattrape (plusieurs pas d'un coup).
    int simulationSteps = 0;
};

/**
 * @brief Compose les lignes du compteur de diagnostic à partir de valeurs déjà mesurées.
 *
 * Fonction **pure** (`EX-NFR-010`), sur le patron de `hmi::gameHudLines` : le calcul (quelles
 * lignes, quel texte) est testable sans rendu ; seul le dessin (`hmi::composeText`, à l'appelant)
 * ne l'est pas.
 * @param measurements Valeurs mesurées pour l'image courante.
 * @param localization Catalogue de traduction, pour les libellés (`EX-REN-033`) — aucune chaîne en
 *                     dur.
 * @return Les lignes à afficher, dans l'ordre : cadence, primitives (composées puis soumises),
 *         passes de dessin, pas de simulation.
 */
[[nodiscard]] std::vector<std::string> composeDiagnosticsHudLines(
    const DiagnosticsMeasurements& measurements, const Localization& localization);

}  // namespace hmi
