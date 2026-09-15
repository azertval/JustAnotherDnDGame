import QtQuick
import Jadg.Ui

/*!
    Une case de la scene isometrique du Colisee -- l'INTERFACE d'une case (LOT-86 Phase 6).

    Jusqu'au LOT-86, l'element dessinait tout : sol, enceinte et combattant venaient de la planche
    de production. Depuis, ces pieces sont composees par le pipeline QRhi du jeu (`ArenaViewport`,
    `HMI/Graphics/ArenaSceneComposer`), qui les dessine sous cette case -- la redessiner ici les
    ferait doubler, et c'etait la brique dont les centaines de delegues (`Image`, `AnimatedSprite`)
    detruits et recrees a chaque geste de combat faisaient monter la memoire jusqu'au plantage.

    L'element EST le losange de la case (largeur x hauteur) et ne porte plus que l'interface,
    volontairement absente de la composition GPU (`ArenaSceneComposer` : « des rectangles et du
    texte d'interface, pas des pieces de la planche ») :

    - la surbrillance : un losange de jetons, atteignable (`info`), allie (`textAlly`), ennemi
      (`textEnemy`), au tour (`goldLight`) ;
    - la jauge de vie d'un ennemi (brique `Gauge`) au-dessus de lui, les points de vie en texte
      sous la case, pour les deux camps ;
    - la zone sensible au pointeur, pour le geste de la souris (`cellTapped`).
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

    readonly property bool occupied: root.side.length > 0
    readonly property bool ally: root.side === "allies"
    readonly property bool enemy: root.side === "enemies"

    readonly property string highlight: root.active ? "active"
                                        : (root.ally ? "ally"
                                        : (root.enemy ? "enemy"
                                        : (root.reachable ? "reachable" : "")))

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

    // --- La jauge d'un ennemi, au-dessus de la figurine que la scene rendue dessine -------------
    Gauge {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.top
        anchors.bottomMargin: root.height * 0.55
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
