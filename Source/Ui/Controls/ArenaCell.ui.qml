import QtQuick
import Jadg.Ui

/*!
    Une case de la grille du Colisee, et ce qu'elle porte (LOT-50, habillage du 14 septembre 2026).

    Cinq pieces de la famille `arena` du cahier des assets, superposees dans l'ordre du sol vers le
    ciel : le sol ou le mur (`ui/arena/floor`, `ui/arena/wall`, des tuiles), la surbrillance de la
    case (`ui/arena/cell`, etats `reachable`, `ally`, `enemy`, `active`), le combattant
    (`ui/arena/unit`, membres `ally` et `enemy`), et pour un ennemi sa marque (`ui/arena/foe-mark`)
    a cote de sa jauge de vie -- la brique `Gauge`, comme le HUD. Le nom et les points de vie
    restent du texte : la charte v2 n'incruste aucune lettre dans une image.

    Tant qu'une piece n'est pas livree, la case dessine ce que l'ecran dessinait avant elle : les
    aplats de jetons de `ArenaForm`, un disque par combattant. Un combattant a terre est
    estompe, pas retire : sa case reste occupee pour le moteur.

    La zone sensible (`pointer`) est exposee au formulaire, qui decide ce que cliquer veut dire.
*/
Item {
    id: root

    /// Case infranchissable (`wall`).
    property bool wall: false

    /// Case ou le combattant actif peut finir son deplacement.
    property bool reachable: false

    /// Case du combattant dont c'est le tour.
    property bool active: false

    /// Camp de l'occupant : `allies`, `enemies`, ou vide.
    property string side: ""

    /// L'occupant est a terre.
    property bool down: false

    /// Points de vie de l'occupant, tels que l'ecran les ecrit (`30/30`), ou vide.
    property string hitPoints: ""

    /// Part des points de vie restants, de 0 a 1, pour la jauge d'un ennemi.
    property real hitPointsRatio: 1

    /// La zone sensible au pointeur, a brancher par le formulaire (`clicked`).
    property alias pointer: cellPointer

    readonly property bool occupied: root.side.length > 0
    readonly property bool ally: root.side === "allies"
    readonly property bool enemy: root.side === "enemies"

    /// L'etat de la surbrillance, du plus fort au plus faible ; vide quand la case est nue.
    readonly property string highlight: root.active ? "active"
                                        : (root.ally ? "ally"
                                        : (root.enemy ? "enemy"
                                        : (root.reachable ? "reachable" : "")))

    implicitWidth: 64 * Tokens.uiScale
    implicitHeight: 64 * Tokens.uiScale

    // --- Le sol, ou le mur -------------------------------------------------------------------
    Rectangle {
        anchors.fill: parent
        visible: !floorArt.delivered && !wallArt.delivered
        color: root.wall ? Tokens.frameEdge : (root.reachable ? Tokens.info : Tokens.surface)
        border.color: root.active ? Tokens.goldLight : Tokens.border
        border.width: (root.active ? 2 : 1) * Tokens.strokeWidth
    }

    TileArt {
        id: floorArt

        anchors.fill: parent
        key: root.wall ? "" : "ui/arena/floor"
    }

    TileArt {
        id: wallArt

        anchors.fill: parent
        key: root.wall ? "ui/arena/wall" : ""
    }

    // Un filet sombre entre les tuiles, pour que la grille reste lisible une fois le sol peint.
    Rectangle {
        anchors.fill: parent
        visible: floorArt.delivered || wallArt.delivered
        color: "transparent"
        border.color: Tokens.frameEdge
        border.width: 1
        opacity: 0.35
    }

    // --- La surbrillance -----------------------------------------------------------------------
    FixedArt {
        anchors.fill: parent
        key: root.highlight.length > 0 ? "ui/arena/cell/" + root.highlight : ""
    }

    // --- Le combattant -------------------------------------------------------------------------
    Rectangle {
        anchors.centerIn: parent
        width: parent.width * 0.7
        height: width
        radius: width / 2
        visible: root.occupied && !unitArt.delivered
        color: root.ally ? Tokens.textAlly : Tokens.textEnemy
        opacity: root.down ? 0.35 : 1
    }

    FixedArt {
        id: unitArt

        anchors.fill: parent
        anchors.margins: parent.width * 0.06
        key: root.occupied ? "ui/arena/unit/" + (root.ally ? "ally" : "enemy") : ""
        opacity: root.down ? 0.35 : 1
    }

    // --- La marque et la jauge d'un ennemi, au-dessus de sa case -----------------------------
    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.top
        anchors.bottomMargin: Tokens.strokeWidth
        spacing: Tokens.gapSmall / 2
        visible: root.enemy && unitArt.delivered

        Item {
            width: Math.max(16 * Tokens.uiScale, root.width * 0.18)
            height: width
            anchors.verticalCenter: parent.verticalCenter

            Rectangle {
                anchors.fill: parent
                visible: !markArt.delivered
                radius: width / 2
                color: Tokens.gem
                border.color: Tokens.panelEdge
                border.width: 1
            }

            FixedArt {
                id: markArt

                anchors.fill: parent
                key: "ui/arena/foe-mark"
            }
        }

        Gauge {
            anchors.verticalCenter: parent.verticalCenter
            width: root.width * 0.55
            height: Math.max(6, root.width * 0.09)
            kind: "health"
            value: root.hitPointsRatio
        }
    }

    // --- Les points de vie ---------------------------------------------------------------------
    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Tokens.strokeWidth
        visible: root.occupied
        text: root.hitPoints
        color: Tokens.textOnPanel
        font.family: Tokens.bodyFamily
        font.pixelSize: Math.max(8, root.width * 0.22)
        style: unitArt.delivered ? Text.Outline : Text.Normal
        styleColor: Tokens.panel
    }

    MouseArea {
        id: cellPointer

        anchors.fill: parent
    }
}
