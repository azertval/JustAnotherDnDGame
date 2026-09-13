import QtQuick
import Jadg.Ui

/*!
    Plaque de titre d'un ecran : grenat sur les panneaux sombres, noire sur le parchemin
    (LOT-87, T2.7).

    Pieces `ui/plate/title-garnet` (120 px de haut a 1080p) et `ui/plate/title-black` (96 px). La
    largeur suit le titre : les extremites ornees tiennent dans les marges du cahier, et le milieu
    se repete sous le texte. Le titre est pose par le jeu, jamais peint dans l'image -- le cahier
    interdit les lettres a toute piece hors le logotype.
*/
Item {
    id: root

    property string text: "Titre"

    /// `garnet` (options, credits, carte, compagnie) ou `black` (fiche, inventaire, sorts).
    property string material: "garnet"

    readonly property bool garnet: root.material !== "black"
    readonly property string key: root.garnet ? "ui/plate/title-garnet" : "ui/plate/title-black"
    readonly property real endWidth: (root.garnet ? 160 : 120) * Tokens.uiScale

    implicitHeight: (root.garnet ? 120 : 96) * Tokens.uiScale
    implicitWidth: titleLabel.implicitWidth + 2 * root.endWidth

    Rectangle {
        anchors.fill: parent
        anchors.topMargin: parent.height * 0.18
        anchors.bottomMargin: parent.height * 0.18
        visible: !art.delivered
        color: root.garnet ? Tokens.gem : Tokens.panel
        border.color: root.garnet ? Tokens.panelEdge : Tokens.accent
        border.width: Tokens.strokeWidth
    }

    NinePatchArt {
        id: art

        anchors.fill: parent
        key: root.key
    }

    Text {
        id: titleLabel

        anchors.centerIn: parent
        text: root.text
        color: Tokens.textOnPanel
        font.family: Tokens.titleFamily
        font.pixelSize: Tokens.fontScreenTitle
        font.weight: Font.DemiBold
    }
}
