import QtQuick
import QtQuick.Layouts
import Jadg.Ui

/*!
    Menu principal -- FORMULAIRE, cote conception (LOT-86).

    Pas de cadre : les entrees se posent sur une scene, et la lisibilite tient a un degrade sombre
    sur le tiers gauche plutot qu'a un voile plein -- le decor doit rester visible. C'est la
    decision que portaient les planches, reprise ici.

    « Mode Edition » a disparu des entrees : l'editeur de niveaux est un BINAIRE SEPARE depuis ce
    lot, et le jeu n'a plus aucun chemin qui y mene.

    The background is deliberately supplied by the developer-side wrapper through
    `backgroundSource`. The design form therefore has no dependency on runtime asset deployment:
    Qt Design Studio can preview the form even when only the source tree is present.
*/
Item {
    id: root

    property int currentIndex: 0
    property url backgroundSource: ""

    // Les entrees sont exposees une a une : c'est le jumeau qui leur attache le survol et le clic,
    // un formulaire ne pouvant pas contenir de code.
    property alias newGameEntry: newGameControl
    property alias optionsEntry: optionsControl
    property alias creditsEntry: creditsControl
    property alias quitEntry: quitControl

    Rectangle {
        anchors.fill: parent
        color: Tokens.background
    }

    // The runtime wrapper supplies the embedded asset. Keeping the URL outside the form avoids a
    // dependency on a build directory or on runtime deployment while designing the form.
    Image {
        anchors.fill: parent
        source: root.backgroundSource
        visible: root.backgroundSource !== ""
        fillMode: Image.PreserveAspectCrop
        smooth: true
    }

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: parent.width * 0.55
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop {
                position: 0.0
                color: Qt.rgba(Tokens.frameEdge.r, Tokens.frameEdge.g, Tokens.frameEdge.b, 0.92)
            }
            GradientStop {
                position: 0.55
                color: Qt.rgba(Tokens.frameEdge.r, Tokens.frameEdge.g, Tokens.frameEdge.b, 0.72)
            }
            GradientStop {
                position: 1.0
                color: Qt.rgba(Tokens.frameEdge.r, Tokens.frameEdge.g, Tokens.frameEdge.b, 0.0)
            }
        }
    }

    ColumnLayout {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: Tokens.spaceExtraLarge * 2
        spacing: Tokens.spaceLarge

        Text {
            text: "JustAnotherDnDGame"
            color: Tokens.surfaceAlt
            font.family: Tokens.titleFamily
            font.pixelSize: Tokens.screenTitle
        }

        Item { Layout.preferredHeight: Tokens.spaceExtraLarge }

        MenuEntry { id: newGameControl; label: qsTr("Nouvelle partie"); current: root.currentIndex === 0 }
        MenuEntry { id: optionsControl; label: qsTr("Options"); current: root.currentIndex === 1 }
        MenuEntry { id: creditsControl; label: qsTr("Crédits"); current: root.currentIndex === 2 }
        MenuEntry { id: quitControl; label: qsTr("Quitter"); current: root.currentIndex === 3 }
    }
}
