import QtQuick
import Jadg.Ui

/*!
    Separateur d'or : entre deux reglages, deux sections, deux paragraphes (LOT-87, T2.7).

    Piece `ui/control/divider-gold`, 16 px de haut a 1080p, extremites ornees dans ses marges de
    48 px et filet repete entre elles. Sans image livree : un filet `panelEdge` et un losange
    `goldLight` au centre -- le motif de l'intertitre de la maquette des options.
*/
Item {
    id: root

    implicitWidth: 512 * Tokens.uiScale
    implicitHeight: 16 * Tokens.uiScale

    Item {
        anchors.fill: parent
        visible: !art.delivered

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            height: Tokens.strokeWidth
            color: Tokens.panelEdge
        }

        Rectangle {
            anchors.centerIn: parent
            width: parent.height * 0.6
            height: parent.height * 0.6
            rotation: 45
            color: Tokens.goldLight
        }
    }

    NinePatchArt {
        id: art

        anchors.fill: parent
        key: "ui/control/divider-gold"
    }
}
