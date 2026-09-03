# TACHE-01 — Entrées bas niveau : molette et texte tapé {#lot-15-tache-01-entrees-molette-texte}

**Lot :** [LOT-15](epic.md) · **Emplacement :** `HMI/Input`, `HMI/Platform` · **Statut :** à faire

## Contexte
Le zoom caméra (TACHE-04) et la saisie de nom de niveau (TACHE-02) ont besoin de deux entrées
qu'`InputState` ne capture pas aujourd'hui : le delta de la molette et les caractères tapés au
clavier (`WM_CHAR`, distinct des codes virtuels `WM_KEYDOWN` déjà gérés). Cette tâche pose ces
fondations avant les tâches qui les consomment.

## Travail à réaliser
- **Molette** : `InputState` gagne un accumulateur de delta molette, remis à zéro à chaque
  `beginFrame()` (même cycle que les fronts pressée/relâchée) et une méthode
  `wheelDelta() const`. `Window::handleMessage` capture `WM_MOUSEWHEEL` (`GET_WHEEL_DELTA_WPARAM`)
  et l'accumule via une nouvelle méthode `InputState::onMouseWheel(int delta)`.
- **Texte tapé** : `InputState` gagne une file de caractères Unicode tapés dans la frame, vidée à
  chaque `beginFrame()`, exposée en lecture via `typedCharacters() const`. `Window::handleMessage`
  capture `WM_CHAR` et pousse le caractère via une nouvelle méthode `InputState::onCharTyped(wchar_t
  character)`.
- `InputState` reste **indépendant de toute fenêtre** (aucune dépendance `<Windows.h>`, même
  principe que l'existant) : les événements s'injectent directement, ce qui le garde testable en
  isolation (`EX-NFR-010`).
- **Nouveaux énumérateurs `Key`** nécessaires aux tâches suivantes de ce lot (aucun n'existe
  aujourd'hui — seuls `A, D, P, Q, S, W, Y, Z` et les touches spéciales sont nommés) :
  `F1` (`0x70`, aide des raccourcis, TACHE-06), `F2` (`0x71`, renommer, TACHE-03), `D0` (`0x30`,
  réinitialiser la caméra, TACHE-04), `C` (`0x43`) et `V` (`0x56`) (copier/coller, TACHE-05).
  Purement additif, à l'identique du principe déjà documenté sur `Key` (« ajouter une touche
  revient à ajouter un énumérateur ») : les codes `VK_*` correspondants sont déjà transmis tels
  quels par `Window`/`InputState`, aucune autre modification requise. Le **changement d'outil**
  (TACHE-05) réutilise `Key::Tab`, déjà déclaré et disponible dans l'éditeur.

## Fichiers impactés
- `Source/HMI/Input/InputState.h`/`.cpp`.
- `Source/HMI/Platform/Window.cpp` (deux nouveaux `case` dans `handleMessage`).
- `Source/Test/Unit/HMI/Input/test_input_state.cpp` (ou équivalent existant).

## Tests (obligatoires)
- `wheelDelta()` reflète la somme des deltas injectés depuis le dernier `beginFrame()`, et revient
  à zéro après.
- `typedCharacters()` reflète, dans l'ordre, les caractères injectés depuis le dernier
  `beginFrame()`, et est vide après.
- Aucune régression sur les fronts clavier/souris existants (tests déjà présents).

## Points d'attention
- Ne pas confondre `WM_CHAR` (caractère déjà traduit selon la disposition clavier active,
  nécessaire pour un champ de saisie multilingue) et `WM_KEYDOWN` (code virtuel physique, déjà
  utilisé pour les raccourcis) : les deux coexistent, aucun des deux ne remplace l'autre.
- `TranslateMessage` (déjà appelé dans `pumpMessages`) est le prérequis Win32 à la génération de
  `WM_CHAR` à partir de `WM_KEYDOWN` — vérifier qu'il est bien en amont dans la boucle de messages
  (déjà le cas).

## Définition de fait (DoD)
- Molette et texte tapé disponibles via `InputState`, testés (`ctest` vert), sans dépendance
  fenêtre ; build `/W4 /WX` ; Doxygen à jour.

## Exigences
`EX-NFR-010` (support des tâches suivantes, aucune exigence fonctionnelle propre).

## Ajustement post-livraison (essai utilisateur)
Un énumérateur `F10` (`0x79`) a été ajouté après coup, pour la bascule de la grille de repère
(TACHE-06, elle-même ajoutée après un essai utilisateur — voir sa fiche). Contrairement aux autres
touches nommées ici, `F10` a exigé plus qu'un énumérateur : un essai interactif a montré qu'elle ne
déclenchait jamais rien, car Win32 délivre `F10` (et les combinaisons `Alt`+quelque-chose) via
`WM_SYSKEYDOWN`/`WM_SYSKEYUP`, pas `WM_KEYDOWN`/`WM_KEYUP` (convention historique d'activation du
menu). `Window::handleMessage` capture désormais aussi ces deux messages, enregistrés dans
`InputState` comme les autres touches, tout en laissant `DefWindowProcW` traiter le comportement
système par défaut (`Alt+F4`, `Alt+Tab`) — sauf pour `F10` lui-même, absorbé pour éviter
l'activation visuelle, inutile ici, du (non-)système de menu de la fenêtre.
