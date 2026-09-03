# TACHE-01 — Population de réseaux {#lot-annexe-10-tache-01-population-reseaux}

**Lot :** [LOT-ANNEXE-10](epic.md) · **Emplacement :** `Source/AiSolver/Training/Evolutionary` ·
**Statut :** fait

## Contexte
LOT-ANNEXE-03 fournit une bibliothèque de réseaux de neurones maison (couches, propagation avant,
sans autodiff) et LOT-ANNEXE-01 un `Tensor` et un `Rng` déterministe à seed explicite. Aucun de ces
deux lots ne sait ce qu'est une « génération » ou un « individu » : cette tâche introduit ces
notions et le conteneur de population qui portera tout l'algorithme évolutionniste.

## Travail à réaliser
- **`aisolver::training::evolutionary::Individual`** : porte une instance du réseau de
  LOT-ANNEXE-03 (poids propres, pas de partage) et un champ `float fitness` (valeur par défaut
  représentant « non évalué », ex. `-std::numeric_limits<float>::infinity()`). Un individu est une
  politique **pure** : il ne référence jamais un environnement ni un état d'exécution — seuls les
  poids et le dernier fitness connu.
- **`aisolver::training::evolutionary::Population`** : conteneur de taille fixe `N` (constante de
  configuration, pas de croissance/réduction dynamique), construit à partir d'un descripteur de
  topologie (tailles de couches, fournies par LOT-ANNEXE-03) et d'une référence à un `Rng`.
- **Initialisation des poids** : chaque individu reçoit une **copie indépendante** de la topologie,
  ses poids tirés via `Rng` (distribution documentée par LOT-ANNEXE-01, ex. uniforme sur un
  intervalle symétrique). Tous les individus partagent exactement la même topologie — seuls les
  poids diffèrent ; aucune évolution de structure dans ce lot (décision de cadrage de l'épic).
- Constante `DEFAULT_POPULATION_SIZE` (valeur documentée par un commentaire justifiant le choix,
  ex. compromis diversité/coût d'évaluation par génération) dans le namespace anonyme du `.cpp`.
- Mise à jour du `CMakeLists.txt` du module `AiSolver` : nouveaux fichiers `Individual.h`,
  `Population.h`/`.cpp` ajoutés à la cible.

## Fichiers impactés
- `Source/AiSolver/Training/Evolutionary/Individual.h` (nouveau).
- `Source/AiSolver/Training/Evolutionary/Population.h`/`.cpp` (nouveaux).
- `Source/AiSolver/CMakeLists.txt`.
- Tests : `Source/Test/Unit/AiSolver/Training/test_population.cpp` (nouveau).

## Tests (obligatoires)
- **Taille de population correcte** : une `Population` construite avec `N` individus en contient
  exactement `N`, chacun avec un réseau de la topologie attendue.
- **Poids indépendants** : deux individus de la même population n'ont jamais exactement les mêmes
  poids (probabilité quasi nulle avec une initialisation continue) et modifier les poids de l'un ne
  modifie pas l'autre (copie profonde vérifiée).
- **Reproductibilité de l'initialisation** : à seed `Rng` fixée, deux populations construites avec
  les mêmes paramètres ont des poids initiaux strictement identiques, individu par individu.
- **Topologie identique** : tous les individus d'une population exposent la même structure de
  réseau (nombre de couches, tailles) — seuls les poids varient.
- **Fitness initial** : chaque individu est créé avec un fitness marqué « non évalué » (pas de
  valeur arbitraire pouvant être confondue avec un vrai score).

## Points d'attention
- **Copie profonde obligatoire** : partager les poids entre individus (ex. par erreur de
  conception avec des pointeurs partagés) contaminerait un individu par la mutation d'un autre —
  invariant central de tout algorithme évolutionniste, à vérifier explicitement par test.
- **Taille de population fixée à la construction** : simplifie la boucle de génération
  (TACHE-04), qui peut réutiliser le même buffer d'une génération à l'autre sans réallocation.
- **`Individual` ne connaît pas l'environnement** : l'évaluation (TACHE-02) est un aller simple
  (réseau + niveau → fitness), jamais l'inverse — un individu reste utilisable sur n'importe quel
  niveau, seule l'évaluation en dépend.

## Définition de fait (DoD)
- `Individual`/`Population` disponibles et testés (`ctest` vert) ; build `/W4 /WX` sans
  avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-algorithmes-evolutionnistes — population, fitness, sélection, croisement,
mutation.

## Exigences
Aucune exigence propre — contribue à `EX-IA-011` (déclarée en TACHE-04).
