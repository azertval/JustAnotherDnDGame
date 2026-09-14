import QtQuick
import Jadg.Ui

/*!
    Repere d'une region sur la carte du monde : icone de point d'interet et nom (LOT-87, T3.6).

    Piece `ui/icon/poi/<genre>` (`city` pour une region). Tant qu'elle n'est pas livree, un losange
    grenat borde d'or. Au survol, le nom se pose a droite du repere, en capitales sur un bandeau
    sombre, pour rester lisible sur toutes les teintes de la carte.

    Le point d'ancrage est le CENTRE du repere : placer `x` et `y` a la position moins la moitie
    de `markerSize`. La zone du pointeur (`pointer`) est branchee par le jumeau.
*/
Item {
    id: root

    property string label: "Region"
    property string kind: "city"

    /// Vrai pour le repere survole ou selectionne : il grandit et son nom s'eclaire.
    property bool active: false

    readonly property real markerSize: (root.active ? 44 : 36) * Tokens.uiScale

    property alias pointer: markerPointer

    width: root.markerSize
    height: root.markerSize

    Rectangle {
        anchors.centerIn: parent
        width: root.markerSize * 0.7
        height: root.markerSize * 0.7
        rotation: 45
        visible: !art.delivered
        color: root.active ? Tokens.gemLight : Tokens.gem
        border.color: root.active ? Tokens.goldLight : Tokens.panelEdge
        border.width: Tokens.strokeWidth
        antialiasing: true
    }

    FixedArt {
        id: art

        anchors.fill: parent
        key: "ui/icon/poi/" + root.kind
    }

    // Le nom ne s'affiche qu'au survol : la carte imprime deja celui de chaque region, et le doubler
    // en permanence le recouvrait.
    Rectangle {
        visible: root.active
        anchors.left: parent.right
        anchors.leftMargin: Tokens.gapSmall
        anchors.verticalCenter: parent.verticalCenter
        width: nameLabel.implicitWidth + 2 * Tokens.gapSmall
        height: nameLabel.implicitHeight + Tokens.gapSmall
        color: Qt.rgba(Tokens.panel.r, Tokens.panel.g, Tokens.panel.b, 0.75)
        border.color: root.active ? Tokens.goldLight : "transparent"
        border.width: Tokens.strokeWidth

        Text {
            id: nameLabel

            anchors.centerIn: parent
            text: root.label
            color: root.active ? Tokens.goldLight : Tokens.textOnPanel
            font.family: Tokens.titleFamily
            font.pixelSize: Tokens.fontCaption
            font.weight: Font.DemiBold
        }
    }

    MouseArea {
        id: markerPointer

        anchors.fill: parent
        anchors.margins: -8 * Tokens.uiScale
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
    }
}
