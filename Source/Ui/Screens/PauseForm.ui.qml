import QtQuick
import Jadg.Ui

/*!
    Pause -- FORMULAIRE, cote conception (LOT-86, restyle LOT-87 T3.9).

    Un RECOUVREMENT, pas une page : le jeu reste derriere, assombri. C'est ce qui distingue une
    pause d'un retour au menu, et le voile ne doit donc pas etre opaque.

    Charte v2 sans maquette : un panneau sombre au centre, sa plaque grenat, et trois entrees de
    menu -- les memes boutons et la meme marque de focus que le menu principal (T3.1).
*/
Item {
    id: root

    property int currentIndex: 0

    // Exposees pour que le jumeau y branche le survol et le clic (voir `MainMenuForm`).
    property alias resumeEntry: resumeControl
    property alias optionsEntry: optionsControl
    property alias quitEntry: quitControl

    // Voile : le jeu doit rester DEVINABLE derriere, sans quoi la pause ressemble a un ecran
    // de plus et l'on ne sait plus si la partie tourne encore.
    Rectangle {
        anchors.fill: parent
        color: Tokens.panel
        opacity: 0.72
    }

    PanelFrame {
        id: panel

        anchors.centerIn: parent
        width: 640 * Tokens.uiScale
        height: 440 * Tokens.uiScale

        Column {
            // Centree sous la plaque de titre, qui mord de moitie sur le haut du panneau (60 px a
            // 1080p) : collee au bas, la derniere entree chevauchait le cadre peint.
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: 30 * Tokens.uiScale
            spacing: Tokens.gapMedium

            Row {
                spacing: Tokens.gapSmall

                FocusMark { anchors.verticalCenter: parent.verticalCenter; opacity: root.currentIndex === 0 ? 1 : 0 }
                OrnateButton {
                    id: resumeControl
                    kind: "menu"
                    text: qsTr("Reprendre")
                    highlighted: root.currentIndex === 0
                    focusPolicy: Qt.NoFocus
                }
            }

            Row {
                spacing: Tokens.gapSmall

                FocusMark { anchors.verticalCenter: parent.verticalCenter; opacity: root.currentIndex === 1 ? 1 : 0 }
                OrnateButton {
                    id: optionsControl
                    kind: "menu"
                    text: qsTr("Options")
                    highlighted: root.currentIndex === 1
                    focusPolicy: Qt.NoFocus
                }
            }

            Row {
                spacing: Tokens.gapSmall

                FocusMark { anchors.verticalCenter: parent.verticalCenter; opacity: root.currentIndex === 2 ? 1 : 0 }
                OrnateButton {
                    id: quitControl
                    kind: "menu"
                    text: qsTr("Quitter vers le menu")
                    highlighted: root.currentIndex === 2
                    focusPolicy: Qt.NoFocus
                }
            }
        }
    }

    TitlePlate {
        anchors.horizontalCenter: panel.horizontalCenter
        anchors.verticalCenter: panel.top
        material: "garnet"
        text: qsTr("Pause")
    }
}
