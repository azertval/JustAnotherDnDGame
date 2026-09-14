import QtQuick
import Jadg.Ui
import Jadg.Runtime

/*!
    Fiche de personnage — CÂBLAGE, côté développeur (LOT-86, LOT-87 T3.4).

    Ce fichier ne décrit aucune apparence : il relie le formulaire `CharacterSheetForm.ui.qml` à la
    vue-modèle C++. C'est la seule moitié que la conception n'ouvre jamais, et la seule où du code
    a le droit d'exister.

    Presque tout vient de `CharacterSheetModel`, dont la table `values` porte le score et le
    modificateur séparés de chaque caractéristique. Deux champs de la maquette n'ont pas encore de
    source et passent par `PendingData` : le **matricule**, et le **seuil du niveau suivant** qui
    remplit la jauge d'expérience (`LOT-74`). Le portrait aussi, tant qu'aucun jeton du corpus n'est
    extrait et déclaré au manifeste.
*/
CharacterSheetForm {
    id: root

    readonly property CharacterSheetModel sheet: CharacterSheetModel {}

    /// Une valeur de la fiche par sa clé, ou le tiret si la fiche ne la porte pas.
    function field(key) {
        const text = root.sheet.values[key];
        return text === undefined ? "—" : text;
    }

    characterName: sheet.name
    className: root.field("sheet.class")
    level: sheet.level
    background: sheet.background
    species: sheet.species
    registration: PendingData.value("character_sheet.registration")
    portrait: PendingData.image("character_sheet.portrait")

    hitPointsText: sheet.hitPoints
    // « 25 / 30 » : le courant se lit avant la barre, le maximum a sa propre clé.
    hitPointsRatio: {
        const current = parseInt(sheet.hitPoints, 10);
        const maximum = parseInt(sheet.hitPointsMax, 10);
        return maximum > 0 && !isNaN(current) ? Math.max(0, Math.min(1, current / maximum)) : 0;
    }

    experienceText: sheet.experience + " / " + root.nextLevelExperience
    experienceRatio: {
        const current = parseInt(sheet.experience, 10);
        const next = parseInt(root.nextLevelExperience, 10);
        return next > 0 && !isNaN(current) ? Math.max(0, Math.min(1, current / next)) : 0;
    }
    readonly property string nextLevelExperience: PendingData.value("character_sheet.experience.next_level")

    strengthScore: root.field("sheet.ability.strength.score")
    strengthModifier: root.field("sheet.ability.strength.modifier")
    dexterityScore: root.field("sheet.ability.dexterity.score")
    dexterityModifier: root.field("sheet.ability.dexterity.modifier")
    constitutionScore: root.field("sheet.ability.constitution.score")
    constitutionModifier: root.field("sheet.ability.constitution.modifier")
    intelligenceScore: root.field("sheet.ability.intelligence.score")
    intelligenceModifier: root.field("sheet.ability.intelligence.modifier")
    wisdomScore: root.field("sheet.ability.wisdom.score")
    wisdomModifier: root.field("sheet.ability.wisdom.modifier")
    charismaScore: root.field("sheet.ability.charisma.score")
    charismaModifier: root.field("sheet.ability.charisma.modifier")

    armorClass: sheet.armorClass
    initiative: sheet.initiative
    speed: sheet.speed
    proficiencyBonus: sheet.proficiencyBonus
    passivePerception: sheet.passivePerception

    skills: sheet.skills

    Component.onCompleted: sheet.loadDemonstrationCharacter()
}
