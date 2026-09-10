import QtQuick
import QtQuick.Layouts
import Jadg.Ui

/*!
    Un bloc de LISTE : des lignes libelle/valeur, en nombre decide par la donnee (LOT-86).

    `rows` fixe la hauteur reservee, pas le contenu : c'est la maquette qui dit « huit quetes
    visibles », et la liste defile au-dela. Sans cette hauteur reservee, un bloc vide se
    reduirait a rien et la mise en page changerait des que la donnee arrive.
*/
SheetBlock {
    id: root

    property alias model: sheetListView.model
    property int rows: 6

    ListView {
        Layout.fillWidth: true
        Layout.preferredHeight: root.rows * (Tokens.body + Tokens.spaceSmall)
        id: sheetListView
        clip: true
        spacing: Tokens.spaceSmall
        boundsBehavior: Flickable.StopAtBounds
        delegate: Item {
            id: row
            required property string label
            required property string value
            width: ListView.view.width
            implicitHeight: line.implicitHeight
            SheetLine {
                id: line
                anchors.fill: parent
                label: row.label
                value: row.value
            }
        }
    }
}
