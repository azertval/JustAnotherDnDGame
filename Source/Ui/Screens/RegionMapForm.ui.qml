import QtQuick
import Jadg.Ui

/*!
    Carte -- vue ZOOM REGION -- FORMULAIRE, cote conception (LOT-94, LOT-95).

    Deuxieme des trois niveaux de l'ecran « Carte ». La carte de la region, peinte par l'auteur sans
    lettrage, porte les lieux de l'atlas (`LOT-37`) a leurs positions relevees (`world-maps.json`)
    et quelques noms de geographie. A gauche, la fiche de la region : regime, faction, population,
    les sept statistiques regionales du livre, la liste des lieux et le texte du lieu choisi.

    Un lieu que l'atlas connait mais que la carte ne situe pas reste dans la liste, en estompe : le
    livre ne dit pas ou il est, et le jeu ne l'invente pas. Un lieu marque ◈ a un plan de ville :
    l'activer zoome encore (`CityMapForm`).

    Les proprietes portent des VALEURS D'EXEMPLE ; le jumeau (`WorldMap.qml`) les remplace.
*/
Item {
    id: root

    property string regionImage: "region-central-empire.jpg"
    property string regionName: "Central Empire"
    property string government: "The center of power of the Tanarean Empire, under absolute control of Emperor Baleroth, the Faceless Emperor."
    property var facts: ["Tanarean Empire", "2 300 000 habitants"]
    property var grades: [0, 1, 4, 3, 1, 1, 3]

    /// Les lieux, poses d'abord : `{ name, x, y, kind, number, gateway, placed, description }`.
    property var places: [
        { name: "The Capital City", x: 0.59, y: 0.565, kind: "city", number: 0, gateway: true,
          placed: true, description: "" },
        { name: "Hajal City", x: 0.63, y: 0.19, kind: "point-of-interest", number: 0,
          gateway: false, placed: true, description: "" },
        { name: "Cursed Ground", x: 0, y: 0, kind: "point-of-interest", number: 0,
          gateway: false, placed: false, description: "" }
    ]

    /// Les lieux poses, seuls : ce que la carte montre (les premiers de `places`).
    property var markers: [
        { name: "The Capital City", x: 0.59, y: 0.565, kind: "city", number: 0, gateway: true },
        { name: "Hajal City", x: 0.63, y: 0.19, kind: "point-of-interest", number: 0,
          gateway: false }
    ]
    property var labels: [{ name: "Lake Plavo", kind: "lake", x: 0.7, y: 0.41 }]

    /// Le lieu choisi (indice dans `places`), ou -1.
    property int selectedPlace: 0
    property string placeDescription: "The Imperial Capital, situated in the heart of Tanares, serves as Emperor Baleroth's seat of power and is the largest city on the continent."

    property alias canvas: mapCanvas
    property alias hud: mapHud
    property alias panel: sidePanel

    width: 1920
    height: 1080

    MapCanvas {
        id: mapCanvas

        anchors.fill: parent
        image: root.regionImage
        markers: root.markers
        labels: root.labels
        activeIndex: root.selectedPlace
        namesAlways: true
        leftInset: sidePanel.x + sidePanel.width
    }

    MapSidePanel {
        id: sidePanel

        x: 24 * Tokens.uiScale
        y: 172 * Tokens.uiScale
        width: 460 * Tokens.uiScale
        height: root.height - 272 * Tokens.uiScale
        title: root.regionName
        lore: root.government
        facts: root.facts
        grades: root.grades
        statisticLabels: [qsTr("Liberté"), qsTr("Crime"), qsTr("Prospérité"), qsTr("Corruption"),
            qsTr("Magie"), qsTr("Monstres"), qsTr("Stabilité")]
        entriesTitle: qsTr("Lieux")
        entries: root.places
        selectedIndex: root.selectedPlace
        description: root.placeDescription
    }

    MapHud {
        id: mapHud

        anchors.fill: parent
        title: root.regionName
        trail: qsTr("Tanares  ›  %1").arg(root.regionName)
        parentImage: "world.jpg"
        canGoBack: true
    }
}
