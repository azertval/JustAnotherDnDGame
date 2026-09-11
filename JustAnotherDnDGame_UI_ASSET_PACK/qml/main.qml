import QtQuick
import QtQuick.Window
import "screens"

Window {
    width: 1600
    height: 900
    visible: true
    title: "Just Another D&D Game — Carte"
    color: "#101318"

    MapScreen { anchors.fill: parent }
}
