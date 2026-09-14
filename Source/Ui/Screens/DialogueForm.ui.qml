import QtQuick
import QtQuick.Layouts
import Jadg.Ui

/*!
    Dialogue -- FORMULAIRE, cote conception (LOT-86, restyle LOT-87 T3.9).

    Pas de maquette : les briques et les jetons de la charte v2, sur la structure v1 -- l'interlocuteur
    a gauche (portrait, nom, attitude), sa replique et les reponses possibles a droite. Parchemin :
    un dialogue suspend le jeu, et se lit comme une page.

    Les proprietes portent des VALEURS D'EXEMPLE ; le jumeau les remplace par l'ancre `PendingData`,
    en attendant le lot des dialogues (`LOT-15`).
*/
ScreenPage {
    id: root

    property url portraitSource: ""
    property var replies: exampleReplies
    property string line: "Vous arrivez tard, et par la mauvaise route. Ceux qui viennent par là ont d'ordinaire quelque chose à cacher — ou quelqu'un à fuir. Lequel des deux, pour vous ?"
    property string speakerName: "—"
    property string attitude: "—"

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
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: lineBanner.bottom
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
            }
        }
    }
}
