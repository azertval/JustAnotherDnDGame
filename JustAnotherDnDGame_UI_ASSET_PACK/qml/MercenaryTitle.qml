import QtQuick
Item {
    id: root
    property string text: "TITRE"
    property bool red: false
    implicitHeight: 48
    Image { anchors.fill: parent
        source: root.red ? "qrc:/MercenaryRpgUiKit/svg/inventory_frame/red_banner.svg"
                         : "qrc:/MercenaryRpgUiKit/svg/character_frame/title_plate.svg"
        fillMode: Image.Stretch }
    Text { anchors.centerIn: parent; text: root.text; color: "#ead8a7"
        font.family: "Cinzel"; font.bold: true; font.pixelSize: 18
        style: Text.Outline; styleColor: "#241b12" }
}