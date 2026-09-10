import QtQuick
import Jadg.Ui

/*!
    Options -- CABLAGE, cote developpeur (LOT-86).

    Les reglages pointent vers l'ancre : ils ne sont PAS branches, et EX-IHM-083 exige qu'un
    reglage expose atteigne le moteur. Le jeu n'a pas encore de viewport a regler.
*/
OptionsForm {
    id: root

    pending: true
    focus: true

    fullscreen: PendingData.value("options.display.fullscreen")
    vsync: PendingData.value("options.display.vsync")
    diagnostics: PendingData.value("options.display.diagnostics")
    volume: PendingData.value("options.audio.volume")
    language: PendingData.value("options.language")
    gamepad: PendingData.value("options.controls.gamepad")

    Keys.onEscapePressed: ScreenRouter.closeOptions()
}
