# TACHE-03 — Plateforme mobile {#lot-63-tache-03-plateforme-mobile}

**Lot :** [LOT-63](epic.md) · **Emplacement :** `Source/Core/Gameplay`, `Source/Core/Physics` ·
**Statut :** fait (moteur Core ; câblage `hmi::GameSession`/rendu reporté à la TACHE-04/TACHE-05,
qui ont besoin d'y poser une entité et un habillage de toute façon)

## Contexte
`vision.md` range les plateformes mobiles dans la boîte à outils du genre, « (plus tard) ». C'est la
tâche à **risque** du lot, et la seule qui touche à la physique.

Toutes les collisions du jeu opposent jusqu'ici un personnage **mobile** à un décor **immobile** :
`core::sweptAabbVsTileMap` balaie une boîte contre une grille fixe. Une plateforme qui bouge inverse
le problème — le sol se déplace sous le personnage — et fait apparaître d'un coup toute la famille
de défauts classiques : le personnage qui ne suit pas la plateforme horizontale, qui est écrasé par
une plateforme montante, qui décolle d'une plateforme descendante, qui tremble à l'affichage parce
que sa position est interpolée et celle de la plateforme non.

`core::BlockController` (`LOT-21`) est le précédent le plus proche : une entité solide qui se
déplace dans la grille. Il faut s'en inspirer et en reprendre les invariants.

## Travail à réaliser
- **Type de tuile `MovingPlatform`** et sa description : deux points de parcours, vitesse, et
  éventuellement un déphasage pour désynchroniser plusieurs plateformes — sur le modèle du danger
  temporisé (`EX-GP-053`), qui a déjà résolu ce besoin.
- **Position fonction du numéro de pas**, jamais du temps réel : c'est ce qui garantit le
  déterminisme (`EX-NFR-002`) et la reproductibilité des rejeux.
- **Portage du personnage** : un personnage au sol sur une plateforme est déplacé **avec** elle,
  avant que sa propre physique ne s'applique, sans jamais être traversé ni enfoncé.
- **Portage des blocs poussables** : `core::BlockController` doit voir la plateforme comme un
  support mouvant.
- **Cas d'écrasement** : une plateforme montante contre un plafond, avec le personnage entre les
  deux. Décider — refus d'avancer, ou mort — et le documenter ; le cas ne doit pas être laissé au
  hasard de l'ordre de résolution.
- **Interpolation d'affichage** : la plateforme doit utiliser `hmi::PreviousPosition`, comme les
  blocs poussables, sans quoi le personnage tremble par rapport à son support.

## Fichiers impactés
- `Source/Core/Levels/TileType.h`, `TileTypeName.{h,cpp}`, `LevelLoader.cpp`, `LevelWriter.cpp`.
- `Source/Core/Gameplay/PlatformController.{h,cpp}` (nouveau).
- `Source/Core/Ecs/Systems/CharacterPhysicsSystem.cpp` — portage du personnage.
- `Source/Core/Gameplay/BlockController.cpp` — support mouvant.
- `Source/Test/Unit/Core/Gameplay/test_platform_controller.cpp` (nouveau).
- `Source/Test/Integration/test_physique_personnage.cpp` (étendu).

## Tests (obligatoires)
- **Déterminisme** : deux exécutions de la même séquence d'entrées donnent exactement les mêmes
  positions de plateforme et de personnage, à chaque pas. Test central.
- **Portage horizontal** : un personnage immobile sur une plateforme se déplace exactement avec
  elle, sans glissement cumulé sur cent pas.
- **Portage vertical** : ni enfoncement, ni décollement, en montée comme en descente.
- **Aucune traversée** : à la vitesse maximale admise pour une plateforme, le personnage n'est jamais
  traversé — c'est la garantie de `EX-GP-014`, à préserver pour un obstacle mobile.
- **Écrasement** : le cas plafond est traité conformément à la décision retenue, et testé.
- Un bloc poussable posé sur une plateforme est porté.
- Les niveaux livrés existants restent franchissables (test système inchangé).
- Tests `Core` purs, sans GPU.

## Points d'attention
- **L'ordre de résolution dans le pas est la décision structurante** : déplacer les plateformes,
  puis porter les entités posées dessus, puis appliquer la physique du personnage. Un autre ordre
  produit des défauts subtils et intermittents, très coûteux à diagnostiquer ensuite. Le fixer, le
  documenter, et le tester.
  **Retenu** : `core::PlatformController::update()` (position déterministe du pas) d'abord ;
  ensuite `core::CharacterPhysicsSystem::update(..., platforms)` — portage (translation directe si
  `Player::grounded`), *puis* résolution normale (grille), *puis* collision continue contre chaque
  plateforme (`core::sweepAabbVsAabb`, même patron que les blocs réduits) — et
  `core::BlockController::update(..., platforms)`, qui porte de même les blocs reposant dessus
  (accumulateur infra-case converti en poussée d'une case entière). **Écrasement** : décidé
  **mortel** (`Player::squished`, plutôt que de mettre la plateforme en pause, ce qui casserait sa
  position purement fonction du numéro de pas). Câblage dans la boucle de jeu vivante
  (`hmi::GameSession`, entité de rendu, interpolation d'affichage) volontairement laissé à la
  TACHE-04/TACHE-05, qui posent de toute façon une entité et un habillage pour les trois nouveaux
  mécanismes.
- **Ne pas exclure les plateformes du balayage.** Le piège déjà rencontré sur le suivi de pente
  vaut ici : une boîte pleine case peut être happée par une colonne solide voisine. Les tests
  précis doivent utiliser `core::playerSize()`, pas une taille supposée.
- **Position par numéro de pas, pas par accumulation.** Accumuler `position += vitesse * dt` à
  chaque pas fait dériver en virgule flottante et casse la reproductibilité au bout d'un temps de
  jeu suffisant.
- L'interpolation d'affichage est déjà résolue pour les blocs poussables : reprendre le même
  mécanisme, ne pas en écrire un second.
- Si cette tâche déborde, **elle est celle qu'on retire du lot** : la clé et l'action « Interagir »
  se livrent sans elle.

## Définition de fait (DoD)
- Une plateforme mobile se déplace de façon déterministe entre deux points, porte le personnage et
  les blocs sans traversée, glissement ni tremblement, traite explicitement le cas d'écrasement,
  respecte un ordre de résolution documenté et testé, et laisse les niveaux existants franchissables ;
  `/W4 /WX` propre.

## Exigences
`EX-GP-026` (plateforme mobile) ; réutilise `EX-GP-014` (collisions sans traversée), `EX-GP-022`
(blocs poussables), `EX-GP-053` (déphasage, patron repris), `EX-NFR-002` (déterminisme),
`EX-REN-021` (pas fixe), `EX-LVL-002`/`EX-LVL-004` (format et validation), `EX-NFR-020` (tests).
