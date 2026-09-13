import QtQuick
import Jadg.Ui

/*!
    Medaillon de caracteristique ou de statistique derivee (LOT-87, T2.7).

    L'anneau est une piece du cahier -- `ui/medallion/ability` (160 px a 1080p) autour du portrait
    de la fiche, `ui/medallion/derived-stat` (120 px) sous la silhouette de l'inventaire. L'abreviation,
    la valeur et le modificateur sont poses par le jeu : l'anneau ne porte aucun chiffre.
*/
Item {
    id: root

    /// `ability` (les six caracteristiques) ou `derived` (classe d'armure, initiative, vitesse...).
    property string kind: "ability"

    /// Abreviation (`FOR`), valeur (`16`) et modificateur (`+3`, vide pour une statistique derivee).
    property string label: "FOR"
    property string value: "10"
    property string modifier: ""

    readonly property bool ability: root.kind !== "derived"
    readonly property real size: (root.ability ? 160 : 120) * Tokens.uiScale

    implicitWidth: root.size
    implicitHeight: root.size

    Rectangle {
        anchors.fill: parent
        visible: !art.delivered
        radius: width / 2
        color: Tokens.surfaceAlt
        border.color: Tokens.accent
        border.width: 3 * Tokens.strokeWidth

        Rectangle {
            anchors.fill: parent
            anchors.margins: 4 * Tokens.strokeWidth
            radius: width / 2
            color: "transparent"
            border.color: Tokens.border
            border.width: Tokens.strokeWidth
        }
    }

    FixedArt {
        id: art

        anchors.fill: parent
        key: root.ability ? "ui/medallion/ability" : "ui/medallion/derived-stat"
    }

    Column {
        anchors.centerIn: parent

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.label
            color: Tokens.textMuted
            font.family: Tokens.titleFamily
            font.pixelSize: Tokens.fontCaption
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.value
            color: Tokens.text
            font.family: Tokens.titleFamily
            font.pixelSize: root.ability ? Tokens.fontScreenTitle : Tokens.fontSectionTitle
            font.weight: Font.DemiBold
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: root.modifier.length > 0
            text: root.modifier
            color: Tokens.text
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontBody
        }
    }
}
