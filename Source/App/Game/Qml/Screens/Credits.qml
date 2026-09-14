import QtQuick
import Jadg.Ui
import Jadg.Runtime

/*!
    Credits -- CABLAGE, cote developpeur (LOT-86, LOT-87 T3.3).

    Les attributions sont des donnees du projet (`Source/Elements/Credits/credits.json`), lues par
    deux `CreditsModel`, un par colonne. Leur langue suit le reglage : changer de langue dans les
    options relit les titres et les roles.

    `Echap` et le bouton Retour ramenent au menu par le routeur.
*/
CreditsForm {
    id: root

    focus: true

    leftSections: leftColumn.sections
    rightSections: rightColumn.sections
    // Posee par Main.cpp depuis core::Engine::version(), comme au menu principal.
    version: Qt.application.version

    CreditsModel {
        id: leftColumn

        column: 0
        language: OptionsModel.language
    }

    CreditsModel {
        id: rightColumn

        column: 1
        language: OptionsModel.language
    }

    Keys.onEscapePressed: ScreenRouter.closeCredits()

    Connections {
        target: root.backButton
        function onClicked() { ScreenRouter.closeCredits() }
    }
}
