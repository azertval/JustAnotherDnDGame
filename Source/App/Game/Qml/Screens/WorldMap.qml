import QtQuick
import Jadg.Ui
import Jadg.Runtime

/*!
    Carte du monde -- CABLAGE, cote developpeur (LOT-86, LOT-87 T3.6).

    Les regions et leurs lieux viennent de l'atlas (`WorldMapModel`) ; l'encart du personnage, de
    la fiche de demonstration (`CharacterSheetModel`). Restent en attente, jusqu'aux quetes
    (`LOT-16`) et au voyage (`LOT-42`) : le jour et l'heure, et le lieu ou se trouve le groupe
    (`PendingData`, cles `world_map.*`).

    Le pointeur deplace la carte (glisser) et l'agrandit (molette) ; les touches `+` et `-` et les
    boutons de la mini-carte aussi. Survoler un repere ouvre la fiche de sa region.
*/
WorldMapForm {
    id: root

    focus: true

    readonly property WorldMapModel atlas: WorldMapModel {}
    readonly property CharacterSheetModel sheet: CharacterSheetModel {}

    regions: atlas.regions
    hoveredRegion: -1

    characterName: sheet.name
    levelText: qsTr("Niv. %1").arg(sheet.level)
    hitPointsText: sheet.hitPoints
    hitPointsRatio: {
        const current = parseInt(sheet.hitPoints, 10);
        const maximum = parseInt(sheet.hitPointsMax, 10);
        return maximum > 0 && !isNaN(current) ? Math.max(0, Math.min(1, current / maximum)) : 0;
    }
    experienceText: sheet.experience + " / " + PendingData.value("world_map.experience.next_level")
    experienceRatio: 0
    portrait: PendingData.image("world_map.character.portrait")

    clock: PendingData.value("world_map.clock")
    partyLocation: PendingData.value("world_map.party.location")

    /// Agrandit la carte d'un pas, en gardant le centre de l'ecran au meme endroit de la carte.
    function zoomBy(factor) {
        const next = Math.max(1, Math.min(2.5, root.zoom * factor));
        const centerX = (root.mapView.contentX + root.width / 2) / root.zoom;
        const centerY = (root.mapView.contentY + root.height / 2) / root.zoom;
        root.zoom = next;
        root.mapView.contentX = Math.max(0, Math.min(root.mapView.contentWidth - root.width,
                                                     centerX * next - root.width / 2));
        root.mapView.contentY = Math.max(0, Math.min(root.mapView.contentHeight - root.height,
                                                     centerY * next - root.height / 2));
    }

    function wireMarker(item) {
        item.pointer.entered.connect(() => root.hoveredRegion = item.index)
        item.pointer.exited.connect(() => {
            if (root.hoveredRegion === item.index)
                root.hoveredRegion = -1
        })
    }

    Component.onCompleted: {
        atlas.load()
        sheet.loadDemonstrationCharacter()
        for (let index = 0; index < root.markerRepeater.count; ++index) {
            if (root.markerRepeater.itemAt(index))
                root.wireMarker(root.markerRepeater.itemAt(index))
        }
        // La carte s'ouvre centree sur l'Empire central, coeur de l'atlas.
        root.mapView.contentY = (root.mapView.contentHeight - root.height) / 2
    }

    Connections {
        target: root.markerRepeater
        function onItemAdded(index, item) { root.wireMarker(item) }
    }

    WheelHandler {
        target: null
        onWheel: (event) => root.zoomBy(event.angleDelta.y > 0 ? 1.15 : 1 / 1.15)
    }

    Keys.onPressed: (event) => {
        if (event.key === Qt.Key_Plus) {
            root.zoomBy(1.25)
            event.accepted = true
        } else if (event.key === Qt.Key_Minus) {
            root.zoomBy(1 / 1.25)
            event.accepted = true
        }
    }

    Connections {
        target: root.zoomInButton
        function onClicked() { root.zoomBy(1.25) }
    }
    Connections {
        target: root.zoomOutButton
        function onClicked() { root.zoomBy(1 / 1.25) }
    }
    Connections {
        target: root.questsButton
        function onClicked() { ScreenRouter.openRpgScreen(ScreenRouter.QuestJournal) }
    }
    Connections {
        target: root.inventoryButton
        function onClicked() { ScreenRouter.openRpgScreen(ScreenRouter.Inventory) }
    }
    Connections {
        target: root.companyButton
        function onClicked() { ScreenRouter.openRpgScreen(ScreenRouter.Company) }
    }
    Connections {
        target: root.optionsButton
        function onClicked() { ScreenRouter.openOptions() }
    }
}
