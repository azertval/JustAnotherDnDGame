# TACHE-02 — `AnimationSystem` généralisé {#lot-46-tache-02-systeme-generalise}

**Lot :** [LOT-46](epic.md) · **Emplacement :** `Source/Core/Ecs/Systems` · **Statut :** fait

## Contexte
`core::AnimationSystem::update` parcourt aujourd'hui `view<Player, Velocity, Animation>` : par
construction, **seul le personnage** peut être animé. Une porte, une torche ou une plaque portant un
composant `Animation` seraient simplement ignorées.

Le système fait par ailleurs deux choses distinctes qu'il faut séparer : **choisir** le clip
(projection de l'état physique du personnage, via `targetClip()`) et **faire avancer** l'animation
(accumulation du temps, changement d'image, bouclage). Seule la seconde est générale.

## Travail à réaliser
- **Séparer les deux responsabilités** :
  - une progression **générale**, appliquée à toute entité portant `core::Animation` : accumuler le
    temps du pas fixe, avancer l'index d'image, boucler ou passer au clip suivant en fin de clip
    joué une fois ;
  - la **projection** état → clip du personnage, qui reste spécifique et conserve sa forme actuelle
    (`!grounded` → saut, `|vitesse.x| > seuil` → course, sinon repos), enrichie en LOT-48.
- **Vue élargie** : `view<Animation>` pour la progression, plus la vue spécifique au personnage pour
  la projection.
- **Pas fixe uniquement** (`EX-REN-021`, `EX-NFR-002`) : la progression reste dans la simulation,
  jamais au temps réel. Deux exécutions de la même séquence d'entrées produisent la même suite
  d'images.
- **Changement de clip** : réinitialise l'index et le temps écoulé, comme aujourd'hui.
- **Clip joué une fois** : à la dernière image, bascule sur le clip suivant déclaré, sans sauter
  d'image ni rejouer la dernière.
- Conserver la boucle `while (elapsed >= duree)` actuelle, qui gère correctement un pas fixe plus
  long qu'une durée d'image.

## Fichiers impactés
- `Source/Core/Ecs/Systems/AnimationSystem.{h,cpp}`.
- `Source/Test/Unit/Core/Ecs/test_animation_system.cpp` (étendu).

## Tests (obligatoires)
- Progression d'un clip bouclé : l'index revient à zéro après la dernière image, sans image sautée.
- Clip joué une fois : bascule sur le clip suivant à la fin, une seule fois.
- Durées inégales entre images.
- Un pas fixe plus long que la durée d'une image fait avancer de plusieurs images, sans dérive.
- **Déterminisme** : deux exécutions identiques produisent la même suite d'indices.
- Une entité non-joueur portant `Animation` s'anime ; le personnage conserve son comportement.

## Points d'attention
- **Ne pas rendre la progression dépendante du temps réel**, même « pour la fluidité » : ce serait
  la première brèche dans le déterminisme, et l'interpolation de rendu (`hmi::PreviousPosition`)
  traite déjà le découplage entre pas fixe et fréquence d'affichage.
- Le système ne doit **pas** connaître le nombre d'entités animées à l'avance : une porte animée
  n'existe pas au démarrage, elle est créée avec la scène.
- Ordre d'exécution : la progression doit avoir lieu **après** la physique du même pas, comme
  aujourd'hui (`GameSession::update`), pour que le clip reflète l'état de fin de pas.

## Définition de fait (DoD)
- Toute entité portant `Animation` progresse au pas fixe ; les clips bouclés et joués une fois se
  comportent comme spécifié ; le personnage est inchangé ; déterminisme asserté ; `/W4 /WX` propre.

## Exigences
`EX-REN-005` (animations par données, toute entité) ; réutilise `EX-REN-021` (pas fixe),
`EX-NFR-002` (déterminisme), `EX-NFR-020` (tests unitaires du gameplay `Core`).
