# TACHE-03 — Moyenne mobile et delta (détection de plateau) {#lot-annexe-09-tache-03-moyenne-mobile-delta}

**Lot :** [LOT-ANNEXE-09](epic.md) · **Emplacement :** `Source/AiSolver/Stats` · **Statut :** à faire

## Contexte
C'est la tâche qui répond le plus directement à la demande explicite de l'utilisateur : pouvoir
savoir, en ouvrant le CSV, si l'entraînement a atteint son plafond — sans recalculer quoi que ce
soit dans un tableur. TACHE-01/02 posent l'interface et le format ; cette tâche calcule les deux
colonnes qui rendent un plateau visible directement.

## Travail à réaliser
- **`aisolver::MovingAverageTracker`** (`Source/AiSolver/Stats/MovingAverage.h/.cpp`) : fenêtre
  glissante de taille configurable (`N` lignes, ex. `20`) sur `bestReward` (ou `meanReward`, à
  documenter comme choix par défaut — la meilleure récompense de chaque génération/épisode reflète
  mieux un plafond que la moyenne, plus bruitée) ; `float push(float value)` retourne la nouvelle
  moyenne mobile après ajout de `value`.
- **Delta** : différence entre la moyenne mobile courante et celle d'il y a `N` lignes (ou, plus
  simplement, entre la moyenne mobile courante et la précédente) — proche de `0` signale un
  plateau, documenté explicitement dans le Doxygen du fichier.
- **`TrainingStatsRecorder::record`** (TACHE-01) intègre un `MovingAverageTracker` interne, calcule
  `movingAverageReward`/`rewardDelta` à chaque appel et les passe à `csvRow` (TACHE-02).

## Fichiers impactés
- `Source/AiSolver/Stats/MovingAverage.h/.cpp` — nouveau.
- `Source/AiSolver/Stats/TrainingStatsRecorder.cpp` — modifié (intègre le tracker).
- `Source/AiSolver/CMakeLists.txt` — ajout des nouveaux fichiers.

## Tests (obligatoires)
- **Moyenne mobile correcte** sur une séquence de valeurs connues, avant et après que la fenêtre
  soit pleine (comportement documenté pour les `N` premières lignes, où la fenêtre n'est pas encore
  complète — moyenne sur les valeurs disponibles, pas une valeur sentinelle).
- **Delta proche de zéro sur un plateau synthétique** : une séquence de valeurs constantes après un
  certain point produit un delta qui tend vers `0`, vérifié explicitement (scénario direct du besoin
  exprimé par l'utilisateur).
- **Delta significativement positif sur une séquence strictement croissante** : distingue un
  entraînement qui progresse encore d'un plateau.

## Points d'attention
- **La taille de fenêtre (`N`) est un paramètre documenté, pas une constante magique enfouie** —
  ajustable par l'appelant selon la volatilité attendue de l'algorithme (l'évolutionniste, par
  génération, et le policy gradient, par épisode, n'ont pas la même échelle de bruit d'une ligne à
  l'autre).
- **Ce lot ne décide jamais qu'un plateau est atteint** (pas de seuil d'arrêt automatique) —
  seulement la donnée qui permet à un humain de le constater, cohérent avec la décision de cadrage
  de l'épic.

## Définition de fait (DoD)
- `MovingAverageTracker` disponible et testé (`ctest` vert), intégré à `TrainingStatsRecorder` ;
  build `/W4 /WX` sans avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-apprentissage-renforcement — récompense cumulée (retour), plafond de performance
d'un entraînement.

## Exigences
`EX-IA-010` (nouvelle, partagée avec TACHE-01/02/04/05 du même lot).
