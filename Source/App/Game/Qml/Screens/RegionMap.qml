import QtQuick
import Jadg.Ui

/*!
    Carte, vue region -- CABLAGE, cote developpeur (LOT-94, LOT-95).

    Deuxieme niveau de l'ecran « Carte ». Ce jumeau ne charge rien : `WorldMap.qml`, qui tient les
    trois niveaux, lui passe la region choisie (une table de `WorldMapModel.regions`) et l'indice du
    lieu choisi ; il en tire ce que le formulaire montre.
*/
RegionMapForm {
    id: root

    /// La region montree : une table de `WorldMapModel.regions`, ou `null`.
    property var region: null
    /// Le lieu choisi, indice dans `region.places`, ou -1.
    property int placeIndex: -1
    property string worldImage: ""

    readonly property var shownPlaces: root.region ? root.region.places : []
    readonly property var shownPlace: root.placeIndex >= 0 && root.placeIndex < root.shownPlaces.length
                                      ? root.shownPlaces[root.placeIndex] : null

    regionImage: root.region ? root.region.image : ""
    regionName: root.region ? root.region.name : ""
    government: root.region ? root.region.government : ""
    facts: {
        const facts = []
        if (root.region && root.region.faction !== "")
            facts.push(root.region.faction)
        if (root.region && root.region.population > 0)
            facts.push(qsTr("%L1 habitants").arg(root.region.population))
        return facts
    }
    grades: root.region ? root.region.grades : []
    places: root.shownPlaces
    // Les lieux poses sont les premiers de la liste : l'indice d'un repere est celui de son lieu.
    markers: root.shownPlaces.filter((place) => place.placed)
    labels: root.region ? root.region.labels : []
    selectedPlace: root.placeIndex
    placeDescription: {
        if (!root.shownPlace)
            return ""
        if (root.shownPlace.placed)
            return root.shownPlace.description
        return qsTr("Le livre ne situe pas ce lieu sur la carte.") + "\n" + root.shownPlace.description
    }

    hud.parentImage: root.worldImage
    hud.parentFrame: root.region ? Qt.rect(root.region.frame.x, root.region.frame.y,
                                           root.region.frame.width, root.region.frame.height)
                                 : Qt.rect(0, 0, 1, 1)
}
