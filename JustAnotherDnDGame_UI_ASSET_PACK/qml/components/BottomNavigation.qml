import QtQuick
import QtQuick.Controls
import "../styles"

Rectangle {
    id: root
    height: 92
    radius: 18
    color: "#0C0D10"
    border.color: JadgTheme.gold
    border.width: 2

    property int selectedIndex: 0
    signal selected(int index)

    Row {
        anchors.centerIn: parent
        spacing: 18
        Repeater {
            model: [
                ["icon_map.svg","Carte"],
                ["icon_quests.svg","Quêtes"],
                ["icon_inventory.svg","Inventaire"],
                ["icon_party.svg","Équipe"],
                ["icon_options.svg","Options"]
            ]
            delegate: Button {
                width: 110; height: 72
                background: Rectangle {
                    radius: 12
                    color: index === root.selectedIndex ? "#33240C" : "transparent"
                    border.color: index === root.selectedIndex ? JadgTheme.gold : "transparent"
                    border.width: 1
                }
                contentItem: Column {
                    spacing: 3
                    Image { width: 34; height: 34; anchors.horizontalCenter: parent.horizontalCenter; source: "../../assets/icons/" + modelData[0] }
                    Text { text: modelData[1]; color: JadgTheme.text; font.pixelSize: 15; anchors.horizontalCenter: parent.horizontalCenter }
                }
                onClicked: { root.selectedIndex = index; root.selected(index) }
            }
        }
    }
}
