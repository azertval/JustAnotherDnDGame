import QtQuick
import QtQuick.Layouts
import Jadg.Ui

/*!
    Inventaire et equipement -- FORMULAIRE, cote conception (LOT-86).

    A gauche ce que le personnage PORTE, a droite ce qu'il TRANSPORTE. La charge est sous
    l'equipement parce qu'elle en decoule : c'est l'armure endossee qui pese, pas le sac.

    Le sac est une PROSE et non une grille de cases : un inventaire de jeu de role se lit ligne a
    ligne, avec le nom et la quantite, la ou une grille supposerait des objets de meme
    encombrement -- ce que le modele ne dit nulle part.
*/
RpgScreenFrame {
    id: root

    property alias equipmentSlots: slotList.model
    property alias backpack: bagProse.text
    property string purse: "—"
    property string carried: "—"
    property string capacity: "—"

    title: qsTr("Inventaire et équipement")

    content: [
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Tokens.spaceExtraLarge

            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                spacing: Tokens.spaceLarge

                SheetList {
                    id: slotList
                    Layout.fillWidth: true
                    title: qsTr("Équipement")
                    rows: 16
                    model: ListModel {
                        ListElement { label: "Tête"; value: "Casque de cuir" }
                        ListElement { label: "Torse"; value: "Cotte de mailles" }
                        ListElement { label: "Main directrice"; value: "Hache d'armes" }
                        ListElement { label: "Main secondaire"; value: "Bouclier" }
                        ListElement { label: "Pieds"; value: "Bottes de marche" }
                    }
                }

                SheetBlock {
                    Layout.fillWidth: true
                    title: qsTr("Charge")
                    SheetLine { Layout.fillWidth: true; label: qsTr("Poids porté"); value: root.carried }
                    SheetLine { Layout.fillWidth: true; label: qsTr("Capacité"); value: root.capacity }
                }
            }

            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                spacing: Tokens.spaceLarge

                SheetProse {
                    id: bagProse
                    Layout.fillWidth: true
                    title: qsTr("Sac")
                    text: "Corde de chanvre ×1, Torche ×4, Rations de voyage ×3, Potion de soins ×2, Pied-de-biche ×1"
                }

                SheetBlock {
                    Layout.fillWidth: true
                    title: qsTr("Bourse")
                    SheetLine { Layout.fillWidth: true; label: qsTr("Pièces d'or"); value: root.purse }
                }
            }
        }
    ]
}
