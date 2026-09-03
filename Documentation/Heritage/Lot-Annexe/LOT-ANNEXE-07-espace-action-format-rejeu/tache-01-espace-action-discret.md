# TACHE-01 — Espace d'action discret {#lot-annexe-07-tache-01-espace-action-discret}

**Lot :** [LOT-ANNEXE-07](epic.md) · **Emplacement :** `Source/AiSolver/Env` · **Statut :** à faire

## Contexte
`core::PlayerInput` (`Source/Core/Physics/PlayerInput.h`) est une structure de données continue
(`moveX`/`moveY` flottants, trois booléens) ; un réseau de neurones (`LOT-ANNEXE-03`) produit plus
naturellement une distribution catégorielle finie (`softmax` sur un nombre fixe de sorties) qu'une
sortie continue non bornée. Cette tâche définit l'énumération finie qui fait le pont entre les deux.

## Travail à réaliser
- **`aisolver::ActionSpace`** (`Source/AiSolver/Env/ActionSpace.h/.cpp`) : `enum class Direction {
  Left, None, Right };`, `enum class ActionId : int { ... }` généré comme produit cartésien
  `Direction × bool jump × bool jumpHeld × bool dash × bool interact`
  (3 × 2 × 2 × 2 × 2 = 48 combinaisons), ou une
  structure `Action { Direction direction; bool jumpPressed; bool jumpHeld; bool dashPressed; }`
  plus une fonction `size_t actionCount()` et `Action actionAt(size_t index)`/`size_t indexOf(const
  Action&)` pour l'aller-retour indice ↔ action (l'indice est ce qu'un réseau de sortie catégorielle
  manipule directement).
- **`core::PlayerInput toPlayerInput(const Action&)`** : traduit une action en structure consommée
  par `core::CharacterPhysicsSystem::update` (via `HeadlessLevelEnvironment::step`, `LOT-ANNEXE-05`),
  `moveX` = `-1`/`0`/`1` selon `Direction`, `moveY` fixé à `0` (décision de cadrage de l'épic — pas
  de dimension verticale de dash dans cette version).

## Fichiers impactés
- `Source/AiSolver/Env/ActionSpace.h/.cpp` — nouveau.
- `Source/AiSolver/CMakeLists.txt` — ajout des nouveaux fichiers.

## Tests (obligatoires)
- **Bijection indice ↔ action** : pour tout indice valide, `indexOf(actionAt(indice)) == indice`,
  et réciproquement pour toute combinaison valide d'`Action`.
- **Traduction correcte vers `core::PlayerInput`** : chaque combinaison de `Direction`/booléens
  produit les champs `moveX`/`jumpPressed`/`jumpHeld`/`dashPressed` attendus, `moveY` toujours `0`.
- **Nombre total de combinaisons** : `actionCount() == 48`, vérifié explicitement (pas seulement
  déduit du parcours des tests précédents), pour détecter tout changement accidentel de la taille de
  l'espace d'action dans une évolution future.

## Points d'attention
- **`moveY` fixé à `0` est une décision de cadrage documentée, pas un oubli** : si un futur niveau
  exige un dash vertical, cette tâche devra être révisée explicitement (nouvelle dimension de
  l'espace d'action), pas contournée silencieusement ailleurs.
- **L'ordre d'énumération des combinaisons (`actionAt`) doit rester stable** une fois des modèles
  entraînés dessus (génération 2/3) : le changer romprait la correspondance entre l'indice de sortie
  d'un réseau déjà entraîné et l'action qu'il désigne réellement — à documenter comme un invariant du
  format, pas seulement du code.

## Définition de fait (DoD)
- `ActionSpace` et `toPlayerInput` disponibles et testés (`ctest` vert) ; build `/W4 /WX` sans
  avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-apprentissage-renforcement — action, espace d'action, politique déterministe ou
stochastique, exploration.

## Exigences
`EX-IA-007` (nouvelle, partagée avec TACHE-02/03/04 du même lot).
