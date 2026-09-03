# TACHE-04 — Comparaison chiffrée de convergence vs REINFORCE brut {#lot-annexe-13-tache-04-comparaison-convergence}

**Lot :** [LOT-ANNEXE-13](epic.md) · **Emplacement :** `Source/AiSolver/Training/ActorCritic` ·
**Statut :** fait

## Contexte
Les tâches précédentes rendent l'acteur-critique fonctionnel ; il reste à **démontrer**, avec des
chiffres et non seulement une intuition, que la réduction de variance apporte un bénéfice mesurable
par rapport à REINFORCE brut (LOT-ANNEXE-12). C'est le critère d'acceptation le plus direct du lot
et le socle sur lequel repose la décision de cadrage différée de LOT-ANNEXE-14.

## Travail à réaliser
- **Protocole de comparaison** : sur le **même** niveau de contrôle (celui déjà utilisé par
  LOT-ANNEXE-12, TACHE-05) et le **même** budget d'épisodes, exécuter un run `ReinforceTrainer`
  (LOT-ANNEXE-12) et un run `ActorCriticTrainer` (TACHE-03) avec des graines de base différentes mais
  un nombre d'essais répétés identique de chaque côté (plusieurs graines par algorithme, pour
  mesurer une variance inter-essais et pas un seul tirage).
- **`aisolver::training::ConvergenceReport`**
  (`Source/AiSolver/Training/ActorCritic/ConvergenceComparator.h/.cpp`) : utilitaire qui lit les CSV
  produits par les deux séries de runs (`TrainingStatsRecorder`, LOT-ANNEXE-09) et calcule, pour
  chaque algorithme : le nombre moyen d'épisodes nécessaires pour atteindre un plafond de récompense
  donné (seuil défini une fois pour les deux), et l'écart-type de la récompense sur la fin du run
  (fenêtre des derniers épisodes), à travers les essais répétés.
- Un outil ou test exécutable produit un résumé lisible (au minimum une sortie texte/console des
  deux séries de métriques comparées ; un export CSV supplémentaire est acceptable mais pas requis) ;
  le point du lot est le calcul et la vérification des métriques, pas un tableau de bord.
- Consignation du résultat obtenu (à titre de mesure, pas de garantie figée) dans la documentation du
  lot (`epic.md`, section Critères d'acceptation, ou un complément dans cette tâche une fois exécutée)
  pour que la décision de LOT-ANNEXE-14 puisse s'appuyer dessus sans avoir à rejouer les runs.

## Fichiers impactés
- `Source/AiSolver/Training/ActorCritic/ConvergenceComparator.h` (nouveau).
- `Source/AiSolver/Training/ActorCritic/ConvergenceComparator.cpp` (nouveau).
- `Source/AiSolver/CMakeLists.txt` (nouveaux fichiers).
- `Documentation/Lot-Annexe/LOT-ANNEXE-13-acteur-critique/epic.md` (résultat chiffré consigné une
  fois la comparaison exécutée).

## Tests (obligatoires)
- **Lecture correcte des CSV des deux algorithmes** : `ConvergenceComparator` lit sans erreur des
  fichiers produits par `ReinforceTrainer` et par `ActorCriticTrainer`, malgré leurs colonnes
  légèrement différentes (perte du critique absente côté REINFORCE brut).
- **Calcul du nombre d'épisodes jusqu'au plafond** : sur des CSV synthétiques construits pour le
  test (courbes de récompense connues à la main), le nombre d'épisodes calculé correspond exactement
  à la première atteinte du seuil.
- **Calcul de l'écart-type de fin de run** : vérifié sur des CSV synthétiques à variance connue.
- **Cas où le plafond n'est jamais atteint** : le comparateur ne plante pas, renvoie un résultat
  explicite (ex. valeur sentinelle ou indicateur dédié) plutôt qu'un nombre d'épisodes erroné.

## Points d'attention
- **Plusieurs essais par algorithme, pas un seul** : une seule graine de chaque côté ne permettrait
  de comparer que deux tirages, pas deux distributions — la variance inter-essais est justement une
  partie de ce qui est comparé (stabilité), un seul essai la rendrait invisible.
- **Même seuil de plafond de récompense pour les deux algorithmes** : un seuil différent biaiserait
  la comparaison du nombre d'épisodes nécessaires ; le fixer une fois, en referençant la même
  configuration de récompense (LOT-ANNEXE-08) que celle du niveau de contrôle.
- Le résultat chiffré est une **mesure**, pas une garantie contractuelle : si l'acteur-critique ne
  montre pas d'amélioration nette sur ce niveau de contrôle précis, cela reste une donnée utile à
  documenter honnêtement plutôt qu'à ajuster jusqu'à obtenir le résultat espéré — la décision de
  LOT-ANNEXE-14 doit pouvoir s'appuyer sur une mesure sincère.

## Définition de fait (DoD)
- `ConvergenceComparator` disponible et testé (`ctest` vert) ; comparaison exécutée au moins une
  fois et son résultat consigné dans `epic.md` ; build `/W4 /WX` sans avertissement ; Doxygen à
  jour ; `EX-IA-014` déclarée comme couverte, mesure de convergence à l'appui.

## Notions abordées
@ref guide-annexe-acteur-critique — variance du gradient, fonction de valeur, critique, avantage.

## Exigences
`EX-IA-014` (nouvelle, mesurée par ce lot de comparaison).
