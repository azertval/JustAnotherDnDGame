import QtQuick
import QtQuick.Layouts
import Jadg.Ui

/*!
    Tableau de la Guilde -- FORMULAIRE, cote conception (LOT-86).

    Le contrat choisi s'ouvre a droite, avec ses termes sous lui. Commanditaire, rang et
    recompense sont des CHAMPS et non de la prose : ce sont les trois valeurs qu'on compare d'un
    contrat a l'autre, et qu'on doit pouvoir lire sans relire.
*/
RpgScreenFrame {
    id: root

    property alias contracts: contractList.model
    property alias contract: contractProse.text
    property string giver: "—"
    property string rank: "—"
    property string reward: "—"

    title: qsTr("Tableau de la Guilde")

    // Le contenu s'affecte au slot du châssis. `content` n'est PAS la propriété par défaut,
    // et ne peut pas l'être : le châssis a ses propres enfants (le double cadre, le titre, le
    // pied), qui y entreraient aussi et s'imbriqueraient dans eux-mêmes.
    content: [
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Tokens.spaceExtraLarge

            SheetList {
                id: contractList
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                title: qsTr("Contrats")
                rows: 6
                model: ListModel {
                    ListElement { label: "Escorte jusqu'à Val-Morne"; value: "Rang I" }
                    ListElement { label: "Les caves de Bourg-la-Rive"; value: "Rang I" }
                    ListElement { label: "La bête du Bois de Cendre"; value: "Rang II" }
                }
            }

            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                spacing: Tokens.spaceLarge

                SheetProse {
                    id: contractProse
                    Layout.fillWidth: true
                    title: qsTr("Contrat")
                    text: "Accompagner le convoi de sel jusqu'à Val-Morne et en revenir. La route passe par le col, qui n'est plus sûr depuis l'automne. La Guilde ne garantit pas le retour."
                }

                SheetBlock {
                    Layout.fillWidth: true
                    title: ""
                    SheetLine { Layout.fillWidth: true; label: qsTr("Commanditaire"); value: root.giver }
                    SheetLine { Layout.fillWidth: true; label: qsTr("Rang"); value: root.rank }
                    SheetLine { Layout.fillWidth: true; label: qsTr("Récompense"); value: root.reward }
                }
            }
        }
    ]
}
