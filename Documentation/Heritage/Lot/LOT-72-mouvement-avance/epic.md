# LOT-72 — Mouvement avancé (dash chargé, poussée, pentes, ground pound, combo) {#lot-72}

> Statut : **fait** (vérification automatisée : build Debug, `ctest` à 1585/1585, lint d'exigences
> vert ; la vérification IHM manuelle du ressenti en jeu — boost, poussée, ground pound, combo —
> reste à faire par l'utilisateur, comme pour tout lot touchant à la physique du personnage).
> Prérequis : [LOT-10](@ref lot-10) (mécaniques avancées : dash, wall jump), [LOT-67](@ref lot-67)
> (charges de dash et budgets par tableau).

## Objectif

Enrichir le nuancier de mouvement du personnage en complexifiant le **dash** existant et en le
faisant composer avec les autres mécaniques (poussée de bloc, saut, mur), sans ajouter de nouvelle
touche.

Le dash (`EX-GP-017`, LOT-10) est déjà en 8 directions et dispose de charges par saut et d'un budget
par tableau (`EX-GP-055`/`EX-GP-024`, LOT-67). Mais deux trous subsistaient : la poussée de bloc
(`EX-GP-022`) ne déplace qu'une case par contact, identique en marche ou en dash ; et sauter pendant
un dash, ou juste après une poussée, ne conservait aucune vitesse (retour immédiat à la physique
normale). Ce lot les comble en réutilisant l'existant (directions du dash, `wallDirection`, charges/
budget de dash) plutôt que d'en dupliquer l'infrastructure — et corrige au passage deux hypothèses de
cadrage erronées, découvertes en relisant le code avant d'écrire quoi que ce soit (voir
« Déjà couvert » ci-dessous).

**Un premier essai a cassé `demo-final`** (voir TACHE-01/TACHE-06) : la charge de dash, telle que
cadrée initialement (maintenir la direction opposée à `facing`), pouvait s'armer sur une simple
inversion de direction en cours de déplacement normal, sans aucune intention de dasher —
`ScriptedLevelSequence` (démo) et l'espace d'action de l'IA en contiennent forcément. La charge
exige donc **aussi** de maintenir le bouton de dash (`PlayerInput::dashHeld`, nouveau champ dérivé
de la touche déjà existante, même patron que `jumpHeld`) ; et le jump-cancel/la poussée renforcée
sont restreints au dash **boosté** (`Player::dashIsBoosted`), jamais à un dash normal — garantie de
non-régression par construction, puisqu'aucun contenu antérieur à ce lot ne peut produire de dash
boosté.

## Périmètre

### Inclus
- **Dash chargé** (`EX-GP-056`) : maintenir le bouton de dash **et** la direction opposée avant de
  dasher charge un boost de vitesse/durée.
- **Poussée renforcée par un dash boosté** (`EX-GP-057`) : un bloc percuté pendant un dash **boosté**
  est repoussé de plusieurs cases d'un coup ; un dash normal pousse comme avant ce lot.
- **Ground pound** (`EX-GP-058`) : chute accélérée dirigée en l'air, réservée aux cas où aucune
  charge de dash n'est disponible (sinon la même entrée reste un dash vertical normal).
- **Combo dash + saut** (`EX-GP-061`) : jump-cancel d'un dash **boosté** (conservation de vitesse),
  wall-jump en sortie de dash, momentum hérité après poussée renforcée, bonus cumulatif plafonné.
- Validation explicite du **dash diagonal** (déjà livré en LOT-10) avec chacune des nouvelles
  mécaniques (non-régression).

### Déjà couvert avant ce lot (relecture de code, pas de nouveau travail)
- **Wall slide** (`EX-GP-060` initialement envisagée) : `EX-GP-016` (LOT-10) couvre déjà le clamp de
  chute au contact d'un mur en l'air. Aucune exigence supplémentaire créée ; seule la non-régression
  avec les nouvelles mécaniques est testée (TACHE-05).
- **Dash et pentes** (`EX-GP-060`) : la passe de suivi de pente/plafond
  (`core::resolveSlopeFollow`/`resolveCeilingSlopeFollow`) s'applique déjà à **chaque** pas de
  simulation, dash ou non (`CharacterPhysicsSystem::resolveCollisionAndState`, appelée
  inconditionnellement) — un dash suit donc déjà les pentes sans code supplémentaire. La « glissade
  de sortie » initialement envisagée n'a pas de sens séparé : le contrôle normal reprend après le
  dash et le suivi de pente s'y applique comme à tout déplacement. `EX-GP-060` documente et **teste**
  ce comportement plutôt que d'en ajouter un (TACHE-03).

### Exclus
- **Casse de blocs fragiles** au ground pound : nécessiterait un nouveau `TileType` (aucun bloc
  fragile n'existe dans `Source/Core/Levels/TileType.h`), avec support éditeur/rendu — hors périmètre
  d'un lot de mouvement. L'atterrissage d'un ground pound déclenche la secousse caméra déjà existante
  pour tout impact lourd (`hmi::GameSession`), sans code dédié.
- Aucune nouvelle action de contrôle (`Documentation/Specification/controles.md` inchangé — `dashHeld`
  dérive la touche de dash déjà mappée, comme `jumpHeld` pour le saut), aucun redimensionnement de
  bloc.
- Poussée/knockback sur des ennemis ou dangers : seuls les blocs poussables sont concernés.
- Réimplémentation du dash diagonal lui-même : déjà livré, seule sa non-régression est garantie.

## Décisions de cadrage

- **Charge de dash** = maintenir **le bouton de dash** (`PlayerInput::dashHeld`) **et** la direction
  opposée à une référence figée au début de la charge (`Player::dashChargeReferenceFacing`, copie de
  `facing` — jamais `facing` lui-même, qui continue de se mettre à jour à l'identique d'avant ce
  lot). La garde `dashHeld` est délibérée : sans elle, une inversion de direction anodine suffirait
  à armer un boost sans rapport avec une intention de dasher (voir Objectif).
- **Le boost ne crée pas de charge de dash supplémentaire** : il modifie seulement vitesse/durée du
  dash consommé normalement (`dashChargesRemaining`/`dashesRemaining` de LOT-67 inchangés dans leur
  fonctionnement).
- **Poussée renforcée et jump-cancel, restreints au dash boosté** (`Player::dashIsBoosted`) : un
  dash normal se comporte exactement comme avant ce lot dans les deux cas (poussée d'une case,
  saut pressé pendant le dash simplement bufferisé jusqu'à son expiration).
- **Poussée renforcée** = déplacement multi-cases dans le même pas fixe (jusqu'à
  `BlockController::DASH_PUSH_MAX_CELLS`, ou au premier obstacle), en réutilisant `isFree` de
  `BlockController`, pas une nouvelle simulation physique.
- **Ground pound** = nouvel état `Player::groundPounding`, armé par le bouton de dash visé
  **purement vers le bas** en l'air **sans charge de dash disponible** (`dashChargesRemaining <= 0`
  — tant qu'une charge existe, la même entrée reste un dash vertical normal, aucune régression sur un
  usage existant), vitesse de chute imposée (`PhysicsConfig::groundPoundSpeed`).
- **Combo dash + saut**, quatre briques qui composent avec l'existant sans nouvelle touche :
  - **Jump-cancel** (dash boosté uniquement) : un saut pendant `dashTimer > 0 && dashIsBoosted` met
    fin au dash immédiatement et conserve sa vitesse horizontale au lieu de revenir à la physique
    normale.
  - **Wall-jump en sortie de dash** : si le jump-cancel a lieu au contact d'un mur, c'est un
    wall-jump qui se déclenche (réutilise `EX-GP-016` tel quel), pas un saut simple.
  - **Momentum hérité après poussée** : un saut dans une courte fenêtre après une poussée renforcée
    hérite d'un pourcentage configurable (`PhysicsConfig::momentumCarryRatio`) de la vitesse
    horizontale du bloc.
  - **Enchaînement plafonné** : chaque jump-cancel réussi dans une fenêtre courte après le précédent
    ajoute un bonus de vitesse cumulatif (`PhysicsConfig::comboSpeedBonus`), plafonné
    (`PhysicsConfig::comboSpeedCap`) ; le compteur de combo retombe à zéro au contact du sol.
- **Déterminisme** : tout nouvel état/minuterie est décompté au pas fixe comme `coyoteTimer`/
  `dashTimer`, conforme à `EX-NFR-002`.

## Exigences couvertes

`EX-GP-056`, `EX-GP-057`, `EX-GP-058`, `EX-GP-060`, `EX-GP-061` (nouvelles). Réutilisées :
`EX-GP-003`, `EX-GP-004`, `EX-GP-006`, `EX-GP-007` (pentes/arrondis et suivi), `EX-GP-016`
(`wallDirection`, wall jump/slide), `EX-GP-017` (dash), `EX-GP-022` (blocs poussables), `EX-GP-024`,
`EX-GP-025` (plaques de pression), `EX-GP-055` (charges de dash par tableau), `EX-ARCH-011`,
`EX-NFR-002`.

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| TACHE-01 | Charge de dash et dash boosté | `Source/Core/{Ecs,Physics}`, `Source/HMI/Input` | ✅ |
| TACHE-02 | Poussée renforcée pendant un dash boosté | `Source/Core/Gameplay/BlockController.*`, `Source/HMI/Game` | ✅ |
| TACHE-03 | Dash et pentes (déjà couvert, validation) | `Source/Test/Integration` | ✅ |
| TACHE-04 | Ground pound | `Source/Core/Ecs/{Components,Systems}`, `Source/Core/Physics` | ✅ |
| TACHE-05 | Wall slide (déjà livré, validation croisée) | `Source/Test/Integration` | ✅ |
| TACHE-06 | Combo dash + saut (jump-cancel, wall-jump, momentum, bonus) | `Source/Core/Ecs/Systems` | ✅ |
| TACHE-07 | Validation croisée et tests | `Source/Test/{Integration,Unit}` | ✅ |
| TACHE-08 | Documentation, exigences, CHANGELOG | `Documentation/Specification`, `CHANGELOG.md` | ✅ |

## Critères d'acceptation du lot

1. Sans déclencher aucune des nouvelles mécaniques, le personnage se comporte **exactement** comme
   avant le lot (build `ctest` 1585/1585, y compris `demo-final`, `RecompenseDemoNiveauxTest`,
   `RejeuIaSysteme`, inchangés).
2. Dash chargé : maintenir le bouton de dash et la direction opposée au-delà du seuil puis dasher
   produit un dash mesurablement plus rapide/long, dans les 8 directions y compris diagonales ; un
   simple changement de direction sans tenir le bouton de dash n'amorce jamais de charge.
3. Poussée renforcée : un dash **boosté** contre un bloc le déplace de plusieurs cases en un pas
   fixe, sans traversée d'obstacle ; un dash normal continue de pousser d'une seule case.
4. Dash sur pente : dasher le long d'une pente suit sa surface (ni clip, ni arrêt net non voulu).
5. Ground pound : déclenché uniquement en l'air et sans charge de dash disponible, vitesse de chute
   imposée, ne casse pas l'interaction existante avec les plaques de pression.
6. Combo dash + saut : un saut pendant un dash **boosté** le coupe et conserve sa vitesse horizontale
   (jump-cancel) ; s'il a lieu contre un mur, c'est un wall-jump qui se déclenche ; un saut dans la
   fenêtre suivant une poussée renforcée hérite du ratio de vitesse configuré ; un enchaînement de
   jump-cancels rapprochés cumule un bonus de vitesse plafonné.
7. `ctest` à 100 %, lint d'exigences vert, build `/W4 /WX`, déterminisme conservé (`EX-NFR-002`).

## Dépendances

Aucun lot ne dépend de celui-ci. Il s'appuie sur le dash et le wall jump du `LOT-10`, sur les charges
de dash par tableau du `LOT-67`, et sur le suivi de pente introduit au `LOT-08`/`LOT-19`.

## Navigation des tâches

- @subpage lot-72-tache-01-dash-charge
- @subpage lot-72-tache-02-poussee-renforcee
- @subpage lot-72-tache-03-dash-pentes
- @subpage lot-72-tache-04-ground-pound
- @subpage lot-72-tache-05-wall-slide
- @subpage lot-72-tache-06-combo-dash-saut
- @subpage lot-72-tache-07-validation-croisee
- @subpage lot-72-tache-08-documentation
