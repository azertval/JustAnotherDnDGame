import QtQuick
import Jadg.Ui
import Jadg.Runtime

/*!
    Vue de jeu -- CABLAGE, cote developpeur (LOT-86).

    Le viewport QRhi est un type C++ runtime : il est volontairement branche ici et non dans le
    `.ui.qml`, afin que Qt Design Studio puisse ouvrir le formulaire sans charger le moteur.
*/
GameViewForm {
    id: root

    focus: true
    status: qsTr("Aucune carte à jouer : le contenu arrive avec un lot ultérieur.")

    GameViewport {
        anchors.fill: parent
        clearColor: Tokens.background
        z: -1
    }

    Keys.onEscapePressed: ScreenRouter.openPause()
}
