// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/PalettePanel.h"

#include <QEvent>
#include <QIcon>
#include <QImage>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QPixmap>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QString>
#include <QTreeView>
#include <QVBoxLayout>
#include <QVariant>
#include <cstdint>
#include <cstring>
#include <string>

#include "HMI/Editor/TaxonomyLabels.h"
#include "HMI/Editor/ThumbnailGeometry.h"
#include "HMI/Editor/TileTaxonomy.h"
#include "HMI/Graphics/ProceduralAtlas.h"
#include "HMI/Graphics/TextureLoader.h"
#include "HMI/Graphics/TileVisuals.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Localization/Localization.h"
#include "ui_PalettePanel.h"

namespace hmi {

namespace {

// Rôle de données portant le `core::TileType` d'une feuille (les en-têtes n'en ont pas).
constexpr int TILE_TYPE_ROLE = Qt::UserRole + 1;

// Libelle de taxonomie traduit (table libelle -> cle de TaxonomyLabels).
[[nodiscard]] QString localized(const Localization* loc, const std::string& label) {
    return QString::fromStdString(localizedTaxonomyLabel(loc, label));
}

// Cote des vignettes de la palette, en pixels d'ecran : jeton de taille (LOT-56), deja un multiple
// entier de la taille d'une case (16) -- toute autre valeur reechantillonnerait le pixel art de
// travers, meme en plus proche voisin.
const int THUMBNAIL_SIZE = editorDarkTokens().size.paletteThumbnail;

// Crée une feuille sélectionnable portant son type de tuile.
[[nodiscard]] QStandardItem* makeLeaf(const TileEntry& entry, const Localization* loc) {
    auto* const item = new QStandardItem(localized(loc, entry.label));
    item->setEditable(false);
    item->setData(static_cast<int>(entry.type), TILE_TYPE_ROLE);
    return item;
}

// Convertit des pixels RGBA decodes en QImage, sans copier la source deux fois.
[[nodiscard]] QImage toImage(const DecodedImage& decoded) {
    QImage image(decoded.width, decoded.height, QImage::Format_RGBA8888);
    for (int y = 0; y < decoded.height; ++y) {
        const std::uint32_t* const row =
            decoded.pixels.data() + (static_cast<std::size_t>(y) * decoded.width);
        std::memcpy(image.scanLine(y), row, static_cast<std::size_t>(decoded.width) * 4);
    }
    return image;
}

// Crée un en-tête (catégorie/sous-groupe) : affiché, mais non sélectionnable comme tuile.
[[nodiscard]] QStandardItem* makeHeader(const QString& label) {
    auto* const item = new QStandardItem(label);
    item->setEditable(false);
    item->setFlags(Qt::ItemIsEnabled);  // ni sélectionnable, ni porteur de type.
    return item;
}

}  // namespace

PalettePanel::PalettePanel(QWidget* parent)
    : QWidget(parent),
      _ui(std::make_unique<Ui::PalettePanel>()),
      _tree(_ui->tree),
      _model(new QStandardItemModel(this)) {
    _ui->setupUi(this);

    _tree->setModel(_model);
    _tree->setSelectionMode(QAbstractItemView::SingleSelection);

    buildModel();
    _tree->expandAll();

    connect(_tree->selectionModel(), &QItemSelectionModel::currentChanged, this,
            [this](const QModelIndex& current, const QModelIndex&) { onCurrentChanged(current); });
}

PalettePanel::~PalettePanel() = default;

void PalettePanel::buildModel() {
    for (const TileCategory& category : tileTaxonomy()) {
        QStandardItem* const categoryItem = makeHeader(localized(_loc, category.label));
        for (const TileEntry& entry : category.tiles) {
            QStandardItem* const leaf = makeLeaf(entry, _loc);
            leaf->setIcon(QIcon(thumbnailFor(entry.type)));
            categoryItem->appendRow(leaf);
        }
        for (const TileSubgroup& subgroup : category.subgroups) {
            QStandardItem* const subgroupItem = makeHeader(localized(_loc, subgroup.label));
            for (const TileEntry& entry : subgroup.tiles) {
                QStandardItem* const leaf = makeLeaf(entry, _loc);
                leaf->setIcon(QIcon(thumbnailFor(entry.type)));
                subgroupItem->appendRow(leaf);
            }
            categoryItem->appendRow(subgroupItem);
        }
        _model->appendRow(categoryItem);
    }
}

void PalettePanel::retranslateUi(const Localization& loc) {
    _loc = &loc;
    _model->clear();
    buildModel();
    _tree->expandAll();
}

// Vignette d'un type : sa couleur dans l'atlas procedural, celle que le canevas peint.
QPixmap PalettePanel::thumbnailFor(core::TileType type) {
    const ProceduralAtlasImage atlas = buildProceduralAtlasImage();
    const QImage source = toImage(
        DecodedImage{.width = atlas.width, .height = atlas.height, .pixels = atlas.pixels});
    const core::AtlasRegion region = regionForTile(type);
    const QImage tile = source.copy(region.x, region.y, region.width, region.height);

    // Mise a l'echelle en PLUS PROCHE VOISIN, a la resolution REELLE (LOT-56 TACHE-05) : sans quoi
    // l'interpolation lisse de Qt (fond d'ecran a 125%/150%) rendrait le pixel art flou, incoherent
    // avec le rendu du canevas (EX-ARCH-022).
    const qreal scale = devicePixelRatioF();
    const int pixelSize = thumbnailPixelSize(THUMBNAIL_SIZE, scale);
    QPixmap pixmap = QPixmap::fromImage(
        tile.scaled(pixelSize, pixelSize, Qt::KeepAspectRatio, Qt::FastTransformation));
    pixmap.setDevicePixelRatio(scale);
    return pixmap;
}

bool PalettePanel::event(QEvent* event) {
    if (event->type() == QEvent::ScreenChangeInternal) {
        // Un deplacement vers un ecran d'echelle differente doit regenerer les vignettes (LOT-56
        // TACHE-05) : seule la mise a l'echelle doit etre rejouee.
        _model->clear();
        buildModel();
        _tree->expandAll();
    }
    return QWidget::event(event);
}

void PalettePanel::onCurrentChanged(const QModelIndex& current) {
    const QVariant tileData = current.data(TILE_TYPE_ROLE);
    if (!tileData.isValid()) {
        return;  // en-tête (catégorie/sous-groupe) : pas un type sélectionnable.
    }
    _selected = static_cast<core::TileType>(tileData.toInt());
    emit tileSelected(_selected);
}

}  // namespace hmi
