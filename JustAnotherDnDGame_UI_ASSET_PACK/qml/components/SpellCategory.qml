import QtQuick
Item {
    property string categoryName: "Évocation"
    property int maximumUses: 2
    property int remainingUses: 1
    property var spells: ["Boule de feu", "Météore"]
    height: 76
    Rectangle { anchors.fill: parent; color: "#3b2518"; border.color: "#b98b32"; border.width: 2; radius: 10 }
    Text { x: 18; y: 10; text: categoryName; color: "#f3dfad"; font.pixelSize: 18; font.bold: true }
    Text { x: 18; y: 38; text: remainingUses + " / " + maximumUses; color: "#fff0c9"; font.pixelSize: 14 }
    Row {
        x: 170; y: 15; spacing: 8
        Repeater {
            model: spells
            Rectangle { width: 125; height: 40; radius: 7; color: "#201811"; border.color: "#9d772d"
                Text { anchors.centerIn: parent; text: modelData; color: "#ead7a8"; font.pixelSize: 12 }
            }
        }
    }
    Row { x: parent.width - 92; y: 26; spacing: 4
        Repeater { model: maximumUses
            Image { width: 18; height: 18; source: index < remainingUses ? "../../assets/svg/slot_used.svg" : "../../assets/svg/slot_empty.svg" }
        }
    }
}