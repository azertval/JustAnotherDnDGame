import QtQuick
Item {
    id: root
    property string title: "Section"
    property string iconSource: ""
    implicitWidth: 420
    implicitHeight: 180
    Image { anchors.fill: parent; source: "qrc:/MercenaryRpgUiKit/assets/options/frame/section_panel.svg" }
    Image {
        anchors.left: parent.left; anchors.leftMargin: 16
        anchors.top: parent.top; anchors.topMargin: 13
        width: 26; height: 26
        source: root.iconSource
    }
    Text {
        anchors.left: parent.left; anchors.leftMargin: 54
        anchors.top: parent.top; anchors.topMargin: 13
        text: root.title
        color: "#e3c56b"
        font.family: "Cinzel"
        font.pixelSize: 17
    }
    default property alias contentData: contentItem.data
    Item { id: contentItem; anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top; anchors.topMargin: 48; anchors.bottom: parent.bottom; anchors.margins: 18 }
}
