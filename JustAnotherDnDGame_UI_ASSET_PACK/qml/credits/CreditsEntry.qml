import QtQuick
Item {
    id: root
    property string role: "PROGRAMMATION"
    property string names: "Nom de l'équipe"
    implicitWidth: 700
    implicitHeight: 58

    Image {
        anchors.fill: parent
        source: "qrc:/JustAnotherDnDGame/svg/frame/section_panel.svg"
        opacity: 0.42
    }
    Text {
        x: 28; anchors.verticalCenter: parent.verticalCenter
        text: root.role
        color: "#b58a32"
        font.family: "Cinzel"
        font.pixelSize: 13
        font.bold: true
    }
    Text {
        x: 250; anchors.verticalCenter: parent.verticalCenter
        text: root.names
        color: "#ead8a7"
        font.family: "IM Fell English"
        font.pixelSize: 18
    }
}