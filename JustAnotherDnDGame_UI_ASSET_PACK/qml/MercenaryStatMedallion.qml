import QtQuick
Item {
    id: root
    property string stat: "STR"
    property int score: 10
    property int modifier: 0
    implicitWidth: 112; implicitHeight: 112
    Image { anchors.fill: parent
        source: "qrc:/MercenaryRpgUiKit/svg/character_stats/" + root.stat.toLowerCase() + ".svg"
        fillMode: Image.PreserveAspectFit }
    Text { anchors.centerIn: parent; text: root.score; color: "#241b12"
        font.family: "Cinzel"; font.bold: true; font.pixelSize: 28 }
    Text { anchors.horizontalCenter: parent.horizontalCenter; anchors.bottom: parent.bottom
        anchors.bottomMargin: 16; text: (root.modifier>=0?"+":"")+root.modifier
        color: "#8d1515"; font.family: "Cinzel"; font.bold: true; font.pixelSize: 13 }
}