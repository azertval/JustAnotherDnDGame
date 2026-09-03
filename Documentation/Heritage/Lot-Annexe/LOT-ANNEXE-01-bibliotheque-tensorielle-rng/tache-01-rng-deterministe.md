# TACHE-01 — RNG déterministe {#lot-annexe-01-tache-01-rng-deterministe}

**Lot :** [LOT-ANNEXE-01](epic.md) · **Emplacement :** `Source/AiSolver/Math` · **Statut :** fait

## Contexte
Première tâche du programme annexe : elle crée le module `Source/AiSolver` lui-même (aucun fichier
n'y existe encore). Avant même le conteneur tensoriel (TACHE-02), le programme a besoin d'une source
d'aléatoire déterministe partagée — initialisation des poids (`LOT-ANNEXE-03`), mutation
évolutionniste (génération 2), échantillonnage stochastique de politique (génération 3) — pour que
chaque exécution d'entraînement soit **rejouable à l'identique** à partir de sa seule graine, et
qu'aucun lot ultérieur ne réimplémente sa propre variante.

## Travail à réaliser
- **Squelette du module** : `Source/AiSolver/CMakeLists.txt`, calqué sur `Source/Core/CMakeLists.txt`
  (`add_library(AiSolver STATIC Math/Rng.cpp)`, `target_include_directories(AiSolver PUBLIC
  ${PROJECT_SOURCE_DIR}/Source)`, `target_link_libraries(AiSolver PRIVATE Core project_warnings
  project_options)`). Ajout de `add_subdirectory(AiSolver)` dans `Source/CMakeLists.txt`, après
  `add_subdirectory(HMI)`, avec une ligne de commentaire au même endroit que celles décrivant
  `Core`/`HMI`/`Elements`.
- **`aisolver::Rng`** (`Source/AiSolver/Math/Rng.h`) : classe non gabarit, RAII, membre privé
  `std::mt19937_64 _engine;`.
  - Constructeur `explicit Rng(std::uint64_t seed);` — pas de constructeur par défaut (une graine
    doit toujours être fournie explicitement, jamais dérivée de l'horloge).
  - `float nextFloat();` — flottant uniforme dans `[0, 1)`, obtenu **à la main** à partir de la
    sortie brute de `_engine()` : conserver les 24 bits de poids fort (`_engine() >> 40`) et diviser
    par `2²⁴` (`16777216.0f`), soit exactement le nombre de valeurs représentables dans la mantisse
    d'un `float` — mapping exact, sans arrondi surprenant, et strictement `< 1.0f`.
  - `float nextFloat(float min, float max);` — `min + nextFloat() * (max - min)`, donc dans
    `[min, max)`. `JADG_ASSERT(min < max)`.
  - `float nextGaussian(float mean = 0.0f, float stddev = 1.0f);` — loi normale par la
    **transformation de Box-Muller**, calculée à la main sur **deux** appels à `nextFloat()` :
    `z = std::sqrt(-2.0f * std::log(u1)) * std::cos(2.0f * kPi * u2)`, puis `mean + stddev * z`.
    `u1` est retiré tant qu'il vaut exactement `0.0f` (le logarithme de zéro n'est pas défini ;
    boucle `do { u1 = nextFloat(); } while (u1 == 0.0f);`, qui termine avec probabilité 1).
    Variante polaire non retenue (deux valeurs produites par appel, donc un état à conserver entre
    appels — exactement ce que la classe cherche à éviter) : on jette la seconde valeur (`sin`) et
    on consomme deux tirages uniformes par appel, coût sans importance ici.
  - `int nextInt(int min, int max);` — entier uniforme dans `[min, max]` (bornes **incluses**), par
    **rejet** sur la sortie brute de `_engine()` : soit `range = max - min + 1`, on tire
    `_engine()` jusqu'à obtenir une valeur strictement inférieure au plus grand multiple de `range`
    représentable sur 64 bits, puis on renvoie `min + valeur % range` — le rejet élimine le biais
    (léger mais réel) du modulo simple sur les dernières valeurs de la plage.
    `JADG_ASSERT(min <= max)`.
- **`Source/AiSolver/Math/Rng.cpp`** : implémentation. **Aucune `std::…_distribution` n'est
  utilisée** : la norme C++ ne spécifie pas la suite de valeurs qu'une distribution standard produit
  pour un moteur et une graine donnés (seul `std::mt19937_64` lui-même est spécifié bit à bit).
  S'appuyer dessus rendrait la « reproductibilité à partir de la seule graine » dépendante du
  compilateur et de la version de la bibliothèque standard — inacceptable pour un programme dont
  les rejeux exportés (LOT-ANNEXE-11/17) sont relus et vérifiés en intégration continue
  (LOT-ANNEXE-20). Les quatre méthodes ci-dessus dérivent donc toutes leurs valeurs de `_engine()`
  par des opérations arithmétiques explicites, elles-mêmes portables.

## Fichiers impactés
- `Source/AiSolver/CMakeLists.txt` (nouveau).
- `Source/CMakeLists.txt` (ajout de `add_subdirectory(AiSolver)`).
- `Source/AiSolver/Math/Rng.h` (nouveau).
- `Source/AiSolver/Math/Rng.cpp` (nouveau).
- `Source/Test/CMakeLists.txt` (ajout de `Unit/AiSolver/Math/test_rng.cpp` à `UnitTests`, ajout de
  `AiSolver` à `target_link_libraries(UnitTests PRIVATE …)`).
- `Source/Test/Unit/AiSolver/Math/test_rng.cpp` (nouveau, cf. TACHE-05).

## Tests (obligatoires)
- **Reproductibilité à seed fixée** : deux instances de `Rng` construites avec la **même** graine
  produisent exactement la **même** séquence de sorties, sur les quatre méthodes.
- **Deux graines différentes divergent** : deux instances à graines différentes produisent des
  séquences différentes (test de non-trivialité, pas de statistique poussée).
- **Bornes respectées** : `nextFloat(min, max)` reste dans `[min, max)` et `nextInt(min, max)` dans
  `[min, max]` sur un grand nombre de tirages.
- **`nextGaussian` moyenne/écart-type plausibles** : sur un grand nombre de tirages, la moyenne et
  l'écart-type empiriques restent proches (tolérance large) des paramètres demandés — test
  statistique **faible**, pas une preuve, juste un garde-fou contre une implémentation inversée.
- **Valeurs de référence figées (*golden values*)** : les cinq premières valeurs de `nextFloat()`
  pour la graine `42` sont écrites **en dur** dans le test, comparées à `1e-6` près. C'est le test
  qui donne sa valeur à la décision « aucune `std::…_distribution` » ci-dessus : il échoue si
  l'implémentation change de formule, donc il détecte qu'un rejeu enregistré avant le changement ne
  serait plus reproductible. Les valeurs sont obtenues à la première implémentation puis figées
  (avec un commentaire le disant explicitement, pour qu'une future revue ne les prenne pas pour un
  résultat calculé à la main).

## Points d'attention
- **Pas de graine par défaut basée sur l'horloge** : casserait la reproductibilité qui est la raison
  d'être de cette classe (rejouabilité d'un entraînement, comparaison de deux runs).
- **Le seul état persistant est `_engine`** : aucune valeur intermédiaire n'est mise en cache entre
  deux appels (en particulier, `nextGaussian` ne conserve pas la seconde valeur que Box-Muller
  produirait « gratuitement »). Un état caché supplémentaire signifierait que la suite des valeurs
  dépend de l'ordre d'appel des **méthodes**, pas seulement de la graine — précisément le genre de
  couplage qui rend un entraînement difficile à rejouer.
- **`std::log`/`std::cos` sont portables mais pas identiques au dernier bit d'une plateforme à
  l'autre** : la reproductibilité garantie ici est celle d'une même machine et d'un même binaire
  (l'usage réel : rejouer un entraînement, comparer deux réglages). Le test de valeurs de référence
  ci-dessus porte donc sur `nextFloat()` (arithmétique entière exacte), pas sur `nextGaussian`.
- **`std::mt19937_64` n'est pas cryptographiquement sûr** — sans importance ici (aucun usage
  sécuritaire), mais à ne pas réutiliser hors de ce contexte si un jour un besoin cryptographique
  apparaissait ailleurs dans le projet.

## Définition de fait (DoD)
- `Source/AiSolver` compile comme bibliothèque CMake indépendante, liée à `Core`. `aisolver::Rng`
  disponible et testé (`ctest` vert) ; build `/W4 /WX` sans avertissement ; Doxygen à jour ;
  `EX-IA-001` déclarée (au niveau de l'épic, cette tâche n'introduit pas d'exigence propre).

## Notions abordées
@ref guide-annexe-algebre-tensorielle (tenseurs, forme et *stride*, produit matriciel, réductions,
générateurs pseudo-aléatoires), en particulier sa section 7 (générateurs pseudo-aléatoires
déterministes, graine, transformation de Box-Muller).

## Exigences
Contribue à `EX-IA-001` (déclarée dans [l'épic](epic.md)) ; aucune exigence propre à cette tâche.
