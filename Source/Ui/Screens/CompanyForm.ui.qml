pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import Jadg.Ui

/*!
    Equipe de mercenaires -- FORMULAIRE, cote conception (LOT-87, T3.7 ; maquette 09).

    Remplace la feuille d'equipe (`TeamSheetForm`, LOT-38) et le tableau de la Guilde
    (`GuildBoardForm`) : une compagnie se lit et se gere au meme endroit, en quatre onglets.

    - **Equipe**, celui de la maquette : blason et devise, quatre compteurs, membres, base principale,
      batiments, personnel, hauts faits, specialisation et tresors ;
    - **Recrutement** et **Reserve** : les mercenaires a engager et ceux au repos ;
    - **Contrats** : ce que la Guilde propose, et la fiche du contrat choisi.

    Presque tout attend les lots de la compagnie (`LOT-45`, `LOT-83`) et passe par `PendingData`
    (cles `company.*`) : le jumeau pose des modeles de lignes aux roles `rowId`, `label`, `value`.
    Les membres, eux, viennent des personnages existants (`members`).

    Les proprietes portent des VALEURS D'EXEMPLE ; le jumeau les remplace.
*/
Item {
    id: root

    /// L'onglet affiche : 0 Equipe, 1 Recrutement, 2 Contrats, 3 Reserve.
    property int currentTab: 0

    // --- Identite de la compagnie ---------------------------------------------------------------------
    property string companyName: "—"
    property string motto: "—"
    property url coatOfArms: ""
    property string careerPoints: "—"
    property string companyLevel: "—"
    property string prestige: "—"
    property string fame: "—"

    /// Les six places de l'equipe : `{name, level, portrait}` ; un nom vide est une place libre.
    property var members: [
        { name: "Brenna Pierrefonte", level: "3", portrait: "" },
        { name: "", level: "", portrait: "" }
    ]

    // --- Base ---------------------------------------------------------------------------------------
    property string baseName: "—"
    property string baseLevel: "—"
    property string baseDescription: "—"
    property string baseCondition: "—"
    property real baseConditionRatio: 0
    property var buildings: exampleRows
    property var staff: exampleRows

    // --- Colonne droite -------------------------------------------------------------------------------
    property var achievements: exampleRows
    property string specialization: "—"
    property string style: "—"
    property string rank: "—"
    property real rankRatio: 0
    property var treasures: exampleRows

    // --- Onglets Recrutement, Contrats, Reserve ----------------------------------------------------------
    property var recruits: exampleRows
    property var contracts: exampleRows
    property string contract: "—"
    property string contractGiver: "—"
    property string contractRank: "—"
    property string contractReward: "—"
    property var reserve: exampleRows

    readonly property ListModel exampleRows: ListModel {
        ListElement { rowId: "a"; label: "—"; value: "—" }
        ListElement { rowId: "b"; label: "—"; value: "—" }
        ListElement { rowId: "c"; label: "—"; value: "—" }
    }

    property alias teamTab: teamTabControl
    property alias recruitmentTab: recruitmentTabControl
    property alias contractsTab: contractsTabControl
    property alias reserveTab: reserveTabControl

    width: 1920
    height: 1080

    Rectangle {
        anchors.fill: parent
        color: Tokens.frameEdge
    }

    PanelFrame {
        x: 16 * Tokens.uiScale
        y: 12 * Tokens.uiScale
        width: 1888 * Tokens.uiScale
        height: 1056 * Tokens.uiScale
        material: "parchment"
        bound: true
        padding: 0
    }

    // --- En-tete : fanion, plaque de titre, onglets (maquette : 30, 15 -> 1535, 85) ----------------------
    FixedArt {
        x: 40 * Tokens.uiScale
        y: 12 * Tokens.uiScale
        width: 104 * Tokens.uiScale
        height: 130 * Tokens.uiScale
        key: "ui/ornament/crest-pennant"
    }

    TitlePlate {
        x: 150 * Tokens.uiScale
        y: 12 * Tokens.uiScale
        width: 660 * Tokens.uiScale
        material: "garnet"
        text: qsTr("Équipe de mercenaires")
    }

    Row {
        x: 880 * Tokens.uiScale
        y: 40 * Tokens.uiScale
        spacing: Tokens.gapSmall

        OrnateTab {
            id: teamTabControl
            width: 240 * Tokens.uiScale
            text: qsTr("Équipe")
            checkable: false
            checked: root.currentTab === 0
            focusPolicy: Qt.NoFocus
        }
        OrnateTab {
            id: recruitmentTabControl
            width: 240 * Tokens.uiScale
            text: qsTr("Recrutement")
            checkable: false
            checked: root.currentTab === 1
            focusPolicy: Qt.NoFocus
        }
        OrnateTab {
            id: contractsTabControl
            width: 240 * Tokens.uiScale
            text: qsTr("Contrats")
            checkable: false
            checked: root.currentTab === 2
            focusPolicy: Qt.NoFocus
        }
        OrnateTab {
            id: reserveTabControl
            width: 240 * Tokens.uiScale
            text: qsTr("Réserve")
            checkable: false
            checked: root.currentTab === 3
            focusPolicy: Qt.NoFocus
        }
    }

    StackLayout {
        x: 56 * Tokens.uiScale
        y: 150 * Tokens.uiScale
        width: 1808 * Tokens.uiScale
        height: 890 * Tokens.uiScale
        currentIndex: root.currentTab

        // ===== Onglet Equipe ===========================================================================
        Item {
            // --- Colonne gauche : blason, compteurs, membres (maquette : 50, 140 -> 470, 900) --------------
            Column {
                width: 480 * Tokens.uiScale
                spacing: Tokens.gapMedium

                PanelFrame {
                    width: parent.width
                    height: 150 * Tokens.uiScale
                    subpanel: true
                    padding: Tokens.gapMedium

                    PortraitFrame {
                        id: arms

                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        shape: "square"
                        size: 110 * Tokens.uiScale
                        source: root.coatOfArms
                    }

                    Column {
                        anchors.left: arms.right
                        anchors.right: parent.right
                        anchors.leftMargin: Tokens.gapMedium
                        anchors.verticalCenter: parent.verticalCenter

                        Text {
                            width: parent.width
                            text: root.companyName
                            color: Tokens.goldLight
                            font.family: Tokens.titleFamily
                            font.pixelSize: Tokens.fontSectionTitle
                            elide: Text.ElideRight
                        }

                        Text {
                            width: parent.width
                            text: root.motto
                            color: Tokens.textOnPanel
                            font.family: Tokens.loreFamily
                            font.italic: true
                            font.pixelSize: Tokens.fontBody
                            elide: Text.ElideRight
                        }
                    }
                }

                Row {
                    spacing: 4 * Tokens.uiScale

                    StatMedallion { kind: "derived"; label: qsTr("CARRIÈRE"); value: root.careerPoints }
                    StatMedallion { kind: "derived"; label: qsTr("NIVEAU"); value: root.companyLevel }
                    StatMedallion { kind: "derived"; label: qsTr("PRESTIGE"); value: root.prestige }
                    StatMedallion { kind: "derived"; label: qsTr("RENOMMÉE"); value: root.fame }
                }

                SectionBanner {
                    width: parent.width
                    text: qsTr("Membres de l'équipe")
                }

                Grid {
                    columns: 3
                    columnSpacing: 30 * Tokens.uiScale
                    rowSpacing: Tokens.gapMedium

                    Repeater {
                        model: root.members

                        Column {
                            id: member

                            required property var modelData

                            width: 140 * Tokens.uiScale
                            spacing: 4 * Tokens.uiScale

                            PortraitFrame {
                                anchors.horizontalCenter: parent.horizontalCenter
                                shape: "hud"
                                size: 120 * Tokens.uiScale
                                level: member.modelData.level
                                source: member.modelData.portrait
                            }

                            Text {
                                width: parent.width
                                horizontalAlignment: Text.AlignHCenter
                                text: member.modelData.name.length > 0 ? member.modelData.name : qsTr("Place libre")
                                color: member.modelData.name.length > 0 ? Tokens.text : Tokens.textMuted
                                font.family: Tokens.bodyFamily
                                font.pixelSize: Tokens.fontCaption
                                wrapMode: Text.WordWrap
                            }
                        }
                    }
                }
            }

            // --- Colonne centrale : base, batiments, personnel (maquette : 495, 105 -> 1190, 900) -----------
            Column {
                x: 520 * Tokens.uiScale
                width: 780 * Tokens.uiScale
                spacing: Tokens.gapMedium

                Item {
                    width: parent.width
                    height: 56 * Tokens.uiScale

                    SectionBanner {
                        anchors.left: parent.left
                        width: 420 * Tokens.uiScale
                        text: qsTr("Base principale")
                    }

                    Text {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr("Niveau %1").arg(root.baseLevel)
                        color: Tokens.text
                        font.family: Tokens.titleFamily
                        font.pixelSize: Tokens.fontBody
                    }
                }

                PanelFrame {
                    width: parent.width
                    height: 260 * Tokens.uiScale
                    material: "parchment"
                    subpanel: true
                    empty: true

                    Column {
                        anchors.left: parent.left
                        anchors.bottom: parent.bottom
                        width: parent.width * 0.55
                        spacing: 4 * Tokens.uiScale

                        Text {
                            text: root.baseName
                            color: Tokens.text
                            font.family: Tokens.titleFamily
                            font.pixelSize: Tokens.fontSectionTitle
                        }

                        Text {
                            width: parent.width
                            text: root.baseDescription
                            color: Tokens.textMuted
                            font.family: Tokens.loreFamily
                            font.italic: true
                            font.pixelSize: Tokens.fontCaption
                            wrapMode: Text.WordWrap
                        }
                    }

                    Column {
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        width: parent.width * 0.38
                        spacing: 4 * Tokens.uiScale

                        Text {
                            text: qsTr("État du bâtiment")
                            color: Tokens.text
                            font.family: Tokens.titleFamily
                            font.pixelSize: Tokens.fontCaption
                        }

                        Gauge {
                            width: parent.width
                            kind: "health"
                            value: root.baseConditionRatio
                            label: root.baseCondition
                        }
                    }
                }

                SectionBanner {
                    width: 420 * Tokens.uiScale
                    text: qsTr("Bâtiments construits")
                }

                Grid {
                    columns: 3
                    spacing: Tokens.gapMedium

                    Repeater {
                        model: root.buildings

                        PanelFrame {
                            id: building

                            required property string label
                            required property string value

                            width: 248 * Tokens.uiScale
                            height: 110 * Tokens.uiScale
                            material: "parchment"
                            subpanel: true
                            padding: Tokens.gapSmall

                            Column {
                                anchors.fill: parent

                                Text {
                                    width: parent.width
                                    text: building.label
                                    color: Tokens.text
                                    font.family: Tokens.bodyFamily
                                    font.pixelSize: Tokens.fontBody
                                    elide: Text.ElideRight
                                }

                                Text {
                                    width: parent.width
                                    text: building.value
                                    color: Tokens.textMuted
                                    font.family: Tokens.bodyFamily
                                    font.pixelSize: Tokens.fontCaption
                                }
                            }
                        }
                    }
                }

                SectionBanner {
                    width: 420 * Tokens.uiScale
                    text: qsTr("Personnel de la base")
                }

                Row {
                    spacing: 40 * Tokens.uiScale

                    Repeater {
                        model: root.staff

                        Column {
                            id: staffMember

                            required property string label
                            required property string value

                            width: 150 * Tokens.uiScale
                            spacing: 4 * Tokens.uiScale

                            PortraitFrame {
                                anchors.horizontalCenter: parent.horizontalCenter
                                shape: "round"
                                size: 88 * Tokens.uiScale
                            }

                            Text {
                                width: parent.width
                                horizontalAlignment: Text.AlignHCenter
                                text: staffMember.label + "\n" + staffMember.value
                                color: Tokens.text
                                font.family: Tokens.bodyFamily
                                font.pixelSize: Tokens.fontCaption
                            }
                        }
                    }
                }
            }

            // --- Colonne droite : hauts faits, specialisation, tresors (maquette : 1210, 105 -> 1640, 900) --
            Column {
                x: 1330 * Tokens.uiScale
                width: 478 * Tokens.uiScale
                spacing: Tokens.gapMedium

                SectionBanner {
                    width: 360 * Tokens.uiScale
                    text: qsTr("Hauts faits")
                }

                Repeater {
                    model: root.achievements

                    Item {
                        id: achievement

                        required property string label
                        required property string value

                        width: 478 * Tokens.uiScale
                        height: 64 * Tokens.uiScale

                        StatMedallion {
                            id: achievementMark

                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
                            width: 56 * Tokens.uiScale
                            height: 56 * Tokens.uiScale
                            kind: "derived"
                            label: ""
                            value: ""
                        }

                        Column {
                            anchors.left: achievementMark.right
                            anchors.right: parent.right
                            anchors.leftMargin: Tokens.gapMedium
                            anchors.verticalCenter: parent.verticalCenter

                            Text {
                                width: parent.width
                                text: achievement.label
                                color: Tokens.text
                                font.family: Tokens.bodyFamily
                                font.pixelSize: Tokens.fontBody
                                elide: Text.ElideRight
                            }

                            Text {
                                width: parent.width
                                text: achievement.value
                                color: Tokens.textMuted
                                font.family: Tokens.bodyFamily
                                font.pixelSize: Tokens.fontCaption
                                elide: Text.ElideRight
                            }
                        }
                    }
                }

                SectionBanner {
                    width: parent.width
                    text: qsTr("Spécialisation & style")
                }

                Column {
                    width: parent.width
                    spacing: 4 * Tokens.uiScale

                    FieldRow { width: parent.width; label: qsTr("Spécialisation"); value: root.specialization }
                    FieldRow { width: parent.width; label: qsTr("Style"); value: root.style }
                    FieldRow { width: parent.width; label: qsTr("Rang"); value: root.rank }

                    Gauge {
                        width: parent.width
                        kind: "experience"
                        value: root.rankRatio
                    }
                }

                SectionBanner {
                    width: 360 * Tokens.uiScale
                    text: qsTr("Trésors de la base")
                }

                Row {
                    spacing: Tokens.gapSmall

                    Repeater {
                        model: root.treasures

                        // Une enveloppe : `ItemSlot` porte deja `label`, qu'un delegue ne peut pas
                        // redeclarer en `required` sans boucler sur lui-meme.
                        Item {
                            id: treasure

                            required property string label
                            required property string value

                            width: 88 * Tokens.uiScale
                            height: 88 * Tokens.uiScale

                            ItemSlot {
                                anchors.fill: parent
                                label: treasure.label + "\n" + treasure.value
                                enabled: false
                            }
                        }
                    }
                }
            }
        }

        // ===== Onglet Recrutement =======================================================================
        LedgerList {
            title: qsTr("Mercenaires à engager")
            rows: root.recruits
        }

        // ===== Onglet Contrats ===========================================================================
        RowLayout {
            spacing: Tokens.gapLarge

            LedgerList {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: qsTr("Contrats de la Guilde")
                rows: root.contracts
            }

            PanelFrame {
                Layout.fillWidth: true
                Layout.fillHeight: true
                material: "parchment"
                subpanel: true

                Column {
                    anchors.fill: parent
                    spacing: Tokens.gapMedium

                    SectionBanner {
                        width: parent.width
                        text: qsTr("Contrat")
                    }

                    Text {
                        width: parent.width
                        text: root.contract
                        color: Tokens.text
                        font.family: Tokens.bodyFamily
                        font.pixelSize: Tokens.fontBody
                        wrapMode: Text.WordWrap
                    }

                    FieldRow { width: parent.width; label: qsTr("Commanditaire"); value: root.contractGiver }
                    FieldRow { width: parent.width; label: qsTr("Rang"); value: root.contractRank }
                    FieldRow { width: parent.width; label: qsTr("Récompense"); value: root.contractReward }
                }
            }
        }

        // ===== Onglet Reserve ===========================================================================
        LedgerList {
            title: qsTr("Mercenaires au repos")
            rows: root.reserve
        }
    }
}
