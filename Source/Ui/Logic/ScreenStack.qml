import QtQuick
import Jadg.Ui

/*!
    La pile d'écrans -- côté DÉVELOPPEUR (LOT-86).

    Elle traduit l'**état** publié par `ScreenRouter` en écran affiché. C'est le seul endroit du
    projet où cette correspondance existe, et elle vit ici plutôt qu'en C++ pour une raison
    précise : la présentation publie un état, jamais un chemin de fichier. La conception peut ainsi
    réorganiser `Screens/` sans qu'une ligne de C++ ne s'en aperçoive.

    **Des composants typés et non des chemins construits.** Assembler « ../Screens/ » + un nom
    produisait une URL que la ressource ne résolvait pas depuis un sous-répertoire — un écran
    introuvable, à l'exécution seulement. Les types, eux, sont vérifiés par `qmllint` : un écran
    renommé fait échouer le contrôle, pas le jeu.

    Chaque écran est enveloppé dans un `Component` : il n'est construit qu'une fois choisi. Les
    treize écrans ne vivent jamais tous en même temps.

    `--screen=<Nom>` court-circuite le routeur. C'est un outil de vérification, pas un chemin de
    jeu : les écrans dessinés mais pas encore alimentés ne sont atteignables par aucun autre moyen.
*/
Item {
    id: root

    /// Écran imposé au lancement, ou chaîne vide pour laisser le routeur décider.
    property string forcedScreen: ""

    Component { id: menuScreen; MainMenu {} }
    Component { id: optionsScreen; Options {} }
    Component { id: creditsScreen; Credits {} }
    Component { id: pauseScreen; Pause {} }
    Component { id: characterSheetScreen; CharacterSheet {} }
    Component { id: inventoryScreen; Inventory {} }
    Component { id: journalScreen; Journal {} }
    Component { id: worldMapScreen; WorldMap {} }
    Component { id: dialogueScreen; Dialogue {} }
    Component { id: merchantScreen; Merchant {} }
    Component { id: guildBoardScreen; GuildBoard {} }
    Component { id: combatHudScreen; CombatHud {} }
    Component { id: teamSheetScreen; TeamSheet {} }

    Loader {
        anchors.fill: parent
        focus: true
        sourceComponent: root.forcedScreen.length > 0 ? root.byName(root.forcedScreen)
                                                      : root.byState()
    }

    /// L'écran que le routeur désigne.
    function byState() {
        switch (ScreenRouter.currentScreen) {
        case ScreenRouter.Menu:      return menuScreen
        case ScreenRouter.Options:   return optionsScreen
        case ScreenRouter.Credits:   return creditsScreen
        case ScreenRouter.Pause:     return pauseScreen
        case ScreenRouter.RpgScreen: return root.rpgScreen(ScreenRouter.currentRpgScreen)
        // Le viewport de jeu n'existe pas encore : il arrive avec le portage du rendu sur Qt
        // Quick. En attendant, « Nouvelle partie » ouvre la fiche du personnage -- c'est ce que
        // le jeu sait montrer de plus vrai.
        case ScreenRouter.Game:      return characterSheetScreen
        }
        return menuScreen
    }

    function rpgScreen(screen) {
        switch (screen) {
        case ScreenRouter.CharacterSheet: return characterSheetScreen
        case ScreenRouter.Inventory:      return inventoryScreen
        case ScreenRouter.QuestJournal:   return journalScreen
        case ScreenRouter.WorldMap:       return worldMapScreen
        case ScreenRouter.Dialogue:       return dialogueScreen
        case ScreenRouter.Merchant:       return merchantScreen
        case ScreenRouter.GuildBoard:     return guildBoardScreen
        case ScreenRouter.CombatHud:      return combatHudScreen
        case ScreenRouter.TeamSheet:      return teamSheetScreen
        }
        return characterSheetScreen
    }

    /// Résolution par nom, pour `--screen=`. Une table explicite : dériver le composant d'une
    /// chaîne par convention aurait marché sur douze écrans et cassé sur le treizième.
    function byName(name) {
        switch (name) {
        case "MainMenu":       return menuScreen
        case "Options":        return optionsScreen
        case "Credits":        return creditsScreen
        case "Pause":          return pauseScreen
        case "CharacterSheet": return characterSheetScreen
        case "Inventory":      return inventoryScreen
        case "Journal":        return journalScreen
        case "WorldMap":       return worldMapScreen
        case "Dialogue":       return dialogueScreen
        case "Merchant":       return merchantScreen
        case "GuildBoard":     return guildBoardScreen
        case "CombatHud":      return combatHudScreen
        case "TeamSheet":      return teamSheetScreen
        }
        return menuScreen
    }
}
