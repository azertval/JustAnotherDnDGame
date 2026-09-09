import QtQuick
import QtQuick.Layouts
import Jadg.Ui

/*!
    Un bloc de PROSE : un paragraphe qui se replie sur la largeur disponible (LOT-86).

    `Layout.preferredHeight` suit la hauteur repliee du texte : sans cela, un layout donnerait au
    paragraphe sa hauteur d'UNE ligne et couperait le reste -- sans rien signaler.
*/
SheetBlock {
    id: root

    property alias text: body.text

    Text {
        id: body
        Layout.fillWidth: true
        Layout.preferredHeight: body.implicitHeight
        text: "—"
        color: Tokens.text
        font.family: Tokens.bodyFamily
        font.pixelSize: Tokens.body
        wrapMode: Text.WordWrap
        lineHeight: 1.3
    }
}
