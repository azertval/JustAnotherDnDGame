import QtQuick
Item {
    id: root
    property string skill: "Acrobatics"
    property string ability: "Dex"
    property int bonus: 0
    property bool proficient: false
    signal clicked()
    implicitHeight: 34
    Image { anchors.fill: parent; source: "qrc:/MercenaryRpgUiKit/svg/character_ui/skill_row.svg"
        fillMode: Image.Stretch }
    Text { anchors.left: parent.left; anchors.leftMargin: 34; anchors.verticalCenter: parent.verticalCenter
        text: root.skill+" ("+root.ability+")"; color: "#241b12"
        font.family: "IM Fell English"; font.pixelSize: 15 }
    Text { anchors.right: parent.right; anchors.rightMargin: 12; anchors.verticalCenter: parent.verticalCenter
        text: (root.bonus>=0?"+":"")+root.bonus; color: "#8d1515"
        font.family: "Cinzel"; font.bold: true; font.pixelSize: 14 }
    MouseArea { anchors.fill: parent; onClicked: root.clicked() }
}