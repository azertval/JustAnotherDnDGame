import QtQuick
import Jadg.Ui

/*!
    Repere pose sur une carte : une region sur le monde, un lieu sur sa region, un quartier sur le
    plan de sa ville (LOT-87 T3.6, refondu aux LOT-94 et LOT-95).

    Piece `ui/icon/poi/<genre>`. Tant qu'elle n'est pas livree, un losange grenat borde d'or ; un
    repere `gateway` -- celui qui ouvre un niveau de zoom de plus -- le porte cercle d'or. Un plan de
    ville numerote ses reperes comme la legende du livre : `number` s'inscrit alors dans le losange.

    Le nom se pose a droite du repere, en capitales sur un bandeau sombre, pour rester lisible sur
    toutes les teintes de la carte : au survol seulement (`active`), ou en permanence
    (`labelAlways`) quand la carte est assez agrandie pour le porter.

    Le point d'ancrage est le CENTRE du repere : placer `x` et `y` a la position moins la moitie de
    `markerSize`. `hovered` et `activated()` remontent le pointeur ; la selection, elle, appartient
    au jumeau.
*/
Item {
    id: root

    property string label: "Region"
    property string kind: "city"

    /// Numero inscrit dans le repere, ou 0 pour n'en porter aucun.
    property int number: 0

    /// Vrai pour le repere survole ou selectionne : il grandit et son nom s'eclaire.
    property bool active: false

    /// Vrai pour un repere qui ouvre un niveau de zoom de plus (une region, une ville a plan).
    property bool gateway: false

    /// Vrai pour garder le nom affiche hors survol.
    property bool labelAlways: false

    readonly property real markerSize: (root.active ? 44 : 36) * Tokens.uiScale
    readonly property bool hovered: markerPointer.containsMouse

    property alias pointer: markerPointer

    signal activated()

    width: root.markerSize
    height: root.markerSize

    // L'anneau d'or d'un repere qui s'ouvre.
    Rectangle {
        anchors.centerIn: parent
        width: root.markerSize * 1.15
        height: root.markerSize * 1.15
        radius: width / 2
        visible: root.gateway
        color: "transparent"
        border.color: root.active ? Tokens.goldLight : Tokens.panelEdge
        border.width: Tokens.strokeWidth
        antialiasing: true
    }

    Rectangle {
        anchors.centerIn: parent
        width: root.markerSize * 0.7
        height: root.markerSize * 0.7
        rotation: 45
        visible: !art.delivered || root.number > 0
        color: root.active ? Tokens.gemLight : Tokens.gem
        border.color: root.active ? Tokens.goldLight : Tokens.panelEdge
        border.width: Tokens.strokeWidth
        antialiasing: true
    }

    FixedArt {
        id: art

        anchors.fill: parent
        visible: root.number <= 0
        key: "ui/icon/poi/" + root.kind
    }

    Text {
        anchors.centerIn: parent
        visible: root.number > 0
        text: root.number
        color: root.active ? Tokens.goldLight : Tokens.textOnPanel
        font.family: Tokens.titleFamily
        font.pixelSize: Tokens.fontCaption
        font.weight: Font.Bold
    }

    Rectangle {
        visible: root.active || root.labelAlways
        anchors.left: parent.right
        anchors.leftMargin: Tokens.gapSmall
        anchors.verticalCenter: parent.verticalCenter
        width: nameLabel.implicitWidth + 2 * Tokens.gapSmall
        height: nameLabel.implicitHeight + Tokens.gapSmall
        color: Qt.rgba(Tokens.panel.r, Tokens.panel.g, Tokens.panel.b, root.active ? 0.85 : 0.6)
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
        onClicked: root.activated()
    }
}
