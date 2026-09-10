import QtQuick
import Jadg.Ui
import Jadg.Runtime

/*!
    Menu principal -- CABLAGE, cote developpeur (LOT-86).

    La navigation est REELLE : chaque entree appelle le routeur, qui suit la table de transitions
    pure. Un geste que la table n'autorise pas est refuse, jamais silencieusement accepte.

    Le CLAVIER et le POINTEUR pilotent le meme etat, `currentIndex` : survoler une entree la rend
    courante, et `Entree` active donc toujours celle que le chevron designe. C'est la raison pour
    laquelle la souris ne s'active pas directement -- deux notions de « l'entree choisie » auraient
    fini par diverger, et l'ecran aurait ouvert autre chose que ce qu'il montrait.
*/
MainMenuForm {
    id: root

    focus: true

    Keys.onUpPressed: root.currentIndex = (root.currentIndex + 3) % 4
    Keys.onDownPressed: root.currentIndex = (root.currentIndex + 1) % 4
    Keys.onReturnPressed: root.activate()
    Keys.onEnterPressed: root.activate()

    /// Le pointeur DESIGNE une entree, sans l'ouvrir : passer la souris en travers du menu ne doit
    /// rien declencher.
    function point(index) {
        root.currentIndex = index;
    }

    /// Le pointeur CHOISIT : on designe d'abord, on ouvre ensuite, par le meme chemin que le
    /// clavier. Un clic ne peut ainsi jamais ouvrir une entree que l'ecran ne montrait pas.
    function choose(index) {
        root.currentIndex = index;
        root.activate();
    }

    function activate() {
        switch (root.currentIndex) {
        case 0: ScreenRouter.openGame(); break
        case 1: ScreenRouter.openOptions(); break
        case 2: ScreenRouter.openCredits(); break
        case 3: Qt.quit(); break
        }
    }

    Connections {
        target: root.newGameEntry.pointer
        function onEntered() { root.point(0) }
        function onClicked() { root.choose(0) }
    }
    Connections {
        target: root.optionsEntry.pointer
        function onEntered() { root.point(1) }
        function onClicked() { root.choose(1) }
    }
    Connections {
        target: root.creditsEntry.pointer
        function onEntered() { root.point(2) }
        function onClicked() { root.choose(2) }
    }
    Connections {
        target: root.quitEntry.pointer
        function onEntered() { root.point(3) }
        function onClicked() { root.choose(3) }
    }
}
