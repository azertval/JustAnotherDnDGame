import QtQuick
import QtQuick.Layouts
import Jadg.Ui

/*!
    Une BARRE D'ACTIONS : les gestes disponibles a cet instant (LOT-86).

    Chaque action porte son RACCOURCI a cote de son nom. Un combat doit se jouer entierement au
    clavier et entierement a la manette (feuille de route, LOT-24) : une action dont on ne connait
    que l'icone n'est atteignable qu'a la souris, et c'est le critere que ce projet dit lui-meme
    « souvent oublie ».
*/
SheetBlock {
    id: root

    property alias model: actions.model
    property int columns: 6

    RowLayout {
        Layout.fillWidth: true
        spacing: Tokens.spaceSmall

        Repeater {
            id: actions
            delegate: Rectangle {
                id: actionCell
                required property string label
                required property string value
                Layout.fillWidth: true
                Layout.preferredHeight: Tokens.sectionTitle + Tokens.spaceLarge
                color: Tokens.surface
                border.width: Tokens.frameThickness
                border.color: Tokens.frameOrnament

                ColumnLayout {
                    anchors.centerIn: parent
                    width: parent.width - Tokens.spaceSmall * 2
                    spacing: 0

                    Text {
                        Layout.fillWidth: true
                        text: actionCell.label
                        color: Tokens.text
                        font.family: Tokens.bodyFamily
                        font.pixelSize: Tokens.caption
                        horizontalAlignment: Text.AlignHCenter
                        elide: Text.ElideRight
                    }
                    Text {
                        Layout.fillWidth: true
                        text: actionCell.value
                        color: Tokens.textMuted
                        font.family: Tokens.bodyFamily
                        font.pixelSize: Tokens.caption
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }
    }
}
