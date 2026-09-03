# TACHE-04 — Cible CMake AiSolver {#lot-annexe-05-tache-04-cible-cmake-aisolver}

**Lot :** [LOT-ANNEXE-05](epic.md) · **Emplacement :** `Source/AiSolver`, `Source/CMakeLists.txt` · **Statut :** à faire

## Contexte
`Source/AiSolver` n'a pour l'instant aucune cible CMake : la génération 0 (LOT-ANNEXE-01 à 04) écrit
`Source/AiSolver/Math`/`Nn`/`Optim` en parallèle, mais c'est cette tâche qui fait exister la
bibliothèque `AiSolver` en tant que telle et l'intègre à la configuration racine, sur le gabarit
exact de `Source/Core/CMakeLists.txt`.

## Travail à réaliser
- **`Source/AiSolver/CMakeLists.txt`** (nouveau) : `add_library(AiSolver STATIC …)` listant les
  sources C++ du module (`Env/HeadlessLevelEnvironment.cpp` de ce lot, plus les sources déjà posées
  par la génération 0 dans `Math`/`Nn`/`Optim`). `target_include_directories(AiSolver PUBLIC
  ${PROJECT_SOURCE_DIR}/Source)` — même racine d'inclusion que `Core`, chemins complets depuis
  `Source/` (`"AiSolver/Env/HeadlessLevelEnvironment.h"`, jamais un nom seul). `target_link_libraries
  (AiSolver PUBLIC Core PRIVATE project_warnings project_options)` — lien **public** vers `Core`
  (les en-têtes publics d'`AiSolver`, comme `HeadlessLevelEnvironment.h`, exposent des types `core::
  …` dans leur interface ; un consommateur d'`AiSolver` doit donc voir transitivement les en-têtes de
  `Core`). Aucune dépendance Qt/HMI/D3D11, aucune nouvelle dépendance tierce (`External/CMakeLists.txt`
  inchangé).
- **`Source/CMakeLists.txt`** : ajoute `add_subdirectory(AiSolver)`, après `add_subdirectory(Core)`
  (ordre de déclaration, `AiSolver` en dépend) et avant ou après `add_subdirectory(HMI)` (ordre
  indifférent entre les deux, aucune dépendance croisée).
- **`Source/Test/CMakeLists.txt`** : `target_link_libraries(UnitTests PRIVATE … AiSolver)` et
  `target_link_libraries(SystemTests PRIVATE … AiSolver)` (nécessaire dès TACHE-01/05 de ce lot pour
  compiler les tests correspondants) — aucune nouvelle cible CMake créée (contrainte transverse :
  les tests d'`AiSolver` rejoignent les cibles `UnitTests`/`IntegrationTests`/`SystemTests`
  existantes), réutilisant la définition `JADG_LEVELS_DIR` déjà présente, identique sur les
  trois cibles.

## Fichiers impactés
- `Source/AiSolver/CMakeLists.txt` (nouveau).
- `Source/CMakeLists.txt` (`add_subdirectory(AiSolver)`).
- `Source/Test/CMakeLists.txt` (`AiSolver` ajouté à `target_link_libraries` de `UnitTests` et
  `SystemTests`, fichiers `Unit/AiSolver/Env/test_headless_level_environment.cpp` et
  `Systeme/test_parcours_complet.cpp` déjà listés/à compléter).

## Tests (obligatoires)
- **Configuration CMake** : `cmake --preset <preset>` puis build complet réussit sans avertissement,
  la bibliothèque `AiSolver` apparaît en sortie de configuration.
- **Absence de dépendance Qt/D3D11** : inspection du graphe de liens (`cmake --graphviz` ou
  équivalent) confirmant qu'`AiSolver` ne lie ni `Qt6::*` ni aucune bibliothèque D3D11.
- **`ctest` découvre les nouveaux tests** : `gtest_discover_tests(UnitTests)`/
  `gtest_discover_tests(SystemTests)` exposent les cas ajoutés par TACHE-01/02/03/05 sans
  configuration supplémentaire.

## Points d'attention
- **`AiSolver` lié à `Core` uniquement** (décision transverse du programme annexe) : toute tentation
  de lier `AiSolver` à `HMI` (pour réutiliser un utilitaire) est un signal que ce code n'a pas sa
  place dans `AiSolver`, ou doit être déplacé dans `Core`.
- **Pas de nouvelle cible CMake pour les tests** : un réflexe naturel serait de créer `AiSolverTests`
  dédiée — explicitement écarté (décision transverse) au profit de l'intégration dans les cibles
  existantes, pour ne pas multiplier les exécutables de test et la configuration CI associée.
- **Ordre des `add_subdirectory`** : `AiSolver` doit être déclaré après `Core` (dont il dépend), sans
  quoi CMake échoue à résoudre la cible `Core` lors de la configuration d'`AiSolver`.

## Définition de fait (DoD)
- `AiSolver` compile en bibliothèque statique séparée, liée uniquement à `Core` ; `UnitTests`/
  `SystemTests` compilent et s'exécutent avec les tests de ce lot ; build `/W4 /WX` sans
  avertissement.

## Notions abordées
@ref guide-annexe-apprentissage-renforcement — agent, environnement, boucle `reset`/`step`, épisode,
propriété de Markov.

## Exigences
`EX-IA-005` (nouvelle).
