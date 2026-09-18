import QtQuick

/*!
    Doublure de `hmi::CityDistrictModel` pour Qt Design Studio (LOT-96).

    Memes methodes que le type C++ (`Source/HMI/Runtime/CityDistrictModel.h`) : Martpart et deux de
    ses ilots, pour juger les vues « quartier » et « ilot » dans l'atelier. L'image d'un ilot est
    dessinee par le jeu ; l'atelier n'en a pas.
*/
QtObject {
    function district(mapId) {
        return {
            mapId: mapId, columns: 48, rows: 40,
            blocks: [
                { blockId: "place-du-marche", name: "La place du marché", column: 12, row: 11,
                  width: 20, height: 18, x: 0.45, y: 0.49 },
                { blockId: "arene-illu-die", name: "L'arène Illu Die", column: 32, row: 5,
                  width: 16, height: 20, x: 0.83, y: 0.37 }
            ]
        }
    }
    function blockImage(mapId, blockId, figure, heroColumn, heroRow) { return "" }
    function blockAt(mapId, column, row) { return "" }
}
