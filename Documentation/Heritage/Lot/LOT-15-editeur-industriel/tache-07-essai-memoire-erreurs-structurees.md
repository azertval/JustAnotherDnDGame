# TACHE-07 — Essai immédiat en mémoire, erreurs de validation structurées {#lot-15-tache-07-essai-memoire-erreurs-structurees}

**Lot :** [LOT-15](epic.md) · **Emplacement :** `Core/Levels`, `HMI/Interface` · **Statut :** à faire

## Contexte
Deux points de dette technique relevés en LOT-14 : l'essai immédiat (`P`) écrit le brouillon dans
un **fichier temporaire partagé** (`projectgaming_playtest_level.json`) avant de le recharger dans
un `GameScreen` — un aller-retour JSON inutile et un risque de collision entre deux instances de
l'éditeur ou un fichier resté d'une session précédente. Et `describeValidationError` traduit les
messages d'erreur en cherchant des **sous-chaînes françaises** (`"entree"`, `"sortie"`,
`"interrupteur"`, `"Porte liee"`) dans le message technique de `LevelLoader` — tout changement de
formulation y casserait silencieusement la traduction.

## Travail à réaliser
- **`LevelValidationError`** (`enum class`, `Core/Levels`) : `None`, `MissingEntry`, `MissingExit`,
  `UnresolvedMechanism`, `OutOfBounds`, `DuplicatePosition`, `ParseError`, … (un cas par famille
  d'erreur déjà distinguée dans les messages actuels de `LevelLoader`). `LevelLoadResult` gagne un
  champ `LevelValidationError error_code` (nom provisoire), rempli par `LevelLoader` en plus du
  message technique `error` existant — **additif**, `error` inchangé (tests/journaux existants non
  affectés).
- **`describeValidationError`** (`EditorScreen`) bascule sur un `switch` de `error_code` plutôt que
  sur la recherche de sous-chaînes.
- **`GameScreen`** : factoriser la construction de scène (monde ECS, spawn du personnage)
  aujourd'hui dans `loadLevel(const std::filesystem::path&)` en un `loadLevel(core::Level)` privé
  partagé. Ajouter un constructeur `GameScreen(SpriteBatch&, const TextureAtlas&, int, int,
  core::Level level)` qui appelle directement ce `loadLevel(core::Level)`, sans `LevelSequence` ni
  accès disque ; en fin de niveau (sortie atteinte), ce mode se comporte comme la fin de séquence
  actuelle (retour au menu — pas d'enchaînement, cohérent avec un essai à niveau unique).
- **`EditorScreen::startPlaytest`** utilise ce nouveau constructeur avec le `core::Level` déjà
  produit par `_draft.toLevel()` ; suppression de `playtestFilePath()` et de toute écriture disque
  pour l'essai immédiat.

## Fichiers impactés
- `Source/Core/Levels/LevelLoader.h`/`.cpp`, `LevelLoadResult` (nouveau champ).
- `Source/HMI/Interface/GameScreen.h`/`.cpp` (factorisation, nouveau constructeur).
- `Source/HMI/Interface/EditorScreen.h`/`.cpp` (`describeValidationError`, `startPlaytest`).
- Tests unitaires (`test_level_loader.cpp` ou équivalent, `test_game_screen.cpp` si existant),
  test système d'essai immédiat.

## Tests (obligatoires)
- Chaque famille d'erreur de `LevelLoader` (entrée manquante, sortie manquante, liaison non
  résolue, position hors bornes, position dupliquée, JSON malformé) produit le `error_code` attendu
  ; `error` (message technique) reste identique à avant (non-régression des tests existants).
- `describeValidationError` produit le message non-codeur attendu pour chaque `error_code`.
- Le nouveau constructeur `GameScreen(core::Level)` construit une scène jouable identique (même
  position de personnage, mêmes tuiles) à celle obtenue via le constructeur par fichier pour un
  niveau équivalent.
- Lancer un essai immédiat ne crée **aucun fichier** sous le dossier temporaire (vérification par
  chemin, absence de `projectgaming_playtest_level.json`).

## Points d'attention
- Garder `LevelLoadResult::error` (texte) pour la compatibilité des tests/journaux déjà en place —
  ce n'est pas remplacé, seulement complété.
- La factorisation de `GameScreen::loadLevel` ne doit rien changer au comportement du mode
  séquence-par-fichiers existant (non-régression LOT-09/LOT-12) : c'est un partage de code, pas une
  réécriture de la logique de scène.

## Définition de fait (DoD)
- Essai immédiat en mémoire et erreurs structurées opérationnels et testés (`ctest` vert) ; build
  `/W4 /WX` ; Doxygen à jour.

## Exigences
`EX-EDIT-007`, `EX-EDIT-008` (approfondies, sens inchangé), `EX-NFR-040` (erreurs récupérables).
