import QtQuick
import Jadg.Ui

/*!
    Carte, vue quartier -- CABLAGE, cote developpeur (LOT-96).

    Quatrieme niveau de l'ecran « Carte ». Ce jumeau ne charge rien : `WorldMap.qml` lui passe le
    point du quartier sur le plan de la ville (une table de `WorldMapModel.city(...).points`), le
    quartier lu par `CityDistrictModel.district(...)`, l'indice de l'ilot choisi et la place du
    heros ; il en tire ce que le formulaire montre.
*/
DistrictMapForm {
    id: root

    /// Le point du quartier sur le plan de sa ville, ou `null`.
    property var point: null
    /// Le quartier : une table de `CityDistrictModel.district(mapId)`.
    property var district: ({})
    property var city: ({})
    property var region: null
    /// L'ilot choisi, indice dans `district.blocks`, ou -1.
    property int blockIndex: -1
    /// Le heros, en fractions du quartier ; `x < 0` s'il n'y est pas.
    property point heroAt: Qt.point(-1, -1)

    readonly property var shownBlocks: root.district.blocks !== undefined ? root.district.blocks : []

    cityImage: root.city.image !== undefined ? root.city.image : ""
    districtImage: root.point && root.point.districtImage !== undefined ? root.point.districtImage : ""
    frame: root.point && root.point.frame !== undefined ? root.point.frame : Qt.rect(0, 0, 1, 1)
    districtName: root.point ? root.point.name : ""
    cityName: root.city.name !== undefined ? root.city.name : ""
    regionName: root.region ? root.region.name : ""
    // Les ilots, numerotes dans l'ordre de la carte ; chacun s'ouvre sur sa vue.
    blocks: root.shownBlocks.map((block, index) => Object.assign(
        { kind: "point-of-interest", number: index + 1, gateway: true, placed: true }, block))
    selectedBlock: root.blockIndex
    blockDescription: ""
    here: root.heroAt
}
