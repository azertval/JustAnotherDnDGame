import QtQuick
import QtQuick.Effects
import Jadg.Ui

/*!
    Cadre de portrait : rond sur la fiche, carre dans la compagnie, HUD avec son niveau
    (LOT-87, T2.7).

    Trois pieces du cahier -- `ui/medallion/portrait-round` (etats `filled`, `empty`),
    `portrait-square` (`normal`, `active`), `portrait-hud` -- et la pastille de niveau
    `ui/medallion/level-pip`. Le portrait lui-meme n'est pas dans le cahier : ce sont les jetons du
    corpus (`LOT-38`), passes par `source`, et decoupes au rond de leur cadre par un masque.
*/
Item {
    id: root

    /// `round` (fiche), `square` (compagnie, dialogue) ou `hud`.
    property string shape: "round"

    /// L'image du portrait ; vide, le cadre prend son etat `empty`.
    property url source: ""

    /// Portrait carre du membre actif.
    property bool active: false

    /// Niveau affiche dans la pastille du cadre HUD ; vide, la pastille est retiree.
    property string level: ""

    /// Cote, a 1080p : 260 (rond), 220 (carre), 180 (HUD) -- les tailles d'affichage du cahier.
    property real size: (root.shape === "square" ? 220 : (root.shape === "hud" ? 180 : 260)) * Tokens.uiScale

    readonly property bool square: root.shape === "square"
    readonly property bool filled: root.source.toString().length > 0

    readonly property string key: root.square ? (root.active ? "ui/medallion/portrait-square/active"
                                                             : "ui/medallion/portrait-square/normal")
                                  : (root.shape === "hud" ? "ui/medallion/portrait-hud"
                                  : (root.filled ? "ui/medallion/portrait-round/filled"
                                                 : "ui/medallion/portrait-round/empty"))

    implicitWidth: root.size
    implicitHeight: root.size

    // Le fond du portrait : ce qui se voit sous un jeton aux angles transparents, ou sans portrait.
    // Sa marge suit l'ouverture de chaque piece livree (mesuree : demi-cote 0,36 pour le carre,
    // rayon 0,28 pour le rond, 0,25 pour le HUD), un peu en deca pour glisser sous l'anneau : une
    // marge unique debordait de l'anneau du HUD et des membres de la compagnie.
    Rectangle {
        id: portraitMask

        anchors.fill: parent
        anchors.margins: root.size * (root.square ? 0.12 : (root.shape === "hud" ? 0.22 : 0.19))
        radius: root.square ? 0 : width / 2
        color: root.square || root.shape === "hud" ? Tokens.panel : Tokens.background
        layer.enabled: true
    }

    Image {
        id: portrait

        anchors.fill: portraitMask
        visible: false
        source: root.source
        fillMode: Image.PreserveAspectCrop
        smooth: true
        mipmap: true
    }

    MultiEffect {
        anchors.fill: portraitMask
        visible: root.filled
        source: portrait
        maskEnabled: true
        maskSource: portraitMask
    }

    Rectangle {
        anchors.fill: parent
        visible: !art.delivered
        radius: root.square ? 0 : width / 2
        color: "transparent"
        border.color: root.square ? (root.active ? Tokens.goldLight : Tokens.panelEdge)
                                  : (root.shape === "hud" ? Tokens.panelEdge : Tokens.accent)
        border.width: 4 * Tokens.strokeWidth
    }

    FixedArt {
        id: art

        anchors.fill: parent
        key: root.key
    }

    Item {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        width: 48 * Tokens.uiScale
        height: 48 * Tokens.uiScale
        visible: root.shape === "hud" && root.level.length > 0

        Rectangle {
            anchors.fill: parent
            visible: !pipArt.delivered
            radius: width / 2
            color: Tokens.panel
            border.color: Tokens.panelEdge
            border.width: Tokens.strokeWidth
        }

        FixedArt {
            id: pipArt

            anchors.fill: parent
            key: "ui/medallion/level-pip"
        }

        Text {
            anchors.centerIn: parent
            text: root.level
            color: Tokens.textOnPanel
            font.family: Tokens.titleFamily
            font.pixelSize: Tokens.fontCaption
            font.weight: Font.DemiBold
        }
    }
}
