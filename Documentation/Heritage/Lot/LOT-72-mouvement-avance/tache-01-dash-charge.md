# TACHE-01 — Charge de dash et dash boosté {#lot-72-tache-01-dash-charge}

**Lot :** [LOT-72](epic.md) · **Emplacement :** `Source/Core/Ecs/{Components,Systems}`,
`Source/Core/Physics` · **Statut :** fait

## Contexte
Le dash (`EX-GP-017`) est aujourd'hui à vitesse et durée fixes. `EX-GP-056` ajoute un **dash chargé**
: maintenir la direction opposée à celle du prochain dash pendant un court délai, avant de dasher,
donne un **dash boosté** — plus rapide et/ou plus long qu'un dash normal.

**Garde `dashHeld` ajoutée en cours d'implémentation** : la charge exige de maintenir **aussi** le
bouton de dash (`PlayerInput::dashHeld`, nouveau champ dérivé de la touche de dash déjà existante —
même patron que `jumpHeld`, aucune nouvelle touche), pas seulement la direction opposée. Un premier
essai sans cette garde a **cassé la séquence `demo-final`** et les tests IA (`RecompenseDemoNiveauxTest`,
`ParcoursCompletSysteme`) : une simple inversion de direction en cours de déplacement normal (sans
aucune intention de charger un dash) suffisait à armer un boost, qui changeait ensuite la
vitesse/durée du dash suivant. `ScriptedLevelSequence`/l'espace d'action de l'IA ne renseignent
jamais `dashHeld` (toujours `false`), donc aucun contenu existant ne peut plus jamais amorcer de
charge — garantie de non-régression **par construction**.

## Travail réalisé
- `PlayerInput.h` : `dashHeld` (bouton de dash maintenu, garde de la charge).
- `Player.h` : `dashChargeTimer`, `dashChargeReferenceFacing` (copie figée de `facing` au début de
  la charge, jamais `facing` lui-même — voir Points d'attention), `dashBoostReady`,
  `dashBoostFacing` (copie figée à l'armement), `dashIsBoosted` (vrai pour le dash en cours si
  déclenché boosté).
- `PhysicsConfig.h` : `dashChargeHoldTime`, `dashBoostSpeedMultiplier`, `dashBoostDurationMultiplier`.
- `Source/HMI/Input/PlayerInputMapper.cpp` : dérive `dashHeld` de la touche de dash (`keyDown`/
  `gamepadButtonDown`), même patron que `jumpHeld`.
- `CharacterPhysicsSystem::resolveVelocity` : chaque pas, si `input.dashHeld` **et** l'entrée
  horizontale est opposée à `dashChargeReferenceFacing` (figée au début de la charge), incrémenter
  `dashChargeTimer` ; sinon le remettre à zéro (sauf charge déjà bankée). Dès que
  `dashChargeTimer >= dashChargeHoldTime`, `dashBoostReady = true` et `dashBoostFacing` capture la
  référence.
- `CharacterPhysicsSystem::applyDash` : si `dashBoostReady` au déclenchement, applique `dashSpeed`/
  `dashDuration` majorés, pose `dashIsBoosted = true`, puis remet `dashBoostReady`/
  `dashChargeTimer` à zéro (la charge est consommée qu'elle serve ou non au boost).
- Ne touche pas à `dashChargesRemaining`/`dashesRemaining` (LOT-67) : le boost ne consomme et ne
  crée aucune charge supplémentaire, seulement le dash normalement déclenché.

## Fichiers impactés
- `Source/Core/Ecs/Components/Player.h`.
- `Source/Core/Physics/PhysicsConfig.h`.
- `Source/Core/Ecs/Systems/CharacterPhysicsSystem.h`/`.cpp`.
- Tests d'intégration.

## Tests (obligatoires)
- **Charge puis dash** : maintenir la direction opposée au-delà du seuil, puis dasher → vitesse/durée
  supérieures à un dash normal, dans les 8 directions (y compris diagonales).
- **Charge insuffisante** : dasher avant le seuil → dash normal (pas de boost).
- **Charge annulée** : relâcher ou changer de direction avant le seuil → la charge suivante repart de
  zéro.
- **Non-consommation des charges/budget** : `dashChargesRemaining`/`dashesRemaining` se comportent
  exactement comme avant le lot, boost ou non.
- **Déterminisme** (`EX-NFR-002`).

## Points d'attention
- La détection de la direction opposée se fait sur `PlayerInput::moveX` comparé à `Player::facing`
  **avant** que le dash ne mette potentiellement à jour `facing` — ne pas inverser l'ordre.
- Le boost doit rester compatible avec un dash diagonal (direction normalisée comme aujourd'hui).
- Réutiliser le balayage existant (LOT-08) pour la résolution physique du dash boosté ; ne pas
  réimplémenter la collision.

## Définition de fait (DoD)
- Dash chargé fonctionnel et **testé** (`ctest` vert) ; build `/W4 /WX`.

## Exigences
`EX-GP-056`, `EX-GP-017`, `EX-GP-055`, `EX-NFR-002`, `EX-ARCH-011`.
