import QtQuick

/*!
    Doublure de `hmi::WorldMapModel` pour Qt Design Studio (LOT-87, T3.6).

    Mêmes propriétés que le type C++ (`Source/HMI/Runtime/WorldMapModel.h`) : trois régions de
    l'atlas, à leurs ancres réelles, pour juger la carte dans l'atelier.
*/
QtObject {
    readonly property var regions: [
        { regionId: "central-empire", name: "Central Empire", x: 0.693, y: 0.454,
          government: "Absolute monarchy", locations: "Hajal City\nImperial Capital", locationCount: 2 },
        { regionId: "kingdom-of-kolbjorn", name: "Kingdom of Kolbjörn", x: 0.204, y: 0.255,
          government: "Monarchy", locations: "Storm Valley", locationCount: 1 },
        { regionId: "seashores", name: "Seashores", x: 0.594, y: 0.609,
          government: "Council", locations: "Bones Valley", locationCount: 1 }
    ]
    readonly property int locationCount: 4

    function load() {}

    signal changed()
}
