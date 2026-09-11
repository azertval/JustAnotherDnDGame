import QtQuick
Item {
    id: root
    width: 1600
    height: 900
    signal back()

    Rectangle { anchors.fill: parent; color: "#080706" }
    Image { anchors.fill: parent; source: "qrc:/JustAnotherDnDGame/svg/background/vignette_overlay.svg" }

    Image {
        x: 330; y: 70; width: 940; height: 730
        source: "qrc:/JustAnotherDnDGame/svg/frame/credits_panel.svg"
    }

    Image {
        x: 590; y: 75; width: 420; height: 150
        source: "qrc:/JustAnotherDnDGame/svg/frame/title_plate.svg"
    }
    Text {
        x: 610; y: 118; width: 380
        text: "CRÉDITS"
        horizontalAlignment: Text.AlignHCenter
        color: "#ead8a7"
        font.family: "Cinzel"
        font.bold: true
        font.pixelSize: 36
    }
    Text {
        x: 520; y: 190; width: 560
        text: "JUST ANOTHER D&D GAME"
        horizontalAlignment: Text.AlignHCenter
        color: "#b58a32"
        font.family: "Cinzel"
        font.pixelSize: 15
        font.letterSpacing: 2
    }

    Column {
        x: 450; y: 270; width: 700; spacing: 10
        CreditsEntry { role: "GAME DESIGN"; names: "Équipe JustAnotherDnDGame" }
        CreditsEntry { role: "PROGRAMMATION"; names: "Qt / C++ / QML" }
        CreditsEntry { role: "UI / UX"; names: "Direction artistique & interface" }
        CreditsEntry { role: "GAMEPLAY"; names: "Système RPG tactique" }
        CreditsEntry { role: "ART"; names: "Univers fantasy & illustrations" }
        CreditsEntry { role: "MUSIQUE / AUDIO"; names: "Ambiances & effets sonores" }
        CreditsEntry { role: "TESTS"; names: "Communauté & contributeurs" }
    }

    Image {
        x: 650; y: 685; width: 100; height: 100
        source: "qrc:/JustAnotherDnDGame/svg/icons/sword_shield.svg"
    }

    Text {
        x: 460; y: 750; width: 680
        text: "Merci à tous ceux qui ont contribué à l'aventure."
        horizontalAlignment: Text.AlignHCenter
        color: "#cdbb91"
        font.family: "IM Fell English"
        font.italic: true
        font.pixelSize: 17
    }

    Image {
        x: 70; y: 800; width: 320; height: 70
        source: "qrc:/JustAnotherDnDGame/svg/navigation/back_button.svg"
    }
    Text {
        x: 130; y: 820
        text: "RETOUR"
        color: "#ead8a7"
        font.family: "Cinzel"
        font.pixelSize: 17
    }
    MouseArea { x: 70; y: 800; width: 320; height: 70; onClicked: root.back() }
}