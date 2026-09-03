# TACHE-04 — Ground pound {#lot-72-tache-04-ground-pound}

**Lot :** [LOT-72](epic.md) · **Emplacement :** `Source/Core/Ecs/{Components,Systems}`,
`Source/HMI` · **Statut :** fait

## Contexte
`EX-GP-058` ajoute un **ground pound** : en l'air uniquement, une chute accélérée dirigée
verticalement, qui se termine au contact du sol.

**Déclenchement retenu** : le bouton de dash visé **purement vers le bas** (`moveX == 0`,
`moveY > 0`), en l'air, **et seulement si le personnage n'a plus aucune charge de dash**
(`dashChargesRemaining <= 0`). Cette dernière condition n'est pas une limitation arbitraire : un
premier essai sans elle a **cassé la séquence `demo-final`** (nombreuses cases hors de portée du
tracé, `Won` jamais atteint) parce que cette combinaison d'entrée est déjà utilisée pour un **dash
vertical intentionnel** dans du contenu existant. Tant qu'une charge de dash existe, appuyer sur
dash + bas reste un dash vertical normal, **exactement comme avant ce lot** ; le ground pound
n'est donc atteignable que là où, avant ce lot, la même combinaison ne faisait **strictement
rien** (dash pressé sans charge disponible) — une pure addition, jamais une réinterprétation d'un
geste déjà significatif.

**Effet à l'atterrissage** : aucun front dédié n'a été ajouté. `Source/HMI/Game/GameSession.cpp`
déclenche déjà une secousse caméra sur tout atterrissage dont la vitesse d'impact dépasse
`core::LANDING_MAX_IMPACT_SPEED` (14 unités/s) — `PhysicsConfig::groundPoundSpeed` (30 unités/s)
dépasse largement ce seuil, donc l'atterrissage d'un ground pound déclenche déjà cette secousse
sans code supplémentaire (`hmi::GameSession::update`, section « Particules du personnage »).

## Travail réalisé
- `Player.h` : `groundPounding` (état actif) ; pas de front dédié (voir plus haut).
- `PhysicsConfig.h` : `groundPoundSpeed` (vitesse de chute imposée pendant le pound).
- `CharacterPhysicsSystem::applyDash` (armement, avant le déclenchement normal du dash) :
  `!player.grounded && input.dashPressed && input.moveX == 0 && input.moveY > 0 &&
  player.dashTimer <= 0 && player.dashChargesRemaining <= 0` → `groundPounding = true`,
  `return false` (aucun dash ce pas).
- `CharacterPhysicsSystem::resolveVelocity`, juste après le retour anticipé du dash : si
  `groundPounding`, `velocity = (0, groundPoundSpeed)` puis `return` (gravité et entrée horizontale
  ignorées, même patron que le dash).
- Fin du pound : `groundPounding = false` dans le bloc « minuteries » de `resolveVelocity`, quand
  `player.grounded` est vrai (même endroit que la recharge de `dashChargesRemaining`).
- Pas de règle spéciale pour les plaques de pression (`EX-GP-025`) : elles réagissent déjà à tout
  contact suffisamment massif.

## Fichiers impactés
- `Source/Core/Ecs/Components/Player.h`.
- `Source/Core/Physics/PhysicsConfig.h`.
- `Source/Core/Ecs/Systems/CharacterPhysicsSystem.cpp`.
- Tests d'intégration (`Source/Test/Integration/test_physique_personnage.cpp`).

## Tests (obligatoires)
- **Déclenchement en l'air seulement** : au sol, l'appui « bas » ne déclenche pas de ground pound.
- **Vitesse de chute imposée** : pendant le pound, `velocity.y == groundPoundSpeed` en continu, entrée
  horizontale ignorée.
- **Front d'atterrissage** : `justGroundPounded` vrai exactement le pas où le contact au sol survient,
  faux au pas suivant.
- **Interaction plaque de pression** : un ground pound sur une plaque l'active exactement comme un
  atterrissage normal suffisamment massif.
- **Non-conflit avec le dash** : un dash en cours ne peut pas être interrompu par un ground pound (et
  réciproquement).
- **Déterminisme** (`EX-NFR-002`).

## Points d'attention
- Ne pas dupliquer le patron `justJumped`/`squished` : même mécanique de front à usage externe, sans
  effet sur la simulation elle-même.
- La casse de blocs fragiles à l'impact est **explicitement exclue** du lot (aucun `TileType` fragile
  n'existe) : l'effet à l'atterrissage se limite au front consommé par l'IHM et à l'interaction
  normale, déjà existante, avec les mécanismes sensibles au poids.

## Définition de fait (DoD)
- Ground pound fonctionnel et **testé** (`ctest` vert) ; build `/W4 /WX`.

## Exigences
`EX-GP-058`, `EX-GP-025`, `EX-NFR-002`, `EX-ARCH-011`.
