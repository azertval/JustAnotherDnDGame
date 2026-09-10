import QtQuick
import Jadg.Ui

/*!
    Pause -- CABLAGE, cote developpeur (LOT-86).
*/
PauseForm {
    id: root

    focus: true

    Keys.onUpPressed: root.currentIndex = (root.currentIndex + 2) % 3
    Keys.onDownPressed: root.currentIndex = (root.currentIndex + 1) % 3
    Keys.onEscapePressed: ScreenRouter.resume()
    Keys.onReturnPressed: root.activate()
    Keys.onEnterPressed: root.activate()

    function activate() {
        switch (root.currentIndex) {
        case 0: ScreenRouter.resume(); break
        case 1: ScreenRouter.openOptions(); break
        case 2: ScreenRouter.quitToMenu(); break
        }
    }
}
