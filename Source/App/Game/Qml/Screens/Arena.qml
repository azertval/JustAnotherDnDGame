import QtQuick
import Jadg.Ui
import Jadg.Runtime

/*!
    Le Colisee -- CABLAGE, cote developpeur (LOT-50, LOT-24).

    Chaque signal du formulaire devient un appel a `ArenaModel`, et chaque propriete du formulaire
    se lit du modele. Le modele ne connait pas l'ecran, l'ecran ne decide rien du combat : c'est
    la session d'arene (`core::ArenaSession`) qui tient la machine a etats, et ce qui se voit ici
    est relu apres chaque geste.

    Le combat se joue entierement au clavier ou a la manette (LOT-24), par les memes gestes :

    | Geste | Clavier | Manette |
    |---|---|---|
    | Deplacer le curseur | fleches | croix ou stick gauche |
    | Confirmer (attaquer, se deplacer, l'action choisie) ; lancer, rejouer | Entree | A |
    | Cible suivante, precedente | Tab, Maj+Tab | X |
    | Action suivante, precedente | Page suivante, Page precedente, 1 a 9 | RB, LB |
    | Recentrer sur le combattant actif ; nouvelle composition une fois fini | Retour arriere | B |
    | Fin du tour | Espace | Y |
    | Retour au menu | Echap | -- |

    La composition, elle, se fait a la souris ou au clavier par les controles (Tab, Espace) : c'est
    l'ecran de mise en place d'un banc d'essai, pas le combat.
*/
ArenaForm {
    id: root

    focus: true

    readonly property ArenaModel arena: ArenaModel {}

    arenaName: arena.arenaName
    status: arena.status
    inCombat: arena.inCombat
    ended: arena.ended
    roster: arena.roster
    allies: arena.allies
    enemies: arena.enemies
    marks: arena.marks
    seed: arena.seed
    enemyAi: arena.enemyAi
    gridColumns: arena.gridColumns
    gridRows: arena.gridRows
    turnOrder: arena.turnOrder
    activeName: arena.activeName
    activeResources: arena.activeResources
    journal: arena.journal
    turnActions: arena.turnActions
    preview: arena.preview
    cursorColumn: arena.cursorColumn
    cursorRow: arena.cursorRow
    pathCells: arena.pathCells
    gamepadConnected: pad.connected

    // La surface de rendu QRhi, posee dans l'hote que le formulaire reserve (LOT-86 Phase 6) --
    // meme mecanisme que `GameViewport` sur la vue d'exploration (`GameView.qml`) : un type C++
    // (`Jadg.Runtime`), invisible a l'atelier, que le cadre du formulaire recouvre par-dessus.
    ArenaViewport {
        parent: root.viewportHost
        anchors.fill: parent
        // La meme marge que le calque d'interface (`ArenaScene`, dans le formulaire) : les deux
        // ajustent leur grille a la meme surface disponible.
        anchors.margins: Tokens.gapMedium
        model: root.arena
        clearColor: Tokens.panelRaised
    }

    onFighterChosen: (id, ally) => ally ? arena.addAlly(id) : arena.addEnemy(id)
    onAllyRemoved: (index) => arena.removeAlly(index)
    onEnemyRemoved: (index) => arena.removeEnemy(index)
    onMarkChosen: (ally, index, markId) => arena.assignMark(ally, index, markId)
    onSeedEdited: (value) => arena.seed = value
    onEnemyAiToggled: (value) => arena.enemyAi = value
    onLaunchRequested: arena.launch()
    onCellTapped: (column, row) => arena.tapCell(column, row)
    onEndTurnRequested: arena.endTurn()
    onActionChosen: (index) => arena.selectAction(index)
    onWithdrawRequested: arena.withdraw()
    onReplayRequested: arena.replay()
    onBackRequested: arena.backToSetup()
    onCloseRequested: ScreenRouter.closeArena()

    // Un controle de la composition garde le focus apres un clic : le combat le reprend.
    onInCombatChanged: root.forceActiveFocus()

    GamepadNavigator {
        id: pad

        active: root.visible
        onPressed: (button) => root.gamepad(button)
    }

    /// Le geste « confirmer » : lancer avant le combat, rejouer apres, sinon l'action choisie.
    function confirm() {
        if (!arena.inCombat) {
            arena.launch()
        } else if (arena.ended) {
            arena.replay()
        } else {
            arena.confirm()
        }
    }

    function back() {
        if (arena.ended) {
            arena.backToSetup()
        } else {
            arena.centerCursor()
        }
    }

    function gamepad(button) {
        switch (button) {
        case "up": arena.moveCursor(0, -1); break
        case "down": arena.moveCursor(0, 1); break
        case "left": arena.moveCursor(-1, 0); break
        case "right": arena.moveCursor(1, 0); break
        case "a": root.confirm(); break
        case "b": root.back(); break
        case "x": arena.cycleTarget(1); break
        case "y": arena.endTurn(); break
        case "lb": arena.cycleAction(-1); break
        case "rb": arena.cycleAction(1); break
        }
    }

    Keys.onPressed: (event) => {
        if (!arena.inCombat) {
            if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                arena.launch()
                event.accepted = true
            }
            return
        }
        switch (event.key) {
        case Qt.Key_Up: arena.moveCursor(0, -1); break
        case Qt.Key_Down: arena.moveCursor(0, 1); break
        case Qt.Key_Left: arena.moveCursor(-1, 0); break
        case Qt.Key_Right: arena.moveCursor(1, 0); break
        case Qt.Key_Return:
        case Qt.Key_Enter: root.confirm(); break
        case Qt.Key_Tab: arena.cycleTarget(1); break
        case Qt.Key_Backtab: arena.cycleTarget(-1); break
        case Qt.Key_PageDown: arena.cycleAction(1); break
        case Qt.Key_PageUp: arena.cycleAction(-1); break
        case Qt.Key_Backspace: root.back(); break
        case Qt.Key_Space: arena.endTurn(); break
        default:
            if (event.key >= Qt.Key_1 && event.key <= Qt.Key_9) {
                arena.selectAction(event.key - Qt.Key_1)
                break
            }
            return
        }
        event.accepted = true
    }
    Keys.onEscapePressed: ScreenRouter.closeArena()
}
