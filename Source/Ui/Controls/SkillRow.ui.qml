import QtQuick
import Jadg.Ui

/*!
    Une ligne de competence : pastille de maitrise, icone, nom, modificateur en case
    (LOT-87, T3.4 ; maquette 03).

    Pieces `ui/control/pip` (etats `empty`, `filled`), `ui/icon/skill/<id>` et
    `ui/control/value-box`. La maitrise se voit a la pastille pleine, et non a une teinte
    (EX-IHM-071 vaut pour toute information d'etat).
*/
Item {
    id: root

    property string skillId: ""
    property string label: "Competence"
    property string value: "+0"
    property bool proficient: false

    implicitWidth: 480 * Tokens.uiScale
    implicitHeight: 44 * Tokens.uiScale

    Item {
        id: pip

        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        width: 24 * Tokens.uiScale
        height: 24 * Tokens.uiScale

        Rectangle {
            anchors.fill: parent
            visible: !pipArt.delivered
            radius: width / 2
            color: root.proficient ? Tokens.text : "transparent"
            border.color: Tokens.text
            border.width: Tokens.strokeWidth
        }

        FixedArt {
            id: pipArt

            anchors.fill: parent
            key: root.proficient ? "ui/control/pip/filled" : "ui/control/pip/empty"
        }
    }

    Rectangle {
        id: iconDisc

        anchors.left: pip.right
        anchors.leftMargin: Tokens.gapMedium
        anchors.verticalCenter: parent.verticalCenter
        width: 36 * Tokens.uiScale
        height: 36 * Tokens.uiScale
        radius: width / 2
        color: "transparent"
        border.color: Tokens.border
        border.width: Tokens.strokeWidth

        FixedArt {
            anchors.fill: parent
            anchors.margins: 4 * Tokens.uiScale
            key: "ui/icon/skill/" + root.skillId
        }
    }

    Text {
        anchors.left: iconDisc.right
        anchors.right: valueBox.left
        anchors.leftMargin: Tokens.gapMedium
        anchors.rightMargin: Tokens.gapMedium
        anchors.verticalCenter: parent.verticalCenter
        text: root.label
        color: Tokens.text
        font.family: Tokens.bodyFamily
        font.pixelSize: Tokens.fontBody
        elide: Text.ElideRight
    }

    Item {
        id: valueBox

        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        width: 64 * Tokens.uiScale
        height: 36 * Tokens.uiScale

        Rectangle {
            anchors.fill: parent
            visible: !boxArt.delivered
            color: Tokens.surfaceAlt
            border.color: Tokens.border
            border.width: Tokens.strokeWidth
        }

        NinePatchArt {
            id: boxArt

            anchors.fill: parent
            key: "ui/control/value-box"
        }

        Text {
            anchors.centerIn: parent
            text: root.value
            color: Tokens.text
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontBody
        }
    }
}
