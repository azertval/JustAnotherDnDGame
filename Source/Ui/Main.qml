import QtQuick
import QtQuick.Window
import Jadg.Ui

/*!
    Fenetre du jeu -- cote DEVELOPPEUR (LOT-86).

    Ce fichier cable ; il ne decrit pas d'apparence. Tout ce qui se voit vit dans les `.ui.qml` de
    `Screens/` et `Controls/`, que Qt Design Studio ouvre et reenregistre sans les casser.

    Sa seule responsabilite visuelle est de poser le facteur d'agrandissement : entier, derive de
    la hauteur de la fenetre, borne par la zone d'affichage. Ce calcul reste en C++ (`IdentityScale`,
    couvert par ses tests) -- il n'a rien d'une decision d'apparence.
*/
Window {
    id: root

    width: 1280
    height: 720
    visible: true
    color: Tokens.background

    // Facteur entier borne a [1, 3], depuis une hauteur de reference de 360 px. Repris tel quel de
    // `hmi::identityScaleFor` en attendant que la vue-modele l'expose (etape 3) : la formule est
    // ici temporairement, jamais l'apparence.
    onHeightChanged: Tokens.scale = Math.max(1, Math.min(3, Math.floor(root.height / 360)))

    Text {
        anchors.centerIn: parent
        text: "JustAnotherDnDGame"
        color: Tokens.text
        font.family: Tokens.titleFamily
        font.pixelSize: Tokens.screenTitle
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.verticalCenter
        anchors.topMargin: Tokens.spaceExtraLarge
        text: "socle Qt Quick -- facteur " + Tokens.scale
        color: Tokens.textMuted
        font.family: Tokens.bodyFamily
        font.pixelSize: Tokens.body
    }
}
