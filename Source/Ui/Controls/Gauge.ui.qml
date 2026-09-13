import QtQuick
import Jadg.Ui

/*!
    Jauge : vie, experience, charge (LOT-87, T2.7).

    Le rail est une piece du cahier (`ui/gauge/track`, 28 px de haut, embouts dans ses marges de
    40 px). Le remplissage, lui, n'est PAS une image peinte par couleur : un aplat de jetons
    -- `gemLight` pour la vie, `goldLight` pour l'experience, `panelEdge` pour la charge -- sous un
    relief en niveaux de gris (`ui/gauge/fill-sheen`), comme le T2.4 l'a tranche. Changer la teinte
    de la vie se fait donc dans `Tokens.qml`, sans regenerer une image.
*/
Item {
    id: root

    /// `health`, `experience` ou `weight`.
    property string kind: "health"

    /// Remplissage, de 0 a 1.
    property real value: 0.5

    /// Texte pose sur la jauge (`32 / 45`), ou vide.
    property string label: ""

    implicitWidth: 320 * Tokens.uiScale
    implicitHeight: 28 * Tokens.uiScale

    Rectangle {
        anchors.fill: parent
        visible: !trackArt.delivered
        radius: height / 2
        color: Tokens.panel
        border.color: Tokens.panelEdge
        border.width: Tokens.strokeWidth
    }

    NinePatchArt {
        id: trackArt

        anchors.fill: parent
        key: "ui/gauge/track"
    }

    // Le remplissage tient DANS le rail : entre ses embouts quand l'image est livree, dans son
    // trait sinon.
    Item {
        id: fillArea

        anchors.fill: parent
        anchors.leftMargin: trackArt.delivered ? 20 * Tokens.uiScale : 2 * Tokens.strokeWidth
        anchors.rightMargin: trackArt.delivered ? 20 * Tokens.uiScale : 2 * Tokens.strokeWidth
        anchors.topMargin: 2 * Tokens.strokeWidth
        anchors.bottomMargin: 2 * Tokens.strokeWidth

        Item {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: parent.width * Math.max(0, Math.min(1, root.value))
            clip: true

            Rectangle {
                anchors.fill: parent
                radius: height / 2
                color: root.kind === "experience" ? Tokens.goldLight
                       : (root.kind === "weight" ? Tokens.panelEdge : Tokens.gemLight)
            }

            NinePatchArt {
                anchors.fill: parent
                key: "ui/gauge/fill-sheen"
            }
        }
    }

    Text {
        anchors.centerIn: parent
        visible: root.label.length > 0
        text: root.label
        color: Tokens.textOnPanel
        font.family: Tokens.bodyFamily
        font.pixelSize: Tokens.fontCaption
    }
}
