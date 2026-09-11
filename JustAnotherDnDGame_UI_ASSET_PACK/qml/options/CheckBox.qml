import QtQuick
Item {
    id: root
    property bool checked: false
    property bool enabled: true
    property string label: ""
    signal clicked()
    implicitHeight: 34
    Image {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        width: 22; height: 22
        source: root.checked
            ? "qrc:/MercenaryRpgUiKit/assets/options/controls/checkbox_checked.svg"
            : "qrc:/MercenaryRpgUiKit/assets/options/controls/checkbox_unchecked.svg"
        opacity: root.enabled ? 1.0 : 0.45
    }
    Text {
        anchors.left: parent.left
        anchors.leftMargin: 34
        anchors.verticalCenter: parent.verticalCenter
        text: root.label
        color: root.enabled ? "#ead8a7" : "#77736b"
        font.family: "IM Fell English"
        font.pixelSize: 15
    }
    MouseArea { anchors.fill: parent; enabled: root.enabled; onClicked: root.clicked() }
}
