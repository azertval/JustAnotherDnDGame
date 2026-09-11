import QtQuick

Item {
    id: root
    property string icon: "◆"
    property string amount: "0"
    property string label: ""

    width: 72
    height: 68

    Rectangle {
        anchors.fill: parent
        color: "#3c2c1c"
        border.color: "#b88b2a"
        border.width: 1
        radius: 5
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 7
        text: root.icon
        color: "#ead8a7"
        font.pixelSize: 19
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 31
        text: root.amount
        color: "#fff0c7"
        font.bold: true
        font.pixelSize: 13
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 49
        text: root.label
        color: "#cdbb91"
        font.pixelSize: 8
    }
}
