import QtQuick
import Jadg.Ui

/*!
    Compteur de diagnostic -- FORMULAIRE, cote conception (LOT-86).

    Le formulaire ne connait pas le mecanisme de mesure. Le texte affiche est fourni par le jumeau
    `DiagnosticsOverlay.qml`, ce qui permet a Qt Design Studio de manipuler librement la presentation.
*/
Item {
    id: overlay

    property string readoutText: "0 ips\n—x— · x1"

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
            text: overlay.readoutText
        }
    }
}
