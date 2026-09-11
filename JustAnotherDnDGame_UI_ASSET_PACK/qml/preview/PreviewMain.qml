import QtQuick
import QtQuick.Controls
import "../components"
import "../screens"
import "../main_menu"
import "../options"
import "../credits"

ApplicationWindow {
    id: root
    width: 1600
    height: 900
    visible: true
    title: "JADG — UI Asset Preview"
    color: "#17120d"

    property int page: 0
    property var pages: ["Compétences", "Équipe mercenaire", "Carte", "Menu principal", "Options", "Crédits", "Galerie assets"]

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0; color: "#2a2117" }
            GradientStop { position: 0.5; color: "#17120d" }
            GradientStop { position: 1; color: "#0d0a07" }
        }
    }

    Row {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 12

        Rectangle {
            width: 210
            height: parent.height
            color: "#1c1711"
            border.color: "#8e681f"
            border.width: 2
            radius: 8

            Column {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 9

                Text {
                    text: "JADG"
                    color: "#d8b866"
                    font.family: "Georgia"
                    font.bold: true
                    font.pixelSize: 28
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                Text {
                    text: "UI ASSET PREVIEW"
                    color: "#bba783"
                    font.pixelSize: 10
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Rectangle { width: parent.width; height: 1; color: "#76551d" }

                Repeater {
                    model: root.pages
                    delegate: Button {
                        width: parent.width
                        height: 46
                        text: modelData
                        checkable: true
                        checked: index === root.page
                        onClicked: root.page = index
                        contentItem: Text {
                            text: parent.text
                            color: parent.checked ? "#f3dfaa" : "#c8b991"
                            font.family: "Georgia"
                            font.bold: parent.checked
                            font.pixelSize: 13
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            radius: 5
                            color: parent.checked ? "#6f1e16" : "#292116"
                            border.color: parent.checked ? "#c99d38" : "#6f5424"
                            border.width: 1
                        }
                    }
                }

                Item { width: 1; height: 1; Layout.fillHeight: true }
                Text {
                    width: parent.width
                    text: "ESC / clic : navigation\nLes écrans utilisent directement\nles assets du pack FINAL."
                    color: "#8f8066"
                    font.pixelSize: 10
                    wrapMode: Text.WordWrap
                }
            }
        }

        Rectangle {
            id: viewport
            width: parent.width - 222
            height: parent.height
            color: "#d8c79d"
            border.color: "#a27b27"
            border.width: 2
            radius: 8
            clip: true

            Loader {
                anchors.fill: parent
                sourceComponent: root.page === 0 ? skillsPage
                                : root.page === 1 ? teamPage
                                : root.page === 2 ? mapPage
                                : root.page === 3 ? menuPage
                                : root.page === 4 ? optionsPage
                                : root.page === 5 ? creditsPage
                                : galleryPage
            }
        }
    }

    Component {
        id: skillsPage
        PreviewCompetencesScreen { anchors.fill: parent }
    }
    Component {
        id: teamPage
        MercenaryTeamScreen { anchors.fill: parent }
    }
    Component {
        id: mapPage
        MapScreen { anchors.fill: parent }
    }
    Component {
        id: menuPage
        MainMenu { anchors.fill: parent }
    }
    Component {
        id: optionsPage
        OptionsScreen { anchors.fill: parent }
    }
    Component {
        id: creditsPage
        CreditsScreen { anchors.fill: parent }
    }
    Component {
        id: galleryPage
        AssetGallery { anchors.fill: parent }
    }
}
