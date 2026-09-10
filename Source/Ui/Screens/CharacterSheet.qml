import QtQuick
import Jadg.Ui

/*!
    Fiche de personnage — CÂBLAGE, côté développeur (LOT-86).

    Ce fichier ne décrit aucune apparence : il relie le formulaire `CharacterSheetForm.ui.qml` à la
    vue-modèle C++. C'est la seule moitié que la conception n'ouvre jamais, et la seule où du code
    a le droit d'exister.

    La division tient à ce que Qt Design Studio sait faire : il relit et réenregistre un `.ui.qml`
    sans l'abîmer, mais pas un fichier qui contient de la logique. Séparer n'est donc pas une
    convention de style — c'est ce qui rend l'écran réellement modifiable par un artiste.
*/
CharacterSheetForm {
    id: root

    // La vue-modèle décide de ce que le jeu SAIT DIRE ; le formulaire décide de ce que ça DONNE à
    // voir. Le pont tient en quelques lignes, et c'est le signe que la frontière est au bon
    // endroit : quand il faut trente lignes pour relier une vue-modèle à son écran, c'est que
    // l'une des deux fait le travail de l'autre.
    readonly property CharacterSheetModel sheet: CharacterSheetModel {}

    characterName: sheet.name
    species: sheet.species
    background: sheet.background
    classAndLevel: sheet.classAndLevel
    experience: sheet.experience

    hitPoints: sheet.hitPoints
    armorClass: sheet.armorClass
    initiative: sheet.initiative
    speed: sheet.speed
    proficiencyBonus: sheet.proficiencyBonus
    passivePerception: sheet.passivePerception

    abilities: sheet.abilities
    skills: sheet.skills

    Component.onCompleted: sheet.loadDemonstrationCharacter()
}
