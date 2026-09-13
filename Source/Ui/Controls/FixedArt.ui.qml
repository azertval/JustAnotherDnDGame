import QtQuick
import Jadg.Ui

/*!
    Une piece de taille fixe du cahier des assets : medaillon, case, ornement (LOT-87, T2.7).

    Jamais etiree hors de ses proportions : elle remplit l'element en gardant son rapport, et se
    reduit avec lissage -- le cahier la fait produire au double de sa plus petite taille d'affichage
    precisement pour cela. Invisible tant que la piece n'est pas livree (`Artwork`) : la brique
    dessine alors son aplat de repli, en se liant a `delivered`.
*/
Item {
    id: root

    /// La cle du cahier, variante comprise (`ui/slot/item/selected`).
    property string key: ""

    /// L'entree de la table des pieces livrees, ou `undefined` tant que la piece ne l'est pas.
    readonly property var entry: Artwork.delivered[root.key]

    /// Vrai si la piece est livree et posee ; la brique affiche son repli sinon.
    readonly property bool delivered: root.entry !== undefined

    visible: root.delivered

    Image {
        id: art

        anchors.fill: parent
        source: root.delivered ? Artwork.baseUrl + root.entry.file : ""
        fillMode: Image.PreserveAspectFit
        smooth: true
        mipmap: true
    }
}
