import QtQuick
Item {
    id: root
    property string label: "Général"
    property string iconSource: ""
    property bool active: false
    property bool enabled: true
    signal clicked()
    implicitWidth: 280
    implicitHeight: 62

    Image {
        anchors.fill: parent
        source: root.active
            ? "qrc:/MercenaryRpgUiKit/assets/options/tabs/tab_active.svg"
            : "qrc:/MercenaryRpgUiKit/assets/options/tabs/tab_normal.svg"
        opacity: root.enabled ? 1.0 : 0.45
    }
    Image {
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.verticalCenter: parent.verticalCenter
        width: 32; height: 32
        source: root.iconSource
        fillMode: Image.PreserveAspectFit
    }
    Text {
        anchors.left: parent.left
        anchors.leftMargin: 68
        anchors.verticalCenter: parent.verticalCenter
        text: root.label
        color: root.enabled ? "#ead8a7" : "#77736b"
        font.family: "Cinzel"
        font.pixelSize: 18
    }
    MouseArea { anchors.fill: parent; enabled: root.enabled; onClicked: root.clicked() }
}
