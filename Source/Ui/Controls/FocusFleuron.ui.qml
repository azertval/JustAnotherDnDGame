import QtQuick
import QtQuick.Shapes
import Jadg.Ui

/*!
    Fleuron de focus : une pointe pleine tournee vers la droite (LOT-86, EX-IHM-071).

    Un POLYGONE, et non des paves : la charte parchemin peint anticrenele, et le triangle en
    escalier du pixel art n'a plus lieu d'etre.

    La pointe est ECHANCREE a l'arriere -- quatre sommets, pas trois -- pour qu'elle se lise comme
    une marque de position et non comme le curseur de saisie d'un champ de texte.

    Porte depuis `hmi::focusFleuronPoints`.
*/
Item {
    id: root

    property int size: Tokens.sectionTitle

    implicitWidth: root.size
    implicitHeight: root.size

    // Sous quatre pixels, l'echancrure arriere ne fait plus qu'un pixel de profondeur : la marque
    // cesse d'etre reconnaissable, et une marque qu'on ne reconnait pas ne designe rien.
    visible: root.size >= 4

    Shape {
        anchors.fill: parent
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            fillColor: Tokens.accent
            strokeWidth: 0
            PathPolyline {
                path: [
                    Qt.point(0, 0),
                    Qt.point(root.size, root.size * 0.5),
                    Qt.point(0, root.size),
                    Qt.point(root.size * 0.25, root.size * 0.5),
                    Qt.point(0, 0)
                ]
            }
        }
    }
}
