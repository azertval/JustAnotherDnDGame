import QtQuick
Item {
    id: root
    property string label: "Appliquer"
    property bool primary: true
    property bool enabled: true
    signal clicked()
    implicitWidth: 180
    implicitHeight: 48
    Image {
        anchors.fill: parent
        source: root.primary
            ? "qrc:/MercenaryRpgUiKit/assets/options/controls/button_apply.svg"
            : "qrc:/MercenaryRpgUiKit/assets/options/controls/button_secondary.svg"
        opacity: root.enabled ? 1.0 : 0.45
    }
    Text {
        anchors.centerIn: parent
        text: root.label
        color: "#ead8a7"
        font.family: "Cinzel"
        font.pixelSize: 16
    }
    MouseArea { anchors.fill: parent; enabled: root.enabled; onClicked: root.clicked() }
}
