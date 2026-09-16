// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <QString>
#include <QWidget>
#include <filesystem>
#include <memory>

/**
 * @file HMI/Editor/LevelBrowserPanel.h
 * @brief Panneau « Niveaux » : liste, recherche et gestion des fichiers de niveaux (LOT-36).
 */

class QModelIndex;
class QSortFilterProxyModel;
class QStandardItemModel;

namespace Ui {
class LevelBrowserPanel;
}

namespace hmi {

class Localization;

/**
 * @brief Panneau de gestion des niveaux : liste filtrable + créer/renommer/dupliquer/supprimer.
 *
 * Liste les fichiers `.json` d'un dossier (`EX-IHM-020`), avec **recherche** incrémentale
 * (`QSortFilterProxyModel`) pour rester lisible quel que soit leur nombre. Les opérations
 * (`EX-IHM-021`) délèguent à `hmi::LevelFileOperations` (couche pure, validée/testée) ; les
 * erreurs sont signalées à l'utilisateur, jamais silencieuses. Un double-clic (ou « Ouvrir ») émet
 * `levelOpenRequested` — le garde-fou des modifications non enregistrées est appliqué par
 * l'appelant (`MainWindow`).
 */
class LevelBrowserPanel : public QWidget {
    Q_OBJECT

public:
    explicit LevelBrowserPanel(std::filesystem::path levelsDir, QWidget* parent = nullptr);
    ~LevelBrowserPanel() override;

    /// Recharge la liste depuis le dossier (après une opération ou un changement externe).
    void refresh();

    /// Applique la langue active (boutons, champ de recherche).
    void retranslateUi(const Localization& loc);

signals:
    /// Émis quand l'utilisateur demande l'ouverture d'un niveau (chemin absolu du fichier).
    void levelOpenRequested(const QString& path);

private:
    void onNew();
    void onRename();
    void onDuplicate();
    void onDelete();
    void onActivated(const QModelIndex& index);

    /// Chemin du niveau sélectionné, ou chemin vide si aucune sélection.
    [[nodiscard]] std::filesystem::path selectedPath() const;

    std::unique_ptr<Ui::LevelBrowserPanel> _ui;  ///< Mise en page (LevelBrowserPanel.ui).
    std::filesystem::path _dir;
    QStandardItemModel* _model;          ///< Modèle source (données), rempli par refresh().
    QSortFilterProxyModel* _proxy;       ///< Filtre de recherche au-dessus du modèle.
    const Localization* _loc = nullptr;  ///< Catalogue courant (pour les dialogues localisés).
};

}  // namespace hmi
