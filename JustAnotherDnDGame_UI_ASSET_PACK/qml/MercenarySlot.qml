import QtQuick
Item {
    id: root
    property string iconSource: ""
    property string raritySource: ""
    property string countText: ""
    property bool selected: false
    property bool equipped: false
    signal clicked()
    implicitWidth: 96; implicitHeight: 96
    Image { anchors.fill: parent
        source: root.selected ? "qrc:/MercenaryRpgUiKit/svg/inventory_slots/slot_selected.svg"
        : root.equipped ? "qrc:/MercenaryRpgUiKit/svg/inventory_slots/slot_equipped.svg"
        : "qrc:/MercenaryRpgUiKit/svg/inventory_slots/slot_empty.svg" }
    Image { anchors.centerIn: parent; width: parent.width*.68; height: parent.height*.68
        source: root.iconSource; fillMode: Image.PreserveAspectFit; visible: source !== "" }
    Image { anchors.left: parent.left; anchors.top: parent.top; width: 16; height: 16
        source: root.raritySource; visible: source !== "" }
    Text { anchors.right: parent.right; anchors.bottom: parent.bottom; anchors.margins: 7
        text: root.countText; color: "#ead8a7"; font.family: "Cinzel"; font.bold: true
        font.pixelSize: 14; visible: text !== "" }
    MouseArea { anchors.fill: parent; onClicked: root.clicked() }
}