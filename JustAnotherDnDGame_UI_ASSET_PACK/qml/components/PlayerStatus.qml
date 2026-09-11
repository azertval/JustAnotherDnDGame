import QtQuick
import "../styles"

GoldPanel {
    width: 410
    height: 118
    Row {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 14
        Rectangle {
            width: 82; height: 82; radius: 41
            color: "#17181C"; border.color: JadgTheme.gold; border.width: 3
            Text { anchors.centerIn: parent; text: "K"; color: JadgTheme.text; font.pixelSize: 42; font.bold: true }
        }
        Column {
            spacing: 6
            Text { text: "Kaelith Voss"; color: JadgTheme.text; font.pixelSize: 22; font.bold: true }
            Text { text: "Niv. 5"; color: JadgTheme.goldLight; font.pixelSize: 16 }
            Rectangle { width: 270; height: 14; radius: 7; color: "#251214"; border.color: JadgTheme.gold
                Rectangle { width: parent.width*0.81; height: parent.height; radius: 7; color: "#B62B22" }
            }
            Text { text: "42 / 52   •   310 / 500 XP"; color: JadgTheme.text; font.pixelSize: 14 }
        }
    }
}
