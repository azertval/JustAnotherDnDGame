// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/AssetThumbnailView.h"

#include <QFileDialog>
#include <QIcon>
#include <QImage>
#include <QInputDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QString>
#include <cstdint>
#include <cstring>
#include <optional>
#include <utility>

#include "HMI/Editor/AssetFileOperations.h"
#include "HMI/Editor/AssetLibrary.h"
#include "HMI/Editor/ThumbnailGeometry.h"
#include "HMI/Graphics/MissingTexture.h"
#include "HMI/Graphics/ProceduralAtlas.h"
#include "HMI/Graphics/TextureLoader.h"
#include "HMI/HmiLog.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Localization/Localization.h"
#include "ui_AssetThumbnailView.h"

namespace hmi {

namespace {

// Cote des vignettes, en pixels d'ecran (coherent avec l'iconSize du .ui) : jeton de taille
// (LOT-56), commun aux deux themes d'editeur (SizeTokens partage entre les portees).
const int THUMBNAIL_SIZE = editorDarkTokens().size.assetThumbnail;

// Role portant le nom de fichier d'un item ("" pour l'entree "(aucun)").
constexpr int FILE_NAME_ROLE = Qt::UserRole + 1;

// Texte localise d'une cle (repli sur la cle si aucun catalogue -- ne survient pas en pratique).
[[nodiscard]] QString t(const Localization* loc, const char* key) {
    return loc != nullptr ? QString::fromStdString(loc->text(key)) : QString::fromLatin1(key);
}

// Convertit des pixels RGBA decodes en QImage, sans copier la source deux fois. Meme conversion
// que PalettePanel (LOT-42) : trop petite pour justifier une fonction partagee entre deux widgets
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

// Signale l'echec eventuel d'une operation a l'utilisateur (jamais silencieux) et le journalise.
void reportIfError(QWidget* parent, const QString& title, const FileOperationResult& result) {
    if (!result.ok()) {
        HMI_LOG_WARNING("Bibliotheque d'assets : operation fichier echouee : " + result.error);
        QMessageBox::warning(parent, title, QString::fromStdString(result.error));
    }
}

}  // namespace

AssetThumbnailView::AssetThumbnailView(QWidget* parent)
    : QWidget(parent), _ui(std::make_unique<Ui::AssetThumbnailView>()) {
    _ui->setupUi(this);

    connect(_ui->filterField, &QLineEdit::textChanged, this, &AssetThumbnailView::onFilterChanged);
    connect(_ui->assetList, &QListWidget::currentItemChanged, this,
            &AssetThumbnailView::onCurrentItemChanged);
    connect(_ui->assetList, &QListWidget::itemActivated, this,
            &AssetThumbnailView::onItemActivated);
    connect(_ui->importButton, &QPushButton::clicked, this, &AssetThumbnailView::onImport);
    connect(_ui->renameButton, &QPushButton::clicked, this, &AssetThumbnailView::onRename);
    connect(_ui->duplicateButton, &QPushButton::clicked, this, &AssetThumbnailView::onDuplicate);
    connect(_ui->deleteButton, &QPushButton::clicked, this, &AssetThumbnailView::onDelete);
}

AssetThumbnailView::~AssetThumbnailView() = default;

void AssetThumbnailView::setDirectory(std::filesystem::path directory) {
    _directory = std::move(directory);
    refresh();
}

void AssetThumbnailView::setNoneOptionVisible(bool visible) {
    _noneOptionVisible = visible;
    rebuildItems();
}

void AssetThumbnailView::refresh() {
    rebuildItems();
}

void AssetThumbnailView::clearCache() {
    _thumbnails.clear();
}

void AssetThumbnailView::selectAsset(const std::string& fileName) {
    const QString target = QString::fromStdString(fileName);
    for (int row = 0; row < _ui->assetList->count(); ++row) {
        QListWidgetItem* const item = _ui->assetList->item(row);
        if (item->data(FILE_NAME_ROLE).toString() == target) {
            _ui->assetList->setCurrentItem(item);
            return;
        }
    }
}

std::string AssetThumbnailView::selectedAsset() const {
    QListWidgetItem* const item = _ui->assetList->currentItem();
    return item == nullptr ? std::string{} : item->data(FILE_NAME_ROLE).toString().toStdString();
}

void AssetThumbnailView::retranslateUi(const Localization& loc) {
    _loc = &loc;
    _ui->filterField->setPlaceholderText(t(_loc, "assets.search"));
    _ui->importButton->setText(t(_loc, "assets.import"));
    _ui->renameButton->setText(t(_loc, "assets.rename"));
    _ui->duplicateButton->setText(t(_loc, "assets.duplicate"));
    _ui->deleteButton->setText(t(_loc, "assets.delete"));
    rebuildItems();  // l'entree "(aucun)" porte un libelle traduit.
}

// Reconstruit entierement la grille depuis le dossier courant et le filtre de recherche.
void AssetThumbnailView::rebuildItems() {
    const std::string previousSelection = selectedAsset();
    _ui->assetList->clear();

    if (_noneOptionVisible) {
        auto* const noneItem = new QListWidgetItem(t(_loc, "assets.none"));
        noneItem->setData(FILE_NAME_ROLE, QString());
        _ui->assetList->addItem(noneItem);
    }
    for (const std::string& fileName : listAssetFiles(_directory, _filterText)) {
        auto* const item =
            new QListWidgetItem(QIcon(thumbnailFor(fileName)), QString::fromStdString(fileName));
        item->setData(FILE_NAME_ROLE, QString::fromStdString(fileName));
        _ui->assetList->addItem(item);
    }

    selectAsset(
        previousSelection);  // conserve la selection a travers un filtrage/rafraichissement.
}

// Vignette d'un fichier, decodee au premier besoin puis mise en cache par chemin absolu.
QPixmap AssetThumbnailView::thumbnailFor(const std::string& fileName) {
    const std::string key = (_directory / fileName).string();
    const auto cached = _thumbnails.find(key);
    if (cached != _thumbnails.end()) {
        return cached->second;
    }

    // Decodage CPU uniquement (hmi::decodeImageFile) : ce widget ne doit jamais dependre du
    // device Direct3D 11.
    QImage source;
    const std::optional<DecodedImage> decoded = decodeImageFile(_directory / fileName);
    if (decoded) {
        source = toImage(*decoded);
    } else {
        // Repli coherent avec le damier magenta du rendu (LOT-40) : un fichier illisible reste
        // visible dans la grille plutot que de faire planter l'affichage.
        const ProceduralAtlasImage missing = buildMissingTextureImage();
        source = toImage(DecodedImage{missing.width, missing.height, missing.pixels});
    }

    // Plus proche voisin : les assets sont du pixel art, une vignette interpolee serait floue.
    // Rendue a la resolution REELLE (LOT-56 TACHE-05) : sans quoi un ecran a 125%/150% l'agrandit
    // par interpolation cote Qt, contradictoire avec le filtrage plus proche voisin choisi ici.
    const qreal scale = devicePixelRatioF();
    const int pixelSize = thumbnailPixelSize(THUMBNAIL_SIZE, scale);
    QPixmap pixmap = QPixmap::fromImage(
        source.scaled(pixelSize, pixelSize, Qt::KeepAspectRatio, Qt::FastTransformation));
    pixmap.setDevicePixelRatio(scale);
    _thumbnails.emplace(key, pixmap);
    return pixmap;
}

bool AssetThumbnailView::event(QEvent* event) {
    if (event->type() == QEvent::ScreenChangeInternal) {
        // Un deplacement vers un ecran d'echelle differente doit regenerer les vignettes (LOT-56
        // TACHE-05) : le cache est a l'ancienne resolution, sinon le defaut reviendrait sans rien
        // le signaler.
        clearCache();
        refresh();
    }
    return QWidget::event(event);
}

void AssetThumbnailView::onFilterChanged(const QString& text) {
    _filterText = text.toStdString();
    rebuildItems();
}

void AssetThumbnailView::onCurrentItemChanged(QListWidgetItem* current, QListWidgetItem*) {
    if (current == nullptr) {
        return;
    }
    emit assetSelected(current->data(FILE_NAME_ROLE).toString());
}

void AssetThumbnailView::onItemActivated(QListWidgetItem* item) {
    if (item == nullptr) {
        return;
    }
    emit assetActivated(item->data(FILE_NAME_ROLE).toString());
}

// References lisibles a un fichier, decrites en un seul message (vide si aucune ou aucun
// verificateur configure). Le widget ignore CE QUI referencerait l'asset : c'est la section
// appelante qui fournit le verificateur (voir la doc de classe).
QString AssetThumbnailView::referencesWarning(const std::string& fileName) const {
    if (!_referenceChecker) {
        return {};
    }
    const std::vector<std::string> references = _referenceChecker(fileName);
    if (references.empty()) {
        return {};
    }
    QString joined;
    for (std::size_t index = 0; index < references.size(); ++index) {
        if (index > 0) {
            joined += QStringLiteral("; ");
        }
        joined += QString::fromStdString(references[index]);
    }
    return joined;
}

void AssetThumbnailView::onImport() {
    const QString title = t(_loc, "assets.import");
    const QString path =
        QFileDialog::getOpenFileName(this, title, QString(), QStringLiteral("Images (*.png)"));
    if (path.isEmpty()) {
        return;
    }
    const std::filesystem::path source(path.toStdString());

    // Decodage cote appelant : AssetFileOperations reste pure (aucune dependance Qt), seul ce
    // widget Qt decode l'image avant de lui transmettre les dimensions.
    const std::optional<DecodedImage> decoded = decodeImageFile(source);
    if (!decoded) {
        QMessageBox::warning(this, title, t(_loc, "assets.operation_failed"));
        return;
    }

    const AssetFileOperations ops(_directory);
    const FileOperationResult result = ops.import(source, _family, decoded->width, decoded->height);
    reportIfError(this, title, result);
    if (!result.ok()) {
        return;
    }
    HMI_LOG_INFO("Bibliotheque d'assets : importe « " + result.path.filename().string() + " ».");
    refresh();
    selectAsset(result.path.filename().string());
    emit assetsChanged();
}

void AssetThumbnailView::onRename() {
    const std::string current = selectedAsset();
    if (current.empty()) {
        return;  // rien de selectionne, ou entree "(aucun)" : rien a renommer.
    }
    bool accepted = false;
    const QString newName = QInputDialog::getText(
        this, t(_loc, "assets.rename"), t(_loc, "assets.rename_prompt"), QLineEdit::Normal,
        QString::fromStdString(std::filesystem::path(current).stem().string()), &accepted);
    if (!accepted || newName.isEmpty()) {
        return;
    }

    const QString warning = referencesWarning(current);
    if (!warning.isEmpty()) {
        const QMessageBox::StandardButton answer = QMessageBox::question(
            this, t(_loc, "assets.rename"),
            t(_loc, "assets.references_warning").arg(QString::fromStdString(current)).arg(warning));
        if (answer != QMessageBox::Yes) {
            return;
        }
    }

    const AssetFileOperations ops(_directory);
    const FileOperationResult result = ops.rename(_directory / current, newName.toStdString());
    reportIfError(this, t(_loc, "assets.rename"), result);
    if (!result.ok()) {
        return;
    }
    HMI_LOG_INFO("Bibliotheque d'assets : renomme « " + current + " » en « " +
                 result.path.filename().string() + " ».");
    refresh();
    selectAsset(result.path.filename().string());
    emit assetsChanged();  // la section doit rafraichir toute assignation qui visait l'ancien nom.
}

void AssetThumbnailView::onDuplicate() {
    const std::string current = selectedAsset();
    if (current.empty()) {
        return;
    }
    // Aucune verification de reference : dupliquer ne casse jamais rien, le fichier d'origine
    // reste intact sous son nom.
    const AssetFileOperations ops(_directory);
    const FileOperationResult result = ops.duplicate(_directory / current);
    reportIfError(this, t(_loc, "assets.duplicate"), result);
    if (!result.ok()) {
        return;
    }
    HMI_LOG_INFO("Bibliotheque d'assets : duplique « " + current + " ».");
    refresh();
    selectAsset(result.path.filename().string());
    emit assetsChanged();
}

void AssetThumbnailView::onDelete() {
    const std::string current = selectedAsset();
    if (current.empty()) {
        return;
    }

    // Toujours confirmer, meme sans reference : la suppression est irreversible (point
    // d'attention LOT-43 TACHE-02).
    QString message = t(_loc, "assets.delete_confirm").arg(QString::fromStdString(current));
    const QString warning = referencesWarning(current);
    if (!warning.isEmpty()) {
        message +=
            QStringLiteral("\n") +
            t(_loc, "assets.references_warning").arg(QString::fromStdString(current)).arg(warning);
    }
    const QMessageBox::StandardButton answer =
        QMessageBox::question(this, t(_loc, "assets.delete_title"), message);
    if (answer != QMessageBox::Yes) {
        return;
    }

    const AssetFileOperations ops(_directory);
    const FileOperationResult result = ops.remove(_directory / current);
    reportIfError(this, t(_loc, "assets.delete_title"), result);
    if (!result.ok()) {
        return;
    }
    HMI_LOG_INFO("Bibliotheque d'assets : supprime « " + current + " ».");
    refresh();
    emit assetsChanged();
}

}  // namespace hmi
