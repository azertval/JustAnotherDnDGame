import QtQuick
import QtQuick.Window
import Jadg.Ui

/*!
    Fenêtre du jeu -- côté DÉVELOPPEUR (LOT-86).

    Ce fichier câble ; il ne décrit pas d'apparence. Tout ce qui se voit vit dans les `.ui.qml` de
    `Screens/` et `Controls/`, que Qt Design Studio ouvre et réenregistre sans les casser.

    Sa seule responsabilité visuelle est de poser le facteur d'agrandissement : entier, dérivé de
    la hauteur de la fenêtre. Ce calcul reste en C++ (`IdentityScale`, couvert par ses tests) -- il
    n'a rien d'une décision d'apparence.
*/
Window {
    id: root

    width: 1280
    height: 720
    visible: true
    color: Tokens.background
    title: "JustAnotherDnDGame"

    // Facteur entier borné à [1, 3], depuis une hauteur de référence de 360 px. Repris tel quel de
    // `hmi::identityScaleFor` en attendant que la vue-modèle l'expose : la formule est ici
    // temporairement, jamais l'apparence.
    onHeightChanged: Tokens.scale = Math.max(1, Math.min(3, Math.floor(root.height / 360)))

    CharacterSheet {
        anchors.fill: parent
    }
}
