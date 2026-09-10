import QtQuick
import QtQuick.Layouts
import Jadg.Ui

/*!
    Marchand -- FORMULAIRE, cote conception (LOT-86).

    Deux listes face a face : ce qu'il vend, ce que l'on porte. La bourse est sous la premiere,
    parce que c'est elle qui decide de ce qu'on peut acheter -- pas de ce qu'on peut vendre.
*/
RpgScreenFrame {
    id: root

    property alias goods: goodsList.model
    property alias bag: bagList.model
    property string gold: "—"

    title: qsTr("Marchand")

    // Le contenu s'affecte au slot du châssis. `content` n'est PAS la propriété par défaut,
    // et ne peut pas l'être : le châssis a ses propres enfants (le double cadre, le titre, le
    // pied), qui y entreraient aussi et s'imbriqueraient dans eux-mêmes.
    content: [
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Tokens.spaceExtraLarge

            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                spacing: Tokens.spaceLarge

                SheetList {
                    id: goodsList
                    Layout.fillWidth: true
                    title: qsTr("Marchandises")
                    rows: 8
                    model: ListModel {
                        ListElement { label: "Corde de chanvre, 15 m"; value: "1 po" }
                        ListElement { label: "Rations de voyage"; value: "5 pa" }
                        ListElement { label: "Lanterne à capote"; value: "5 po" }
                        ListElement { label: "Cotte de mailles"; value: "75 po" }
                    }
                }

                SheetBlock {
                    Layout.fillWidth: true
                    title: qsTr("Bourse")
                    SheetLine { Layout.fillWidth: true; label: qsTr("Pièces d'or"); value: root.gold }
                }
            }

            SheetList {
                id: bagList
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                title: qsTr("Votre sac")
                rows: 8
                model: ListModel {
                    ListElement { label: "Torche"; value: "×4" }
                    ListElement { label: "Pied-de-biche"; value: "×1" }
                    ListElement { label: "Potion de soins"; value: "×2" }
                }
            }
        }
    ]
}
