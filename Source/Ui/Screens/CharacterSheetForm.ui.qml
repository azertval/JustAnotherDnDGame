import QtQuick
import QtQuick.Layouts
import Jadg.Ui

/*!
    Fiche de personnage — FORMULAIRE, côté conception (LOT-86).

    Ce fichier appartient à l'artiste. Il s'ouvre et se modifie dans Qt Design Studio, et rien de
    ce qu'on y déplace, colore ou anime ne demande de recompiler quoi que ce soit.

    C'est un `.ui.qml` : le sous-ensemble DÉCLARATIF de QML. Aucune fonction, aucun bloc de code,
    aucun gestionnaire impératif — c'est ce que Design Studio sait relire et réenregistrer sans
    l'abîmer, et `scripts/check_ui_layers.py` le vérifie. La logique vit dans `CharacterSheet.qml`,
    son jumeau côté développeur.

    Le suffixe `Form` n'est pas décoratif : sans lui, le formulaire et son jumeau déclareraient tous
    deux un type nommé `CharacterSheet`, et le module refuserait de se charger.

    Les propriétés ci-dessous portent des VALEURS D'EXEMPLE. Elles ne sont pas décoratives : sans
    elles, Design Studio afficherait un écran vide et aucune mise en page ne pourrait s'y juger. À
    l'exécution, le jumeau les remplace par celles de la vue-modèle.
*/
RpgScreenFrame {
    id: root

    // --- Ce que l'écran reçoit ------------------------------------------------------------
    property string characterName: "Brenna Pierrefonte"
    property string species: "Naine des collines"
    property string background: "Artisane de guilde"
    property string classAndLevel: "Guerrière 3"
    property string experience: "900"

    property string hitPoints: "25 / 30"
    property string armorClass: "16"
    property string initiative: "+1"
    property string speed: "9 m"
    property string proficiencyBonus: "+2"
    property string passivePerception: "11"

    property var abilities: exampleAbilities
    property var skills: exampleSkills

    // Jeux d'exemple, uniquement pour la conception. Le jumeau les écrase au lancement.
    //
    // Des `ListModel` et non des tableaux JavaScript, et la raison mérite d'être écrite : un
    // tableau n'expose que `modelData` à son délégué, là où un modèle expose ses RÔLES. Les deux
    // formes ne se délèguent donc pas pareil, et un écran validé sur des tableaux se serait
    // affiché vide dès qu'on lui aurait branché la vraie vue-modèle -- sans la moindre erreur.
    // Le jeu d'exemple doit avoir exactement la forme des vraies données, sinon il ne prouve rien.
    readonly property ListModel exampleAbilities: ListModel {
        ListElement { rowId: "strength"; label: "Force"; value: "16 (+3)" }
        ListElement { rowId: "dexterity"; label: "Dextérité"; value: "12 (+1)" }
        ListElement { rowId: "constitution"; label: "Constitution"; value: "15 (+2)" }
        ListElement { rowId: "intelligence"; label: "Intelligence"; value: "10 (+0)" }
        ListElement { rowId: "wisdom"; label: "Sagesse"; value: "13 (+1)" }
        ListElement { rowId: "charisma"; label: "Charisme"; value: "8 (-1)" }
    }
    readonly property ListModel exampleSkills: ListModel {
        ListElement { rowId: "athletics"; label: "Athlétisme"; value: "+5 •" }
        ListElement { rowId: "stealth"; label: "Discrétion"; value: "+1" }
        ListElement { rowId: "perception"; label: "Perception"; value: "+1" }
        ListElement { rowId: "insight"; label: "Intuition"; value: "+1" }
    }

    title: root.characterName

    content: [
        ColumnLayout {
            Layout.fillWidth: true
            spacing: Tokens.spaceSmall

            Text {
                // `horizontalAlignment` sur un texte qui remplit la largeur, plutot que
                // `Layout.alignment` : cette derniere centre l'ELEMENT dans la cellule, et un
                // element large comme sa cellule est deja centre -- son texte, lui, reste a gauche.
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: root.species + " · " + root.background + " · " + root.classAndLevel
                color: Tokens.textMuted
                font.family: Tokens.bodyFamily
                font.pixelSize: Tokens.body
            }
        },
        // --- Corps : caractéristiques | constantes | compétences ---------------------------
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Tokens.spaceExtraLarge

            // Caractéristiques
            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                // `Layout.fillWidth` et NON une fraction de `parent.width` : lier la taille d'un
                // enfant à celle de son propre layout crée une boucle, que Qt interrompt au bout
                // de deux passes en signalant une « réorganisation récursive ». La mise en page
                // s'affichait quand même -- mais dans un état arbitraire.
                Layout.fillWidth: true
                spacing: Tokens.spaceSmall

                Text {
                    text: qsTr("Caractéristiques")
                    color: Tokens.accent
                    font.family: Tokens.bodyFamily
                    font.pixelSize: Tokens.body
                }
                Repeater {
                    model: root.abilities
                    // L'enveloppe EXIGE ses rôles du modèle (`required`), ce qui les rend
                    // vérifiables : un modèle qui ne les fournirait pas échoue au chargement, au
                    // lieu d'afficher des lignes vides. `SheetLine` porte déjà `label` et `value`
                    // et ne peut pas les redéclarer -- d'où l'enveloppe plutôt qu'un accès
                    // `model.<rôle>`, que `qmllint` signale à juste titre comme non qualifié.
                    delegate: Item {
                        id: abilityRow
                        required property string label
                        required property string value
                        Layout.fillWidth: true
                        implicitHeight: abilityLine.implicitHeight
                        SheetLine {
                            id: abilityLine
                            anchors.fill: parent
                            label: abilityRow.label
                            value: abilityRow.value
                        }
                    }
                }
            }

            // Constantes de combat
            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                // `Layout.fillWidth` et NON une fraction de `parent.width` : lier la taille d'un
                // enfant à celle de son propre layout crée une boucle, que Qt interrompt au bout
                // de deux passes en signalant une « réorganisation récursive ». La mise en page
                // s'affichait quand même -- mais dans un état arbitraire.
                Layout.fillWidth: true
                spacing: Tokens.spaceSmall

                Text {
                    text: qsTr("En jeu")
                    color: Tokens.accent
                    font.family: Tokens.bodyFamily
                    font.pixelSize: Tokens.body
                }
                SheetLine { Layout.fillWidth: true; label: qsTr("Points de vie"); value: root.hitPoints }
                SheetLine { Layout.fillWidth: true; label: qsTr("Classe d'armure"); value: root.armorClass }
                SheetLine { Layout.fillWidth: true; label: qsTr("Initiative"); value: root.initiative }
                SheetLine { Layout.fillWidth: true; label: qsTr("Vitesse"); value: root.speed }
                SheetLine { Layout.fillWidth: true; label: qsTr("Maîtrise"); value: root.proficiencyBonus }
                SheetLine { Layout.fillWidth: true; label: qsTr("Perception passive"); value: root.passivePerception }
                SheetLine { Layout.fillWidth: true; label: qsTr("Expérience"); value: root.experience }
            }

            // Compétences
            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                spacing: Tokens.spaceSmall

                Text {
                    text: qsTr("Compétences")
                    color: Tokens.accent
                    font.family: Tokens.bodyFamily
                    font.pixelSize: Tokens.body
                }
                // Une `ListView` et non un `Repeater` : les dix-huit compétences ne tiennent pas
                // dans la hauteur d'une fenêtre 720p, et un `Repeater` les aurait laissées déborder
                // hors du cadre -- rognées, sans que rien ne le signale. C'est exactement le défaut
                // qui s'était produit trois fois du côté des widgets.
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: Tokens.spaceSmall
                    model: root.skills
                    boundsBehavior: Flickable.StopAtBounds
                    delegate: Item {
                        id: skillRow
                        required property string label
                        required property string value
                        width: ListView.view.width
                        implicitHeight: skillLine.implicitHeight
                        SheetLine {
                            id: skillLine
                            anchors.fill: parent
                            label: skillRow.label
                            value: skillRow.value
                        }
                    }
                }
            }
        }
    ]
}
