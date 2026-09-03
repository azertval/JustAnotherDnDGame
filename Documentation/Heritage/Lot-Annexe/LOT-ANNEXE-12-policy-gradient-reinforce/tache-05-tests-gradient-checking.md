# TACHE-05 — Tests : gradient checking et non-régression de progression {#lot-annexe-12-tache-05-tests-gradient-checking}

**Lot :** [LOT-ANNEXE-12](epic.md) · **Emplacement :** `Source/Test/Unit/AiSolver/Training` ·
**Statut :** fait

## Contexte
Les tâches précédentes intègrent déjà des tests unitaires locaux à chaque brique (TACHE-01 à
TACHE-04 listent chacune leurs cas). Cette tâche consolide et **complète** la couverture au niveau
du lot entier : elle rassemble le gradient checking spécifique à la perte REINFORCE dans un fichier
dédié réutilisant explicitement l'infrastructure de LOT-ANNEXE-02, et ajoute un test de
non-régression de bout en bout qui exerce `ReinforceTrainer` comme le ferait un usage réel, sur un
niveau de contrôle fixe versionné avec les tests.

## Travail à réaliser
- **`Source/Test/Unit/AiSolver/Training/test_reinforce_loss.cpp`** : reprend l'utilitaire de
  gradient checking générique introduit par LOT-ANNEXE-02 (différences finies vs `backward()`) et
  l'applique spécifiquement au graphe produit par `computeReinforceLoss` (TACHE-03), sur plusieurs
  configurations (réseau à une couche cachée minuscule, trajectoires synthétiques de 1, 3 et 10
  pas, retours positifs/négatifs/nuls).
- **`Source/Test/Unit/AiSolver/Training/test_reinforce_trainer.cpp`** : test de bout en bout sur un
  niveau de contrôle dédié (petit niveau JSON, chemin direct et sans ambiguïté vers la sortie,
  stocké avec les fixtures de test existantes) — lance un run court de `ReinforceTrainer`, vérifie
  que la récompense moyenne progresse entre le début et la fin du run (seuil de progression minimal
  fixé empiriquement, avec une marge confortable pour éviter un test fragile).
- Ajout des deux fichiers à la cible `UnitTests` existante (`Source/Test/CMakeLists.txt` ou
  équivalent local au dossier `AiSolver`, selon la structure déjà en place pour les tests des lots
  amont).
- Fixture de niveau de contrôle : réutilise si possible un niveau déjà utilisé comme référence par
  LOT-ANNEXE-05/08/10 (pour que la comparaison future avec l'évolutionniste porte sur exactement le
  même niveau) ; sinon, en créer un minimal dédié à ce test, documenté comme tel.

## Fichiers impactés
- `Source/Test/Unit/AiSolver/Training/test_reinforce_loss.cpp` (nouveau).
- `Source/Test/Unit/AiSolver/Training/test_reinforce_trainer.cpp` (nouveau).
- `Source/Test/CMakeLists.txt` (ou fichier CMake local à `AiSolver/Training`, selon la structure
  déjà établie par les tests des lots amont).

## Tests (obligatoires)
- Le gradient checking de `test_reinforce_loss.cpp` couvre au minimum : action de log-probabilité
  faible avec retour fort positif, action de log-probabilité forte avec retour négatif, retour nul
  (gradient attendu nul), trajectoire multi-pas avec retours de signes mélangés.
- Le test de bout en bout de `test_reinforce_trainer.cpp` est **déterministe** (graine fixe) et
  reproductible en local comme en CI, sans dépendance GPU ni fenêtre.
- Temps d'exécution du run de contrôle borné (budget d'épisodes volontairement petit) pour rester
  compatible avec le temps global de la suite `ctest`.

## Points d'attention
- **Éviter un test fragile sur le seuil de progression** : un seuil trop strict ferait échouer le
  test au moindre changement d'implémentation numériquement neutre (ordre de sommation, etc.) ; un
  seuil trop lâche ne détecterait plus de régression réelle — calibrer avec une marge explicite et
  documentée dans le fichier de test.
- Le niveau de contrôle utilisé ici est le même, autant que possible, que celui qui servira de base
  à la comparaison chiffrée de LOT-ANNEXE-13 (TACHE-04) — le documenter clairement pour que ce choix
  soit repris sans ambiguïté dans les lots suivants.

## Définition de fait (DoD)
- Les deux fichiers de test compilent et passent (`ctest` vert) dans la cible `UnitTests` ; build
  `/W4 /WX` sans avertissement ; `EX-IA-013` couverte de bout en bout par au moins un test
  d'intégration (le trainer complet) en plus des tests unitaires par brique.

## Notions abordées
@ref guide-annexe-reinforce — policy gradient, trajectoire, retour actualisé, algorithme REINFORCE.

## Exigences
`EX-IA-013` (nouvelle, vérifiée par ce lot de tests).
