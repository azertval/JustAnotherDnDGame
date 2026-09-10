import QtQuick
import Jadg.Ui

/*!
    Credits -- CABLAGE, cote developpeur (LOT-86).

    Aucune donnee a brancher : les attributions sont ecrites dans le formulaire, parce que les
    licences des assets et des bibliotheques EXIGENT d'etre citees -- ce sont des donnees du
    projet, pas un contenu de partie.
*/
CreditsForm {
    id: root

    focus: true

    // Le pied devient un retour cliquable : l'écran s'ouvre à la souris depuis le menu, il doit
    // pouvoir se fermer de la même main.
    backAvailable: true

    Keys.onEscapePressed: ScreenRouter.closeCredits()

    Connections {
        target: root.backPointer
        function onClicked() { ScreenRouter.closeCredits() }
    }
}
