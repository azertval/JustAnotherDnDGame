// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <QImage>
#include <QPixmap>
#include <QWidget>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Core/Ecs/Components/Animation.h"
#include "Core/Levels/CameraFraming.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/TileType.h"
#include "HMI/Graphics/AnimationCatalog.h"
#include "HMI/Graphics/SkinCatalog.h"

/**
 * @file HMI/Editor/TexturePanel.h
 * @brief Panneau « Textures » : jeu de skins courant et assignation par type (LOT-42).
 */

class QEvent;
class QModelIndex;
class QStandardItem;
class QStandardItemModel;
class QTimer;

namespace Ui {
class TexturePanel;
}

namespace core {
class LevelDraft;
}  // namespace core

namespace hmi {

class AssetThumbnailView;
class Localization;

/**
 * @brief Panneau d'habillage, organisé en **sections** extensibles (`EX-EDIT-042`, `EX-EDIT-024`).
 *
 * Premier — et unique — panneau du programme d'habillage : `LOT-44` (Fond), `LOT-45` (Objets) et
 * `LOT-47` (Animations) y ont ajouté leur onglet plutôt que de créer chacun leur dock. Recentré sur
 * ce qui **définit l'apparence** par `LOT-57` : le mode d'inspection par calque (`LOT-51`), qui
 * *regarde* le niveau plutôt qu'il ne le définit, a rejoint le menu Affichage ; l'inspecteur des
 * décors posés (`LOT-50`) a rejoint le panneau Décors (`LOT-57`, amendement post-essai manuel), qui
 * regroupe désormais tout ce qui touche aux décors.
 *
 * Le panneau **ne possède pas** le catalogue : il agit sur celui que lui confie `setCatalog`, dont
 * `GameViewport` reste propriétaire — le rendu doit voir les mêmes assignations, immédiatement.
 * Toute modification est écrite sur disque (`skins.json`, au chemin déployé) et signalée par
 * `assignmentsChanged`, que le viewport et la palette consomment pour se rafraîchir.
 *
 * La logique alimentant l'arbre est **hors** du widget (`hmi::buildSkinRows`,
 * `hmi::applySkinAssignment`), donc testée sans Qt (`EX-NFR-010`). Le choix du fichier assigné à un
 * type se fait par vignettes (`hmi::AssetThumbnailView`, `LOT-43`), pas par saisie de nom.
 */
class TexturePanel : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Construit le panneau.
     * @param skinsDirectory       Dossier balayé pour peupler la liste des fichiers assignables.
     * @param catalogPath          Chemin d'écriture de `skins.json`.
     * @param backgroundsDirectory Dossier balayé pour peupler la grille de fonds (section « Fond »,
     *                             `LOT-44`).
     * @param objectsDirectory     Dossier balayé pour peupler la grille d'assets de la section
     *                             « Objets » (`LOT-45`).
     * @param parent               Widget parent.
     */
    explicit TexturePanel(std::filesystem::path skinsDirectory, std::filesystem::path catalogPath,
                          std::filesystem::path backgroundsDirectory,
                          std::filesystem::path objectsDirectory, QWidget* parent = nullptr);
    ~TexturePanel() override;

    /**
     * @brief Confie au panneau le catalogue à éditer.
     * @param catalog Catalogue, non possédé, qui doit survivre au panneau.
     */
    void setCatalog(SkinCatalog* catalog);

    /**
     * @brief Synchronise la section « Fond » avec le niveau courant (`LOT-44`).
     *
     * À appeler après tout changement de brouillon (ouverture, undo/redo, chargement) pour que la
     * sélection affichée reste fidèle au niveau — l'inverse du sens habituel des signaux de ce
     * panneau (ici c'est l'appelant qui pousse l'état, la section ne le possède pas).
     * @param background Asset de fond du niveau courant (`core::LevelDraft::background`).
     * @param skinSet    Jeu de skins du niveau courant (`core::LevelDraft::skinSet`), distinct du
     *                   jeu courant d'édition (`currentSet`).
     */
    void setLevelProperties(const std::optional<std::string>& background,
                            const std::optional<std::string>& skinSet);

    /**
     * @brief Synchronise la section « Cadrage » avec le niveau courant (`EX-EDIT-028`, LOT-64).
     *
     * Même sens que `setLevelProperties` : l'appelant pousse l'état, la section ne le possède pas.
     * @param cameraFraming Cadrage résolu du niveau courant (`core::LevelDraft::cameraFraming`).
     */
    void setLevelCameraFraming(const core::CameraFramingConfig& cameraFraming);

    /// @return Le nom du jeu de skins actuellement sélectionné.
    [[nodiscard]] const std::string& currentSet() const noexcept {
        return _currentSet;
    }

    /// Applique la langue active (titre des sections, en-têtes de colonnes, libellés de types).
    void retranslateUi(const Localization& loc);

    /**
     * @brief Recharge l'arbre depuis le disque après un rechargement à chaud (`LOT-43` TACHE-03).
     *
     * Vide le cache de vignettes des lignes (un fichier a pu être modifié hors de l'application)
     * puis reconstruit l'arbre : le catalogue lui-même est déjà à jour (rechargé en place par
     * l'appelant, `GameViewport::reloadAssets`), seuls le balayage du dossier et les vignettes
     * doivent être refaits.
     */
    void reloadAssets();

    /**
     * @brief Resynchronise la section « Objets » avec le brouillon courant (`LOT-45`).
     *
     * À appeler après tout changement de brouillon (peinture, undo/redo, chargement), comme
     * `LinkPanel::refresh` : reconstruit la liste des surcharges depuis @p draft, sans en garder de
     * copie entre deux appels.
     * @param draft Brouillon dont on lit `textureOverrides()`.
     */
    void refreshObjects(const core::LevelDraft& draft);

protected:
    /// Régénère les vignettes lors d'un changement d'écran (`QEvent::ScreenChangeInternal`) :
    /// l'échelle d'affichage a pu changer (`LOT-56` TACHE-05).
    bool event(QEvent* event) override;

signals:
    /// Émis après toute modification d'assignation ou changement de jeu courant, une fois le
    /// catalogue à jour — le rendu et la palette n'ont qu'à se redessiner.
    void assignmentsChanged();
    /// Émis quand l'utilisateur demande un rechargement à chaud des textures (`LOT-43` TACHE-03).
    void reloadRequested();
    /// Émis quand l'utilisateur choisit un fond dans la section « Fond » (`LOT-44`) : nom de
    /// l'asset, vide pour « aucun fond ».
    void backgroundChanged(const QString& fileName);
    /// Émis quand l'utilisateur choisit le jeu de skins **du niveau** (`LOT-44`), distinct du jeu
    /// courant d'édition : nom du jeu, vide pour « jeu par défaut ».
    void levelSkinSetChanged(const QString& setName);
    /// Émis quand l'utilisateur choisit un nouveau cadrage de caméra dans la section « Cadrage »
    /// (`EX-EDIT-028`, LOT-64) : mode et, pour le mode *par salle*, taille de salle (`std::nullopt`
    /// = taille par défaut).
    void cameraFramingChanged(core::CameraFramingConfig cameraFraming);
    /// Émis par le bouton « Retirer » du tableau des zones de caméra dessinées à la main
    /// (`EX-LVL-007`, `EX-EDIT-029`) pour la ligne sélectionnée.
    void cameraZoneRemoveRequested(std::size_t index);
    /// Émis quand l'utilisateur choisit un asset dans la grille « Objets » (`LOT-45`) : asset actif
    /// de l'outil « Texture par instance » ; vide si aucun n'est sélectionné.
    void textureOverrideAssetSelected(const QString& fileName);
    /// Émis quand la sélection change dans la liste des surcharges du niveau (surbrillance dans le
    /// viewport), ou `std::nullopt` si aucune ligne n'est sélectionnée.
    void textureOverrideSelectionChanged(std::optional<core::GridPosition> position);
    /// Émis par le bouton « Retirer » de la section « Objets » pour la ligne sélectionnée.
    void textureOverrideRemoveRequested(core::GridPosition position);

private:
    void rebuildTree();
    void onSetChanged(int index);
    void onItemChanged(QStandardItem* item);
    void onAssetColumnActivated(const QModelIndex& index);
    void save();
    /// Vignette d'un fichier de skin (vide pour « aucun »), décodée au premier besoin puis mise en
    /// cache — repli en damier magenta si absent/illisible (cohérent avec le rendu, `LOT-40`).
    [[nodiscard]] QPixmap thumbnailFor(const std::string& asset);
    /// Reconstruit la liste du sélecteur de jeu de skins **du niveau** (section « Fond »), à partir
    /// des jeux du catalogue, plus l'entrée « jeu par défaut ».
    void rebuildLevelSkinSetSelector();
    /// Reconstruit le sélecteur de mode de la section « Cadrage » (3 entrées fixes, traduites) et
    /// synchronise la visibilité des champs de taille de vue (`PerRoom`/`Follow`, `EX-REN-017`) et
    /// du tableau des zones (`PerRoom` uniquement) avec `_levelCameraFraming.mode`.
    void rebuildCameraFramingSelector();
    /// Reconstruit le tableau des zones de caméra dessinées à la main depuis
    /// `_levelCameraFraming.zones` (`EX-LVL-007`), même patron que `rebuildObjectRows`.
    void rebuildCameraFramingZonesTable();
    /// Émet `cameraFramingChanged` à partir de l'état courant des widgets de mode/taille de la
    /// section « Cadrage » -- point d'appel unique de leurs gestionnaires de changement, pour ne
    /// jamais dupliquer la construction du `core::CameraFramingConfig` résultant. Les zones
    /// dessinées à la main ne sont jamais modifiées ici (mutées directement via `GameViewport`).
    void emitCameraFramingChanged();
    void onCameraFramingZonesSelectionChanged();
    void onCameraFramingZonesRemoveClicked();
    /// Reconstruit le tableau des surcharges depuis `_objectRows` (tri stable par position, déjà
    /// posé par `refreshObjects`).
    void rebuildObjectRows();
    void onObjectsSelectionChanged();
    void onObjectsRemoveClicked();
    /// Reconstruit l'arbre de la section « Animations » (`LOT-47` TACHE-04) depuis le catalogue et
    /// le diagnostic de clips manquants (`hmi::buildMechanismAnimationRows`).
    void rebuildAnimationsTree();
    /// Ouvre le selecteur a vignettes sur un double-clic dans la colonne Fichier de l'arbre
    /// « Animations » (meme patron que `onAssetColumnActivated`, un mecanisme est un type de tuile
    /// comme un autre pour `hmi::SkinCatalog`).
    void onAnimationsAssetActivated(const QModelIndex& index);
    /// Charge l'apercu (spritesheet decodee + description d'animation) de la ligne selectionnee.
    void onAnimationsSelectionChanged();
    /// Avance l'horloge d'apercu d'un pas de minuterie et rafraichit la vignette (`LOT-47`
    /// TACHE-04) : lecture en temps reel du meme catalogue que le jeu (`hmi::AnimationCatalog`),
    /// pas une reimplementation parallele.
    void tickAnimationPreview();

    std::unique_ptr<Ui::TexturePanel> _ui;
    QStandardItemModel* _model;
    AssetThumbnailView*
        _backgroundView;                ///< Grille de vignettes de `Assets/Backgrounds/` (LOT-44).
    AssetThumbnailView* _objectView;    ///< Grille de vignettes de `Assets/Objects/` (LOT-45).
    QStandardItemModel* _objectsModel;  ///< Modèle du tableau des surcharges (LOT-45).
    /// Modèle du tableau des zones de caméra dessinées à la main (LOT-64).
    QStandardItemModel* _cameraFramingZonesModel;
    QStandardItemModel*
        _animationsModel;            ///< Modèle de l'arbre de la section « Animations » (LOT-47).
    QTimer* _animationPreviewTimer;  ///< Fait avancer l'aperçu de la section « Animations ».
    /// Spritesheet décodée de l'asset actuellement prévisualisé (vide si aucune sélection/asset
    /// illisible) — décodée une fois par sélection, pas à chaque image de la minuterie.
    QImage _animationPreviewSheet;
    /// Description d'animation de l'asset prévisualisé, ou `std::nullopt` si absente/invalide/non
    /// sélectionnée : l'aperçu reste alors sur l'image entière de `_animationPreviewSheet`.
    std::optional<AnimationDescription> _animationPreviewDescription;
    /// Horloge d'aperçu, avancée par `_animationPreviewTimer` (temps réel, comme l'aperçu
    /// d'édition de `hmi::DraftRenderer` — la détermination n'a pas de sens hors simulation).
    core::Animation _animationPreviewAnimation;
    std::filesystem::path _skinsDirectory;
    std::filesystem::path _catalogPath;
    std::filesystem::path _backgroundsDirectory;
    std::filesystem::path _objectsDirectory;
    /// Surcharges affichées par le tableau, indexées comme ses lignes (`LOT-45`).
    std::vector<core::TileTextureOverride> _objectRows;
    SkinCatalog* _catalog = nullptr;  ///< Non possédé (propriété de `GameViewport`).
    std::string _currentSet;
    /// Dernières valeurs poussées par `setLevelProperties` (LOT-44) : reappliquées après tout
    /// rechargement/reconstruction qui perdrait la sélection courante des widgets (grille, combo).
    std::optional<std::string> _levelBackground;
    std::optional<std::string> _levelSkinSet;
    /// Dernière valeur poussée par `setLevelCameraFraming` (LOT-64) : réappliquée après toute
    /// reconstruction du sélecteur (langue), même principe que `_levelBackground`/`_levelSkinSet`.
    core::CameraFramingConfig _levelCameraFraming;
    bool _updating = false;  ///< Garde de réentrance pendant la reconstruction du modèle.
    const Localization* _loc = nullptr;  ///< Catalogue courant (nul avant première retraduction).
    /// Fichier de skin (vide pour « aucun ») -> vignette décodée. Vidé par `reloadAssets`.
    std::unordered_map<std::string, QPixmap> _thumbnails;
};

}  // namespace hmi
