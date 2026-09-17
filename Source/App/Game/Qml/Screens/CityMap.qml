import QtQuick
import Jadg.Ui

/*!
    Carte, vue ville -- CABLAGE, cote developpeur (LOT-94, LOT-95).

    Troisieme niveau de l'ecran « Carte ». Ce jumeau ne charge rien : `WorldMap.qml` lui passe le
    plan (`WorldMapModel.city(...)`), la region et le lieu d'ou l'on vient, et l'indice du point
    choisi.
*/
CityMapForm {
    id: root

    /// Le plan montre : une table de `WorldMapModel.city(placeId)`, vide tant qu'aucun n'est ouvert.
    property var city: ({})
    /// La region de la ville, et la ville comme lieu de cette region (pour la mini-carte).
    property var region: null
    property var place: null
    /// Le point choisi, indice dans `city.points`, ou -1.
    property int pointIndex: -1

    readonly property var shownPoints: root.city.points !== undefined ? root.city.points : []

    cityImage: root.city.image !== undefined ? root.city.image : ""
    cityName: root.city.name !== undefined ? root.city.name : ""
    regionName: root.region ? root.region.name : ""
    regionImage: root.region ? root.region.image : ""
    cityX: root.place ? root.place.x : 0.5
    cityY: root.place ? root.place.y : 0.5
    // Tout point d'un plan est pose : la liste ne porte aucun lieu en estompe.
    points: root.shownPoints.map((point) => Object.assign({ placed: true }, point))
    labels: root.city.labels !== undefined ? root.city.labels : []
    selectedPoint: root.pointIndex
    pointDescription: root.pointIndex >= 0 && root.pointIndex < root.shownPoints.length
                      ? root.shownPoints[root.pointIndex].description : ""
}
