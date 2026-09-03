# LOT-30 — Remappage manette (jeu) {#lot-30}

> Statut : **✅ terminé**. Nouveau sous-menu Options → « Touches de la manette », symétrique de
> « Touches de jeu » (`LOT-29`) : le joueur choisit quel bouton XInput déclenche chaque action de
> jeu, au lieu du câblage en dur actuel.

## Objectif
`LOT-29` a livré le remappage clavier (jeu et éditeur), avec un filet de sécurité pour que la
manette continue de fonctionner malgré un remap clavier : chaque action de jeu revérifie sa touche
par défaut, en plus de la touche remappée, tant qu'aucune autre action ne se l'est appropriée. Ce
filet reste un palliatif — la manette elle-même n'a toujours **aucune** couche de configuration :
`Window::pollGamepad` câble chaque bouton XInput utile sur une touche fixe (D-pad/stick gauche →
flèches, bouton A → Espace/Entrée, B/Start → Échap, épaule droite → Maj) et c'est ce câblage, pas
un choix du joueur, qui décide quel bouton déclenche quelle action.

Discuté avec le demandeur en cours de `LOT-29` (`AskUserQuestion`) : plutôt que de se contenter du
filet de sécurité, un **vrai** remappage manette — différé à ce lot dédié, la manette n'ayant
aucune capacité de bouton libre ni de couche de configuration à l'époque.

## Périmètre

### Inclus
- Les **six mêmes actions de jeu** que `GameKeyBindings` (Aller à gauche, Aller à droite, Sauter,
  Dash, Viser le dash vers le haut, Viser le dash vers le bas) — aucune nouvelle action, seule la
  source manette devient configurable.
- `GamepadButton` (nouveau, `HMI/Input`) : dix valeurs — `Up`/`Down`/`Left`/`Right` (D-pad et stick
  gauche fusionnés en une seule notion logique par direction, comme le fait déjà
  `Window::pollGamepad` aujourd'hui), `A`/`B`/`X`/`Y`, `LeftShoulder`/`RightShoulder`.
- `GamepadBindings` (nouveau, `HMI/Input`) : une des dix touches par action, échange automatique en
  cas de conflit, réinitialisation, persistance dans la section `"manette"` du même
  `Settings/keybindings.json` que `GameKeyBindings`/`EditorKeyBindings`.
- Piste d'état manette **brute**, indépendante de la fusion clavier/manette existante sur `Key` :
  `InputState` expose l'état de chaque `GamepadButton`, alimenté par `Window::pollGamepad` depuis
  le relevé XInput déjà lu (aucun sondage supplémentaire).
- `PlayerInputMapper` gagne une source manette véritablement configurable : chaque action vérifie
  sa touche clavier liée **ou** son bouton manette lié. Le filet de sécurité de `LOT-29` devient
  inutile et est retiré.
- Sous-menu Options → « Touches de la manette » (même patron que « Touches de jeu ») : capture
  d'un bouton manette pressé, invite dédiée si aucune manette n'est connectée.

### Exclus (hors périmètre de ce lot)
- Gâchettes analogiques (LT/RT) traitées comme boutons numériques, clics de stick — non
  nécessaires aux six actions existantes.
- Remappage manette pour l'éditeur — aucune action d'éditeur n'est aujourd'hui pilotée par la
  manette (hors navigation de menu, non remappable, comme au clavier).
- Manette secondaire (joueur 2) — XInput joueur 0 uniquement, comme aujourd'hui.
- `Start`/`Back` restent câblés en dur (Échap), conventions globales non remappables — même
  principe que `Échap`/`Entrée` côté clavier (`LOT-29`).

## Décisions de cadrage
- **Piste `GamepadButton` séparée de la fusion `Key` existante**, plutôt que de réutiliser le
  mécanisme `onGamepadKeyDown`/`onGamepadKeyUp` : ce dernier reste nécessaire tel quel pour la
  navigation de menu (`MenuModel`/`OptionsModel`/`LevelPicker`), qui n'est pas remappable ; y
  ajouter une indirection configurable aurait mélangé deux besoins différents (navigation fixe vs
  actions de jeu remappables) dans un seul mécanisme.
- **Retrait du filet de sécurité de `LOT-29`** (`PlayerInputMapper::safeDefaultDown/Pressed`,
  `GameKeyBindings::isKeyClaimedByOtherAction`) : une fois la manette correctement configurable par
  elle-même via `GamepadBindings`, elle n'a plus besoin d'emprunter la touche par défaut du
  clavier — le palliatif est remplacé par la vraie fonctionnalité qu'il annonçait.
- **`GamepadBindings` séparée de `GameKeyBindings`**, même mécanique dupliquée plutôt qu'une
  abstraction commune — cohérent avec la décision déjà prise pour `GameKeyBindings`/
  `EditorKeyBindings` en `LOT-29` (deux à trois cas concrets connus, pas d'abstraction générique).
- **Directions fusionnées D-pad/stick**, pas deux `GamepadButton` séparés par direction : le joueur
  ne perçoit pas la différence entre les deux, et `Window::pollGamepad` les traite déjà comme
  équivalentes aujourd'hui.

## Exigences couvertes
- `EX-CTRL-002` — la manette reste fonctionnelle, désormais avec un mapping choisi par le joueur
  plutôt qu'un câblage fixe.
- `EX-CTRL-012` — étend la reconfigurabilité du mapping à la manette.

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-modele-bindings-manette.md) | Modèle de bindings manette | `HMI/Input` | ✅ |
| [TACHE-02](tache-02-integration-jeu.md) | Intégration jeu | `HMI/Input`, `HMI/Platform`, `HMI/Interface` | ✅ |
| [TACHE-03](tache-03-ui-remappage-manette.md) | UI de remappage et câblage | `HMI/Interface`, `HMI/main.cpp` | ✅ |
| [TACHE-04](tache-04-documentation-verification.md) | Documentation et vérification | `Documentation` | ✅ |

## Critères d'acceptation du lot
1. Depuis Options, une nouvelle entrée « Touches de la manette » ouvre un sous-menu listant les
   six actions de jeu avec le bouton manette actuellement lié.
2. Sélectionner une action puis appuyer sur un bouton manette le lie à cette action ; en cas de
   conflit avec une autre action, les deux échangent leurs boutons.
3. Le remappage manette est effectif immédiatement en jeu (y compris l'essai immédiat depuis
   l'éditeur) et persiste après fermeture/relance de l'application.
4. Sans manette connectée, le sous-menu affiche une invite dédiée plutôt que d'attendre
   indéfiniment une capture impossible.
5. **Aucune régression** sur le jeu avec les valeurs par défaut (comportement manette identique à
   avant ce lot tant qu'aucun remappage n'a eu lieu) ni sur le remappage clavier de `LOT-29`.
6. Logique nouvelle couverte par des tests. Build `/W4 /WX` sans avertissement, Doxygen (vérifié
   avec le binaire 1.9.8 de la CI avant tout push) et lint des exigences verts.

## Dépendances
- Dépend de l'infrastructure de `LOT-29` (`GameKeyBindings`, persistance `keybindings.json`,
  patron `GameKeybindingsModel`/`Screen`) — branché sur sa branche, pas encore mergée.
- Étend `Window::pollGamepad`/`InputState` (`LOT-20`) et `PlayerInputMapper`.

## Navigation des tâches
- @subpage lot-30-tache-01-modele-bindings-manette
- @subpage lot-30-tache-02-integration-jeu
- @subpage lot-30-tache-03-ui-remappage-manette
- @subpage lot-30-tache-04-documentation-verification
