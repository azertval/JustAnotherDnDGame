pragma ComponentBehavior: Bound
import QtQuick
import Jadg.Ui

/*!
    Carte du monde -- FORMULAIRE, cote conception (LOT-87, T3.6 ; maquette 08).

    Transcrit de la maquette `08_Map_Mockup.png` (1492 x 1054) : la carte VTT de Tanares en plein
    ecran, qu'on deplace et qu'on agrandit ; par-dessus, la plaque de titre, la barre de boussole,
    l'encart du personnage, la mini-carte et ses boutons de zoom, la barre d'etat (jour et lieu) et
    la barre de navigation.

    **Les reperes sont ceux des regions.** L'atlas du `LOT-37` ne porte aucune coordonnee : les
    treize regions sont posees a des ancres relevees sur la carte (`world-map-regions.json`) ; les
    94 lieux sont listes dans la fiche de leur region, et attendent leur placement (`LOT-42`).

    Les proprietes portent des VALEURS D'EXEMPLE ; le jumeau les remplace.
*/
Item {
    id: root

    /// Les regions : `{regionId, name, x, y, government, locations, locationCount}`.
    property var regions: [
        { regionId: "central-empire", name: "Central Empire", x: 0.693, y: 0.454,
          government: "Absolute monarchy", locations: "Hajal City", locationCount: 1 }
    ]

    /// La region survolee (indice dans `regions`), ou -1.
    property int hoveredRegion: 0

    /// Agrandissement de la carte : 1 remplit la largeur de l'ecran.
    property real zoom: 1

    // --- L'encart du personnage ---------------------------------------------------------------------
    property string characterName: "Brenna Pierrefonte"
    property string levelText: "Niv. 3"
    property string hitPointsText: "25 / 30"
    property real hitPointsRatio: 25 / 30
    property string experienceText: "900 / —"
    property real experienceRatio: 0
    property url portrait: ""

    // --- La barre d'etat --------------------------------------------------------------------------
    property string clock: "—"
    property string partyLocation: "—"

    property alias mapView: mapFlick
    property alias markerRepeater: markers
    property alias zoomInButton: zoomInControl
    property alias zoomOutButton: zoomOutControl
    property alias questsButton: questsControl
    property alias inventoryButton: inventoryControl
    property alias companyButton: companyControl
    property alias optionsButton: optionsControl

    readonly property real mapAspect: 3072 / 2170
    readonly property bool hasHover: root.hoveredRegion >= 0 && root.hoveredRegion < root.regions.length
    readonly property var hovered: root.hasHover ? root.regions[root.hoveredRegion] : ({})

    width: 1920
    height: 1080

    Rectangle {
        anchors.fill: parent
        color: Tokens.panel
    }

    // --- La carte ---------------------------------------------------------------------------------
    Flickable {
        id: mapFlick

        anchors.fill: parent
        contentWidth: root.width * root.zoom
        contentHeight: root.width * root.zoom / root.mapAspect
        boundsBehavior: Flickable.StopAtBounds
        clip: true

        Image {
            id: mapImage

            width: mapFlick.contentWidth
            height: mapFlick.contentHeight
            source: "../../Elements/Assets/UI/world-map-hd.jpg"
            fillMode: Image.Stretch
            smooth: true
            mipmap: true
        }

        Repeater {
            id: markers

            model: root.regions

            MapMarker {
                id: marker

                required property var modelData
                required property int index

                x: marker.modelData.x * mapImage.width - marker.markerSize / 2
                y: marker.modelData.y * mapImage.height - marker.markerSize / 2
                label: marker.modelData.name
                active: root.hoveredRegion === marker.index
            }
        }
    }

    // La fiche de la region survolee, a droite de son repere.
    PanelFrame {
        id: regionCard

        visible: root.hasHover
        x: root.hasHover ? root.hovered.x * mapImage.width - mapFlick.contentX + 48 * Tokens.uiScale : 0
        y: root.hasHover ? root.hovered.y * mapImage.height - mapFlick.contentY + 32 * Tokens.uiScale : 0
        width: 420 * Tokens.uiScale
        height: cardColumn.implicitHeight + 2 * Tokens.gapMedium
        subpanel: true

        Column {
            id: cardColumn

            width: parent.width
            spacing: Tokens.gapSmall

            Text {
                width: parent.width
                text: root.hasHover ? root.hovered.name : ""
                color: Tokens.goldLight
                font.family: Tokens.titleFamily
                font.pixelSize: Tokens.fontSectionTitle
                font.weight: Font.DemiBold
                wrapMode: Text.WordWrap
            }

            Text {
                width: parent.width
                text: root.hasHover ? root.hovered.government : ""
                color: Tokens.textOnPanelMuted
                font.family: Tokens.loreFamily
                font.italic: true
                font.pixelSize: Tokens.fontCaption
                wrapMode: Text.WordWrap
                maximumLineCount: 3
                elide: Text.ElideRight
            }

            GoldDivider {
                width: parent.width
            }

            Text {
                width: parent.width
                text: root.hasHover ? qsTr("%n lieu(x)", "", root.hovered.locationCount) : ""
                color: Tokens.textOnPanel
                font.family: Tokens.titleFamily
                font.pixelSize: Tokens.fontCaption
            }

            Text {
                width: parent.width
                text: root.hasHover ? root.hovered.locations : ""
                color: Tokens.textOnPanel
                font.family: Tokens.bodyFamily
                font.pixelSize: Tokens.fontCaption
                wrapMode: Text.WordWrap
                maximumLineCount: 10
                elide: Text.ElideRight
            }
        }
    }

    // --- Plaque de titre (maquette : 20, 20 -> 510, 130) ----------------------------------------------
    PanelFrame {
        x: 24 * Tokens.uiScale
        y: 24 * Tokens.uiScale
        width: 600 * Tokens.uiScale
        height: 128 * Tokens.uiScale
        subpanel: true

        FixedArt {
            id: crest

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            width: 88 * Tokens.uiScale
            height: 88 * Tokens.uiScale
            key: "ui/ornament/crest-shield"
        }

        Column {
            anchors.left: crest.right
            anchors.leftMargin: Tokens.gapMedium
            anchors.verticalCenter: parent.verticalCenter

            Text {
                text: qsTr("Tanares impérial")
                color: Tokens.goldLight
                font.family: Tokens.titleFamily
                font.pixelSize: Tokens.fontScreenTitle
                font.weight: Font.DemiBold
            }

            Text {
                text: qsTr("Provinces et points d'intérêt")
                color: Tokens.textOnPanel
                font.family: Tokens.loreFamily
                font.italic: true
                font.pixelSize: Tokens.fontBody
            }
        }
    }

    // --- Barre de boussole (maquette : 580, 35 -> 940, 80) ---------------------------------------------
    Item {
        x: 745 * Tokens.uiScale
        y: 40 * Tokens.uiScale
        width: 440 * Tokens.uiScale
        height: 48 * Tokens.uiScale

        Rectangle {
            anchors.fill: parent
            visible: !compassArt.delivered
            color: Tokens.panel
            border.color: Tokens.panelEdge
            border.width: Tokens.strokeWidth
        }

        NinePatchArt {
            id: compassArt

            anchors.fill: parent
            key: "ui/plate/compass-bar"
        }

        Row {
            anchors.centerIn: parent
            spacing: 120 * Tokens.uiScale

            Text { text: qsTr("O"); color: Tokens.textOnPanel; font.family: Tokens.titleFamily; font.pixelSize: Tokens.fontBody }
            Text { text: qsTr("N"); color: Tokens.goldLight; font.family: Tokens.titleFamily; font.pixelSize: Tokens.fontBody; font.weight: Font.Bold }
            Text { text: qsTr("E"); color: Tokens.textOnPanel; font.family: Tokens.titleFamily; font.pixelSize: Tokens.fontBody }
        }
    }

    // --- Encart du personnage (maquette : 1110, 20 -> 1470, 125) -----------------------------------------
    PanelFrame {
        x: 1400 * Tokens.uiScale
        y: 24 * Tokens.uiScale
        width: 496 * Tokens.uiScale
        height: 128 * Tokens.uiScale
        subpanel: true
        padding: Tokens.gapSmall

        PortraitFrame {
            id: characterPortrait

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            shape: "round"
            size: 108 * Tokens.uiScale
            source: root.portrait
        }

        Column {
            anchors.left: characterPortrait.right
            anchors.right: parent.right
            anchors.leftMargin: Tokens.gapMedium
            anchors.rightMargin: Tokens.gapSmall
            anchors.verticalCenter: parent.verticalCenter
            spacing: 4 * Tokens.uiScale

            Text {
                width: parent.width
                text: root.characterName + "  ·  " + root.levelText
                color: Tokens.textOnPanel
                font.family: Tokens.bodyFamily
                font.pixelSize: Tokens.fontBody
                elide: Text.ElideRight
            }

            Gauge {
                width: parent.width
                kind: "health"
                value: root.hitPointsRatio
                label: root.hitPointsText
            }

            Gauge {
                width: parent.width
                kind: "experience"
                value: root.experienceRatio
                label: root.experienceText
            }
        }
    }

    // --- Mini-carte et zoom (maquette : 1280, 135 -> 1470, 320) ------------------------------------------
    PortraitFrame {
        id: minimap

        x: 1660 * Tokens.uiScale
        y: 172 * Tokens.uiScale
        shape: "round"
        size: 220 * Tokens.uiScale
        source: "../../Elements/Assets/UI/world-map-hd.jpg"
    }

    Column {
        x: 1820 * Tokens.uiScale
        y: 380 * Tokens.uiScale
        spacing: Tokens.gapSmall

        OrnateButton {
            id: zoomInControl

            width: 64 * Tokens.uiScale
            leftPadding: 0
            rightPadding: 0
            kind: "secondary"
            text: "+"
            focusPolicy: Qt.NoFocus
        }

        OrnateButton {
            id: zoomOutControl

            width: 64 * Tokens.uiScale
            leftPadding: 0
            rightPadding: 0
            kind: "secondary"
            text: "−"
            focusPolicy: Qt.NoFocus
        }
    }

    // --- Barre d'etat : jour et lieu du groupe (maquette : 25, 965 -> 390, 1015) ---------------------------
    Item {
        x: 24 * Tokens.uiScale
        y: 1000 * Tokens.uiScale
        width: 520 * Tokens.uiScale
        height: 56 * Tokens.uiScale

        Rectangle {
            anchors.fill: parent
            visible: !statusArt.delivered
            color: Tokens.panel
            border.color: Tokens.panelEdge
            border.width: Tokens.strokeWidth
        }

        NinePatchArt {
            id: statusArt

            anchors.fill: parent
            key: "ui/plate/status-bar"
        }

        Row {
            anchors.left: parent.left
            anchors.leftMargin: Tokens.gapLarge
            anchors.verticalCenter: parent.verticalCenter
            spacing: Tokens.gapLarge

            Text {
                text: root.clock
                color: Tokens.textOnPanel
                font.family: Tokens.bodyFamily
                font.pixelSize: Tokens.fontBody
            }

            Text {
                text: root.partyLocation
                color: Tokens.textOnPanel
                font.family: Tokens.bodyFamily
                font.pixelSize: Tokens.fontBody
            }
        }
    }

    // --- Barre de navigation (maquette : 1030, 920 -> 1475, 1015) --------------------------------------
    PanelFrame {
        x: 1330 * Tokens.uiScale
        y: 960 * Tokens.uiScale
        width: 566 * Tokens.uiScale
        height: 100 * Tokens.uiScale
        subpanel: true
        padding: Tokens.gapSmall

        Row {
            anchors.centerIn: parent
            spacing: Tokens.gapSmall

            OrnateButton {
                width: 100 * Tokens.uiScale
                leftPadding: 0
                rightPadding: 0
                kind: "primary"
                text: qsTr("Carte")
                enabled: false
                focusPolicy: Qt.NoFocus
            }
            OrnateButton {
                id: questsControl
                width: 100 * Tokens.uiScale
                leftPadding: 0
                rightPadding: 0
                kind: "secondary"
                text: qsTr("Quêtes")
                focusPolicy: Qt.NoFocus
            }
            OrnateButton {
                id: inventoryControl
                width: 100 * Tokens.uiScale
                leftPadding: 0
                rightPadding: 0
                kind: "secondary"
                text: qsTr("Sac")
                focusPolicy: Qt.NoFocus
            }
            OrnateButton {
                id: companyControl
                width: 100 * Tokens.uiScale
                leftPadding: 0
                rightPadding: 0
                kind: "secondary"
                text: qsTr("Équipe")
                focusPolicy: Qt.NoFocus
            }
            OrnateButton {
                id: optionsControl
                width: 100 * Tokens.uiScale
                leftPadding: 0
                rightPadding: 0
                kind: "secondary"
                text: qsTr("Options")
                focusPolicy: Qt.NoFocus
            }
        }
    }
}
