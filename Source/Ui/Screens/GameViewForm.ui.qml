import QtQuick
import Jadg.Ui

/*!
    Vue de jeu -- FORMULAIRE, cote conception (LOT-86, LOT-87 T4.1 ; maquette 01).

    La surface de rendu occupe l'ecran ; le cadre du HUD se pose PAR-DESSUS (`HudFrame`), en mode
    exploration : personnage actif, membres, boussole, mini-carte, quetes, jour et lieu, raccourcis
    de navigation. Ce qui n'appartient qu'au combat -- journal, barre d'actions, cible -- est dans
    `CombatHudForm`.

    Aucune scene n'y est encore dessinee -- Source/Elements/Levels/ est vide par construction, et
    le contenu du RPG arrive avec un lot ulterieur. La surface efface au parchemin, et le rappel
    `status` le dit au centre de l'ecran.
*/
HudFrame {
    id: root

    property string status: "—"

    mode: "exploration"

    // Le rappel se pose PAR-DESSUS la surface de rendu, comme un enfant ordinaire : plus aucun
    // recouvrement ne depend d'un empilement de fenetres natives.
    Text {
        anchors.centerIn: parent
        width: 720 * Tokens.uiScale
        text: root.status
        color: Tokens.textMuted
        font.family: Tokens.loreFamily
        font.italic: true
        font.pixelSize: Tokens.fontBody
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
    }
}
