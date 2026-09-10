import QtQuick
import QtQuick.Layouts
import Jadg.Ui

/*!
    Vue de jeu -- FORMULAIRE, cote conception (LOT-86).

    La surface de rendu occupe l'ecran ; ce qui se pose PAR-DESSUS est de l'interface ordinaire,
    et c'est tout l'interet du portage : un recouvrement n'a plus a etre une fenetre native.

    Aucune scene n'y est encore dessinee -- Source/Elements/Levels/ est vide par construction, et
    le contenu du RPG arrive avec un lot ulterieur. La surface efface au parchemin, ce qui suffit a
    verifier que la plomberie tient.
*/
Item {
    id: root

    property string status: "—"

    GameViewport {
        anchors.fill: parent
        clearColor: Tokens.background
    }

    // Le rappel se pose PAR-DESSUS la surface de rendu, comme un enfant ordinaire. C'est ce que le
    // portage sur une texture d'appui rend possible : plus aucun recouvrement ne depend d'un
    // empilement de fenetres natives.
    Text {
        anchors.centerIn: parent
        text: root.status
        color: Tokens.textMuted
        font.family: Tokens.bodyFamily
        font.pixelSize: Tokens.body
    }
}
