import QtQuick
import Jadg.Ui

/*!
    Menu principal -- CABLAGE, cote developpeur (LOT-86).

    La navigation est REELLE : chaque entree appelle le routeur, qui suit la table de transitions
    pure. Un geste que la table n'autorise pas est refuse, jamais silencieusement accepte.
*/
MainMenuForm {
    id: root

    focus: true

    Keys.onUpPressed: root.currentIndex = (root.currentIndex + 3) % 4
    Keys.onDownPressed: root.currentIndex = (root.currentIndex + 1) % 4
    Keys.onReturnPressed: root.activate()
    Keys.onEnterPressed: root.activate()

    function activate() {
        switch (root.currentIndex) {
        case 0: ScreenRouter.openGame(); break
        case 1: ScreenRouter.openOptions(); break
        case 2: ScreenRouter.openCredits(); break
        case 3: Qt.quit(); break
        }
    }
}
