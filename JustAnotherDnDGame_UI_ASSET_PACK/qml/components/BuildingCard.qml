import QtQuick

Item {
    id: root
    property string name: "Bâtiment"
    property string level: "Niv. 1"
    property string status: ""
    property bool locked: false

    Rectangle {
        anchors.fill: parent
        color: root.locked ? "#e3d7b7" : "#efe2c0"
        border.color: "#9a7020"
        border.width: 1
        radius: 6
    }

    Rectangle {
        x: 8; y: 8
        width: 34; height: 34
        radius: 17
        color: "#4a2419"
        border.color: "#bd902b"
        border.width: 1

        Text {
            anchors.centerIn: parent
            text: root.locked ? "×" : "⌂"
            color: "#f1e4bd"
            font.pixelSize: 17
        }
    }

    Text {
        x: 50; y: 9
        width: parent.width - 58
        text: root.name
        color: "#34281a"
        font.family: "Georgia"
        font.bold: true
        font.pixelSize: 12
        elide: Text.ElideRight
    }

    Text {
        x: 50; y: 29
        text: root.locked ? "Emplacement verrouillé" : root.level
        color: "#806a42"
        font.pixelSize: 10
    }

    Text {
        visible: root.status.length > 0
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 7
        text: root.status
        color: "#6b1712"
        font.bold: true
        font.pixelSize: 10
    }
}
