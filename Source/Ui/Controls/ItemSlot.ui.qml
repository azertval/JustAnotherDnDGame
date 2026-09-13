import QtQuick
import Jadg.Ui

/*!
    Emplacement d'objet : grille d'inventaire, equipement, butin (LOT-87, T2.7).

    Trois pieces du cahier superposees : la case (`ui/slot/item`, etats `empty`, `hover`,
    `selected`, `equipped`), le liseré de rarete par-dessus (`ui/slot/rarity/<rarete>`) et la
    pastille de quantite (`ui/slot/quantity-pip`). L'icone de l'objet n'est pas du cahier : ce sont
    les marqueurs du `LOT-39`, passes par `iconSource`.

    Comme `MenuEntry`, l'emplacement expose la zone du pointeur (`pointer`) : c'est le jumeau qui
    decide ce que survoler et cliquer veulent dire. L'etat `hover` s'en deduit, `forcedState`
    l'impose.
*/
Item {
    id: root

    property url iconSource: ""

    /// Rarete de l'objet : `common`, `uncommon`, `rare`, `very-rare`, `legendary`, ou vide.
    property string rarity: ""

    /// Quantite empilee ; la pastille ne s'affiche qu'au-dela d'un exemplaire.
    property int quantity: 0

    property bool selected: false
    property bool equipped: false

    /// Etat impose (`empty`, `hover`, `selected`, `equipped`), ou vide pour le deduire.
    property string forcedState: ""

    /// La zone sensible au pointeur, a brancher par le jumeau (`entered`, `clicked`).
    property alias pointer: slotPointer

    readonly property string visualState: root.forcedState.length > 0 ? root.forcedState
                                          : (root.selected ? "selected"
                                          : (root.equipped ? "equipped"
                                          : (slotPointer.containsMouse ? "hover" : "empty")))

    implicitWidth: 96 * Tokens.uiScale
    implicitHeight: 96 * Tokens.uiScale

    Rectangle {
        anchors.fill: parent
        visible: !art.delivered
        color: Tokens.panel
        border.color: root.visualState === "selected" || root.visualState === "hover" ? Tokens.goldLight
                      : (root.visualState === "equipped" ? Tokens.panelEdge : Tokens.accent)
        border.width: root.visualState === "selected" ? 2 * Tokens.strokeWidth : Tokens.strokeWidth
    }

    FixedArt {
        id: art

        anchors.fill: parent
        key: "ui/slot/item/" + root.visualState
    }

    Image {
        anchors.fill: parent
        anchors.margins: parent.width * 0.16
        source: root.iconSource
        fillMode: Image.PreserveAspectFit
        smooth: true
        mipmap: true
    }

    // Le liseré de rarete : sans image livree, un second filet en retrait, aux couleurs de l'or --
    // les teintes de rarete ne sont pas des jetons, et le cahier ne leur donne pas d'aplat.
    Item {
        anchors.fill: parent
        visible: root.rarity.length > 0

        Rectangle {
            anchors.fill: parent
            anchors.margins: 3 * Tokens.strokeWidth
            visible: !rarityArt.delivered
            color: "transparent"
            border.color: Tokens.panelEdge
            border.width: Tokens.strokeWidth
        }

        FixedArt {
            id: rarityArt

            anchors.fill: parent
            key: "ui/slot/rarity/" + root.rarity
        }
    }

    Item {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: Tokens.strokeWidth
        width: Math.max(40 * Tokens.uiScale, quantityLabel.implicitWidth + 16 * Tokens.uiScale)
        height: 28 * Tokens.uiScale
        visible: root.quantity > 1

        Rectangle {
            anchors.fill: parent
            visible: !pipArt.delivered
            color: Tokens.panel
            border.color: Tokens.panelEdge
            border.width: Tokens.strokeWidth
        }

        NinePatchArt {
            id: pipArt

            anchors.fill: parent
            key: "ui/slot/quantity-pip"
        }

        Text {
            id: quantityLabel

            anchors.centerIn: parent
            text: root.quantity
            color: Tokens.textOnPanel
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontCaption
        }
    }

    MouseArea {
        id: slotPointer

        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
    }
}
