import QtQuick

/*!
    Doublure de `hmi::GamepadNavigator` pour Qt Design Studio (LOT-24).

    Mêmes propriétés et signal que le type C++ (`Source/HMI/Runtime/GamepadNavigator.h`). Dans
    l'atelier, aucune manette n'est sondée.
*/
QtObject {
    property bool active: false
    readonly property bool connected: false

    signal pressed(string button)
}
