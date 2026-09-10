import QtQuick
import Jadg.Ui

/*!
    Compteur de diagnostic -- CABLAGE, cote developpeur.

    FrameAnimation appartient au runtime : le formulaire `.ui.qml` reste purement visuel et
    Qt Design Studio peut donc le reouvrir et le reecrire sans connaitre la logique de mesure.
*/
DiagnosticsOverlayForm {
    id: root

    FrameAnimation {
        id: frame
        running: root.visible
    }

    readonly property int fps: frame.smoothFrameTime > 0
        ? Math.round(1 / frame.smoothFrameTime)
        : 0

    readoutText: fps + " ips\n" + root.width + "x" + root.height + " · x" + Tokens.scale
}
