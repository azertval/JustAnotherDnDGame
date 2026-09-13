import QtQuick
import Jadg.Ui
import Jadg.Runtime

/*!
    Options -- CABLAGE, cote developpeur (LOT-86).

    Les reglages sont REELS. `EX-IHM-083` l'exige : un reglage expose doit atteindre le moteur, et
    une case a cocher qui ne fait rien est pire qu'une case absente -- elle fait croire a un
    reglage, et l'utilisateur cherche ensuite pourquoi il n'a pas d'effet.

    Ce fichier ne fait que relier les controles a `OptionsModel`, qui persiste et publie. C'est
    l'application qui branche ensuite chaque signal sur ce qu'il doit atteindre : le moteur audio,
    la fenetre, les traducteurs. Le faire ici obligerait la conception a connaitre le moteur.
*/
OptionsForm {
    id: root

    focus: true
    // Le pied devient un retour cliquable (voir `RpgScreenFrame`).
    backAvailable: true
    logsEnabled: OptionsModel.logsAvailable
    logsMessage: root.lastLogsMessage

    property string lastLogsMessage: ""

    Component.onCompleted: {
        fullscreenSwitch.checked = OptionsModel.fullscreen
        vsyncSwitch.checked = OptionsModel.vsync
        diagnosticsSwitch.checked = OptionsModel.diagnostics
        volumeSlider.value = OptionsModel.volume
        // Le NOM s'affiche, le CODE se pose : les deux listes ont le meme ordre, et
        // c'est le rang choisi qui fait le lien entre elles.
        languageBox.model = OptionsModel.languageNames
        languageBox.currentIndex = OptionsModel.languages.indexOf(OptionsModel.language)
    }

    Connections {
        target: root.fullscreenSwitch
        function onToggled() { OptionsModel.fullscreen = root.fullscreenSwitch.checked }
    }
    Connections {
        target: root.vsyncSwitch
        function onToggled() { OptionsModel.vsync = root.vsyncSwitch.checked }
    }
    Connections {
        target: root.diagnosticsSwitch
        function onToggled() { OptionsModel.diagnostics = root.diagnosticsSwitch.checked }
    }
    Connections {
        target: root.volumeSlider
        // `moved` et non `valueChanged` : le second se declenche aussi quand le code pose la
        // valeur initiale, ce qui reecrirait le reglage a chaque ouverture de l'ecran.
        function onMoved() { OptionsModel.volume = Math.round(root.volumeSlider.value) }
    }
    Connections {
        target: root.languageBox
        function onActivated(index) { OptionsModel.language = OptionsModel.languages[index] }
    }
    Connections {
        target: root.saveLogsButton
        function onClicked() { root.lastLogsMessage = OptionsModel.saveLogs() }
    }

    Keys.onEscapePressed: ScreenRouter.closeOptions()

    Connections {
        target: root.backPointer
        function onClicked() { ScreenRouter.closeOptions() }
    }
}
