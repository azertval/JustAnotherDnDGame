# LOT-ANNEXE-04 — Optimiseurs maison {#lot-annexe-04}

> Statut : **fait**. Prérequis : [LOT-ANNEXE-02](@ref lot-annexe-02) (autodiff, source des
> gradients) et [LOT-ANNEXE-03](@ref lot-annexe-03) (réseaux, source des paramètres à optimiser).
> Dernier lot de la génération 0 : ferme la chaîne de calcul qui rend l'apprentissage par gradient
> (génération 3) possible.

## Objectif
`LOT-ANNEXE-02` calcule un gradient par nœud (`autodiff::Node::grad()`), `LOT-ANNEXE-03` expose les
paramètres d'un réseau (`nn::Network::parameters()`), mais rien ne relie encore les deux : un
gradient calculé et jamais appliqué ne change rien. Ce lot écrit ce dernier maillon — les règles de
mise à jour des poids à partir de leur gradient accumulé — sans lequel aucun algorithme de la
génération 3 (REINFORCE, acteur-critique, algorithme avancé) ne pourrait effectivement apprendre.
Deux règles suffisent à couvrir les besoins identifiés : SGD (avec/sans inertie), la plus simple et
la référence de calibration, et Adam, la plus robuste au choix du taux d'apprentissage — le choix
entre les deux reste un paramètre d'expérience de chaque lot de génération 3, pas une décision figée
ici.

## Périmètre

### Inclus
- **`aisolver::optim::Sgd`** : mise à jour `poids -= tauxApprentissage × gradient`, avec un terme
  d'inertie (*momentum*) optionnel et configurable (`vitesse = momentum × vitesse - taux × gradient
  ; poids += vitesse`).
- **`aisolver::optim::Adam`** : moments d'ordre 1 et 2 (moyennes mobiles exponentielles du gradient
  et de son carré), correction de biais d'initialisation, mise à jour normalisée par la racine du
  second moment.
- **Interface commune** (`aisolver::optim::IOptimizer`) : `step(paramètres)` applique une mise à
  jour à partir des gradients déjà accumulés sur chaque `autodiff::Node` ; `zeroGrad(paramètres)`
  remet les gradients à zéro entre deux passes — nécessaire puisque `LOT-ANNEXE-02` accumule
  (`+=`) plutôt que d'écraser.
- **Tests de convergence sur fonctions jouets** (ex. régression sur un polynôme connu, minimisation
  d'une quadratique à minimum connu) — condition **bloquante** avant tout usage par la génération 3,
  au même titre que le gradient checking de `LOT-ANNEXE-02`.

### Exclus (hors périmètre de ce lot)
- **Autres optimiseurs** (RMSProp, Adagrad, variantes à taux d'apprentissage adaptatif par
  paramètre autres qu'Adam) : SGD et Adam couvrent l'éventail usuel (référence simple / référence
  robuste) sans qu'aucun besoin supplémentaire ne soit identifié par la génération 3.
- **Planification du taux d'apprentissage** (*learning rate scheduling*, décroissance au fil de
  l'entraînement) : un taux constant suffit tant qu'aucune instabilité d'entraînement réelle ne l'a
  justifié — à réévaluer, si besoin, à la lumière des résultats de `LOT-ANNEXE-12`/`13`.
- **Écrêtage de gradient** (*gradient clipping*) : aucune divergence numérique observée avant tout
  entraînement réel (la génération 3 n'a pas encore tourné) ; à introduire seulement si les tests de
  `LOT-ANNEXE-12` révèlent un besoin concret, pas de façon préventive.
- **Optimisation multi-paramètres par groupes** (taux différent par couche) : chaque paramètre
  reçoit le même taux d'apprentissage de l'optimiseur qui le porte — suffisant pour des réseaux de
  la taille visée (`LOT-ANNEXE-03`, couches denses peu nombreuses).

## Décisions de cadrage
- **L'optimiseur ne connaît que des `autodiff::Node` porteurs de gradient, jamais le réseau ou
  l'algorithme d'apprentissage qui les a produits.** `step(parameters)` prend en entrée exactement
  ce que `nn::Network::parameters()` expose (`LOT-ANNEXE-03`) — aucun couplage à `PolicyGradient`,
  `Evolutionary` ou tout autre module de génération 2/3, cohérent avec le principe déjà appliqué à
  `Network` (bibliothèque de calcul pure, sans logique d'entraînement spécifique au jeu).
- **`zeroGrad` est explicite, jamais implicite dans `step`.** Séparer les deux évite qu'un appelant
  qui a besoin d'accumuler des gradients sur plusieurs passes avant (ex. plusieurs épisodes avant
  une mise à jour, cas potentiel de `LOT-ANNEXE-14`) soit forcé de contourner l'optimiseur — la
  responsabilité de remettre à zéro reste à l'appelant, à l'endroit qui a du sens pour son
  algorithme.
- **État interne de l'optimiseur (vitesse pour SGD-momentum, moments pour Adam) indexé par
  identité de paramètre, pas recalculé à chaque `step`.** Un optimiseur Adam vit aussi longtemps
  que l'entraînement qu'il sert ; son état (moments, compteur de pas pour la correction de biais)
  persiste d'un appel à l'autre — sans quoi Adam dégénérerait en SGD à chaque étape.
- **Aucun optimiseur ne modifie le graphe d'autodiff.** `step` lit `Node::grad()` et écrit
  directement dans le tenseur de valeur du paramètre (`Node::value()`), en dehors de toute
  construction de graphe — l'optimiseur n'est jamais différentié lui-même, cohérent avec le
  périmètre de `LOT-ANNEXE-02` (dérivées du premier ordre uniquement).
- **Les tests de convergence sur fonctions jouets sont bloquants avant tout usage par la
  génération 3**, exactement comme le gradient checking de `LOT-ANNEXE-02` : un optimiseur qui ne
  fait pas converger une régression polynomiale connue ne doit pas être laissé entraîner un agent
  réel — le signal d'échec serait alors noyé dans le bruit inhérent à l'apprentissage par renforcement.

## Notions abordées
Voir @ref guide-annexe-optimisation (descente de gradient, taux d'apprentissage, inertie, Adam).
Sources directes : Robbins & Monro (1951, approximation stochastique, fondement de SGD) ; Kingma &
Ba (2015, article d'origine d'Adam) ; Ruder (2016, synthèse comparative) — bibliographie complète
dans le chapitre.

## Exigences couvertes
- Nouvelle, déclarée dans [la spécification IA](@ref spec-ia) : [`EX-IA-004`](@ref EX-IA-004).
- Réutilisées : `EX-IA-002` (moteur d'autodiff, source des gradients), `EX-IA-003` (réseaux de
  neurones, source des paramètres), `EX-NFR-010`/`EX-NFR-012`/`EX-NFR-013`/`EX-NFR-020`
  (testabilité headless, conventions, `/W4 /WX`, couverture de tests), `EX-ARCH-001` (sens de
  dépendance `AiSolver → Core`, jamais l'inverse).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-sgd.md) | SGD (avec/sans inertie) | `Source/AiSolver/Optim` | ✅ |
| [TACHE-02](tache-02-adam.md) | Adam (moments d'ordre 1/2, correction de biais) | `Source/AiSolver/Optim` | ✅ |
| [TACHE-03](tache-03-tests-convergence.md) | Tests de convergence sur fonctions jouets (bloquant) | `Source/Test/Unit/AiSolver/Optim` | ✅ |

## Critères d'acceptation du lot
1. `Sgd` (sans inertie) minimise une fonction quadratique convexe connue jusqu'à une tolérance
   documentée, en un nombre d'itérations cohérent avec le taux d'apprentissage choisi.
2. `Sgd` avec inertie converge sur le même cas en moins d'itérations que sans inertie (comparaison
   explicite en test), démontrant l'effet attendu du terme de momentum.
3. `Adam` converge sur une régression polynomiale connue à une tolérance documentée, avec un taux
   d'apprentissage identique à celui qui ferait diverger ou stagner `Sgd` sans inertie sur le même
   cas (démontre l'intérêt pratique d'Adam, pas seulement sa correction).
4. `zeroGrad` remet effectivement à zéro les gradients de tous les paramètres fournis, vérifié par
   test dédié distinct des tests de convergence.
5. Logique nouvelle **couverte par des tests** (`ctest` vert), déterministe, sans GPU. Build
   `/W4 /WX` sans avertissement, Doxygen et `scripts/lint_exigences.py` verts.

## Dépendances
Bâtit sur [LOT-ANNEXE-02](@ref lot-annexe-02) (`autodiff::Node::grad()`) et
[LOT-ANNEXE-03](@ref lot-annexe-03) (`nn::Network::parameters()`). Dernier lot de la génération 0 :
[LOT-ANNEXE-12](@ref lot-annexe-12), [LOT-ANNEXE-13](@ref lot-annexe-13) et
[LOT-ANNEXE-14](@ref lot-annexe-14) (génération 3, apprentissage par gradient) en dépendent
directement pour la mise à jour de leurs réseaux.

## Navigation des tâches
- @subpage lot-annexe-04-tache-01-sgd
- @subpage lot-annexe-04-tache-02-adam
- @subpage lot-annexe-04-tache-03-tests-convergence
