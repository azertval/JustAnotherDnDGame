import QtQuick
import Jadg.Ui

/*!
    Une piece 9-patch du cahier des assets, posee a l'echelle de la fenetre (LOT-87, T2.7).

    Invisible tant que la piece n'est pas livree (`Artwork`) : la brique qui la porte dessine alors
    son aplat de repli, en se liant a `delivered`.

    **Pourquoi le `scale`.** Les marges d'un `BorderImage` sont en pixels de l'IMAGE, et les coins
    se dessinent a cette taille quelle que soit celle de l'element. Les pieces sont produites a
    1080p : a 720p, des coins de 112 px mangeraient un panneau qui n'en fait plus que les deux
    tiers. L'image est donc posee a la taille de CONCEPTION (l'element divise par `uiScale`), puis
    reduite d'un bloc -- coins, bords et centre ensemble, dans les proportions de la maquette.
*/
Item {
    id: root

    /// La cle du cahier, variante comprise (`ui/button/apply/hover`).
    property string key: ""

    /// Repetition des bords, telle que le cahier la declare pour la piece (`tileMode`).
    property int horizontalTileMode: BorderImage.Repeat
    property int verticalTileMode: BorderImage.Stretch

    /// L'entree de la table des pieces livrees, ou `undefined` tant que la piece ne l'est pas.
    readonly property var entry: Artwork.delivered[root.key]

    /// Vrai si la piece est livree et posee ; la brique affiche son repli sinon.
    readonly property bool delivered: root.entry !== undefined

    visible: root.delivered

    BorderImage {
        id: art

        width: root.width / Tokens.uiScale
        height: root.height / Tokens.uiScale
        scale: Tokens.uiScale
        transformOrigin: Item.TopLeft
        source: root.delivered ? Artwork.baseUrl + root.entry.file : ""
        border.left: root.delivered && root.entry.margins ? root.entry.margins.left : 0
        border.top: root.delivered && root.entry.margins ? root.entry.margins.top : 0
        border.right: root.delivered && root.entry.margins ? root.entry.margins.right : 0
        border.bottom: root.delivered && root.entry.margins ? root.entry.margins.bottom : 0
        horizontalTileMode: root.horizontalTileMode
        verticalTileMode: root.verticalTileMode
        smooth: true
    }
}
