import QtQuick
import Jadg.Ui

/*!
    Independent Qt Design Studio entry point.

    This file intentionally imports only Jadg.Ui. It never imports Jadg.Runtime and therefore can be
    opened by Qt Design Studio 6.8.x without configuring or building the C++ application.
*/
Item {
    id: root

    width: 1280
    height: 720

    MainMenuForm {
        anchors.fill: parent
        currentIndex: 0
    }
}
