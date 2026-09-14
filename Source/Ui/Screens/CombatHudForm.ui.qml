pragma ComponentBehavior: Bound
import QtQuick
import Jadg.Ui

/*!
    HUD de combat -- FORMULAIRE, cote conception (LOT-86, LOT-87 T4.1 ; maquette 01).

    Le seul ecran qui ne s'ouvre PAS par-dessus le jeu : il EST le jeu pendant un combat. Il ne
    suspend donc rien, et le tour par tour decidera de son rythme. Le cadre commun a la vue de jeu
    vient de `HudFrame`, en mode `combat` ; ce formulaire y ajoute ce qui n'appartient qu'au combat :
    le journal, l'ordre d'initiative, la roue et la barre d'actions, la fiche de la cible.

    Chaque action porte son RACCOURCI, ecrit sous sa case : un combat doit se jouer entierement au
    clavier et entierement a la manette, le critere que la feuille de route dit « souvent oublie ».

    Les listes ont les roles de `SheetRowModel` (`rowId`, `label`, `value`) :
    - `initiative` : `label` le combattant, `value` son camp (`ally`, `enemy`) ;
    - `combatLog` : `label` la ligne, `value` le tour qu'elle ouvre (`ally`, `enemy`) ou vide ;
    - `actions` : `label` l'action, `value` ses charges restantes ou vide.

    Les proprietes portent des VALEURS D'EXEMPLE ; le jumeau les remplace.
*/
HudFrame {
    id: root

    property var initiative: exampleInitiative
    /// Le combattant dont c'est le tour (indice dans `initiative`).
    property int activeIndex: 0

    property var combatLog: exampleLog

    property var actions: exampleActions
    /// L'action choisie (indice dans `actions`).
    property int activeAction: 0
    property string activeActionLabel: "Arc long"

    property string targetName: "Bandit"
    property string targetLevel: "3"
    property string targetHitPoints: "18 / 32"
    property real targetHitPointsRatio: 18 / 32
    property string targetArmorClass: "13"
    property string targetInitiative: "+2"
    property string targetSpeed: "9 m"
    property string targetConditions: "À terre"
    property url targetPortrait: ""

    readonly property ListModel exampleInitiative: ListModel {
        ListElement { rowId: "brenna"; label: "Brenna"; value: "ally" }
        ListElement { rowId: "bandit-1"; label: "Bandit"; value: "enemy" }
        ListElement { rowId: "sarre"; label: "Sarre"; value: "ally" }
        ListElement { rowId: "bandit-2"; label: "Bandit"; value: "enemy" }
        ListElement { rowId: "ourse"; label: "Ourse"; value: "ally" }
    }

    readonly property ListModel exampleLog: ListModel {
        ListElement { rowId: "1"; label: "Tour de Brenna"; value: "ally" }
        ListElement { rowId: "2"; label: "Brenna se déplace de 4 cases."; value: "" }
        ListElement { rowId: "3"; label: "Brenna tire sur Bandit : 12 dégâts."; value: "" }
        ListElement { rowId: "4"; label: "Bandit est à terre."; value: "" }
        ListElement { rowId: "5"; label: "Tour de l'ennemi"; value: "enemy" }
    }

    readonly property ListModel exampleActions: ListModel {
        ListElement { rowId: "arc"; label: "Arc long"; value: "" }
        ListElement { rowId: "trait"; label: "Trait de givre"; value: "3" }
        ListElement { rowId: "cacher"; label: "Se cacher"; value: "" }
        ListElement { rowId: "potion"; label: "Potion"; value: "3" }
        ListElement { rowId: "parade"; label: "Parade"; value: "" }
        ListElement { rowId: "feu"; label: "Flèche de feu"; value: "3" }
        ListElement { rowId: "piege"; label: "Piège"; value: "1" }
        ListElement { rowId: "passer"; label: "Passer"; value: "" }
    }

    mode: "combat"

    // --- Ordre d'initiative, sous la boussole ------------------------------------------------------------
    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 128 * Tokens.uiScale
        spacing: Tokens.gapSmall

        Repeater {
            model: root.initiative

            Rectangle {
                id: combatant

                required property int index
                required property string label
                required property string value

                readonly property bool current: combatant.index === root.activeIndex

                width: chipRow.implicitWidth + 2 * Tokens.gapMedium
                height: 36 * Tokens.uiScale
                color: combatant.current ? Tokens.panelRaised : Tokens.panel
                border.color: combatant.current ? Tokens.goldLight : Tokens.panelEdge
                border.width: (combatant.current ? 2 : 1) * Tokens.strokeWidth

                Row {
                    id: chipRow

                    anchors.centerIn: parent
                    spacing: Tokens.gapSmall

                    // Le camp se lit a la marque autant qu'a la teinte : losange pour un allie,
                    // rond pour un ennemi.
                    Rectangle {
                        anchors.verticalCenter: parent.verticalCenter
                        width: 10 * Tokens.uiScale
                        height: 10 * Tokens.uiScale
                        rotation: combatant.value === "enemy" ? 0 : 45
                        radius: combatant.value === "enemy" ? width / 2 : 0
                        color: combatant.value === "enemy" ? Tokens.textEnemy
                               : (combatant.value === "ally" ? Tokens.textAlly : Tokens.textOnPanelMuted)
                    }

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: combatant.label
                        color: combatant.current ? Tokens.goldLight : Tokens.textOnPanel
                        font.family: Tokens.bodyFamily
                        font.pixelSize: Tokens.fontCaption
                    }
                }
            }
        }
    }

    // --- Journal de combat (maquette : 20, 678 -> 400, 835) ------------------------------------------------
    PanelFrame {
        x: 24 * Tokens.uiScale
        y: 776 * Tokens.uiScale
        width: 440 * Tokens.uiScale
        height: 188 * Tokens.uiScale
        subpanel: true

        Column {
            anchors.fill: parent
            spacing: 2 * Tokens.uiScale
            clip: true

            Repeater {
                model: root.combatLog

                Item {
                    id: entry

                    required property string label
                    required property string value

                    readonly property bool turn: entry.value === "ally" || entry.value === "enemy"

                    width: parent.width
                    height: 28 * Tokens.uiScale

                    Rectangle {
                        id: entryMark

                        anchors.left: parent.left
                        anchors.leftMargin: 4 * Tokens.uiScale
                        anchors.verticalCenter: parent.verticalCenter
                        width: (entry.turn ? 14 : 8) * Tokens.uiScale
                        height: width
                        rotation: entry.value === "ally" ? 45 : 0
                        radius: entry.value === "ally" ? 0 : width / 2
                        color: entry.value === "ally" ? Tokens.textAlly
                               : (entry.value === "enemy" ? Tokens.textEnemy : "transparent")
                        border.color: entry.turn ? Tokens.panel : Tokens.panelEdge
                        border.width: Tokens.strokeWidth
                    }

                    Text {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.leftMargin: 32 * Tokens.uiScale
                        anchors.verticalCenter: parent.verticalCenter
                        text: entry.label
                        color: entry.value === "ally" ? Tokens.textAlly
                               : (entry.value === "enemy" ? Tokens.textEnemy : Tokens.textOnPanel)
                        font.family: entry.turn ? Tokens.titleFamily : Tokens.bodyFamily
                        font.pixelSize: Tokens.fontBody
                        font.weight: entry.turn ? Font.DemiBold : Font.Normal
                        elide: Text.ElideRight
                    }
                }
            }
        }
    }

    // --- Roue d'action (maquette : 425, 758 -> 562, 892) ---------------------------------------------------
    Item {
        x: 476 * Tokens.uiScale
        y: 872 * Tokens.uiScale
        width: 156 * Tokens.uiScale
        height: 156 * Tokens.uiScale

        Rectangle {
            anchors.fill: parent
            visible: !wheelArt.delivered
            radius: width / 2
            color: Tokens.gem
            border.color: Tokens.panelEdge
            border.width: 3 * Tokens.strokeWidth
        }

        FixedArt {
            id: wheelArt

            anchors.fill: parent
            key: "ui/medallion/action-wheel"
        }

        Text {
            anchors.centerIn: parent
            width: parent.width * 0.64
            text: root.activeActionLabel
            color: Tokens.textOnPanel
            font.family: Tokens.titleFamily
            font.pixelSize: Tokens.fontBody
            font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            maximumLineCount: 3
            elide: Text.ElideRight
        }
    }

    // --- Barre d'actions (maquette : 555, 778 -> 1090, 880) ------------------------------------------------
    PanelFrame {
        x: 640 * Tokens.uiScale
        y: 884 * Tokens.uiScale
        width: 612 * Tokens.uiScale
        height: 136 * Tokens.uiScale
        subpanel: true

        Row {
            anchors.centerIn: parent
            spacing: Tokens.gapSmall

            Repeater {
                model: root.actions

                ActionSlot {
                    id: actionCell

                    required property int index
                    required property string value
                    required label

                    quantity: actionCell.value
                    shortcut: "" + (actionCell.index + 1)
                    active: actionCell.index === root.activeAction
                }
            }
        }
    }

    // --- Fiche de la cible (maquette : 1100, 678 -> 1390, 828) ----------------------------------------------
    PanelFrame {
        x: 1256 * Tokens.uiScale
        y: 756 * Tokens.uiScale
        width: 332 * Tokens.uiScale
        height: 204 * Tokens.uiScale
        subpanel: true

        Text {
            id: targetTitle

            anchors.left: parent.left
            anchors.right: targetLevelLabel.left
            anchors.top: parent.top
            anchors.rightMargin: Tokens.gapSmall
            text: root.targetName
            color: Tokens.textOnPanel
            font.family: Tokens.titleFamily
            font.pixelSize: Tokens.fontBody
            font.weight: Font.DemiBold
            elide: Text.ElideRight
        }

        Text {
            id: targetLevelLabel

            anchors.right: parent.right
            anchors.baseline: targetTitle.baseline
            text: qsTr("Niv. %1").arg(root.targetLevel)
            color: Tokens.textOnPanel
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontBody
        }

        PortraitFrame {
            id: targetPortraitFrame

            anchors.left: parent.left
            anchors.top: targetTitle.bottom
            anchors.topMargin: Tokens.gapSmall
            shape: "square"
            size: 96 * Tokens.uiScale
            source: root.targetPortrait
        }

        Column {
            anchors.left: targetPortraitFrame.right
            anchors.right: parent.right
            anchors.top: targetPortraitFrame.top
            anchors.leftMargin: Tokens.gapMedium
            spacing: 2 * Tokens.uiScale

            Gauge {
                width: parent.width
                height: 26 * Tokens.uiScale
                kind: "health"
                value: root.targetHitPointsRatio
                label: root.targetHitPoints
            }

            Item { width: 1; height: 4 * Tokens.uiScale }

            Repeater {
                model: [
                    { label: qsTr("CA"), value: root.targetArmorClass },
                    { label: qsTr("Initiative"), value: root.targetInitiative },
                    { label: qsTr("Vitesse"), value: root.targetSpeed },
                    { label: qsTr("États"), value: root.targetConditions }
                ]

                Item {
                    id: stat

                    required property var modelData

                    width: parent.width
                    height: 24 * Tokens.uiScale

                    Text {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        text: stat.modelData.label
                        color: Tokens.textOnPanel
                        font.family: Tokens.bodyFamily
                        font.pixelSize: Tokens.fontCaption
                    }

                    Text {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        text: stat.modelData.value
                        color: Tokens.textOnPanel
                        font.family: Tokens.bodyFamily
                        font.pixelSize: Tokens.fontCaption
                    }
                }
            }
        }
    }
}
