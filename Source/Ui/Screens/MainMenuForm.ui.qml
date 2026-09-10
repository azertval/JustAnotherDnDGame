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

    Rectangle {
        anchors.fill: parent
        color: Tokens.background
    }

    // Le degrade qui porte la lisibilite du titre et des entrees, sur le tiers gauche.
    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: parent.width / 3
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: Tokens.frameEdge }
            GradientStop { position: 1.0; color: "transparent" }
        }
        opacity: 0.35
    }

    ColumnLayout {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: Tokens.spaceExtraLarge * 2
        spacing: Tokens.spaceLarge

        Text {
            text: "JustAnotherDnDGame"
            color: Tokens.text
            font.family: Tokens.titleFamily
            font.pixelSize: Tokens.screenTitle
        }

        Item { Layout.preferredHeight: Tokens.spaceExtraLarge }

        MenuEntry { label: qsTr("Nouvelle partie"); current: root.currentIndex === 0 }
        MenuEntry { label: qsTr("Options"); current: root.currentIndex === 1 }
        MenuEntry { label: qsTr("Crédits"); current: root.currentIndex === 2 }
        MenuEntry { label: qsTr("Quitter"); current: root.currentIndex === 3 }
    }
}
