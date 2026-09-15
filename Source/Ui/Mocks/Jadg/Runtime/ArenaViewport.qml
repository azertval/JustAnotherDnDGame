import QtQuick

/*!
    Doublure de `hmi::ArenaViewportItem` (type QML `ArenaViewport`) pour Qt Design Studio (LOT-86).

    La vraie surface rend la scene du Colisee par QRhi et publie son cadrage ; dans l'atelier, un
    aplat de la couleur d'effacement et un cadrage fixe suffisent a voir ou elle se pose et ou le
    calque de ciblage tombe.
*/
Rectangle {
    property var model: null
    property color clearColor: "transparent"

    readonly property real tileWidth: 64
    readonly property real tileHeight: 40
    readonly property real originX: width / 2 - tileWidth / 2
    readonly property real originY: tileHeight

    function cellAt(x, y) {
        return Qt.point(-1, -1)
    }

    color: clearColor
}
