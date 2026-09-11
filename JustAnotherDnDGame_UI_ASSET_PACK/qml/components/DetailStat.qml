import QtQuick
Item {
    property string label: "Dégâts"
    property string value: "8d6"
    height: 48
    Text { x: 0; y: 4; text: label; color: "#4b3820"; font.bold: true; font.pixelSize: 14 }
    Text { anchors.right: parent.right; y: 4; text: value; color: "#241b11"; font.pixelSize: 15 }
    Rectangle { y: 39; width: parent.width; height: 1; color: "#aa8a50"; opacity: .5 }
}