# LOT-ANNEXE-06 — Encodage observation {#lot-annexe-06}

> Statut : **fait**. Prérequis : [LOT-ANNEXE-05](@ref lot-annexe-05) (environnement
> headless) et [LOT-ANNEXE-01](@ref lot-annexe-01) (`Tensor`). Transforme l'état de jeu lu via
> `HeadlessLevelEnvironment` en tenseur exploitable par un réseau de neurones.

## Objectif
`HeadlessLevelEnvironment` (LOT-ANNEXE-05) expose l'état du jeu sous forme de types `Core` (`core::
TileMap`, `core::Player`, `core::Aabb`…) — une représentation faite pour la simulation, pas pour un
réseau de neurones. Ce lot construit la traduction : une fenêtre de tuiles centrée sur le personnage,
son état cinématique et le statut des mécanismes environnants, assemblés en un `aisolver::Tensor`
(LOT-ANNEXE-01) de forme stable, consommé sans modification par tout algorithme des générations 2 et
3 (recherche évolutionniste comme apprentissage par gradient partagent la même observation).

## Périmètre

### Inclus
- **Fenêtre de tuiles** centrée sur la case du personnage, rayon configurable, encodage
  **catégoriel** (one-hot) sur les 29 valeurs de `core::TileType`.
- **Vecteur d'état joueur** : vitesse, `grounded`, `wallDirection`, timers de *game feel* pertinents
  (coyote time, jump buffering, verrouillage de wall jump, dash), budgets de saut/dash restants
  normalisés — tous lus depuis `core::Player`/`core::Velocity`.
- **État des mécanismes dans la fenêtre** : porte ouverte/fermée (`core::MechanismController`),
  danger actif (`core::DangerController`), superposés à la fenêtre de tuiles comme canaux
  supplémentaires.
- Extension mineure, en lecture seule, de la surface publique de `HeadlessLevelEnvironment`
  (accesseurs vers ses contrôleurs internes) pour que l'encodeur puisse interroger cet état sans
  dupliquer la simulation.
- Tests de déterminisme et de stabilité dimensionnelle près des bords de carte.

### Exclus (hors périmètre de ce lot)
- **Le réseau de neurones lui-même** (architecture, propagation) : `Source/AiSolver/Nn`,
  LOT-ANNEXE-03, déjà cadré en amont — ce lot ne produit que son **entrée**.
- **L'espace d'action et le décodage de la sortie du réseau** : LOT-ANNEXE-07, symétrique côté
  sortie plutôt qu'entrée.
- **Toute normalisation apprise** (moyenne/écart-type calculés sur un corpus d'observations) : les
  normalisations de ce lot sont des constantes fixes documentées (bornes connues des grandeurs
  physiques du jeu), pas une statistique à entraîner — cohérent avec l'absence de framework ML.
- **Encodage d'éléments hors fenêtre** (ex. position absolue de la sortie du niveau) : un agent
  entraîné niveau par niveau (régime d'entraînement du programme annexe) n'a pas besoin de savoir où
  se trouve une sortie qu'il ne voit pas encore ; l'introduire ouvrirait la porte à un surapprentissage
  de la position plutôt que de la trajectoire.

## Décisions de cadrage
- **One-hot plutôt qu'un indice entier unique.** `TileType` n'a pas d'ordre numérique significatif
  (`Empty` n'est pas « plus proche » de `Solid` que de `DangerBlink`) : un indice scalaire
  suggérerait au réseau une relation d'ordre inexistante. Le one-hot sur 29 canaux évite ce biais, au
  prix d'une dimension d'entrée plus grande — acceptable, la fenêtre reste petite (rayon
  configurable, quelques dizaines de cases).
- **Cases hors limites de la grille (`core::TileMap::inBounds` faux) encodées en vecteur nul**, sur
  les 29 canaux catégoriels comme sur les deux canaux de mécanisme. Pas de 30ᵉ catégorie « hors
  limites » : cela nécessiterait un nouveau `TileType` dans `Core`, exclu par construction du
  programme annexe (aucune modification de `Core`). Un vecteur nul reste distinguable de toute case
  réelle (dont au moins un canal catégoriel vaut 1), sans ambiguïté pour le réseau en aval.
- **Le danger mobile (`core::DangerMoverConfig`) est rastérisé sur le canal « danger actif », pas
  sur le canal catégoriel.** Sa position **de départ** dans `TileMap` (case `DangerMover`) reste fixe
  toute la partie, alors que sa boîte réellement mortelle (`core::DangerController::moverBox`) se
  déplace en continu (`EX-GP-051`) — les cases actuellement recouvertes par cette boîte sont donc
  marquées actives sur le canal de mécanisme, indépendamment de leur type statique (souvent `Empty`
  hors de la case de départ).
- **Timers normalisés contre des constantes fixes propres à `AiSolver`, pas lues depuis
  `PhysicsConfig`.** `CharacterPhysicsSystem` garde sa configuration **privée** (`_config`, aucun
  accesseur public) : introduire un accesseur uniquement pour ce besoin de normalisation toucherait
  `Core` pour un usage cosmétique (échelle d'entrée réseau). Les bornes nominales (durée de coyote
  time, de jump buffering, etc.) sont donc dupliquées comme constantes documentées côté `AiSolver`,
  à recalibrer si `PhysicsConfig` change (couvert par les tests de ce lot, pas un couplage silencieux).
- **Budget illimité (`jumpsRemaining`/`dashesRemaining == -1`) encodé comme `1.0` constant** (« budget
  toujours disponible »), plutôt qu'une valeur hors intervalle `[0, 1]` qui casserait l'hypothèse
  d'entrée normalisée de tout le reste du vecteur d'état.

## Notions abordées
Voir @ref guide-annexe-apprentissage-renforcement, section « État, observation » (distinction
état réel / observation perçue par l'agent) et @ref guide-annexe-algebre-tensorielle pour
l'encodage sous forme de tenseur (one-hot notamment). Aucune source spécifique au-delà de celles
déjà citées dans ces deux chapitres — ce lot applique les notions, il n'en introduit pas de
nouvelle.

## Exigences couvertes
- Nouvelle, déclarée dans [la spécification IA](@ref spec-ia) : [`EX-IA-006`](@ref EX-IA-006).
- Réutilisées : `EX-IA-005` (environnement headless), `EX-GP-001` (types de tuiles), `EX-GP-019`/
  `025` (masse, plaque de pression), `EX-GP-050`-`053` (dangers avancés), `EX-NFR-002` (déterminisme).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-fenetre-tuiles-encodage-categoriel.md) | Fenêtre de tuiles centrée, encodage catégoriel par `TileType` | `Source/AiSolver/Env` | ✅ |
| [TACHE-02](tache-02-vecteur-etat-joueur.md) | Vecteur d'état joueur | `Source/AiSolver/Env` | ✅ |
| [TACHE-03](tache-03-etat-mecanismes-fenetre.md) | État des mécanismes actifs dans la fenêtre | `Source/AiSolver/Env` | ✅ |
| [TACHE-04](tache-04-tests-determinisme-dimension.md) | Tests : déterminisme et stabilité dimensionnelle aux bords | `Source/Test/Unit/AiSolver/Env` | ✅ |

## Critères d'acceptation du lot
1. L'encodeur produit un tenseur de forme stable (dimensions connues à l'avance à partir du seul
   rayon de fenêtre configuré), quelle que soit la position du personnage sur la carte.
2. Deux appels successifs sur un état `Core` identique produisent des tenseurs bit-à-bit identiques
   (déterminisme, `EX-NFR-002`).
3. Un personnage proche d'un bord de carte produit un tenseur de la **même** forme que loin d'un
   bord, avec les cases hors limites en vecteur nul.
4. Une porte ouverte, une plaque de pression active et un danger avancé actif dans la fenêtre sont
   distinguables dans le tenseur produit (canaux de mécanisme non nuls aux bonnes positions).
5. Build `/W4 /WX` sans avertissement, Doxygen et lint des exigences verts, tests unitaires verts.

## Dépendances
S'appuie sur [LOT-ANNEXE-05](@ref lot-annexe-05) (`HeadlessLevelEnvironment`, dont il étend
légèrement la surface publique) et [LOT-ANNEXE-01](@ref lot-annexe-01) (`aisolver::Tensor`). Aucun
lot du programme principal en dépendance directe au-delà de ce que LOT-ANNEXE-05 référence déjà.

## Navigation des tâches
- @subpage lot-annexe-06-tache-01-fenetre-tuiles-encodage-categoriel
- @subpage lot-annexe-06-tache-02-vecteur-etat-joueur
- @subpage lot-annexe-06-tache-03-etat-mecanismes-fenetre
- @subpage lot-annexe-06-tache-04-tests-determinisme-dimension
