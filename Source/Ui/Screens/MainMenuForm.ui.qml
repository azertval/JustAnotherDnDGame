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
*/
Item {
    id: root

    property int currentIndex: 0

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

    // Le decor : la carte du monde, extraite du corpus (EX-IHM-076). La maquette voulait un FOND,
    // pas un aplat -- « les entrees ne sont plus enfermees dans un panneau, elles se posent sur une
    // scene ». `PreserveAspectCrop` remplit sans deformer : une carte etiree se lit tout de suite.
    Image {
        anchors.fill: parent
        source: "../assets/world-map.jpg"
        fillMode: Image.PreserveAspectCrop
        // Peinte, pas en pixel art : elle se redimensionne en interpole (EX-ARCH-022, LOT-66).
        smooth: true
    }

    // Le degrade qui porte la lisibilite du titre et des entrees -- et NON un voile plein sur tout
    // l'ecran : la maquette voulait que le decor reste visible.
    //
    // L'ALPHA est dans les paliers, pas dans l'opacite de l'element : une opacite uniforme
    // eclaircirait aussi le bord gauche, la ou le texte a le plus besoin de fond. Et la teinte
    // vient du jeton d'encre, pas d'une couleur ecrite ici -- elle suivra un changement de palette.
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
