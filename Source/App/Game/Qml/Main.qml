import QtQuick
import QtQuick.Window
import Jadg.Ui
import Jadg.Runtime
import Jadg.App

/*!
    Fenêtre du jeu -- côté DÉVELOPPEUR (LOT-86).

    Ce fichier câble ; il ne décrit pas d'apparence. Tout ce qui se voit vit dans les `.ui.qml` de
    `Screens/` et `Controls/`, que Qt Design Studio ouvre et réenregistre sans les casser.

    Sa seule responsabilité visuelle est de poser les facteurs d'agrandissement : l'entier de la
    charte v1, dérivé de la hauteur de la fenêtre, et le réel de la charte v2 (`uiScale`), rapport
    de la fenêtre à la définition de conception. Ni l'un ni l'autre n'est une décision d'apparence.
*/
Window {
    id: root

    width: 1280
    height: 720
    visible: true
    // Plein écran : le seul réglage qui atteint la FENÊTRE, et il l'atteint par liaison plutôt que
    // par un gestionnaire. Une liaison rend l'état de la fenêtre et celui du réglage indissociables
    // -- ils ne peuvent pas se désynchroniser, faute d'un chemin par lequel le faire.
    visibility: OptionsModel.fullscreen ? Window.FullScreen : Window.Windowed
    color: Tokens.background
    title: "JustAnotherRpgGame"

    /*!
        La palette des contrôles Qt, tirée des jetons.

        `Switch`, `Slider`, `ComboBox` et `Button` se peignent avec la palette de leur fenêtre. Sans
        cette liaison, ils gardaient le bleu du style par défaut au milieu du parchemin -- et la
        seule façon de le corriger aurait été d'écrire une couleur dans chaque écran, c'est-à-dire
        exactement ce que `Tokens.qml` existe pour empêcher.

        Elle est posée sur la FENÊTRE, une fois : la palette descend par héritage, donc un contrôle
        ajouté demain l'aura sans que personne n'ait à y penser.
    */
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

    // Facteur entier borné à [1, 3], depuis une hauteur de référence de 360 px. Repris tel quel de
    // `hmi::identityScaleFor` en attendant que la vue-modèle l'expose : la formule est ici
    // temporairement, jamais l'apparence.
    onHeightChanged: Tokens.scale = Math.max(1, Math.min(3, Math.floor(root.height / 360)))

    /*!
        Facteur réel de la charte v2 (LOT-87) : la fenêtre rapportée à 1920 x 1080.

        Le plus petit des deux rapports, pour que l'écran de conception tienne entier dans la
        fenêtre quel que soit son format : une fenêtre plus large que le 16:9 garde de la marge sur
        les côtés, une plus haute en haut et en bas, et rien n'est rogné. Aucune boucle possible
        (`EX-IHM-080`, `EX-IHM-081`) : aucun écran ne contraint la taille de la fenêtre, donc rien
        de ce qui dépend de ce facteur ne la fait croître.

        Plancher à 0,5 : en deçà, un corps de 18 px tombe sous 9 px et ne se lit plus ; l'écran
        défile plutôt que de rétrécir encore. Une liaison, et non deux gestionnaires : le facteur
        dépend de la largeur ET de la hauteur, et un gestionnaire oublié sur l'une le figerait.
    */
    Binding {
        target: Tokens
        property: "uiScale"
        value: Math.max(0.5, Math.min(root.width / 1920, root.height / 1080))
    }

    /*!
        Écran imposé au lancement, ou vide pour laisser le routeur décider.

        Pouvoir ouvrir un écran précis est ce qui rend sa vérification visuelle possible : les sept
        écrans dessinés mais pas encore alimentés ne sont atteignables par aucun autre chemin.
    */
    // `startScreen` et non `screen` : `Window` porte deja une propriete `screen`, qui designe
    // l'ECRAN PHYSIQUE. La masquer aurait rendu la vraie inatteignable, et une liaison qui
    // aurait voulu la definition de l'affichage aurait recu un nom d'ecran de jeu.
    // Vide par defaut : c'est le ROUTEUR qui decide, et le jeu s'ouvre sur son menu. Renseignee
    // par `--screen=<Nom>`, elle court-circuite le routeur pour ouvrir un ecran directement --
    // outil de verification, pas chemin de jeu.
    property string startScreen: ""

    ScreenStack {
        anchors.fill: parent
        forcedScreen: root.startScreen
    }

    /*!
        Le compteur de diagnostic, commandé par les options (`EX-IHM-083`).

        Il vit ici et non dans l'écran des options : les planches l'avaient tranché, c'est un
        élément de recouvrement, et un compteur d'images par seconde affiché dans une page de
        réglages ne mesurerait que cette page.
    */
    DiagnosticsOverlay {
        anchors.fill: parent
        visible: OptionsModel.diagnostics
    }
}
