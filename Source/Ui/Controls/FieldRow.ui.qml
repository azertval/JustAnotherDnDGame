import QtQuick
import Jadg.Ui

/*!
    Un champ de fiche : son etiquette sur une plaquette, sa valeur sur une ligne reglee
    (LOT-87, T3.4 ; maquette 03).

    Pieces `ui/plate/field-tag` (40 px de haut, l'etiquette en capitales) et `ui/control/ruled-line`
    (la ligne d'ecriture sous la valeur). Tant qu'elles ne sont pas livrees : une plaquette de
    `panel` bordee d'or, et un filet d'encre.
*/
Item {
    id: root

    property string label: "Champ"
    property string value: "Valeur"

    /// Largeur reservee a l'etiquette, a 1080p.
    property real tagWidth: 180 * Tokens.uiScale

    implicitWidth: 560 * Tokens.uiScale
    implicitHeight: 48 * Tokens.uiScale

    Item {
        id: tag

        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        width: root.tagWidth
        height: 40 * Tokens.uiScale

        Rectangle {
            anchors.fill: parent
            visible: !tagArt.delivered
            color: Tokens.panel
            border.color: Tokens.accent
            border.width: Tokens.strokeWidth
        }

        NinePatchArt {
            id: tagArt

            anchors.fill: parent
            key: "ui/plate/field-tag"
        }

        Text {
            anchors.fill: parent
            anchors.leftMargin: Tokens.gapMedium
            anchors.rightMargin: Tokens.gapMedium
            verticalAlignment: Text.AlignVCenter
            text: root.label
            color: Tokens.textOnPanel
            font.family: Tokens.titleFamily
            font.pixelSize: Tokens.fontCaption
            font.capitalization: Font.AllUppercase
            elide: Text.ElideRight
        }
    }

    Text {
        id: valueLabel

        anchors.left: tag.right
        anchors.right: parent.right
        anchors.leftMargin: Tokens.gapMedium
        anchors.verticalCenter: parent.verticalCenter
        text: root.value
        color: Tokens.text
        font.family: Tokens.loreFamily
        font.italic: true
        font.pixelSize: Tokens.fontSectionTitle
        elide: Text.ElideRight
    }

    Item {
        anchors.left: valueLabel.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 8 * Tokens.uiScale

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: Tokens.strokeWidth
            visible: !lineArt.delivered
            color: Tokens.border
        }

        NinePatchArt {
            id: lineArt

            anchors.fill: parent
            key: "ui/control/ruled-line"
        }
    }
}
