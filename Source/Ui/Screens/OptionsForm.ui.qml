import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Jadg.Ui

/*!
    Options -- FORMULAIRE, côté conception (LOT-86).

    Les réglages sont **réels** : chacun atteint le moteur (`EX-IHM-083`). Deux d'entre eux
    s'appliquent au prochain lancement, et l'écran le dit — un réglage qui s'applique plus tard
    atteint bien le moteur, mais l'utilisateur doit savoir quand.

    Le compteur d'images par seconde ne figure pas ici : les planches l'avaient tranché, c'est un
    élément de HUD, il vit avec le HUD et non dans les écrans.

    Les contrôles sont exposés par `property alias` : c'est le jumeau qui leur attache une réaction,
    un formulaire ne pouvant pas contenir de code.
*/
RpgScreenFrame {
    id: root

    property alias fullscreenSwitch: fullscreenControl
    property alias vsyncSwitch: vsyncControl
    property alias diagnosticsSwitch: diagnosticsControl
    property alias volumeSlider: volumeControl
    property alias languageBox: languageControl
    property alias saveLogsButton: saveLogsControl

    property string logsMessage: ""
    property bool logsEnabled: true

    navigationHint: qsTr("Échap") + " · " + qsTr("Retour")
    title: qsTr("Options")

    content: [
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Tokens.spaceExtraLarge

            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                spacing: Tokens.spaceLarge

                SheetBlock {
                    Layout.fillWidth: true
                    title: qsTr("Affichage")

                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            Layout.fillWidth: true
                            text: qsTr("Plein écran")
                            color: Tokens.textMuted
                            font.family: Tokens.bodyFamily
                            font.pixelSize: Tokens.body
                        }
                        Switch { id: fullscreenControl }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            Layout.fillWidth: true
                            text: qsTr("Synchronisation verticale")
                            color: Tokens.textMuted
                            font.family: Tokens.bodyFamily
                            font.pixelSize: Tokens.body
                        }
                        Switch { id: vsyncControl }
                    }

                    Text {
                        Layout.fillWidth: true
                        text: qsTr("Appliquée au prochain lancement.")
                        color: Tokens.textMuted
                        font.family: Tokens.bodyFamily
                        font.pixelSize: Tokens.caption
                        opacity: 0.8
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            Layout.fillWidth: true
                            text: qsTr("Compteur de diagnostic")
                            color: Tokens.textMuted
                            font.family: Tokens.bodyFamily
                            font.pixelSize: Tokens.body
                        }
                        Switch { id: diagnosticsControl }
                    }
                }

                SheetBlock {
                    Layout.fillWidth: true
                    title: qsTr("Volume")

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Tokens.spaceMedium
                        Slider {
                            id: volumeControl
                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            stepSize: 1
                        }
                        Text {
                            text: Math.round(volumeControl.value) + " %"
                            color: Tokens.text
                            font.family: Tokens.bodyFamily
                            font.pixelSize: Tokens.body
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                spacing: Tokens.spaceLarge

                SheetBlock {
                    Layout.fillWidth: true
                    title: qsTr("Langue")

                    ComboBox {
                        id: languageControl
                        Layout.fillWidth: true
                    }
                }

                SheetBlock {
                    Layout.fillWidth: true
                    title: qsTr("Journaux")

                    Button {
                        id: saveLogsControl
                        Layout.fillWidth: true
                        enabled: root.logsEnabled
                        text: qsTr("Enregistrer les journaux de session")
                    }

                    Text {
                        Layout.fillWidth: true
                        text: root.logsMessage
                        color: Tokens.textMuted
                        font.family: Tokens.bodyFamily
                        font.pixelSize: Tokens.caption
                        wrapMode: Text.WrapAnywhere
                        visible: root.logsMessage.length > 0
                    }
                }
            }
        }
    ]
}
