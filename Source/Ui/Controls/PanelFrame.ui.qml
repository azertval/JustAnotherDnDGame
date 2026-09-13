import QtQuick
import Jadg.Ui

/*!
    Panneau de la charte v2 : sombre a filets d'or, ou parchemin a liseré (LOT-87, T2.7).

    Une brique pour les quatre cadres du cahier (`ui/frame/panel-dark`, `panel-parchment`,
    `subpanel-dark`, `subpanel-parchment`) : ce qui les distingue est une MATIERE et un RANG, deux
    proprietes, et non quatre composants qu'un ecran devrait choisir par leur nom. Le contenu se pose
    dedans comme dans un `Item`, en retrait de `padding`.

    Tant que l'image du cadre n'est pas livree, le panneau est l'aplat que le cahier prevoit :
    `panel` borde de `panelEdge`, ou `surface` borde de `border`.

    **Taille minimale.** Un 9-patch plus petit que ses deux marges superpose ses coins : 224 px a
    1080p pour le panneau sombre (marges de 112), 256 pour le parchemin (128), 96 et 80 pour les
    sous-panneaux. En dessous, c'est un sous-panneau qu'il faut, pas un panneau ecrase.
*/
Item {
    id: root

    /// `dark` (menu, options, credits, carte, HUD) ou `parchment` (fiches, inventaire, compagnie).
    property string material: "dark"

    /// Sous-panneau : une section posee dans un panneau, au cadre plus fin.
    property bool subpanel: false

    /// Panneau parchemin relie d'une bande de cuir grenat le long du bord gauche.
    property bool bound: false

    /// Sous-panneau parchemin en attente de contenu (etat `empty` du cahier).
    property bool empty: false

    /// Retrait du contenu depuis le bord du panneau.
    property real padding: root.subpanel ? Tokens.gapMedium : Tokens.gapLarge

    default property alias content: contentArea.data

    readonly property bool dark: root.material !== "parchment"

    readonly property string key: "ui/frame/" + (root.subpanel ? "subpanel-" : "panel-")
                                  + (root.dark ? "dark"
                                               : (root.subpanel ? (root.empty ? "parchment/empty" : "parchment/normal")
                                                                : (root.bound ? "parchment/bound" : "parchment/plain")))

    implicitWidth: 480 * Tokens.uiScale
    implicitHeight: 320 * Tokens.uiScale

    Rectangle {
        id: fallback

        anchors.fill: parent
        visible: !art.delivered
        color: root.dark ? (root.subpanel ? Tokens.panelRaised : Tokens.panel)
                         : (root.subpanel ? Tokens.surfaceAlt : Tokens.surface)
        border.color: root.dark ? Tokens.panelEdge : Tokens.border
        border.width: Tokens.strokeWidth
        opacity: root.empty ? 0.6 : 1.0

        // La reliure : une bande grenat, a l'interieur du trait gauche.
        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.margins: Tokens.strokeWidth
            width: Tokens.gapMedium
            visible: !root.dark && !root.subpanel && root.bound
            color: Tokens.gem
        }
    }

    NinePatchArt {
        id: art

        anchors.fill: parent
        key: root.key
        horizontalTileMode: BorderImage.Repeat
        verticalTileMode: BorderImage.Repeat
    }

    Item {
        id: contentArea

        anchors.fill: parent
        anchors.margins: root.padding
    }
}
