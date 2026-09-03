# TACHE-03 — Double saut (sauts aériens rechargés au sol) {#lot-10-tache-03-double-saut}

**Lot :** [LOT-10](epic.md) · **Emplacement :** `Source/Core/Ecs/Systems` · **Statut :** à faire

## Contexte
Première mécanique : autoriser un nombre **paramétrable** de sauts **aériens** en plus du saut au
sol (`EX-GP-015`), rechargés au contact du sol. Étend la logique de saut du LOT-09 sans casser le
coyote/buffer ni la hauteur variable.

## Travail à réaliser
Dans `CharacterPhysicsSystem::update` :
- **Recharge** : quand le personnage est **au sol**, `airJumpsRemaining = _config.airJumps`.
- **Saut au sol / coyote** (LOT-09) : inchangé.
- **Saut aérien** : si un saut est demandé (buffer) alors que le personnage **n'est ni au sol ni en
  coyote** mais qu'il reste `airJumpsRemaining > 0`, appliquer l'impulsion (`-jumpSpeed`) et
  **décrémenter** `airJumpsRemaining`. Consommer le buffer.
- Unifier avec le déclenchement existant : une **seule** décision « peut sauter maintenant ? »
  couvrant sol/coyote **ou** saut aérien restant.
- La **hauteur variable** s'applique aussi au saut aérien.

## Fichiers impactés
- `Source/Core/Ecs/Systems/CharacterPhysicsSystem.h`/`.cpp`.
- Tests d'intégration.

## Tests (obligatoires)
- **Double saut** : au sol → saut ; en l'air → un **second** saut fonctionne (regain de vitesse
  ascendante) ; un **troisième** en l'air ne fait rien (avec `airJumps = 1`).
- **Recharge au sol** : après avoir atterri, deux sauts sont de nouveau disponibles.
- **Paramétrable** : avec `airJumps = 2`, trois sauts au total sont possibles avant de retoucher le
  sol.
- **Déterminisme** (`EX-NFR-002`).

## Points d'attention
- **Ordre** : recharge au sol avant l'évaluation du saut ; ne pas re-remplir en l'air.
- Ne pas régresser le **coyote time** : coyote et saut aérien sont deux sources d'autorisation
  distinctes ; le saut au sol/coyote ne doit pas consommer un saut aérien.
- Logique dans le système, état dans `Player` (`EX-ARCH-011`).

## Définition de fait (DoD)
- Double saut fonctionnel et **testé** (`ctest` vert) ; build `/W4 /WX`.

## Exigences
`EX-GP-015`, `EX-GP-011`, `EX-NFR-002`, `EX-ARCH-011`.
