# TACHE-05 — Bloc éphémère : disparition au départ du personnage {#lot-74-tache-05-bloc-ephemere}

**Lot :** [LOT-74](epic.md) · **Emplacement :**
`Source/Core/Gameplay/VolatileBlockController.{h,cpp}` · **Statut :** fait

## Contexte
`EX-GP-029` : le bloc éphémère est solide ; une fois que le personnage s'y est posé **puis l'a
quitté**, il disparaît après un court délai, définitivement jusqu'au rechargement.

C'est la seconde règle de retrait du `VolatileBlockController` introduit en TACHE-04 — même overlay,
même mécanique de disparition, déclencheur différent. La difficulté n'est pas la disparition : c'est
la **détection du départ**.

Le moteur ne conserve aucun état de contact persistant. `player.grounded` et `player.wallDirection`
sont **entièrement recalculés** à chaque pas fixe depuis la géométrie
(`core::CharacterPhysicsSystem::resolveCollisionAndState`) : quitter un bloc, ce n'est pas un
événement, c'est simplement une normale qui n'est plus produite au pas suivant. Il n'existe par
ailleurs nulle part de notion « le personnage touche le bloc n° i ».

Le patron du dépôt pour ce genre de question est la **détection de front** : `hmi::PlayerEventState`
échantillonne l'état à chaque pas et `hmi::detectPlayerEvents` compare deux échantillons successifs
pour en tirer `Landed`, `Jumped`, `WallContactEnter` (`Source/HMI/Game/GameEvents.h`). Le contrôleur
fait la même chose, par bloc.

## Travail à réaliser
- Dans `core::VolatileBlockController`, mémoriser par bloc éphémère un booléen « le personnage
  reposait-il dessus au pas précédent », évalué avec `core::restsOnTopOfPlatform` (le test de
  portage, pas une intersection quelconque : passer **sous** un bloc éphémère ne doit rien armer).
- Déclencher sur la transition `true → false` : démarrer un compte à rebours en **pas fixes entiers**
  (`VANISH_DELAY_STEPS`, constante nommée et documentée du contrôleur, à l'image de
  `BlockController::FALL_INTERVAL_STEPS`), au terme duquel le bloc quitte l'overlay de collision,
  définitivement.
- Le délai est une **constante du moteur**, pas un champ de niveau (décision de TACHE-01) : un délai
  réglable par tuile multiplierait les cas de conception sans besoin identifié, et il est toujours
  possible de l'ouvrir plus tard sans casser les fichiers existants.
- Exposer, comme pour le bloc fragile, les blocs qui disparaissent **au pas courant** et, si utile à
  l'IHM, le fait qu'un bloc est en compte à rebours — pour un clignotement d'avertissement rendu en
  TACHE-06. Le contrôleur ne dessine ni ne sonne rien lui-même.

## Fichiers impactés
- `Source/Core/Gameplay/VolatileBlockController.{h,cpp}` (créés en TACHE-04),
  `Source/Core/Gameplay/README.md`.
- Câblage et rendu du compte à rebours : voir TACHE-06.

## Tests (obligatoires)
- **Rien tant qu'on reste dessus** : le personnage se tient sur le bloc pendant un nombre de pas très
  supérieur au délai — le bloc reste solide. Le compte à rebours ne démarre qu'au départ.
- **Front de départ** : le compte à rebours démarre exactement au pas où le personnage cesse de
  reposer dessus, et le bloc quitte la collision exactement `VANISH_DELAY_STEPS` pas plus tard.
- **Passer dessous ne l'arme pas** ; le toucher par le côté non plus.
- **Aller-retour** : revenir sur le bloc pendant le compte à rebours — décider et **tester** la règle
  retenue (le retour n'annule pas la disparition, cohérent avec « aller simple » du bloc descendant
  et avec un bloc qui « s'effrite »), et l'écrire dans `EX-GP-029`.
- **Disparition définitive** : il ne revient pas ; il revient après `reload()`.
- **Déterminisme** (`EX-NFR-002`) : le compte à rebours est en pas entiers, pas en secondes.

## Points d'attention
- **Ne pas réintroduire un état de contact dans `core::Player`.** La question « sur quel bloc suis-je
  ? » se résout côté contrôleur, contre la boîte du personnage, comme `BlockController` teste déjà la
  poussée dans ce sens-là. Ajouter un identifiant de bloc au personnage ferait fuir une notion de
  contenu dans un composant de physique.
- **Risque de level design assumé** : un bloc éphémère mal placé peut rendre un tableau insoluble, et
  la disparition est définitive. C'est précisément ce que `ParcoursCompletSysteme` détecte, en
  relevant la trajectoire **réelle** et non la seule présence des tuiles — ne pas contourner ce test
  si le tableau de TACHE-08 le fait échouer : c'est le tableau qu'il faut corriger.
- Le clignotement d'avertissement est un **confort de lisibilité**, pas une mécanique : s'il n'est
  pas livré, le bloc reste jouable, seulement plus sec. Ne pas le laisser bloquer la tâche.
- Deux blocs éphémères adjacents doivent se compter séparément : le tableau d'état est par bloc,
  jamais global.

## Définition de fait (DoD)
- Disparition fonctionnelle et **testée** (front de départ, non-armement par le dessous, caractère
  définitif, déterminisme) ; build `/W4 /WX`.

## Exigences
`EX-GP-029`, `EX-NFR-002`, `EX-NFR-021`, `EX-ARCH-011`.
