// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/TexturePanel.h"

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QEvent>
#include <QHeaderView>
#include <QIcon>
#include <QImage>
#include <QItemSelectionModel>
#include <QList>
#include <QModelIndex>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QString>
#include <QStringList>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QTimer>
#include <QTreeView>
#include <QVBoxLayout>
#include <QVariant>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "Core/Ecs/AnimationClip.h"
#include "Core/Ecs/Systems/AnimationSystem.h"
#include "Core/Levels/LevelDraft.h"
#include "HMI/Editor/AssetReferences.h"
#include "HMI/Editor/AssetThumbnailView.h"
#include "HMI/Editor/MechanismAnimationAssignments.h"
#include "HMI/Editor/SkinAssignments.h"
#include "HMI/Editor/TaxonomyLabels.h"
#include "HMI/Editor/ThumbnailGeometry.h"
#include "HMI/Graphics/GraphicsLog.h"
#include "HMI/Graphics/MechanismVisuals.h"
#include "HMI/Graphics/MissingTexture.h"
#include "HMI/Graphics/ProceduralAtlas.h"
#include "HMI/Graphics/TextureLoader.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Localization/Localization.h"
#include "ui_TexturePanel.h"

namespace hmi {

namespace {

// Colonnes de l'arbre : le type, le fichier assigne, et son mode de decoupage.
constexpr int COLUMN_TYPE = 0;
constexpr int COLUMN_ASSET = 1;
constexpr int COLUMN_MODE = 2;
constexpr int COLUMN_COUNT = 3;

// Colonnes du tableau des surcharges de la section « Objets » (LOT-45) : position et asset.
constexpr int OBJECTS_COLUMN_POSITION = 0;
constexpr int OBJECTS_COLUMN_ASSET = 1;
constexpr int OBJECTS_COLUMN_COUNT = 2;

// Colonnes du tableau des zones de camera dessinees a la main (section « Cadrage », LOT-64).
constexpr int CAMERA_FRAMING_ZONES_COLUMN_X = 0;
constexpr int CAMERA_FRAMING_ZONES_COLUMN_Y = 1;
constexpr int CAMERA_FRAMING_ZONES_COLUMN_WIDTH = 2;
constexpr int CAMERA_FRAMING_ZONES_COLUMN_HEIGHT = 3;
constexpr int CAMERA_FRAMING_ZONES_COLUMN_COUNT = 4;

// Colonnes de l'arbre de la section « Animations » (LOT-47) : famille de mecanisme, fichier
// assigne, diagnostic des clips manquants.
constexpr int ANIMATIONS_COLUMN_TYPE = 0;
constexpr int ANIMATIONS_COLUMN_ASSET = 1;
constexpr int ANIMATIONS_COLUMN_DIAGNOSTIC = 2;
constexpr int ANIMATIONS_COLUMN_COUNT = 3;

// Intervalle de la minuterie d'apercu (LOT-47 TACHE-04) : temps reel (comme l'apercu d'edition de
// hmi::DraftRenderer), pas la determination du pas fixe -- suffisant pour "verifier le rythme"
// sans entrer en mode essai.
constexpr int ANIMATION_PREVIEW_INTERVAL_MS = 66;
// Facteur d'agrandissement de la vignette d'apercu (une case de 16x16 px serait illisible telle
// quelle dans le panneau).
constexpr int ANIMATION_PREVIEW_SCALE = 4;

// Libelle "(colonne, ligne)" d'une position de grille -- meme format que LinkPanel.cpp.
QString positionText(core::GridPosition position) {
    return QStringLiteral("(%1, %2)").arg(position.column).arg(position.row);
}

// Role portant le core::TileType d'une ligne (les en-tetes de section n'en ont pas).
constexpr int TILE_TYPE_ROLE = Qt::UserRole + 1;

// Cote des vignettes affichees dans l'arbre, en pixels d'ecran (repere visuel, pas une vignette
// de selection -- le choix se fait dans AssetPickerDialog, a une taille plus grande) : jeton de
// taille (LOT-56, icone moyenne), commun aux deux themes d'editeur.
const int ROW_ICON_SIZE = editorDarkTokens().size.iconMedium;

// Valeur affichee pour « aucun skin assigne ». Traduite : c'est une entree de liste comme une
// autre pour l'utilisateur.
[[nodiscard]] QString noneLabel(const Localization* loc) {
    return loc == nullptr ? QStringLiteral("(aucun)")
                          : QString::fromStdString(loc->text("textures.none"));
}

// Valeur affichee pour « jeu de skins par defaut » dans le selecteur du NIVEAU (section Fond,
// LOT-44) -- distincte de noneLabel : une chaine vide designe ici le jeu par defaut du catalogue,
// pas une absence d'assignation.
[[nodiscard]] QString defaultSetLabel(const Localization* loc) {
    return loc == nullptr ? QStringLiteral("(jeu par defaut)")
                          : QString::fromStdString(loc->text("textures.default_set"));
}

// Texte localise d'une cle (repli sur la cle si aucun catalogue -- ne survient pas en pratique).
[[nodiscard]] QString t(const Localization* loc, const char* key) {
    return loc != nullptr ? QString::fromStdString(loc->text(key)) : QString::fromLatin1(key);
}

// Libelle traduit d'un mode de cadrage (section « Cadrage », LOT-64) : cle par mode, jamais un
// identifiant technique affiche a l'ecran (EX-VIS-006).
[[nodiscard]] QString cameraFramingModeLabel(const Localization* loc,
                                             core::CameraFramingMode mode) {
    switch (mode) {
        case core::CameraFramingMode::WholeLevel:
            return t(loc, "camera_framing.whole_level");
        case core::CameraFramingMode::PerRoom:
            return t(loc, "camera_framing.per_room");
        case core::CameraFramingMode::Follow:
            return t(loc, "camera_framing.follow");
    }
    return t(loc, "camera_framing.whole_level");
}

// Ordre fixe des trois entrees du selecteur de mode -- meme ordre partout (selecteur, previsu,
// barre d'etat) pour qu'un index de combo se traduise toujours vers le meme mode.
constexpr std::array<core::CameraFramingMode, 3> CAMERA_FRAMING_MODES{
    core::CameraFramingMode::WholeLevel, core::CameraFramingMode::PerRoom,
    core::CameraFramingMode::Follow};

// Texte de diagnostic d'une ligne de la section « Animations » (LOT-47) : vide sans asset assigne
// -- rien a diagnostiquer -- « complet » si tous les clips attendus sont presents, sinon la liste
// des clips manquants.
[[nodiscard]] QString diagnosticText(const Localization* loc, const MechanismAnimationRow& row) {
    if (row.asset.empty()) {
        return QString();
    }
    if (row.missingClips.empty()) {
        return t(loc, "textures.animations_complete");
    }
    QStringList missing;
    for (const std::string& clipName : row.missingClips) {
        missing << QString::fromStdString(clipName);
    }
    return t(loc, "textures.animations_missing") + QStringLiteral(" : ") + missing.join(", ");
}

// Convertit des pixels RGBA decodes en QImage. Meme conversion que PalettePanel/
// AssetThumbnailView : trop petite pour justifier une fonction partagee entre trois widgets
// independants.
[[nodiscard]] QImage toImage(const DecodedImage& decoded) {
    QImage image(decoded.width, decoded.height, QImage::Format_RGBA8888);
    for (int y = 0; y < decoded.height; ++y) {
        const std::uint32_t* const row =
            decoded.pixels.data() + static_cast<std::size_t>(y) * decoded.width;
        std::memcpy(image.scanLine(y), row, static_cast<std::size_t>(decoded.width) * 4);
    }
    return image;
}

// Editeur en liste fermee pour la colonne Mode (deux choix fixes) : le seul delegue par defaut de
// Qt offrirait une saisie libre, incoherente avec des valeurs enumerees.
class ChoiceDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/,
                          const QModelIndex& index) const override {
        auto* const combo = new QComboBox(parent);
        combo->addItems(index.data(Qt::UserRole).toStringList());
        return combo;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        if (auto* const combo = qobject_cast<QComboBox*>(editor)) {
            combo->setCurrentText(index.data(Qt::EditRole).toString());
        }
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override {
        if (auto* const combo = qobject_cast<QComboBox*>(editor)) {
            model->setData(index, combo->currentText(), Qt::EditRole);
        }
    }
};

// Dialogue de selection d'un asset a vignettes (LOT-43 TACHE-01), enveloppe minimale autour du
// widget partage : ce panneau ne connait que les skins, donc configure AssetThumbnailView pour
// cette semantique (dossier, famille, verificateur de references) sans que le widget lui-meme en
// sache quoi que ce soit.
class AssetPickerDialog : public QDialog {
public:
    AssetPickerDialog(std::filesystem::path directory, const Localization* loc, QWidget* parent)
        : QDialog(parent), _view(new AssetThumbnailView(this)) {
        setWindowTitle(t(loc, "textures.pick_asset"));
        _view->setAssetFamily(AssetFamily::TileSkin);
        if (loc != nullptr) {
            _view->retranslateUi(*loc);
        }
        _view->setDirectory(std::move(directory));

        auto* const buttons =
            new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        connect(_view, &AssetThumbnailView::assetActivated, this, [this] { accept(); });

        auto* const layout = new QVBoxLayout(this);
        layout->addWidget(_view);
        layout->addWidget(buttons);
        resize(480, 360);
    }

    void setReferenceChecker(AssetThumbnailView::ReferenceChecker checker) {
        _view->setReferenceChecker(std::move(checker));
    }
    void selectAsset(const std::string& fileName) {
        _view->selectAsset(fileName);
    }
    [[nodiscard]] std::string selectedAsset() const {
        return _view->selectedAsset();
    }

private:
    AssetThumbnailView* _view;
};

}  // namespace

TexturePanel::TexturePanel(std::filesystem::path skinsDirectory, std::filesystem::path catalogPath,
                           std::filesystem::path backgroundsDirectory,
                           std::filesystem::path objectsDirectory, QWidget* parent)
    : QWidget(parent),
      _ui(std::make_unique<Ui::TexturePanel>()),
      _model(new QStandardItemModel(this)),
      _backgroundView(nullptr),
      _objectView(nullptr),
      _objectsModel(new QStandardItemModel(0, OBJECTS_COLUMN_COUNT, this)),
      _cameraFramingZonesModel(new QStandardItemModel(0, CAMERA_FRAMING_ZONES_COLUMN_COUNT, this)),
      _animationsModel(new QStandardItemModel(0, ANIMATIONS_COLUMN_COUNT, this)),
      _animationPreviewTimer(new QTimer(this)),
      _skinsDirectory(std::move(skinsDirectory)),
      _catalogPath(std::move(catalogPath)),
      _backgroundsDirectory(std::move(backgroundsDirectory)),
      _objectsDirectory(std::move(objectsDirectory)) {
    _ui->setupUi(this);
    _model->setColumnCount(COLUMN_COUNT);
    _ui->skinsTree->setModel(_model);
    _ui->skinsTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    // Le mode se choisit dans une liste fermee (deux valeurs) ; le fichier se choisit desormais
    // par vignettes (LOT-43 TACHE-01), via un double-clic ouvrant AssetPickerDialog.
    _ui->skinsTree->setItemDelegateForColumn(COLUMN_MODE, new ChoiceDelegate(this));

    connect(_ui->setSelector, &QComboBox::currentIndexChanged, this, &TexturePanel::onSetChanged);
    connect(_model, &QStandardItemModel::itemChanged, this, &TexturePanel::onItemChanged);
    connect(_ui->skinsTree, &QTreeView::doubleClicked, this, &TexturePanel::onAssetColumnActivated);
    connect(_ui->reloadButton, &QPushButton::clicked, this, [this] { emit reloadRequested(); });

    // Section « Fond » (LOT-44) : grille de vignettes embarquee directement (pas un dialogue de
    // selection comme pour les skins) -- selectionner une case choisit le fond immediatement, avec
    // les memes controles d'import/renommage/suppression que la bibliotheque de skins (LOT-43).
    _backgroundView = new AssetThumbnailView(this);
    _backgroundView->setAssetFamily(AssetFamily::Background);
    _backgroundView->setDirectory(_backgroundsDirectory);
    _ui->backgroundContainerLayout->addWidget(_backgroundView);
    connect(_backgroundView, &AssetThumbnailView::assetSelected, this,
            [this](const QString& fileName) { emit backgroundChanged(fileName); });
    connect(_ui->levelSkinSetSelector, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index < 0) {
            return;
        }
        const QString text = _ui->levelSkinSetSelector->itemText(index);
        emit levelSkinSetChanged(text == defaultSetLabel(_loc) ? QString() : text);
    });

    // Section « Cadrage » (LOT-64, EX-EDIT-028) : la taille de salle n'a de sens qu'en mode
    // *par salle* -- masquee pour les deux autres modes plutot que grisee, pour ne pas laisser
    // deviner un reglage sans effet (meme choix que le hint "mode Texture requis" de la
    // section « Fond »).
    for (const core::CameraFramingMode mode : CAMERA_FRAMING_MODES) {
        _ui->cameraFramingModeSelector->addItem(cameraFramingModeLabel(_loc, mode));
    }
    connect(_ui->cameraFramingModeSelector, &QComboBox::currentIndexChanged, this,
            [this](int) { emitCameraFramingChanged(); });
    connect(_ui->cameraFramingRoomWidthSpin, &QSpinBox::valueChanged, this,
            [this](int) { emitCameraFramingChanged(); });
    connect(_ui->cameraFramingRoomHeightSpin, &QSpinBox::valueChanged, this,
            [this](int) { emitCameraFramingChanged(); });

    // Zones de camera dessinees a la main (mode *par salle*, LOT-64) : dessinees depuis le
    // canevas (outil dedie, GameViewport::addCameraZoneFromDrag), retirees depuis ce tableau --
    // meme separation que la section « Objets » (dessine/assigne au canevas, retire au panneau).
    _ui->cameraFramingZonesTable->setModel(_cameraFramingZonesModel);
    _ui->cameraFramingZonesTable->horizontalHeader()->setStretchLastSection(true);
    _ui->cameraFramingZonesTable->verticalHeader()->setVisible(false);
    connect(_ui->cameraFramingZonesTable->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [this] { onCameraFramingZonesSelectionChanged(); });
    connect(_ui->cameraFramingZonesRemoveButton, &QPushButton::clicked, this,
            [this] { onCameraFramingZonesRemoveClicked(); });

    // Section « Objets » (LOT-45) : grille de vignettes pour choisir l'asset actif de l'outil
    // « Texture par instance » (aucune entree "(aucun)" -- selectionner une case vide n'a pas de
    // sens, on choisit toujours un asset avant d'assigner), plus le tableau des surcharges deja
    // posees sur le niveau, avec surbrillance croisee et retrait.
    _objectView = new AssetThumbnailView(this);
    _objectView->setAssetFamily(AssetFamily::Object);
    _objectView->setNoneOptionVisible(false);
    _objectView->setDirectory(_objectsDirectory);
    _ui->objectPickerContainerLayout->addWidget(_objectView);
    connect(_objectView, &AssetThumbnailView::assetSelected, this,
            [this](const QString& fileName) { emit textureOverrideAssetSelected(fileName); });

    _ui->objectsTable->setModel(_objectsModel);
    _ui->objectsTable->horizontalHeader()->setStretchLastSection(true);
    _ui->objectsTable->verticalHeader()->setVisible(false);
    connect(_ui->objectsTable->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            [this] { onObjectsSelectionChanged(); });
    connect(_ui->objectsRemoveButton, &QPushButton::clicked, this,
            [this] { onObjectsRemoveClicked(); });

    // Section « Animations » (LOT-47) : asset anime par defaut de chaque famille de mecanisme a
    // etat, stocke comme un skin de type ordinaire (meme catalogue, meme jeu courant) -- avec le
    // diagnostic des clips manquants et un apercu qui rejoue le meme catalogue d'animation que le
    // jeu (hmi::AnimationCatalog), pas une reimplementation parallele.
    _ui->animationsTree->setModel(_animationsModel);
    _ui->animationsTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    connect(_ui->animationsTree, &QTreeView::doubleClicked, this,
            &TexturePanel::onAnimationsAssetActivated);
    connect(_ui->animationsTree->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            [this] { onAnimationsSelectionChanged(); });
    connect(_animationPreviewTimer, &QTimer::timeout, this, &TexturePanel::tickAnimationPreview);
    _animationPreviewTimer->start(ANIMATION_PREVIEW_INTERVAL_MS);
}

TexturePanel::~TexturePanel() = default;

void TexturePanel::setCatalog(SkinCatalog* catalog) {
    _catalog = catalog;
    _currentSet = catalog == nullptr ? std::string{} : catalog->defaultSetName();
    rebuildTree();
    rebuildLevelSkinSetSelector();
    rebuildAnimationsTree();
}

void TexturePanel::setLevelProperties(const std::optional<std::string>& background,
                                      const std::optional<std::string>& skinSet) {
    _levelBackground = background;
    _levelSkinSet = skinSet;
    {
        // Synchronisation programmatique : ne doit jamais reemettre backgroundChanged, sous peine
        // de reboucler avec l'appelant (GameViewport::setLevelBackground -> draftChanged ->
        // setLevelProperties -> ...).
        const QSignalBlocker blocker(_backgroundView);
        _backgroundView->selectAsset(background.value_or(std::string{}));
    }
    rebuildLevelSkinSetSelector();  // bloque son propre signal en interne
}

void TexturePanel::setLevelCameraFraming(const core::CameraFramingConfig& cameraFraming) {
    _levelCameraFraming = cameraFraming;
    rebuildCameraFramingSelector();  // bloque son propre signal en interne
}

void TexturePanel::retranslateUi(const Localization& loc) {
    _loc = &loc;
    _ui->setLabel->setText(QString::fromStdString(loc.text("textures.skin_set")));
    // Distingue explicitement ce selecteur (session d'edition) de celui de la section « Fond »
    // (niveau, textures.level_skin_set_tooltip) -- deux etats distincts derriere un libelle proche
    // (LOT-57 TACHE-03).
    _ui->setLabel->setToolTip(QString::fromStdString(loc.text("textures.skin_set_tooltip")));
    _ui->reloadButton->setText(QString::fromStdString(loc.text("textures.reload")));
    _ui->reloadButton->setToolTip(QString::fromStdString(loc.text("textures.reload_tooltip")));
    _ui->sections->setTabText(0, QString::fromStdString(loc.text("textures.section_skins")));
    _ui->sections->setTabText(1, QString::fromStdString(loc.text("textures.section_background")));
    _ui->sections->setTabText(2,
                              QString::fromStdString(loc.text("textures.section_camera_framing")));
    _ui->sections->setTabText(3, QString::fromStdString(loc.text("textures.section_objects")));
    _ui->sections->setTabText(4, QString::fromStdString(loc.text("textures.section_animations")));
    _ui->backgroundModeHintLabel->setText(
        QString::fromStdString(loc.text("textures.background_mode_hint")));
    _ui->backgroundLabel->setText(QString::fromStdString(loc.text("textures.background_label")));
    _ui->levelSkinSetLabel->setText(QString::fromStdString(loc.text("textures.level_skin_set")));
    _ui->levelSkinSetLabel->setToolTip(
        QString::fromStdString(loc.text("textures.level_skin_set_tooltip")));
    _ui->cameraFramingModeLabel->setText(
        QString::fromStdString(loc.text("textures.camera_framing_mode")));
    _ui->cameraFramingRoomWidthLabel->setText(
        QString::fromStdString(loc.text("textures.camera_framing_room_width")));
    _ui->cameraFramingRoomHeightLabel->setText(
        QString::fromStdString(loc.text("textures.camera_framing_room_height")));
    _ui->cameraFramingRoomWidthSpin->setSpecialValueText(
        QString::fromStdString(loc.text("textures.camera_framing_room_default")));
    _ui->cameraFramingRoomHeightSpin->setSpecialValueText(
        QString::fromStdString(loc.text("textures.camera_framing_room_default")));
    _ui->cameraFramingZonesHintLabel->setText(
        QString::fromStdString(loc.text("textures.camera_framing_zones_hint")));
    _ui->cameraFramingZonesRemoveButton->setText(
        QString::fromStdString(loc.text("textures.camera_framing_zones_remove")));
    _cameraFramingZonesModel->setHorizontalHeaderLabels(
        {QString::fromStdString(loc.text("textures.camera_framing_zones_column_x")),
         QString::fromStdString(loc.text("textures.camera_framing_zones_column_y")),
         QString::fromStdString(loc.text("textures.camera_framing_zones_column_width")),
         QString::fromStdString(loc.text("textures.camera_framing_zones_column_height"))});
    _ui->objectsAssetLabel->setText(
        QString::fromStdString(loc.text("textures.objects_asset_label")));
    _ui->objectsListLabel->setText(QString::fromStdString(loc.text("textures.objects_list_label")));
    _ui->objectsRemoveButton->setText(QString::fromStdString(loc.text("textures.objects_remove")));
    _objectsModel->setHorizontalHeaderLabels(
        {QString::fromStdString(loc.text("textures.objects_column_position")),
         QString::fromStdString(loc.text("textures.objects_column_asset"))});
    _ui->animationPreviewCaption->setText(
        QString::fromStdString(loc.text("textures.animations_preview")));
    _animationsModel->setHorizontalHeaderLabels(
        {QString::fromStdString(loc.text("textures.column_type")),
         QString::fromStdString(loc.text("textures.column_asset")),
         QString::fromStdString(loc.text("textures.animations_column_diagnostic"))});
    _backgroundView->retranslateUi(loc);
    _objectView->retranslateUi(loc);
    rebuildTree();
    rebuildLevelSkinSetSelector();   // le libelle "jeu par defaut" vient de changer de langue
    rebuildCameraFramingSelector();  // les libelles de mode viennent de changer de langue
    rebuildObjectRows();
    rebuildAnimationsTree();
}

// Reconstruit entierement l'arbre depuis le catalogue et le contenu du dossier de skins.
void TexturePanel::rebuildTree() {
    // Garde de reentrance : peupler le modele emet itemChanged pour chaque cellule, ce qui
    // reecrirait le catalogue avec les valeurs qu'on vient d'y lire.
    _updating = true;
    _model->clear();
    _model->setColumnCount(COLUMN_COUNT);
    if (_loc != nullptr) {
        _model->setHorizontalHeaderLabels(
            QStringList{QString::fromStdString(_loc->text("textures.column_type")),
                        QString::fromStdString(_loc->text("textures.column_asset")),
                        QString::fromStdString(_loc->text("textures.column_mode"))});
    }

    // Selecteur de jeu, resynchronise sans emettre de changement de selection parasite.
    const QSignalBlocker blocker(_ui->setSelector);
    _ui->setSelector->clear();
    if (_catalog != nullptr) {
        for (const std::string& name : _catalog->setNames()) {
            _ui->setSelector->addItem(QString::fromStdString(name));
        }
        _ui->setSelector->setCurrentText(QString::fromStdString(_currentSet));
    }

    if (_catalog != nullptr) {
        for (const SkinSection& section : buildSkinRows(*_catalog, _currentSet)) {
            auto* const header = new QStandardItem(
                QString::fromStdString(localizedTaxonomyLabel(_loc, section.label)));
            header->setEditable(false);
            for (const SkinRow& row : section.rows) {
                auto* const typeItem = new QStandardItem(
                    QString::fromStdString(localizedTaxonomyLabel(_loc, row.typeLabel)));
                typeItem->setEditable(false);
                typeItem->setData(static_cast<int>(row.type), TILE_TYPE_ROLE);

                // Le fichier se choisit par vignettes (double-clic -> AssetPickerDialog), jamais en
                // saisie libre ni en liste texte (LOT-43 TACHE-01) : la cellule affiche le nom et
                // une vignette, mais n'est plus editable directement.
                auto* const assetItem = new QStandardItem(
                    row.asset.empty() ? noneLabel(_loc) : QString::fromStdString(row.asset));
                assetItem->setEditable(false);
                assetItem->setIcon(QIcon(thumbnailFor(row.asset)));
                assetItem->setData(static_cast<int>(row.type), TILE_TYPE_ROLE);

                auto* const modeItem =
                    new QStandardItem(QString::fromLatin1(skinModeName(row.mode)));
                modeItem->setData(
                    QStringList{QString::fromLatin1(skinModeName(SkinMode::Single)),
                                QString::fromLatin1(skinModeName(SkinMode::Bitmask16))},
                    Qt::UserRole);
                modeItem->setData(static_cast<int>(row.type), TILE_TYPE_ROLE);

                header->appendRow(QList<QStandardItem*>{typeItem, assetItem, modeItem});
            }
            _model->appendRow(header);
        }
    }

    _ui->skinsTree->expandAll();
    _updating = false;
}

// Change le jeu de skins courant.
void TexturePanel::onSetChanged(int index) {
    if (_updating || _catalog == nullptr || index < 0) {
        return;
    }
    _currentSet = _ui->setSelector->itemText(index).toStdString();
    rebuildTree();
    rebuildAnimationsTree();
    emit assignmentsChanged();
}

// Applique la modification d'une cellule (fichier ou mode) au catalogue, puis l'enregistre.
void TexturePanel::onItemChanged(QStandardItem* item) {
    if (_updating || _catalog == nullptr || item == nullptr) {
        return;
    }
    const QVariant typeData = item->data(TILE_TYPE_ROLE);
    if (!typeData.isValid()) {
        return;  // en-tete de section : rien a assigner.
    }
    const auto type = static_cast<core::TileType>(typeData.toInt());

    // L'etat cible est lu sur la LIGNE entiere, pas sur la seule cellule modifiee : changer le mode
    // ne doit pas effacer le fichier, ni l'inverse.
    QStandardItem* const parent = item->parent();
    if (parent == nullptr) {
        return;
    }
    const int row = item->row();
    const QStandardItem* const assetItem = parent->child(row, COLUMN_ASSET);
    const QStandardItem* const modeItem = parent->child(row, COLUMN_MODE);
    if (assetItem == nullptr || modeItem == nullptr) {
        return;
    }

    const QString assetText = assetItem->text();
    const std::string asset =
        assetText == noneLabel(_loc) ? std::string{} : assetText.toStdString();
    const SkinMode mode =
        skinModeFromName(modeItem->text().toStdString()).value_or(SkinMode::Single);

    applySkinAssignment(*_catalog, _currentSet, type, asset, mode);
    save();
    rebuildAnimationsTree();  // le type modifie peut etre l'une des six familles de mecanismes.
    emit assignmentsChanged();
}

// Ouvre le selecteur a vignettes sur un double-clic dans la colonne Fichier (LOT-43 TACHE-01).
// Le resultat est applique via _model->setData(..., Qt::EditRole), qui declenche itemChanged ->
// onItemChanged exactement comme l'ancien delegue en liste texte : aucun cablage supplementaire.
void TexturePanel::onAssetColumnActivated(const QModelIndex& index) {
    if (_catalog == nullptr || index.column() != COLUMN_ASSET) {
        return;
    }
    if (!index.data(TILE_TYPE_ROLE).isValid()) {
        return;  // en-tete de section : rien a choisir.
    }

    const QString currentText = index.data(Qt::DisplayRole).toString();
    const std::string currentAsset =
        currentText == noneLabel(_loc) ? std::string{} : currentText.toStdString();

    AssetPickerDialog dialog(_skinsDirectory, _loc, this);
    // Le panneau ne connait que les jeux de skins : c'est lui qui sait CE QUI peut referencer un
    // asset, le widget partage n'en sait rien (voir sa doc de classe).
    dialog.setReferenceChecker([this](const std::string& fileName) {
        std::vector<std::string> lines;
        for (const AssetReference& reference : findSkinCatalogReferences(*_catalog, fileName)) {
            lines.push_back(reference.setName + " (" + reference.typeName + ")");
        }
        return lines;
    });
    dialog.selectAsset(currentAsset);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    const std::string chosen = dialog.selectedAsset();
    const QString displayText = chosen.empty() ? noneLabel(_loc) : QString::fromStdString(chosen);
    _model->setData(index, displayText, Qt::EditRole);
}

// Vignette d'un fichier de skin, decodee au premier besoin puis mise en cache par nom (dossier
// fixe pour ce panneau, _skinsDirectory).
QPixmap TexturePanel::thumbnailFor(const std::string& asset) {
    const auto cached = _thumbnails.find(asset);
    if (cached != _thumbnails.end()) {
        return cached->second;
    }

    QImage source;
    const std::optional<DecodedImage> decoded =
        asset.empty() ? std::nullopt : decodeImageFile(_skinsDirectory / asset);
    if (decoded) {
        source = toImage(*decoded);
    } else {
        // « Aucun » assigne ou fichier illisible : meme repli que le rendu (LOT-40), pour que la
        // ligne ne prenne pas silencieusement l'apparence d'un skin different.
        const ProceduralAtlasImage missing = buildMissingTextureImage();
        source = toImage(DecodedImage{missing.width, missing.height, missing.pixels});
    }

    // Resolution reelle (LOT-56 TACHE-05) : sans quoi l'ecran a 125%/150% agrandirait cette
    // vignette par interpolation, incoherent avec le plus proche voisin choisi ici.
    const qreal scale = devicePixelRatioF();
    const int pixelSize = thumbnailPixelSize(ROW_ICON_SIZE, scale);
    QPixmap pixmap = QPixmap::fromImage(
        source.scaled(pixelSize, pixelSize, Qt::KeepAspectRatio, Qt::FastTransformation));
    pixmap.setDevicePixelRatio(scale);
    _thumbnails.emplace(asset, pixmap);
    return pixmap;
}

// Reconstruit la liste du selecteur de jeu de skins DU NIVEAU (section Fond), a partir des jeux du
// catalogue plus l'entree "jeu par defaut" ; reapplique la derniere valeur poussee par
// setLevelProperties, sans reemettre levelSkinSetChanged.
void TexturePanel::rebuildLevelSkinSetSelector() {
    const QSignalBlocker blocker(_ui->levelSkinSetSelector);
    _ui->levelSkinSetSelector->clear();
    _ui->levelSkinSetSelector->addItem(defaultSetLabel(_loc));
    if (_catalog != nullptr) {
        for (const std::string& name : _catalog->setNames()) {
            _ui->levelSkinSetSelector->addItem(QString::fromStdString(name));
        }
    }
    const QString target =
        _levelSkinSet ? QString::fromStdString(*_levelSkinSet) : defaultSetLabel(_loc);
    const int index = _ui->levelSkinSetSelector->findText(target);
    _ui->levelSkinSetSelector->setCurrentIndex(index >= 0 ? index : 0);
}

// Reconstruit le selecteur de mode et resynchronise les champs de taille de salle (LOT-64, voir
// en-tete) : bloque son propre signal (comme rebuildLevelSkinSetSelector) pour ne jamais reemettre
// cameraFramingChanged lors d'une synchronisation programmatique.
void TexturePanel::rebuildCameraFramingSelector() {
    {
        const QSignalBlocker modeBlocker(_ui->cameraFramingModeSelector);
        _ui->cameraFramingModeSelector->clear();
        for (const core::CameraFramingMode mode : CAMERA_FRAMING_MODES) {
            _ui->cameraFramingModeSelector->addItem(cameraFramingModeLabel(_loc, mode));
        }
        const auto found = std::find(CAMERA_FRAMING_MODES.begin(), CAMERA_FRAMING_MODES.end(),
                                     _levelCameraFraming.mode);
        const int index = found == CAMERA_FRAMING_MODES.end()
                              ? 0
                              : static_cast<int>(found - CAMERA_FRAMING_MODES.begin());
        _ui->cameraFramingModeSelector->setCurrentIndex(index);
    }
    {
        const QSignalBlocker widthBlocker(_ui->cameraFramingRoomWidthSpin);
        const QSignalBlocker heightBlocker(_ui->cameraFramingRoomHeightSpin);
        _ui->cameraFramingRoomWidthSpin->setValue(_levelCameraFraming.roomWidthTiles.value_or(0));
        _ui->cameraFramingRoomHeightSpin->setValue(_levelCameraFraming.roomHeightTiles.value_or(0));
    }
    // La taille de vue a un sens en mode "par salle" (grille automatique, LOT-64) ET "suivi"
    // (taille fixe de la camera, LOT-64) -- pas en mode "niveau entier".
    const bool sizeApplies = _levelCameraFraming.mode == core::CameraFramingMode::PerRoom ||
                             _levelCameraFraming.mode == core::CameraFramingMode::Follow;
    _ui->cameraFramingRoomSizeWidget->setVisible(sizeApplies);
    // Les zones dessinees a la main (LOT-64) n'ont de sens qu'en mode "par salle".
    _ui->cameraFramingZonesWidget->setVisible(_levelCameraFraming.mode ==
                                              core::CameraFramingMode::PerRoom);
    rebuildCameraFramingZonesTable();
}

// Reconstruit le tableau des zones de camera dessinees a la main depuis _levelCameraFraming.zones
// (LOT-64) -- meme patron que rebuildObjectRows pour le tableau des surcharges de texture.
void TexturePanel::rebuildCameraFramingZonesTable() {
    _cameraFramingZonesModel->removeRows(0, _cameraFramingZonesModel->rowCount());
    for (const core::CameraZone& zone : _levelCameraFraming.zones) {
        QList<QStandardItem*> row;
        for (const int value : {zone.x, zone.y, zone.width, zone.height}) {
            auto* const item = new QStandardItem(QString::number(value));
            item->setEditable(false);
            row.push_back(item);
        }
        _cameraFramingZonesModel->appendRow(row);
    }
    _ui->cameraFramingZonesRemoveButton->setEnabled(false);
}

// Construit le core::CameraFramingConfig resultant de l'etat courant des widgets de mode/taille et
// l'emet -- point d'appel unique (voir en-tete), jamais reconstruit ailleurs. Les zones dessinees a
// la main (LOT-64) ne sont jamais modifiees ici (mutees directement via
// GameViewport::addCameraZoneFromDrag/removeCameraZone) : reportees telles quelles tant que le
// mode reste "par salle", videes sinon (invalides pour tout autre mode,
// core::validateCameraFramingConfig).
void TexturePanel::emitCameraFramingChanged() {
    const int modeIndex = _ui->cameraFramingModeSelector->currentIndex();
    if (modeIndex < 0 || static_cast<std::size_t>(modeIndex) >= CAMERA_FRAMING_MODES.size()) {
        return;  // selecteur pas encore peuple (construction) : rien a emettre.
    }
    core::CameraFramingConfig cameraFraming;
    cameraFraming.mode = CAMERA_FRAMING_MODES[static_cast<std::size_t>(modeIndex)];
    const bool sizeApplies = cameraFraming.mode == core::CameraFramingMode::PerRoom ||
                             cameraFraming.mode == core::CameraFramingMode::Follow;
    _ui->cameraFramingRoomSizeWidget->setVisible(sizeApplies);
    _ui->cameraFramingZonesWidget->setVisible(cameraFraming.mode ==
                                              core::CameraFramingMode::PerRoom);
    if (sizeApplies) {
        const int width = _ui->cameraFramingRoomWidthSpin->value();
        const int height = _ui->cameraFramingRoomHeightSpin->value();
        // 0 = "Par defaut" (specialValueText) : reste std::nullopt, jamais une taille de zero
        // case (validee comme invalide par core::validateCameraFramingConfig).
        cameraFraming.roomWidthTiles = width > 0 ? std::optional<int>(width) : std::nullopt;
        cameraFraming.roomHeightTiles = height > 0 ? std::optional<int>(height) : std::nullopt;
    }
    if (cameraFraming.mode == core::CameraFramingMode::PerRoom) {
        cameraFraming.zones = _levelCameraFraming.zones;
    }
    _levelCameraFraming = cameraFraming;
    rebuildCameraFramingZonesTable();
    emit cameraFramingChanged(cameraFraming);
}

bool TexturePanel::event(QEvent* event) {
    if (event->type() == QEvent::ScreenChangeInternal) {
        // Un deplacement vers un ecran d'echelle differente doit regenerer les vignettes des
        // lignes et de l'apercu d'animation (LOT-56 TACHE-05) : les caches decodes restent
        // valables (memes fichiers), seule la mise a l'echelle doit etre rejouee.
        _thumbnails.clear();
        rebuildTree();
        tickAnimationPreview();
    }
    return QWidget::event(event);
}

void TexturePanel::reloadAssets() {
    _thumbnails.clear();
    rebuildTree();
    rebuildLevelSkinSetSelector();
    _backgroundView->clearCache();
    _backgroundView->refresh();
    // La selection a pu etre perdue au rafraichissement (fichier renomme/supprime hors
    // application) : la resynchroniser avec la propriete du niveau, sans reemettre le signal.
    const QSignalBlocker blocker(_backgroundView);
    _backgroundView->selectAsset(_levelBackground.value_or(std::string{}));
    _objectView->clearCache();
    _objectView->refresh();
    // Un fichier d'animation a pu changer hors application : le diagnostic et l'apercu doivent
    // relire le disque, pas une copie memorisee (LOT-43 TACHE-03, LOT-47).
    _animationPreviewSheet = QImage();
    _animationPreviewDescription.reset();
    rebuildAnimationsTree();
}

// Resynchronise la section « Objets » avec le brouillon courant (LOT-45).
void TexturePanel::refreshObjects(const core::LevelDraft& draft) {
    _objectRows = draft.textureOverrides();
    // Tri stable par position (colonne puis ligne) : un grand niveau peut porter beaucoup de
    // surcharges, une liste dans l'ordre d'insertion serait illisible.
    std::stable_sort(
        _objectRows.begin(), _objectRows.end(),
        [](const core::TileTextureOverride& lhs, const core::TileTextureOverride& rhs) {
            if (lhs.position.column != rhs.position.column) {
                return lhs.position.column < rhs.position.column;
            }
            return lhs.position.row < rhs.position.row;
        });
    rebuildObjectRows();
}

// Reconstruit le tableau depuis _objectRows.
void TexturePanel::rebuildObjectRows() {
    _objectsModel->removeRows(0, _objectsModel->rowCount());
    for (const core::TileTextureOverride& override : _objectRows) {
        auto* const positionItem = new QStandardItem(positionText(override.position));
        auto* const assetItem = new QStandardItem(QString::fromStdString(override.assetName));
        positionItem->setEditable(false);
        assetItem->setEditable(false);
        _objectsModel->appendRow(QList<QStandardItem*>{positionItem, assetItem});
    }
    _ui->objectsRemoveButton->setEnabled(false);
}

void TexturePanel::onObjectsSelectionChanged() {
    const QModelIndexList selected = _ui->objectsTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        _ui->objectsRemoveButton->setEnabled(false);
        emit textureOverrideSelectionChanged(std::nullopt);
        return;
    }
    const int row = selected.first().row();
    if (row < 0 || static_cast<std::size_t>(row) >= _objectRows.size()) {
        _ui->objectsRemoveButton->setEnabled(false);
        emit textureOverrideSelectionChanged(std::nullopt);
        return;
    }
    _ui->objectsRemoveButton->setEnabled(true);
    emit textureOverrideSelectionChanged(_objectRows[static_cast<std::size_t>(row)].position);
}

void TexturePanel::onObjectsRemoveClicked() {
    const QModelIndexList selected = _ui->objectsTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        return;
    }
    const int row = selected.first().row();
    if (row < 0 || static_cast<std::size_t>(row) >= _objectRows.size()) {
        return;
    }
    emit textureOverrideRemoveRequested(_objectRows[static_cast<std::size_t>(row)].position);
}

// Meme patron que onObjectsSelectionChanged, pour le tableau des zones de camera (LOT-64).
void TexturePanel::onCameraFramingZonesSelectionChanged() {
    const QModelIndexList selected = _ui->cameraFramingZonesTable->selectionModel()->selectedRows();
    _ui->cameraFramingZonesRemoveButton->setEnabled(!selected.isEmpty());
}

// Meme patron que onObjectsRemoveClicked : l'index de ligne EST l'index de la zone dans
// _levelCameraFraming.zones, rebuildCameraFramingZonesTable() les peuplant dans le meme ordre.
void TexturePanel::onCameraFramingZonesRemoveClicked() {
    const QModelIndexList selected = _ui->cameraFramingZonesTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        return;
    }
    const int row = selected.first().row();
    if (row < 0) {
        return;
    }
    emit cameraZoneRemoveRequested(static_cast<std::size_t>(row));
}

// Reconstruit l'arbre de la section « Animations » (LOT-47 TACHE-04) : logique hors du widget
// (hmi::buildMechanismAnimationRows), meme patron que rebuildTree pour les skins.
void TexturePanel::rebuildAnimationsTree() {
    _animationsModel->removeRows(0, _animationsModel->rowCount());
    if (_loc != nullptr) {
        _animationsModel->setHorizontalHeaderLabels(
            {QString::fromStdString(_loc->text("textures.column_type")),
             QString::fromStdString(_loc->text("textures.column_asset")),
             QString::fromStdString(_loc->text("textures.animations_column_diagnostic"))});
    }
    if (_catalog == nullptr) {
        return;
    }

    for (const MechanismAnimationRow& row :
         buildMechanismAnimationRows(*_catalog, _currentSet, _skinsDirectory)) {
        auto* const typeItem =
            new QStandardItem(QString::fromStdString(localizedTaxonomyLabel(_loc, row.typeLabel)));
        typeItem->setEditable(false);
        typeItem->setData(static_cast<int>(row.type), TILE_TYPE_ROLE);

        auto* const assetItem = new QStandardItem(
            row.asset.empty() ? noneLabel(_loc) : QString::fromStdString(row.asset));
        assetItem->setEditable(false);
        assetItem->setIcon(QIcon(thumbnailFor(row.asset)));
        assetItem->setData(static_cast<int>(row.type), TILE_TYPE_ROLE);

        auto* const diagnosticItem = new QStandardItem(diagnosticText(_loc, row));
        diagnosticItem->setEditable(false);
        diagnosticItem->setData(static_cast<int>(row.type), TILE_TYPE_ROLE);

        _animationsModel->appendRow(QList<QStandardItem*>{typeItem, assetItem, diagnosticItem});
    }
}

// Ouvre le selecteur a vignettes sur un double-clic dans la colonne Fichier de l'arbre
// « Animations » (meme patron que onAssetColumnActivated : un mecanisme est un type de tuile
// comme un autre pour hmi::SkinCatalog, seul le mode SkinMode::Single a un sens ici, EX-REN-046).
void TexturePanel::onAnimationsAssetActivated(const QModelIndex& index) {
    if (_catalog == nullptr || index.column() != ANIMATIONS_COLUMN_ASSET) {
        return;
    }
    const QVariant typeData = index.data(TILE_TYPE_ROLE);
    if (!typeData.isValid()) {
        return;
    }
    const auto type = static_cast<core::TileType>(typeData.toInt());

    const QString currentText = index.data(Qt::DisplayRole).toString();
    const std::string currentAsset =
        currentText == noneLabel(_loc) ? std::string{} : currentText.toStdString();

    AssetPickerDialog dialog(_skinsDirectory, _loc, this);
    dialog.setReferenceChecker([this](const std::string& fileName) {
        std::vector<std::string> lines;
        for (const AssetReference& reference : findSkinCatalogReferences(*_catalog, fileName)) {
            lines.push_back(reference.setName + " (" + reference.typeName + ")");
        }
        return lines;
    });
    dialog.selectAsset(currentAsset);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    // Toujours SkinMode::Single : un mecanisme a etat n'a pas de voisinage a raccorder (LOT-47
    // TACHE-01, meme famille que dangers/blocs dans la section « Skins »).
    applySkinAssignment(*_catalog, _currentSet, type, dialog.selectedAsset(), SkinMode::Single);
    save();
    rebuildTree();  // le meme catalogue alimente aussi la section « Skins ».
    rebuildAnimationsTree();
    emit assignmentsChanged();
}

// Charge l'apercu (spritesheet decodee + description d'animation) de la ligne selectionnee.
void TexturePanel::onAnimationsSelectionChanged() {
    _animationPreviewSheet = QImage();
    _animationPreviewDescription.reset();
    _animationPreviewAnimation = core::Animation{};

    const QModelIndexList selected = _ui->animationsTree->selectionModel()->selectedRows();
    if (selected.isEmpty() || _catalog == nullptr) {
        return;
    }
    const QVariant typeData = selected.first().data(TILE_TYPE_ROLE);
    if (!typeData.isValid()) {
        return;
    }
    const auto type = static_cast<core::TileType>(typeData.toInt());
    const std::optional<SkinEntry> entry = _catalog->resolve(_currentSet, type);
    if (!entry || entry->asset.empty()) {
        return;
    }

    const std::optional<DecodedImage> decoded = decodeImageFile(_skinsDirectory / entry->asset);
    if (!decoded) {
        return;
    }
    _animationPreviewSheet = toImage(*decoded);

    const AnimationDescriptionResult result = AnimationCatalog::loadFromFile(
        _skinsDirectory / AnimationCatalog::descriptorFileName(entry->asset));
    if (!result.ok()) {
        return;  // asset non anime (ou description invalide) : l'apercu reste sur l'image entiere.
    }
    _animationPreviewDescription = result.description;
    _animationPreviewAnimation.clips =
        std::make_shared<core::ClipSet>(_animationPreviewDescription->clips);
    // Etat "au repos" (porte fermee, interrupteur/plaque relache, danger inoffensif...) : la
    // transition elle-meme se voit en jeu, l'apercu montre l'apparence de base (LOT-47 TACHE-04).
    const std::string clipName = mechanismTargetClip(type, false).value_or(std::string{});
    const int clipIndex = _animationPreviewDescription->clips.indexOf(clipName);
    _animationPreviewAnimation.clipIndex = clipIndex >= 0 ? clipIndex : 0;
}

// Avance l'horloge d'apercu d'un pas de minuterie et rafraichit la vignette (LOT-47 TACHE-04).
void TexturePanel::tickAnimationPreview() {
    if (_animationPreviewSheet.isNull()) {
        _ui->animationPreviewLabel->clear();
        return;
    }

    QImage frame;
    if (_animationPreviewDescription && _animationPreviewAnimation.clips) {
        core::advanceAnimation(_animationPreviewAnimation,
                               static_cast<float>(ANIMATION_PREVIEW_INTERVAL_MS) / 1000.0f);
        const core::AtlasRegion region = AnimationCatalog::currentFrameRegion(
            *_animationPreviewDescription, _animationPreviewAnimation);
        frame = _animationPreviewSheet.copy(region.x, region.y, region.width, region.height);
    } else {
        frame = _animationPreviewSheet;
    }

    // Resolution reelle (LOT-56 TACHE-05), meme raison que thumbnailFor.
    const qreal scale = devicePixelRatioF();
    const int pixelWidth = thumbnailPixelSize(frame.width() * ANIMATION_PREVIEW_SCALE, scale);
    const int pixelHeight = thumbnailPixelSize(frame.height() * ANIMATION_PREVIEW_SCALE, scale);
    QPixmap pixmap = QPixmap::fromImage(
        frame.scaled(pixelWidth, pixelHeight, Qt::KeepAspectRatio, Qt::FastTransformation));
    pixmap.setDevicePixelRatio(scale);
    _ui->animationPreviewLabel->setPixmap(pixmap);
}

// Enregistre le catalogue au chemin deploye, comme l'enregistrement d'un niveau.
void TexturePanel::save() {
    if (_catalog == nullptr) {
        return;
    }
    if (!_catalog->saveToFile(_catalogPath)) {
        // Echec recuperable : l'assignation reste active en memoire et se voit a l'ecran, elle ne
        // survivra simplement pas au redemarrage.
        GRAPHICS_LOG_WARNING("Panneau Textures : enregistrement de " + _catalogPath.string() +
                             " impossible.");
    }
}

}  // namespace hmi
