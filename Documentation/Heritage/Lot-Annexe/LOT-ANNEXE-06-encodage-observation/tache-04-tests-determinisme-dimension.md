# TACHE-04 — Tests : déterminisme et stabilité dimensionnelle aux bords {#lot-annexe-06-tache-04-tests-determinisme-dimension}

**Lot :** [LOT-ANNEXE-06](epic.md) · **Emplacement :** `Source/Test/Unit/AiSolver/Env` · **Statut :** à faire

## Contexte
TACHE-01 à 03 couvrent chacune leurs cas unitaires propres. Cette tâche ajoute la couche de tests
**transverse** aux trois encodeurs, sur les deux garanties dont dépend tout le reste du programme
annexe : le déterminisme (mêmes entrées `Core` → même tenseur, condition d'un entraînement
reproductible) et la stabilité de forme près des bords de carte (condition d'un réseau à topologie
fixe, LOT-ANNEXE-03).

## Travail à réaliser
- **`Source/Test/Unit/AiSolver/Env/test_observation_determinisme.cpp`** (nouveau) : charge un niveau
  réel (`JADG_LEVELS_DIR`) via `aisolver::HeadlessLevelEnvironment`, avance de quelques pas
  avec un script d'entrée fixe, encode l'observation complète (les trois encodeurs de TACHE-01/02/03
  assemblés) **deux fois** sur le même état (sans avancer entre les deux), compare bit-à-bit.
  Répété sur plusieurs niveaux de la séquence de `test_parcours_complet.cpp` (au moins un par famille
  de mécanique : mécanisme, bloc, danger avancé).
- **`Source/Test/Unit/AiSolver/Env/test_observation_bords_carte.cpp`** (nouveau) : pour chaque coin
  et chaque bord d'un niveau réel (personnage placé, via un `reset` suivi d'entrées scriptées ou d'un
  niveau dédié minimal, à `column = 0`, `column = width - 1`, `row = 0`, `row = height - 1`), encode
  l'observation avec plusieurs rayons (`0`, `1`, `3`, `5`) et vérifie que la forme du tenseur
  catégoriel et celle du tenseur de mécanisme sont **exactement** celles attendues
  (`(29, 2r+1, 2r+1)` et `(2, 2r+1, 2r+1)`), y compris quand `2r + 1` dépasse la dimension de la
  carte (rayon plus grand que le niveau lui-même).
- **Cas limite carte 1×1** (`TileMap` minimale construite en test, pas un fichier niveau) : un
  personnage sur une carte à une seule case, avec `radius = 3`, produit un tenseur `(29, 7, 7)` où
  une seule case est réellement dans la grille, les 48 autres en vecteur nul.

## Fichiers impactés
- `Source/Test/Unit/AiSolver/Env/test_observation_determinisme.cpp` (nouveau).
- `Source/Test/Unit/AiSolver/Env/test_observation_bords_carte.cpp` (nouveau).
- `Source/Test/CMakeLists.txt` (ajout des deux fichiers à `UnitTests`, dans la liste déjà mise à jour
  par LOT-ANNEXE-05/TACHE-04 pour lier `AiSolver`).

## Tests (obligatoires)
Cette tâche **est** la suite de tests (voir « Travail à réaliser » ci-dessus) ; pas de sous-ensemble
distinct.

## Points d'attention
- **Le déterminisme testé ici couvre l'assemblage complet**, pas seulement chaque encodeur pris
  isolément (déjà couvert par TACHE-01/02/03) : une source d'aléa introduite dans l'assemblage
  (itération sur une structure non ordonnée, par exemple) ne serait détectée qu'à ce niveau.
- **Les tests de bord doivent utiliser un niveau réel des assets livrés en plus du cas 1×1
  synthétique** : un niveau réel exerce des tuiles non triviales près des bords (souvent `Solid`, les
  murs de bordure), le cas 1×1 isole la mécanique pure d'un `radius` qui dépasse largement la carte.
- **Pas de comparaison de valeurs flottantes approximatives** (`EXPECT_NEAR`) pour le déterminisme :
  l'encodage est un remplissage direct de constantes (`0.0f`/`1.0f`) et de lectures de champs, sans
  aucune opération flottante qui justifierait une tolérance — une égalité bit-à-bit (`EXPECT_EQ`) est
  le bon niveau d'exigence, et sa violation serait un signal fort d'un bug réel.

## Définition de fait (DoD)
- Les deux fichiers de test compilent et passent (`ctest` vert) ; build `/W4 /WX` sans avertissement.

## Notions abordées
@ref guide-annexe-apprentissage-renforcement — état et observation, conception d'une observation :
one-hot, normalisation, observation partielle.

## Exigences
`EX-IA-006` (nouvelle).
