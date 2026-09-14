import QtQuick
import QtQuick.Controls
import Jadg.Ui

/*!
    Le Colisee -- FORMULAIRE, cote conception (LOT-50).

    Un ECRAN DE DEVELOPPEUR, sans maquette et sans charte, comme la feuille de route le dit : le
    bac a sable ou l'on compose deux camps, lance un affrontement, le joue case par case et le
    rejoue a graine fixee. Il dessine la grille lui-meme, depuis `cells`, parce que la surface de
    rendu n'affiche encore aucune scene ; l'IHM de combat (LOT-24) la dessinera sur la carte.

    Trois zones : a gauche la composition (le roster, les deux camps, la graine), au centre la
    grille et sa barre d'actions, a droite l'ordre d'initiative et le journal. Les couleurs
    viennent des jetons pour que l'ecran ne jure pas au milieu du jeu, sans pretendre a la charte.
*/
Item {
    id: root

    // --- Ce que le jumeau alimente -----------------------------------------------------------
    property string arenaName: "Arene"
    property string status: ""
    property bool inCombat: false
    property bool ended: false
    property var roster: []
    property var allies: []
    property var enemies: []
    property var marks: []
    property int seed: 0
    property int gridColumns: 0
    property int gridRows: 0
    property var cells: []
    property var turnOrder: []
    property string activeName: ""
    property string activeResources: ""
    property var journal: []

    // --- Ce que le jumeau ecoute -------------------------------------------------------------
    signal fighterChosen(string id, bool ally)
    signal allyRemoved(int index)
    signal enemyRemoved(int index)
    signal markChosen(bool ally, int index, string markId)
    signal seedEdited(int value)
    signal launchRequested()
    signal cellTapped(int column, int row)
    signal endTurnRequested()
    signal withdrawRequested()
    signal replayRequested()
    signal backRequested()
    signal closeRequested()

    readonly property real cellSize: Math.max(16, Math.min((gridHost.width - 8) / Math.max(1, root.gridColumns),
                                                           (gridHost.height - 8) / Math.max(1, root.gridRows)))

    Rectangle {
        anchors.fill: parent
        color: Tokens.panel
    }

    // --- Bandeau ------------------------------------------------------------------------------
    Row {
        id: header

        x: Tokens.gapLarge
        y: Tokens.gapMedium
        spacing: Tokens.gapLarge

        Text {
            text: root.arenaName
            color: Tokens.goldLight
            font.family: Tokens.titleFamily
            font.pixelSize: Tokens.fontSectionTitle
        }
        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: root.status
            color: Tokens.textOnPanel
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontBody
            elide: Text.ElideRight
            width: root.width - header.x - 520 * Tokens.uiScale
        }
    }
    Button {
        id: closeButton

        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: Tokens.gapMedium
        text: qsTr("Retour au menu")
        onClicked: root.closeRequested()
    }

    // --- Composition, a gauche ----------------------------------------------------------------
    Column {
        id: setupColumn

        x: Tokens.gapLarge
        y: header.y + header.height + Tokens.gapMedium
        width: 300 * Tokens.uiScale
        height: root.height - y - Tokens.gapMedium
        spacing: Tokens.gapSmall
        visible: !root.inCombat

        Text {
            text: qsTr("Combattants disponibles")
            color: Tokens.panelEdge
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontBody
        }
        ListView {
            id: rosterList

            width: parent.width
            height: parent.height * 0.45
            clip: true
            model: root.roster
            delegate: Row {
                spacing: Tokens.gapSmall
                Text {
                    width: rosterList.width - 2 * 64 * Tokens.uiScale - 2 * Tokens.gapSmall
                    anchors.verticalCenter: parent.verticalCenter
                    text: modelData.name + "  (" + modelData.hitPoints + " PV, CA " + modelData.armorClass + ")"
                    color: Tokens.textOnPanel
                    font.family: Tokens.bodyFamily
                    font.pixelSize: Tokens.fontCaption
                    elide: Text.ElideRight
                }
                Button {
                    width: 64 * Tokens.uiScale
                    text: "+A"
                    onClicked: root.fighterChosen(modelData.id, true)
                }
                Button {
                    width: 64 * Tokens.uiScale
                    text: "+E"
                    onClicked: root.fighterChosen(modelData.id, false)
                }
            }
        }

        Text {
            text: qsTr("Allies")
            color: Tokens.textAlly
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontBody
        }
        ListView {
            id: alliesList

            width: parent.width
            height: parent.height * 0.16
            clip: true
            model: root.allies
            delegate: Row {
                spacing: Tokens.gapSmall
                Text {
                    width: alliesList.width - 150 * Tokens.uiScale
                    anchors.verticalCenter: parent.verticalCenter
                    text: modelData.name
                    color: Tokens.textOnPanel
                    font.family: Tokens.bodyFamily
                    font.pixelSize: Tokens.fontCaption
                    elide: Text.ElideRight
                }
                ComboBox {
                    width: 100 * Tokens.uiScale
                    model: [""].concat(root.marks)
                    currentIndex: Math.max(0, model.indexOf(modelData.mark))
                    onActivated: root.markChosen(true, index, currentText)
                }
                Button {
                    width: 40 * Tokens.uiScale
                    text: "x"
                    onClicked: root.allyRemoved(index)
                }
            }
        }

        Text {
            text: qsTr("Ennemis")
            color: Tokens.textEnemy
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontBody
        }
        ListView {
            id: enemiesList

            width: parent.width
            height: parent.height * 0.16
            clip: true
            model: root.enemies
            delegate: Row {
                spacing: Tokens.gapSmall
                Text {
                    width: enemiesList.width - 150 * Tokens.uiScale
                    anchors.verticalCenter: parent.verticalCenter
                    text: modelData.name
                    color: Tokens.textOnPanel
                    font.family: Tokens.bodyFamily
                    font.pixelSize: Tokens.fontCaption
                    elide: Text.ElideRight
                }
                ComboBox {
                    width: 100 * Tokens.uiScale
                    model: [""].concat(root.marks)
                    currentIndex: Math.max(0, model.indexOf(modelData.mark))
                    onActivated: root.markChosen(false, index, currentText)
                }
                Button {
                    width: 40 * Tokens.uiScale
                    text: "x"
                    onClicked: root.enemyRemoved(index)
                }
            }
        }

        Row {
            spacing: Tokens.gapSmall
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Graine")
                color: Tokens.textOnPanel
                font.family: Tokens.bodyFamily
                font.pixelSize: Tokens.fontBody
            }
            SpinBox {
                id: seedBox
                from: 0
                to: 999999
                value: root.seed
                editable: true
                onValueModified: root.seedEdited(value)
            }
        }
        Button {
            width: parent.width
            text: qsTr("Lancer l'affrontement")
            onClicked: root.launchRequested()
        }
        Text {
            width: parent.width
            text: qsTr("+A : enroler en allie, +E : en ennemi.")
            color: Tokens.textOnPanelMuted
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontCaption
            wrapMode: Text.WordWrap
        }
    }

    // --- Le combat en cours, a gauche aussi ---------------------------------------------------
    Column {
        id: combatColumn

        x: Tokens.gapLarge
        y: setupColumn.y
        width: setupColumn.width
        spacing: Tokens.gapSmall
        visible: root.inCombat

        Text {
            text: root.ended ? qsTr("Combat termine") : qsTr("Tour de ") + root.activeName
            color: Tokens.goldLight
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontBody
        }
        Text {
            width: parent.width
            text: root.activeResources
            color: Tokens.textOnPanel
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontCaption
            wrapMode: Text.WordWrap
        }
        Text {
            width: parent.width
            text: qsTr("Cliquer une case surlignee : se deplacer. Cliquer un ennemi au contact : frapper.")
            color: Tokens.textOnPanelMuted
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontCaption
            wrapMode: Text.WordWrap
        }
        Button {
            text: qsTr("Fin du tour")
            enabled: !root.ended
            onClicked: root.endTurnRequested()
        }
        Button {
            text: qsTr("Se retirer")
            enabled: !root.ended
            onClicked: root.withdrawRequested()
        }
        Button {
            text: qsTr("Rejouer (meme graine)")
            onClicked: root.replayRequested()
        }
        Button {
            text: qsTr("Nouvelle composition")
            onClicked: root.backRequested()
        }
    }

    // --- La grille, au centre -----------------------------------------------------------------
    Rectangle {
        id: gridHost

        x: setupColumn.x + setupColumn.width + Tokens.gapLarge
        y: setupColumn.y
        width: journalColumn.x - x - Tokens.gapLarge
        height: root.height - y - Tokens.gapMedium
        color: Tokens.panelRaised
        border.color: Tokens.panelEdge
        border.width: Tokens.strokeWidth

        Grid {
            anchors.centerIn: parent
            columns: root.gridColumns
            rows: root.gridRows
            spacing: 0

            Repeater {
                model: root.cells

                Rectangle {
                    width: root.cellSize
                    height: root.cellSize
                    color: modelData.wall ? Tokens.frameEdge
                         : modelData.reachable ? Tokens.info
                         : Tokens.surface
                    border.color: modelData.active ? Tokens.goldLight : Tokens.border
                    border.width: modelData.active ? 2 : 1

                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width * 0.7
                        height: width
                        radius: width / 2
                        visible: modelData.occupant.length > 0
                        color: modelData.side === "allies" ? Tokens.textAlly : Tokens.textEnemy
                        opacity: modelData.down ? 0.35 : 1
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.bottom: parent.bottom
                        text: modelData.hitPoints
                        color: Tokens.textOnPanel
                        font.pixelSize: Math.max(8, root.cellSize * 0.28)
                        visible: modelData.occupant.length > 0
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: root.cellTapped(modelData.column, modelData.row)
                    }
                }
            }
        }
    }

    // --- Ordre et journal, a droite -----------------------------------------------------------
    Column {
        id: journalColumn

        x: root.width - width - Tokens.gapLarge
        y: setupColumn.y
        width: 300 * Tokens.uiScale
        height: root.height - y - Tokens.gapMedium
        spacing: Tokens.gapSmall

        Text {
            text: qsTr("Initiative")
            color: Tokens.panelEdge
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontBody
        }
        ListView {
            width: parent.width
            height: parent.height * 0.3
            clip: true
            model: root.turnOrder
            delegate: Text {
                text: (modelData.active ? "> " : "   ") + modelData.total + "  " + modelData.name
                      + (modelData.down ? qsTr("  (a terre)") : "")
                color: modelData.side === "allies" ? Tokens.textAlly : Tokens.textEnemy
                font.family: Tokens.bodyFamily
                font.pixelSize: Tokens.fontCaption
                font.bold: modelData.active
            }
        }
        Text {
            text: qsTr("Journal")
            color: Tokens.panelEdge
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontBody
        }
        ListView {
            id: journalList

            width: parent.width
            height: parent.height * 0.6
            clip: true
            model: root.journal
            delegate: Text {
                width: journalList.width
                text: modelData
                color: Tokens.textOnPanel
                font.family: Tokens.bodyFamily
                font.pixelSize: Tokens.fontCaption
                wrapMode: Text.WordWrap
            }
            onCountChanged: positionViewAtEnd()
        }
    }
}
