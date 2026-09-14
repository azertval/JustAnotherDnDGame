import QtQuick
import Jadg.Ui

/*!
    Une marque posee sur une case de la scene isometrique du Colisee (LOT-24).

    L'element EST le losange de la case, comme `ArenaTile` : la scene le place par la meme
    projection. Il ne dessine qu'un losange de jetons :

    - `cursor` : le contour d'or epais de la case visee par le curseur de ciblage ;
    - `path` : un petit losange d'or plein sur une case que le deplacement traverserait.

    Les marques vivent dans leur propre calque, au-dessus des cases : deplacer le curseur ne
    reconstruit pas les centaines de cases de la scene, seulement ces quelques marques.
*/
Item {
    id: root

    /// `cursor` ou `path`.
    property string kind: "cursor"

    readonly property bool cursor: root.kind === "cursor"

    Rectangle {
        id: diamond

        anchors.centerIn: parent
        width: root.width * (root.cursor ? 0.7 : 0.26)
        height: width
        color: root.cursor ? "transparent" : Tokens.goldLight
        opacity: root.cursor ? 1 : 0.7
        border.color: Tokens.goldLight
        border.width: root.cursor ? 4 : 0
        transform: [
            Rotation { angle: 45; origin.x: diamond.width / 2; origin.y: diamond.height / 2 },
            Scale { yScale: root.height / root.width; origin.y: diamond.height / 2 }
        ]
    }
}
