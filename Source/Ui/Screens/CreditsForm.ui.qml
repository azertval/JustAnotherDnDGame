import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Jadg.Ui

/*!
    Credits -- FORMULAIRE, cote conception (LOT-87, T3.3 ; maquette 07).

    Transcrit de la maquette `07_Credit_Mockup.png` (1672 x 941), cotes ramenees a 1920 x 1080 puis
    multipliees par `Tokens.uiScale` : la scene du menu, le logotype en haut a gauche, le grand
    panneau sombre coiffe de sa plaque « Credits », deux colonnes de sections, la citation au pied du
    panneau, le bouton Retour en bas a gauche et la version en bas a droite.

    **Aucun nom propre ici.** Les attributions sont des donnees (`Source/Elements/Credits/credits.json`,
    lues par `CreditsModel`) : le jumeau pose les sections de chaque colonne dans `leftSections` et
    `rightSections`. Les valeurs ci-dessous sont celles de la conception.
*/
Item {
    id: root

    /// Sections de chaque colonne : listes de `sectionId`, `title`, `iconKey`, `lines`.
    property var leftSections: []
    property var rightSections: []

    /// La version du jeu, sans le « v », posee par le jumeau.
    property string version: "0.0.0"

    property alias backButton: backControl
    /// La barre de defilement des sections : le jumeau la fait avancer au clavier.
    property alias scrollBar: creditsScrollBar

    width: 1920
    height: 1080

    // --- Le fond : la scene du menu, et son repli -------------------------------------------------
    Rectangle {
        anchors.fill: parent
        color: Tokens.panel
    }

    Image {
        anchors.fill: parent
        visible: !sceneArt.delivered
        source: "../../Elements/Assets/UI/world-map.jpg"
        fillMode: Image.PreserveAspectCrop
        smooth: true
    }

    CoverArt {
        id: sceneArt

        anchors.fill: parent
        key: "ui/background/menu-scene"
    }

    // La carte de repli est claire : un voile garde le logotype et le bouton lisibles. La scene
    // produite, cadree a droite derriere le panneau, n'en a pas besoin.
    Rectangle {
        anchors.fill: parent
        visible: !sceneArt.delivered
        color: Qt.rgba(Tokens.panel.r, Tokens.panel.g, Tokens.panel.b, 0.55)
    }

    // --- Panneau des credits (maquette : 385, 170 -> 1285, 825) ------------------------------------
    PanelFrame {
        id: creditsPanel

        x: 442 * Tokens.uiScale
        y: 195 * Tokens.uiScale
        width: 1034 * Tokens.uiScale
        height: 752 * Tokens.uiScale
        padding: 0

        // Les sections defilent quand elles depassent le panneau : la barre se loge dans la marge
        // droite, et les colonnes gardent leur largeur qu'elle soit la ou non -- une largeur qui
        // suivrait la barre relancerait le retour a la ligne, donc la hauteur, donc la barre.
        Flickable {
            id: creditsScroll

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: quote.top
            anchors.leftMargin: 40 * Tokens.uiScale
            anchors.rightMargin: 14 * Tokens.uiScale
            anchors.topMargin: 80 * Tokens.uiScale
            anchors.bottomMargin: Tokens.gapMedium
            clip: true
            contentWidth: width
            contentHeight: creditsColumns.implicitHeight
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: OrnateScrollBar {
                id: creditsScrollBar
            }

            RowLayout {
                id: creditsColumns

                width: creditsScroll.width - 26 * Tokens.uiScale
                spacing: 56 * Tokens.uiScale

                ColumnLayout {
                    Layout.alignment: Qt.AlignTop
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    spacing: Tokens.gapLarge

                    Repeater {
                        model: root.leftSections

                        CreditSection {
                            required property var modelData

                            Layout.fillWidth: true
                            title: modelData.title
                            iconKey: modelData.iconKey
                            lines: modelData.lines
                        }
                    }
                }

                ColumnLayout {
                    Layout.alignment: Qt.AlignTop
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    spacing: Tokens.gapLarge

                    Repeater {
                        model: root.rightSections

                        CreditSection {
                            required property var modelData

                            Layout.fillWidth: true
                            title: modelData.title
                            iconKey: modelData.iconKey
                            lines: modelData.lines
                        }
                    }
                }
            }
        }

        // La citation, au pied du panneau (maquette : 660, 765 -> 990, 792).
        Text {
            id: quote

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 48 * Tokens.uiScale
            text: qsTr("« Une grande aventure ne se fait jamais seul. »")
            color: Tokens.textOnPanel
            font.family: Tokens.loreFamily
            font.italic: true
            font.pixelSize: Tokens.fontBody
        }
    }

    // La plaque de titre chevauche le bord haut du panneau (maquette : 620, 110 -> 1050, 205).
    TitlePlate {
        anchors.horizontalCenter: creditsPanel.horizontalCenter
        y: 126 * Tokens.uiScale
        material: "garnet"
        text: qsTr("Crédits")
    }

    // --- Logotype (maquette : 90, 5 -> 530, 285) --------------------------------------------------
    LogoPlate {
        x: 103 * Tokens.uiScale
        y: 12 * Tokens.uiScale
        width: 450 * Tokens.uiScale
    }

    // --- Retour (maquette : 40, 775 -> 300, 840) ---------------------------------------------------
    OrnateButton {
        id: backControl

        x: 46 * Tokens.uiScale
        y: 890 * Tokens.uiScale
        kind: "back"
        text: qsTr("Retour")
    }

    // --- Version ------------------------------------------------------------------------------------
    Text {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: Tokens.gapLarge
        anchors.bottomMargin: Tokens.gapLarge
        text: "v" + root.version
        color: Tokens.textOnPanel
        font.family: Tokens.bodyFamily
        font.pixelSize: Tokens.fontCaption
        style: Text.Outline
        styleColor: Tokens.panel
    }
}
