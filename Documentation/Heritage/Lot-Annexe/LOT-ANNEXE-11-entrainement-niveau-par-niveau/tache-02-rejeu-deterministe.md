# TACHE-02 — Rejeu déterministe du meilleur individu {#lot-annexe-11-tache-02-rejeu-deterministe}

**Lot :** [LOT-ANNEXE-11](epic.md) · **Emplacement :** `Source/AiSolver/Training` · **Statut :**
fait

## Contexte
TACHE-01 produit un `TrainingResult` contenant le meilleur individu (poids figés) au moment de
l'arrêt. Cette tâche transforme cette politique en une séquence concrète d'actions, en la faisant
rejouer une dernière fois de façon strictement déterministe — c'est cette séquence, et non le
réseau lui-même, qui sera exportée (TACHE-03) et qui pourrait un jour être rejouée en jeu.

## Travail à réaliser
- **`aisolver::training::replayBestIndividual(const Individual&, HeadlessLevelEnvironment&) ->
  ActionSequence`** (type `ActionSequence` fourni par LOT-ANNEXE-07/`Source/AiSolver/Replay`) :
  réinitialise l'environnement, boucle jusqu'à fin d'épisode (LOT-ANNEXE-08), à chaque pas encode
  l'observation (LOT-ANNEXE-06), effectue la propagation avant du réseau, **décode l'action en
  `argmax`** avec exactement le même code que l'évaluation de fitness de LOT-ANNEXE-10 (pas de
  réimplémentation séparée — décision de cadrage de l'épic), ajoute l'action décodée à la séquence.
- **Vérification de cohérence** : si `TrainingResult::solved == true`, le rejeu doit se terminer par
  un succès — mêmes poids, même niveau, même décodage déterministe ⇒ même trajectoire que celle
  observée pendant l'entraînement. Une incohérence ici (rejeu échoué alors que l'entraînement a
  déclaré une résolution) signale un bug de divergence entre les deux chemins de code, pas un aléa
  à masquer.
- **Cas `solved == false`** : le rejeu est produit quand même (comportement du meilleur individu
  trouvé, utile en diagnostic) mais TACHE-03 ne doit jamais l'exporter comme solution valide.
- Le réseau de l'individu n'est jamais modifié pendant le rejeu (lecture seule, aucune mutation
  résiduelle, aucun apprentissage en ligne).

## Fichiers impactés
- `Source/AiSolver/Training/DeterministicReplay.h`/`.cpp` (nouveaux).
- Tests : `Source/Test/Unit/AiSolver/Training/test_deterministic_replay.cpp` (nouveau).

## Tests (obligatoires)
- **Rejeu indépendant réussi** : le rejeu d'un individu connu pour résoudre un niveau trivial
  produit une séquence d'actions qui, rejouée sur une **nouvelle** instance
  `HeadlessLevelEnvironment` (même niveau), atteint bien la sortie — double vérification
  indépendante de la boucle d'entraînement.
- **Déterminisme du rejeu** : deux rejeux successifs du même individu sur le même niveau produisent
  une séquence d'actions strictement identique.
- **Borne de longueur** : la séquence produite ne dépasse jamais la limite de pas maximum définie
  par LOT-ANNEXE-08 (pas de rejeu infini pour un individu non résolvant).
- **Rejeu d'un individu non résolvant** : produit une séquence exploitable sans exception ni
  plantage, correctement marquée comme non résolue.

## Points d'attention
- **Ce rejeu n'est pas une nouvelle inférence en jeu** : c'est un aller simple hors ligne (headless)
  dont le seul livrable est la séquence d'actions elle-même — cohérent avec la décision transverse
  « aucune inférence live » du programme Lot-Annexe.
- **Même code de décodage que l'évaluation de fitness** (LOT-ANNEXE-10) : réutiliser la fonction
  plutôt que la réécrire élimine par construction tout risque de divergence entre « ce que
  l'algorithme a mesuré » et « ce qui est effectivement exporté ».
- **Aucune mutation du réseau pendant le rejeu** : les poids de l'individu rejoué doivent être
  identiques avant et après l'appel (vérifiable par comparaison, si besoin, dans les tests).

## Définition de fait (DoD)
- Rejeu déterministe disponible, testé (`ctest` vert), cohérent avec l'évaluation de fitness de
  LOT-ANNEXE-10 ; build `/W4 /WX` sans avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-algorithmes-evolutionnistes — boucle générationnelle, élitisme, reproductibilité
d'un entraînement.

## Exigences
Aucune exigence propre — contribue à `EX-IA-012` (déclarée en TACHE-03).
