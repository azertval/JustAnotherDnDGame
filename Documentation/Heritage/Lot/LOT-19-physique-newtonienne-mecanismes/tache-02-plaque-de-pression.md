# TACHE-02 — Plaque de pression {#lot-19-tache-02-plaque-de-pression}

**Lot :** [LOT-19](epic.md) · **Emplacement :** `Core/Levels`, `Core/Gameplay` · **Statut :** fait

## Contexte
Le seul mécanisme existant est l'interrupteur à bascule (`EX-GP-020`, `MechanismController`) :
bascule au **front** de contact, l'état persiste après le départ. Ce lot ajoute une **plaque de
pression** — même infrastructure de liaison (identifiant/`opensWith`), mais activation
**continue** : la porte reste ouverte tant qu'un poids suffisant y repose. Dépend de TACHE-01
(`Player::mass`).

## Travail à réaliser
- **`core::TileType`** (`Source/Core/Levels/TileType.h`) : nouvelle valeur `PressurePlate`.
- **`core::LevelLoader`** (`LevelLoader.cpp`) : `parseTileType` reconnaît `"pressurePlate"` ;
  traite `PressurePlate` comme `Switch` pour l'attribution/la validation d'`id` (une plaque a
  besoin d'un identifiant, au même titre qu'un interrupteur — même erreur `MissingSwitchId` /
  `DuplicateSwitchId` si absent/dupliqué).
- **`core::LevelWriter`** (`LevelWriter.cpp`) : `tileTypeName` gère `PressurePlate` ; l'attribution
  d'identifiants (balayage de la grille) inclut les tuiles `PressurePlate` au même titre que
  `Switch`.
- **`core::LevelDraft::linkMechanism`** (`LevelDraft.cpp`) : l'assertion sur la position source
  accepte `Switch` **ou** `PressurePlate` (la position cible reste `Door`, inchangée).
- **`core::MechanismController`** (`MechanismController.h`/`.cpp`) :
  - Nouveau vecteur privé `_continuous` (parallèle à `_mechanisms`), calculé **une fois** au
    constructeur : `true` si la tuile à `switchPosition` (dans le `TileMap` du `Level` d'origine)
    est `PressurePlate`.
  - `update(const Aabb& playerBox, float playerMass = 1.0f)` : nouveau paramètre (défaut
    conservant la compatibilité des appels existants sans poids). Pour chaque mécanisme :
    - Si `_continuous[index]` : porte ouverte **si et seulement si** `onSwitch &&
      playerMass >= MIN_TRIGGER_MASS` — pas de logique de front, réévalué à **chaque** pas.
    - Sinon (interrupteur classique) : comportement **inchangé** (bascule au front), le poids
      n'intervient pas.
  - Constante `constexpr float MIN_TRIGGER_MASS = 1.0f;` dans le namespace anonyme du `.cpp` —
    calée sur la masse par défaut du personnage (`core::Player::mass`), pour qu'une plaque
    fonctionne « prête à l'emploi » sans configuration.
- **`hmi::GameScreen::update`** : récupère `core::Player::mass` du personnage et le transmet à
  `_mechanisms->update(box, playerMass)`.
- **`EX-GP-025`** : nouvelle exigence dans `Documentation/Specification/gameplay.md`, section 3
  (après `EX-GP-024`) — plaque de pression, activation continue, distincte de l'interrupteur.

## Fichiers impactés
- `Source/Core/Levels/TileType.h`, `LevelLoader.cpp`, `LevelWriter.cpp`, `LevelDraft.cpp`.
- `Source/Core/Gameplay/MechanismController.h`/`.cpp`.
- `Source/HMI/Interface/GameScreen.cpp`.
- `Documentation/Specification/gameplay.md` (`EX-GP-025`).
- Tests : `Source/Test/Unit/Core/Gameplay/test_mechanism_controller.cpp`,
  `Source/Test/Unit/Core/Levels/test_level_loader.cpp`,
  `Source/Test/Unit/Core/Levels/test_level_writer.cpp`,
  `Source/Test/Unit/Core/Levels/test_level_draft.cpp` (nouveaux cas `PressurePlate`).

## Tests (obligatoires)
- Une plaque de pression : porte fermée par défaut ; ouverte **tant que** le personnage est
  dessus (poids suffisant) ; refermée **dès qu'il en part** — sans effet de bascule (rester dessus
  ne la ferme pas, revenir ne re-bascule pas deux fois).
- Un interrupteur classique dans le **même** niveau qu'une plaque : comportement **inchangé**
  (non-régression, `MechanismControllerTest` existants toujours verts).
- `LevelLoader`/`LevelWriter` : une tuile `PressurePlate` avec `id` se charge/s'enregistre comme un
  `Switch` (mêmes règles d'identifiant, aller-retour JSON fidèle).
- `LevelDraft::linkMechanism` accepte une source `PressurePlate` (comme `Switch`).

## Points d'attention
- **Le choix `_continuous[index]` est figé au chargement** (tuile d'origine, jamais réévalué) :
  cohérent avec le reste de `MechanismController`, qui ne fait jamais muter `Switch`/`PressurePlate`
  eux-mêmes (seule la porte change de type dans `_collision`).
- **`playerMass` a une valeur par défaut** dans la signature d'`update` : les appels existants
  (tests d'interrupteurs classiques, où le poids n'intervient jamais) restent valides sans
  modification.
- Une porte de plaque de pression peut se refermer **alors que le personnage la traverse** — c'est
  la tension voulue du mécanisme (course contre la fermeture), déjà le comportement implicite de
  l'interrupteur à bascule existant (aucun traitement spécial n'existait déjà pour ce cas) : pas de
  nouvelle règle à ajouter, juste ne pas en introduire une par erreur.

## Définition de fait (DoD)
- Plaque de pression fonctionnelle et testée (`ctest` vert), pipeline niveau (chargement/écriture/
  édition) à jour ; build `/W4 /WX` sans avertissement ; Doxygen à jour ; `EX-GP-025` déclarée.

## Exigences
`EX-GP-025` (nouvelle).
