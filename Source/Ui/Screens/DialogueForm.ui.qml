import QtQuick
import QtQuick.Layouts
import Jadg.Ui

/*!
    Dialogue -- FORMULAIRE, cote conception (LOT-86).

    Un dialogue SUSPEND le jeu : l'interlocuteur attend une reponse, il ne la recoit pas en
    courant. C'est une regle de l'ecran, pas du point d'appel -- ouvert depuis le jeu ou depuis une
    touche, il se comporte pareil (EX-IHM-091).
*/
RpgScreenFrame {
    id: root

    property alias portraitSource: portrait.source
    property alias replies: replyList.model
    property alias line: lineProse.text
    property string speakerName: "—"
    property string attitude: "—"

    title: qsTr("Dialogue")

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

                SheetPortrait {
                    id: portrait
                    Layout.fillWidth: true
                    title: qsTr("Interlocuteur")
                    aspectRatio: 0.8
                }

                SheetBlock {
                    Layout.fillWidth: true
                    title: ""
                    SheetLine { Layout.fillWidth: true; label: qsTr("Nom"); value: root.speakerName }
                    SheetLine { Layout.fillWidth: true; label: qsTr("Attitude"); value: root.attitude }
                }
            }

            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                Layout.preferredWidth: 2
                Layout.fillWidth: true
                spacing: Tokens.spaceLarge

                SheetProse {
                    id: lineProse
                    Layout.fillWidth: true
                    title: qsTr("Réplique")
                    text: "Vous arrivez tard, et par la mauvaise route. Ceux qui viennent par là ont d'ordinaire quelque chose à cacher — ou quelqu'un à fuir. Lequel des deux, pour vous ?"
                }

                SheetList {
                    id: replyList
                    Layout.fillWidth: true
                    title: qsTr("Réponses")
                    rows: 4
                    model: ListModel {
                        ListElement { label: "Ni l'un ni l'autre. Je cherche du travail."; value: "" }
                        ListElement { label: "Cela ne vous regarde pas."; value: "" }
                        ListElement { label: "Qui fuit, ici ?"; value: "" }
                    }
                }
            }
        }
    ]
}
