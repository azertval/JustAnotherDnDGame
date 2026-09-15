import QtQuick
import QtQuick.Controls
import Jadg.Ui

/*!
    Le Colisee -- FORMULAIRE, cote conception (LOT-50).

    Un ECRAN DE DEVELOPPEUR, sans maquette et sans charte, comme la feuille de route le dit : le
    bac a sable ou l'on compose deux camps, lance un affrontement, le joue case par case et le
    rejoue a graine fixee.

    Trois zones : a gauche la composition (le roster, les deux camps, la graine), puis en combat les
    actions du tour, la previsualisation de la case visee et les commandes ; au centre la grille
    rendue par QRhi avec son curseur et chemin de ciblage (LOT-24) ; a droite l'ordre d'initiative
    et le journal. Depuis le LOT-24, le combat se joue entierement au clavier ou a la manette :
    le formulaire montre, le jumeau traduit les touches et les boutons en gestes. Les couleurs
    viennent des jetons pour que l'ecran ne jure pas au milieu du jeu, sans pretendre a la charte.

    Depuis le LOT-86 (Phase 7), la scene (sol, enceinte, figurines) se dessine par le pipeline QRhi
    du jeu, pose dans `viewportHost` par le jumeau (`ArenaViewport`) : toutes les pieces de la
    planche de production du Colisee (`Source/Elements/Assets/Coliseum/`), composees en primitives
    par `hmi::ArenaSceneComposer`, plutot qu'en delegues QML. Par-dessus, un calque d'interface --
    cases atteignables, surbrillances, jauges et points de vie, chemin et curseur de ciblage --
    cale sur le cadrage que la surface publie (`gridTileWidth`...). Le cadre de l'ecran, lui,
    reste celui du developpeur.
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
    property bool enemyAi: true
    property int gridColumns: 0
    property int gridRows: 0
    property var turnOrder: []
    property string activeName: ""
    property string activeResources: ""
    property var journal: []
    /// Les actions du tour : `{label, kind, enabled, selected}` (LOT-24).
    property var turnActions: []
    /// Ce que la confirmation ferait sur la case visee, ligne par ligne (LOT-24).
    property var preview: []
    property int cursorColumn: 0
    property int cursorRow: 0
    /// Le chemin jusqu'au curseur : `{column, row}` (LOT-24).
    property var pathCells: []
    /// Les combattants sur la grille : `{column, row, footprint, side, active, down, hitPoints,
    /// hitPointsRatio}`.
    property var fighters: []
    /// Les cases ou le combattant actif peut finir son deplacement : `{column, row}`.
    property var reachableCells: []
    property bool gamepadConnected: false

    /// Le cadrage de la grille dans la surface de rendu, en unites d'element : celui du rendu
    /// lui-meme (`ArenaViewport`), relaye par le jumeau. Le calque de ciblage ne le recalcule pas --
    /// le zoom du rendu est arrondi a l'entier, un calcul parallele tomberait a cote des cases.
    /// Les valeurs par defaut ne servent qu'a l'atelier.
    property real gridTileWidth: 64
    property real gridTileHeight: 40
    property real gridOriginX: 0
    property real gridOriginY: 0

    /// L'hote de la surface de rendu. La surface elle-meme (`ArenaViewport`) est un type C++ que
    /// l'atelier ne connait pas : c'est le jumeau qui la pose ici, a l'execution -- meme mecanisme
    /// que `HudFrame.viewportHost` pour la vue d'exploration (LOT-86 Phase 6).
    property alias viewportHost: viewportHost

    // --- Ce que le jumeau ecoute -------------------------------------------------------------
    signal fighterChosen(string id, bool ally)
    signal allyRemoved(int index)
    signal enemyRemoved(int index)
    signal markChosen(bool ally, int index, string markId)
    signal seedEdited(int value)
    signal enemyAiToggled(bool value)
    signal launchRequested()
    /// Le pointeur au-dessus de la grille, et le clic : en coordonnees de la surface de rendu, que
    /// le jumeau traduit en case (`ArenaViewport.cellAt`).
    signal gridHovered(real x, real y)
    signal gridClicked(real x, real y)
    signal endTurnRequested()
    signal actionChosen(int index)
    signal withdrawRequested()
    signal replayRequested()
    signal backRequested()
    signal closeRequested()

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
        CheckBox {
            text: qsTr("Ennemis joues par l'IA")
            checked: root.enemyAi
            onToggled: root.enemyAiToggled(checked)
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
        // Les actions du tour, numerotees comme au clavier ; l'action choisie est surlignee.
        Repeater {
            model: root.turnActions

            Button {
                width: combatColumn.width
                // La selection se lit au texte : le style « highlighted » assombrit le libelle.
                text: (modelData.selected ? "> " : "") + (index + 1) + ". " + modelData.label
                      + (modelData.selected ? " <" : "")
                enabled: modelData.enabled
                font.bold: modelData.selected
                focusPolicy: Qt.NoFocus
                onClicked: root.actionChosen(index)
            }
        }

        // Ce que la confirmation ferait sur la case visee, avant que le joueur ne s'engage.
        Rectangle {
            width: parent.width
            height: previewColumn.height + 2 * Tokens.gapSmall
            visible: root.preview.length > 0
            color: Tokens.panelRaised
            border.color: Tokens.accent
            border.width: Tokens.strokeWidth

            Column {
                id: previewColumn

                x: Tokens.gapSmall
                y: Tokens.gapSmall
                width: parent.width - 2 * Tokens.gapSmall

                Repeater {
                    model: root.preview

                    Text {
                        width: previewColumn.width
                        text: modelData
                        color: index === 0 ? Tokens.goldLight : Tokens.textOnPanel
                        font.family: Tokens.bodyFamily
                        font.pixelSize: Tokens.fontCaption
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }

        Button {
            text: qsTr("Fin du tour")
            enabled: !root.ended
            focusPolicy: Qt.NoFocus
            onClicked: root.endTurnRequested()
        }
        Button {
            text: qsTr("Se retirer")
            enabled: !root.ended
            focusPolicy: Qt.NoFocus
            onClicked: root.withdrawRequested()
        }
        Button {
            text: qsTr("Rejouer (meme graine)")
            focusPolicy: Qt.NoFocus
            onClicked: root.replayRequested()
        }
        Button {
            text: qsTr("Nouvelle composition")
            focusPolicy: Qt.NoFocus
            onClicked: root.backRequested()
        }
        Text {
            width: parent.width
            text: qsTr("Souris : survol, viser ; clic, se deplacer ou attaquer.")
            color: Tokens.textOnPanelMuted
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontCaption
            wrapMode: Text.WordWrap
        }
        Text {
            width: parent.width
            text: qsTr("Clavier : fleches, le curseur ; Entree, confirmer ; Tab, cible suivante ; 1 a 9 ou Page precedente / suivante, l'action ; Retour arriere, recentrer ; Espace, fin du tour.")
            color: Tokens.textOnPanelMuted
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontCaption
            wrapMode: Text.WordWrap
        }
        Text {
            width: parent.width
            text: (root.gamepadConnected ? qsTr("Manette connectee") : qsTr("Manette absente"))
                  + qsTr(" : croix, le curseur ; A, confirmer ; X, cible suivante ; LB / RB, l'action ; B, recentrer ; Y, fin du tour.")
            color: Tokens.textOnPanelMuted
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontCaption
            wrapMode: Text.WordWrap
        }
    }

    // --- La grille, au centre -----------------------------------------------------------------
    // L'hote de la surface de rendu (LOT-86 Phase 6) : sol, enceinte et figurines animees viennent
    // desormais du pipeline QRhi du jeu (`ArenaViewport`, pose ici par le jumeau), au lieu de
    // centaines de delegues QML detruits et recrees a chaque geste de combat. Ce que le jumeau ne
    // pose pas encore s'y voit comme un aplat au panneau.
    Rectangle {
        id: viewportHost

        x: setupColumn.x + setupColumn.width + Tokens.gapLarge
        y: setupColumn.y
        width: journalColumn.x - x - Tokens.gapLarge
        height: root.height - y - Tokens.gapMedium
        color: Tokens.panelRaised
        border.color: Tokens.panelEdge
        border.width: Tokens.strokeWidth
    }

    // Le calque d'interface au-dessus de la scene rendue. Au-dessus de `viewportHost` dans l'ordre
    // de peinture : c'est ce qui le garde visible par-dessus la scene du jumeau (`ArenaViewport`).
    // La scene (sol, enceinte, figurines) est dessinee par QRhi (`ArenaSceneComposer`) ; ce calque
    // n'y ajoute que des jetons et du texte : cases atteignables, surbrillance et points de vie de
    // chaque combattant, jauge des ennemis, chemin et curseur de ciblage (LOT-24). Une marque par
    // combattant ou par case atteignable, jamais une par case de la grille.
    // Meme rectangle que la surface de rendu (le jumeau la pose dans `viewportHost`, a la meme
    // marge) : les coordonnees du calque sont celles de la surface.
    Item {
        id: targetingLayer

        anchors.fill: viewportHost
        anchors.margins: Tokens.gapMedium
        visible: root.inCombat

        // Cases ou le combattant actif peut finir son deplacement
        Repeater {
            model: root.reachableCells

            ArenaMark {
                kind: "reachable"
                x: root.gridOriginX + (modelData.column - modelData.row) * root.gridTileWidth / 2
                y: root.gridOriginY + (modelData.column + modelData.row) * root.gridTileHeight / 2
                width: root.gridTileWidth
                height: root.gridTileHeight
            }
        }

        // Les combattants : l'element est le losange de leur emprise (n cases de cote).
        Repeater {
            model: root.fighters

            Item {
                id: fighter

                required property var modelData

                readonly property int footprint: Math.max(1, fighter.modelData.footprint)
                readonly property bool ally: fighter.modelData.side === "allies"
                readonly property bool active: fighter.modelData.active

                x: root.gridOriginX + (fighter.modelData.column - fighter.modelData.row - (fighter.footprint - 1)) * root.gridTileWidth / 2
                y: root.gridOriginY + (fighter.modelData.column + fighter.modelData.row) * root.gridTileHeight / 2
                width: root.gridTileWidth * fighter.footprint
                height: root.gridTileHeight * fighter.footprint

                // La surbrillance : au tour (or), allie, ennemi
                Rectangle {
                    id: highlight

                    anchors.centerIn: parent
                    width: fighter.width * 0.62
                    height: width
                    color: fighter.active ? Tokens.goldLight : (fighter.ally ? Tokens.textAlly : Tokens.textEnemy)
                    opacity: fighter.active ? 0.5 : 0.36
                    border.color: fighter.active ? Tokens.goldLight : (fighter.ally ? Tokens.textAlly : Tokens.textEnemy)
                    border.width: fighter.active ? 3 : 2
                    transform: [
                        Rotation { angle: 45; origin.x: highlight.width / 2; origin.y: highlight.height / 2 },
                        Scale { yScale: fighter.height / fighter.width; origin.y: highlight.height / 2 }
                    ]
                }

                // La jauge d'un ennemi, au-dessus de sa figurine
                Gauge {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.top
                    anchors.bottomMargin: fighter.height * 0.55
                    visible: !fighter.ally && !fighter.modelData.down
                    width: fighter.width * 0.5
                    height: Math.max(5, root.gridTileWidth * 0.08)
                    kind: "health"
                    value: fighter.modelData.hitPointsRatio
                }

                // Les points de vie, sous la case
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.bottom
                    anchors.topMargin: -root.gridTileHeight * 0.3
                    visible: text.length > 0
                    text: fighter.modelData.hitPoints
                    color: fighter.ally ? Tokens.textAlly : Tokens.textOnPanel
                    font.family: Tokens.bodyFamily
                    font.pixelSize: Math.max(9, root.gridTileWidth * 0.2)
                    style: Text.Outline
                    styleColor: Tokens.panel
                }
            }
        }

        // Chemin de ciblage (LOT-24)
        Repeater {
            model: root.pathCells

            ArenaMark {
                kind: "path"
                x: root.gridOriginX + (modelData.column - modelData.row) * root.gridTileWidth / 2
                y: root.gridOriginY + (modelData.column + modelData.row) * root.gridTileHeight / 2
                width: root.gridTileWidth
                height: root.gridTileHeight
            }
        }

        // Curseur de ciblage (LOT-24)
        ArenaMark {
            kind: "cursor"
            visible: !root.ended
            x: root.gridOriginX + (root.cursorColumn - root.cursorRow) * root.gridTileWidth / 2
            y: root.gridOriginY + (root.cursorColumn + root.cursorRow) * root.gridTileHeight / 2
            width: root.gridTileWidth
            height: root.gridTileHeight
        }

        // La souris : le survol vise, le clic se deplace ou attaque.
        MouseArea {
            anchors.fill: parent
            enabled: !root.ended
            hoverEnabled: true
            onPositionChanged: (mouse) => root.gridHovered(mouse.x, mouse.y)
            onClicked: (mouse) => root.gridClicked(mouse.x, mouse.y)
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
