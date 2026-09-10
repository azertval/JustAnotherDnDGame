import QtQuick
import QtQuick.Layouts
import Jadg.Ui

/*!
    Feuille d'equipe -- FORMULAIRE, cote conception (LOT-86).

    Ce n'est pas la fiche d'un personnage mais celle de son EQUIPE : renommee, blason, quartier
    general, mecenat. La ranger dans la fiche aurait mele deux sujets sur un meme ecran.

    C'est aussi la preuve de ce que le chassis affirme : cet ecran s'ajoute par son formulaire et
    ses libelles, sans qu'aucun des huit autres n'ait ete touche.
*/
RpgScreenFrame {
    id: root

    property alias members: memberList.model
    property alias headquarters: headquartersList.model
    property alias relations: relationsProse.text
    property alias dream: dreamProse.text
    property alias hiddenAgenda: agendaProse.text
    property alias legendaryRewards: rewardsProse.text
    property alias coatOfArmsSource: coatOfArms.source

    property string teamName: "—"
    property string careerPoints: "—"
    property string fame: "—"
    property string prestige: "—"
    property string beneficiary: "—"
    property string style: "—"
    property string specialization: "—"

    title: qsTr("Feuille d'équipe")

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

                SheetBlock {
                    Layout.fillWidth: true
                    title: qsTr("Équipe")
                    SheetLine { Layout.fillWidth: true; label: qsTr("Nom de l'équipe"); value: root.teamName }
                    SheetLine { Layout.fillWidth: true; label: qsTr("Points de carrière"); value: root.careerPoints }
                    SheetLine { Layout.fillWidth: true; label: qsTr("Renommée"); value: root.fame }
                    SheetLine { Layout.fillWidth: true; label: qsTr("Prestige"); value: root.prestige }
                    SheetLine { Layout.fillWidth: true; label: qsTr("Bénéficiaire"); value: root.beneficiary }
                    SheetLine { Layout.fillWidth: true; label: qsTr("Style"); value: root.style }
                    SheetLine { Layout.fillWidth: true; label: qsTr("Spécialisation"); value: root.specialization }
                }

                SheetList {
                    id: memberList
                    Layout.fillWidth: true
                    title: qsTr("Membres")
                    rows: 4
                    model: ListModel {
                        ListElement { label: "Brenna Vaugris"; value: "Bagarreuse 3" }
                        ListElement { label: "Sarre"; value: "Éclaireuse 2" }
                    }
                }

                SheetProse {
                    id: relationsProse
                    Layout.fillWidth: true
                    title: qsTr("Relations")
                    text: "La Guilde vous tolère. Le prévôt de Bourg-la-Rive vous doit une faveur, et ne l'a dit à personne."
                }
            }

            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                spacing: Tokens.spaceLarge

                SheetPortrait {
                    id: coatOfArms
                    Layout.fillWidth: true
                    title: qsTr("Blason")
                    aspectRatio: 1.0
                }

                SheetProse {
                    id: dreamProse
                    Layout.fillWidth: true
                    title: qsTr("Rêve")
                    text: "Racheter la vieille halle et en faire un quartier général."
                }

                SheetProse {
                    id: agendaProse
                    Layout.fillWidth: true
                    title: qsTr("Dessein caché")
                    text: "—"
                }

                SheetProse {
                    id: rewardsProse
                    Layout.fillWidth: true
                    title: qsTr("Récompenses légendaires")
                    text: "—"
                }

                SheetList {
                    id: headquartersList
                    Layout.fillWidth: true
                    title: qsTr("Quartier général")
                    rows: 8
                    model: ListModel {
                        ListElement { label: "Forge"; value: "—" }
                        ListElement { label: "Herboristerie"; value: "—" }
                    }
                }
            }
        }
    ]
}
