# LOT-31 — Blocs de danger avancés {#lot-31}

> Statut : **✅ terminé**. Quatre nouvelles variantes de la tuile `Danger`
> (`EX-GP-031`) : directionnel (pics), mobile (va-et-vient autonome), commuté (lié à un
> interrupteur/plaque) et temporisé (clignotant) — un seul lot, une tâche par variante.

## Objectif
La tuile `Danger` (`core::TileType::Danger`) est aujourd'hui **unique** : case pleine, statique,
mortelle sur toute sa surface, vérifiée par simple recouvrement AABB dans `core::evaluateOutcome`
(`Source/Core/Levels/LevelOutcome.cpp`). Ce vocabulaire est suffisant pour un danger « mur » ou
« fosse », mais ne permet aucun des motifs classiques du genre plateforme/puzzle évoqués par
`vision.md` (« boîte à outils classique... dangers ») : pics contre lesquels on peut se tenir sans
mourir, danger qui patrouille, piège désactivé tant qu'on n'a pas déclenché un mécanisme, ou
minuterie à mémoriser. Ce lot ajoute ces quatre variantes sans toucher à la règle de fin de niveau
elle-même (`EX-GP-031` : contact = échec, `EX-GP-032` : redémarrage) — seules la **géométrie** et
l'**activation** du danger varient.

## Périmètre

### Inclus
- **Danger directionnel** (`EX-GP-050`) : quatre nouveaux types (un par bord — haut/bas/gauche/
  droite), hitbox = bande étroite le long du bord concerné plutôt que la case entière. Même
  infrastructure de test que `Danger` (recouvrement AABB dans `LevelOutcome`), juste un rectangle
  plus petit et décalé — pas de notion de face d'approche/sens de déplacement.
- **Danger mobile** (`EX-GP-051`) : un type dont la position **évolue** chaque pas fixe, aller-retour
  linéaire sur un axe (horizontal ou vertical) autour de sa position de départ dans le fichier.
  Nouveau petit contrôleur cinématique dédié (position déterministe en fonction du nombre de pas
  fixes écoulés, pas de gravité ni de poussée par le personnage — à la différence du bloc poussable
  `EX-GP-022`).
- **Danger commuté** (`EX-GP-052`) : un type mortel uniquement quand l'interrupteur/la plaque de
  pression qui lui est lié est **actif** — inverse de la porte (`EX-GP-021`), même infrastructure de
  liaison par identifiant (`opensWith`) et même geste éditeur (clic déclencheur, clic cible).
- **Danger temporisé** (`EX-GP-053`) : un type qui alterne mortel/inoffensif selon une **période
  fixe** (constante de conception), avec un **déphasage** par tuile pour permettre des motifs
  désynchronisés.
- Intégration éditeur (palette — nouvelle sous-catégorie sous « Piège », rendu) et jeu (résolution
  de fin de niveau, rendu) pour les quatre variantes.
- Niveau(x) de démonstration illustrant au moins une des nouvelles variantes.

### Exclus (hors périmètre de ce lot)
- **Dégâts progressifs / vies multiples** : le contact avec un danger (quelle que soit la variante)
  reste un échec **instantané** du niveau (`EX-GP-031`), pas de système de vie — cohérent avec le
  reste du jeu.
- **Danger mobile suivant un chemin arbitraire multi-points** : seul l'aller-retour linéaire sur un
  axe est couvert ; un système de rails/waypoints multiples est un lot naturel ultérieur si besoin.
- **Édition numérique en jeu de la vitesse/portée du mobile ou de la période/déphasage du
  temporisé** : valeurs de conception (constantes ou champs JSON à valeur par défaut), éditables en
  modifiant le fichier de niveau mais pas via un nouveau widget numérique de l'éditeur — cohérent
  avec `jumpBudget`/`dashBudget` (`EX-GP-024`), déjà non exposés dans l'éditeur aujourd'hui.
  L'éditeur place la tuile (et, pour le mobile, l'axe/la portée par un second clic façon
  waypoint) ; le réglage fin reste dans le fichier.
- **Danger commuté porté par une plaque de pression avec poids différencié par tuile** : reprend
  telle quelle la limite déjà actée en `LOT-19` (seuil global, pas de poids configurable par case).

## Décisions de cadrage
- **Un seul lot, quatre tâches** (une par variante) plutôt que quatre lots séparés : décidé avec le
  demandeur — les quatre variantes partagent la même case d'entrée (`TileType`, `LevelLoader`/
  `LevelWriter`, `LevelOutcome`, palette, rendu) et gagnent à être revues ensemble plutôt qu'en série
  de petites PR qui retouchent les mêmes fichiers.
- **Danger directionnel = hitbox réduite positionnée sur un bord**, pas une détection de face
  d'approche/sens de déplacement : plus simple, déterministe, et cohérent avec l'infrastructure
  AABB déjà en place dans `LevelOutcome` (le même principe qu'une boîte de collision centrée pour
  `BlockHalf`/`BlockQuarter`, `EX-GP-005`, mais décalée sur un bord plutôt que centrée).
- **Danger mobile = nouveau contrôleur cinématique dédié**, pas de réutilisation de
  `core::BlockController` : la sémantique (poussée par le personnage, chute sous gravité) ne
  correspond pas à un mouvement autonome sur rail, imposer l'abstraction commune aurait forcé des
  branches mortes dans `BlockController`.
- **Danger commuté réutilise le même schéma de liaison par identifiant** (`switch.id` ↔
  `opensWith`) que l'interrupteur/porte, mais via une struct **dupliquée** (`core::DangerLink`,
  résolue dans une liste séparée) plutôt qu'une généralisation de `core::Mechanism` — tranché en
  TACHE-01 une fois constaté que `Mechanism::doorPosition` est consommé par nom dans une trentaine
  de sites (contrôleur, éditeur, tests), tous spécifiques à une porte.
- **Danger temporisé n'a aucune dépendance à un interrupteur** : sa période/déphasage sont des
  champs propres à la tuile, pas une variante du danger commuté — les deux mécanismes
  d'activation (minuterie vs. déclencheur) restent séparés plutôt que fusionnés en un seul type
  paramétrable, pour rester lisible dans la palette et le format JSON.

## Exigences couvertes
- `EX-GP-050` — danger directionnel.
- `EX-GP-051` — danger mobile.
- `EX-GP-052` — danger commuté.
- `EX-GP-053` — danger temporisé.
- Étend `EX-GP-031`/`EX-GP-032` (échec/redémarrage, comportement inchangé) et `EX-EDIT-002`/
  `EX-EDIT-003` (palette, liaison visuelle de mécanismes) aux quatre nouvelles variantes.

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-modele-dangers-avances.md) | Modèle (types, format, liaisons) | `Core/Levels` | ✅ |
| [TACHE-02](tache-02-integration-jeu.md) | Intégration jeu (contrôleurs, résolution de fin de niveau) | `Core/Gameplay`, `Core/Levels` | ✅ |
| [TACHE-03](tache-03-integration-editeur.md) | Intégration éditeur (palette, rendu, liaison) | `HMI/Editor`, `HMI/Graphics`, `HMI/Interface` | ✅ |
| [TACHE-04](tache-04-documentation-verification.md) | Documentation et vérification | `Documentation` | ✅ |

## Critères d'acceptation du lot
1. Les quatre nouvelles variantes sont peignables depuis la palette de l'éditeur, s'enregistrent et
   se rechargent fidèlement au format JSON (round-trip, `EX-EDIT-011`).
2. En jeu : un danger directionnel ne tue que depuis son bord désigné ; un danger mobile tue à sa
   position **courante** (pas sa position de départ) ; un danger commuté ne tue que lorsque son
   déclencheur est actif ; un danger temporisé alterne selon sa période, de façon déterministe et
   reproductible (`EX-NFR-002`).
3. Un niveau existant (non modifié) continue de se comporter à l'identique — aucune régression sur
   `Danger` classique ni sur les mécanismes interrupteur↔porte existants.
4. Un niveau de démonstration (`demo-dangers-avances`) illustre les **quatre** nouvelles variantes
   et est intégré à la séquence jouée.
5. Logique nouvelle couverte aux **trois niveaux de test** du projet : Unit (chaque brique isolée —
   `dangerHitbox`, `DangerController`, parsing JSON), Integration (`DangerController` assemblé à
   `LevelOutcome`/`CharacterPhysicsSystem`, un scénario par variante), Système
   (`demo-dangers-avances` rejoué bout en bout dans `test_parcours_complet.cpp`, et round-trip
   d'édition dans `test_parcours_edition.cpp`). Build `/W4 /WX` sans avertissement, Doxygen (vérifié
   avec le binaire 1.9.8 de la CI avant tout push) et lint des exigences verts.

## Dépendances
- Étend `core::TileType`/`core::LevelOutcome` (`LOT-07`), `core::MechanismController` (`LOT-08`,
  liaison interrupteur↔porte), `core::BlockController` (`LOT-21`, pour contraste — non réutilisé) et
  la palette par catégories (`LOT-27`).

## Navigation des tâches
- @subpage lot-31-tache-01-modele-dangers-avances
- @subpage lot-31-tache-02-integration-jeu
- @subpage lot-31-tache-03-integration-editeur
- @subpage lot-31-tache-04-documentation-verification
