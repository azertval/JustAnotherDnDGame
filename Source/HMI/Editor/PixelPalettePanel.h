// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <QWidget>
#include <cstddef>
#include <cstdint>
#include <memory>

#include "HMI/Editor/PixelPalette.h"

class QCheckBox;
class QListWidget;
class QPushButton;

/**
 * @file HMI/Editor/PixelPalettePanel.h
 * @brief Panneau d'édition de la palette de projet de l'atelier pixel art (`LOT-54` TACHE-07).
 */

namespace Ui {
class PixelPalettePanel;
}

namespace hmi {

class Localization;

/**
 * @brief Liste éditable de la palette de projet (`hmi::PixelPalette`), avec le réglage «
 * contraindre à la palette ».
 *
 * **Vue du modèle**, pas un état : `refresh` reconstruit l'affichage depuis une `hmi::PixelPalette`
 * déjà chargée, même patron que `hmi::LinkPanel`/`hmi::PixelHistoryPanel`. Chaque action
 * utilisateur émet un signal ; la mutation effective de la palette (et son enregistrement) reste à
 * la charge de `MainWindow`, seul propriétaire du fichier `palettes.json`. Seules les pastilles de
 * couleur montrent les couleurs de la palette elle-même — le reste de l'habillage vient des jetons
 * (`EX-IHM-051`).
 */
class PixelPalettePanel : public QWidget {
    Q_OBJECT

public:
    explicit PixelPalettePanel(QWidget* parent = nullptr);

    /// Hors-ligne : `std::unique_ptr<Ui::PixelPalettePanel>` porte un type incomplet.
    ~PixelPalettePanel() override;

    /// Reconstruit la liste depuis @p palette (après toute mutation).
    void refresh(const PixelPalette& palette);
    /// Applique la langue active.
    void retranslateUi(const Localization& loc);

    /// Coche/décoche le réglage « contraindre à la palette » **sans** émettre `constrainToggled`
    /// (resynchronisation depuis un état persisté).
    void setConstrainEnabled(bool enabled);
    [[nodiscard]] bool constrainEnabled() const;

signals:
    /// L'utilisateur veut ajouter la couleur courante du canevas à la palette.
    void addRequested();
    /// Retirer l'entrée @p index.
    void removeRequested(std::size_t index);
    /// Renommer l'entrée @p index (le nouveau nom est demandé par l'appelant, `MainWindow`).
    void renameRequested(std::size_t index);
    /// Déplacer l'entrée @p index d'un cran (@p up : vers le haut, sinon vers le bas).
    void moveRequested(std::size_t index, bool up);
    /// Extraire la palette de l'asset actuellement ouvert dans le canevas.
    void extractRequested();
    /// Le réglage « contraindre à la palette » a changé (choix explicite de l'utilisateur).
    void constrainToggled(bool enabled);
    /// Double-clic sur une pastille : adopter cette couleur comme couleur courante du canevas.
    void colorActivated(std::uint32_t color);

private:
    void onRemoveClicked();
    void onRenameClicked();
    void onMoveUpClicked();
    void onMoveDownClicked();
    void onItemActivated(int row);
    void updateButtonsEnabled();

    /// Mise en page issue de `PixelPalettePanel.ui` (`LOT-68`) : le C++ ne branche plus que le
    /// fonctionnel, conformément à la convention du projet.
    std::unique_ptr<Ui::PixelPalettePanel> _ui;
    QListWidget* _list;
    QPushButton* _addButton;
    QPushButton* _removeButton;
    QPushButton* _renameButton;
    QPushButton* _moveUpButton;
    QPushButton* _moveDownButton;
    QPushButton* _extractButton;
    QCheckBox* _constrainCheck;
    const Localization* _loc = nullptr;
    std::vector<PixelPaletteEntry> _entries;  ///< Dernier contenu affiché (retranslateUi le relit).
};

}  // namespace hmi
