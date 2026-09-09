import QtQuick
import Jadg.Ui

/*!
    Une ligne de fiche : un libellé à gauche, sa valeur à droite (LOT-86).

    C'est la brique la plus répétée des écrans du RPG — caractéristiques, jets de sauvegarde,
    compétences, constantes de combat. L'écrire une fois est ce qui garantit qu'elles se
    ressemblent toutes : sans elle, la vingtième ligne finit toujours par avoir une marge de
    travers que personne ne remarque avant longtemps.

    `.ui.qml` : déclaratif seulement, ouvrable dans Qt Design Studio.
*/
Item {
    id: root

    property string label: "Libellé"
    property string value: "—"

    implicitHeight: Math.max(labelText.implicitHeight, valueText.implicitHeight)
    implicitWidth: labelText.implicitWidth + valueText.implicitWidth + Tokens.spaceLarge

    Text {
        id: labelText
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        text: root.label
        color: Tokens.textMuted
        font.family: Tokens.bodyFamily
        font.pixelSize: Tokens.body
    }

    // Le filet qui relie le libellé à sa valeur. Il occupe l'espace restant plutôt que d'être
    // dimensionné à la main : une fiche dont les lignes n'ont pas toutes la même longueur de
    // libellé garde ainsi ses valeurs alignées.
    Rectangle {
        anchors.left: labelText.right
        anchors.right: valueText.left
        anchors.leftMargin: Tokens.spaceSmall
        anchors.rightMargin: Tokens.spaceSmall
        anchors.verticalCenter: parent.verticalCenter
        height: Tokens.frameThickness
        color: Tokens.frameOrnament
        opacity: 0.4
    }

    Text {
        id: valueText
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        text: root.value
        color: Tokens.text
        font.family: Tokens.bodyFamily
        font.pixelSize: Tokens.body
    }
}
