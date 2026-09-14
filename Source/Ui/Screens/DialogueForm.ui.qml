import QtQuick
import QtQuick.Layouts
import Jadg.Ui

/*!
    Dialogue -- FORMULAIRE, cote conception (LOT-86, restyle LOT-87 T3.9).

    Pas de maquette : les briques et les jetons de la charte v2, sur la structure v1 -- l'interlocuteur
    a gauche (portrait, nom, attitude), sa replique et les reponses possibles a droite. Parchemin :
    un dialogue suspend le jeu, et se lit comme une page.

    Les proprietes portent des VALEURS D'EXEMPLE ; le jumeau les lie a `DialogueModel` (LOT-15).
    Cliquer une reponse emet `replyChosen(rowId)` ; le jet que la derniere reponse a joue s'ecrit
    au-dessus de la replique (`checkOutcome`), vide sinon.
*/
ScreenPage {
    id: root

    property url portraitSource: ""
    property var replies: exampleReplies
    property string line: "Vous arrivez tard, et par la mauvaise route. Ceux qui viennent par là ont d'ordinaire quelque chose à cacher — ou quelqu'un à fuir. Lequel des deux, pour vous ?"
    property string speakerName: "—"
    property string attitude: "—"
    property string checkOutcome: ""

    signal replyChosen(string rowId)

    readonly property ListModel exampleReplies: ListModel {
        ListElement { rowId: "a"; label: "Ni l'un ni l'autre. Je cherche du travail."; value: "" }
        ListElement { rowId: "b"; label: "Cela ne vous regarde pas."; value: "" }
        ListElement { rowId: "c"; label: "Qui fuit, ici ?"; value: "" }
    }

    title: qsTr("Dialogue")
    material: "parchment"

    RowLayout {
        anchors.fill: parent
        spacing: Tokens.gapLarge

        ColumnLayout {
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            spacing: Tokens.gapMedium

            PortraitFrame {
                Layout.alignment: Qt.AlignHCenter
                shape: "square"
                size: 360 * Tokens.uiScale
                source: root.portraitSource
            }

            FieldRow {
                Layout.fillWidth: true
                label: qsTr("Nom")
                value: root.speakerName
            }

            FieldRow {
                Layout.fillWidth: true
                label: qsTr("Attitude")
                value: root.attitude
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 2
            spacing: Tokens.gapLarge

            PanelFrame {
                Layout.fillWidth: true
                Layout.preferredHeight: 340 * Tokens.uiScale
                material: "parchment"
                subpanel: true

                SectionBanner {
                    id: lineBanner

                    anchors.left: parent.left
                    anchors.right: parent.right
                    text: qsTr("Réplique")
                }

                Text {
                    id: outcomeLabel

                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: lineBanner.bottom
                    anchors.topMargin: Tokens.gapSmall
                    visible: root.checkOutcome.length > 0
                    height: visible ? implicitHeight : 0
                    text: root.checkOutcome
                    color: Tokens.textMuted
                    font.family: Tokens.bodyFamily
                    font.pixelSize: Tokens.fontBody
                    elide: Text.ElideRight
                }

                Text {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: outcomeLabel.bottom
                    anchors.bottom: parent.bottom
                    anchors.topMargin: Tokens.gapMedium
                    text: root.line
                    color: Tokens.text
                    font.family: Tokens.loreFamily
                    font.italic: true
                    font.pixelSize: Tokens.fontSectionTitle
                    wrapMode: Text.WordWrap
                    elide: Text.ElideRight
                }
            }

            LedgerList {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: qsTr("Réponses")
                rows: root.replies
                interactive: true
                onRowActivated: (rowId) => root.replyChosen(rowId)
            }
        }
    }
}
