import QtQuick
import QtQuick.Window
import Jadg.Ui
import Jadg.Runtime

/*!
    Fenêtre du jeu -- côté DÉVELOPPEUR (LOT-86).

    Ce fichier câble ; il ne décrit pas d'apparence. Tout ce qui se voit vit dans les `.ui.qml` de
    `Screens/` et `Controls/`, que Qt Design Studio ouvre et réenregistre sans les casser.
*/
Window {
    id: root

    width: 1280
    height: 720
    visible: true
    visibility: OptionsModel.fullscreen ? Window.FullScreen : Window.Windowed
    color: Tokens.background
    title: "JustAnotherDnDGame"

    palette.window: Tokens.surface
    palette.windowText: Tokens.text
    palette.base: Tokens.surfaceAlt
    palette.text: Tokens.text
    palette.button: Tokens.surfaceAlt
    palette.buttonText: Tokens.text
    palette.highlight: Tokens.accent
    palette.highlightedText: Tokens.text
    palette.accent: Tokens.accent
    palette.mid: Tokens.border
    palette.dark: Tokens.frameEdge
    palette.light: Tokens.surfaceAlt
    palette.placeholderText: Tokens.textMuted

    onHeightChanged: Tokens.scale = Math.max(1, Math.min(3, Math.floor(root.height / 360)))

    property string startScreen: ""

    ScreenStack {
        anchors.fill: parent
        forcedScreen: root.startScreen
    }

    // La mesure de cadence appartient au runtime, pas au formulaire editable.
    FrameAnimation {
        id: diagnosticsFrame
        running: OptionsModel.diagnostics
    }

    DiagnosticsOverlay {
        id: diagnosticsOverlay
        anchors.fill: parent
        visible: OptionsModel.diagnostics
        readoutText: (diagnosticsFrame.smoothFrameTime > 0
                      ? Math.round(1 / diagnosticsFrame.smoothFrameTime)
                      : 0)
                      + " ips\n" + root.width + "x" + root.height + " · x" + Tokens.scale
    }
}
