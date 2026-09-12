import QtQuick

/*!
    Doublure de `hmi::CharacterSheetModel` pour Qt Design Studio (LOT-87).

    Mêmes propriétés que le type C++ (`Source/HMI/Runtime/CharacterSheetModel.h`), avec les valeurs
    du personnage de démonstration ; les listes sont des `ListModel` aux rôles de `SheetRowModel`
    (`rowId`, `label`, `value`), parce qu'un tableau n'exposerait que `modelData`.
*/
QtObject {
    readonly property string name: "Brenna Pierrefonte"
    readonly property string species: "Naine des collines"
    readonly property string background: "Artisane de guilde"
    readonly property string classAndLevel: "Guerrière 3"
    readonly property string level: "3"
    readonly property string experience: "900"
    readonly property string hitPoints: "25 / 30"
    readonly property string hitPointsMax: "30"
    readonly property string hitDice: "3d10"
    readonly property string armorClass: "16"
    readonly property string initiative: "+1"
    readonly property string speed: "9 m"
    readonly property string proficiencyBonus: "+2"
    readonly property string passivePerception: "11"

    readonly property ListModel abilities: ListModel {
        ListElement { rowId: "strength"; label: "Force"; value: "16 (+3)" }
        ListElement { rowId: "dexterity"; label: "Dextérité"; value: "12 (+1)" }
        ListElement { rowId: "constitution"; label: "Constitution"; value: "15 (+2)" }
        ListElement { rowId: "intelligence"; label: "Intelligence"; value: "10 (+0)" }
        ListElement { rowId: "wisdom"; label: "Sagesse"; value: "13 (+1)" }
        ListElement { rowId: "charisma"; label: "Charisme"; value: "8 (-1)" }
    }

    readonly property ListModel savingThrows: ListModel {
        ListElement { rowId: "strength"; label: "Force"; value: "+5 •" }
        ListElement { rowId: "dexterity"; label: "Dextérité"; value: "+1" }
        ListElement { rowId: "constitution"; label: "Constitution"; value: "+4 •" }
        ListElement { rowId: "intelligence"; label: "Intelligence"; value: "+0" }
        ListElement { rowId: "wisdom"; label: "Sagesse"; value: "+1" }
        ListElement { rowId: "charisma"; label: "Charisme"; value: "-1" }
    }

    readonly property ListModel skills: ListModel {
        ListElement { rowId: "athletics"; label: "Athlétisme"; value: "+5 •" }
        ListElement { rowId: "stealth"; label: "Discrétion"; value: "+1" }
        ListElement { rowId: "perception"; label: "Perception"; value: "+1" }
        ListElement { rowId: "insight"; label: "Intuition"; value: "+1" }
    }

    function loadDemonstrationCharacter() {}

    signal changed()
}
