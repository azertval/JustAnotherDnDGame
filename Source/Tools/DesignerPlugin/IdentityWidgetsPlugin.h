// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QIcon>
#include <QList>
#include <QObject>
#include <QSize>
#include <QString>
#include <QWidget>
#include <functional>

#include <QtUiPlugin/QDesignerCustomWidgetCollectionInterface>
#include <QtUiPlugin/QDesignerCustomWidgetInterface>

/**
 * @file Tools/DesignerPlugin/IdentityWidgetsPlugin.h
 * @brief Les widgets promus des `.ui` d'identité, rendus dans Qt Designer (`LOT-85`,
 *        `EX-IHM-006`).
 *
 * ## Ce que ce plugin rend possible
 *
 * Qt Designer ne compile pas le C++ : d'un widget promu il ne connaît que la classe de base, et
 * dessine donc un rectangle gris là où le jeu peint un parchemin. Une maquette qu'on ne voit pas
 * ne s'édite pas — c'est ce qui privait `RpgRendering::DesignerPlate` de son sens.
 *
 * Chargé dans le processus de Designer, ce plugin lui donne de vraies instances. Elles peignent
 * exactement ce que le jeu peint, et pour une raison précise : `hmi::identityTokens()` est une
 * table **pure** (`DesignTokens.cpp`), sans état d'application. Aucun de ces peintres n'a besoin
 * du moteur, de la fenêtre, ni du catalogue de traduction pour se dessiner.
 *
 * ## Ce qui n'est pas ici
 *
 * `EquipmentSlotRow` prend le nom de son emplacement à la construction : c'est une rangée
 * **engendrée par une donnée**, qui reste bâtie en code (`EX-IHM-006`). L'exposer inviterait à
 * écrire à la main ce que la table d'équipement doit produire.
 *
 * Les écrans eux-mêmes (`RpgCharacterSheetPlate`, `RpgScreenFrame`) n'y sont pas non plus : ils
 * prennent un `RpgScreenDescriptor` et connaissent la table des écrans. Ce sont les formulaires
 * que Designer ouvre, pas des pièces qu'on y dépose.
 */
namespace jadg {

/**
 * @brief Une entrée du catalogue : ce que Designer doit savoir d'un widget pour l'offrir.
 *
 * Une seule classe pour les onze widgets, paramétrée par une fabrique, plutôt qu'onze classes
 * jumelles : ce qui les distingue tient en quatre champs, et onze copies d'un même corps se
 * seraient désynchronisées au premier ajout.
 */
class IdentityWidgetEntry : public QObject, public QDesignerCustomWidgetInterface {
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    using Factory = std::function<QWidget*(QWidget*)>;

    IdentityWidgetEntry(QString name, QString header, QString tooltip, bool container,
                        QSize initialSize, Factory factory);

    [[nodiscard]] QString name() const override {
        return _name;
    }
    /// Le groupe de la boîte à outils de Designer. Le même pour tous : ces widgets ne se
    /// comprennent qu'ensemble, et les éparpiller les rendrait introuvables.
    [[nodiscard]] QString group() const override {
        return QStringLiteral("JADG — Identité");
    }
    [[nodiscard]] QString toolTip() const override {
        return _tooltip;
    }
    [[nodiscard]] QString whatsThis() const override {
        return _tooltip;
    }
    /// Le chemin écrit dans le `<header>` du `.ui`, **mot pour mot** : c'est lui que `uic`
    /// recopiera dans le `ui_*.h`, et un chemin approché ne se verrait qu'à la compilation.
    [[nodiscard]] QString includeFile() const override {
        return _header;
    }
    [[nodiscard]] bool isContainer() const override {
        return _container;
    }
    [[nodiscard]] QIcon icon() const override {
        return {};
    }
    [[nodiscard]] QWidget* createWidget(QWidget* parent) override {
        return _factory(parent);
    }
    [[nodiscard]] bool isInitialized() const override {
        return _initialized;
    }
    void initialize(QDesignerFormEditorInterface* core) override;
    [[nodiscard]] QString domXml() const override;

private:
    QString _name;
    QString _header;
    QString _tooltip;
    bool _container = false;
    QSize _initialSize;
    Factory _factory;
    bool _initialized = false;
};

/// @brief Le catalogue que Designer charge : les widgets promus par les `.ui` d'identité.
class IdentityWidgetsPlugin : public QObject, public QDesignerCustomWidgetCollectionInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.Designer.CustomWidgetCollection")
    Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)

public:
    explicit IdentityWidgetsPlugin(QObject* parent = nullptr);

    [[nodiscard]] QList<QDesignerCustomWidgetInterface*> customWidgets() const override {
        return _widgets;
    }

private:
    QList<QDesignerCustomWidgetInterface*> _widgets;
};

}  // namespace jadg
