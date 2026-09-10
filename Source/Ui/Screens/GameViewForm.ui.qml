import QtQuick
import Jadg.Ui

/*!
    Vue de jeu -- FORMULAIRE, cote conception (LOT-86).

    Le formulaire ne contient que la composition visuelle editable dans Qt Design Studio.
    La surface de rendu QRhi est branchee dans `GameView.qml`, cote developpeur, afin que ce
    formulaire reste autonome et ouvrable sans les types C++ du moteur.
*/
Item {
    id: root

    property string status: "—"

    // Placeholder visuel pour la conception. Le vrai viewport QRhi est ajoute par GameView.qml.
    Rectangle {
        anchors.fill: parent
        color: Tokens.background
    }

    Text {
        anchors.centerIn: parent
        text: root.status
        color: Tokens.textMuted
        font.family: Tokens.bodyFamily
        font.pixelSize: Tokens.body
        z: 1
    }
}
