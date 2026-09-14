pragma ComponentBehavior: Bound
import QtQuick
import Jadg.Ui

/*!
    Competences et sorts -- FORMULAIRE, cote conception (LOT-87, T3.8 ; maquette 10).

    Transcrit de la maquette `10_Skills_Mockup.png` (1672 x 941). Trois colonnes :

    - a gauche, les attaques du personnage et ses sortileges ;
    - au centre, les huit ecoles de magie, leurs emplacements de sort et les sorts connus ;
    - a droite, le detail du sort choisi.

    **Aucune donnee avant le `LOT-35`** : l'ecran est entierement dessine, et chaque valeur passe par
    `PendingData` (cles `skills.*`). Les huit ecoles, elles, sont celles du lexique des regles
    (`LOT-30`) : ce ne sont pas des donnees de partie, et la maquette en montre une neuvieme,
    « Thaumaturgie », que les regles ne connaissent pas.

    `currentSchool` est l'ecole designee au clavier : une marque d'or la signale (EX-IHM-071).
*/
Item {
    id: root

    /// L'ecole designee, 0 a 7, dans l'ordre de `schools`.
    property int currentSchool: 0

    /// Les huit ecoles du lexique, dans l'ordre du livre.
    readonly property var schools: [
        { schoolId: "abjuration", name: qsTr("Abjuration") },
        { schoolId: "conjuration", name: qsTr("Invocation") },
        { schoolId: "divination", name: qsTr("Divination") },
        { schoolId: "enchantment", name: qsTr("Enchantement") },
        { schoolId: "evocation", name: qsTr("Évocation") },
        { schoolId: "illusion", name: qsTr("Illusion") },
        { schoolId: "necromancy", name: qsTr("Nécromancie") },
        { schoolId: "transmutation", name: qsTr("Transmutation") }
    ]

    /// Par ecole, dans le meme ordre : les emplacements (« 2 / 3 ») et les sorts connus.
    property var schoolSlots: ["—", "—", "—", "—", "—", "—", "—", "—"]
    property var schoolSpells: ["—", "—", "—", "—", "—", "—", "—", "—"]

    property string spellSlots: "—"
    property var attacks: exampleRows
    property var cantrips: exampleRows

    // --- Le sort choisi -----------------------------------------------------------------------------
    property string spellName: "—"
    property string spellSchool: "—"
    property string spellDescription: "—"
    property string spellDamageType: "—"
    property string spellRange: "—"
    property string spellDamage: "—"
    property string spellCastingTime: "—"
    property string spellComponents: "—"
    property string spellEffects: "—"

    readonly property ListModel exampleRows: ListModel {
        ListElement { rowId: "a"; label: "—"; value: "—" }
        ListElement { rowId: "b"; label: "—"; value: "—" }
    }

    property alias sheetButton: sheetControl

    width: 1920
    height: 1080

    Rectangle {
        anchors.fill: parent
        color: Tokens.frameEdge
    }

    // === Colonne gauche : attaques et sortileges (maquette : 30, 80 -> 485, 905) ======================
    PanelFrame {
        x: 34 * Tokens.uiScale
        y: 16 * Tokens.uiScale
        width: 523 * Tokens.uiScale
        height: 1048 * Tokens.uiScale
        material: "parchment"
        bound: true
        padding: 0
    }

    TitlePlate {
        x: 60 * Tokens.uiScale
        y: 16 * Tokens.uiScale
        width: 480 * Tokens.uiScale
        material: "garnet"
        text: qsTr("Compétences")
    }

    Column {
        x: 70 * Tokens.uiScale
        y: 160 * Tokens.uiScale
        width: 460 * Tokens.uiScale
        spacing: Tokens.gapMedium

        SectionBanner {
            width: 260 * Tokens.uiScale
            text: qsTr("Attaques")
        }

        Repeater {
            model: root.attacks

            SpellEntry {
                id: entry1

                required property string label
                required property string value

                width: 460 * Tokens.uiScale
                name: entry1.label
                details: entry1.value
            }
        }

        SectionBanner {
            width: 260 * Tokens.uiScale
            text: qsTr("Sortilèges")
        }

        Repeater {
            model: root.cantrips

            SpellEntry {
                id: entry2

                required property string label
                required property string value

                width: 460 * Tokens.uiScale
                name: entry2.label
                details: entry2.value
            }
        }
    }

    Text {
        x: 70 * Tokens.uiScale
        y: 980 * Tokens.uiScale
        width: 460 * Tokens.uiScale
        text: qsTr("« La maîtrise des arts et des armes fait la force d'un véritable héros. »")
        color: Tokens.textMuted
        font.family: Tokens.loreFamily
        font.italic: true
        font.pixelSize: Tokens.fontCaption
        wrapMode: Text.WordWrap
    }

    // === Colonne centrale : les ecoles (maquette : 495, 90 -> 1150, 900) ================================
    PanelFrame {
        x: 568 * Tokens.uiScale
        y: 16 * Tokens.uiScale
        width: 752 * Tokens.uiScale
        height: 1048 * Tokens.uiScale
        material: "parchment"
        padding: 0
    }

    Item {
        x: 590 * Tokens.uiScale
        y: 110 * Tokens.uiScale
        width: 710 * Tokens.uiScale
        height: 56 * Tokens.uiScale

        SectionBanner {
            anchors.fill: parent
            material: "dark"
            text: qsTr("Sorts")
        }

        Text {
            anchors.right: parent.right
            anchors.rightMargin: 100 * Tokens.uiScale
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("Emplacements de sort : %1").arg(root.spellSlots)
            color: Tokens.textOnPanel
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontBody
        }
    }

    Column {
        x: 590 * Tokens.uiScale
        y: 186 * Tokens.uiScale
        spacing: 12 * Tokens.uiScale

        Repeater {
            model: root.schools

            Row {
                id: schoolRow

                required property var modelData
                required property int index

                spacing: Tokens.gapSmall

                FocusMark {
                    anchors.verticalCenter: parent.verticalCenter
                    opacity: root.currentSchool === schoolRow.index ? 1 : 0
                }

                PanelFrame {
                    width: 678 * Tokens.uiScale
                    height: 96 * Tokens.uiScale
                    subpanel: true
                    padding: Tokens.gapSmall

                    Item {
                        id: schoolIcon

                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        width: 72 * Tokens.uiScale
                        height: 72 * Tokens.uiScale

                        Rectangle {
                            anchors.fill: parent
                            visible: !schoolArt.delivered
                            radius: width / 2
                            color: "transparent"
                            border.color: root.currentSchool === schoolRow.index ? Tokens.goldLight : Tokens.panelEdge
                            border.width: Tokens.strokeWidth
                        }

                        FixedArt {
                            id: schoolArt

                            anchors.fill: parent
                            key: "ui/icon/school/" + schoolRow.modelData.schoolId
                        }
                    }

                    Column {
                        anchors.left: schoolIcon.right
                        anchors.leftMargin: Tokens.gapMedium
                        anchors.verticalCenter: parent.verticalCenter
                        width: 200 * Tokens.uiScale

                        Text {
                            text: schoolRow.modelData.name
                            color: root.currentSchool === schoolRow.index ? Tokens.goldLight : Tokens.textOnPanel
                            font.family: Tokens.titleFamily
                            font.pixelSize: Tokens.fontSectionTitle
                        }

                        Text {
                            text: root.schoolSlots[schoolRow.index]
                            color: Tokens.textOnPanelMuted
                            font.family: Tokens.bodyFamily
                            font.pixelSize: Tokens.fontBody
                        }
                    }

                    Text {
                        anchors.right: parent.right
                        anchors.left: parent.left
                        anchors.leftMargin: 300 * Tokens.uiScale
                        anchors.verticalCenter: parent.verticalCenter
                        text: root.schoolSpells[schoolRow.index]
                        color: Tokens.textOnPanel
                        font.family: Tokens.bodyFamily
                        font.pixelSize: Tokens.fontBody
                        elide: Text.ElideRight
                    }
                }
            }
        }
    }

    // === Colonne droite : le detail du sort (maquette : 1160, 90 -> 1640, 905) ==========================
    PanelFrame {
        x: 1332 * Tokens.uiScale
        y: 16 * Tokens.uiScale
        width: 552 * Tokens.uiScale
        height: 1048 * Tokens.uiScale
        material: "parchment"
        padding: 0
    }

    OrnateButton {
        id: sheetControl

        x: 1540 * Tokens.uiScale
        y: 30 * Tokens.uiScale
        kind: "back"
        width: 320 * Tokens.uiScale
        text: qsTr("Fiche")
    }

    Column {
        x: 1360 * Tokens.uiScale
        y: 116 * Tokens.uiScale
        width: 496 * Tokens.uiScale
        spacing: Tokens.gapMedium

        SectionBanner {
            width: parent.width
            material: "dark"
            text: qsTr("Détail du sort")
        }

        Row {
            spacing: Tokens.gapMedium

            PortraitFrame {
                shape: "square"
                size: 140 * Tokens.uiScale
            }

            Column {
                width: 330 * Tokens.uiScale

                Text {
                    width: parent.width
                    text: root.spellName
                    color: Tokens.text
                    font.family: Tokens.titleFamily
                    font.pixelSize: Tokens.fontSectionTitle
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: root.spellSchool
                    color: Tokens.textMuted
                    font.family: Tokens.bodyFamily
                    font.pixelSize: Tokens.fontBody
                }

                Text {
                    width: parent.width
                    text: root.spellDescription
                    color: Tokens.text
                    font.family: Tokens.bodyFamily
                    font.pixelSize: Tokens.fontCaption
                    wrapMode: Text.WordWrap
                }
            }
        }

        GoldDivider {
            width: parent.width
        }

        FieldRow { width: parent.width; tagWidth: 220 * Tokens.uiScale; label: qsTr("Type de dégâts"); value: root.spellDamageType }
        FieldRow { width: parent.width; tagWidth: 220 * Tokens.uiScale; label: qsTr("Portée"); value: root.spellRange }
        FieldRow { width: parent.width; tagWidth: 220 * Tokens.uiScale; label: qsTr("Dégâts"); value: root.spellDamage }
        FieldRow { width: parent.width; tagWidth: 220 * Tokens.uiScale; label: qsTr("Incantation"); value: root.spellCastingTime }
        FieldRow { width: parent.width; tagWidth: 220 * Tokens.uiScale; label: qsTr("Composantes"); value: root.spellComponents }

        SectionBanner {
            width: parent.width
            text: qsTr("Effets spéciaux")
        }

        Text {
            width: parent.width
            text: root.spellEffects
            color: Tokens.text
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontBody
            wrapMode: Text.WordWrap
        }
    }
}
