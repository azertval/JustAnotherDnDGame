# LOT-ANNEXE-10 — Algorithme évolutionniste maison {#lot-annexe-10}

> Statut : **fait**. Dépend de [LOT-ANNEXE-01](@ref lot-annexe-01) (`Tensor`/`Rng` maison),
> [LOT-ANNEXE-03](@ref lot-annexe-03) (bibliothèque de réseaux de neurones maison),
> [LOT-ANNEXE-05](@ref lot-annexe-05) (`HeadlessLevelEnvironment`), [LOT-ANNEXE-08](@ref
> lot-annexe-08) (fonction de récompense et critères d'épisode) et [LOT-ANNEXE-09](@ref
> lot-annexe-09) (`TrainingStatsRecorder`). Premier lot de la génération 2 : chemin le plus court
> vers un agent qui termine réellement un niveau, sans dépendre de l'autodiff.

## Objectif
La génération 0/1 a livré les fondations (calcul, réseaux, pont avec le jeu, mesure) mais aucun
mécanisme n'apprend encore quoi que ce soit : un réseau de neurones aux poids fixes ne fait que
calculer une sortie, il ne s'améliore pas. Ce lot livre le **premier algorithme d'apprentissage**
du programme Lot-Annexe — une recherche par **population** (sélection, croisement, mutation,
élitisme) plutôt que par gradient, qui ne nécessite ni rétropropagation ni fonction de perte
différentiable (`Source/AiSolver/Nn` n'a pas besoin de calculer de gradient pour ce lot). C'est
délibérément le chemin le plus **simple** vers un agent qui termine effectivement un niveau : il
sert de **ligne de base** (« qu'obtient-on sans rien de plus sophistiqué qu'une population qui
mute ? ») à laquelle comparer, via les CSV de LOT-ANNEXE-09, les résultats de la génération 3
(LOT-ANNEXE-12/13/14, apprentissage par gradient, exigence ferme d'un vrai modèle appris).

## Périmètre

### Inclus
- **Population de réseaux** (`aisolver::training::evolutionary::Population`) : un ensemble de taille
  fixe d'individus, chacun un réseau complet de la bibliothèque de LOT-ANNEXE-03, aux poids
  initialisés indépendamment via `aisolver::Rng` (LOT-ANNEXE-01) — jamais `std::rand`, jamais
  l'horloge.
- **Évaluation de fitness** : un individu = un run complet, déterministe, sur un
  `HeadlessLevelEnvironment` (LOT-ANNEXE-05) déjà construit pour un niveau donné ; le fitness est la
  récompense cumulée sur l'épisode, telle que définie par LOT-ANNEXE-08 — aucune redéfinition ni
  repondération locale de la récompense dans ce lot.
- **Opérateurs génétiques** : sélection par tournoi, croisement des poids par moyenne, mutation par
  bruit gaussien à taux configurable (tous deux via `Rng`), élitisme à un individu (le meilleur
  survit inchangé, sans réévaluation).
- **Boucle de génération**, assemblant les briques ci-dessus, journalisée à chaque génération via
  `TrainingStatsRecorder` (LOT-ANNEXE-09) — générique, sans connaître de notion de « niveau
  résolu » (voir décision de cadrage).
- Tests de **reproductibilité stricte à seed fixée** et de **non-régression de la croissance du
  fitness** sur un niveau trivial de contrôle.

### Exclus (hors périmètre de ce lot)
- **Autodiff/rétropropagation** : réservée à la génération 3 (LOT-ANNEXE-12 et suivants) — ce lot
  prouve délibérément qu'un agent peut apprendre **sans** gradient, comme point de comparaison.
- **Évolution de la topologie du réseau** (type NEAT) : l'architecture (nombre/taille de couches)
  est fixe et partagée par toute la population, seuls les **poids** évoluent — complexité inutile
  pour une ligne de base.
- **Parallélisation de l'évaluation** (multi-thread, multi-environnement) : l'évaluation reste
  séquentielle, un individu après l'autre sur un unique `HeadlessLevelEnvironment` réinitialisé à
  chaque run — optimisation potentielle future, non nécessaire pour prouver le principe.
- **Auto-adaptation du taux/de la force de mutation** en cours d'entraînement : valeurs fixes,
  configurables au lancement, jamais ajustées automatiquement par l'algorithme lui-même.
- **Critère d'arrêt métier** (« niveau résolu N fois d'affilée ») et **application à un niveau
  précis avec export du résultat** : c'est tout le périmètre de LOT-ANNEXE-11, qui pilote cette
  boucle depuis l'extérieur.

## Décisions de cadrage
- **Un run d'algorithme évolutionniste = un niveau, toujours.** Cette boucle prend en entrée un
  `HeadlessLevelEnvironment` déjà construit pour **un** niveau ; elle ne connaît jamais de liste de
  niveaux ni de notion d'enchaînement. C'est une contrainte transverse du programme Lot-Annexe,
  réaffirmée ici parce que c'est la première brique qui pourrait être tentée de généraliser trop
  tôt (ex. faire tourner un même individu sur plusieurs environnements pour « moyenner » son
  fitness) — explicitement écarté.
- **Décodage de politique en `argmax` (déterministe), même pendant l'évaluation de fitness pendant
  l'entraînement.** Contrairement à une méthode de gradient de politique, un algorithme
  évolutionniste tire son exploration de la **diversité de la population** et de la **mutation des
  poids**, pas d'un tirage stochastique de l'action à chaque pas — un réseau donné produit donc
  toujours la même trajectoire sur un niveau donné. Conséquence directe : le comportement observé
  pendant l'entraînement est **identique** à celui du rejeu final (LOT-ANNEXE-11/TACHE-02), aucune
  divergence entraînement/rejeu à expliquer.
- **La boucle de génération est agnostique de tout critère d'arrêt métier.** Elle exécute une
  génération à la fois (ou un nombre fixe passé en configuration), sans savoir ce qu'est un
  « niveau résolu » — cette notion et le pilotage qui en découle (résolu N fois d'affilée, plafond
  de générations) appartiennent à LOT-ANNEXE-11. Sépare la **mécanique générique** (ce lot,
  réutilisable indépendamment du critère de succès choisi) de la **politique d'usage** (le lot
  suivant).
- **Élitisme à un seul individu, jamais réévalué.** Le meilleur individu d'une génération est
  recopié tel quel dans la suivante avec son fitness déjà connu (pas de nouveau run) : économise une
  évaluation par génération et garantit, par construction, que le meilleur fitness observé ne peut
  jamais régresser d'une génération à l'autre — propriété directement testable.
- **Croisement par moyenne des poids, pas par point de coupure.** Un réseau n'a pas d'ordre linéaire
  de ses poids qui rendrait un point de coupure sémantiquement significatif (contrairement à un
  chromosome classique) ; la moyenne élément par élément reste valide quelle que soit l'indexation
  interne et s'implémente directement avec l'API `Tensor` de LOT-ANNEXE-01.
- **Toute la stochasticité passe par une seule instance de `Rng`**, transmise explicitement à chaque
  opérateur (jamais une source globale) — condition nécessaire à la reproductibilité stricte exigée
  par les tests de ce lot, et à la capacité de déboguer un entraînement en le rejouant à l'identique.

## Notions abordées
Voir @ref guide-annexe-algorithmes-evolutionnistes (population, fitness, sélection par tournoi,
croisement, mutation, élitisme). Sources directes : Holland (1975, algorithmes génétiques) ; Such
et al. (2017, *Deep Neuroevolution*, justification empirique directe de l'évolution de poids de
réseau pour du RL) ; Salimans et al. (2017, stratégies d'évolution comme alternative au RL par
gradient) — bibliographie complète dans le chapitre.

## Exigences couvertes
- Nouvelle, déclarée dans [la spécification IA](@ref spec-ia) : [`EX-IA-011`](@ref EX-IA-011).
- Réutilisées (inchangées) : bibliothèque de calcul et générateur pseudo-aléatoire de
  LOT-ANNEXE-01, bibliothèque de réseaux de neurones de LOT-ANNEXE-03, `HeadlessLevelEnvironment`
  de LOT-ANNEXE-05, fonction de récompense et critères d'épisode de LOT-ANNEXE-08, journalisation
  CSV de LOT-ANNEXE-09.

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-population-reseaux.md) | Population de réseaux | `Source/AiSolver/Training/Evolutionary` | ✅ |
| [TACHE-02](tache-02-evaluation-fitness.md) | Évaluation de fitness | `Source/AiSolver/Training/Evolutionary` | ✅ |
| [TACHE-03](tache-03-selection-croisement-mutation.md) | Sélection, croisement, mutation, élitisme | `Source/AiSolver/Training/Evolutionary` | ✅ |
| [TACHE-04](tache-04-boucle-generation.md) | Boucle de génération | `Source/AiSolver/Training/Evolutionary` | ✅ |
| [TACHE-05](tache-05-tests-reproductibilite.md) | Tests de reproductibilité et non-régression | `Source/Test/Unit/AiSolver/Training` | ✅ |

## Critères d'acceptation du lot
1. Une population de N individus (réseaux de LOT-ANNEXE-03, topologie identique) est instanciée
   avec des poids initiaux indépendants, générés uniquement via `Rng` à seed fixée.
2. Chaque génération évalue tous les individus sur un run complet de `HeadlessLevelEnvironment` et
   leur assigne un fitness = récompense cumulée (LOT-ANNEXE-08).
3. Sélection, croisement, mutation et élitisme produisent la génération suivante ; le meilleur
   fitness d'une génération est toujours ≥ celui de la génération précédente (élitisme, testé).
4. À seed fixée, deux exécutions complètes de N générations produisent des poids finaux et un
   historique de fitness strictement identiques (testé, comparaison bit-à-bit).
5. Sur un niveau trivial de contrôle, le meilleur fitness croît de façon significative sur une
   fenêtre de générations fixée et franchit un seuil documenté (non-régression testée).
6. Chaque génération est enregistrée via `TrainingStatsRecorder` (au minimum : indice de
   génération, meilleur fitness, fitness moyen).
7. Build `/W4 /WX` sans avertissement, `ctest` vert, Doxygen à jour, `EX-IA-011` déclarée.

## Dépendances
Bâtit sur [LOT-ANNEXE-01](@ref lot-annexe-01) (`Tensor`/`Rng`), [LOT-ANNEXE-03](@ref lot-annexe-03)
(réseaux de neurones), [LOT-ANNEXE-05](@ref lot-annexe-05) (`HeadlessLevelEnvironment`),
[LOT-ANNEXE-06](@ref lot-annexe-06) (encodage d'observation, via les réseaux évalués),
[LOT-ANNEXE-07](@ref lot-annexe-07) (espace d'action/décodage, via le décodage `argmax`),
[LOT-ANNEXE-08](@ref lot-annexe-08) (récompense/critères d'épisode) et [LOT-ANNEXE-09](@ref
lot-annexe-09) (statistiques). [LOT-ANNEXE-11](@ref lot-annexe-11) dépend de celui-ci.

## Navigation des tâches
- @subpage lot-annexe-10-tache-01-population-reseaux
- @subpage lot-annexe-10-tache-02-evaluation-fitness
- @subpage lot-annexe-10-tache-03-selection-croisement-mutation
- @subpage lot-annexe-10-tache-04-boucle-generation
- @subpage lot-annexe-10-tache-05-tests-reproductibilite
