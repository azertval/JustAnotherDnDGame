# LOT-29 — Remappage des touches (jeu + éditeur) {#lot-29}

> Statut : **✅ terminé**. Deux nouveaux sous-menus dans **Options** : « Touches de jeu » et
> « Touches de l'éditeur », permettant de remapper au clavier un sous-ensemble d'actions
> significatives de chaque catégorie (`EX-CTRL-012`), avec persistance dans un fichier de
> configuration à côté de l'exécutable.

## Objectif
`EX-CTRL-012` (souhaitée, [`controles.md`](../../Specification/controles.md)) demande un mapping
actions-logiques ↔ touches **reconfigurable**, au minimum via un fichier de configuration.
Aujourd'hui, il n'existe **aucune** couche d'indirection : `hmi::Key` (codes virtuels Win32) est lu
en dur partout où une touche compte — `HMI/Input/PlayerInputMapper.cpp` (jeu) et
`HMI/Interface/EditorScreen.cpp` (éditeur). Le commentaire de `PlayerInputMapper.h` anticipait déjà
ce lot : « un futur remappage… ne toucherait que cette fonction ». Aucun réglage ne persiste non
plus entre deux lancements (V-Sync et langue sont perdus au redémarrage, constaté en cours de
cadrage) : ce lot introduit la **première** persistance de réglages du projet.

## Périmètre

### Inclus
- **Modèle de bindings** (`GameKeyBindings`, `EditorKeyBindings`, `HMI/Input`) : une touche par
  action, échange automatique en cas de conflit (jamais deux actions du même ensemble sur la même
  touche), réinitialisation aux valeurs par défaut.
- **Persistance JSON** : un seul fichier `Settings/keybindings.json` à côté de l'exécutable (même
  dossier que `Levels/`/`Localization/`, via `hmi::executableDirectory()`), écrit avec
  `nlohmann::json` (déjà une dépendance du projet, `Core/Levels/LevelWriter.cpp`). Fichier
  absent/corrompu/clé inconnue → valeurs par défaut pour l'entrée concernée, avertissement
  journalisé, jamais bloquant (même esprit qu'`EX-NFR-040`).
- **Six actions de jeu remappables**, miroir de `PlayerInputMapper.cpp` : Aller à gauche, Aller à
  droite, Sauter, Dash, Viser le dash vers le haut, Viser le dash vers le bas.
- **Neuf actions éditeur remappables**, sous-ensemble significatif (pas exhaustif) de
  `EditorScreen.cpp` : Sauvegarder, Annuler, Refaire, Copier, Coller, Test rapide, Grille, Aide,
  Renommer.
- **Deux nouveaux écrans** de remappage (patron `OptionsScreen`/`OptionsModel`) : liste
  d'actions avec la touche courante, capture d'une nouvelle touche à la confirmation,
  réinitialisation, retour aux Options. `OptionsModel`/`OptionsScreen` étendus de deux entrées
  ouvrant ces écrans.
- **Panneau d'aide de l'éditeur** (`F1`) dynamisé pour refléter les touches réellement liées, au
  lieu des libellés câblés en dur actuels (deviendraient faux après un remap).

### Exclus (hors périmètre de ce lot)
- **Remappage manette** — hors périmètre, seul le clavier est concerné ici. Discuté avec le
  demandeur en cours de lot : `Window::pollGamepad` câble aujourd'hui chaque bouton XInput utile en
  dur sur une touche fixe (A→Espace/Entrée, D-pad/stick→flèches, RB→Maj, B/Start→Échap) — les
  boutons utiles sont déjà tous occupés, sans capacité de configuration. Un vrai remappage manette
  nécessiterait de retirer ce câblage en dur : portée bien plus large, différée à un lot dédié
  ultérieur (`LOT-30`). Ce lot-ci corrige seulement la régression que le remappage clavier
  introduirait sans filet (voir décision ci-dessous) — pas un remappage manette en soi.
- **Remappage exhaustif de l'éditeur** — restent câblés en dur, décision de cadrage assumée :
  navigation de menu (Haut/Bas/Entrée/Échap, y compris dans ce nouveau menu lui-même),
  redimensionnement par flèches, `Ctrl+R` (saisie de taille), `"0"` (reset caméra), `Tab` (cycle
  d'outil), `Maj`+clic (liaison de mécanisme). Un chevauchement entre une action remappée et un
  raccourci resté fixe reste possible (ex. remapper « Sauvegarder » sur `←`, qui redimensionne
  aussi la grille) — accepté comme limite de portée.
- **Le modificateur `Ctrl`** des actions éditeur Sauvegarder/Annuler/Refaire/Copier/Coller reste
  câblé en dur ; seule la touche-lettre associée se remappe.
- **`Échap` et `Entrée`** ne sont jamais assignables (conventions globales de navigation/retour —
  les assigner casserait la navigation elle-même, y compris celle du menu de remappage).
- **Remappage souris** — seules les touches clavier sont concernées.

## Décisions de cadrage
- **Filet de sécurité manette dans `PlayerInputMapper`** (écart constaté après revue du demandeur,
  qui a explicitement demandé de ne pas oublier la manette) : chaque action de jeu vérifie sa
  touche liée et, si elle reste libre, sa touche par défaut (`GameKeyBindings::defaultKey`) — la
  manette (`Window::pollGamepad`) n'écrivant que dans les touches par défaut, câblées en dur, sans
  connaître les bindings courants, ce filet la garde fonctionnelle quel que soit le remap clavier.
  Une première version de ce filet, combinée à un alias fixe alors envisagé pour Gauche/Droite/
  Sauter (`Q`/`D`/`W`), a provoqué une régression réelle en usage (mouvement neutralisé après
  remap) ; corrigée par `GameKeyBindings::isKeyClaimedByOtherAction` (le filet ne revérifie une
  touche par défaut que si aucune autre action ne se l'est appropriée) et le retrait de l'alias.
  Détail complet, cause racine et tests de régression : voir TACHE-02.
- **Portée « actions clés seulement »**, pas un remappage exhaustif — confirmée avec le demandeur
  (`AskUserQuestion`) : couvre les actions de gameplay et un sous-ensemble éditeur significatif,
  pas les raccourcis souris/outils ni la navigation de menu.
- **Persistance JSON à côté de l'exécutable** — confirmée avec le demandeur, plutôt qu'un
  remappage en mémoire seule (qui contredirait l'esprit d'`EX-CTRL-012`). Valeur stockée = code VK
  brut (`Key` **est** déjà un code VK, cf. `InputState.h`) : pas de table de noms symboliques à
  maintenir en plus pour la persistance (seul l'affichage à l'écran a besoin d'un nom lisible).
- **Deux classes séparées** (`GameKeyBindings`/`EditorKeyBindings`) plutôt qu'une abstraction
  générique commune : deux cas concrets connus, aucun troisième anticipé — dupliquer une
  mécanique simple (~50 lignes) reste plus lisible qu'un template pour ce projet.
- **Résolution de conflit par échange** plutôt que par rejet : si la touche capturée est déjà liée
  à une autre action du même ensemble (jeu ou éditeur), les deux actions échangent leurs touches —
  jamais d'état invalide/dupliqué à afficher, jamais de message d'erreur à concevoir.

## Exigences couvertes
- `EX-CTRL-012` — lève son statut « souhaité » (mapping reconfigurable via fichier de
  configuration).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-modele-bindings.md) | Modèle de bindings et persistance JSON | `HMI/Input` | ✅ |
| [TACHE-02](tache-02-integration-jeu-editeur.md) | Intégration jeu/éditeur | `HMI/Input`, `HMI/Interface` | ✅ |
| [TACHE-03](tache-03-ui-remappage.md) | UI de remappage et câblage | `HMI/Interface`, `HMI/main.cpp` | ✅ |
| [TACHE-04](tache-04-documentation-verification.md) | Documentation et vérification | `Documentation` | ✅ |

## Critères d'acceptation du lot
1. Depuis Options, deux nouvelles entrées ouvrent « Touches de jeu » et « Touches de l'éditeur »,
   chacune listant ses actions avec la touche actuellement liée.
2. Sélectionner une action puis appuyer sur une touche la lie à cette action ; si la touche était
   déjà utilisée par une autre action du même sous-menu, les deux échangent leurs touches (jamais
   de doublon affiché).
3. Le remappage est **effectif immédiatement** en jeu/dans l'éditeur (y compris l'essai immédiat
   depuis l'éditeur) et **persiste** après fermeture/relance de l'application
   (`Settings/keybindings.json`).
4. Une entrée « Réinitialiser » restaure les valeurs par défaut de tout le sous-menu.
5. `Échap`/`Entrée` restent toujours utilisables pour naviguer, y compris dans ces deux nouveaux
   écrans (jamais assignables à une action).
6. **Aucune régression** sur le jeu ou l'éditeur avec les valeurs par défaut (comportement
   identique à avant ce lot tant qu'aucun remappage n'a eu lieu).
7. Logique nouvelle **couverte par des tests** (bindings, échange, persistance JSON, capture).
   Build `/W4 /WX` sans avertissement, Doxygen et lint des exigences verts.

## Dépendances
- Étend `PlayerInputMapper` (existant depuis les premiers lots de jeu) et `EditorScreen`
  (`LOT-14`/`LOT-15`).
- Réutilise le patron `OptionsModel`/`OptionsScreen` (`LOT-20`).

## Navigation des tâches
- @subpage lot-29-tache-01-modele-bindings
- @subpage lot-29-tache-02-integration-jeu-editeur
- @subpage lot-29-tache-03-ui-remappage
- @subpage lot-29-tache-04-documentation-verification
