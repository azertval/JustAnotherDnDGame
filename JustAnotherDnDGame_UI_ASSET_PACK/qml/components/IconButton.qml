import QtQuick
import QtQuick.Controls
import "../styles"

Button {
    id: root
    property string iconSource: ""
    implicitWidth: 58
    implicitHeight: 58

    background: Rectangle {
        radius: 29
        color: root.down ? "#33240C" : "#111318"
        border.color: JadgTheme.gold
        border.width: 2
    }
    contentItem: Image {
        source: root.iconSource
        anchors.fill: parent
        anchors.margins: 10
        fillMode: Image.PreserveAspectFit
    }
}
