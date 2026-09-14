import QtQuick
import Jadg.Ui

/*!
    La scene isometrique du Colisee (LOT-50, habillage du 14 septembre 2026).

    Projette la grille de combat (`cells`, une entree par case, ligne par ligne) en losanges
    isometriques et pose sur chaque case une brique `ArenaTile` : le sol, le mur ou la colonne
    d'enceinte, la surbrillance, le combattant anime, sa jauge. Les pieces viennent de la planche
    de production du Colisee (`Source/Elements/Assets/Coliseum/`, decoupee par
    `scripts/extract_coliseum_atlas.py`).

    La projection : une case (c, r) a son losange en x = (c - r) * L/2, y = (c + r) * H/2, ou L
    est la largeur du losange et H sa hauteur (H = 0,62 L : les tuiles de la planche sont dessinees
    a cet angle, pas au 2:1 classique). La scene se dimensionne pour que la grille entiere tienne
    dans l'element, murs compris. La profondeur est `z: c + r` : une piece posee sur une case
    passe devant tout ce qui est plus haut a l'ecran, sans tri explicite.

    L'element ne connait ni le moteur ni le modele : le formulaire lui donne les cases et ecoute
    `cellTapped`.
*/
Item {
    id: root

    property int gridColumns: 0
    property int gridRows: 0
    property var cells: []

    signal cellTapped(int column, int row)

    /// Rapport hauteur / largeur du losange, celui des tuiles de la planche.
    readonly property real diamondRatio: 0.62

    /// Hauteur reservee aux murs au-dessus du losange du fond, en largeurs de losange.
    readonly property real wallRise: 0.85

    readonly property int diagonals: Math.max(1, root.gridColumns + root.gridRows)

    /// Largeur du losange : la plus grande qui fasse tenir la grille et ses murs dans l'element.
    readonly property real tileWidth: Math.max(8, Math.min(root.width / (root.diagonals / 2),
                                                           root.height / (root.diagonals / 2 * root.diamondRatio + root.wallRise)))
    readonly property real tileHeight: root.tileWidth * root.diamondRatio

    readonly property real sceneWidth: root.diagonals / 2 * root.tileWidth
    readonly property real sceneHeight: root.diagonals / 2 * root.tileHeight + root.wallRise * root.tileWidth

    /// Origine : la case (0, rows - 1) est la plus a gauche ; la grille est centree dans l'element.
    readonly property real originX: (root.width - root.sceneWidth) / 2 + (root.gridRows - 1) * root.tileWidth / 2
    readonly property real originY: (root.height - root.sceneHeight) / 2 + root.wallRise * root.tileWidth

    Repeater {
        model: root.cells

        ArenaTile {
            x: root.originX + (modelData.column - modelData.row) * root.tileWidth / 2
            y: root.originY + (modelData.column + modelData.row) * root.tileHeight / 2
            z: modelData.column + modelData.row
            width: root.tileWidth
            height: root.tileHeight
            column: modelData.column
            row: modelData.row
            columns: root.gridColumns
            rows: root.gridRows
            wall: modelData.wall
            reachable: modelData.reachable
            active: modelData.active
            side: modelData.side
            occupant: modelData.occupant
            down: modelData.down
            hitPoints: modelData.hitPoints
            hitPointsRatio: modelData.hitPointsRatio
            pointer.onClicked: root.cellTapped(modelData.column, modelData.row)
        }
    }
}
