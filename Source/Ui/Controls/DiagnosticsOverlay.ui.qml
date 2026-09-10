import QtQuick
import Jadg.Ui

/*!
    Compteur de diagnostic -- recouvrement, côté conception (LOT-86).

    Ce que le réglage « Compteur de diagnostic » allume (`EX-IHM-083` : tout réglage exposé atteint
    quelque chose de réel). Il ne figure pas dans la page des options mais par-dessus le jeu : un
    compteur d'images par seconde affiché dans une page de réglages ne mesurerait que cette page.

    La cadence vient de `FrameAnimation`, qui est **déclarative** -- elle est cadencée par le graphe
    de scène lui-même et n'a besoin d'aucune ligne de JavaScript. C'est ce qui permet à ce
    recouvrement de rester un `.ui.qml`, donc modifiable par la conception.

    `running` suit `visible` : une animation par image laissée en marche forcerait le rendu continu
    d'une fenêtre par ailleurs immobile, et un menu ferait alors tourner la machine pour rien.
*/
Item {
    id: overlay

    FrameAnimation {
        id: frame

        running: overlay.visible
    }

    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: Tokens.spaceSmall
        width: readout.width + Tokens.spaceMedium * 2
        height: readout.height + Tokens.spaceSmall * 2
        color: Tokens.surface
        border.color: Tokens.border
        border.width: Tokens.frameThickness
        opacity: 0.85

        Text {
            id: readout

            anchors.centerIn: parent
            color: Tokens.text
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.caption
            horizontalAlignment: Text.AlignRight
            // `smoothFrameTime` vaut zéro avant la première image : sans cette garde, le compteur
            // afficherait « Infinity » le temps d'une image, à chaque activation.
            text: (frame.smoothFrameTime > 0 ? Math.round(1 / frame.smoothFrameTime) : 0)
                  + " ips\n" + overlay.width + "x" + overlay.height + " · x" + Tokens.scale
        }
    }
}
