import QtQuick
import Jadg.Ui
import Jadg.Runtime

/*!
    Inventaire et equipement -- CABLAGE, cote developpeur (LOT-86, LOT-87 T3.5).

    Tout vient d'`InventoryModel`, qui garde l'inventaire du personnage de demonstration et
    RECALCULE tout apres chaque action : filtrer, selectionner, equiper, retirer, jeter, trier.
    L'ecran ne tient aucun etat a lui ; il suit le modele.

    Les cases sont creees par les `Repeater` du formulaire, et recreees quand la grille change : le
    pointeur de chaque case est branche a sa creation (`itemAdded`), et une fois pour celles qui
    existaient deja quand ce fichier se charge.
*/
InventoryForm {
    id: root

    readonly property InventoryModel inventory: InventoryModel {}

    equipped: inventory.equipped
    cells: inventory.cells
    filter: inventory.filter
    selectedItem: inventory.selectedItem
    selectedSlot: inventory.selectedSlot
    selection: inventory.selection
    carried: inventory.carried
    capacity: inventory.capacity
    loadRatio: inventory.loadRatio
    gold: inventory.gold
    armorClass: inventory.armorClass
    initiative: inventory.initiative
    speed: inventory.speed
    passivePerception: inventory.passivePerception

    function wireCell(item) {
        item.pointer.clicked.connect(() => root.inventory.selectItem(item.modelData.itemId))
    }

    function wireSlot(item) {
        item.pointer.clicked.connect(() => root.inventory.selectSlot(item.modelData))
    }

    function wireExisting(repeater, wire) {
        for (let index = 0; index < repeater.count; ++index) {
            if (repeater.itemAt(index))
                wire(repeater.itemAt(index))
        }
    }

    Component.onCompleted: {
        inventory.loadDemonstrationCharacter()
        // L'ecran s'ouvre sur le premier objet du sac : une fiche vide n'apprend rien.
        if (inventory.cells.length > 0)
            inventory.selectItem(inventory.cells[0].itemId)
        root.wireExisting(root.slotRepeaterLeft, root.wireSlot)
        root.wireExisting(root.slotRepeaterRight, root.wireSlot)
        root.wireExisting(root.cellRepeater, root.wireCell)
    }

    Connections {
        target: root.cellRepeater
        function onItemAdded(index, item) { root.wireCell(item) }
    }

    Connections {
        target: root.allTab
        function onClicked() { root.inventory.filter = 0 }
    }
    Connections {
        target: root.equipmentTab
        function onClicked() { root.inventory.filter = 1 }
    }
    Connections {
        target: root.gearTab
        function onClicked() { root.inventory.filter = 2 }
    }
    Connections {
        target: root.toolsTab
        function onClicked() { root.inventory.filter = 3 }
    }

    Connections {
        target: root.sortButton
        function onClicked() { root.inventory.sortBackpack() }
    }
    Connections {
        target: root.equipButton
        function onClicked() {
            if (root.inventory.selectedSlot.length > 0)
                root.inventory.unequipSelected()
            else
                root.inventory.equipSelected()
        }
    }
    Connections {
        target: root.dropButton
        function onClicked() { root.inventory.dropSelected() }
    }
}
