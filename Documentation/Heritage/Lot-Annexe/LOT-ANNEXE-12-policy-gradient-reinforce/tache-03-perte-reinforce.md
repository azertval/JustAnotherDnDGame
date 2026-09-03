# TACHE-03 — Perte REINFORCE rétropropagée via l'autodiff maison {#lot-annexe-12-tache-03-perte-reinforce}

**Lot :** [LOT-ANNEXE-12](epic.md) · **Emplacement :** `Source/AiSolver/Training/PolicyGradient` ·
**Statut :** fait

## Contexte
C'est la tâche **centrale** du lot : celle qui fait réellement exister l'exigence ferme
« apprentissage par gradient » (`EX-IA-013`). TACHE-01 a produit une trajectoire (observations,
actions, log-probabilités, récompenses) ; TACHE-02 a produit un retour `G_t` par pas. Il reste à
construire, pour chaque pas, la perte scalaire `-log π(a_t | s_t) × G_t` comme un **graphe** du
moteur d'autodiff maison (LOT-ANNEXE-02) — pas comme un nombre calculé une fois pour toutes — pour
que `backward()` puisse ensuite propager le gradient jusqu'aux poids du réseau de politique
(LOT-ANNEXE-03).

## Travail à réaliser
- **`aisolver::training::computeReinforceLoss`**
  (`Source/AiSolver/Training/PolicyGradient/ReinforceLoss.h/.cpp`) : fonction
  `autodiff::NodePtr computeReinforceLoss(nn::Network& policy, const Trajectory& trajectory, const std::vector<float>& returns)` —
  pour chaque pas `t` de la trajectoire : **rejoue** le passage avant de la politique sur
  l'observation `s_t` (nouveau nœud de graphe, poids du réseau **actuels**, potentiellement déjà mis
  à jour depuis la collecte si l'entraînement enchaîne plusieurs passes — ce lot fait une seule
  passe par trajectoire collectée, cf. décision de cadrage de la boucle en TACHE-04), extrait la
  log-probabilité de l'action `a_t` effectivement jouée (même indice que celui enregistré dans
  `TrajectoryStep`, pas une nouvelle action tirée), multiplie par `-returns[t]` ; somme les termes de
  tous les pas en un unique nœud scalaire.
- **Opérations d'autodiff mises en œuvre**, toutes livrées par
  @ref lot-annexe-03-tache-05-operations-differentiables-complementaires — aucune ne doit être
  réécrite ici : `selectIndex(distribution, step.actionIndex)` pour extraire la probabilité de
  l'action jouée **en restant dans le graphe**, `logOp` pour en prendre le logarithme,
  `multiplyScalar(…, -returns[t])` pour appliquer le retour (grandeur détachée, jamais un nœud),
  `add` (LOT-ANNEXE-02) pour cumuler les pas, puis `multiplyScalar(…, 1.0f / nombreDePas)` pour la
  moyenne. Si l'une de ces opérations manque à l'appel du lot, c'est `LOT-ANNEXE-03` (TACHE-05) qui
  est incomplet — ne pas contourner en calculant un gradient à la main.
- Le graphe résultant est un graphe d'autodiff **normal** : `backward()` (LOT-ANNEXE-02) sur ce nœud
  final calcule, par rétropropagation, le gradient de la perte par rapport à **tous** les paramètres
  du réseau de politique traversés par le passage avant — aucun calcul de gradient écrit à la main
  dans ce fichier.
- Convention de signe : la perte est déjà le **négatif** de l'objectif de policy gradient
  (`log π × G_t` doit être **maximisé**, la perte le **minimise**) — les optimiseurs de
  LOT-ANNEXE-04 descendent le gradient de la perte sans modification, cohérent avec leur usage pour
  les réseaux supervisés.
- Moyenne (pas somme brute) sur le nombre de pas de l'épisode, pour que l'amplitude de la perte ne
  dépende pas de la longueur variable des épisodes (un épisode long et un épisode court ne doivent
  pas produire des mises à jour d'amplitude mécaniquement différente pour cette seule raison).

## Fichiers impactés
- `Source/AiSolver/Training/PolicyGradient/ReinforceLoss.h` (nouveau).
- `Source/AiSolver/Training/PolicyGradient/ReinforceLoss.cpp` (nouveau).
- `Source/AiSolver/CMakeLists.txt` (nouveaux fichiers).

## Tests (obligatoires)
- **Gradient checking** : sur un réseau de politique minuscule (quelques poids) et une trajectoire
  synthétique courte, le gradient obtenu par `backward()` sur `computeReinforceLoss` correspond au
  gradient obtenu par différences finies, à la tolérance retenue par l'infrastructure de gradient
  checking de LOT-ANNEXE-02 — c'est le test le plus important du lot.
- **Signe du gradient** : une action à log-probabilité **basse** associée à un retour **positif
  élevé** produit, après un pas d'optimiseur, une augmentation mesurable de la probabilité de cette
  action lors d'un nouveau passage avant sur la même observation (vérifie le sens de la mise à jour,
  pas seulement sa magnitude).
- **Perte nulle dégénérée absente** : sur une trajectoire non triviale (retours non tous nuls), la
  perte calculée est non nulle et son gradient par rapport à au moins un poids est non nul.
- **Indépendance à la longueur d'épisode** (moyenne, pas somme) : deux trajectoires synthétiques de
  longueurs différentes mais de retours par pas comparables produisent des pertes d'ordre de
  grandeur comparable.

## Points d'attention
- **Rejouer le passage avant, ne jamais réutiliser les `logProbability` stockés dans
  `TrajectoryStep` comme valeurs de graphe** : ces derniers sont des `float` détachés (TACHE-01),
  utiles pour les tests et diagnostics, mais **pas** connectés au graphe d'autodiff — seul le nœud
  reconstruit ici porte l'historique nécessaire à `backward()`.
- **Ne pas mélanger plusieurs trajectoires dans un seul graphe** sans y avoir explicitement pensé :
  ce lot est épisode-par-épisode (TACHE-04) ; un futur passage à des lots d'épisodes (hors périmètre
  ici) demanderait de revisiter l'agrégation, pas seulement cette fonction.
- La **convention de signe** (perte = négatif de l'objectif) est le piège classique de REINFORCE :
  toute confusion inverse silencieusement le sens de l'apprentissage sans qu'aucune erreur de
  compilation ou d'exécution ne le signale — seul le test de signe ci-dessus le détecte.

## Définition de fait (DoD)
- `computeReinforceLoss` disponible, gradient checking vert (`ctest`) ; build `/W4 /WX` sans
  avertissement ; Doxygen à jour ; `EX-IA-013` effectivement exercée par ce calcul.

## Notions abordées
@ref guide-annexe-reinforce (policy gradient, trajectoire, retour actualisé, algorithme REINFORCE),
ainsi que @ref guide-annexe-autodiff : la perte est un **graphe** dont `backward()` tire le
gradient, pas un nombre calculé une fois pour toutes.

## Exigences
`EX-IA-013` (nouvelle).
