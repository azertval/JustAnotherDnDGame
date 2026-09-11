import QtQuick
Item {
    id: root
    width: 1600
    height: 900
    property string playerName: "Kaelith"
    signal continueGame()
    signal newGame()
    signal loadGame()
    signal openOptions()
    signal openCredits()
    signal quitGame()

    Rectangle { anchors.fill: parent; color: "#080706" }
    Image { anchors.fill: parent; source: "qrc:/JustAnotherDnDGame/svg/background/menu_dark_overlay.svg" }
    Image { anchors.fill: parent; source: "qrc:/JustAnotherDnDGame/svg/background/vignette_overlay.svg" }

    Image {
        x: 55; y: 42; width: 570; height: 250
        source: "qrc:/JustAnotherDnDGame/svg/branding/logo_plate.svg"
    }
    Text {
        x: 92; y: 102; width: 490
        text: "JUST ANOTHER"
        horizontalAlignment: Text.AlignHCenter
        color: "#ead8a7"
        font.family: "Cinzel"; font.pixelSize: 35
    }
    Text {
        x: 75; y: 145; width: 525
        text: "D&D GAME"
        horizontalAlignment: Text.AlignHCenter
        color: "#e3c56b"
        font.family: "Cinzel"; font.bold: true; font.pixelSize: 58
    }

    Column {
        x: 90; y: 330; width: 440; spacing: 9
        MainMenuButton { label: "Continuer"; active: true; iconSource: "qrc:/JustAnotherDnDGame/svg/icons/continue.svg"; onClicked: root.continueGame() }
        MainMenuButton { label: "Nouvelle partie"; iconSource: "qrc:/JustAnotherDnDGame/svg/icons/new_game.svg"; onClicked: root.newGame() }
        MainMenuButton { label: "Charger une partie"; iconSource: "qrc:/JustAnotherDnDGame/svg/icons/load.svg"; onClicked: root.loadGame() }
        MainMenuButton { label: "Options"; iconSource: "qrc:/JustAnotherDnDGame/svg/icons/settings.svg"; onClicked: root.openOptions() }
        MainMenuButton { label: "Crédits"; iconSource: "qrc:/JustAnotherDnDGame/svg/icons/credits.svg"; onClicked: root.openCredits() }
        MainMenuButton { label: "Quitter"; iconSource: "qrc:/JustAnotherDnDGame/svg/icons/quit.svg"; onClicked: root.quitGame() }
    }

    MainMenuProfile {
        x: width - 375; y: 35
        playerName: root.playerName
        avatarSource: "qrc:/JustAnotherDnDGame/svg/icons/profile.svg"
    }

    Text {
        x: 55; y: height - 68
        text: "« Chaque choix façonne une nouvelle histoire. »"
        color: "#cdbb91"
        font.family: "IM Fell English"
        font.italic: true
        font.pixelSize: 17
    }
    Text {
        anchors.right: parent.right; anchors.bottom: parent.bottom
        anchors.rightMargin: 35; anchors.bottomMargin: 25
        text: "v0.1.0"
        color: "#b9a77d"
        font.family: "Cinzel"; font.pixelSize: 14
    }
}