import QtQuick

/*!
    Doublure de `hmi::GameViewportItem` (type QML `GameViewport`) pour Qt Design Studio (LOT-87).

    La vraie surface rend la scène par QRhi ; dans l'atelier, un aplat de la couleur d'effacement
    suffit à voir où elle se pose et ce qui se dessine par-dessus.
*/
Rectangle {
    property color clearColor: "transparent"

    color: clearColor
}
