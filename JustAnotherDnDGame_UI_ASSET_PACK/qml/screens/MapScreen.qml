import QtQuick
import QtQuick.Controls
import "../components"

Item {
    id: root
    anchors.fill: parent

    Image {
        anchors.fill: parent
        source: "../../assets/maps/world_map.jpg"
        fillMode: Image.PreserveAspectCrop
        smooth: true
    }

    Rectangle {
        anchors.fill: parent
        color: "#000000"
        opacity: 0.10
    }

    Text {
        x: 32; y: 24
        text: "IMPERIAL TANARES"
        color: "#F2E6C9"
        font.family: "Georgia"
        font.pixelSize: 34
        font.bold: true
    }
    Text {
        x: 34; y: 62
        text: "Provinces et points d’intérêt"
        color: "#E7D6AE"
        font.family: "Georgia"
        font.pixelSize: 17
        font.italic: true
    }

    PlayerStatus { x: 28; y: 110 }
    MapLegend { anchors.left: parent.left; anchors.leftMargin: 28; anchors.bottom: nav.top; anchors.bottomMargin: 22 }

    Image {
        source: "../../assets/icons/icon_compass.svg"
        width: 118; height: 118
        anchors.left: parent.left; anchors.leftMargin: 330
        anchors.bottom: nav.top; anchors.bottomMargin: 55
    }

    MapControls {
        anchors.right: parent.right; anchors.rightMargin: 28
        anchors.verticalCenter: parent.verticalCenter
        onZoomIn: map.scale = Math.min(2.5, map.scale + 0.15)
        onZoomOut: map.scale = Math.max(1.0, map.scale - 0.15)
    }

    Item {
        id: map
        anchors.fill: parent
        property real scale: 1.0
        Image {
            anchors.centerIn: parent
            width: parent.width
            height: parent.height
            source: "../../assets/maps/world_map.jpg"
            fillMode: Image.PreserveAspectCrop
            visible: false
        }
    }

    BottomNavigation {
        id: nav
        anchors.left: parent.left; anchors.right: parent.right
        anchors.bottom: parent.bottom; anchors.margins: 24
    }
}
