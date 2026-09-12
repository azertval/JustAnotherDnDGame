import QtQuick
import Jadg.Ui
import Jadg.Runtime

/*!
    Vue de jeu -- CABLAGE, cote developpeur (LOT-86).

    La session de jeu se branchera ici quand il y aura un niveau a jouer.
*/
GameViewForm {
    id: root

    focus: true
    status: qsTr("Aucune carte à jouer : le contenu arrive avec un lot ultérieur.")

    // La surface de rendu QRhi, posée dans l'hôte que le formulaire réserve. Elle est ici et non
    // dans le formulaire parce que c'est un type C++ (`Jadg.Runtime`), invisible à l'atelier ; le
    // rappel du formulaire reste au-dessus d'elle, comme un enfant ordinaire.
    GameViewport {
        parent: root.viewportHost
        anchors.fill: parent
        clearColor: Tokens.background
    }

    Keys.onEscapePressed: ScreenRouter.openPause()
}
