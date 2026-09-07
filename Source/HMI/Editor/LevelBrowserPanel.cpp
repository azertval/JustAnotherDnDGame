// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Editor/LevelBrowserPanel.h"

#include <QInputDialog>
#include <QLineEdit>
#include <QListView>
#include <QMessageBox>
#include <QModelIndex>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QStandardItemModel>
#include <utility>

#include "HMI/Editor/LevelFileOperations.h"
#include "HMI/HmiLog.h"
#include "HMI/Localization/Localization.h"
#include "ui_LevelBrowserPanel.h"

namespace hmi {

namespace {

// Rôle portant le chemin absolu du fichier d'un item de la liste.
constexpr int PATH_ROLE = Qt::UserRole + 1;

// Texte localisé d'une clé (repli sur la clé si aucun catalogue — ne survient pas en pratique).
[[nodiscard]] QString t(const Localization* loc, const char* key) {
    return loc != nullptr ? QString::fromStdString(loc->text(key)) : QString::fromLatin1(key);
}

// Signale l'échec éventuel d'une opération à l'utilisateur (jamais silencieux) et le journalise.
void reportIfError(QWidget* parent, const QString& title, const FileOperationResult& result) {
    if (!result.ok()) {
        HMI_LOG_WARNING("Niveaux : operation fichier echouee : " + result.error);
        QMessageBox::warning(parent, title, QString::fromStdString(result.error));
    }
}

}  // namespace

LevelBrowserPanel::LevelBrowserPanel(std::filesystem::path levelsDir, QWidget* parent)
    : QWidget(parent),
      _ui(std::make_unique<Ui::LevelBrowserPanel>()),
      _dir(std::move(levelsDir)),
      _model(new QStandardItemModel(this)),
      _proxy(new QSortFilterProxyModel(this)) {
    _ui->setupUi(this);

    // Filtre de recherche (modèle -> proxy) et branchement de la liste (mise en page dans le .ui).
    _proxy->setSourceModel(_model);
    _proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    _ui->levelList->setModel(_proxy);
    _ui->levelList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _ui->levelList->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(_ui->searchField, &QLineEdit::textChanged, _proxy,
            &QSortFilterProxyModel::setFilterFixedString);
    connect(_ui->levelList, &QListView::doubleClicked, this, &LevelBrowserPanel::onActivated);
    connect(_ui->newButton, &QPushButton::clicked, this, &LevelBrowserPanel::onNew);
    connect(_ui->renameButton, &QPushButton::clicked, this, &LevelBrowserPanel::onRename);
    connect(_ui->duplicateButton, &QPushButton::clicked, this, &LevelBrowserPanel::onDuplicate);
    connect(_ui->deleteButton, &QPushButton::clicked, this, &LevelBrowserPanel::onDelete);

    refresh();
}

LevelBrowserPanel::~LevelBrowserPanel() = default;

void LevelBrowserPanel::refresh() {
    _model->clear();
    const LevelFileOperations ops(_dir);
    for (const std::filesystem::path& path : ops.list()) {
        auto* const item = new QStandardItem(QString::fromStdString(path.stem().string()));
        item->setEditable(false);
        item->setData(QString::fromStdString(path.string()), PATH_ROLE);
        _model->appendRow(item);
    }
    _model->sort(0);
}

std::filesystem::path LevelBrowserPanel::selectedPath() const {
    const QModelIndex proxyIndex = _ui->levelList->currentIndex();
    if (!proxyIndex.isValid()) {
        return {};
    }
    const QVariant pathData = _proxy->mapToSource(proxyIndex).data(PATH_ROLE);
    return std::filesystem::path(pathData.toString().toStdString());
}

void LevelBrowserPanel::onNew() {
    bool accepted = false;
    const QString name =
        QInputDialog::getText(this, t(_loc, "map.new_title"), t(_loc, "map.name_prompt"),
                              QLineEdit::Normal, QString(), &accepted);
    if (!accepted || name.isEmpty()) {
        return;
    }
    const LevelFileOperations ops(_dir);
    const FileOperationResult result = ops.create(name.toStdString(), 24, 14);  // taille par défaut
    if (result.ok()) {
        HMI_LOG_INFO("Niveaux : cree « " + name.toStdString() + " ».");
    }
    reportIfError(this, t(_loc, "map.operation_failed"), result);
    refresh();
}

void LevelBrowserPanel::onRename() {
    const std::filesystem::path path = selectedPath();
    if (path.empty()) {
        return;
    }
    bool accepted = false;
    const QString name = QInputDialog::getText(
        this, t(_loc, "map.rename"), t(_loc, "map.rename_prompt"), QLineEdit::Normal,
        QString::fromStdString(path.stem().string()), &accepted);
    if (!accepted || name.isEmpty()) {
        return;
    }
    const LevelFileOperations ops(_dir);
    HMI_LOG_INFO("Niveaux : renommage de « " + path.stem().string() + " » en « " +
                 name.toStdString() + " ».");
    reportIfError(this, t(_loc, "map.operation_failed"), ops.rename(path, name.toStdString()));
    refresh();
}

void LevelBrowserPanel::onDuplicate() {
    const std::filesystem::path path = selectedPath();
    if (path.empty()) {
        return;
    }
    const LevelFileOperations ops(_dir);
    HMI_LOG_INFO("Niveaux : duplication de « " + path.stem().string() + " ».");
    reportIfError(this, t(_loc, "map.operation_failed"), ops.duplicate(path));
    refresh();
}

void LevelBrowserPanel::onDelete() {
    const std::filesystem::path path = selectedPath();
    if (path.empty()) {
        return;
    }
    const QMessageBox::StandardButton answer = QMessageBox::question(
        this, t(_loc, "map.delete_title"),
        t(_loc, "map.delete_confirm").arg(QString::fromStdString(path.stem().string())));
    if (answer != QMessageBox::Yes) {
        return;
    }
    const LevelFileOperations ops(_dir);
    HMI_LOG_INFO("Niveaux : suppression de « " + path.stem().string() + " ».");
    reportIfError(this, t(_loc, "map.operation_failed"), ops.remove(path));
    refresh();
}

void LevelBrowserPanel::retranslateUi(const Localization& loc) {
    _loc = &loc;
    _ui->searchField->setPlaceholderText(t(_loc, "map.search"));
    _ui->newButton->setText(t(_loc, "map.new"));
    _ui->renameButton->setText(t(_loc, "map.rename"));
    _ui->duplicateButton->setText(t(_loc, "map.duplicate"));
    _ui->deleteButton->setText(t(_loc, "map.delete"));
}

void LevelBrowserPanel::onActivated(const QModelIndex& index) {
    const QVariant pathData = _proxy->mapToSource(index).data(PATH_ROLE);
    if (pathData.isValid()) {
        emit levelOpenRequested(pathData.toString());
    }
}

}  // namespace hmi
