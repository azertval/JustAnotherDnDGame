import QtQuick
Item {
    id: root
    property real value: .5
    property bool health: true
    property string valueText: ""
    implicitHeight: 26
    Image { anchors.fill: parent
        source: "qrc:/MercenaryRpgUiKit/svg/character_ui/" +
                (root.health ? "hp_bar_bg.svg" : "xp_bar_fill.svg")
        fillMode: Image.Stretch }
    Rectangle { x: 7; y: 6
        width: Math.max(0,(root.width-14)*Math.max(0,Math.min(1,root.value)))
        height: root.height-12; radius: 3
        color: root.health ? "#8d1515" : "#b58a32" }
    Text { anchors.centerIn: parent; text: root.valueText; color: "#ead8a7"
        font.family: "Cinzel"; font.bold: true; font.pixelSize: 12 }
}