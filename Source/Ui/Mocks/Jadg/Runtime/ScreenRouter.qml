pragma Singleton
import QtQuick

/*!
    Doublure de `hmi::ScreenRouter` pour Qt Design Studio (LOT-87).

    Les deux énumérations et les propriétés du type C++ (`Source/HMI/Runtime/ScreenRouter.h`), et
    des méthodes de navigation qui ne font rien : dans l'atelier, un écran ne mène nulle part.
*/
QtObject {
    enum Screen {
        Menu,
        Game,
        Options,
        Pause,
        Credits,
        RpgScreen,
        Arena
    }

    enum RpgScreen {
        CharacterSheet,
        Skills,
        Inventory,
        QuestJournal,
        WorldMap,
        Dialogue,
        Merchant,
        Company,
        CombatHud
    }

    readonly property int currentScreen: ScreenRouter.Menu
    readonly property int currentRpgScreen: ScreenRouter.CharacterSheet
    readonly property bool developerBuild: true

    signal changed()

    function openMenu() {}
    function openGame() {}
    function openOptions() {}
    function closeOptions() {}
    function openPause() {}
    function resume() {}
    function quitToMenu() {}
    function openCredits() {}
    function closeCredits() {}
    function openArena() {}
    function closeArena() {}
    function openRpgScreen(screen) {}
    function closeRpgScreen() {}
    function nextRpgScreen() {}
    function previousRpgScreen() {}
}
