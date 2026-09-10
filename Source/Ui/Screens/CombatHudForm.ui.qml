import QtQuick
import QtQuick.Layouts
import Jadg.Ui

/*!
    ATH de combat -- FORMULAIRE, cote conception (LOT-86).

    Le seul ecran qui ne s'ouvre PAS par-dessus le jeu : il EST le jeu pendant un combat. Il ne
    suspend donc rien, et le tour par tour decidera de son rythme.

    Chaque action porte son RACCOURCI : un combat doit se jouer entierement au clavier et
    entierement a la manette, et c'est le critere que la feuille de route dit elle-meme
    « souvent oublie ».
*/
RpgScreenFrame {
    id: root

    property alias initiative: initiativeTrack.model
    property alias actions: actionBar.model
    property alias activeIndex: initiativeTrack.activeIndex
    property string targetName: "—"
    property string targetHitPoints: "—"
    property string targetArmorClass: "—"
    property string targetConditions: "—"

    title: qsTr("Combat")

    // Le contenu s'affecte au slot du châssis. `content` n'est PAS la propriété par défaut,
    // et ne peut pas l'être : le châssis a ses propres enfants (le double cadre, le titre, le
    // pied), qui y entreraient aussi et s'imbriqueraient dans eux-mêmes.
    content: [
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Tokens.spaceLarge

            SheetTrack {
                id: initiativeTrack
                Layout.fillWidth: true
                title: qsTr("Ordre d'initiative")
                columns: 6
                activeIndex: 0
                model: ListModel {
                    ListElement { label: "Brenna" }
                    ListElement { label: "Gobelin" }
                    ListElement { label: "Gobelin" }
                    ListElement { label: "Sarre" }
                    ListElement { label: "Ourse" }
                }
            }

            SheetBlock {
                Layout.fillWidth: true
                title: qsTr("Cible")
                SheetLine { Layout.fillWidth: true; label: qsTr("Nom"); value: root.targetName }
                SheetLine { Layout.fillWidth: true; label: qsTr("Points de vie"); value: root.targetHitPoints }
                SheetLine { Layout.fillWidth: true; label: qsTr("Classe d'armure"); value: root.targetArmorClass }
                SheetLine { Layout.fillWidth: true; label: qsTr("États"); value: root.targetConditions }
            }

            Item { Layout.fillHeight: true }

            SheetActionBar {
                id: actionBar
                Layout.fillWidth: true
                title: qsTr("Actions")
                columns: 6
                model: ListModel {
                    ListElement { label: "Attaquer"; value: "A" }
                    ListElement { label: "Se déplacer"; value: "D" }
                    ListElement { label: "Sort"; value: "S" }
                    ListElement { label: "Objet"; value: "O" }
                    ListElement { label: "Se cacher"; value: "C" }
                    ListElement { label: "Passer"; value: "P" }
                }
            }
        }
    ]
}
