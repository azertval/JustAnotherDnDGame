# Entrées et actions logiques {#guide-entrees}

Cette page explique comment une touche physique devient un mouvement du personnage, et pourquoi ce
trajet passe par une étape intermédiaire qui, au premier abord, peut sembler superflue.

## Le principe : ne jamais coder « en dur » une touche dans le gameplay

Le gameplay ne dépend **jamais** directement d'une touche physique (`EX-CTRL-010`). Autrement dit,
`core::CharacterPhysicsSystem` ne contient **aucun** test du genre « si la touche flèche gauche est
enfoncée ». À la place, les entrées brutes (clavier, souris) sont d'abord traduites en une
**intention** neutre — « aller à gauche », « sauter » — que `Core` consomme sans jamais savoir
**quelle touche** a produit cette intention.

**Pourquoi cette indirection.** Sans elle, chaque système qui a besoin d'une entrée devrait
connaître les touches physiques, ce qui pose plusieurs problèmes concrets :

- **remappage impossible sans toucher la logique de jeu** — si un joueur veut réassigner « sauter »
  d'`Espace` à une autre touche (`EX-CTRL-012`, concrétisé par `LOT-29`, voir plus bas), ou si le
  jeu ajoute le support d'une manette, il faudrait modifier chaque système qui teste directement les
  touches, au lieu d'un seul point de traduction ;
- **tests plus difficiles** — tester la physique du saut obligerait à simuler de vrais événements
  clavier de la plateforme plutôt qu'à construire directement une intention `{ jumpPressed: true }` ;
- **couplage entre `Core` et `HMI`** — `Core` n'a, par ailleurs, **aucune** dépendance à Win32
  (`EX-ARCH-010`, @ref guide-boucle) ; lui faire connaître des codes de touche briserait cette
  frontière architecturale.

Le trajet complet est donc : **touche physique → `hmi::InputState` (état brut) → `hmi::toPlayerInput` (traduction) → `core::PlayerInput` (intention) → `Core` (logique de jeu)**. Les deux
premières étapes vivent dans `HMI` (dépendantes de la plateforme), la dernière dans `Core`
(indépendante).

## Échantillonner plutôt que réagir : \ref hmi::InputState "hmi::InputState"

Deux façons classiques d'observer les entrées existent :

- **piloté par les événements** : le code réagit immédiatement à chaque message du système
  d'exploitation (« touche enfoncée », « touche relâchée ») dès qu'il arrive, à un instant
  arbitraire ;
- **échantillonné** (*polling*) : le code lit un **état courant**, mis à jour une fois par frame, à
  un instant **prévisible** de la boucle de jeu.

Ce moteur choisit le second : `hmi::InputState` capture l'état clavier/souris **une fois par frame**
(`EX-CTRL-021`), pas à chaque événement Qt individuel. **Pourquoi** : la logique de jeu tourne au
pas de temps fixe (@ref guide-boucle) — elle a besoin d'un état d'entrée **stable** pendant toute la
durée d'un pas, pas d'un flux d'événements arrivant de façon désynchronisée par rapport à ce
rythme. Échantillonner une fois par frame donne cette photographie stable.

### Détecter les fronts, pas seulement l'état

Savoir qu'une touche est enfoncée ne suffit pas : la physique a besoin de distinguer *plusieurs*
questions liées mais différentes, par exemple pour le saut (@ref guide-physique §3) :

- « le bouton **vient-il d'être pressé** cette frame ? » (le **front montant**) — déclenche le saut
  et alimente le *jump buffering* ;
- « le bouton est-il **maintenu** ? » — sert à la hauteur de saut variable (relâcher tôt écourte le
  saut) ;
- « le bouton **vient-il d'être relâché** ? » (le **front descendant**) — c'est ce relâchement,
  détecté précisément à la frame où il se produit, qui déclenche la coupe de vitesse ascendante.

`InputState` calcule ces trois fronts en gardant **deux instantanés** : l'état de la frame
**courante** et celui de la frame **précédente**. Une touche est « pressée » (front montant)
précisément quand elle est enfoncée maintenant mais ne l'était **pas** à la frame d'avant ; de même,
« relâchée » (front descendant) quand elle ne l'est plus mais l'était juste avant. Sans conserver
cet historique d'une frame, on ne pourrait connaître que l'état courant (`keyDown`), jamais le
moment précis de transition (`keyPressed`/`keyReleased`) — pourtant essentiel au *game feel* du
saut et du dash (des actions **ponctuelles**, à déclencher une seule fois par appui, pas à chaque
frame où la touche reste enfoncée).

```cpp
// keyDown : vraie si enfoncée maintenant, quelle que soit la source (clavier OU manette).
bool InputState::keyDown(Key key) const noexcept {
    return _keysCurrent[index] || _gamepadCurrent[index];
}

// keyPressed (front montant) : enfoncée maintenant, mais ne l'était sur AUCUNE source
// à la frame précédente.
bool InputState::keyPressed(Key key) const noexcept {
    const bool keyboardEdge = _keysCurrent[index] && !_keysPrevious[index];
    const bool gamepadEdge = _gamepadCurrent[index] && !_gamepadPrevious[index];
    return keyboardEdge || gamepadEdge;
}
```

Remarque sur la fusion manette (détaillée plus bas) : `keyPressed` calcule un front **par source**
puis les combine par OU logique — un bouton manette pressé alors que la touche clavier équivalente
était déjà maintenue produit bien un nouveau front (celui de la manette), sans que le clavier
« masque » cette pression. C'est cette même logique de combinaison en lecture, jamais en écriture,
qui protège de tout effacement accidentel d'une touche (voir « La manette » ci-dessous).

### Le cycle d'une frame

1. le viewport Qt (`hmi::GameViewport`) traduit les événements clavier/souris **Qt**
   (`keyPressEvent`/`keyReleaseEvent`/`mouseMoveEvent`/…) en appels `onKeyDown`/`onKeyUp`/
   `onMouseMove`/… sur l'`InputState`, qui met à jour l'état **courant** — l'échantillonnage a lieu
   **une fois par frame réelle** (`EX-CTRL-021`) ;
2. pour **chaque pas de simulation** exécuté cette frame, la logique lit les fronts et l'état courant
   (`keyPressed`, `keyDown`, `keyReleased`, et leurs équivalents souris), puis la boucle appelle
   `hmi::InputState::beginFrame()` — qui recopie l'état **courant** vers l'état **précédent**, ouvrant
   la fenêtre d'observation du pas suivant.

**Pourquoi avancer les fronts par pas et non par frame.** Le rendu est découplé de la simulation
(@ref guide-boucle) : sur un écran à 120/144 Hz, une partie des frames réelles n'exécute **aucun**
pas fixe. Si l'état précédent était recopié à chaque frame de rendu (comme c'était le cas avant
`LOT-33`), un appui capturé sur une de ces frames serait effacé **avant** qu'un pas ne l'ait lu — le
front `keyPressed` disparaîtrait sans jamais déclencher l'action. En liant l'avancée de la ligne de
base au **pas consommé**, un appui survit jusqu'à sa lecture, quel que soit le framerate : la latence
entrée → action reste bornée à **un pas** (`EX-CTRL-020`) à 60 comme à 240 Hz. Le viewport
échantillonne toujours l'état une fois par frame ; seule l'avancée des fronts change de cadence.

**Perte de focus.** À un `Alt+Tab` (ou tout basculement de fenêtre), le viewport ne reçoit **pas**
d'événement de relâchement pour les touches maintenues. Sans précaution, une direction maintenue
resterait « collée » et le personnage avancerait seul au retour. Le viewport traite donc
`QEvent::FocusOut` en appelant `InputState::releaseAll()`, qui remet à zéro l'état courant **et**
précédent de toutes les touches/boutons — sans produire de front « relâchée » parasite
(courant == précédent == relâché).

`InputState` est délibérément **indépendant de toute fenêtre** (aucune dépendance `<Windows.h>` ni
Qt dans son en-tête) : les événements peuvent être injectés directement en test, sans ouvrir de
fenêtre, ce qui le rend testable en isolation (`EX-NFR-010`).

### Le pont Qt → \ref hmi::Key "Key" : \ref hmi::qtKeyToHmiKey "qtKeyToHmiKey"

`hmi::Key` est une énumération dont les valeurs coïncident avec les codes virtuels Windows
(`VK_*`) — un héritage pratique : pour les lettres, chiffres et l'espace, `Qt::Key_A == 'A' == 0x41`,
si bien qu'aucune conversion n'est nécessaire. Seules les touches **spéciales** (flèches, `Échap`,
`Tab`, `Maj`, `Ctrl`, `F1`/`F2`/`F10`) diffèrent entre l'énumération Qt et les codes VK : `hmi::qtKeyToHmiKey`
(`Source/HMI/Input/QtKeyMap`) les convertit par une petite table, le reste passant directement. Le
viewport y appelle cette fonction dans `keyPressEvent`/`keyReleaseEvent` avant de mettre à jour
l'`InputState`.

## Traduire l'état en intention : \ref hmi::toPlayerInput "hmi::toPlayerInput"

`hmi::toPlayerInput(input, gameKeyBindings, gamepadBindings)` est la fonction qui **seule** connaît
la correspondance entre touches/boutons et intentions ; elle produit un `core::PlayerInput` — le
**contrat** de données entre `HMI` et `Core` :

- `moveX` ∈ `[-1, 1]` : `-1` = gauche, `+1` = droite, `0` = immobile. Appuyer sur les deux touches
  simultanément **neutralise** l'intention (`moveX == 0`) plutôt que de privilégier arbitrairement
  l'une ou l'autre ;
- `moveY` : intention de **visée verticale**, utilisée uniquement pour orienter le **dash** — le
  déplacement au sol reste purement horizontal, ce champ ne sert qu'à choisir une direction de dash
  parmi les 8 possibles (@ref guide-physique §4) ;
- `jumpPressed` (front) / `jumpHeld` (maintenu) ;
- `dashPressed` (front, `EX-CTRL-013`), une action dédiée et distincte du saut.

`Core` ne reçoit **que** cette structure : il ignore totalement l'existence des touches/boutons
physiques.

**Deux sources indépendantes par action.** `hmi::GameAction` (Gauche/Droite/Sauter/Dash/Viser
haut/Viser bas) est l'action logique commune ; `gameKeyBindings` (`hmi::GameKeyBindings`) l'associe
à une touche clavier, `gamepadBindings` (`hmi::GamepadBindings`) à un bouton manette — chacune
remappable **indépendamment** de l'autre depuis Options (Touches de jeu / Touches de la manette),
persistées dans le même `Settings/keybindings.json` (sections distinctes). Pour chaque action,
`toPlayerInput` vérifie simplement `input.keyDown(gameKeyBindings.key(action)) ||
input.gamepadButtonDown(gamepadBindings.button(action))` : l'une des deux sources suffit, remapper
l'une n'affecte jamais l'autre — deux touches/boutons ne sont d'ailleurs jamais partagés par deux
actions du **même** ensemble (`GameKeyBindings`/`GamepadBindings::setKey` échangent plutôt que de
dupliquer une touche), ce qui évite par construction qu'une touche ne déclenche deux actions à la
fois.

`toPlayerInput` est une fonction **pure** (aucun état interne, aucun effet de bord) : lui passer les
mêmes `InputState`/bindings produit toujours le même `PlayerInput`, ce qui la rend testable sans
fenêtre ni minuteur, en construisant directement un `InputState` avec les touches/boutons voulus.
Elle est appelée **une fois par frame**, en amont de toute logique de jeu (`EX-CTRL-020`,
`EX-CTRL-021`) — jamais à l'intérieur du pas fixe lui-même, pour que tous les pas exécutés dans une
même frame (@ref guide-boucle) voient exactement la même intention.

## La manette : une seconde source, fusionnée en lecture (EX-CTRL-002, LOT-20)

`InputState` ne connaît qu'un seul `Key` par touche, mais **deux** sources indépendantes qui
peuvent l'enfoncer : le clavier (`onKeyDown`/`onKeyUp`) et la manette (`onGamepadKeyDown`/
`onGamepadKeyUp`), chacune avec sa propre paire courant/précédent. `keyDown`/`keyPressed`/
`keyReleased` **combinent** les deux (OU logique) au moment de la lecture — jamais à l'écriture.

**Pourquoi pas une seule table partagée ?** Parce que la manette est **sondée**, pas événementielle
: `hmi::GamepadPoller::poll` interroge XInput une fois par frame et doit explicitement relâcher
(`onGamepadKeyUp`) chaque touche dont le bouton correspondant n'est plus enfoncé — y compris quand
la manette est débranchée. Si ce relâchement écrivait dans la **même** table que le clavier, il
effacerait une touche clavier réellement maintenue dès que la manette (absente ou relâchée) ne la
tient plus. Deux tables, combinées seulement en lecture, rendent ce bug structurellement
impossible plutôt que de compter sur la discipline du code appelant.

Chaque bouton/direction manette synthétise le **même** `Key` fixe que son équivalent clavier par
défaut (D-pad/stick gauche → `Left`/`Right`/`Up`/`Down` ; **A** → `Enter` **et** `Space` ; **B**/
**Start** → `Escape` ; épaule droite → `Shift`, le dash) — câblage en dur dans
`hmi::GamepadPoller::poll`, jamais remappé. Conséquence directe de la fusion : les raccourcis fixes
lus dans le viewport (essai de jeu, éditeur) n'ont jamais eu besoin d'être modifiés pour « apprendre »
la manette — ils lisent des `Key` fixes, non remappables (comme `Échap`/`Entrée` côté clavier), ce
qui satisfait déjà `EX-CTRL-010` par construction.

**Une seconde piste, brute et indépendante, pour les actions de jeu remappables** (`LOT-30`) :
`hmi::GamepadPoller::poll` alimente aussi, à partir du **même** relevé XInput, un état brut par
`hmi::GamepadButton` (`onGamepadButtonDown`/`onGamepadButtonUp`, `InputState::gamepadButtonDown`/
`gamepadButtonPressed`) — dix boutons/directions (D-pad et stick gauche fusionnés en une seule
notion logique par direction, comme la piste `Key`). C'est cette piste, et non la fusion `Key`
ci-dessus, que consulte `toPlayerInput` via `gamepadBindings` : la navigation de menu reste sur des
`Key` fixes, la manette côté gameplay devient pleinement configurable sans toucher à la première
piste.

Le sondage XInput lui-même (`<Xinput.h>`) vit dans `hmi::GamepadPoller` (`Input`), jamais dans
`InputState` : `InputState` reste indépendant de toute fenêtre (`EX-NFR-010`), y compris pour
tester la fusion manette ou la piste brute — les tests appellent `onGamepadKeyDown`/
`onGamepadKeyUp`/`onGamepadButtonDown`/`onGamepadButtonUp` directement, sans manette réelle ni
`<Windows.h>`.

**Sondage throttlé quand aucune manette n'est branchée** (`LOT-33`). `XInputGetState` est
notablement **coûteux** quand le slot interrogé est vide : le pilote énumère les périphériques à
chaque appel. Le sonder à chaque frame alors qu'aucune manette n'est connectée provoquait des
micro-saccades chez un joueur clavier. `hmi::GamepadPoller::poll` ne re-sonde donc un slot resté
**déconnecté** qu'une frame sur `GAMEPAD_DISCONNECTED_POLL_INTERVAL` (≈ 2 s) ; entre-temps l'état
reste « déconnecté » (aucune touche synthétique enfoncée). Dès qu'une manette est présente, le
sondage redevient systématique — un branchement à chaud est détecté au plus tard après un intervalle.

## Le menu d'options

> **Refondu au `LOT-86`.** La page Options du jeu est passee en QML
> (`Source/Ui/Screens/Options*`) ; la page Qt a onglets decrite ci-dessous n'existe plus. Ce que ses
> onglets regroupaient -- V-Sync, langue, remappage -- reste le contenu attendu.

Le menu d'options (accessible depuis le menu principal, @ref guide-ecrans) est, depuis le `LOT-38`,
une **page à onglets** : la
navigation, le défilement et la souris sont pris en charge nativement par Qt — plus aucun modèle de
liste « maison ». Ses onglets regroupent la bascule **V-Sync** (`EX-REN-022`), le choix de **langue**
(une liste déroulante `QComboBox`) et le **remappage** des touches de jeu, des touches d'éditeur et
des boutons de manette (sous-sections suivantes). L'état de connexion de la manette y est affiché à
titre informatif (`InputState::gamepadConnected()`).

## Remapper les touches et boutons : \ref hmi::GameKeyBindings "GameKeyBindings"/\ref hmi::EditorKeyBindings "EditorKeyBindings" (LOT-29), \ref hmi::GamepadBindings "GamepadBindings" (LOT-30)

Les onglets « Touches de jeu »/« Touches de l'éditeur »/« Touches de la manette » de la page Options
sont des widgets Qt (`hmi::KeybindingsWidget` pour le clavier, `hmi::GamepadBindingsWidget` pour la
manette). Chaque ligne affiche une action et sa touche ou son bouton **actuel**
(`hmi::keyDisplayName`/`hmi::gamepadButtonDisplayName`) ; déclencher une ligne entre en **capture** :
la frame suivante qui voit une touche ou un bouton assignable pressé (`hmi::capturedKey`, qui scrute
les 256 codes suivis par `InputState` ; `hmi::capturedGamepadButton`, qui scrute les dix
`GamepadButton` — `Échap`/`Entrée` et `B`/`Start` exclus, réservés à la navigation) la lie via
`setKey` (`(Game|Editor)KeyBindings` ou `GamepadBindings`), qui **échange** avec toute autre action
déjà sur cette touche/ce bouton plutôt que de rejeter le conflit — jamais deux actions du même
ensemble sur la même touche ou le même bouton. Annuler la capture laisse la liaison inchangée. Le
remappage manette exige en plus une manette connectée (`InputState::gamepadConnected()`) pour entrer
en capture.

`GameKeyBindings`/`EditorKeyBindings`/`GamepadBindings` sont trois classes **séparées** (pas de
généricité commune) : même mécanique (`key`/`button`, `setKey`, `resetToDefaults`, `load`/`save`),
mais des cas concrets distincts, pas de base commune anticipée au-delà. Elles partagent un **seul**
fichier de persistance (`Settings/keybindings.json`, sections `"jeu"`/`"editeur"`/`"manette"`) —
chaque `save` relit le fichier existant pour préserver les sections des deux autres classes plutôt
que de les écraser. `GameKeyBindings`/`EditorKeyBindings` stockent le code VK **brut** (`Key` en est
déjà un, voir plus haut) ; `GamepadBindings`, elle, stocke un **nom symbolique** (`"gauche"`, `"a"`,
`"rb"`, …) — `GamepadButton` n'a pas d'équivalent numérique naturel comme le VK du clavier, et un
nom reste lisible/stable si l'ordre de l'enum change. Un fichier absent, corrompu, ou une entrée
invalide (bouton inconnu compris) retombe sur les valeurs par défaut pour l'entrée concernée
(`EX-NFR-040`), jamais bloquant.

Le remappage éditeur ne couvre qu'un **sous-ensemble significatif** (Sauvegarder/Annuler/Refaire/
Copier/Coller/Test rapide/Grille…), pas l'intégralité des raccourcis de l'éditeur — certains gestes
(redimensionnement, `Tab`) restent câblés en dur (décision de cadrage `LOT-29`, portée volontairement
limitée).

## La langue de l'interface : \ref hmi::Localization "hmi::Localization"

Tous les textes d'interface (menus, options) passent par une **clé** stable (`EX-REN-033`) plutôt
que par un libellé en dur : `hmi::Localization` résout une clé (« menu.jouer ») vers la chaîne de
la **langue active**, chargée depuis un fichier `<langue>.lang` (format `clé = valeur`, un fichier
par langue dans `Source/Elements/Localization/`). La résolution suit un **repli déterministe** —
langue active, puis langue par défaut, puis la clé elle-même — pour ne jamais planter ni afficher
un vide si une traduction manque (`EX-NFR-040`), au prix, dans ce dernier cas, d'un texte
visiblement « brut » (la clé) plutôt qu'un texte manquant silencieux.

Le **sélecteur de langue** (français ↔ anglais, `EX-REN-033`) vit dans l'onglet **Général** de la
page Options : une `QComboBox`. Le changer **recharge le catalogue à chaud**
(`Localization::loadLanguage`) et **retraduit toute l'IHM Qt** — `hmi::MainWindow` détient l'unique
`hmi::Localization` et propage un `retranslateUi()` à chaque widget (menu, options, docks, palette,
navigateur, remappage) ; les libellés de la palette de tuiles sont résolus par une table
libellé→clé. La langue choisie est **persistée** (`QSettings`) et restaurée au lancement. Un échec de
chargement (fichier absent) est **récupérable** : la langue courante est simplement conservée plutôt
que de planter.

## Voir aussi
- `hmi::InputState`, `hmi::Key`, `hmi::qtKeyToHmiKey`, `hmi::GamepadButton`, `hmi::toPlayerInput`, `core::PlayerInput`.
- `hmi::GamepadPoller` — le sondage XInput, partagé par le viewport.
- `Source/Ui/Screens/MainMenu*`, `Options*` — le menu principal et la page Options, en QML.
- `hmi::GameKeyBindings`, `hmi::EditorKeyBindings`, `hmi::keyDisplayName`, `hmi::capturedKey`,
  `hmi::KeybindingsWidget` — remappage des touches (`LOT-29`, IHM Qt `LOT-38`).
- `hmi::GamepadBindings`, `hmi::gamepadButtonDisplayName`, `hmi::capturedGamepadButton`,
  `hmi::GamepadBindingsWidget` — remappage manette (`LOT-30`, IHM Qt `LOT-38`).
- `hmi::Localization`.
- @ref guide-physique — comment la physique consomme `PlayerInput` (saut, dash, mouvement).
- @ref guide-boucle — pourquoi l'entrée est échantillonnée une fois par frame et non par pas fixe.
- @ref guide-ihm-qt — les widgets Qt (menu, options, remappage) qui pilotent tout ceci.
