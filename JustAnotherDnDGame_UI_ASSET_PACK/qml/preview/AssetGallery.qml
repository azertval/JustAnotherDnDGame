import QtQuick
import QtQuick.Controls

Item {
    property var groups: [
        ["Character", ["../../assets/svg/character_frame/panel.svg","../../assets/svg/character_frame/title_plate.svg","../../assets/svg/character_identity/heart.svg","../../assets/svg/character_stats/dex.svg","../../assets/svg/character_stats/str.svg"]],
        ["Inventory", ["../../assets/svg/inventory_frame/inventory_frame.svg","../../assets/svg/inventory_frame/inventory_panel.svg","../../assets/svg/inventory_inventory/longsword.svg","../../assets/svg/inventory_inventory/potion_red.svg","../../assets/svg/inventory_slots/slot_selected.svg"]],
        ["Team", ["../../assets/svg/panel_frame.svg","../../assets/svg/gold_divider.svg","../../assets/svg/shield.svg","../../assets/icons/icon_party.svg"]],
        ["Navigation", ["../../assets/icons/icon_map.svg","../../assets/icons/icon_inventory.svg","../../assets/icons/icon_quests.svg","../../assets/icons/icon_options.svg","../../assets/icons/icon_compass.svg"]]
    ]

    Rectangle { anchors.fill: parent; color: "#e3d1a2" }
    Column {
        anchors.fill: parent; anchors.margins: 24; spacing: 16
        Text { text: "GALERIE — ASSETS DU PACK FINAL"; font.family: "Georgia"; font.bold: true; font.pixelSize: 28; color: "#332515" }
        Text { text: "Prévisualisation directe des SVG réutilisables"; font.pixelSize: 13; color: "#6b5434" }
        ScrollView {
            width: parent.width; height: parent.height - 90
            clip: true
            Column {
                width: parent.width; spacing: 22
                Repeater {
                    model: groups
                    delegate: Column {
                        width: parent.width; spacing: 8
                        Text { text: modelData[0]; font.family: "Georgia"; font.bold: true; font.pixelSize: 18; color: "#4a321a" }
                        Row {
                            spacing: 12
                            Repeater {
                                model: modelData[1]
                                delegate: Rectangle {
                                    width: 150; height: 120; color: "#c8b487"; border.color: "#8d671e"; border.width: 1; radius: 5
                                    Image { anchors.centerIn: parent; width: 125; height: 95; source: modelData; fillMode: Image.PreserveAspectFit }
                                    Text { anchors.bottom: parent.bottom; anchors.horizontalCenter: parent.horizontalCenter; text: modelData.split("/").pop(); font.pixelSize: 9; color: "#4b3822" }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
