import QtQuick
import Jadg.Ui

/*!
    Une piece « couvrante » du cahier des assets : un fond de scene (LOT-87, T3.1).

    Le troisieme porteur d'image, a cote de `NinePatchArt` et `FixedArt`, pour les pieces que le
    cahier declare `display: cover` (`ui/background/menu-scene`). Elle remplit l'element sans se
    deformer et rogne ce qui deborde : un paysage etire se lit tout de suite, un paysage rogne sur
    ses bords ne se remarque pas.

    Invisible tant que la piece n'est pas livree (`Artwork`) : l'ecran pose alors son propre fond de
    repli dessous, en se liant a `delivered`.
*/
Item {
    id: root

    /// La cle du cahier (`ui/background/menu-scene`).
    property string key: ""

    /// L'entree de la table des pieces livrees, ou `undefined` tant que la piece ne l'est pas.
    readonly property var entry: Artwork.delivered[root.key]

    /// Vrai si la piece est livree et posee ; l'ecran affiche son repli sinon.
    readonly property bool delivered: root.entry !== undefined

    visible: root.delivered

    Image {
        anchors.fill: parent
        source: root.delivered ? Artwork.baseUrl + root.entry.file : ""
        fillMode: Image.PreserveAspectCrop
        // Peinte : elle se redimensionne en interpole (EX-ARCH-022).
        smooth: true
        mipmap: true
    }
}
