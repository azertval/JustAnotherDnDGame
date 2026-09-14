import QtQuick
import Jadg.Ui
import Jadg.Runtime

/*!
    Le Colisee -- CABLAGE, cote developpeur (LOT-50).

    Chaque signal du formulaire devient un appel a `ArenaModel`, et chaque propriete du formulaire
    se lit du modele. Le modele ne connait pas l'ecran, l'ecran ne decide rien du combat : c'est
    la session d'arene (`core::ArenaSession`) qui tient la machine a etats, et ce qui se voit ici
    est relu apres chaque geste.

    `Echap` revient au menu ; `Espace` termine le tour.
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
    gridColumns: arena.gridColumns
    gridRows: arena.gridRows
    cells: arena.cells
    turnOrder: arena.turnOrder
    activeName: arena.activeName
    activeResources: arena.activeResources
    journal: arena.journal

    onFighterChosen: (id, ally) => ally ? arena.addAlly(id) : arena.addEnemy(id)
    onAllyRemoved: (index) => arena.removeAlly(index)
    onEnemyRemoved: (index) => arena.removeEnemy(index)
    onMarkChosen: (ally, index, markId) => arena.assignMark(ally, index, markId)
    onSeedEdited: (value) => arena.seed = value
    onLaunchRequested: arena.launch()
    onCellTapped: (column, row) => arena.tapCell(column, row)
    onEndTurnRequested: arena.endTurn()
    onWithdrawRequested: arena.withdraw()
    onReplayRequested: arena.replay()
    onBackRequested: arena.backToSetup()
    onCloseRequested: ScreenRouter.closeArena()

    Keys.onEscapePressed: ScreenRouter.closeArena()
    Keys.onSpacePressed: arena.endTurn()
}
