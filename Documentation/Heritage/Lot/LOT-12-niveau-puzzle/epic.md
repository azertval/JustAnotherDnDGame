# LOT-12 — Niveau puzzle : mécanismes interrupteur/porte + budget de mouvements {#lot-12}

> Statut : **terminé**. Ce lot rend les **mécanismes** interrupteur↔porte **fonctionnels**
> (`EX-GP-020`, `EX-GP-021`, chargés depuis le LOT-07 mais inertes jusqu'ici) et ajoute un
> **budget de sauts/dashs par tableau** (`EX-GP-024`) — les deux briques d'un gameplay **puzzle**.

## Objectif
- **Interrupteur ↔ porte** : un interrupteur, activé au **contact** du personnage, **bascule** l'état
  de sa **porte** liée ; une porte **fermée** bloque (solide), **ouverte** laisse passer.
- **Budget de mouvements** : un niveau peut **limiter** le nombre de sauts et/ou de dashs ; à budget
  épuisé, l'action est refusée ; le budget est **réinitialisé** au (re)chargement.
- Un **niveau puzzle** de démonstration (`demo4`) qui **exige** ces mécaniques.

Toute la logique de gameplay reste **pure et déterministe au pas fixe** dans `Core` (`EX-NFR-002`,
`EX-ARCH-011`), testée sans GPU. La collision consomme une **grille reflétant l'état des portes**.

## Périmètre

### Inclus
- **Spec** : `EX-GP-024` (budget), ajoutée au cadrage ; `EX-GP-020`/`EX-GP-021` implémentées.
- **Données (Core)** : `Player` (sauts/dashs restants) ; `Level` (budgets `jumpBudget`/`dashBudget`,
  défaut illimité) + parsing `LevelLoader`.
- **Mécanismes (Core)** : `MechanismController` — état des interrupteurs/portes + **grille de
  collision** de travail (portes fermées = solides), mise à jour par le **contact** du personnage.
- **Budget (Core)** : la physique **refuse** saut/dash quand le budget est épuisé et **décompte**
  chaque usage.
- **Intégration (HMI)** : `GameScreen` exécute les mécanismes, passe la **grille de collision** à la
  physique, initialise le budget au spawn, reflète l'état des **portes** au rendu ; niveau `demo4`.
- **Contenu + preuve** : `demo4` (puzzle) exigeant l'usage de l'interrupteur **et** un budget serré ;
  tests d'intégration + parcours **système**.

### Exclus (lots ultérieurs)
- **Bloc poussable** (`EX-GP-022`), **clé/porte verrouillée** (`EX-GP-023`).
- **Animation** d'ouverture de porte (`EX-REN-012`) : ici, changement d'état **instantané** +
  retour visuel simple (teinte).
- Interrupteurs à **action dédiée** (touche) : ici uniquement au **contact**.

## Décisions de cadrage
- **Interrupteur** : activé au **contact** (recouvrement boîte ↔ tuile), **bascule sur front**
  (entrée sur la case) → ne re-bascule pas tant qu'on reste dessus.
- **Porte** : **fermée = solide** par défaut ; interrupteur activé → **ouverte = franchissable**. La
  collision opère sur une **copie mutable** du `TileMap` (portes = `Solid`/`Door` selon l'état),
  laissant la carte du niveau intacte (source de vérité).
- **Budget par niveau** : champs JSON **optionnels** `jumpBudget`/`dashBudget` (défaut **-1** =
  illimité) ; portés par `Player` (restants), initialisés au spawn, **décomptés** par la physique,
  **réinitialisés** au (re)chargement.
- **Retour visuel des portes** : teinte du sprite de la porte selon l'état (fermée opaque / ouverte
  atténuée) — minimal, sans animation.

## Exigences couvertes
- `EX-GP-020` (interrupteur), `EX-GP-021` (porte), `EX-GP-024` (budget).
- `EX-LVL-002`/`EX-LVL-003`/`EX-LVL-004` (modèle/format/validation du niveau, budgets),
  `EX-NFR-002`, `EX-NFR-010`/`EX-NFR-020`, `EX-ARCH-011`.

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-donnees.md) | Données : budget (`Player`, `Level`, `LevelLoader`) | `Core` | ✅ |
| [TACHE-02](tache-02-mecanismes.md) | Mécanismes interrupteur/porte (`MechanismController`) | `Core/Gameplay` | ✅ |
| [TACHE-03](tache-03-budget.md) | Budget de sauts/dashs dans la physique | `Core/Ecs/Systems` | ✅ |
| [TACHE-04](tache-04-integration-puzzle.md) | Intégration `GameScreen` + niveau `demo4` + preuves | `HMI`, `Elements/Levels` | ✅ |

## Critères d'acceptation du lot
1. Toucher un **interrupteur** ouvre/ferme sa **porte** liée ; une porte **fermée bloque**, une
   porte **ouverte** laisse passer.
2. Un niveau à **budget** limité **refuse** le saut/dash au-delà du quota ; le budget se
   **réinitialise** au (re)chargement.
3. `demo4` **exige** l'interrupteur (porte à ouvrir) et un budget serré ; franchissabilité **prouvée
   par test** (et non-franchissable en ignorant la mécanique) ; parcours **système** vert.
4. Logique **couverte par des tests** (`ctest` vert), déterministe.
5. Build `/W4 /WX` sans avertissement, Doxygen et `CHANGELOG.md` à jour, `lint` des exigences vert.

## Dépendances
- Réutilise le modèle de niveau et les liaisons de mécanismes (LOT-07), la physique
  (`CharacterPhysicsSystem`, LOT-08→11), le `GameScreen`/`LevelSequence` (LOT-09).

## Navigation des tâches
- @subpage lot-12-tache-01-donnees
- @subpage lot-12-tache-02-mecanismes
- @subpage lot-12-tache-03-budget
- @subpage lot-12-tache-04-integration-puzzle
