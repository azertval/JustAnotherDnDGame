import QtQuick
import Jadg.Ui

/*!
    Une entree de menu (LOT-86).

    Le focus se signale par un CHEVRON et par la couleur, jamais par la seule couleur : un menu se
    parcourt a la manette, sans pointeur pour dire ou l'on en est, et un signal qui ne tient qu'a
    une teinte disparait pour qui les distingue mal (EX-IHM-040).
*/
Item {
    id: root

    property string label: "Entrée"
    property bool current: false

    implicitHeight: entryLabel.implicitHeight
    implicitWidth: entryChevron.implicitWidth + entryLabel.implicitWidth + Tokens.spaceMedium

    Text {
        id: entryChevron
        anchors.verticalCenter: parent.verticalCenter
        text: ">"
        color: Tokens.accent
        font.family: Tokens.bodyFamily
        font.pixelSize: Tokens.sectionTitle
        opacity: root.current ? 1.0 : 0.0
    }

    Text {
        id: entryLabel
        anchors.left: entryChevron.right
        anchors.leftMargin: Tokens.spaceMedium
        anchors.verticalCenter: parent.verticalCenter
        text: root.label
        color: root.current ? Tokens.text : Tokens.textMuted
        font.family: Tokens.bodyFamily
        font.pixelSize: Tokens.sectionTitle
    }
}
