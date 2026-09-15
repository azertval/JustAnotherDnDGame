import QtQuick
import QtQuick.Controls
import Jadg.Ui

/*!
    Barre de defilement de la charte v2 : un panneau dont le contenu depasse (credits).

    Un `ScrollBar` de Qt restyle, pose sur un `Flickable` par `ScrollBar.vertical` : le glisser, la
    molette et `increase()` / `decrease()` viennent avec. Aucune piece du cahier : un rail `panel`
    borde d'or et une poignee `panelEdge`, qui s'eclaire au survol. Invisible tant que tout le
    contenu tient dans la vue -- une barre sans rien a faire defiler dirait le contraire.
*/
ScrollBar {
    id: root

    implicitWidth: 12 * Tokens.uiScale
    padding: Tokens.strokeWidth
    minimumSize: 0.1
    stepSize: 0.1
    policy: root.size < 1.0 ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff

    background: Rectangle {
        color: Tokens.panel
        border.color: Tokens.panelEdge
        border.width: Tokens.strokeWidth
        radius: width / 2
    }

    contentItem: Rectangle {
        implicitWidth: 8 * Tokens.uiScale
        radius: width / 2
        color: root.hovered || root.pressed ? Tokens.goldLight : Tokens.panelEdge
    }
}
