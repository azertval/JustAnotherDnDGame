# TACHE-03 — Perte du critique et optimisation conjointe acteur/critique {#lot-annexe-13-tache-03-perte-critique-optimisation-conjointe}

**Lot :** [LOT-ANNEXE-13](epic.md) · **Emplacement :** `Source/AiSolver/Training/ActorCritic` ·
**Statut :** fait

## Contexte
Le critique (TACHE-01) doit lui-même être entraîné pour que son estimation de valeur devienne fiable
— sans quoi l'avantage (TACHE-02) resterait un signal bruité, ne réduisant en rien la variance
recherchée. Cette tâche ajoute la perte d'entraînement du critique et assemble la boucle complète
qui optimise **les deux réseaux à chaque épisode**, en réutilisant et en étendant
`ReinforceTrainer` (LOT-ANNEXE-12).

## Travail à réaliser
- **`aisolver::training::computeCriticLoss`**
  (`Source/AiSolver/Training/ActorCritic/CriticLoss.h/.cpp`) : fonction
  `autodiff::NodePtr computeCriticLoss(training::CriticNetwork& critic, const Trajectory& trajectory, const std::vector<float>& returns)` —
  pour chaque pas, rejoue `critic.forward` sur l'observation (nœud de graphe, poids du critique
  actuels), calcule l'écart quadratique `(valeur_t − returns[t])²`, moyenne sur l'épisode. Graphe
  d'autodiff indépendant de celui de la perte de politique (TACHE-02).
- **`aisolver::training::ActorCriticTrainer`**
  (`Source/AiSolver/Training/ActorCritic/ActorCriticTrainer.h/.cpp`) : construit à partir des mêmes
  éléments que `ReinforceTrainer` (LOT-ANNEXE-12) plus un `CriticNetwork` et un **second**
  optimiseur (indépendant de celui de la politique). Méthode `void run(std::size_t episodeCount)` :
  pour chaque épisode — collecte de trajectoire (TrajectoryCollector, LOT-ANNEXE-12, inchangé),
  calcul des retours (`computeReturns`, LOT-ANNEXE-12, inchangé), calcul des avantages
  (`computeAdvantages`, TACHE-02), perte de politique (`computeActorCriticLoss`, TACHE-02) et perte
  du critique (`computeCriticLoss`, ci-dessus) — `backward()` et `step()` de l'**optimiseur de la
  politique** sur la perte de politique, `backward()` et `step()` de l'**optimiseur du critique** sur
  la perte du critique, **remise à zéro des gradients des deux réseaux** avant l'épisode suivant,
  journalisation via `TrainingStatsRecorder` (LOT-ANNEXE-09) incluant désormais la perte du critique
  en plus des colonnes déjà présentes pour LOT-ANNEXE-12.
- Ordre des deux mises à jour **non significatif** dans ce lot (aucune dépendance entre les deux
  passages arrière, puisque l'avantage utilisé dans la perte de politique est une valeur détachée du
  graphe du critique, cf. TACHE-02) — documenté explicitement pour éviter qu'un futur lot suppose un
  ordre implicite.

## Fichiers impactés
- `Source/AiSolver/Training/ActorCritic/CriticLoss.h` (nouveau).
- `Source/AiSolver/Training/ActorCritic/CriticLoss.cpp` (nouveau).
- `Source/AiSolver/Training/ActorCritic/ActorCriticTrainer.h` (nouveau).
- `Source/AiSolver/Training/ActorCritic/ActorCriticTrainer.cpp` (nouveau).
- `Source/AiSolver/CMakeLists.txt` (nouveaux fichiers).

## Tests (obligatoires)
- **Gradient checking de `computeCriticLoss`** : gradient rétropropagé vs différences finies, même
  méthodologie que LOT-ANNEXE-02/12.
- **Convergence du critique** : sur le niveau de contrôle, l'erreur quadratique moyenne du critique
  (moyenne de `(valeur_t − returns[t])²` sur l'épisode) **diminue** entre le début et la fin d'un run
  d'entraînement.
- **Indépendance des deux optimisations** : une exécution où l'optimiseur du critique est
  volontairement figé (pas de `step()`) laisse néanmoins l'optimiseur de la politique continuer à
  produire des mises à jour normales (les deux boucles ne sont pas couplées par erreur).
- **Gradients remis à zéro correctement pour les deux réseaux** entre deux épisodes consécutifs (pas
  de fuite d'un épisode à l'autre, ni entre acteur et critique).
- **CSV enrichi cohérent** : la colonne de perte du critique est présente et numériquement plausible
  (finie, décroissante en tendance) sur un run court.

## Points d'attention
- **Deux graphes d'autodiff bien séparés par épisode** : construire la perte de politique et la
  perte du critique comme deux appels indépendants, chacun suivi de son propre `backward()` — ne
  jamais les combiner dans un seul nœud scalaire final (cela recréerait un couplage de gradient que
  la décision de cadrage « pas de poids partagés » exclut explicitement).
- **Ne pas oublier de remettre à zéro les gradients du critique** en plus de ceux de la politique
  entre deux épisodes — piège d'oubli facile en étendant `ReinforceTrainer`, qui ne gérait qu'un
  seul réseau.
- Le format CSV **étend** celui de LOT-ANNEXE-09/12 (colonne supplémentaire), il ne le **remplace**
  pas : les runs REINFORCE bruts déjà produits par LOT-ANNEXE-12 restent lisibles et comparables sans
  transformation, condition nécessaire à TACHE-04.

## Définition de fait (DoD)
- `computeCriticLoss` et `ActorCriticTrainer` disponibles et testés (`ctest` vert), convergence du
  critique observée sur le niveau de contrôle ; build `/W4 /WX` sans avertissement ; Doxygen à jour ;
  `EX-IA-014` déclarée dans l'épic.

## Notions abordées
@ref guide-annexe-acteur-critique — variance du gradient, fonction de valeur, critique, avantage.

## Exigences
`EX-IA-014` (nouvelle).
