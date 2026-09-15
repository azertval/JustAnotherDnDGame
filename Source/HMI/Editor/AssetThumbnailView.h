// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <QPixmap>
#include <QWidget>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "HMI/Graphics/AssetContract.h"

/**
 * @file HMI/Editor/AssetThumbnailView.h
 * @brief Widget de vignettes partagé, pour parcourir, choisir et gérer les fichiers d'un dossier
 *        d'assets (`EX-EDIT-026`).
 */

class QEvent;
class QListWidgetItem;

namespace Ui {
class AssetThumbnailView;
}

namespace hmi {

class Localization;

/**
 * @brief Grille de vignettes d'un dossier d'assets, avec recherche, sélection et gestion des
 *        fichiers (importer/renommer/dupliquer/supprimer).
 *
 * **Réutilisable tel quel** par toutes les sections du panneau « Textures » (Skins pour l'instant,
 * puis Fond, Objets, Animations, Décors) : le widget ignore délibérément la **sémantique** d'une
 * section (à quoi sert l'asset une fois choisi) — il liste un dossier et émet une sélection ou une
 * notification de changement. La sémantique (quel type de tuile, quel niveau…) reste dans la
 * section appelante.
 *
 * Pour la même raison, la **détection des références** avant renommage/suppression n'est pas
 * codée en dur ici : la section fournit un `ReferenceChecker` (`setReferenceChecker`), seule
 * partie qui sait CE QUI peut référencer un asset (un jeu de skins aujourd'hui, un niveau à partir
 * de LOT-44).
 *
 * Décodage **CPU uniquement** (`hmi::decodeImageFile` → `QPixmap`), indépendant du device
 * Direct3D 11 : ce widget doit rester utilisable même quand aucun viewport n'est actif. Les
 * vignettes sont mises à l'échelle en **plus proche voisin** (pixel art, `EX-ARCH-022`) et mises en
 * cache par chemin de fichier ; `clearCache` vide ce cache pour le rechargement à chaud (LOT-43
 * TACHE-03).
 */
class AssetThumbnailView : public QWidget {
    Q_OBJECT

public:
    /// Fonction fournie par la section : renvoie les références lisibles à un fichier (jeu de
    /// skins, futur niveau…), vide si aucune. Le widget ne sait rien de plus sur leur origine.
    using ReferenceChecker = std::function<std::vector<std::string>(const std::string&)>;

    explicit AssetThumbnailView(QWidget* parent = nullptr);
    ~AssetThumbnailView() override;

    /// Désigne le dossier balayé et recharge la grille.
    void setDirectory(std::filesystem::path directory);

    /// Famille de l'asset (`EX-REN-007`), utilisée pour valider les imports. À définir avant tout
    /// import ; sans effet sur le balayage ou l'affichage.
    void setAssetFamily(AssetFamily family) noexcept {
        _family = family;
    }

    /// Fournit le vérificateur de références de la section (voir la doc de classe).
    void setReferenceChecker(ReferenceChecker checker) {
        _referenceChecker = std::move(checker);
    }

    /// Affiche ou masque l'entrée « (aucun) » en tête de grille (sélection représentant l'absence
    /// d'assignation). Activée par défaut.
    void setNoneOptionVisible(bool visible);

    /// Recharge la grille depuis le dossier courant (après un import/renommage/suppression, ou un
    /// rechargement à chaud externe).
    void refresh();

    /// Vide le cache de vignettes décodées, sans recharger la grille (rechargement à chaud, LOT-43
    /// TACHE-03) — le prochain `refresh()` redécode donc chaque fichier depuis le disque.
    void clearCache();

    /// Sélectionne un fichier par son nom (aucun effet s'il n'est pas listé). Chaîne vide pour
    /// sélectionner l'entrée « (aucun) », si affichée.
    void selectAsset(const std::string& fileName);

    /// @return Le fichier actuellement sélectionné, vide si aucun ou si « (aucun) » est choisi.
    [[nodiscard]] std::string selectedAsset() const;

    /// Applique la langue active (champ de recherche, boutons, entrée « (aucun) »).
    void retranslateUi(const Localization& loc);

protected:
    /// Régénère les vignettes lors d'un changement d'écran (`QEvent::ScreenChangeInternal`) :
    /// l'échelle d'affichage a pu changer (`LOT-56` TACHE-05).
    bool event(QEvent* event) override;

signals:
    /// Émis quand la sélection change dans la grille (feuille, pas l'entrée « (aucun) »… incluse).
    void assetSelected(const QString& fileName);
    /// Émis sur double-clic/activation : confirme le choix (fermeture d'un dialogue englobant).
    void assetActivated(const QString& fileName);
    /// Émis après un import/renommage/duplication/suppression réussi : la section doit rafraîchir
    /// ce qui dépend du contenu du dossier (ex. une assignation qui visait le fichier supprimé).
    void assetsChanged();

private:
    void rebuildItems();
    [[nodiscard]] QPixmap thumbnailFor(const std::string& fileName);
    void onFilterChanged(const QString& text);
    void onCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
    void onItemActivated(QListWidgetItem* item);
    void onImport();
    void onRename();
    void onDuplicate();
    void onDelete();

    /// Références de @p fileName, décrites en un seul message ; chaîne vide si aucune ou si aucun
    /// vérificateur n'est configuré.
    [[nodiscard]] QString referencesWarning(const std::string& fileName) const;

    std::unique_ptr<Ui::AssetThumbnailView> _ui;
    std::filesystem::path _directory;
    AssetFamily _family = AssetFamily::TileSkin;
    ReferenceChecker _referenceChecker;
    std::string _filterText;
    bool _noneOptionVisible = true;
    /// Chemin absolu -> vignette décodée. Décodage paresseux : rempli au premier `refresh()`
    /// (donc à la première ouverture réelle de la vue), jamais à la construction du widget —
    /// une section non consultée ne décode donc jamais ses assets.
    std::unordered_map<std::string, QPixmap> _thumbnails;
    const Localization* _loc = nullptr;
};

}  // namespace hmi
