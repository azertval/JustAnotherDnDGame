import QtQuick
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

    // L'hôte de la surface de rendu. La surface elle-même (`GameViewport`) est un type C++ que
    // l'atelier ne connaît pas : c'est le jumeau qui la pose ici, à l'exécution. Dans Qt Design
    // Studio, l'hôte se dessine comme un aplat au parchemin -- ce que la surface efface de toute
    // façon tant qu'aucune scène n'est jouée.
    property alias viewportHost: viewportHost

    Rectangle {
        id: viewportHost

        anchors.fill: parent
        color: Tokens.background
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
