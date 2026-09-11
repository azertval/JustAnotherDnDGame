import QtQuick
import QtQuick.Controls
import "../components"

Item {
    id: root
    property color paper: "#e7d5a7"
    Rectangle { anchors.fill: parent; color: root.paper }

    // faint map / parchment watermark
    Text { anchors.centerIn: parent; text: "JADG"; color: "#8c774f"; opacity: 0.045; font.family: "Georgia"; font.bold: true; font.pixelSize: 220 }

    Row {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 12

        OrnatePanel {
            width: parent.width * .29; height: parent.height
            Column {
                anchors.fill: parent; anchors.margins: 18; spacing: 10
                TitlePlaque { width: parent.width; title: "ATTAQUES & SORTILÈGES" }
                Text { text: "ATTAQUES"; font.bold: true; font.pixelSize: 16; color: "#2b2115" }
                SkillRow { width: parent.width; skillName: "Épée longue"; damageType: "Tranchant"; rangeText: "1,5 m"; damageText: "1d8 + 3"; effectText: "—" }
                SkillRow { width: parent.width; skillName: "Arc long"; damageType: "Perforant"; rangeText: "45 / 180 m"; damageText: "1d8 + 3"; effectText: "Cible en mouvement" }
                Text { text: "SORTILÈGES"; font.bold: true; font.pixelSize: 16; color: "#2b2115" }
                SkillRow { width: parent.width; skillName: "Projectile magique"; damageType: "Force"; rangeText: "120 m"; damageText: "1d10"; effectText: "—"; iconSource: "../../assets/icons/spell.svg" }
                SkillRow { width: parent.width; skillName: "Soin"; damageType: "Guérison"; rangeText: "Contact"; damageText: "1d8 + 4"; effectText: "Récupération"; iconSource: "../../assets/icons/heal.svg" }
            }
        }

        OrnatePanel {
            width: parent.width * .40; height: parent.height
            Column {
                anchors.fill: parent; anchors.margins: 18; spacing: 6
                TitlePlaque { width: parent.width; title: "SORTS • 9 CATÉGORIES" }
                SpellCategory { width: parent.width; categoryName: "Abjuration"; maximumUses: 3; remainingUses: 3; spells: ["Bouclier","Protection","Lien de force"] }
                SpellCategory { width: parent.width; categoryName: "Invocation"; maximumUses: 2; remainingUses: 2; spells: ["Flammes nécrotiques","Invisibilité"] }
                SpellCategory { width: parent.width; categoryName: "Divination"; maximumUses: 2; remainingUses: 2; spells: ["Détection de la magie","Clairvoyance"] }
                SpellCategory { width: parent.width; categoryName: "Enchantement"; maximumUses: 1; remainingUses: 1; spells: ["Charme-personne"] }
                SpellCategory { width: parent.width; categoryName: "Évocation"; maximumUses: 2; remainingUses: 1; spells: ["Boule de feu","Météore"] }
                SpellCategory { width: parent.width; categoryName: "Illusion"; maximumUses: 1; remainingUses: 1; spells: ["Image miroir"] }
                SpellCategory { width: parent.width; categoryName: "Nécromancie"; maximumUses: 2; remainingUses: 1; spells: ["Main spectrale"] }
                SpellCategory { width: parent.width; categoryName: "Transmutation"; maximumUses: 1; remainingUses: 0; spells: ["Métamorphose"] }
                SpellCategory { width: parent.width; categoryName: "Thaumaturgie"; maximumUses: 1; remainingUses: 1; spells: ["Lumière"] }
            }
        }

        OrnatePanel {
            width: parent.width * .27; height: parent.height
            Column {
                anchors.fill: parent; anchors.margins: 20; spacing: 10
                TitlePlaque { width: parent.width; title: "DÉTAIL DE LA COMPÉTENCE" }
                Image { width: 92; height: 92; source: "../../assets/icons/fire.svg"; anchors.horizontalCenter: parent.horizontalCenter }
                Text { text: "Boule de feu"; font.pixelSize: 25; font.bold: true; color: "#251b10"; anchors.horizontalCenter: parent.horizontalCenter }
                Text { text: "Évocation — Niveau 3"; font.pixelSize: 14; color: "#62451f"; anchors.horizontalCenter: parent.horizontalCenter }
                DetailStat { width: parent.width; label: "Type de dégâts"; value: "Feu" }
                DetailStat { width: parent.width; label: "Portée"; value: "150 m • rayon 6 m" }
                DetailStat { width: parent.width; label: "Dégâts"; value: "8d6" }
                DetailStat { width: parent.width; label: "Incantation"; value: "1 action" }
                DetailStat { width: parent.width; label: "Composants"; value: "V, S, M" }
                Text { width: parent.width; text: "Effets spéciaux"; font.bold: true; font.pixelSize: 16; color: "#3b2a18" }
                Text { width: parent.width; text: "• Inflige des dégâts de feu.\n• Peut enflammer les objets inflammables.\n• Jet de DEX pour réduire les dégâts."; wrapMode: Text.WordWrap; font.pixelSize: 13; color: "#3b2a18" }
            }
        }
    }
}
