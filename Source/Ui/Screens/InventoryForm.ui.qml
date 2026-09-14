pragma ComponentBehavior: Bound
import QtQuick
import Jadg.Ui

/*!
    Inventaire et equipement -- FORMULAIRE, cote conception (LOT-87, T3.5 ; maquette 04).

    Transcrit de la maquette `04_Inventory_Equipment_Mockup.png` (1536 x 1024), cotes ramenees a
    1920 x 1080 puis multipliees par `Tokens.uiScale`. Trois parchemins cote a cote :

    - a gauche, les emplacements portes (cinq de chaque cote) et les quatre statistiques derivees ;
    - au centre, les onglets de filtre, la grille du sac, la charge, l'or et le bouton Trier ;
    - a droite, la fiche de l'objet selectionne, et ce qu'on peut en faire.

    **Les cases sont des repetitions.** Le formulaire ne sait dessiner qu'une case ; les emplacements
    et la grille sont des `Repeater`, exposes au jumeau (`slotRepeaterLeft`, `slotRepeaterRight`,
    `cellRepeater`) pour qu'il branche le pointeur de chaque case -- un formulaire ne contient pas
    de code.

    Les proprietes portent des VALEURS D'EXEMPLE ; le jumeau les remplace par celles de la vue-modele.
*/
Item {
    id: root

    /// Ce que chaque emplacement porte, par identifiant (`main-hand`) : `{itemId, name}`.
    property var equipped: ({
        "torso": { itemId: "cuir-cloute", name: "Cuir clouté" },
        "main-hand": { itemId: "epee-longue", name: "Épée longue" },
        "off-hand": { itemId: "bouclier", name: "Bouclier" }
    })

    /// La grille : `{itemId, name, quantity}`, dans l'ordre du sac.
    property var cells: [
        { itemId: "dague", name: "Dague", quantity: 1 },
        { itemId: "torche", name: "Torche", quantity: 5 }
    ]

    property int filter: 0
    property string selectedItem: "dague"
    property string selectedSlot: ""

    /// La fiche de la selection (vide sans selection).
    property var selection: ({
        name: "Dague", kind: "Arme courante", damage: "1d4", armor: "", weight: "0,5 kg",
        text: "Finesse, légère, lancer", canEquip: true, canUnequip: false, canDrop: true
    })

    property string carried: "12,5 kg"
    property string capacity: "225,0 kg"
    property real loadRatio: 0.06
    property string gold: "42"

    property string armorClass: "16"
    property string initiative: "+1"
    property string speed: "9 m"
    property string passivePerception: "13"

    property alias slotRepeaterLeft: slotsLeft
    property alias slotRepeaterRight: slotsRight
    property alias cellRepeater: cellsRepeater
    property alias allTab: allTabControl
    property alias equipmentTab: equipmentTabControl
    property alias gearTab: gearTabControl
    property alias toolsTab: toolsTabControl
    property alias sortButton: sortControl
    property alias equipButton: equipControl
    property alias dropButton: dropControl

    readonly property bool hasSelection: root.selection.name !== undefined

    width: 1920
    height: 1080

    Rectangle {
        anchors.fill: parent
        color: Tokens.frameEdge
    }

    // === Parchemin gauche : l'equipement porte (maquette : 30, 15 -> 540, 1010) ====================
    PanelFrame {
        x: 38 * Tokens.uiScale
        y: 16 * Tokens.uiScale
        width: 638 * Tokens.uiScale
        height: 1048 * Tokens.uiScale
        material: "parchment"
        bound: true
        padding: 0
    }

    FixedArt {
        x: 38 * Tokens.uiScale
        y: 16 * Tokens.uiScale
        width: 96 * Tokens.uiScale
        height: 120 * Tokens.uiScale
        key: "ui/ornament/crest-pennant"
    }

    TitlePlate {
        x: 140 * Tokens.uiScale
        y: 44 * Tokens.uiScale
        width: 510 * Tokens.uiScale
        material: "black"
        text: qsTr("Inventaire & équipement")
    }

    FixedArt {
        x: 157 * Tokens.uiScale
        y: 240 * Tokens.uiScale
        width: 400 * Tokens.uiScale
        height: 400 * Tokens.uiScale
        key: "ui/ornament/compass-watermark/parchment"
    }

    // Cinq emplacements de chaque cote (maquette : x 70 et 420, pas de 110 en hauteur).
    Column {
        x: 88 * Tokens.uiScale
        y: 160 * Tokens.uiScale
        spacing: 16 * Tokens.uiScale

        Repeater {
            id: slotsLeft

            model: ["head", "cloak", "torso", "hands", "feet"]

            ItemSlot {
                id: leftSlot

                required property string modelData

                width: 104 * Tokens.uiScale
                height: 104 * Tokens.uiScale
                label: root.equipped[leftSlot.modelData] ? root.equipped[leftSlot.modelData].name : ""
                equipped: leftSlot.label.length > 0
                selected: root.selectedSlot === leftSlot.modelData
            }
        }
    }

    Column {
        x: 530 * Tokens.uiScale
        y: 160 * Tokens.uiScale
        spacing: 16 * Tokens.uiScale

        Repeater {
            id: slotsRight

            model: ["main-hand", "off-hand", "ranged", "ring-left", "neck"]

            ItemSlot {
                id: rightSlot

                required property string modelData

                width: 104 * Tokens.uiScale
                height: 104 * Tokens.uiScale
                label: root.equipped[rightSlot.modelData] ? root.equipped[rightSlot.modelData].name : ""
                equipped: rightSlot.label.length > 0
                selected: root.selectedSlot === rightSlot.modelData
            }
        }
    }

    // Les quatre statistiques derivees (maquette : 60, 725 -> 525, 820).
    Row {
        x: 82 * Tokens.uiScale
        y: 790 * Tokens.uiScale
        spacing: 30 * Tokens.uiScale

        StatMedallion { kind: "derived"; label: qsTr("CA"); value: root.armorClass }
        StatMedallion { kind: "derived"; label: qsTr("INIT."); value: root.initiative }
        StatMedallion { kind: "derived"; label: qsTr("VITESSE"); value: root.speed }
        StatMedallion { kind: "derived"; label: qsTr("PERC. PASS."); value: root.passivePerception }
    }

    FixedArt {
        x: 75 * Tokens.uiScale
        y: 935 * Tokens.uiScale
        width: 100 * Tokens.uiScale
        height: 100 * Tokens.uiScale
        key: "ui/ornament/wax-seal"
    }

    FixedArt {
        x: 525 * Tokens.uiScale
        y: 925 * Tokens.uiScale
        width: 112 * Tokens.uiScale
        height: 112 * Tokens.uiScale
        key: "ui/ornament/crossed-crest"
    }

    // === Parchemin central : le sac (maquette : 550, 15 -> 1080, 1010) ============================
    PanelFrame {
        x: 688 * Tokens.uiScale
        y: 16 * Tokens.uiScale
        width: 663 * Tokens.uiScale
        height: 1048 * Tokens.uiScale
        material: "parchment"
        padding: 0
    }

    // Onglets de filtre (maquette : 560, 70 -> 1065, 110). Les familles des DONNEES, pas celles de
    // la maquette : aucun objet n'est « consommable » ou « divers » dans les catalogues.
    Row {
        x: 712 * Tokens.uiScale
        y: 70 * Tokens.uiScale
        spacing: 4 * Tokens.uiScale

        OrnateTab {
            id: allTabControl
            width: 150 * Tokens.uiScale
            // Marges resserrees : quatre segments tiennent dans la largeur de la grille.
            leftPadding: 10 * Tokens.uiScale
            rightPadding: 10 * Tokens.uiScale
            material: "parchment"
            text: qsTr("Tous")
            checkable: false
            checked: root.filter === 0
            focusPolicy: Qt.NoFocus
        }
        OrnateTab {
            id: equipmentTabControl
            width: 150 * Tokens.uiScale
            // Marges resserrees : quatre segments tiennent dans la largeur de la grille.
            leftPadding: 10 * Tokens.uiScale
            rightPadding: 10 * Tokens.uiScale
            material: "parchment"
            text: qsTr("Équipement")
            checkable: false
            checked: root.filter === 1
            focusPolicy: Qt.NoFocus
        }
        OrnateTab {
            id: gearTabControl
            width: 150 * Tokens.uiScale
            // Marges resserrees : quatre segments tiennent dans la largeur de la grille.
            leftPadding: 10 * Tokens.uiScale
            rightPadding: 10 * Tokens.uiScale
            material: "parchment"
            text: qsTr("Matériel")
            checkable: false
            checked: root.filter === 2
            focusPolicy: Qt.NoFocus
        }
        OrnateTab {
            id: toolsTabControl
            width: 150 * Tokens.uiScale
            // Marges resserrees : quatre segments tiennent dans la largeur de la grille.
            leftPadding: 10 * Tokens.uiScale
            rightPadding: 10 * Tokens.uiScale
            material: "parchment"
            text: qsTr("Outils")
            checkable: false
            checked: root.filter === 3
            focusPolicy: Qt.NoFocus
        }
    }

    // La grille : cinq colonnes sur six rangs (maquette : 575, 140 -> 1060, 755).
    Grid {
        id: grid

        x: 719 * Tokens.uiScale
        y: 150 * Tokens.uiScale
        columns: 5
        spacing: 12 * Tokens.uiScale

        Repeater {
            id: cellsRepeater

            model: root.cells

            ItemSlot {
                id: cell

                required property var modelData

                width: 110 * Tokens.uiScale
                height: 96 * Tokens.uiScale
                label: cell.modelData.name
                quantity: cell.modelData.quantity
                selected: root.selectedItem === cell.modelData.itemId
            }
        }

        // Les cases libres completent la grille : la place du sac se lit, pas seulement son contenu.
        Repeater {
            model: Math.max(0, 30 - root.cells.length)

            ItemSlot {
                width: 110 * Tokens.uiScale
                height: 96 * Tokens.uiScale
                enabled: false
            }
        }
    }

    // La charge (maquette : 570, 780 -> 1060, 840).
    Item {
        x: 719 * Tokens.uiScale
        y: 812 * Tokens.uiScale
        width: 606 * Tokens.uiScale
        height: 72 * Tokens.uiScale

        FixedArt {
            id: weightIcon

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            width: 56 * Tokens.uiScale
            height: 56 * Tokens.uiScale
            key: "ui/icon/resource/weight"
        }

        Text {
            anchors.left: weightIcon.right
            anchors.leftMargin: Tokens.gapMedium
            anchors.top: parent.top
            text: qsTr("Poids : %1 / %2").arg(root.carried).arg(root.capacity)
            color: Tokens.text
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontBody
        }

        Gauge {
            anchors.left: weightIcon.right
            anchors.leftMargin: Tokens.gapMedium
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            kind: "weight"
            value: root.loadRatio
        }
    }

    // L'or, l'ecusson et le tri (maquette : 570, 870 -> 1055, 935).
    SectionBanner {
        x: 719 * Tokens.uiScale
        y: 925 * Tokens.uiScale
        width: 330 * Tokens.uiScale
        material: "dark"
        text: qsTr("Pièces d'or") + "   " + root.gold
    }

    OrnateButton {
        id: sortControl

        x: 1105 * Tokens.uiScale
        y: 925 * Tokens.uiScale
        width: 220 * Tokens.uiScale
        kind: "secondary"
        text: qsTr("Trier")
    }

    // === Parchemin droit : la fiche de l'objet (maquette : 1090, 15 -> 1515, 1010) ================
    PanelFrame {
        x: 1363 * Tokens.uiScale
        y: 16 * Tokens.uiScale
        width: 531 * Tokens.uiScale
        height: 1048 * Tokens.uiScale
        material: "parchment"
        padding: 0
    }

    FixedArt {
        x: 1790 * Tokens.uiScale
        y: 16 * Tokens.uiScale
        width: 96 * Tokens.uiScale
        height: 120 * Tokens.uiScale
        key: "ui/ornament/crest-pennant"
    }

    FixedArt {
        x: 1430 * Tokens.uiScale
        y: 230 * Tokens.uiScale
        width: 240 * Tokens.uiScale
        height: 240 * Tokens.uiScale
        key: "ui/ornament/compass-watermark/parchment"
    }

    TitlePlate {
        anchors.horizontalCenter: parent.left
        anchors.horizontalCenterOffset: 1615 * Tokens.uiScale
        y: 96 * Tokens.uiScale
        width: 440 * Tokens.uiScale
        material: "black"
        visible: root.hasSelection
        text: root.hasSelection ? root.selection.name : ""
    }

    Text {
        x: 1420 * Tokens.uiScale
        y: 380 * Tokens.uiScale
        width: 430 * Tokens.uiScale
        visible: !root.hasSelection
        text: qsTr("Choisissez un objet du sac ou un emplacement porté.")
        color: Tokens.textMuted
        font.family: Tokens.loreFamily
        font.italic: true
        font.pixelSize: Tokens.fontBody
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
    }

    Column {
        x: 1420 * Tokens.uiScale
        y: 230 * Tokens.uiScale
        width: 430 * Tokens.uiScale
        spacing: Tokens.gapSmall
        visible: root.hasSelection

        Text {
            width: parent.width
            text: root.hasSelection ? root.selection.kind : ""
            color: Tokens.text
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontBody
            wrapMode: Text.WordWrap
        }

        Text {
            width: parent.width
            text: root.hasSelection ? qsTr("Poids : %1").arg(root.selection.weight) : ""
            color: Tokens.textMuted
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontCaption
        }

        Item { width: 1; height: 150 * Tokens.uiScale }

        SectionBanner {
            width: parent.width
            visible: root.hasSelection && root.selection.damage.length > 0
            text: qsTr("Dégâts")
        }

        Text {
            width: parent.width
            visible: root.hasSelection && root.selection.damage.length > 0
            text: root.hasSelection ? root.selection.damage : ""
            color: Tokens.text
            font.family: Tokens.titleFamily
            font.pixelSize: Tokens.fontSectionTitle
        }

        SectionBanner {
            width: parent.width
            visible: root.hasSelection && root.selection.armor.length > 0
            text: qsTr("Protection")
        }

        Text {
            width: parent.width
            visible: root.hasSelection && root.selection.armor.length > 0
            text: root.hasSelection ? root.selection.armor : ""
            color: Tokens.text
            font.family: Tokens.titleFamily
            font.pixelSize: Tokens.fontSectionTitle
        }

        SectionBanner {
            width: parent.width
            visible: root.hasSelection && root.selection.text.length > 0
            text: qsTr("Propriétés")
        }

        Text {
            width: parent.width
            visible: root.hasSelection && root.selection.text.length > 0
            text: root.hasSelection ? root.selection.text : ""
            color: Tokens.text
            font.family: Tokens.bodyFamily
            font.pixelSize: Tokens.fontBody
            wrapMode: Text.WordWrap
        }
    }

    // Les actions (maquette : 1140, 755 -> 1450, 925). « Equiper » devient « Retirer » sur un
    // emplacement porte ; « Examiner » n'existe pas : la fiche est deja sous les yeux.
    Column {
        x: 1425 * Tokens.uiScale
        y: 850 * Tokens.uiScale
        spacing: Tokens.gapMedium

        OrnateButton {
            id: equipControl

            width: 400 * Tokens.uiScale
            kind: "primary"
            enabled: root.hasSelection && (root.selection.canEquip || root.selection.canUnequip)
            text: root.hasSelection && root.selection.canUnequip ? qsTr("Retirer") : qsTr("Équiper")
        }

        OrnateButton {
            id: dropControl

            width: 400 * Tokens.uiScale
            kind: "secondary"
            enabled: root.hasSelection && root.selection.canDrop
            text: qsTr("Jeter")
        }
    }
}
