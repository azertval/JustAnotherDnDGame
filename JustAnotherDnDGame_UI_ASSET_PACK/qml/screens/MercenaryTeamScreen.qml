import QtQuick
import "../components"

Item {
    id: root

    // Main integration hooks
    property string teamName: "Les Lames d’Acier"
    property string motto: "Plus forts ensemble."
    property string rank: "Mercenaires confirmés"
    property string specialization: "Tactique & Polyvalence"
    property string style: "Équilibré"

    Rectangle {
        anchors.fill: parent
        color: "#d9c89e"
    }

    // Paper texture / map-like watermark
    Text {
        anchors.centerIn: parent
        text: "JADG"
        color: "#8c774f"
        opacity: 0.055
        font.family: "Georgia"
        font.bold: true
        font.pixelSize: Math.min(root.width, root.height) * 0.22
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: 12
        color: "transparent"
        border.color: "#8d671c"
        border.width: 2
        radius: 8
    }

    // Header
    Rectangle {
        x: 26; y: 22
        width: root.width - 52
        height: 58
        color: "#5a1812"
        border.color: "#b88b2a"
        border.width: 2
        radius: 7

        Text {
            x: 20
            anchors.verticalCenter: parent.verticalCenter
            text: "ÉQUIPE DE MERCENAIRES"
            color: "#f4e8c5"
            font.family: "Georgia"
            font.bold: true
            font.pixelSize: 25
        }

        Row {
            anchors.right: parent.right
            anchors.rightMargin: 18
            anchors.verticalCenter: parent.verticalCenter
            spacing: 8

            Repeater {
                model: ["ÉQUIPE", "RECRUTEMENT", "CONTRATS", "RÉSERVE"]
                delegate: Rectangle {
                    width: 104
                    height: 31
                    color: index === 0 ? "#741d15" : "#30271c"
                    border.color: "#aa8127"
                    border.width: 1
                    radius: 5
                    Text {
                        anchors.centerIn: parent
                        text: modelData
                        color: "#eadbb6"
                        font.family: "Georgia"
                        font.bold: true
                        font.pixelSize: 10
                    }
                }
            }
        }
    }

    // Three-column layout
    Row {
        x: 26; y: 98
        width: root.width - 52
        height: root.height - 122
        spacing: 18

        // LEFT
        Item {
            width: parent.width * 0.29
            height: parent.height

            OrnatePanel {
                anchors.top: parent.top
                width: parent.width
                height: 235
                title: "ÉQUIPE"

                Rectangle {
                    x: 16; y: 22
                    width: 76; height: 76
                    color: "#641914"
                    border.color: "#c29a34"
                    border.width: 2
                    radius: 5
                    Text {
                        anchors.centerIn: parent
                        text: "⚔"
                        color: "#e9d6a2"
                        font.pixelSize: 36
                    }
                }

                Text {
                    x: 108; y: 31
                    text: root.teamName
                    color: "#3b2b1c"
                    font.family: "Georgia"
                    font.bold: true
                    font.pixelSize: 17
                }
                Text {
                    x: 108; y: 57
                    text: "« " + root.motto + " »"
                    color: "#806b48"
                    font.italic: true
                    font.pixelSize: 11
                }

                Row {
                    x: 14; y: 118
                    spacing: 6
                    StatBadge { label: "CARRIÈRE"; value: "1 250"; iconText: "✦" }
                    StatBadge { label: "NIVEAU"; value: "3"; iconText: "⬟" }
                    StatBadge { label: "PRESTIGE"; value: "12"; iconText: "♛" }
                    StatBadge { label: "FAME"; value: "8"; iconText: "★" }
                }
            }

            OrnatePanel {
                anchors.top: parent.top
                anchors.topMargin: 252
                width: parent.width
                height: parent.height - 252
                title: "MEMBRES DE L’ÉQUIPE"

                Grid {
                    x: 16; y: 26
                    width: parent.width - 32
                    columns: 3
                    rowSpacing: 8
                    columnSpacing: 2

                    MemberToken { name: "Kaelith Voss"; role: "Chef"; level: "5"; initials: "KV" }
                    MemberToken { name: "Brom Draknar"; role: "Guerrier"; level: "4"; initials: "BD" }
                    MemberToken { name: "Seraphine du Val"; role: "Éclaireuse"; level: "4"; initials: "SV" }
                    MemberToken { name: "Jax le Rouge"; role: "Rôdeur"; level: "3"; initials: "JR" }
                    MemberToken { name: "Emplacement libre"; role: "Recruter"; level: "+"; initials: "+" }
                }
            }
        }

        // CENTER
        Item {
            width: parent.width * 0.43
            height: parent.height

            OrnatePanel {
                anchors.top: parent.top
                width: parent.width
                height: 275
                title: "BASE PRINCIPALE"

                Rectangle {
                    x: 14; y: 25
                    width: parent.width - 28
                    height: 155
                    color: "#cdbd92"
                    border.color: "#9a7020"
                    border.width: 1
                    radius: 4

                    Text {
                        anchors.centerIn: parent
                        text: "FORT DE LA BRUME"
                        color: "#6d5735"
                        font.family: "Georgia"
                        font.bold: true
                        font.pixelSize: 22
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        y: parent.height - 31
                        text: "Illustration / vue de la base"
                        color: "#8b7650"
                        font.pixelSize: 10
                    }
                }

                Text {
                    x: 20; y: 190
                    text: "ÉTAT DU BÂTIMENT"
                    color: "#4b3822"
                    font.family: "Georgia"
                    font.bold: true
                    font.pixelSize: 12
                }

                Rectangle {
                    x: 20; y: 214
                    width: parent.width - 40
                    height: 14
                    color: "#30271d"
                    border.color: "#86631e"
                    radius: 7

                    Rectangle {
                        width: parent.width * 0.78
                        height: parent.height
                        color: "#8a2018"
                        radius: 7
                    }
                }

                Text {
                    anchors.right: parent.right
                    anchors.rightMargin: 20
                    y: 234
                    text: "780 / 1000 PV   •   Entretien : 120 / jour"
                    color: "#705b39"
                    font.pixelSize: 9
                }
            }

            OrnatePanel {
                anchors.top: parent.top
                anchors.topMargin: 292
                width: parent.width
                height: 250
                title: "BÂTIMENTS CONSTRUITS"

                Grid {
                    x: 14; y: 27
                    width: parent.width - 28
                    columns: 3
                    rows: 2
                    columnSpacing: 9
                    rowSpacing: 9

                    BuildingCard { width: (parent.width - 18) / 3; height: 92; name: "Quartiers"; level: "Niveau 2" }
                    BuildingCard { width: (parent.width - 18) / 3; height: 92; name: "Forge"; level: "Niveau 1" }
                    BuildingCard { width: (parent.width - 18) / 3; height: 92; name: "Salle de stratégie"; level: "Niveau 1" }
                    BuildingCard { width: (parent.width - 18) / 3; height: 92; name: "Entrepôt"; level: "Niveau 1" }
                    BuildingCard { width: (parent.width - 18) / 3; height: 92; name: "Écuries"; level: "Niveau 1" }
                    BuildingCard { width: (parent.width - 18) / 3; height: 92; name: "Emplacement libre"; locked: true }
                }
            }

            OrnatePanel {
                anchors.top: parent.top
                anchors.topMargin: 559
                width: parent.width
                height: parent.height - 559
                title: "PERSONNEL DE LA BASE"

                MemberToken { x: 15; y: 28; name: "Garrick"; role: "Butler • Niv. 2"; level: "2"; initials: "G" }
                MemberToken { x: 135; y: 28; name: "Forgeron"; role: "Staff • Niv. 1"; level: "1"; initials: "F" }
                MemberToken { x: 255; y: 28; name: "Intendante"; role: "Staff • Niv. 1"; level: "1"; initials: "I" }
                MemberToken { x: 375; y: 28; name: "Soigneur"; role: "Staff • Niv. 1"; level: "1"; initials: "S" }
            }
        }

        // RIGHT
        Item {
            width: parent.width * 0.24
            height: parent.height

            OrnatePanel {
                anchors.top: parent.top
                width: parent.width
                height: 345
                title: "HAUTS FAITS"

                Column {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: 28
                    spacing: 4

                    AchievementRow { width: parent.width; title: "Premiers pas"; description: "Réussir le premier contrat"; date: "12/04/25" }
                    AchievementRow { width: parent.width; title: "Équipe soudée"; description: "Atteindre le niveau 2"; date: "28/04/25" }
                    AchievementRow { width: parent.width; title: "Gloire au combat"; description: "Remporter 10 batailles"; date: "15/05/25" }
                    AchievementRow { width: parent.width; title: "Réseau d’influence"; description: "Rencontrer un contact important"; date: "03/06/25" }
                    AchievementRow { width: parent.width; title: "Légende en marche"; description: "Atteindre le prestige 3"; date: "22/06/25" }
                }
            }

            OrnatePanel {
                anchors.top: parent.top
                anchors.topMargin: 363
                width: parent.width
                height: 185
                title: "SPÉCIALISATION & STYLE"

                Text {
                    x: 22; y: 34
                    text: "Spécialisation"
                    color: "#816c48"
                    font.pixelSize: 10
                }
                Text {
                    x: 22; y: 52
                    text: root.specialization
                    color: "#3c2d1c"
                    font.family: "Georgia"
                    font.bold: true
                    font.pixelSize: 13
                }

                Text {
                    x: 22; y: 82
                    text: "Style"
                    color: "#816c48"
                    font.pixelSize: 10
                }
                Text {
                    x: 22; y: 100
                    text: root.style
                    color: "#3c2d1c"
                    font.family: "Georgia"
                    font.bold: true
                    font.pixelSize: 13
                }

                Text {
                    x: 22; y: 130
                    text: "Rang"
                    color: "#816c48"
                    font.pixelSize: 10
                }
                Text {
                    x: 22; y: 148
                    text: root.rank
                    color: "#6b1712"
                    font.family: "Georgia"
                    font.bold: true
                    font.pixelSize: 12
                }
            }

            OrnatePanel {
                anchors.top: parent.top
                anchors.topMargin: 566
                width: parent.width
                height: parent.height - 566
                title: "TRÉSORS DE LA BASE"

                Row {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: 32
                    anchors.leftMargin: 10
                    spacing: 6

                    ResourceTile { icon: "◉"; amount: "2 450"; label: "OR" }
                    ResourceTile { icon: "◆"; amount: "320"; label: "GEMMES" }
                    ResourceTile { icon: "▤"; amount: "18"; label: "LINGOTS" }
                    ResourceTile { icon: "⚿"; amount: "6"; label: "RELIQUES" }
                }
            }
        }
    }
}
