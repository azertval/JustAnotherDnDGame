import QtQuick
import Jadg.Ui

/*!
    La marque du focus de la charte v2 : un losange d'or facette (LOT-87, T3.1, EX-IHM-071).

    Piece `ui/control/focus-fleuron`, posee devant l'element courant. L'element focalise se signale
    par une MARQUE, jamais par sa seule teinte : la plaque grenat de l'entree active du menu est une
    teinte, et un joueur qui distingue mal le grenat du noir ne la verrait pas. Tant que la piece
    n'est pas livree, le repli est un carre tourne de 45 degres, or eclaire borde de `panelEdge`.

    Remplace `FocusFleuron` (v1) ecran par ecran ; tailles d'affichage du cahier : 24 et 40 px.
*/
Item {
    id: root

    /// Cote, a 1080p.
    property real size: 24 * Tokens.uiScale

    implicitWidth: root.size
    implicitHeight: root.size

    Rectangle {
        anchors.centerIn: parent
        // Le carre inscrit dans le cote, une fois tourne, tient dans le meme cote.
        width: root.size * 0.7
        height: root.size * 0.7
        rotation: 45
        visible: !art.delivered
        color: Tokens.goldLight
        border.color: Tokens.panelEdge
        border.width: Tokens.strokeWidth
        antialiasing: true
    }

    FixedArt {
        id: art

        anchors.fill: parent
        key: "ui/control/focus-fleuron"
    }
}
