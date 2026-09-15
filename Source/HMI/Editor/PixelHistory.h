// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstddef>
#include <string_view>
#include <vector>

#include "HMI/Editor/PixelOperations.h"

/**
 * @file HMI/Editor/PixelHistory.h
 * @brief Historique nommé, annulation et rétablissement du canevas pixel art (LOT-54 TACHE-02).
 *
 * **Local au canevas**, totalement indépendant de `core::LevelDraft` : annuler un coup de pinceau
 * n'annule jamais une action d'édition de niveau, et réciproquement (critère d'acceptation du
 * lot). Sans dépendance Qt/GPU (`EX-NFR-010`).
 */

namespace hmi {

/**
 * @brief Type d'une opération d'historique — détermine sa clé de traduction (`EX-REN-033`).
 *
 * Ce nom sert deux fois : dans le libellé de l'action « Annuler *…* » (TACHE-04) et dans le
 * panneau d'historique visuel. D'autres membres rejoignent cette énumération aux tâches
 * suivantes (déplacement de région, symétries, rotations, coller — TACHE-06).
 */
enum class PixelOperationKind {
    Brush,
    Eraser,
    Fill,
    // Outils de région (LOT-54 TACHE-06).
    Move,
    FlipHorizontal,
    FlipVertical,
    RotateClockwise,
    RotateCounterClockwise,
    Paste,
};

/**
 * @brief Clé de traduction décrivant une opération.
 * @param kind Type d'opération.
 * @return La clé (existe dans les deux catalogues, `fr.lang`/`en.lang`).
 */
[[nodiscard]] std::string_view pixelOperationTranslationKey(PixelOperationKind kind) noexcept;

/**
 * @brief Une entrée d'historique : la région affectée, son contenu avant et après l'opération.
 *
 * Mémoriser la région plutôt qu'une copie complète de l'image borne le coût mémoire de
 * l'historique (épic, section « Découpage » TACHE-02). `before`/`after` ont pour taille
 * `region.width() * region.height()` (capturés via `hmi::readRegion`).
 */
struct PixelHistoryEntry {
    PixelOperationKind kind = PixelOperationKind::Brush;
    PixelRegion region;
    std::vector<std::uint32_t> before;
    std::vector<std::uint32_t> after;
};

/**
 * @brief Pile d'annulation/rétablissement nommée d'un canevas pixel art.
 *
 * Même sémantique que l'historique de `core::LevelDraft` (deux piles, la nouvelle opération après
 * une annulation vide la pile de rétablissement) — mais un tampon de pixels et un
 * `core::LevelDraft` sont deux mondes disjoints, d'où une classe séparée plutôt qu'un partage de
 * code qui les couplerait.
 */
class PixelHistory {
public:
    /// @param maxDepth Nombre maximal d'entrées conservées dans la pile d'annulation ; au-delà,
    ///                 les plus anciennes sont oubliées (coût mémoire borné).
    explicit PixelHistory(std::size_t maxDepth = 100);

    /**
     * @brief Enregistre une opération déjà appliquée au tampon.
     *
     * Vide la pile de rétablissement (une branche de refaire abandonnée ne redevient jamais
     * valide après une nouvelle opération — historique linéaire classique). Si la profondeur
     * dépasse `maxDepth`, l'entrée la plus ancienne est oubliée : l'état courant du tampon n'est
     * jamais affecté, seule la portée de l'annulation recule.
     */
    void push(PixelOperationKind kind, PixelRegion region, std::vector<std::uint32_t> before,
              std::vector<std::uint32_t> after);

    /// Annule la dernière opération sur @p image. @return `false`, sans effet, si `!canUndo()`.
    bool undo(DecodedImage& image);

    /// Rétablit la dernière opération annulée sur @p image. @return `false`, sans effet, si
    /// `!canRedo()`.
    bool redo(DecodedImage& image);

    /**
     * @brief Revient à l'état immédiatement après l'entrée @p index de `appliedEntries()`, en un
     *        seul appel plutôt que par annulations successives.
     * @param image Image ramenée à cet état.
     * @param index Index (0-based) dans `appliedEntries()`.
     * @return `false`, sans effet, si @p index est hors bornes.
     */
    bool jumpTo(DecodedImage& image, std::size_t index);

    /// @return `true` si `undo()` aurait un effet.
    [[nodiscard]] bool canUndo() const noexcept {
        return !_applied.empty();
    }
    /// @return `true` si `redo()` aurait un effet.
    [[nodiscard]] bool canRedo() const noexcept {
        return !_undone.empty();
    }

    /// @return Les opérations actuellement appliquées, dans l'ordre chronologique — de quoi
    ///         alimenter le panneau d'historique visuel (TACHE-04) et le libellé « Annuler … ».
    [[nodiscard]] const std::vector<PixelHistoryEntry>& appliedEntries() const noexcept {
        return _applied;
    }

private:
    std::size_t _maxDepth;
    std::vector<PixelHistoryEntry> _applied;  ///< Pile d'annulation, plus ancienne en premier.
    std::vector<PixelHistoryEntry> _undone;   ///< Pile de rétablissement, plus récente en dernier.
};

}  // namespace hmi
