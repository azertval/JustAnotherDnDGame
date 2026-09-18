import QtQuick
import Jadg.Ui

/*!
    Carte -- vue ZOOM VILLE -- FORMULAIRE, cote conception (LOT-94, LOT-95).

    Troisieme et dernier niveau de l'ecran « Carte » : le plan d'une ville, peint par l'auteur sans
    lettrage. Les reperes sont NUMEROTES comme la legende du livre -- les douze quartiers de la
    Capitale imperiale (des lieux de l'atlas), les douze lieux de Fisherman's Wharf -- et la fiche,
    a gauche, en donne la liste et le texte.

    Le plan sert a s'orienter : on ne s'y deplace pas, on n'y voyage pas (feuille de route, §8). Le
    deplacement se fait sur les cartes de niveau.

    Les proprietes portent des VALEURS D'EXEMPLE ; le jumeau (`WorldMap.qml`) les remplace.
*/
Item {
    id: root

    property string cityImage: "city-central-empire-the-capital-city.jpg"
    property string cityName: "The Capital City"
    property string regionName: "Central Empire"
    property string regionImage: "region-central-empire.jpg"

    /// Position de la ville sur la carte de sa region, en fractions : le cadre de la mini-carte.
    property real cityX: 0.59
    property real cityY: 0.565

    /// Les quartiers et lieux numerotes : `{ name, x, y, kind, number, gateway, placed,
    /// description }`.
    property var points: [
        { name: "Sloghood", x: 0.276, y: 0.7, kind: "point-of-interest", number: 1,
          gateway: false, placed: true, description: "" },
        { name: "Martpart", x: 0.702, y: 0.505, kind: "point-of-interest", number: 6,
          gateway: false, placed: true, description: "" },
        { name: "Arenarea", x: 0.562, y: 0.337, kind: "point-of-interest", number: 7,
          gateway: false, placed: true, description: "" }
    ]
    property var labels: [{ name: "Arena of Fate", kind: "site", x: 0.534, y: 0.236 }]

    /// Le point choisi (indice dans `points`), ou -1.
    property int selectedPoint: 1
    /// Le quartier ou se tient le heros, en fractions du plan ; `x < 0` hors de la ville (LOT-96).
    property point here: Qt.point(0.702, 0.505)
    property string pointDescription: "As the Capital's market district, it boasts diverse products and crowds of patrons."

    property alias canvas: mapCanvas
    property alias hud: mapHud
    property alias panel: sidePanel

    width: 1920
    height: 1080

    MapCanvas {
        id: mapCanvas

        anchors.fill: parent
        image: root.cityImage
        markers: root.points
        labels: root.labels
        activeIndex: root.selectedPoint
        here: root.here
        leftInset: sidePanel.x + sidePanel.width
    }

    MapSidePanel {
        id: sidePanel

        x: 24 * Tokens.uiScale
        y: 172 * Tokens.uiScale
        width: 460 * Tokens.uiScale
        height: root.height - 272 * Tokens.uiScale
        title: root.cityName
        lore: ""
        facts: [root.regionName]
        grades: []
        entriesTitle: qsTr("Quartiers et lieux")
        entries: root.points
        selectedIndex: root.selectedPoint
        description: root.pointDescription
    }

    MapHud {
        id: mapHud

        anchors.fill: parent
        title: root.cityName
        trail: qsTr("Tanares  ›  %1  ›  %2").arg(root.regionName).arg(root.cityName)
        parentImage: root.regionImage
        parentFrame: Qt.rect(Math.max(0, Math.min(0.84, root.cityX - 0.08)),
                             Math.max(0, Math.min(0.84, root.cityY - 0.08)), 0.16, 0.16)
        canGoBack: true
    }
}
