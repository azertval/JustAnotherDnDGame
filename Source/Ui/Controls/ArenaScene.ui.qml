import QtQuick
import Jadg.Ui

/*!
    La scene isometrique du Colisee -- le calque d'INTERFACE, au-dessus du rendu (LOT-86 Phase 6).

    Projette la grille de combat (`cells`, une entree par case, ligne par ligne) en losanges
    isometriques et pose sur chaque case une brique `ArenaTile` : la surbrillance, la jauge et les
    points de vie -- des rectangles et du texte, pas des pieces de la planche. Le sol, l'enceinte
    et les combattants animes viennent desormais du pipeline QRhi du jeu (`ArenaViewport`), pose
    dans l'hote que ce calque recouvre.

    Les deux partagent le meme rapport de losange (`core::ARENA_DIAMOND_RATIO`, la meme valeur que
    `diamondRatio` ci-dessous) et le meme cadrage centre, mais pas le meme calcul : ce calque
    ajuste `tileWidth` en continu a l'element, la camera du moteur (`hmi::Camera2D::fitZoom`)
    arrondit son zoom a l'entier pixel le plus proche pour un rendu net. L'alignement entre les
    deux reste donc approximatif -- suffisant pour situer une surbrillance sur la bonne case, pas
    garanti au pixel pres ; un reglage plus fin est laisse a un lot ulterieur.

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
    /// Le curseur de ciblage et le chemin jusqu'a lui (LOT-24), dans leur propre calque.
    property bool showCursor: false
    property int cursorColumn: 0
    property int cursorRow: 0
    property var pathCells: []

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

    // --- Le calque du ciblage (LOT-24) : le chemin, puis le curseur -----------------------------
    // Au-dessus de toute la scene : c'est de l'interface, pas du decor. Place a la profondeur de
    // sa case, le curseur etait coupe par le sol des cases dessinees devant lui.
    Repeater {
        model: root.pathCells

        ArenaMark {
            kind: "path"
            x: root.originX + (modelData.column - modelData.row) * root.tileWidth / 2
            y: root.originY + (modelData.column + modelData.row) * root.tileHeight / 2
            z: root.diagonals + 1
            width: root.tileWidth
            height: root.tileHeight
        }
    }

    ArenaMark {
        kind: "cursor"
        visible: root.showCursor
        x: root.originX + (root.cursorColumn - root.cursorRow) * root.tileWidth / 2
        y: root.originY + (root.cursorColumn + root.cursorRow) * root.tileHeight / 2
        z: root.diagonals + 2
        width: root.tileWidth
        height: root.tileHeight
    }
}
