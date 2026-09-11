import QtQuick

Item {
    id: root
    property string label: "STAT"
    property string value: "0"
    property string iconText: "✦"

    width: 92
    height: 86

    Rectangle {
        anchors.fill: parent
        radius: width / 2
        color: "#ead9ad"
        border.color: "#8d671c"
        border.width: 2
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 8
        text: root.iconText
        color: "#6c1510"
        font.pixelSize: 19
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 31
        text: root.label
        color: "#4b3a20"
        font.family: "Georgia"
        font.bold: true
        font.pixelSize: 10
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 49
        text: root.value
        color: "#251d13"
        font.family: "Georgia"
        font.bold: true
        font.pixelSize: 19
    }
}
