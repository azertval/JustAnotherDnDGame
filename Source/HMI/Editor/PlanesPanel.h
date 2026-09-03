// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>
#include <cstddef>
#include <optional>

#include "Core/Levels/CameraFraming.h"
#include "Core/Levels/Plane.h"
#include "HMI/Graphics/PlaneVisibility.h"

/**
 * @file HMI/Editor/PlanesPanel.h
 * @brief Panneau « Plans » : liste ordonnée des plans picturaux et réglages du plan sélectionné
 *        (`EX-EDIT-047`, LOT-69 TACHE-08).
 */

namespace Ui {
class PlanesPanel;
}

namespace core {
class LevelDraft;
}

namespace hmi {

class Localization;

/**
 * @brief Liste ordonnée des plans d'un niveau, avec leurs réglages.
 *
 * **Vue du modèle**, pas un état : `refresh` repeuple les champs depuis un `core::LevelDraft`,
 * aucune copie n'est conservée entre deux appels. Le panneau n'écrit rien — il émet un signal typé,
 * et l'appelant (`hmi::MainWindow` → `hmi::GameViewport`) applique la mutation au brouillon, seul
 * propriétaire. Même patron que les autres panneaux de l'éditeur.
 *
 * L'**ordre de la liste est significatif** : il décide de la superposition à l'intérieur d'une
 * profondeur. D'où des boutons Monter/Descendre plutôt qu'un tri de colonne, qui laisserait croire
 * que l'ordre n'est qu'un confort d'affichage.
 *
 * Les champs numériques émettent sur `editingFinished`, **jamais** sur `valueChanged` : taper
 * « 0.75 » dans un facteur de parallaxe produirait sinon quatre mutations, donc quatre pas
 * d'annulation pour un seul geste (leçon du `LOT-67`).
 *
 * La **visibilité** des plans (masquage, isolement) n'est pas une propriété du niveau : elle vit
 * dans `hmi::PlaneVisibility`, n'est pas persistée, et n'empile aucun pas d'annulation.
 */
class PlanesPanel : public QWidget {
    Q_OBJECT

public:
    explicit PlanesPanel(QWidget* parent = nullptr);
    ~PlanesPanel() override;

    /**
     * @brief Repeuple le panneau depuis @p draft.
     * @param draft      Brouillon courant (lecture seule).
     * @param selected   Rang du plan sélectionné, si un l'est.
     * @param visibility Visibilité courante par rang (masquage, isolement).
     */
    void refresh(const core::LevelDraft& draft, std::optional<std::size_t> selected,
                 const PlaneVisibility& visibility);

    /// @return Le rang du plan sélectionné dans la liste, si un l'est.
    [[nodiscard]] std::optional<std::size_t> selectedIndex() const;

    /// Applique la langue active (titres, libellés, entrées de listes déroulantes).
    void retranslateUi(const Localization& loc);

signals:
    /// Un plan a été sélectionné dans la liste (ou désélectionné).
    void planeSelected(std::optional<std::size_t> index);
    /// Créer un plan : l'appelant crée le PNG et ajoute l'entrée au brouillon.
    void addRequested();
    /// Retirer l'entrée du plan @p index. **Le fichier n'est jamais supprimé** : le brouillon
    /// annule l'entrée JSON, pas la disparition d'une image — un fichier orphelin est moins grave
    /// qu'un travail perdu.
    void removeRequested(std::size_t index);
    /// Ouvrir le plan @p index dans le mode création.
    void paintRequested(std::size_t index);
    /// Réordonner : `forward` rapproche le plan du dessus de la liste.
    void reorderRequested(std::size_t index, bool forward);
    /// Changer la profondeur du plan @p index.
    void depthChangeRequested(std::size_t index, core::PlaneDepth depth);
    /// Changer la densité du plan @p index : l'appelant rééchantillonne l'image et la réécrit.
    void densityChangeRequested(std::size_t index, int pixelsPerUnit);
    /// Changer les facteurs de parallaxe du plan @p index.
    void parallaxChangeRequested(std::size_t index, float parallaxX, float parallaxY);
    /// Changer l'opacité du plan @p index.
    void opacityChangeRequested(std::size_t index, float opacity);
    /// Changer le drapeau de parallaxe du **niveau**.
    void levelParallaxToggled(bool enabled);
    /// Masquer/réafficher le plan @p index (aide d'édition, jamais persistée).
    void visibilityToggled(std::size_t index, bool visible);
    /// Isoler le plan @p index, ou tout réafficher si @p isolate est faux.
    void isolateToggled(std::size_t index, bool isolate);

private:
    /// Repeuple la liste sans réémettre `planeSelected` (repeuplement, pas geste utilisateur).
    void rebuildTree(const core::LevelDraft& draft, std::optional<std::size_t> selected,
                     const PlaneVisibility& visibility);
    /// Repeuple les champs du plan sélectionné, ou les désactive si aucun ne l'est.
    void refreshSettings(const core::LevelDraft& draft, std::optional<std::size_t> selected);
    /// Branche les signaux des contrôles (une seule fois, à la construction).
    void connectControls();

    Ui::PlanesPanel* _ui;
    /// Vrai pendant un repeuplement : les signaux des contrôles sont alors ignorés, sinon
    /// repeupler la vue émettrait des mutations que personne n'a demandées.
    bool _refreshing = false;
    /// Mode de cadrage du niveau, pour griser la case de parallaxe là où le moteur la neutralise.
    core::CameraFramingMode _framingMode = core::CameraFramingMode::WholeLevel;
};

}  // namespace hmi
