import QtQuick
import QtQuick.Shapes
import Jadg.Ui

/*!
    Bandeau de titre : une plaque de grenat sertie d'or, entre deux ailes déployées
    (LOT-86, EX-IHM-075).

    **L'envergure suit la HAUTEUR, jamais la largeur**, et c'est tout l'enjeu du tracé : une aile
    dont l'envergure suivrait la largeur s'étirerait avec le titre et cesserait d'être une aile.
    Elle cède ensuite, et seule, si la plaque n'a plus sa part minimale — réduire la plaque à sa
    place laisserait deux ailes autour de rien.

    Les deux bords d'une aile **ne se ressemblent pas** : le bord d'attaque file haut et presque
    droit vers la pointe extérieure, le bord de fuite redescend en trois doigts profondément
    échancrés. C'est cette dissymétrie qui fait lire une aile ; deux bords échancrés donnent une
    feuille, deux bords lisses donnent une banderole.

    Porté depuis `hmi::titleBannerShapes`, relevé sur la planche.
*/
Item {
    id: root

    property string title: "Titre"

    implicitHeight: Tokens.sectionTitle + Tokens.spaceLarge

    // Envergure relevée : l'aile couvre 50 points de large pour une plaque haute de 31, soit une
    // fois et demie. En dessous, elle se lit comme une touffe accrochée au bout de la plaque.
    readonly property real wantedSpan: root.height * 1.6
    // La plaque garde au moins un tiers de la largeur totale.
    readonly property real availableSpan: (root.width * 0.66) * 0.5
    readonly property real span: Math.max(0, Math.min(root.wantedSpan, root.availableSpan))

    readonly property real chevron: root.height * 0.30
    readonly property real faceInset: root.height * 0.16
    readonly property real plateLeft: root.span
    readonly property real plateRight: root.width - root.span

    Shape {
        anchors.fill: parent
        preferredRendererType: Shape.CurveRenderer

        // Les ailes d'abord : la plaque se pose dessus, et c'est ce recouvrement qui fait lire
        // l'emplanture comme passant DERRIÈRE la plaque plutôt que collée à son bord.
        ShapePath {
            fillColor: Tokens.frameOrnament
            strokeWidth: 0
            PathPolyline {
                path: root.span >= 8 ? [
                    Qt.point(root.span, root.height * 0.10),
                    Qt.point(root.span * 0.72, root.height * 0.05),
                    Qt.point(root.span * 0.28, root.height * 0.13),
                    Qt.point(root.span * 0.02, root.height * 0.33),
                    Qt.point(root.span * 0.21, root.height * 0.47),
                    Qt.point(root.span * 0.06, root.height * 0.63),
                    Qt.point(root.span * 0.35, root.height * 0.62),
                    Qt.point(root.span * 0.23, root.height * 0.83),
                    Qt.point(root.span * 0.57, root.height * 0.76),
                    Qt.point(root.span * 0.50, root.height * 0.97),
                    Qt.point(root.span, root.height * 0.88),
                    Qt.point(root.span, root.height * 0.10)
                ] : []
            }
        }

        // L'aile droite : la gauche, retournée autour de l'axe vertical du bandeau.
        ShapePath {
            fillColor: Tokens.frameOrnament
            strokeWidth: 0
            PathPolyline {
                path: root.span >= 8 ? [
                    Qt.point(root.width - root.span, root.height * 0.10),
                    Qt.point(root.width - root.span * 0.72, root.height * 0.05),
                    Qt.point(root.width - root.span * 0.28, root.height * 0.13),
                    Qt.point(root.width - root.span * 0.02, root.height * 0.33),
                    Qt.point(root.width - root.span * 0.21, root.height * 0.47),
                    Qt.point(root.width - root.span * 0.06, root.height * 0.63),
                    Qt.point(root.width - root.span * 0.35, root.height * 0.62),
                    Qt.point(root.width - root.span * 0.23, root.height * 0.83),
                    Qt.point(root.width - root.span * 0.57, root.height * 0.76),
                    Qt.point(root.width - root.span * 0.50, root.height * 0.97),
                    Qt.point(root.width - root.span, root.height * 0.88),
                    Qt.point(root.width - root.span, root.height * 0.10)
                ] : []
            }
        }

        // La plaque : un hexagone allongé à bouts en chevron. Le fond profond d'abord.
        ShapePath {
            fillColor: Tokens.gemShadow
            strokeWidth: 0
            PathPolyline {
                path: [
                    Qt.point(root.plateLeft, root.height * 0.5),
                    Qt.point(root.plateLeft + root.chevron, 0),
                    Qt.point(root.plateRight - root.chevron, 0),
                    Qt.point(root.plateRight, root.height * 0.5),
                    Qt.point(root.plateRight - root.chevron, root.height),
                    Qt.point(root.plateLeft + root.chevron, root.height),
                    Qt.point(root.plateLeft, root.height * 0.5)
                ]
            }
        }

        // La face éclairée, en retrait.
        ShapePath {
            fillColor: Tokens.gem
            strokeWidth: 0
            PathPolyline {
                path: [
                    Qt.point(root.plateLeft + root.faceInset, root.height * 0.5),
                    Qt.point(root.plateLeft + root.chevron, root.faceInset),
                    Qt.point(root.plateRight - root.chevron, root.faceInset),
                    Qt.point(root.plateRight - root.faceInset, root.height * 0.5),
                    Qt.point(root.plateRight - root.chevron, root.height - root.faceInset),
                    Qt.point(root.plateLeft + root.chevron, root.height - root.faceInset),
                    Qt.point(root.plateLeft + root.faceInset, root.height * 0.5)
                ]
            }
        }

        // Le filet d'or, en dernier : il borde la plaque ET recouvre l'emplanture des ailes, ce qui
        // ferme la silhouette. TRACÉ, pas rempli — un contour rempli serait la plaque elle-même.
        ShapePath {
            fillColor: "transparent"
            strokeColor: Tokens.frameOrnament
            strokeWidth: Tokens.frameThickness
            PathPolyline {
                path: [
                    Qt.point(root.plateLeft, root.height * 0.5),
                    Qt.point(root.plateLeft + root.chevron, 0),
                    Qt.point(root.plateRight - root.chevron, 0),
                    Qt.point(root.plateRight, root.height * 0.5),
                    Qt.point(root.plateRight - root.chevron, root.height),
                    Qt.point(root.plateLeft + root.chevron, root.height),
                    Qt.point(root.plateLeft, root.height * 0.5)
                ]
            }
        }
    }

    Text {
        anchors.centerIn: parent
        text: root.title
        color: Tokens.surfaceAlt
        font.family: Tokens.titleFamily
        font.pixelSize: Tokens.sectionTitle
    }
}
