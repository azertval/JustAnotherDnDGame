import QtQuick
import QtQuick.Layouts
import Jadg.Ui

/*!
    Marchand -- FORMULAIRE, cote conception (LOT-86, restyle LOT-87 T3.9).

    Pas de maquette : les briques et les jetons de la charte v2, sur la structure v1 -- les
    marchandises a gauche, la bourse au centre, le sac du personnage a droite. Parchemin : un
    marchand se consulte a l'arret, comme un inventaire.

    Les proprietes portent des VALEURS D'EXEMPLE ; le jumeau les remplace par l'ancre `PendingData`,
    en attendant le lot du commerce (`LOT-26`).
*/
ScreenPage {
    id: root

    property var goods: exampleRows
    property var bag: exampleRows
    property string gold: "—"

    readonly property ListModel exampleRows: ListModel {
        ListElement { rowId: "a"; label: "Corde de chanvre (15 m)"; value: "1 po" }
        ListElement { rowId: "b"; label: "Torche"; value: "1 pc" }
    }

    title: qsTr("Marchand")
    material: "parchment"

    RowLayout {
        anchors.fill: parent
        spacing: Tokens.gapLarge

        LedgerList {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 3
            title: qsTr("Marchandises")
            rows: root.goods
        }

        PanelFrame {
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
            Layout.preferredWidth: 2
            Layout.preferredHeight: 200 * Tokens.uiScale
            material: "parchment"
            subpanel: true

            Column {
                anchors.fill: parent
                spacing: Tokens.gapMedium

                SectionBanner {
                    width: parent.width
                    text: qsTr("Bourse")
                }

                FieldRow {
                    width: parent.width
                    label: qsTr("Pièces d'or")
                    value: root.gold
                }
            }
        }

        LedgerList {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 3
            title: qsTr("Votre sac")
            rows: root.bag
        }
    }
}
