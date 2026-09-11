import QtQuick
Item {
    property string skillName: "Épée longue"
    property string damageType: "Tranchant"
    property string rangeText: "1,5 m"
    property string damageText: "1d8 + 3"
    property string effectText: "—"
    property string iconSource: "../../assets/icons/attack.svg"
    height: 118
    Rectangle { anchors.fill: parent; color: "#efe0b9"; opacity: .65; border.color: "#92702d"; border.width: 1; radius: 8 }
    Image { x: 12; y: 18; width: 70; height: 70; source: iconSource }
    Column {
        x: 95; y: 12; spacing: 4
        Text { text: skillName; font.pixelSize: 19; font.bold: true; color: "#241b11" }
        Text { text: "Type : " + damageType; font.pixelSize: 13; color: "#3b2c1a" }
        Text { text: "Portée : " + rangeText; font.pixelSize: 13; color: "#3b2c1a" }
        Text { text: "Dégâts : " + damageText; font.pixelSize: 13; color: "#3b2c1a" }
        Text { text: "Effets : " + effectText; font.pixelSize: 13; color: "#3b2c1a"; wrapMode: Text.WordWrap; width: parent.parent.width - 105 }
    }
}