import QtQuick
Item {
    property string title: "COMPÉTENCES"
    implicitHeight: 64
    Image { anchors.fill: parent; source: "../../assets/svg/header_plaque.svg"; fillMode: Image.Stretch }
    Text { anchors.centerIn: parent; text: title; color: "#f4e4bd"; font.bold: true; font.pixelSize: 25; font.family: "Georgia" }
}