# TACHE-03 — Tests de convergence sur fonctions jouets (bloquant) {#lot-annexe-04-tache-03-tests-convergence}

**Lot :** [LOT-ANNEXE-04](epic.md) · **Emplacement :** `Source/Test/Unit/AiSolver/Optim` · **Statut :** à faire

## Contexte
`LOT-ANNEXE-02` impose le gradient checking comme condition bloquante avant qu'une opération
différentiable soit consommée en aval ; ce lot applique le même principe à l'étage au-dessus : un
optimiseur qui ne fait pas converger un problème **connu et trivial** ne doit jamais être laissé
entraîner un agent réel (génération 3), où un échec de convergence serait indiscernable du bruit
inhérent à l'apprentissage par renforcement lui-même. Cette tâche consolide, en une suite dédiée, les
cas de convergence déjà exercés unitairement par TACHE-01/02 et y ajoute la comparaison croisée entre
les deux optimiseurs sur des cas partagés.

## Travail à réaliser
- **Problèmes jouets communs** (`Source/Test/Unit/AiSolver/Optim/ToyProblems.h`, utilitaire de
  test partagé, pas dupliqué entre suites) : une quadratique convexe à minimum connu (`f(x) = (x -
  cible)²`, gradient `2(x - cible)`) et une régression polynomiale à coefficients connus (données
  synthétiques générées avec `aisolver::Rng`, `LOT-ANNEXE-01`, graine fixe).
- **Suite de tests dédiée** (`Source/Test/Unit/AiSolver/Optim/test_convergence.cpp`) : exécute
  `Sgd` (avec/sans inertie) et `Adam` sur chaque problème jouet, avec les mêmes conditions
  initiales (graine `Rng` fixe), et vérifie la convergence à une tolérance documentée en un nombre
  d'itérations borné.
- **Comparaison croisée** : sur le même problème et avec le **même** taux d'apprentissage choisi
  pour faire diverger ou stagner `Sgd` sans inertie, vérifie qu'`Adam` converge — matérialise
  explicitement la décision de cadrage de `LOT-ANNEXE-04` sur l'intérêt pratique d'Adam.

## Fichiers impactés
- `Source/Test/Unit/AiSolver/Optim/ToyProblems.h` — nouveau.
- `Source/Test/Unit/AiSolver/Optim/test_convergence.cpp` — nouveau.
- `Source/Test/CMakeLists.txt` — ajout des nouveaux fichiers à la cible `UnitTests`.

## Tests (obligatoires)
- **`Sgd` sans inertie converge** sur la quadratique convexe, tolérance documentée, nombre
  d'itérations borné.
- **`Sgd` avec inertie converge plus vite** que sans inertie sur le même cas (comparaison directe
  du nombre d'itérations nécessaires).
- **`Adam` converge** sur la régression polynomiale à coefficients connus, tolérance documentée.
- **`Adam` converge là où `Sgd` sans inertie diverge/stagne**, à taux d'apprentissage identique et
  choisi précisément pour ce contraste.
- **Reproductibilité** : deux exécutions de la même suite, même graine `Rng`, produisent une
  trajectoire de convergence identique (même nombre d'itérations, mêmes valeurs intermédiaires).

## Points d'attention
- **Cette suite est bloquante avant tout usage des optimiseurs par la génération 3** : un échec ici
  doit interrompre l'intégration continue avant que quiconque ne commence un entraînement réel sur
  un niveau du jeu — documenté explicitement en commentaire en tête de fichier, à l'image de
  `GradientCheck.h` (`LOT-ANNEXE-02`).
- **Les problèmes jouets restent volontairement triviaux** (convexes, à solution connue en forme
  close) : le but n'est pas de tester la qualité de l'apprentissage par renforcement (hors de
  portée d'un test unitaire déterministe), seulement l'exactitude mécanique de la règle de mise à
  jour elle-même.

## Définition de fait (DoD)
- Suite `test_convergence.cpp` verte pour les deux optimiseurs sur les deux problèmes jouets ;
  build `/W4 /WX` sans avertissement ; `EX-IA-004` déclarée dans l'`epic.md` du lot.

## Notions abordées
@ref guide-annexe-optimisation — descente de gradient, taux d'apprentissage, inertie, Adam.

## Exigences
`EX-IA-004` (nouvelle, partagée avec TACHE-01/02 du même lot).
