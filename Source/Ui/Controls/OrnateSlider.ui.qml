import QtQuick
import QtQuick.Controls
import Jadg.Ui

/*!
    Curseur de la charte v2 : volume, luminosite (LOT-87, T2.7).

    Un `Slider` de Qt restyle : le glisser, le clavier et `value` viennent avec. Deux pieces du
    cahier, le rail (`ui/control/slider-track`, 24 px de haut) et la poignee
    (`ui/control/slider-handle`, 24 px, etats `normal` et `hover`). Le remplissage n'est pas une
    image : un aplat `panelEdge` sous la poignee, qui suit la valeur -- une image etiree a la
    largeur de la valeur deformerait ses extremites.
*/
Slider {
    id: root

    /// Etat impose de la poignee (`normal`, `hover`), ou vide pour le deduire.
    property string forcedState: ""

    readonly property string handleState: root.forcedState.length > 0 ? root.forcedState
                                          : (root.hovered || root.pressed ? "hover" : "normal")

    implicitWidth: 320 * Tokens.uiScale
    implicitHeight: 40 * Tokens.uiScale
    padding: 0
    opacity: root.enabled ? 1.0 : 0.5

    background: Item {
        x: root.leftPadding
        y: root.topPadding + (root.availableHeight - height) / 2
        width: root.availableWidth
        height: 24 * Tokens.uiScale

        Rectangle {
            anchors.fill: parent
            visible: !trackArt.delivered
            color: Tokens.panel
            border.color: Tokens.panelEdge
            border.width: Tokens.strokeWidth
        }

        NinePatchArt {
            id: trackArt

            anchors.fill: parent
            key: "ui/control/slider-track"
        }

        Rectangle {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: parent.height * 0.25
            width: Math.max(0, root.visualPosition * (parent.width - parent.height * 0.5))
            height: parent.height * 0.5
            color: Tokens.panelEdge
        }
    }

    handle: Item {
        x: root.leftPadding + root.visualPosition * (root.availableWidth - width)
        y: root.topPadding + (root.availableHeight - height) / 2
        implicitWidth: 32 * Tokens.uiScale
        implicitHeight: 32 * Tokens.uiScale

        Rectangle {
            anchors.fill: parent
            visible: !handleArt.delivered
            radius: width / 2
            color: root.handleState === "hover" ? Tokens.goldLight : Tokens.panelEdge
            border.color: Tokens.panel
            border.width: Tokens.strokeWidth
        }

        FixedArt {
            id: handleArt

            anchors.fill: parent
            key: "ui/control/slider-handle/" + root.handleState
        }
    }
}
