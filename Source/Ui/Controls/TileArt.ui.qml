import QtQuick
import Jadg.Ui

/*!
    Une piece « tuile » du cahier des assets : un sol, un mur, un parchemin repete (LOT-50).

    Le quatrieme porteur d'image, a cote de `NinePatchArt`, `FixedArt` et `CoverArt`, pour les
    pieces que le cahier declare `display: tile`. L'image se repete dans les deux sens a sa taille
    de production, multipliee par `uiScale` comme les autres pieces : une tuile de 256 px couvre
    donc 256 px de conception, quelle que soit la taille de l'element qu'elle remplit.

    Invisible tant que la piece n'est pas livree (`Artwork`) : la brique qui la porte dessine alors
    son aplat de repli, en se liant a `delivered`.
*/
Item {
    id: root

    /// La cle du cahier (`ui/arena/floor`).
    property string key: ""

    /// L'entree de la table des pieces livrees, ou `undefined` tant que la piece ne l'est pas.
    readonly property var entry: Artwork.delivered[root.key]

    /// Vrai si la piece est livree et posee ; la brique affiche son repli sinon.
    readonly property bool delivered: root.entry !== undefined

    visible: root.delivered
    clip: true

    Image {
        width: root.width / Tokens.uiScale
        height: root.height / Tokens.uiScale
        scale: Tokens.uiScale
        transformOrigin: Item.TopLeft
        source: root.delivered ? Artwork.baseUrl + root.entry.file : ""
        fillMode: Image.Tile
        smooth: true
    }
}
