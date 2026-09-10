import QtQuick
import QtQuick.Layouts
import Jadg.Ui

/*!
    Options -- FORMULAIRE, cote conception (LOT-86).

    Dessine, mais PAS BRANCHE, et c'est delibere : `EX-IHM-083` exige que tout reglage expose
    atteigne reellement le moteur. Le jeu n'a pas encore de viewport ; un curseur de volume ou une
    synchronisation verticale n'y toucheraient rien. Les afficher comme actifs serait mentir a
    l'utilisateur -- une case a cocher qui ne fait rien est pire qu'une case absente.

    Ils portent donc des cles d'attribution, comme les sept ecrans du RPG, et le pied l'avoue.

    Le compteur d'images par seconde ne figure pas ici : les planches l'avaient tranche, c'est un
    element de HUD, il vit avec le HUD et non dans les ecrans.
*/
RpgScreenFrame {
    id: root

    property string vsync: "—"
    property string fullscreen: "—"
    property string diagnostics: "—"
    property string volume: "—"
    property string language: "—"
    property string gamepad: "—"

    navigationHint: qsTr("Échap") + " · " + qsTr("Retour")
    title: qsTr("Options")

    content: [
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Tokens.spaceExtraLarge

            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                spacing: Tokens.spaceLarge

                SheetBlock {
                    Layout.fillWidth: true
                    title: qsTr("Affichage")
                    SheetLine { Layout.fillWidth: true; label: qsTr("Plein écran"); value: root.fullscreen }
                    SheetLine { Layout.fillWidth: true; label: qsTr("Synchronisation verticale"); value: root.vsync }
                    SheetLine { Layout.fillWidth: true; label: qsTr("Compteur de diagnostic"); value: root.diagnostics }
                }

                SheetBlock {
                    Layout.fillWidth: true
                    title: qsTr("Volume")
                    SheetLine { Layout.fillWidth: true; label: qsTr("Volume"); value: root.volume }
                }
            }

            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                spacing: Tokens.spaceLarge

                SheetBlock {
                    Layout.fillWidth: true
                    title: qsTr("Langue")
                    SheetLine { Layout.fillWidth: true; label: qsTr("Langue"); value: root.language }
                }

                SheetBlock {
                    Layout.fillWidth: true
                    title: qsTr("Contrôles")
                    SheetLine { Layout.fillWidth: true; label: qsTr("Manette"); value: root.gamepad }
                }
            }
        }
    ]
}
