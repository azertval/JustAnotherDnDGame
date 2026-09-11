import QtQuick
Item {
    id: root
    width: 1280
    height: 720

    Rectangle { anchors.fill: parent; color: "#090806" }
    Image {
        anchors.fill: parent
        source: "qrc:/MercenaryRpgUiKit/assets/options/background/overlay.svg"
        fillMode: Image.Stretch
    }

    Image {
        anchors.centerIn: parent
        width: parent.width * 0.92
        height: parent.height * 0.88
        source: "qrc:/MercenaryRpgUiKit/assets/options/frame/options_panel.svg"
        fillMode: Image.Stretch
    }

    Text {
        x: 70; y: 54
        text: "OPTIONS"
        color: "#ead8a7"
        font.family: "Cinzel"
        font.bold: true
        font.pixelSize: 30
    }

    Column {
        x: 70; y: 125; width: 285; spacing: 10
        OptionsTab { label: "Général"; active: true; iconSource: "qrc:/MercenaryRpgUiKit/assets/options/icons/general.svg" }
        OptionsTab { label: "Graphismes"; iconSource: "qrc:/MercenaryRpgUiKit/assets/options/icons/graphics.svg" }
        OptionsTab { label: "Audio"; iconSource: "qrc:/MercenaryRpgUiKit/assets/options/icons/audio.svg" }
        OptionsTab { label: "Jeu"; iconSource: "qrc:/MercenaryRpgUiKit/assets/options/icons/game.svg" }
        OptionsTab { label: "Commandes"; iconSource: "qrc:/MercenaryRpgUiKit/assets/options/icons/controls.svg" }
        OptionsTab { label: "Accessibilité"; iconSource: "qrc:/MercenaryRpgUiKit/assets/options/icons/accessibility.svg" }
    }

    Text {
        x: 390; y: 115
        text: "Général"
        color: "#e3c56b"
        font.family: "Cinzel"
        font.pixelSize: 23
    }

    OptionsSection {
        x: 390; y: 155; width: 390; height: 190
        title: "Langue"
        iconSource: "qrc:/MercenaryRpgUiKit/assets/options/icons/language.svg"
        OptionsCheckBox { y: 12; label: "Français"; checked: true }
        OptionsCheckBox { y: 52; label: "Sous-titres"; checked: true }
    }

    OptionsSection {
        x: 800; y: 155; width: 390; height: 190
        title: "Difficulté"
        iconSource: "qrc:/MercenaryRpgUiKit/assets/options/icons/difficulty.svg"
        OptionsCheckBox { y: 12; label: "Mode tactique"; checked: true }
        OptionsCheckBox { y: 52; label: "Missions secondaires"; checked: true }
    }

    OptionsSection {
        x: 390; y: 365; width: 390; height: 205
        title: "Interface"
        iconSource: "qrc:/MercenaryRpgUiKit/assets/options/icons/general.svg"
        OptionsCheckBox { y: 10; label: "Afficher les tutoriels"; checked: true }
        OptionsCheckBox { y: 48; label: "Afficher les infobulles"; checked: true }
        OptionsCheckBox { y: 86; label: "Toujours afficher la mini-carte" }
        OptionsCheckBox { y: 124; label: "Afficher les dégâts"; checked: true }
    }

    OptionsSection {
        x: 800; y: 365; width: 390; height: 205
        title: "Réseau"
        iconSource: "qrc:/MercenaryRpgUiKit/assets/options/icons/network.svg"
        OptionsCheckBox { y: 10; label: "Mode solo"; checked: true }
        OptionsCheckBox { y: 48; label: "Invitations"; enabled: false }
    }

    OptionsSection {
        x: 390; y: 590; width: 390; height: 95
        title: "Sauvegarde"
        iconSource: "qrc:/MercenaryRpgUiKit/assets/options/icons/save.svg"
        OptionsCheckBox { y: 8; label: "Sauvegarde automatique"; checked: true }
    }

    OptionsSection {
        x: 800; y: 590; width: 390; height: 95
        title: "Confidentialité"
        iconSource: "qrc:/MercenaryRpgUiKit/assets/options/icons/privacy.svg"
        OptionsCheckBox { y: 8; label: "Données statistiques"; checked: true }
    }

    OptionsButton { x: 390; y: 675; label: "Par défaut"; primary: false }
    OptionsButton { x: 1000; y: 675; label: "Appliquer"; primary: true }
}
