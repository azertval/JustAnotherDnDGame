# TACHE-01 — Composant et système d'animation {#lot-18-tache-01-composant-systeme-animation}

**Lot :** [LOT-18](epic.md) · **Emplacement :** `Core/Ecs` · **Statut :** fait

## Contexte
Le personnage n'a aujourd'hui aucun état d'animation : `GameScreen::spawnPlayer` fixe une région
d'atlas une seule fois, au spawn. Ce lot introduit l'état qui permettra au rendu de choisir la
bonne image à chaque frame, en pur `Core` (`EX-ARCH-011`), sur le modèle de
`CharacterPhysicsSystem`/`MovementSystem`.

## Travail à réaliser
- **`core::AnimationClip`** (nouvel enum, `Source/Core/Ecs/Components/Animation.h`) : `Idle`,
  `Run`, `Jump`.
- **`core::Animation`** (nouveau composant, même fichier) : donnée pure —
  `AnimationClip clip = Idle`, `int frameIndex = 0`, `float elapsed = 0.0f`. Constantes publiques
  `IDLE_FRAME_COUNT = 2`, `RUN_FRAME_COUNT = 4`, `JUMP_FRAME_COUNT = 1` (seule source de vérité du
  nombre d'images par clip — consommée aussi bien par le système que par `HMI` en TACHE-02, pas
  dupliquée).
- **`core::AnimationSystem`** (nouveau, `Source/Core/Ecs/Systems/AnimationSystem.h`/`.cpp`,
  implémente `ISystem` comme `MovementSystem`) : pour chaque entité `Player` + `Velocity` +
  `Animation` :
  1. Détermine le clip cible : `!player.grounded` → `Jump` ; `grounded` et
     `abs(velocity.value.x) > epsilon` → `Run` ; sinon → `Idle`.
  2. Si le clip cible diffère du clip courant : `clip = cible`, `frameIndex = 0`, `elapsed = 0`
     (réinitialisation nette, cf. décision de cadrage de l'épic).
  3. Sinon, avance `elapsed += fixedDelta` et fait progresser `frameIndex` (avec bouclage modulo
     le nombre d'images du clip) chaque fois qu'une durée d'image (constante privée au système,
     ex. `IDLE_FRAME_DURATION`, `RUN_FRAME_DURATION`) est atteinte ; `Jump` (une seule image) ne
     progresse jamais.

## Fichiers impactés
- `Source/Core/Ecs/Components/Animation.h` (nouveau).
- `Source/Core/Ecs/Systems/AnimationSystem.h`/`.cpp` (nouveau).
- `Source/Core/CMakeLists.txt` (ajout de `Ecs/Systems/AnimationSystem.cpp`).
- Tests : `Source/Test/Integration/test_animation_personnage.cpp` (nouveau, sur le modèle de
  `test_ecs_mouvement.cpp`/`test_physique_personnage.cpp`).

## Tests (obligatoires)
- Au sol, vitesse horizontale nulle → clip `Idle`, l'image alterne 0/1 après chaque durée
  d'image écoulée (pas avant).
- Au sol, vitesse horizontale non nulle → clip `Run`, l'image boucle sur les 4 images dans
  l'ordre.
- Pas au sol (`grounded = false`) → clip `Jump`, image 0 immobile même après de nombreux pas.
- Changement de clip en cours d'animation (ex. `Run` → `Jump` en décollant) : `frameIndex` et
  `elapsed` repartent à zéro immédiatement, pas de saut d'image incohérent.
- Une entité sans composant `Animation` n'est pas affectée (le système ne plante pas, ne lui en
  ajoute pas).

## Points d'attention
- **`IDLE_FRAME_COUNT`/`RUN_FRAME_COUNT`/`JUMP_FRAME_COUNT` sont la seule source de vérité** du
  nombre d'images par clip : TACHE-02 les réutilise pour dimensionner/indexer la grille de
  l'atlas — ne pas coder ces nombres en dur une seconde fois côté `HMI`.
- **Le système doit s'exécuter après `CharacterPhysicsSystem`** dans `GameScreen::update`
  (TACHE-02) : il lit `Player::grounded`, calculé par la physique **pour ce même pas** — l'exécuter
  avant lirait l'état du pas précédent (décalage d'une frame, peu visible mais incorrect).
- Le seuil (epsilon) distinguant « immobile » de « en mouvement » doit tolérer le bruit flottant
  sans jamais déclencher `Run` pour un personnage réellement immobile (`velocity.value.x` peut
  être exactement `0.0f` en pratique, mais un epsilon reste plus sûr qu'une égalité stricte).

## Définition de fait (DoD)
- `core::Animation`/`core::AnimationSystem` disponibles, testés (`ctest` vert) ; build `/W4 /WX`
  sans avertissement ; Doxygen à jour.

## Exigences
`EX-REN-012` (logique de sélection de clip/image, partie `Core`).
