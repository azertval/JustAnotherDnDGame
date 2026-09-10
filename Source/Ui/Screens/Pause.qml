import QtQuick
import Jadg.Ui

/*!
    Pause -- CABLAGE, cote developpeur (LOT-86).

    Clavier et pointeur pilotent le meme `currentIndex`, comme au menu principal : survoler designe,
    cliquer ouvre ce que le chevron designait.
*/
PauseForm {
    id: root

    focus: true

    Keys.onUpPressed: root.currentIndex = (root.currentIndex + 2) % 3
    Keys.onDownPressed: root.currentIndex = (root.currentIndex + 1) % 3
    Keys.onEscapePressed: ScreenRouter.resume()
    Keys.onReturnPressed: root.activate()
    Keys.onEnterPressed: root.activate()

    function point(index) {
        root.currentIndex = index;
    }

    function choose(index) {
        root.currentIndex = index;
        root.activate();
    }

    function activate() {
        switch (root.currentIndex) {
        case 0: ScreenRouter.resume(); break
        case 1: ScreenRouter.openOptions(); break
        case 2: ScreenRouter.quitToMenu(); break
        }
    }

    Connections {
        target: root.resumeEntry.pointer
        function onEntered() { root.point(0) }
        function onClicked() { root.choose(0) }
    }
    Connections {
        target: root.optionsEntry.pointer
        function onEntered() { root.point(1) }
        function onClicked() { root.choose(1) }
    }
    Connections {
        target: root.quitEntry.pointer
        function onEntered() { root.point(2) }
        function onClicked() { root.choose(2) }
    }
}
