# TACHE-03 — Test de robustesse au bruit d'observation {#lot-annexe-15-tache-03-robustesse-bruit}

**Lot :** [LOT-ANNEXE-15](epic.md) · **Emplacement :** `Source/AiSolver/Eval` · **Statut :** fait

## Contexte
TACHE-01/02 mesurent la performance brute d'un modèle sur son niveau d'origine, dans les mêmes
conditions exactes qu'à l'entraînement. Un modèle en surapprentissage peut afficher un taux de
réussite excellent dans ces conditions tout en n'ayant appris qu'une trajectoire précise plutôt que
des réflexes robustes. Cette tâche introduit une perturbation légère et contrôlée de l'observation
perçue par la politique, pour révéler cet écart.

## Travail à réaliser
- **`aisolver::NoisyObservationWrapper`** (`Source/AiSolver/Eval/NoisyObservation.h/.cpp`) : décore
  l'encodeur d'observation de `LOT-ANNEXE-06` — applique un bruit gaussien de faible amplitude
  (configurable, via `aisolver::Rng` dérivé d'une graine de base + indice de répétition, cohérence
  avec la décision de cadrage de l'épic) sur le tenseur d'observation **avant** qu'il soit transmis
  à la politique, sans jamais toucher l'état réel simulé par `HeadlessLevelEnvironment`
  (`LOT-ANNEXE-05`) — l'issue (`core::evaluateOutcome`) reste jugée sur l'état réel non perturbé.
- **`BenchmarkResult runWithNoise(const TrainedPolicy&, std::string_view levelPath, int
  repetitions, float noiseAmplitude, uint64_t baseSeed)`** (réutilise TACHE-01) : exécute les mêmes
  répétitions que `run()` (TACHE-01) mais à travers `NoisyObservationWrapper`, produit un
  `BenchmarkResult` comparable directement (même structure) à celui obtenu sans bruit.
- Comparaison documentée : l'écart de taux de réussite entre `run()` et `runWithNoise()` sur le
  même modèle est le signal de robustesse recherché.

## Fichiers impactés
- `Source/AiSolver/Eval/NoisyObservation.h/.cpp` — nouveau.
- `Source/AiSolver/Eval/BenchmarkRunner.h/.cpp` — modifié (ajout de `runWithNoise`, TACHE-01).
- `Source/AiSolver/CMakeLists.txt` — ajout des nouveaux fichiers.

## Tests (obligatoires)
- **Le bruit n'affecte jamais l'état réel simulé** : pour une même graine, la trajectoire réelle
  (positions successives dans `HeadlessLevelEnvironment`) est identique entre `run()` et
  `runWithNoise()`, seule l'observation transmise à la politique diffère — vérifié explicitement
  (pas seulement déduit du taux de réussite final).
- **Amplitude nulle == comportement de `run()` sans bruit** : `runWithNoise(..., noiseAmplitude =
  0.0f, ...)` produit des résultats identiques à `run()` (cas limite de cohérence).
- **Modèle évolutionniste (décodage `Argmax` uniquement)** : le bruit peut faire basculer une
  décision `argmax` d'une action à une autre sur une observation proche d'une frontière de décision
  — contrairement au cas sans bruit (TACHE-01, variance nulle attendue), la variance sous bruit peut
  redevenir non nulle, vérifié comme comportement attendu, pas comme une anomalie.
- **Reproductibilité** : deux exécutions de `runWithNoise` avec la même graine de base produisent
  les mêmes résultats (même tirage de bruit à chaque répétition).

## Points d'attention
- **`NoisyObservationWrapper` ne modifie aucun fichier de `LOT-ANNEXE-06`** : c'est un décorateur
  qui s'interpose entre l'encodeur existant et la politique, sans toucher à `Source/AiSolver/Env`
  (cohérent avec le critère d'acceptation 5 de l'épic — aucune ligne de `Training/*` ni de `Env`
  modifiée par la génération 4).
- **L'amplitude du bruit reste un paramètre documenté et borné** (pas de recherche adversariale
  d'amplitude qui ferait échouer la politique à tout prix — décision de cadrage de l'épic).

## Définition de fait (DoD)
- `NoisyObservationWrapper`/`runWithNoise` disponibles et testés (`ctest` vert) ; build `/W4 /WX`
  sans avertissement ; Doxygen à jour ; `EX-IA-016` déclarée dans l'`epic.md` du lot.

## Notions abordées
@ref guide-annexe-evaluation-rl — variance entre exécutions, graines multiples, mesure honnête d'un
agent entraîné.

## Exigences
`EX-IA-016` (nouvelle, partagée avec TACHE-01/02 du même lot).
