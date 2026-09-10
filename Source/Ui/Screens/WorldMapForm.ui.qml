import QtQuick
import QtQuick.Layouts
import Jadg.Ui

/*!
    Carte du monde -- FORMULAIRE, cote conception (LOT-86).

    Treize regions a l'atlas (LOT-37) ; la liste en montre six et defile. C'est le seul ecran, avec
    l'ATH de combat, qui se consulte EN MARCHANT : on l'ouvre pour savoir ou l'on va sans s'arreter,
    et le suspendre en ferait un ecran de bilan.
*/
RpgScreenFrame {
    id: root

    property alias regions: regionList.model
    property alias mapSource: mapPicture.source
    property string region: "—"
    property string placeType: "—"
    property string danger: "—"

    title: qsTr("Carte du monde")

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
                Layout.preferredWidth: 1
                Layout.fillWidth: true
                spacing: Tokens.spaceLarge

                SheetList {
                    id: regionList
                    Layout.fillWidth: true
                    title: qsTr("Régions")
                    rows: 6
                    model: ListModel {
                        ListElement { label: "Vallée de Tanares"; value: "" }
                        ListElement { label: "Marches du Nord"; value: "" }
                        ListElement { label: "Bois de Cendre"; value: "" }
                        ListElement { label: "Côte Brisée"; value: "" }
                    }
                }

                SheetBlock {
                    Layout.fillWidth: true
                    title: qsTr("Lieu")
                    SheetLine { Layout.fillWidth: true; label: qsTr("Région"); value: root.region }
                    SheetLine { Layout.fillWidth: true; label: qsTr("Type de lieu"); value: root.placeType }
                    SheetLine { Layout.fillWidth: true; label: qsTr("Danger"); value: root.danger }
                }
            }

            SheetPortrait {
                id: mapPicture
                Layout.alignment: Qt.AlignTop
                Layout.preferredWidth: 2
                Layout.fillWidth: true
                title: qsTr("Carte")
                aspectRatio: 1.4
            }
        }
    ]
}
