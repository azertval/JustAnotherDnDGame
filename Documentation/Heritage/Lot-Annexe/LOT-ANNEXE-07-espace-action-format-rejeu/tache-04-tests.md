# TACHE-04 — Tests : couverture, absence de biais, round-trip {#lot-annexe-07-tache-04-tests}

**Lot :** [LOT-ANNEXE-07](epic.md) · **Emplacement :** `Source/Test/Unit/AiSolver` · **Statut :** à faire

## Contexte
TACHE-01/02/03 introduisent chacune leurs propres cas de test unitaires locaux ; cette tâche
consolide les vérifications transversales entre l'espace d'action, son décodage et le format de
rejeu — en particulier le lien de bout en bout entre une distribution de sortie de réseau et une
séquence exportée rejouable.

## Travail à réaliser
- **`Source/Test/Unit/AiSolver/Env/test_action_space.cpp`** : couverture exhaustive des 24
  combinaisons de `ActionSpace` (TACHE-01), bijection indice ↔ action.
- **`Source/Test/Unit/AiSolver/Env/test_action_decoding.cpp`** : tests statistiques de
  `decodeStochastic`, tests de `decodeArgmax` (TACHE-02).
- **`Source/Test/Unit/AiSolver/Replay/test_replay_file.cpp`** : round-trip du format de rejeu
  (TACHE-03), y compris un test **de bout en bout** : une séquence d'`Action` décodées par
  `decodeArgmax` à partir de distributions synthétiques est convertie en `core::PlayerInput`
  (TACHE-01), assemblée en `ReplayFile`, écrite puis relue — la séquence relue doit produire,
  rejouée sur `HeadlessLevelEnvironment` (`LOT-ANNEXE-05`), exactement la même trajectoire que la
  séquence d'origine (déterminisme de bout en bout, pas seulement égalité structurelle des données).

## Fichiers impactés
- `Source/Test/Unit/AiSolver/Env/test_action_space.cpp` — nouveau.
- `Source/Test/Unit/AiSolver/Env/test_action_decoding.cpp` — nouveau.
- `Source/Test/Unit/AiSolver/Replay/test_replay_file.cpp` — nouveau.
- `Source/Test/CMakeLists.txt` — ajout des nouveaux fichiers à la cible `UnitTests`.

## Tests (obligatoires)
- Voir TACHE-01/02/03 pour le détail des cas unitaires ; cette tâche ajoute spécifiquement :
- **Test de bout en bout déterministe** : une séquence d'actions décodées, exportée puis rejouée
  sur un niveau simple de `Source/Elements/Levels`, produit la même issue (`core::LevelOutcome`) et
  la même trajectoire finale que lors de sa production initiale.
- **Stabilité de la bijection indice ↔ action à travers une sérialisation** : un `ReplayFile`
  contenant des `core::PlayerInput` issus de `toPlayerInput(actionAt(i))` pour tout `i` valide,
  une fois relu, reproduit exactement les mêmes `core::PlayerInput` (pas de perte de précision
  flottante introduite par la sérialisation JSON, à la tolérance documentée en TACHE-03).

## Points d'attention
- **Le test de bout en bout est le seul endroit du lot qui exerce réellement
  `HeadlessLevelEnvironment`** (TACHE-01/02/03 restent unitaires, sans dépendance à un niveau réel) —
  volontairement placé ici, en dernier, une fois les trois briques individuellement vérifiées.
- **Niveau de test choisi simple et court** (ex. `demo-deplacement.json`) pour que ce test reste
  rapide malgré son caractère bout-en-bout — pas un niveau représentatif de la difficulté visée par
  la génération 2/3, seulement de la fidélité du pipeline décodage → export → rejeu.

## Définition de fait (DoD)
- Les trois suites de tests vertes (`ctest`), y compris le test de bout en bout ; build `/W4 /WX`
  sans avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-apprentissage-renforcement — action, espace d'action, politique déterministe ou
stochastique, exploration.

## Exigences
`EX-IA-007`, `EX-IA-008` (nouvelles, du même lot).
