# TACHE-03 — Budget de pas et mesure de progression (blocage) {#lot-annexe-05-tache-03-budget-pas-detection-blocage}

**Lot :** [LOT-ANNEXE-05](epic.md) · **Emplacement :** `Source/AiSolver/Env` · **Statut :** à faire

## Contexte
`playLevel()` borne sa boucle par un `maxSteps = 3000` codé en dur, suffisant pour un test unitaire
(un scénario scripté termine toujours). Un run d'entraînement automatisé (générations 2/3) n'a pas
cette garantie : une politique aléatoire ou mal entraînée peut faire du sur-place indéfiniment
(walk-in-place contre un mur, oscillation). Sans borne, un seul épisode dégénéré bloquerait tout un
batch. Cette tâche ajoute au `HeadlessLevelEnvironment` de TACHE-01/02 un budget de pas dur et des
compteurs de progression **exposés**, sans que l'environnement décide lui-même d'une fin d'épisode
sur blocage (cette politique appartient à LOT-ANNEXE-08).

## Travail à réaliser
- **`aisolver::EnvironmentConfig`** (`Source/AiSolver/Env/HeadlessLevelEnvironment.h`) : struct de
  configuration passée à la construction ou à `reset` — `int maxSteps = 3000;` (même valeur par
  défaut que `playLevel()`, un budget dur : `step` refuse un appel au-delà via
  `JADG_ASSERT`, symétrique de l'assertion « pas de `step` avant `reset` » de TACHE-01) ;
  `float progressEpsilon = 0.05f;` (unités monde, seuil en dessous duquel un déplacement n'est pas
  considéré comme un progrès).
- **Mesure de progression** : à chaque `step`, calcule la distance euclidienne entre le centre de la
  boîte du personnage et le centre de la case de sortie (`level().exit()`) ; conserve
  `_bestDistanceToExit` (minimum observé depuis le dernier `reset`) et `_stepsSinceProgress` (remis à
  zéro si la distance courante améliore `_bestDistanceToExit` de plus de `progressEpsilon`,
  incrémenté sinon).
- **Accesseurs** : `[[nodiscard]] bool budgetExhausted() const noexcept;` (`stepIndex >= maxSteps`),
  `[[nodiscard]] int stepsSinceProgress() const noexcept;`, `[[nodiscard]] float bestDistanceToExit()
  const noexcept;` — lus par l'appelant (boucle d'entraînement ou LOT-ANNEXE-08), jamais interprétés
  ici en une issue `Lost`/`Won` supplémentaire (`core::LevelOutcome` n'est pas étendu : rester dans
  le vocabulaire de `Core`, ne pas y ajouter un pseudo-état côté `AiSolver`).

## Fichiers impactés
- `Source/AiSolver/Env/HeadlessLevelEnvironment.h` (`EnvironmentConfig`, accesseurs, membres privés
  `_bestDistanceToExit`/`_stepsSinceProgress`).
- `Source/AiSolver/Env/HeadlessLevelEnvironment.cpp` (calcul de progression dans `step`, remise à
  zéro dans `reset`).
- Tests : `Source/Test/Unit/AiSolver/Env/test_headless_level_environment.cpp` (complété).

## Tests (obligatoires)
- **Budget atteint** : après `maxSteps` appels à `step` sur un niveau où le personnage reste
  immobile (`PlayerInput{}` par défaut), `budgetExhausted()` devient vrai exactement au pas
  `maxSteps`, jamais avant.
- **Appel au-delà du budget** : appeler `step` une fois `budgetExhausted()` vrai déclenche
  l'assertion de programmation (`JADG_ASSERT`), pas un comportement silencieux — documente
  que c'est à l'appelant de s'arrêter.
- **Progression détectée** : sur `demo-deplacement.json` avec `moveX = 1.0f` constant,
  `stepsSinceProgress()` reste à `0` (ou proche) tant que le personnage avance vers la sortie.
- **Blocage détecté** : sur le même niveau avec `PlayerInput{}` (immobile) contre un mur ou au sol
  sans déplacement, `stepsSinceProgress()` croît de façon monotone, sans jamais se réinitialiser.
- **`reset` remet les compteurs à zéro** : `bestDistanceToExit()`/`stepsSinceProgress()` reviennent à
  leur état initial après un nouveau `reset`, y compris sur le même niveau qu'un run précédent.

## Points d'attention
- **La distance à la sortie est une heuristique de progression, pas une garantie de solvabilité.**
  Un niveau où le chemin s'éloigne temporairement de la sortie (détour obligé, `demo-salles.json`)
  ferait croître `stepsSinceProgress` sans qu'il y ait de blocage réel — c'est un signal brut pour
  l'algorithme consommateur (LOT-ANNEXE-08 choisit la fenêtre de tolérance), pas une vérité absolue.
- **`maxSteps` est un budget dur de sécurité, pas un réglage d'entraînement.** Sa valeur par défaut
  (3000, alignée sur `playLevel()`) n'a pas vocation à être le seuil utilisé pour détecter un
  blocage en pratique (bien plus court, voir LOT-ANNEXE-08) — seulement à empêcher un run
  pathologique de tourner indéfiniment.
- **Pas de nouvel état dans `core::LevelOutcome`.** Céder à la tentation d'ajouter un `TimedOut` ou
  `Stalled` dans `Core` violerait la contrainte « aucune ligne de `Core` modifiée » pour un besoin
  qui n'existe que côté entraînement.

## Définition de fait (DoD)
- Budget de pas et compteurs de progression disponibles et testés (`ctest` vert) ; build `/W4 /WX`
  sans avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-apprentissage-renforcement — agent, environnement, boucle `reset`/`step`, épisode,
propriété de Markov.

## Exigences
`EX-IA-005` (nouvelle).
