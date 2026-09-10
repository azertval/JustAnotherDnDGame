import QtQuick
import QtQuick.Layouts
import Jadg.Ui

/*!
    Un bloc d'IMAGE : portrait d'interlocuteur, carte, blason (LOT-86).

    Tant qu'aucune source n'est branchee, le bloc montre son CADRE et rien d'autre. Il occupe
    donc deja sa place : c'est ce qui permet de juger la mise en page avant que l'image existe,
    et ce qui evitera de la rejuger le jour ou elle arrive.
*/
SheetBlock {
    id: root

    property alias source: picture.source
    property real aspectRatio: 1.0

    /*!
        Hauteur maximale du bloc, en pixels a l'echelle courante.

        Sans elle, un blason carre dans une colonne large de 640 px devient haut de 640 px et
        chasse tout le reste de l'ecran hors du cadre. Le rapport d'aspect decide de la FORME ;
        cette borne decide de la PLACE.
    */
    property real maximumHeight: 96 * Tokens.scale

    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: Math.min(root.width / root.aspectRatio, root.maximumHeight)
        color: Tokens.surface
        border.width: Tokens.frameThickness
        border.color: Tokens.frameEdge

        Image {
            id: picture
            anchors.fill: parent
            anchors.margins: Tokens.frameThickness
            fillMode: Image.PreserveAspectFit
            // Les illustrations du corpus sont peintes, pas en pixel art : elles se redimensionnent
            // en interpole (EX-ARCH-022, refondue au LOT-66).
            smooth: true
            visible: status === Image.Ready
        }
    }
}
