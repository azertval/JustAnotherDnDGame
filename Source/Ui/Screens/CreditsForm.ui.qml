import QtQuick
import QtQuick.Layouts
import Jadg.Ui

/*!
    Credits -- FORMULAIRE, cote conception (LOT-86).

    Les attributions ne sont pas decoratives : les assets et les bibliotheques employes portent
    des licences qui EXIGENT d'etre citees. Ce sont donc des donnees, ecrites ici et nulle part
    ailleurs, et non un texte qu'on retouche a vue.
*/
RpgScreenFrame {
    id: root

    navigationHint: qsTr("Échap") + " · " + qsTr("Retour")
    title: qsTr("Crédits")

    content: [
        ColumnLayout {
            Layout.fillWidth: true
            spacing: Tokens.spaceLarge

            SheetProse {
                Layout.fillWidth: true
                title: qsTr("Développement")
                text: "Valentin Eloy"
            }
            SheetProse {
                Layout.fillWidth: true
                title: qsTr("Musique et bruitages")
                text: "Kenney (www.kenney.nl) — Digital Audio, Impact Sounds, RPG Audio, Interface Sounds — licence CC0 1.0 Universal (domaine public)"
            }
            SheetProse {
                Layout.fillWidth: true
                title: qsTr("Graphismes")
                text: "Kenney (www.kenney.nl) — Platformer Pack Remastered, Platformer Pack: Industrial — licence CC0 1.0 Universal (domaine public)"
            }
            SheetProse {
                Layout.fillWidth: true
                title: qsTr("Polices")
                text: "Inter, Pixelify Sans, Press Start 2P — SIL Open Font License 1.1"
            }
            SheetProse {
                Layout.fillWidth: true
                title: qsTr("Bibliothèques")
                text: "Qt 6 (www.qt.io) — licence LGPL v3, lien dynamique"
            }
            SheetProse {
                Layout.fillWidth: true
                title: qsTr("Licence")
                text: "Just Another D&D Game — GNU General Public License v3.0 ou ultérieure. Ressources et bibliothèques sous leurs licences propres, ci-dessus."
            }
        }
    ]
}
