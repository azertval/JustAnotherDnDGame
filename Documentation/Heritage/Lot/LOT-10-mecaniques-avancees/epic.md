# LOT-10 — Mécaniques aériennes avancées : double saut, wall jump, dash {#lot-10}

> Statut : **terminé**. Ce lot enrichit le personnage du LOT-09 de trois mécaniques de platformer
> avancé : **double saut**, **wall jump** (+ wall slide) et **dash** 8 directions. Il **étend le
> MVP** et **assouplit** `EX-GP-013` (pas de double saut) via les nouvelles exigences
> `EX-GP-015`/`EX-GP-016`/`EX-GP-017` et l'action `EX-CTRL-013`.

## Objectif
Ajouter au `CharacterPhysicsSystem` (LOT-08/09) trois mécaniques aériennes, en gardant la
simulation **pure et déterministe au pas fixe** dans `Core` (`EX-NFR-002`, `EX-ARCH-011`), testée
sans GPU :

- **Double saut** (`EX-GP-015`) : un nombre paramétrable de sauts **aériens** supplémentaires,
  **rechargés au contact du sol**.
- **Wall jump + wall slide** (`EX-GP-016`) : contre un mur en l'air, le personnage **glisse** (chute
  ralentie) ; sauter le propulse **en diagonale opposée** au mur.
- **Dash** (`EX-GP-017`) : une **ruée** à vitesse élevée dans l'une des **8 directions** (données par
  les touches directionnelles, à défaut l'orientation), sur une **courte durée**, **disponible une
  fois** puis **rechargée au contact du sol**.

Le lot se conclut par un **niveau de démonstration « parkour »** exigeant ces mécaniques et des
tests (unitaires, intégration, système) prouvant qu'elles fonctionnent et que le niveau est
franchissable.

## Périmètre

### Inclus
- **Spec** : nouvelles exigences `EX-GP-015`/`016`/`017` et action `EX-CTRL-013` (déjà ajoutées au
  cadrage).
- **Données (Core)** : `PlayerInput` enrichi (intention de **dash** + direction de visée verticale,
  orientation) ; `Player` enrichi (sauts aériens restants, état de contact mural, disponibilité et
  minuterie de dash, orientation) ; `PhysicsConfig` enrichi (nombre de sauts aériens, vitesses de
  wall jump / wall slide, vitesse et durée de dash).
- **Mapping (HMI)** : touche **Maj** → intention de dash ; direction de visée depuis les touches
  directionnelles (haut/bas/gauche/droite), orientation courante mémorisée (`EX-CTRL-013`).
- **Double saut (Core)** : compteur de sauts aériens, consommé en l'air, rechargé au sol.
- **Wall jump + wall slide (Core)** : détection du contact mural (via la normale du balayage),
  glisse ralentie, impulsion diagonale opposée au mur.
- **Dash (Core)** : burst directionnel (8 directions) à vitesse élevée sur une durée, gravité
  suspendue pendant le dash, recharge au sol.
- **Contenu + preuve** : niveau « parkour » (murs à remonter, gouffre à franchir au dash) ; tests
  de franchissabilité et parcours système.

### Exclus (lots ultérieurs)
- **Gravité asymétrique**, **coyote/buffer** appliqués au wall jump (raffinements de *feel*).
- **Dash directionnel infini / rechargeable en l'air**, **grab de rebord (ledge grab)**, **wall
  climb**.
- **Niveau puzzle** (mécanismes interrupteur/porte, `EX-GP-020`…) et **animations** (`EX-REN-012`).

## Décisions de cadrage
- **Dash à 8 directions**, **consommé** à l'usage et **rechargé au contact du sol** (une ruée par
  phase aérienne). Direction depuis les touches directionnelles ; à défaut, l'**orientation**
  courante (dernier sens horizontal). Gravité **suspendue** pendant le dash (trajectoire nette).
- **Double saut** : **1** saut aérien supplémentaire par défaut (total 2), nombre **paramétrable**
  dans `PhysicsConfig`. Réutilise le buffering du LOT-09.
- **Wall slide inclus** : sans lui le wall jump est injouable — contre un mur en chute, la vitesse de
  descente est plafonnée à une valeur douce.
- **Touche de dash : Maj** (`VK_SHIFT`). L'action reste **dissociée** de la touche (`EX-CTRL-013`).
- **`EX-GP-013` assoupli** : le saut n'est plus « au sol uniquement » — il est **contextuel** (sol,
  coyote, saut aérien restant, ou mur). Documenté comme extension au-delà du MVP.
- **État porté par les données** : compteurs/minuteries/orientation vivent dans `Player`, décomptés
  par le système au pas fixe → déterminisme (`EX-NFR-002`).

## Exigences couvertes
- `EX-GP-015` (double/multi saut), `EX-GP-016` (wall jump + slide), `EX-GP-017` (dash 8 directions).
- `EX-CTRL-013` (action dash), `EX-CTRL-010`/`EX-CTRL-011` (action logique, fronts).
- `EX-GP-011` (saut, réutilisé), `EX-NFR-002` (pas fixe déterministe), `EX-NFR-010`/`EX-NFR-020`
  (testabilité, tests), `EX-ARCH-011` (composants = données pures).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-donnees.md) | Données des mécaniques (`PlayerInput`, `Player`, `PhysicsConfig`) | `Core` | ✅ |
| [TACHE-02](tache-02-mapping-dash.md) | Mapping du dash + direction de visée / orientation | `HMI/Input` | ✅ |
| [TACHE-03](tache-03-double-saut.md) | Double saut (sauts aériens rechargés au sol) | `Core/Ecs/Systems` | ✅ |
| [TACHE-04](tache-04-wall-jump.md) | Wall jump + wall slide | `Core/Ecs/Systems` | ✅ |
| [TACHE-05](tache-05-dash.md) | Dash 8 directions (burst, durée, recharge au sol) | `Core/Ecs/Systems` | ✅ |
| [TACHE-06](tache-06-niveau-parkour.md) | Niveau de démo « parkour » + preuve système | `Elements/Levels` | ✅ |

## Critères d'acceptation du lot
1. **Double saut** : un second saut est possible **en l'air**, puis plus, jusqu'au retour au sol.
2. **Wall jump** : contre un mur en l'air, le personnage glisse ; un saut le propulse **à l'opposé**
   du mur (composantes horizontale et verticale).
3. **Dash** : une ruée rapide dans la direction visée (8 directions) sur une courte durée,
   **une seule fois** avant de retoucher le sol.
4. Un niveau de démo **exige** ces mécaniques (mur à remonter, gouffre à franchir au dash) ; des
   tests prouvent sa franchissabilité, et le test **système** enchaîne la séquence.
5. Logique **couverte par des tests** (`ctest` vert), déterministe ; build `/W4 /WX`, Doxygen et
   `CHANGELOG.md` à jour, `lint` des exigences vert.

## Dépendances
- Prolonge le LOT-09 : réutilise le saut, la hauteur variable, le buffering, `grounded`, la normale
  de contact du balayage (`sweepAabb`) pour le mur, et l'orientation issue de `moveX`.
- L'`InputState` (LOT-06) fournit la touche Maj et les directions ; le `GameScreen`/`LevelSequence`
  (LOT-09) accueillent le niveau parkour.

## Navigation des tâches
- @subpage lot-10-tache-01-donnees
- @subpage lot-10-tache-02-mapping-dash
- @subpage lot-10-tache-03-double-saut
- @subpage lot-10-tache-04-wall-jump
- @subpage lot-10-tache-05-dash
- @subpage lot-10-tache-06-niveau-parkour
