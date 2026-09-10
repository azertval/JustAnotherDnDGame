import QtQuick
import QtQuick.Layouts
import Jadg.Ui

/*!
    Journal de quetes -- FORMULAIRE, cote conception (LOT-86).

    Dessine avant que la donnee existe : le lot des quetes n'est pas ecrit. La mise en page, elle,
    etait deja decidee, et la jeter en attendant reviendrait a la redessiner plus tard, autrement,
    sans que personne ne se souvienne de ce qui avait ete tranche.

    Les valeurs ci-dessous sont des EXEMPLES, pour que Qt Design Studio montre un ecran plein et
    que la mise en page se juge. Le jumeau de cablage les remplace par l'ancre `PendingData`, en
    attendant la vraie source.
*/
RpgScreenFrame {
    id: root

    property alias quests: questList.model
    property alias objectives: objectiveList.model
    property alias detail: detailProse.text

    title: qsTr("Journal de quêtes")

    // Le contenu s'affecte au slot du châssis. `content` n'est PAS la propriété par défaut,
    // et ne peut pas l'être : le châssis a ses propres enfants (le double cadre, le titre, le
    // pied), qui y entreraient aussi et s'imbriqueraient dans eux-mêmes.
    content: [
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Tokens.spaceExtraLarge

            SheetList {
                id: questList
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                title: qsTr("Quêtes")
                rows: 8
                model: ListModel {
                    ListElement { label: "Les caves de Bourg-la-Rive"; value: "En cours" }
                    ListElement { label: "La cargaison disparue"; value: "En cours" }
                    ListElement { label: "Le convoi de Val-Morne"; value: "Terminée" }
                    ListElement { label: "Ce qui rôde sous la halle"; value: "Nouvelle" }
                }
            }

            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                spacing: Tokens.spaceLarge

                SheetProse {
                    id: detailProse
                    Layout.fillWidth: true
                    title: qsTr("Détail")
                    text: "Le tavernier jure que les tonneaux descendent seuls à la cave, et qu'aucun ne remonte. Il propose trois pièces d'or et le gîte pour la nuit à qui voudra bien y descendre voir."
                }

                SheetList {
                    id: objectiveList
                    Layout.fillWidth: true
                    title: qsTr("Objectifs")
                    rows: 4
                    model: ListModel {
                        ListElement { label: "Descendre à la cave"; value: "✓" }
                        ListElement { label: "Trouver l'origine du bruit"; value: "" }
                        ListElement { label: "En parler au tavernier"; value: "" }
                    }
                }
            }
        }
    ]
}
