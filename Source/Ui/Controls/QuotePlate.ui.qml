import QtQuick
import Jadg.Ui

/*!
    Cartouche de citation : un bandeau sombre etroit, un fleuron d'or a chaque bout (LOT-87, T3.1).

    Piece `ui/plate/quote`, 56 px de haut a 1080p, marges de 64 px a gauche et a droite qui portent
    les fleurons ; le milieu se repete sous le texte. La largeur suit la citation. Le texte est
    pose par le jeu en `loreFamily` italique, jamais peint : le cahier interdit les lettres a toute
    piece hors le logotype.
*/
Item {
    id: root

    property string text: "Citation"

    readonly property real endWidth: 64 * Tokens.uiScale

    implicitHeight: 56 * Tokens.uiScale
    implicitWidth: quoteLabel.implicitWidth + 2 * root.endWidth

    Rectangle {
        anchors.fill: parent
        visible: !art.delivered
        color: Tokens.panel
        border.color: Tokens.panelEdge
        border.width: Tokens.strokeWidth
    }

    NinePatchArt {
        id: art

        anchors.fill: parent
        key: "ui/plate/quote"
    }

    Text {
        id: quoteLabel

        anchors.centerIn: parent
        text: root.text
        color: Tokens.textOnPanel
        font.family: Tokens.loreFamily
        font.italic: true
        font.pixelSize: Tokens.fontBody
    }
}
