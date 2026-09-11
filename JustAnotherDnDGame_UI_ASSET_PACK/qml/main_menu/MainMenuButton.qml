import QtQuick
Item {
    id: root
    property string label: "Continuer"
    property string iconSource: ""
    property bool active: false
    property bool enabled: true
    signal clicked()
    implicitWidth: 430
    implicitHeight: 74
    Image {
        anchors.fill: parent
        source: root.active
            ? "qrc:/JustAnotherDnDGame/svg/navigation/menu_button_active.svg"
            : "qrc:/JustAnotherDnDGame/svg/navigation/menu_button_normal.svg"
        opacity: root.enabled ? 1 : 0.45
    }
    Image {
        anchors.left: parent.left
        anchors.leftMargin: 26
        anchors.verticalCenter: parent.verticalCenter
        width: 30; height: 30
        source: root.iconSource
    }
    Text {
        anchors.left: parent.left
        anchors.leftMargin: 78
        anchors.verticalCenter: parent.verticalCenter
        text: root.label
        color: "#ead8a7"
        font.family: "Cinzel"
        font.pixelSize: 21
    }
    MouseArea {
        anchors.fill: parent
        enabled: root.enabled
        onClicked: root.clicked()
    }
}