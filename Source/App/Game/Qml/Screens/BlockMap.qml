import QtQuick
import Jadg.Ui

/*!
    Carte, vue ilot -- CABLAGE, cote developpeur (LOT-96).

    Cinquieme niveau de l'ecran « Carte ». `WorldMap.qml` lui passe l'ilot choisi, son quartier et
    l'adresse de son image (`CityDistrictModel.blockImage`, dessinee par `image://cityblock/`).
*/
BlockMapForm {
    id: root

    property var point: null
    property var block: null
    property var city: ({})
    property var region: null
    property string imageSource: ""
    property bool heroInBlock: false

    blockImage: root.imageSource
    blockName: root.block ? root.block.name : ""
    districtName: root.point ? root.point.name : ""
    cityName: root.city.name !== undefined ? root.city.name : ""
    regionName: root.region ? root.region.name : ""
    cityImage: root.city.image !== undefined ? root.city.image : ""
    districtFrame: root.point && root.point.frame !== undefined ? root.point.frame
                                                               : Qt.rect(0, 0, 1, 1)
    heroHere: root.heroInBlock
}
