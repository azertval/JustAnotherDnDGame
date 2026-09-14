import QtQuick
import Jadg.Ui

/*!
    Le logotype du jeu : « Just Another D&D Game » sur son etendard grenat (LOT-87, T3.1).

    Piece `ui/plate/logo`, la seule du cahier qui porte des lettres : un logotype est un dessin, et
    le nom du jeu ne se traduit pas. Tant qu'elle n'est pas livree, la brique dessine son repli --
    une plaque grenat bordee d'or, et le nom en deux lignes de `titleFamily` ; le jour ou elle
    arrive, les lettres du repli s'effacent, puisque l'image porte les siennes.

    Pose au menu principal et aux credits (T3.3), a deux tailles d'affichage du cahier : 532 x 332
    et 450 x 282 a 1080p. La brique garde le rapport de la piece (1024 x 640) ; `width` suffit.
*/
Item {
    id: root

    implicitWidth: 532 * Tokens.uiScale
    implicitHeight: root.width * 640 / 1024

    Rectangle {
        anchors.fill: parent
        anchors.topMargin: parent.height * 0.12
        anchors.bottomMargin: parent.height * 0.12
        visible: !art.delivered
        color: Tokens.gem
        border.color: Tokens.goldLight
        border.width: Tokens.strokeWidth

        Column {
            anchors.centerIn: parent

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                // Le nom du jeu ne se traduit pas : pas de qsTr.
                text: "Just Another"
                color: Tokens.goldLight
                font.family: Tokens.titleFamily
                font.pixelSize: Tokens.fontScreenTitle * root.width / (532 * Tokens.uiScale)
                font.weight: Font.DemiBold
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "D&D Game"
                color: Tokens.goldLight
                font.family: Tokens.titleFamily
                font.pixelSize: Tokens.fontDisplay * root.width / (532 * Tokens.uiScale)
                font.weight: Font.Bold
            }
        }
    }

    FixedArt {
        id: art

        anchors.fill: parent
        key: "ui/plate/logo"
    }
}
