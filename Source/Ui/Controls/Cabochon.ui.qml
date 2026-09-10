import QtQuick
import QtQuick.Shapes
import Jadg.Ui

/*!
    Cabochon d'angle : un grenat taillé, serti d'or (LOT-86, EX-IHM-075).

    C'est l'ornement que les feuilles du corpus posent aux quatre angles de chaque panneau. Il
    remplace un carré d'or plein qui n'était pas faux, seulement muet : un aplat ne dit pas qu'il y
    a une pierre.

    **Tracé, et non livré en image**, et les trois raisons tiennent toujours en QML : une image
    s'ovalise quand on l'étire, fige ses couleurs hors des jetons, et n'est nette qu'à un seul
    facteur d'agrandissement. Une `Shape` se redessine à la taille demandée, prend ses couleurs de
    `Tokens.qml`, et reste nette partout.

    Porté depuis `hmi::cabochonShapes`, dont les proportions sont relevées sur
    `Character_Sheets_Tanares.pdf` — jamais choisies à vue.
*/
Item {
    id: root

    /// Côté du carré englobant, en pixels.
    property int size: 24 * Tokens.scale

    implicitWidth: root.size
    implicitHeight: root.size

    // Sous huit pixels, l'octogone du serti ne dessine plus rien de reconnaissable, et une marque
    // qu'on ne reconnaît pas ne désigne rien. Mieux vaut alors ne rien poser.
    visible: root.size >= 8

    readonly property real cut: root.size * 0.28
    readonly property real gemInset: root.size * 0.24
    readonly property real half: root.size * 0.5

    Shape {
        anchors.fill: parent
        preferredRendererType: Shape.CurveRenderer

        // Le serti : un octogone plutôt qu'un carré. Les angles coupés sont les griffes qui
        // tiennent la pierre, et ils se lisent même à la plus petite taille — quand les facettes
        // internes, elles, ne se lisent plus.
        ShapePath {
            fillColor: Tokens.frameOrnament
            strokeColor: Tokens.frameEdge
            strokeWidth: Tokens.frameThickness
            PathPolyline {
                path: [
                    Qt.point(root.cut, 0), Qt.point(root.size - root.cut, 0),
                    Qt.point(root.size, root.cut), Qt.point(root.size, root.size - root.cut),
                    Qt.point(root.size - root.cut, root.size), Qt.point(root.cut, root.size),
                    Qt.point(0, root.size - root.cut), Qt.point(0, root.cut),
                    Qt.point(root.cut, 0)
                ]
            }
        }

        // La gemme : un losange inscrit. DEUX facettes, et deux seulement — une claire au-dessus de
        // la diagonale, une profonde en dessous. Trois facettes ou plus ne se distinguent plus dès
        // que le cabochon descend sous la vingtaine de pixels, et un dégradé ne serait plus un rôle.
        ShapePath {
            fillColor: Tokens.gemShadow
            strokeWidth: 0
            PathPolyline {
                path: [
                    Qt.point(root.half, root.gemInset),
                    Qt.point(root.size - root.gemInset, root.half),
                    Qt.point(root.half, root.size - root.gemInset),
                    Qt.point(root.gemInset, root.half),
                    Qt.point(root.half, root.gemInset)
                ]
            }
        }

        // La facette claire couvre la moitié NORD-OUEST : la lumière de cette charte vient d'en
        // haut à gauche pour les gemmes, et d'elle seule — l'encadrement, lui, est plat.
        ShapePath {
            fillColor: Tokens.gem
            strokeWidth: 0
            PathPolyline {
                path: [
                    Qt.point(root.half, root.gemInset),
                    Qt.point(root.size - root.gemInset, root.half),
                    Qt.point(root.gemInset, root.half),
                    Qt.point(root.half, root.gemInset)
                ]
            }
        }

        // L'éclat, enfin : un petit losange contre le sommet nord. Posé en DERNIER, il recouvre la
        // facette claire — ce qui est exactement ce qu'un reflet fait.
        ShapePath {
            fillColor: Tokens.accentHover
            strokeWidth: 0
            PathPolyline {
                path: [
                    Qt.point(root.half, root.gemInset),
                    Qt.point(root.half + root.size * 0.10, root.gemInset + root.size * 0.10),
                    Qt.point(root.half, root.gemInset + root.size * 0.18),
                    Qt.point(root.half - root.size * 0.10, root.gemInset + root.size * 0.10),
                    Qt.point(root.half, root.gemInset)
                ]
            }
        }
    }
}
