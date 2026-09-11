import QtQuick

Item {
    id: root
    property string name: "Membre"
    property string role: "Mercenaire"
    property string level: "1"
    property string initials: "?"
    signal clicked()

    width: 108
    height: 132

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 4
        width: 74
        height: 74
        radius: 37
        color: "#c7b78d"
        border.color: "#8d671c"
        border.width: 2

        Text {
            anchors.centerIn: parent
            text: root.initials
            color: "#4c241b"
            font.family: "Georgia"
            font.bold: true
            font.pixelSize: 24
        }
    }

    Rectangle {
        x: 5; y: 69
        width: 25; height: 25
        radius: 12.5
        color: "#5b1712"
        border.color: "#c59a32"
        border.width: 1

        Text {
            anchors.centerIn: parent
            text: root.level
            color: "#f5e9c8"
            font.bold: true
            font.pixelSize: 12
        }
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 84
        width: parent.width
        text: root.name
        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
        color: "#352a1b"
        font.family: "Georgia"
        font.bold: true
        font.pixelSize: 12
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 103
        width: parent.width
        text: root.role
        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
        color: "#786545"
        font.pixelSize: 10
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
    }
}
