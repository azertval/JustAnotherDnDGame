# LOT-24 — Blocs à taille fractionnaire {#lot-24}

> Statut : **terminé**. Des blocs poussables plus **petits** qu'une case pleine (`×0.5`, `×0.25`,
> `EX-GP-005`), pour des défis de précision — en préparation de blocs poussables **plus grands**
> qu'une case (multi-cases), envisagés pour un lot ultérieur non encore planifié.

## Objectif
`EX-GP-005` demande des blocs poussables à taille réduite (`×0.5`/`×0.25` d'une case), pour des
sauts de précision autour de futurs blocs de grande taille. Contrairement aux pentes (`LOT-22`) et
à l'arrondi (`LOT-23`), qui redéfinissent la **surface** d'une case pleine, un bloc réduit occupe
une case **partiellement** : sa boîte de collision est plus petite que la case et **centrée**
dedans, laissant un espace vide symétrique tout autour. `sweepAabb` (balayage sur grille,
solide/vide par case entière) ne peut pas représenter cela — ce lot introduit donc une **seconde**
routine de collision, boîte-contre-boîte, complémentaire au balayage sur grille plutôt qu'une
modification de celui-ci.

## Périmètre

### Inclus
- Deux nouvelles tuiles : `BlockHalf` (`×0.5`) et `BlockQuarter` (`×0.25`), gérées par
  `core::BlockController` (`LOT-21`) au même titre que `Block` (poussée, chute) — mais avec une
  boîte de collision **réduite et centrée** dans leur case.
- Nouvelle routine de balayage **AABB contre AABB** (le personnage contre un bloc réduit), en
  complément du balayage sur grille existant — pas une modification de `sweepAabb`.
- Chargement/écriture JSON, palette de l'éditeur, rendu (sprite mis à l'échelle).

### Exclus (hors périmètre de ce lot)
- **Blocs plus grands qu'une case (multi-cases)** — explicitement la motivation à terme de cette
  exigence, mais **pas** son contenu : un lot distinct, non encore planifié.
- **Facteurs de taille autres que `0.5`/`0.25`** — pas de taille continue arbitraire.
- **Décalage non centré** — un bloc réduit reste toujours centré dans sa case ; un positionnement
  fin (ex. collé à un bord) n'est pas couvert.
- **Interaction avec les pentes/l'arrondi** (`LOT-22`/`LOT-23`) — un bloc réduit posé sur une
  surface non plate est un cas non défini, laissé de côté.

## Décisions de cadrage
- **La boîte de collision est centrée dans la case, taille `facteur × 1` unité.** Un bloc `×0.5`
  laisse `0.25` unité de vide de chaque côté à l'intérieur de sa case ; un bloc `×0.25` laisse
  `0.375` unité. `BlockController` continue de suivre la **position de case** (`GridPosition`,
  comme pour `Block`) pour la poussée/chute (déplacement toujours d'une case entière à la fois,
  cohérent avec `LOT-21`) ; seule la boîte de collision **testée contre le personnage** est plus
  petite.
- **Nouvelle routine de balayage, pas une extension de `sweepAabb`.** `sweepAabb` reste la
  référence testée pour la collision sur grille (murs, sols, pentes/arrondis de `LOT-22`/`LOT-23`,
  blocs pleins de `LOT-21`) ; les blocs réduits sont résolus par un **second** passage, boîte
  contre boîte, exécuté après le balayage sur grille (dans le même esprit que la poussée de bloc
  de `LOT-21`, déjà résolue dans son propre passage avant la physique). Une collision boîte-boîte
  classique (test de chevauchement sur les deux axes, résolution par l'axe de pénétration minimal)
  est un problème bien plus simple que le suivi de surface de `LOT-22`/`LOT-23` — ce lot reste donc
  d'un risque physique **modéré**, malgré l'ajout d'une routine de collision supplémentaire.
- **La poussée d'un bloc réduit reste case par case**, identique à `Block`. Seule sa boîte de
  collision est plus petite ; le contact déclenchant la poussée (le personnage touche le bloc) doit
  être testé contre cette boîte réduite, pas contre la case entière — sinon le personnage semblerait
  pousser un bloc « à distance », dans l'espace vide qui l'entoure.

## Exigences couvertes
- `EX-GP-005` — implémentée.

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-modele-bloc-reduit.md) | Modèle de bloc réduit | `Core/Levels`, `Core/Gameplay` | ✅ |
| [TACHE-02](tache-02-collision-boite-boite.md) | Collision boîte-contre-boîte | `Core/Physics` | ✅ |
| [TACHE-03](tache-03-editeur-rendu.md) | Éditeur et rendu | `HMI/Editor`, `HMI/Graphics` | ✅ |
| [TACHE-04](tache-04-documentation-verification.md) | Documentation et vérification | `Documentation` | ✅ |

## Critères d'acceptation du lot
1. Un bloc `×0.5`/`×0.25` est poussable et tombe exactement comme un bloc plein (`LOT-21`),
   déplacement case par case.
2. Sa boîte de collision, plus petite et centrée, laisse un espace franchissable autour de lui
   (vérifiable visuellement et par un saut de précision).
3. **Aucune régression** sur les mécanismes existants (`Block` plein, portes, pentes/arrondis si
   déjà livrés).
4. Logique nouvelle couverte par des tests. Build `/W4 /WX` sans avertissement, Doxygen et lint des
   exigences verts.

## Dépendances
- Étend `core::BlockController` (`LOT-21`) sans le modifier en profondeur ; **indépendant** de
  `LOT-22`/`LOT-23` (surface plane uniquement pour ce lot), peut donc être mené en parallèle ou
  avant eux si l'ordre s'avère plus pratique à l'implémentation — l'ordre 22 → 23 → 24 proposé
  reflète la priorité exprimée, pas une dépendance technique stricte de 24 envers 22/23.

## Navigation des tâches
- @subpage lot-24-tache-01-modele-bloc-reduit
- @subpage lot-24-tache-02-collision-boite-boite
- @subpage lot-24-tache-03-editeur-rendu
- @subpage lot-24-tache-04-documentation-verification
