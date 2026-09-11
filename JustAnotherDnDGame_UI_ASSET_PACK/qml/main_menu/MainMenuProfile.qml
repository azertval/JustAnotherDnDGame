import QtQuick
Item {
    id: root
    property string playerName: "Kaelith"
    property string avatarSource: ""
    signal clicked()
    implicitWidth: 330
    implicitHeight: 82
    Image { anchors.fill: parent; source: "qrc:/JustAnotherDnDGame/svg/profile/profile_panel.svg" }
    Image {
        x: 8; y: 8; width: 66; height: 66
        source: root.avatarSource
        fillMode: Image.PreserveAspectFit
    }
    Text { x: 94; y: 18; text: "Profil joueur"; color: "#b9a77d"; font.family: "IM Fell English"; font.pixelSize: 14 }
    Text { x: 94; y: 40; text: root.playerName; color: "#ead8a7"; font.family: "Cinzel"; font.pixelSize: 19 }
    MouseArea { anchors.fill: parent; onClicked: root.clicked() }
}