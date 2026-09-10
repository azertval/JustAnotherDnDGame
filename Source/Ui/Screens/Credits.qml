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

    Keys.onEscapePressed: ScreenRouter.closeCredits()
}
