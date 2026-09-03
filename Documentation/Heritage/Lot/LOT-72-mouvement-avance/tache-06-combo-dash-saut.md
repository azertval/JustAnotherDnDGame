# TACHE-06 — Combo dash + saut maximisé {#lot-72-tache-06-combo-dash-saut}

**Lot :** [LOT-72](epic.md) · **Emplacement :** `Source/Core/Ecs/Systems` · **Statut :** fait

## Contexte
`EX-GP-061` est le lot le plus riche des enchaînements demandés : faire composer le dash (TACHE-01),
la poussée renforcée (TACHE-02), le saut et le wall-jump existants (`EX-GP-013`/`EX-GP-016`) sans
nouvelle touche, avec un plafond explicite pour ne pas casser l'équilibrage des niveaux.

**Décision de cadrage prise en cours d'implémentation** : le jump-cancel (et, par cohérence, la
poussée renforcée du bloc, TACHE-02) est restreint au dash **boosté** (`Player::dashIsBoosted`,
armé par la charge de TACHE-01) — jamais à un dash normal. Un premier essai sans cette restriction
a **cassé la séquence `demo-final`** (`ParcoursCompletSysteme.FranchitTouteLaSequence`,
`RecompenseDemoNiveauxTest`) : un saut pressé pendant un dash normal était auparavant simplement
**bufferisé** (honoré à l'expiration naturelle du dash) ; le jump-cancel le remplaçait par une
interruption immédiate, modifiant la trajectoire de tout contenu existant qui enchaîne dash puis
saut sans intention particulière de « hyper-dash ». Restreindre au dash boosté élimine ce risque
**par construction** : aucun contenu antérieur à ce lot ne peut produire de dash boosté (la charge
n'existait pas), donc `dashTimer > 0 && dashIsBoosted` ne peut jamais être vrai pour lui. Un dash
**normal** se comporte donc exactement comme avant ce lot, y compris avec un saut pressé pendant sa
durée.

## Travail à réaliser
- `Player.h` :
  - `momentumWindowTimer` (fenêtre de combo après une poussée renforcée) et une vitesse horizontale
    mémorisée à hériter.
  - `comboChainCount` (compteur d'enchaînements de jump-cancels rapprochés), remis à zéro au contact
    du sol ou à l'expiration de la fenêtre de combo.
- `PhysicsConfig` : `momentumCarryRatio` (fraction de la vitesse du bloc héritée au saut),
  `comboSpeedBonus` (bonus par enchaînement), `comboSpeedCap` (plafond cumulé), fenêtre de combo
  (`comboWindowTime`).
- `CharacterPhysicsSystem` :
  - **Jump-cancel** : si un saut est déclenché (`jumpPressed`/buffer) pendant `dashTimer > 0` **et**
    `player.dashIsBoosted` (jamais pour un dash normal, voir Contexte), mettre `dashTimer = 0`
    immédiatement et **conserver** la vitesse horizontale du dash (ne pas la remplacer par la
    vitesse de saut/marche normale).
  - **Wall-jump en sortie de dash** : si ce jump-cancel a lieu avec `wallDirection != 0`, router vers
    la logique de wall-jump existante (réutilisée telle quelle) plutôt qu'un saut simple.
  - **Momentum hérité après poussée** : quand TACHE-02 déclenche une poussée renforcée, armer
    `momentumWindowTimer` et mémoriser la vitesse horizontale du bloc ; un saut déclenché avant
    expiration ajoute `momentumCarryRatio × vitesse mémorisée` à la vitesse horizontale du saut.
  - **Bonus cumulatif plafonné** : chaque jump-cancel réussi dans `comboWindowTime` après le
    précédent incrémente `comboChainCount` et ajoute `min(comboSpeedBonus × comboChainCount,
    comboSpeedCap)` à la vitesse horizontale du saut résultant.
- Ne créer **aucune** charge de dash ni budget supplémentaire : le nombre d'enchaînements possibles
  reste borné par `dashChargesRemaining`/`dashesRemaining` (LOT-67) déjà existants.

## Fichiers impactés
- `Source/Core/Ecs/Components/Player.h`.
- `Source/Core/Physics/PhysicsConfig.h`.
- `Source/Core/Ecs/Systems/CharacterPhysicsSystem.h`/`.cpp`.
- Tests d'intégration.

## Tests (obligatoires)
- **Jump-cancel** : sauter pendant un dash coupe immédiatement le dash et conserve sa vitesse
  horizontale (mesurable, supérieure à un saut depuis l'arrêt).
- **Wall-jump en sortie de dash** : jump-cancel au contact d'un mur déclenche un wall-jump (vitesse
  d'éjection, verrouillage `wallJumpLockTimer`) et non un saut simple.
- **Momentum hérité** : un saut dans la fenêtre suivant une poussée renforcée hérite du ratio de
  vitesse configuré ; hors fenêtre, aucun héritage.
- **Bonus cumulatif et plafond** : des jump-cancels rapprochés augmentent la vitesse jusqu'au plafond
  `comboSpeedCap`, jamais au-delà.
- **Remise à zéro** : le compteur de combo retombe à zéro au contact du sol et hors fenêtre.
- **Borne par les charges existantes** : le nombre de dashs enchaînables reste limité par
  `dashChargesRemaining`/`dashesRemaining`, inchangés par cette tâche.
- **Déterminisme** (`EX-NFR-002`).

## Points d'attention
- Le jump-cancel doit interagir proprement avec le **coyote time**/**jump buffering** existants :
  un saut bufferisé juste avant l'atterrissage ne doit pas être traité deux fois.
- Le plafond `comboSpeedCap` est une valeur de conception à ajuster par playtest ; documenter la
  valeur de départ choisie dans `PhysicsConfig.h`.
- Réutiliser la logique de wall-jump existante telle quelle (pas de duplication de son calcul de
  vitesse d'éjection).

## Définition de fait (DoD)
- Combo dash + saut fonctionnel et **testé** (`ctest` vert) ; build `/W4 /WX`.

## Exigences
`EX-GP-061`, `EX-GP-013`, `EX-GP-016`, `EX-GP-017`, `EX-GP-055`, `EX-GP-057`, `EX-NFR-002`,
`EX-ARCH-011`.
