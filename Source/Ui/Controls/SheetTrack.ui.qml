// Lie les delegues a la portee de CE fichier : sans ce pragma, `root.activeIndex` lu depuis un
// delegue passe par un mecanisme que QML ne garantit plus, et que qmllint signale.
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import Jadg.Ui

/*!
    Une PISTE : des cases alignees, dont une seule est active (LOT-86).

    Sert l'ordre d'initiative du combat. La case active se distingue par sa BORDURE et son fond,
    jamais par la seule couleur du texte : le focus doit rester lisible pour qui distingue mal les
    teintes, et c'est la meme raison qui fait exister `FocusMarker` cote widgets (EX-IHM-040).
*/
SheetBlock {
    id: root

    property alias model: cells.model
    property int columns: 6
    property int activeIndex: 0

    RowLayout {
        Layout.fillWidth: true
        spacing: Tokens.spaceSmall

        Repeater {
            id: cells
            delegate: Rectangle {
                id: cell
                required property int index
                required property string label
                Layout.fillWidth: true
                Layout.preferredHeight: Tokens.sectionTitle + Tokens.spaceMedium
                color: cell.index === root.activeIndex ? Tokens.surfaceAlt : Tokens.surface
                border.width: Tokens.frameThickness
                border.color: cell.index === root.activeIndex ? Tokens.accent : Tokens.frameOrnament

                Text {
                    anchors.centerIn: parent
                    width: parent.width - Tokens.spaceSmall * 2
                    text: cell.label
                    color: Tokens.text
                    font.family: Tokens.bodyFamily
                    font.pixelSize: Tokens.caption
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideRight
                }
            }
        }
    }
}
