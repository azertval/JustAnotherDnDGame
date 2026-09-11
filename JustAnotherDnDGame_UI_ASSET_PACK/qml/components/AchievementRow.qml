import QtQuick

Item {
    id: root
    property string title: "Haut fait"
    property string description: ""
    property string date: ""
    property bool unlocked: true

    height: 58

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.color: "#a17a2b"
        border.width: 1
        opacity: 0.42
    }

    Rectangle {
        x: 8; y: 9
        width: 38; height: 38
        radius: 19
        color: root.unlocked ? "#5a1812" : "#8b8067"
        border.color: "#c59a32"
        border.width: 1

        Text {
            anchors.centerIn: parent
            text: root.unlocked ? "★" : "?"
            color: "#f4e7c1"
            font.pixelSize: 17
        }
    }

    Text {
        x: 55; y: 9
        text: root.title
        color: "#34281a"
        font.family: "Georgia"
        font.bold: true
        font.pixelSize: 12
    }

    Text {
        x: 55; y: 29
        width: parent.width - 130
        text: root.description
        color: "#766342"
        font.pixelSize: 10
        elide: Text.ElideRight
    }

    Text {
        anchors.right: parent.right
        anchors.rightMargin: 8
        y: 20
        text: root.date
        color: "#8a734b"
        font.pixelSize: 9
    }
}
