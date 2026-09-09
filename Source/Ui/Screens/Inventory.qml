import QtQuick
import Jadg.Ui

/*!
    Inventaire et equipement -- CABLAGE, cote developpeur (LOT-86).

    Le chargement est PARTAGE avec la fiche de personnage : les deux ecrans decrivent le meme
    personnage, et deux chargements separes auraient pu ne pas voir le meme equipement -- alors que
    la classe d'armure de la fiche vient de ce que cet inventaire contient.
*/
InventoryForm {
    id: root

    readonly property InventoryModel inventory: InventoryModel {}

    equipmentSlots: inventory.equipmentSlots
    backpack: inventory.backpack
    purse: inventory.purse
    carried: inventory.carried
    capacity: inventory.capacity

    Component.onCompleted: inventory.loadDemonstrationCharacter()
}
