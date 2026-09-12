import QtQuick

/*!
    Doublure de `hmi::InventoryModel` pour Qt Design Studio (LOT-87).

    Mêmes propriétés que le type C++ (`Source/HMI/Runtime/InventoryModel.h`), avec l'équipement du
    personnage de démonstration ; `equipmentSlots` est un `ListModel` aux rôles de `SheetRowModel`.
*/
QtObject {
    readonly property ListModel equipmentSlots: ListModel {
        ListElement { rowId: "head"; label: "Tête"; value: "Casque de cuir" }
        ListElement { rowId: "torso"; label: "Torse"; value: "Cotte de mailles" }
        ListElement { rowId: "main_hand"; label: "Main directrice"; value: "Hache d'armes" }
        ListElement { rowId: "off_hand"; label: "Main secondaire"; value: "Bouclier" }
        ListElement { rowId: "feet"; label: "Pieds"; value: "Bottes de marche" }
    }
    readonly property string purse: "37 po"
    readonly property string carried: "24 kg"
    readonly property string capacity: "80 kg"
    readonly property string backpack: "Corde (15 m), torches (5), rations (3)"

    function loadDemonstrationCharacter() {}

    signal changed()
}
