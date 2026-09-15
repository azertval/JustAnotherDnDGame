import QtQuick

/*!
    Doublure de `hmi::ArenaModel` pour Qt Design Studio (LOT-50).

    Mêmes propriétés et méthodes que le type C++ (`Source/HMI/Runtime/ArenaModel.h`), avec une
    petite grille d'exemple et deux camps composés, pour que l'écran se dessine dans l'atelier.
    Les méthodes ne font rien : dans l'atelier, personne ne combat.
*/
QtObject {
    readonly property string arenaName: "Arena of the Future"
    readonly property string status: "Exemple de conception"
    readonly property bool inCombat: true
    readonly property bool ended: false
    readonly property var roster: [
        { id: "character:Brenna", name: "Brenna", kind: "personnage", hitPoints: 30, armorClass: 16 },
        { id: "goblin", name: "Gobelin", kind: "creature", hitPoints: 7, armorClass: 15 },
        { id: "wolf", name: "Loup", kind: "creature", hitPoints: 11, armorClass: 13 }
    ]
    readonly property var allies: [ { id: "character:Brenna", name: "Brenna", mark: "tank" } ]
    readonly property var enemies: [ { id: "goblin", name: "Gobelin", mark: "" },
                                     { id: "wolf", name: "Loup", mark: "bruiser" } ]
    readonly property var marks: [ "bruiser", "brute", "commander", "controller", "healer",
                                   "shooter", "tactician", "tank" ]
    property int seed: 2026
    property bool enemyAi: true
    readonly property int gridColumns: 6
    readonly property int gridRows: 4
    readonly property var cells: [
        { column: 0, row: 0, wall: true,  occupant: "", side: "", reachable: false, active: false, down: false, hitPoints: "", hitPointsRatio: 1 },
        { column: 1, row: 0, wall: true,  occupant: "", side: "", reachable: false, active: false, down: false, hitPoints: "", hitPointsRatio: 1 },
        { column: 2, row: 0, wall: true,  occupant: "", side: "", reachable: false, active: false, down: false, hitPoints: "", hitPointsRatio: 1 },
        { column: 3, row: 0, wall: true,  occupant: "", side: "", reachable: false, active: false, down: false, hitPoints: "", hitPointsRatio: 1 },
        { column: 4, row: 0, wall: true,  occupant: "", side: "", reachable: false, active: false, down: false, hitPoints: "", hitPointsRatio: 1 },
        { column: 5, row: 0, wall: true,  occupant: "", side: "", reachable: false, active: false, down: false, hitPoints: "", hitPointsRatio: 1 },
        { column: 0, row: 1, wall: true,  occupant: "", side: "", reachable: false, active: false, down: false, hitPoints: "", hitPointsRatio: 1 },
        { column: 1, row: 1, wall: false, occupant: "Brenna", side: "allies", reachable: false, active: true, down: false, hitPoints: "30/30", hitPointsRatio: 1 },
        { column: 2, row: 1, wall: false, occupant: "", side: "", reachable: true, active: false, down: false, hitPoints: "", hitPointsRatio: 1 },
        { column: 3, row: 1, wall: false, occupant: "", side: "", reachable: true, active: false, down: false, hitPoints: "", hitPointsRatio: 1 },
        { column: 4, row: 1, wall: false, occupant: "Gobelin", side: "enemies", reachable: false, active: false, down: false, hitPoints: "7/7", hitPointsRatio: 1 },
        { column: 5, row: 1, wall: true,  occupant: "", side: "", reachable: false, active: false, down: false, hitPoints: "", hitPointsRatio: 1 },
        { column: 0, row: 2, wall: true,  occupant: "", side: "", reachable: false, active: false, down: false, hitPoints: "", hitPointsRatio: 1 },
        { column: 1, row: 2, wall: false, occupant: "", side: "", reachable: true, active: false, down: false, hitPoints: "", hitPointsRatio: 1 },
        { column: 2, row: 2, wall: false, occupant: "", side: "", reachable: true, active: false, down: false, hitPoints: "", hitPointsRatio: 1 },
        { column: 3, row: 2, wall: false, occupant: "", side: "", reachable: false, active: false, down: false, hitPoints: "", hitPointsRatio: 1 },
        { column: 4, row: 2, wall: false, occupant: "Loup", side: "enemies", reachable: false, active: false, down: true, hitPoints: "0/11", hitPointsRatio: 0 },
        { column: 5, row: 2, wall: true,  occupant: "", side: "", reachable: false, active: false, down: false, hitPoints: "", hitPointsRatio: 1 },
        { column: 0, row: 3, wall: true,  occupant: "", side: "", reachable: false, active: false, down: false, hitPoints: "", hitPointsRatio: 1 },
        { column: 1, row: 3, wall: true,  occupant: "", side: "", reachable: false, active: false, down: false, hitPoints: "", hitPointsRatio: 1 },
        { column: 2, row: 3, wall: true,  occupant: "", side: "", reachable: false, active: false, down: false, hitPoints: "", hitPointsRatio: 1 },
        { column: 3, row: 3, wall: true,  occupant: "", side: "", reachable: false, active: false, down: false, hitPoints: "", hitPointsRatio: 1 },
        { column: 4, row: 3, wall: true,  occupant: "", side: "", reachable: false, active: false, down: false, hitPoints: "", hitPointsRatio: 1 },
        { column: 5, row: 3, wall: true,  occupant: "", side: "", reachable: false, active: false, down: false, hitPoints: "", hitPointsRatio: 1 }
    ]
    readonly property var turnOrder: [
        { name: "Brenna", total: 17, side: "allies", active: true, down: false },
        { name: "Gobelin", total: 12, side: "enemies", active: false, down: false },
        { name: "Loup", total: 9, side: "enemies", active: false, down: true }
    ]
    readonly property string activeName: "Brenna"
    readonly property string activeResources: "action 1 · bonusAction 1 · reaction 1 · movement 4 · heroicAction 1"
    readonly property var journal: [ "montage : 1 allies, 2 ennemis, 0 refus", "initiative Brenna #1 = 17",
                                     "round 1", "debut du tour Brenna #1" ]

    readonly property int cursorColumn: 3
    readonly property int cursorRow: 1
    readonly property var pathCells: [ { column: 2, row: 1 }, { column: 3, row: 1 } ]
    readonly property var turnActions: [
        { label: "Epee longue", kind: "attack", enabled: true, selected: true },
        { label: "Esquiver", kind: "dodge", enabled: true, selected: false },
        { label: "Se desengager", kind: "disengage", enabled: true, selected: false },
        { label: "Se precipiter", kind: "dash", enabled: true, selected: false },
        { label: "Reaction : saisir les opportunites", kind: "reaction", enabled: true, selected: false }
    ]
    readonly property var preview: [ "Deplacement : 2 case(s), il en restera 2.",
                                     "Attaque d'opportunite : Gobelin" ]

    signal changed()

    function addAlly(id) {}
    function addEnemy(id) {}
    function removeAlly(index) {}
    function removeEnemy(index) {}
    function assignMark(ally, index, markId) {}
    function launch() {}
    function tapCell(column, row) {}
    function endTurn() {}
    function dodge() {}
    function disengage() {}
    function dash() {}
    function moveCursor(columns, rows) {}
    function pointCursor(column, row) {}
    function centerCursor() {}
    function cycleTarget(step) {}
    function selectAction(index) {}
    function cycleAction(step) {}
    function confirm() {}
    function withdraw() {}
    function replay() {}
    function backToSetup() {}
}
