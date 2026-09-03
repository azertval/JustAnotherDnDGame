# TACHE-01 — Masse et chute newtonienne {#lot-19-tache-01-masse-chute-newtonienne}

**Lot :** [LOT-19](epic.md) · **Emplacement :** `Core/Physics`, `Core/Ecs` · **Statut :** fait

## Contexte
`CharacterPhysicsSystem` intègre déjà la gravité en continu, pondérée par des multiplicateurs de
ressenti (chute renforcée, fast-fall, flottement à l'apex, `EX-GP-018`), mais la borne haute
(`PhysicsConfig::maxFallSpeed`) est un `std::min` brutal — un plafond arbitraire, sans traduire de
masse ni de résistance. Ce lot introduit une masse par personnage et remplace ce plafond par une
**traînée** dont l'équilibre avec le poids fait émerger naturellement une vitesse terminale.

## Travail à réaliser
- **`core::Player`** (`Source/Core/Ecs/Components/Player.h`) : nouveau champ `float mass = 1.0f;`
  (unité de jeu arbitraire).
- **`core::PhysicsConfig`** (`Source/Core/Physics/PhysicsConfig.h`) : remplace `maxFallSpeed` par
  `float fallDragCoefficient = 3.6f;` (traînée verticale, en 1/s). Valeur calibrée pour retomber
  sur l'ancienne vitesse terminale à masse par défaut : à l'équilibre, `effectiveGravity =
  fallDragCoefficient × vitesse`, soit `90 / 3.6 = 25` (gravité effective en chute = `50 ×
  fallGravityMultiplier(1.8)`), identique à l'ancien `maxFallSpeed`.
- **`core::CharacterPhysicsSystem::update`** (`Source/Core/Ecs/Systems/CharacterPhysicsSystem.cpp`,
  étape 2 « Gravité effective ») :
  - Calcule `effectiveGravity` **exactement comme avant** (multiplicateurs chute/fast-fall/apex,
    inchangés).
  - Si `velocity.value.y >= 0` (chute) : nouvelle intégration newtonienne — `netAcceleration =
    effectiveGravity − (fallDragCoefficient × velocity.value.y) / player.mass` ; `velocity.value.y
    += netAcceleration × fixedDelta`. **Pas** de `std::min` après (la traînée borne naturellement).
  - Sinon (`velocity.value.y < 0`, montée) : intégration **inchangée** (`velocity.value.y +=
    effectiveGravity × fixedDelta`, pas de traînée) — le ressenti de saut (LOT-11) n'est pas
    affecté (voir décision de cadrage de l'épic).
- **`EX-GP-019`** : nouvelle exigence dans `Documentation/Specification/gameplay.md`, section 2
  (juste après `EX-GP-018`) — masse du personnage et modèle de chute newtonien (poids − traînée,
  vitesse terminale émergente).

## Fichiers impactés
- `Source/Core/Ecs/Components/Player.h`.
- `Source/Core/Physics/PhysicsConfig.h`.
- `Source/Core/Ecs/Systems/CharacterPhysicsSystem.cpp`.
- `Documentation/Specification/gameplay.md` (`EX-GP-019`).
- Tests : `Source/Test/Integration/test_physique_personnage.cpp` (mise à jour du test existant
  utilisant `maxFallSpeed` + nouveaux tests), `Source/Test/Unit/Core/Ecs/test_player_components.cpp`
  (mise à jour de `PhysicsConfigParDefautPlausible`, ajout d'une assertion sur `Player::mass`).

## Tests (obligatoires)
- **Vitesse terminale émergente** : après une chute suffisamment longue, `velocity.y` converge
  vers `≈ 25` (masse par défaut) sans jamais la dépasser.
- **Accélération décroissante** : l'augmentation de vitesse entre deux pas consécutifs **diminue**
  à mesure que la vitesse approche le régime permanent (courbe asymptotique, pas de plafond net) —
  distingue le nouveau modèle d'un simple clamp.
- **Masse plus grande → chute plus rapide** : à traînée égale, un personnage deux fois plus lourd
  atteint une vitesse terminale plus élevée (vérifiable en comparant deux simulations à masse
  différente après un temps de chute identique).
- **Non-régression du saut** : les tests existants sur la hauteur/durée de saut, l'apex, le
  fast-fall, le wall slide, le dash restent verts **sans modification** (seule la chute change).
- Test `NeTraversePasLeSolEnChuteRapide` (`test_physique_personnage.cpp`) : adapter
  la configuration « chute très rapide » (`fast.maxFallSpeed` n'existe plus) en fixant
  `fast.fallDragCoefficient` à une valeur quasi nulle (traînée négligeable → vitesse non bornée en
  pratique sur la durée du test), pour continuer à exercer le balayage continu (non-tunneling).

## Points d'attention
- **Ne pas appliquer la traînée à la montée** : c'est la garantie de non-régression du ressenti de
  saut (LOT-11). Le test au `velocity.y == 0` exact (apex) doit être **continu** entre les deux
  branches (vérifié : à `v=0`, les deux formules donnent `effectiveGravity`).
- `fallDragCoefficient` est en **1/s**, pas en unités/s² : une valeur trop grande sur-amortit la
  chute (vitesse terminale basse) ; trop petite, la traînée devient négligeable (comportement proche
  de l'ancien plafond, sans jamais le dépasser).
- La **division par `player.mass`** suppose `mass > 0` — aucune validation supplémentaire n'est
  ajoutée (donnée pure, `EX-ARCH-011` : la garantie relève de qui construit le composant, comme les
  autres champs de `Player`).

## Définition de fait (DoD)
- `Player::mass`, `PhysicsConfig::fallDragCoefficient` et le nouveau modèle de chute disponibles et
  testés (`ctest` vert) ; build `/W4 /WX` sans avertissement ; Doxygen à jour ; `EX-GP-019` déclarée.

## Exigences
`EX-GP-019` (nouvelle).
