import QtQuick
import Jadg.Ui
import Jadg.Runtime

/*!
    Vue de jeu -- CABLAGE, cote developpeur (LOT-86, LOT-87 T4.1).

    La session de jeu se branchera ici quand il y aura un niveau a jouer. Le cadre du HUD est
    dessine, et chacune de ses valeurs porte une cle `hud.*` qui aboutit a `PendingData` jusqu'a
    l'exploration et au groupe (`LOT-19`, `LOT-20`) : ils remplaceront `PendingData` ici, sans
    toucher au formulaire. Les memes cles alimentent le cadre du HUD de combat (`CombatHud.qml`).

    Les raccourcis de navigation ouvrent l'inventaire, le journal, la carte et les options par le
    routeur.
*/
GameViewForm {
    id: root

    focus: true
    pending: true
    status: qsTr("Aucune carte à jouer : le contenu arrive avec un lot ultérieur.")

    characterName: PendingData.value("hud.character.name")
    level: PendingData.value("hud.character.level")
    hitPointsText: PendingData.value("hud.character.hit_points")
    hitPointsRatio: 0
    experienceText: PendingData.value("hud.character.experience")
    experienceRatio: 0
    portrait: PendingData.image("hud.character.portrait")
    party: PendingData.rows("hud.party", 4)
    activeMember: -1
    quests: PendingData.rows("hud.quests", 2)
    clock: PendingData.value("hud.clock")
    location: PendingData.value("hud.location")
    minimap: PendingData.image("hud.minimap")

    // La surface de rendu QRhi, posée dans l'hôte que le formulaire réserve. Elle est ici et non
    // dans le formulaire parce que c'est un type C++ (`Jadg.Runtime`), invisible à l'atelier ; le
    // cadre du formulaire reste au-dessus d'elle, comme un enfant ordinaire.
    GameViewport {
        parent: root.viewportHost
        anchors.fill: parent
        clearColor: Tokens.background
    }

    Keys.onEscapePressed: ScreenRouter.openPause()

    Connections {
        target: root.inventoryButton
        function onClicked() { ScreenRouter.openRpgScreen(ScreenRouter.Inventory) }
    }
    Connections {
        target: root.journalButton
        function onClicked() { ScreenRouter.openRpgScreen(ScreenRouter.QuestJournal) }
    }
    Connections {
        target: root.mapButton
        function onClicked() { ScreenRouter.openRpgScreen(ScreenRouter.WorldMap) }
    }
    Connections {
        target: root.optionsButton
        function onClicked() { ScreenRouter.openOptions() }
    }
}
