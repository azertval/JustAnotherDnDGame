import QtQuick
import QtQuick.Layouts
import Jadg.Ui

/*!
    Pause -- FORMULAIRE, cote conception (LOT-86).

    Un RECOUVREMENT, pas une page : le jeu reste derriere, assombri. C'est ce qui distingue une
    pause d'un retour au menu, et le voile ne doit donc pas etre opaque.
*/
Item {
    id: root

    property int currentIndex: 0

    // Voile : le jeu doit rester DEVINABLE derriere, sans quoi la pause ressemble a un ecran
    // de plus et l'on ne sait plus si la partie tourne encore.
    Rectangle {
        anchors.fill: parent
        color: Tokens.frameEdge
        opacity: 0.72
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: Tokens.spaceLarge

        Text {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Pause")
            color: Tokens.surfaceAlt
            font.family: Tokens.titleFamily
            font.pixelSize: Tokens.sectionTitle
        }

        Item { Layout.preferredHeight: Tokens.spaceMedium }

        MenuEntry { Layout.alignment: Qt.AlignHCenter; label: qsTr("Reprendre"); current: root.currentIndex === 0 }
        MenuEntry { Layout.alignment: Qt.AlignHCenter; label: qsTr("Options"); current: root.currentIndex === 1 }
        MenuEntry { Layout.alignment: Qt.AlignHCenter; label: qsTr("Quitter vers le menu"); current: root.currentIndex === 2 }
    }
}
