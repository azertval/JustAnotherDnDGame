// Le delegue et la liste ouverte sont des composants imbriques qui lisent `root` : sans ce pragma,
// cet acces a un identifiant exterieur n'est pas garanti, et `qmllint` le signale.
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import Jadg.Ui

/*!
    Liste deroulante de la charte v2 : langue, definition, qualite (LOT-87, T2.7).

    Un `ComboBox` de Qt restyle : `model`, `textRole`, `currentIndex` et la navigation au clavier
    viennent avec. Deux pieces du cahier : le champ (`ui/control/combo`, 48 px de haut, etats
    `normal`, `hover`, `open`, `disabled` -- la fleche est dans sa marge droite de 56 px) et la
    liste ouverte (`ui/control/combo-popup`).
*/
ComboBox {
    id: root

    /// Etat impose (`normal`, `hover`, `open`, `disabled`), ou vide pour le deduire.
    property string forcedState: ""

    readonly property string visualState: root.forcedState.length > 0 ? root.forcedState
                                          : (!root.enabled ? "disabled"
                                          : (root.popup.visible ? "open" : (root.hovered ? "hover" : "normal")))

    readonly property bool disabledLook: root.visualState === "disabled"

    implicitWidth: 320 * Tokens.uiScale
    implicitHeight: 48 * Tokens.uiScale

    leftPadding: 24 * Tokens.uiScale
    rightPadding: 56 * Tokens.uiScale

    background: Item {
        Rectangle {
            anchors.fill: parent
            visible: !fieldArt.delivered
            color: Tokens.panel
            border.color: root.disabledLook ? Tokens.textOnPanelMuted
                          : (root.visualState === "normal" ? Tokens.panelEdge : Tokens.goldLight)
            border.width: Tokens.strokeWidth
        }

        NinePatchArt {
            id: fieldArt

            anchors.fill: parent
            key: "ui/control/combo/" + root.visualState
        }
    }

    // La fleche du repli seulement : la piece livree porte la sienne dans sa marge droite.
    indicator: Text {
        x: root.width - root.rightPadding + (root.rightPadding - width) / 2
        y: (root.height - height) / 2
        visible: !fieldArt.delivered
        text: root.visualState === "open" ? "▴" : "▾"
        color: root.disabledLook ? Tokens.textOnPanelMuted : Tokens.panelEdge
        font.family: Tokens.bodyFamily
        font.pixelSize: Tokens.fontBody
    }

    contentItem: Text {
        text: root.displayText
        color: root.disabledLook ? Tokens.textOnPanelMuted : Tokens.textOnPanel
        font.family: Tokens.bodyFamily
        font.pixelSize: Tokens.fontBody
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    delegate: ItemDelegate {
        id: option

        // Proprietes requises : le modele les injecte, et `qmllint` sait d'ou elles viennent.
        required property var model
        required property int index

        width: root.popup.availableWidth
        height: 40 * Tokens.uiScale
        leftPadding: 16 * Tokens.uiScale
        highlighted: root.highlightedIndex === option.index
        text: root.textRole.length > 0 ? option.model[root.textRole] : option.model.modelData

        background: Rectangle {
            color: Tokens.gem
            visible: option.highlighted
        }

        contentItem: Text {
            text: option.text
            color: Tokens.textOnPanel
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontBody
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    popup: Popup {
        y: root.height
        width: root.width
        implicitHeight: contentItem.implicitHeight + topPadding + bottomPadding
        topPadding: 16 * Tokens.uiScale
        bottomPadding: 24 * Tokens.uiScale
        leftPadding: 8 * Tokens.uiScale
        rightPadding: 8 * Tokens.uiScale

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: root.popup.visible ? root.delegateModel : null
            currentIndex: root.highlightedIndex
        }

        background: Item {
            Rectangle {
                anchors.fill: parent
                visible: !popupArt.delivered
                color: Tokens.panelRaised
                border.color: Tokens.panelEdge
                border.width: Tokens.strokeWidth
            }

            NinePatchArt {
                id: popupArt

                anchors.fill: parent
                key: "ui/control/combo-popup"
                verticalTileMode: BorderImage.Repeat
            }
        }
    }
}
