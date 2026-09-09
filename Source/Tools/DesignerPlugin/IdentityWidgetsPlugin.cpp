// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tools/DesignerPlugin/IdentityWidgetsPlugin.h"

#include <utility>

#include "HMI/Interface/AbilityWheel.h"
#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/MenuEntryButton.h"
#include "HMI/Interface/ParchmentPanel.h"
#include "HMI/Interface/SheetWidgets.h"
#include "HMI/Interface/TitleBanner.h"

namespace jadg {
namespace {

/// Le facteur d'agrandissement des maquettes : elles sont dessinées à 1280 x 720, la taille de la
/// fenêtre de jeu, et le jeu y applique x2 (`.design-mockups/README.md`). Le poser ici est ce qui
/// donne aux planches ouvertes dans Designer les proportions du jeu, et non celles d'un x1 que
/// personne ne voit jamais.
constexpr int DESIGNER_IDENTITY_SCALE = 2;

/// Nom d'objet par défaut d'un widget déposé : la classe sans son espace de noms, initiale en
/// minuscule. Designer y ajoute un numéro si le nom est déjà pris.
QString defaultObjectName(const QString& qualifiedClass) {
    QString bare = qualifiedClass.section(QStringLiteral("::"), -1);
    if (!bare.isEmpty()) {
        bare[0] = bare[0].toLower();
    }
    return bare;
}

}  // namespace

IdentityWidgetEntry::IdentityWidgetEntry(QString name, QString header, QString tooltip,
                                         bool container, QSize initialSize, Factory factory)
    : _name(std::move(name)),
      _header(std::move(header)),
      _tooltip(std::move(tooltip)),
      _container(container),
      _initialSize(initialSize),
      _factory(std::move(factory)) {}

void IdentityWidgetEntry::initialize(QDesignerFormEditorInterface* /*core*/) {
    _initialized = true;
}

QString IdentityWidgetEntry::domXml() const {
    // Une taille de départ, et non le carré de 100 x 100 que Designer donne par défaut : ces
    // widgets se peignent d'après leur géométrie, et plusieurs (la roue, l'écu) sont illisibles
    // en dessous d'un certain calibre -- on les croirait cassés au moment même où on les découvre.
    return QStringLiteral(
               "<ui language=\"c++\">\n"
               "  <widget class=\"%1\" name=\"%2\">\n"
               "    <property name=\"geometry\">\n"
               "      <rect><x>0</x><y>0</y><width>%3</width><height>%4</height></rect>\n"
               "    </property>\n"
               "  </widget>\n"
               "</ui>\n")
        .arg(_name, defaultObjectName(_name))
        .arg(_initialSize.width())
        .arg(_initialSize.height());
}

IdentityWidgetsPlugin::IdentityWidgetsPlugin(QObject* parent) : QObject(parent) {
    // Avant toute instance : les peintres lisent ce facteur dans leurs `sizeHint` comme dans leur
    // `paintEvent`, et le poser après en aurait laissé quelques-uns calibrés pour x1.
    hmi::setIdentityScale(DESIGNER_IDENTITY_SCALE);

    const auto add = [this](const char* name, const char* header, const char* tooltip,
                            bool container, QSize size, IdentityWidgetEntry::Factory factory) {
        _widgets.append(new IdentityWidgetEntry(QString::fromUtf8(name), QString::fromUtf8(header),
                                                QString::fromUtf8(tooltip), container, size,
                                                std::move(factory)));
    };

    add("hmi::ParchmentPanel", "HMI/Interface/ParchmentPanel.h",
        "Encadrement de parchemin. Conteneur : on y dépose une disposition.", true, {320, 220},
        [](QWidget* parent) -> QWidget* { return new hmi::ParchmentPanel(parent); });

    add("hmi::TitleBanner", "HMI/Interface/TitleBanner.h",
        "Titre d'écran posé sur son bandeau de grenat.", false, {240, 48},
        [](QWidget* parent) -> QWidget* { return new hmi::TitleBanner(parent); });

    add("hmi::MenuEntryButton", "HMI/Interface/MenuEntryButton.h",
        "Entrée de menu, portant la marque de focus.", false, {200, 40},
        [](QWidget* parent) -> QWidget* { return new hmi::MenuEntryButton(parent); });

    add("hmi::AbilityWheel", "HMI/Interface/AbilityWheel.h",
        "La roue des six caractéristiques, portrait et nom dans son ouverture.", false, {420, 420},
        [](QWidget* parent) -> QWidget* { return new hmi::AbilityWheel(parent); });

    add("hmi::SheetTabBar", "HMI/Interface/SheetWidgets.h",
        "Barre des cinq sections de la fiche. Interne à la planche, pas un cycle de navigation.",
        false, {480, 40}, [](QWidget* parent) -> QWidget* { return new hmi::SheetTabBar(parent); });

    add("hmi::SheetGauge", "HMI/Interface/SheetWidgets.h",
        "Jauge de la fiche. Son encre suit la propriété « tone ».", false, {220, 28},
        [](QWidget* parent) -> QWidget* { return new hmi::SheetGauge(parent); });

    add("hmi::ShieldValue", "HMI/Interface/SheetWidgets.h",
        "La classe d'armure, sur son écu : la seule silhouette non ronde de la planche.", false,
        {96, 110}, [](QWidget* parent) -> QWidget* { return new hmi::ShieldValue(parent); });

    add("hmi::StatMedallion", "HMI/Interface/SheetWidgets.h",
        "Médaillon d'une valeur isolée (maîtrise, inspiration, initiative).", false, {96, 96},
        [](QWidget* parent) -> QWidget* { return new hmi::StatMedallion(parent); });

    add("hmi::PipRow", "HMI/Interface/SheetWidgets.h", "Rangée de pastilles.", false, {160, 24},
        [](QWidget* parent) -> QWidget* { return new hmi::PipRow(parent); });

    add("hmi::WaxSeal", "HMI/Interface/SheetWidgets.h", "Cachet de cire.", false, {72, 72},
        [](QWidget* parent) -> QWidget* { return new hmi::WaxSeal(parent); });

    add("hmi::RoundShield", "HMI/Interface/SheetWidgets.h", "Rondache centrale de l'équipement.",
        false, {120, 120},
        [](QWidget* parent) -> QWidget* { return new hmi::RoundShield(parent); });
}

}  // namespace jadg
