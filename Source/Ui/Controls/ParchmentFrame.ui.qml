import QtQuick
import Jadg.Ui

/*!
    Encadrement de parchemin (LOT-86, EX-IHM-075).

    Trois unités d'épaisseur : un **trait** d'encre au bord, une **réserve** de parchemin, un
    **filet** doré. C'est la réserve du milieu qui fait l'encadrement — sans elle, trait et filet se
    touchent et l'ensemble se lit comme une bordure épaisse de deux tons, pas comme un cadre.

    Les coins ne sont **pas entaillés** : ils portent un cabochon doré posé par-dessus le trait.
    C'est ce détail qui distingue un encadrement d'une simple bordure à deux traits — et c'est
    exactement ce que le pixel art faisait à l'envers, en *retirant* de la matière aux angles au
    lieu d'en ajouter.

    Porté depuis `hmi::parchmentFrameStrokes`, relevé sur le corpus.
*/
Item {
    id: root

    /// Épaisseur d'une unité de maquette, en pixels. Bornée pour que les deux bords opposés ne se
    /// chevauchent jamais : au-delà, l'encadrement sortirait du cadre au lieu de s'y réduire.
    readonly property int unit: Math.max(0, Math.min(Tokens.scale,
                                Math.floor(Math.min(root.width, root.height) / 6)))

    /// Côté du carré d'angle : trois unités, soit exactement l'épaisseur de l'encadrement.
    readonly property int cornerSquare: 3 * root.unit

    /*!
        Côté de la pierre. Elle DÉBORDE du carré d'angle — un cabochon exactement à la taille du
        carré disparaît : il fait alors l'épaisseur de l'encadrement, et son octogone se confond
        avec le filet qui le borde. Sur les feuilles du corpus, la pierre mesure plus du double du
        filet : elle est *posée sur* l'angle, pas *encastrée dedans*. Deux fois, donc, et pas
        trois — au-delà elle mordrait sur le champ.

        Elle reste **ancrée au coin** et jamais centrée dessus : centrée, elle sortirait du panneau
        d'une demi-largeur et se ferait rogner — une pierre coupée en deux à chaque angle, sans
        qu'aucune erreur ne le dise.
    */
    readonly property int cornerSize: Math.min(2 * root.cornerSquare,
                                               Math.floor(Math.min(root.width, root.height) / 2))

    // Le champ d'abord : tout le reste se pose dessus. Le peindre INTÉGRALEMENT, plutôt que le seul
    // intérieur de l'encadrement, évite qu'un arrondi d'épaisseur laisse une bande de pixels non
    // peints — que Qt rendrait par ce qui se trouve derrière, jamais par du parchemin.
    Rectangle {
        anchors.fill: parent
        color: Tokens.surface
    }

    // Trop petit pour un encadrement honnête : un champ nu, pas un cadre écrasé.
    Item {
        anchors.fill: parent
        visible: root.unit > 0

        // Le trait d'encre, au bord.
        Rectangle {
            anchors.fill: parent
            color: "transparent"
            border.width: root.unit
            border.color: Tokens.frameEdge
        }

        // Le filet doré, en retrait de deux unités : une pour le trait, une pour la réserve.
        Rectangle {
            anchors.fill: parent
            anchors.margins: 2 * root.unit
            color: "transparent"
            border.width: root.unit
            border.color: Tokens.frameOrnament
        }

        // L'ombre se pose JUSTE À L'INTÉRIEUR du filet, jamais entre le trait et lui : c'est le
        // filet qui est en relief sur le champ, et une ombre placée en amont comblerait la réserve
        // de parchemin — la seule chose qui fasse lire les deux traits comme un encadrement.
        Rectangle {
            anchors.fill: parent
            anchors.margins: 3 * root.unit
            color: "transparent"
            border.width: root.unit
            border.color: Tokens.frameShadow
            visible: Math.min(root.width, root.height) >= 8 * root.unit
        }

        // Cabochons d'angle, posés en DERNIER : ils recouvrent le trait et la réserve aux quatre
        // angles, ce qui donne l'encadrement doré d'une feuille de personnage.
        Cabochon {
            size: root.cornerSize
            anchors.left: parent.left
            anchors.top: parent.top
        }
        Cabochon {
            size: root.cornerSize
            anchors.right: parent.right
            anchors.top: parent.top
        }
        Cabochon {
            size: root.cornerSize
            anchors.left: parent.left
            anchors.bottom: parent.bottom
        }
        Cabochon {
            size: root.cornerSize
            anchors.right: parent.right
            anchors.bottom: parent.bottom
        }
    }
}
