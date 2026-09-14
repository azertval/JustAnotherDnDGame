import QtQuick
import Jadg.Ui

/*!
    Une case de la scene isometrique du Colisee (LOT-50, habillage du 14 septembre 2026).

    L'element EST le losange de la case (largeur x hauteur) ; ce qu'il porte deborde vers le haut,
    comme un mur ou une figurine debout sur le sol. Du sol vers le ciel :

    - le sol : sable de la planche, ou une dalle du tileset du Colisee une case sur trois, tiree
      de la position pour que la meme grille se dessine toujours pareil ;
    - l'enceinte, sur les cases `wall` : un pan de mur, une colonne aux angles, une banniere ou
      une torche a intervalle regulier ; une arche sur les deux portes (les cases libres du bord) ;
    - la surbrillance : un losange de jetons, atteignable (`info`), allie (`textAlly`), ennemi
      (`textEnemy`), au tour (`goldLight`) ;
    - le combattant : une bande d'animation de la planche, un heros pour un allie, un gladiateur
      pour un ennemi, choisi d'apres le nom pour rester le meme d'un tour a l'autre ; a terre, la
      figurine s'arrete et s'estompe ;
    - la jauge de vie d'un ennemi (brique `Gauge`) au-dessus de lui, les points de vie en texte
      sous la case, pour les deux camps.

    Les chemins des pieces sont relatifs a ce fichier, le meme mecanisme que les illustrations de
    la charte v2 : identiques dans l'atelier, depuis les sources et depuis la ressource.
*/
Item {
    id: root

    property int column: 0
    property int row: 0
    property int columns: 1
    property int rows: 1

    property bool wall: false
    property bool reachable: false
    property bool active: false

    /// `allies`, `enemies`, ou vide.
    property string side: ""
    property string occupant: ""
    property bool down: false
    property string hitPoints: ""
    property real hitPointsRatio: 1

    /// La zone sensible au pointeur, a brancher par la scene (`clicked`).
    property alias pointer: tilePointer

    readonly property string assets: "../../Elements/Assets/Coliseum/"

    readonly property bool occupied: root.side.length > 0
    readonly property bool ally: root.side === "allies"
    readonly property bool enemy: root.side === "enemies"

    // --- La planche : ses tuiles font 86 px de large ; tout s'y rapporte -------------------------
    readonly property real unit: root.width / 86

    // --- L'enceinte ---------------------------------------------------------------------------
    readonly property bool corner: (root.column === 0 || root.column === root.columns - 1)
                                   && (root.row === 0 || root.row === root.rows - 1)
    readonly property bool topOrBottom: root.row === 0 || root.row === root.rows - 1
    readonly property bool bannerSpot: root.wall && !root.corner && root.topOrBottom && root.column % 5 === 0
    readonly property bool torchSpot: root.wall && !root.corner && !root.topOrBottom && root.row % 4 === 2
    readonly property bool gateSpot: !root.wall && (root.column === 0 || root.column === root.columns - 1
                                                    || root.row === 0 || root.row === root.rows - 1)

    readonly property string highlight: root.active ? "active"
                                        : (root.ally ? "ally"
                                        : (root.enemy ? "enemy"
                                        : (root.reachable ? "reachable" : "")))

    // --- Le combattant ------------------------------------------------------------------------
    readonly property var heroes: ["kaelith_voss", "bram", "elira", "darin"]
    readonly property var gladiators: ["gladiator_sword_shield", "gladiator_lance", "retiarius", "archer"]
    readonly property int figure: root.occupant.length > 0
                                  ? (root.occupant.length * 7 + root.occupant.charCodeAt(0)) % 4 : 0
    readonly property string sheet: !root.occupied ? ""
                                    : (root.ally ? root.assets + "characters/" + root.heroes[root.figure] + (root.down ? "/death.png" : "/idle.png")
                                                 : root.assets + "enemies/" + root.gladiators[root.figure] + "/idle.png")
    readonly property int frames: root.ally ? 5 : 8

    // --- Le sol -------------------------------------------------------------------------------
    // Sable partout, une dalle claire du tileset une case sur sept, tiree de la position ; les
    // dalles rouges et sombres du tileset sont reservees a un futur marquage (autel, sang).
    readonly property var paleSlabs: ["01", "02", "03", "04", "05", "10", "11", "13", "14", "15"]
    readonly property bool slab: !root.wall && (root.column * 3 + root.row * 5 + root.column * root.row) % 7 === 0

    Image {
        anchors.fill: parent
        source: root.assets + (root.slab
                               ? "coliseum/" + root.paleSlabs[(root.column * 3 + root.row * 5) % 10] + ".png"
                               : (root.wall ? "terrain/stone.png" : "terrain/sand.png"))
        fillMode: Image.Stretch
        smooth: true
        mipmap: true
    }

    // --- La surbrillance : un losange de jetons ------------------------------------------------
    Rectangle {
        id: diamond

        anchors.centerIn: parent
        width: root.width * 0.62
        height: width
        visible: root.highlight.length > 0
        color: root.highlight === "active" ? Tokens.goldLight
               : (root.highlight === "ally" ? Tokens.textAlly
               : (root.highlight === "enemy" ? Tokens.textEnemy : Tokens.textAlly))
        opacity: root.highlight === "reachable" ? 0.28 : 0.42
        border.color: root.highlight === "active" ? Tokens.goldLight
                      : (root.highlight === "enemy" ? Tokens.textEnemy : Tokens.textAlly)
        border.width: root.highlight === "active" ? 3 : 2
        transform: [
            Rotation { angle: 45; origin.x: diamond.width / 2; origin.y: diamond.height / 2 },
            Scale { yScale: root.height / root.width; origin.y: diamond.height / 2 }
        ]
    }

    // --- L'enceinte ---------------------------------------------------------------------------
    Image {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: root.height * 0.12
        visible: root.wall
        source: root.assets + (root.corner ? "structures/column_large.png" : "structures/wall.png")
        width: sourceSize.width * root.unit
        height: sourceSize.height * root.unit
        smooth: true
        mipmap: true
    }

    Image {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: root.height * 0.3
        visible: root.bannerSpot || root.torchSpot
        source: root.assets + (root.bannerSpot ? "structures/banner_01.png" : "structures/torch_01.png")
        width: sourceSize.width * root.unit
        height: sourceSize.height * root.unit
        smooth: true
        mipmap: true
    }

    Image {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: root.height * 0.1
        visible: root.gateSpot
        source: root.assets + "structures/arch.png"
        width: sourceSize.width * root.unit
        height: sourceSize.height * root.unit
        smooth: true
        mipmap: true
    }

    // --- Le combattant ------------------------------------------------------------------------
    AnimatedSprite {
        id: sprite

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: root.height * 0.42
        visible: root.occupied
        width: 48 * root.unit * 1.25
        height: 64 * root.unit * 1.25
        source: root.sheet
        frameCount: root.frames
        frameWidth: 48
        frameHeight: 64
        frameRate: 5
        running: root.occupied && !root.down
        currentFrame: root.down ? root.frames - 1 : 0
        interpolate: false
        smooth: true
        opacity: root.down && root.enemy ? 0.45 : 1
    }

    // --- La jauge d'un ennemi, au-dessus de lui --------------------------------------------------
    Gauge {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: sprite.top
        anchors.bottomMargin: 2
        visible: root.enemy && !root.down
        width: root.width * 0.5
        height: Math.max(5, root.width * 0.08)
        kind: "health"
        value: root.hitPointsRatio
    }

    // --- Les points de vie -----------------------------------------------------------------------
    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.bottom
        anchors.topMargin: -root.height * 0.3
        visible: root.occupied
        text: root.hitPoints
        color: root.ally ? Tokens.textAlly : Tokens.textOnPanel
        font.family: Tokens.bodyFamily
        font.pixelSize: Math.max(9, root.width * 0.2)
        style: Text.Outline
        styleColor: Tokens.panel
    }

    MouseArea {
        id: tilePointer

        anchors.fill: parent
    }
}
