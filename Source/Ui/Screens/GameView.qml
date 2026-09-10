import QtQuick
import Jadg.Ui

/*!
    Vue de jeu -- CABLAGE, cote developpeur (LOT-86).

    La session de jeu se branchera ici quand il y aura un niveau a jouer.
*/
GameViewForm {
    id: root

    focus: true
    status: qsTr("Aucune carte à jouer : le contenu arrive avec un lot ultérieur.")

    Keys.onEscapePressed: ScreenRouter.openPause()
}
