import QtQuick
import QtQuick.Controls
import "../styles"

GoldPanel {
    id: root
    width: 280
    height: 430
    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 14
        Text { text: "LÉGENDE"; color: JadgTheme.text; font.pixelSize: 25; font.bold: true }
        Repeater {
            model: [
                ["icon_city.svg","Ville / Cité"],
                ["icon_fortress.svg","Forteresse"],
                ["icon_dungeon.svg","Donjon / Ruines"],
                ["icon_camp.svg","Camp"],
                ["icon_quest_main.svg","Quête principale"],
                ["icon_quest_side.svg","Quête secondaire"],
                ["icon_poi.svg","Point d’intérêt"]
            ]
            delegate: Row {
                spacing: 12
                Image { width: 30; height: 30; source: "../../assets/icons/" + modelData[0] }
                Text { text: modelData[1]; color: JadgTheme.text; font.pixelSize: 17; anchors.verticalCenter: parent.verticalCenter }
            }
        }
    }
}
