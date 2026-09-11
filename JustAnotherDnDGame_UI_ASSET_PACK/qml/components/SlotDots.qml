import QtQuick
Row {
    property int maximum: 3
    property int remaining: 2
    spacing: 4
    Repeater {
        model: maximum
        Image { width: 20; height: 20; source: index < remaining ? "../../assets/svg/slot_used.svg" : "../../assets/svg/slot_empty.svg" }
    }
}