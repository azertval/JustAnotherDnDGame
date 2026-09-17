pragma Singleton
import QtQuick

/*!
    Doublure de `hmi::WorldModel` pour Qt Design Studio (LOT-09).

    Memes proprietes et methodes que le type C++ (`Source/HMI/Runtime/WorldModel.h`), avec une
    carte d'exemple chargee : l'ecran de jeu se dessine ainsi dans l'atelier, HUD compris. Les
    methodes ne font rien -- dans l'atelier, personne ne marche.
*/
QtObject {
    readonly property string mapId: "coliseum"
    readonly property string mapName: "Le Colisée"
    readonly property string status: ""
    readonly property bool loaded: true
    readonly property int columns: 40
    readonly property int rows: 30
    readonly property real heroColumn: 20.5
    readonly property real heroRow: 15.5
    property string heroFigure: "jade"
    property bool frozen: false

    function startNewGame() { return true }
    function enterMap(mapId, arrival) { return true }
    function setMove(x, y) {}
    function interact() {}
}
