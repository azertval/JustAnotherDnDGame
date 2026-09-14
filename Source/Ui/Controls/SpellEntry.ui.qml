import QtQuick
import Jadg.Ui

/*!
    Une attaque ou un sortilege : icone dans son anneau, nom, et ses caracteristiques ligne a ligne
    (LOT-87, T3.8 ; maquette 10).

    L'icone est la piece `iconKey` posee dans `ui/medallion/icon-ring` ; tant qu'aucune n'est livree,
    un anneau d'aplat tient la place. `details` s'ecrit tel quel, une caracteristique par ligne
    (« Type : Tranchant », « Portee : CaC »…) : c'est le lot des sorts qui les formera.
*/
Item {
    id: root

    property string name: "Attaque"
    property string details: "—"
    property string iconKey: ""

    implicitWidth: 460 * Tokens.uiScale
    implicitHeight: Math.max(96 * Tokens.uiScale, textColumn.implicitHeight + Tokens.gapMedium)

    Item {
        id: icon

        anchors.left: parent.left
        anchors.top: parent.top
        width: 80 * Tokens.uiScale
        height: 80 * Tokens.uiScale

        Rectangle {
            anchors.fill: parent
            visible: !ringArt.delivered
            radius: width / 2
            color: Tokens.panel
            border.color: Tokens.accent
            border.width: 2 * Tokens.strokeWidth
        }

        FixedArt {
            id: ringArt

            anchors.fill: parent
            key: "ui/medallion/icon-ring"
        }

        FixedArt {
            anchors.fill: parent
            anchors.margins: 14 * Tokens.uiScale
            key: root.iconKey
        }
    }

    Column {
        id: textColumn

        anchors.left: icon.right
        anchors.right: parent.right
        anchors.leftMargin: Tokens.gapMedium
        anchors.top: parent.top

        Text {
            width: parent.width
            text: root.name
            color: Tokens.text
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontBody
            font.weight: Font.DemiBold
            elide: Text.ElideRight
        }

        Text {
            width: parent.width
            text: root.details
            color: Tokens.textMuted
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontCaption
            wrapMode: Text.WordWrap
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: Tokens.strokeWidth
        color: Tokens.border
        opacity: 0.5
    }
}
