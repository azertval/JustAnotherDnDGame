import QtQuick
import Jadg.Ui

/*!
    Bandeau de section : l'intertitre d'un panneau (LOT-87, T2.7).

    Deux pieces du cahier, 56 px de haut a 1080p : `ui/plate/section-banner`, plaque noire centree
    posee sur le parchemin (competences, statistiques), et `ui/plate/section-bar`, barre sombre
    pleine largeur dont l'ornement est a gauche et le libelle aligne dessus (sections des options).
*/
Item {
    id: root

    property string text: "Section"

    /// `parchment` (bandeau centre sur une fiche) ou `dark` (barre de section d'un panneau sombre).
    property string material: "parchment"

    readonly property bool bar: root.material === "dark"
    readonly property string key: root.bar ? "ui/plate/section-bar" : "ui/plate/section-banner"

    implicitHeight: 56 * Tokens.uiScale
    implicitWidth: sectionLabel.implicitWidth + (root.bar ? 144 : 128) * Tokens.uiScale

    Rectangle {
        anchors.fill: parent
        visible: !art.delivered
        color: Tokens.panel
        border.color: root.bar ? Tokens.panelEdge : Tokens.accent
        border.width: Tokens.strokeWidth
    }

    NinePatchArt {
        id: art

        anchors.fill: parent
        key: root.key
    }

    Text {
        id: sectionLabel

        // Les marges sont celles des extremites ornees du cahier : le libelle ne mord jamais dessus.
        anchors.fill: parent
        anchors.leftMargin: (root.bar ? 48 : 64) * Tokens.uiScale
        anchors.rightMargin: (root.bar ? 96 : 64) * Tokens.uiScale
        horizontalAlignment: root.bar ? Text.AlignLeft : Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        text: root.text
        color: root.bar ? Tokens.goldLight : Tokens.textOnPanel
        font.family: Tokens.titleFamily
        font.pixelSize: Tokens.fontSectionTitle
    }
}
