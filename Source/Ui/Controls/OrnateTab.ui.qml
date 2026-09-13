import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Jadg.Ui

/*!
    Onglet de la charte v2 (LOT-87, T2.7).

    Un `TabButton` de Qt restyle : il se range dans un `TabBar` (ou un `ButtonGroup`) et l'onglet
    actif est celui qui est `checked`. Deux pieces du cahier :

    - `ui/tab/ribbon` (`dark`) : ruban des panneaux sombres -- colonne des options, onglets de la
      compagnie. Etats `active`, `normal`, `disabled`.
    - `ui/tab/segment` (`parchment`) : segment pose sur une fiche -- filtres de l'inventaire. Etats
      `active`, `normal` ; desactive, il garde l'image `normal` et s'estompe.

    `forcedState` impose l'etat, comme sur `OrnateButton`.
*/
TabButton {
    id: root

    /// `dark` (ruban) ou `parchment` (segment).
    property string material: "dark"

    /// Cle de cahier de l'icone posee a gauche du libelle, ou vide.
    property string iconKey: ""

    /// Etat impose (`active`, `normal`, `disabled`), ou vide pour le deduire.
    property string forcedState: ""

    readonly property bool dark: root.material !== "parchment"

    readonly property string visualState: root.forcedState.length > 0 ? root.forcedState
                                          : (!root.enabled ? "disabled" : (root.checked ? "active" : "normal"))

    readonly property bool active: root.visualState === "active"
    readonly property bool disabledLook: root.visualState === "disabled"

    // Le segment n'a pas d'image desactivee : il reprend `normal`.
    readonly property string key: (root.dark ? "ui/tab/ribbon/" : "ui/tab/segment/")
                                  + (!root.dark && root.disabledLook ? "normal" : root.visualState)

    implicitWidth: (root.dark ? 256 : 320) * Tokens.uiScale
    implicitHeight: 56 * Tokens.uiScale

    leftPadding: (root.dark ? 48 : 32) * Tokens.uiScale
    rightPadding: (root.dark ? 48 : 32) * Tokens.uiScale
    topPadding: 0
    bottomPadding: 0

    opacity: !root.dark && root.disabledLook ? 0.5 : 1.0

    background: Item {
        Rectangle {
            anchors.fill: parent
            visible: !art.delivered
            color: root.active ? Tokens.gem : (root.dark ? Tokens.panel : Tokens.surfaceAlt)
            border.color: root.dark ? (root.disabledLook ? Tokens.textOnPanelMuted
                                                         : (root.hovered ? Tokens.goldLight : Tokens.panelEdge))
                                    : Tokens.border
            border.width: Tokens.strokeWidth
        }

        NinePatchArt {
            id: art

            anchors.fill: parent
            key: root.key
        }
    }

    contentItem: RowLayout {
        spacing: Tokens.gapSmall

        Image {
            Layout.preferredWidth: 32 * Tokens.uiScale
            Layout.preferredHeight: 32 * Tokens.uiScale
            Layout.alignment: Qt.AlignVCenter
            visible: source.toString().length > 0
            source: Artwork.delivered[root.iconKey] ? Artwork.baseUrl + Artwork.delivered[root.iconKey].file : ""
            fillMode: Image.PreserveAspectFit
            smooth: true
            mipmap: true
        }

        Text {
            Layout.fillWidth: true
            Layout.fillHeight: true
            text: root.text
            // Sur le grenat et le ruban sombre, le texte clair ; sur le segment de parchemin, l'encre.
            color: root.disabledLook && root.dark ? Tokens.textOnPanelMuted
                   : (root.active || root.dark ? Tokens.textOnPanel : Tokens.text)
            font.family: Tokens.titleFamily
            font.pixelSize: Tokens.fontBody
            horizontalAlignment: root.dark ? Text.AlignLeft : Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }
}
