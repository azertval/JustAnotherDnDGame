# TACHE-01 — Action logique « Interagir » {#lot-63-tache-01-action-interagir}

**Lot :** [LOT-63](epic.md) · **Emplacement :** `Source/Core/Physics`, `Source/HMI/Input` ·
**Statut :** fait

## Contexte
Le tableau des contrôles de `Documentation/Specification/controles.md` liste sept actions. Six sont
implémentées et remappables. La septième — « Interagir (⚠️ souhaité, non implémenté) », **E** au
clavier et **X** à la manette — n'existe pas, et n'a même pas d'identifiant d'exigence : c'est la
seule ligne du tableau sans ancre.

Sa conséquence est concrète : un interrupteur ne s'actionne que par **contact**. `EX-GP-020`
l'autorise (« contact **ou** action dédiée »), mais tout mécanisme demandant une **intention** — et
la clé de la `TACHE-02` en est un — reste hors de portée.

## Travail à réaliser
- **Ajouter l'action** à `core::PlayerInput` (`Source/Core/Physics/PlayerInput.h`), à côté des
  actions existantes : c'est une **intention**, donc elle appartient à `Core`, comme le saut et le
  dash.
- **Mapper l'action** dans `hmi::PlayerInputMapper`, avec ses fronts (pressée / maintenue /
  relâchée), conformément à `EX-CTRL-011`.
- **Rendre l'action remappable** : `hmi::GameKeyBindings` (clavier, `LOT-29`) et
  `hmi::GamepadBindings` (manette, `LOT-30`), avec **E** et **X** par défaut. Elle doit apparaître
  dans les écrans de remappage sans traitement particulier — c'est le signe que le patron est
  respecté.
- **Nom traduisible** de l'action dans les deux catalogues, pour l'affichage dans les écrans de
  remappage.
- **Ne rien changer au comportement existant** : aucun mécanisme actuel ne consomme cette action à
  l'issue de cette tâche. C'est la `TACHE-02` qui lui donnera son premier usage.

## Fichiers impactés
- `Source/Core/Physics/PlayerInput.h`.
- `Source/HMI/Input/PlayerInputMapper.{h,cpp}`, `GameKeyBindings.{h,cpp}`, `GamepadBindings.{h,cpp}`.
- `Source/HMI/Interface/KeybindingsWidget.cpp`, `GamepadBindingsWidget.cpp` — si la liste des
  actions y est énumérée plutôt que dérivée.
- `Source/Elements/Localization/{fr,en}.lang`.
- `Source/Test/Unit/HMI/Input/test_player_input_mapper.cpp`, `test_game_key_bindings.cpp`,
  `test_gamepad_bindings.cpp` (étendus).

## Tests (obligatoires)
- L'action distingue **pressée**, **maintenue** et **relâchée** dans un pas, comme les autres
  (`EX-CTRL-011`).
- Le remappage clavier et le remappage manette fonctionnent **indépendamment** l'un de l'autre —
  c'est la garantie déjà donnée par `EX-CTRL-012`, et déjà testée pour les autres actions.
- Les valeurs par défaut sont **E** et **X**, conformes au tableau des contrôles.
- Un fichier de remappage écrit **avant** ce lot (sans l'action) se relit sans erreur, l'action
  prenant sa valeur par défaut — c'est le cas de compatibilité qui casse en silence.
- Le nom de l'action est présent dans les **deux** catalogues de traduction.

## Points d'attention
- **La compatibilité du fichier de remappage existant est le vrai risque.** Un utilisateur ayant
  déjà `Settings/keybindings.json` ne doit pas perdre ses réglages ni voir l'application refuser le
  fichier. La lecture est déjà tolérante ; le vérifier explicitement par un test.
- **Ne pas modifier le comportement par contact** des interrupteurs : des niveaux livrés et le test
  système en dépendent.
- Grossir `core::PlayerInput` ou `hmi::InputState` change la disposition d'une classe très incluse :
  une build incrémentale peut se comporter étrangement. Reconstruire avec `--clean-first` avant de
  chercher un bogue.
- Vérifier qu'aucune touche par défaut n'entre en conflit avec un raccourci d'éditeur.

## Définition de fait (DoD)
- L'action « Interagir » existe dans `Core`, est mappée avec ses fronts, est remappable clavier et
  manette avec **E**/**X** par défaut, apparaît dans les écrans de remappage, est traduite, ne
  change aucun comportement existant, et un fichier de réglages antérieur reste lisible ;
  `/W4 /WX` propre.

## Exigences
`EX-CTRL-022` (action logique « Interagir ») ; réutilise `EX-CTRL-010` (actions logiques),
`EX-CTRL-011` (fronts), `EX-CTRL-012` (remappage), `EX-GP-020` (activation par contact ou action
dédiée), `EX-REN-033` (traduction), `EX-NFR-040` (lecture tolérante).
