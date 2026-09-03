# TACHE-02 — Détection de fin d'épisode {#lot-annexe-08-tache-02-detection-fin-episode}

**Lot :** [LOT-ANNEXE-08](epic.md) · **Emplacement :** `Source/AiSolver/Env` · **Statut :** à faire

## Contexte
`core::LevelOutcome` (`Playing`/`Won`/`Lost`) suffit au jeu (qui relance immédiatement sur `Lost`,
`Source/HMI/Game/GameSession.cpp`), mais un entraînement automatisé (génération 2/3) a besoin de
deux cas supplémentaires que le jeu ne connaît pas : un blocage (l'agent ne meurt pas mais ne
progresse plus) et un plafond dur de pas — sans quoi un épisode pourrait durer indéfiniment.

## Travail à réaliser
- **`enum class aisolver::EpisodeStatus { Ongoing, Won, Lost, TimedOut, Stuck };`** (`Source/
  AiSolver/Env/Episode.h`).
- **`EpisodeStatus classifyEpisode(core::LevelOutcome outcome, int stepIndex, int
  stepsSinceProgress, int hardStepBudget, int stuckThreshold)`** (`Source/AiSolver/Env/Episode.cpp`) :
  `outcome == Won` → `Won` ; `outcome == Lost` → `Lost` ; sinon, si `stepIndex >= hardStepBudget` →
  `TimedOut` ; sinon, si `stepsSinceProgress >= stuckThreshold` → `Stuck` ; sinon → `Ongoing`. Ordre
  de priorité explicite : victoire/défaite réelles priment toujours sur les critères artificiels
  (`TimedOut`/`Stuck`).
- Réutilise directement `stepIndex`/`stepsSinceProgress` déjà exposés par `HeadlessLevelEnvironment`
  (`LOT-ANNEXE-05`, TACHE-03) — aucune duplication de compteur.

## Fichiers impactés
- `Source/AiSolver/Env/Episode.h/.cpp` — nouveau.
- `Source/AiSolver/CMakeLists.txt` — ajout des nouveaux fichiers.

## Tests (obligatoires)
- **Priorité de `Won`/`Lost`** : même avec `stepIndex >= hardStepBudget` ou `stepsSinceProgress >=
  stuckThreshold`, un `outcome` réel (`Won`/`Lost`) domine toujours.
- **`TimedOut` déclenché exactement au plafond**, pas avant, pas après.
- **`Stuck` déclenché exactement au seuil de stagnation**, indépendamment de `hardStepBudget` (un
  agent qui stagne bien avant le plafond dur est signalé `Stuck`, pas laissé continuer jusqu'au
  plafond).
- **`Ongoing`** dans tous les autres cas (progression en cours, sous les deux plafonds).

## Points d'attention
- **`Stuck` et `TimedOut` sont deux critères indépendants, jamais l'un substitué à l'autre** : un
  niveau très long mais progressant lentement doit pouvoir dépasser le seuil de stagnation sans
  jamais le déclencher, tant qu'il progresse régulièrement, même si cela le rapproche du plafond dur.
- **`classifyEpisode` est une fonction pure**, sans accès direct à `HeadlessLevelEnvironment` —
  reçoit ses compteurs en paramètres, testable sans construire d'environnement réel.

## Définition de fait (DoD)
- `EpisodeStatus`/`classifyEpisode` disponibles et testés (`ctest` vert) ; build `/W4 /WX` sans
  avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-apprentissage-renforcement (récompense, conception d'une fonction de récompense
(*shaping*, *reward hacking*), épisode et horizon), en particulier sa section 7 (épisode, horizon,
fins artificielles : plafond de pas et blocage).

## Exigences
`EX-IA-009` (nouvelle, partagée avec TACHE-01/03 du même lot).
