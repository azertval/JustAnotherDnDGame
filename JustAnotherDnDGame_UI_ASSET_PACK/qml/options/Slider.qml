import QtQuick
Item {
    id: root
    property real value: 0.5
    property bool enabled: true
    implicitHeight: 34
    Image { anchors.fill: parent; source: "qrc:/MercenaryRpgUiKit/assets/options/controls/slider_track.svg" }
    Rectangle {
        x: 4; y: height/2 - 4
        width: Math.max(0, (parent.width - 8) * Math.max(0, Math.min(1, root.value)))
        height: 8; radius: 4
        color: "#b58a32"
        opacity: root.enabled ? 1.0 : 0.45
    }
    Image {
        x: Math.max(0, Math.min(parent.width - width, parent.width * root.value - width/2))
        anchors.verticalCenter: parent.verticalCenter
        width: 24; height: 24
        source: "qrc:/MercenaryRpgUiKit/assets/options/controls/slider_handle.svg"
        opacity: root.enabled ? 1.0 : 0.45
    }
}
