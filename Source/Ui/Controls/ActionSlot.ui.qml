import QtQuick
import Jadg.Ui

/*!
    Case de la barre d'actions du HUD, et son raccourci (LOT-87, T4.1).

    Piece `ui/slot/action` (etats `normal`, `active`, `disabled`), affichee a 64 px. Sous la case,
    la touche qui la declenche : un combat se joue entierement au clavier, et le raccourci s'ecrit
    donc a l'ecran plutot que de s'apprendre dans les options.

    Tant qu'une action n'a pas d'icone, son nom s'ecrit dans la case -- comme `ItemSlot` pour un
    objet : une case pleine et une case vide ne se distingueraient pas sinon.
*/
Item {
    id: root

    /// Nom de l'action, ecrit dans la case tant que son icone manque.
    property string label: ""

    /// Cle de cahier de l'icone de l'action, ou vide.
    property string iconKey: ""

    /// Charges ou munitions restantes (`3`), ou vide.
    property string quantity: ""

    /// Touche qui declenche l'action (`1`).
    property string shortcut: ""

    /// Action choisie pour ce tour.
    property bool active: false

    /// Etat impose (`normal`, `active`, `disabled`), ou vide pour le deduire.
    property string forcedState: ""

    readonly property string visualState: root.forcedState.length > 0 ? root.forcedState
                                          : (!root.enabled ? "disabled" : (root.active ? "active" : "normal"))
    readonly property bool lit: root.visualState === "active"
    readonly property bool disabledLook: root.visualState === "disabled"

    implicitWidth: 64 * Tokens.uiScale
    implicitHeight: 96 * Tokens.uiScale

    Item {
        id: slot

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: width

        Rectangle {
            anchors.fill: parent
            visible: !art.delivered
            color: root.lit ? Tokens.panelRaised : Tokens.panel
            border.color: root.disabledLook ? Tokens.textOnPanelMuted : (root.lit ? Tokens.goldLight : Tokens.panelEdge)
            border.width: (root.lit ? 2 : 1) * Tokens.strokeWidth
        }

        FixedArt {
            id: art

            anchors.fill: parent
            key: "ui/slot/action/" + root.visualState
        }

        FixedArt {
            id: icon

            anchors.fill: parent
            anchors.margins: parent.width * 0.12
            key: root.iconKey
            opacity: root.disabledLook ? 0.5 : 1.0
        }

        Text {
            anchors.fill: parent
            anchors.margins: Tokens.strokeWidth * 2
            visible: !icon.delivered
            text: root.label
            color: root.disabledLook ? Tokens.textOnPanelMuted : Tokens.textOnPanel
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontCaption
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
            maximumLineCount: 3
            elide: Text.ElideRight
        }

        Text {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.rightMargin: Tokens.strokeWidth * 2
            visible: root.quantity.length > 0
            text: root.quantity
            color: Tokens.goldLight
            font.family: Tokens.titleFamily
            font.pixelSize: Tokens.fontCaption
            font.weight: Font.DemiBold
        }
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: slot.bottom
        anchors.topMargin: Tokens.gapSmall
        text: root.shortcut
        color: root.lit ? Tokens.goldLight : Tokens.textOnPanel
        font.family: Tokens.titleFamily
        font.pixelSize: Tokens.fontCaption
        font.weight: Font.DemiBold
    }
}
