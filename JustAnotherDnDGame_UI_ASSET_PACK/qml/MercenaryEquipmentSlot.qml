import QtQuick
Item {
    id: root
    property string iconSource: ""
    property string label: ""
    property bool equipped: false
    signal clicked()
    implicitWidth: 118; implicitHeight: 142
    Image { anchors.top: parent.top; anchors.horizontalCenter: parent.horizontalCenter
        width: 118; height: 118
        source: "qrc:/MercenaryRpgUiKit/svg/inventory_slots/equipment_slot.svg" }
    Image { anchors.centerIn: parent; width: 72; height: 72
        source: root.iconSource; fillMode: Image.PreserveAspectFit; visible: source !== "" }
    Text { anchors.top: parent.top; anchors.topMargin: 120; anchors.horizontalCenter: parent.horizontalCenter
        text: root.label; color: "#241b12"; font.family: "IM Fell English"; font.pixelSize: 13 }
    MouseArea { anchors.fill: parent; onClicked: root.clicked() }
}