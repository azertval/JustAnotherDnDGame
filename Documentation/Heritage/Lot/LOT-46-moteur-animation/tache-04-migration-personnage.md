# TACHE-04 — Migration des clips du personnage {#lot-46-tache-04-migration-personnage}

**Lot :** [LOT-46](epic.md) · **Emplacement :** `Source/Core`, `Source/HMI/Graphics`, `Source/Test` · **Statut :** fait

## Contexte
Le personnage est le seul consommateur actuel de l'animation, et il l'utilise à travers trois
mécanismes couplés :

- `core::AnimationSystem::targetClip()` choisit parmi `{ Idle, Run, Jump }` ;
- `hmi::flatPlayerFrameIndex(clip, index)` (`ProceduralAtlas.cpp`) est la **source de vérité unique**
  de l'ordre des images dans l'atlas — repos, puis course, puis saut ;
- `hmi::TextureAtlas::playerFrameRegion` calcule la région correspondante, et
  `GameSession::refreshPlayerSprite()` l'applique à chaque image.

Ce lot étant une refonte d'infrastructure, la contrainte est simple : **rien ne doit changer à
l'écran**. C'est le filet qui permet de valider le nouveau moteur avant d'y ajouter quoi que ce soit
(LOT-47, LOT-48).

## Travail à réaliser
- **Exprimer les clips actuels dans le nouveau format** : trois clips nommés, avec les durées
  d'aujourd'hui (repos `0,5 s`, course `0,1 s`) et les nombres d'images d'aujourd'hui (2, 4, 1),
  dans l'ordre porté par `flatPlayerFrameIndex`.
- **Adapter `targetClip()`** pour désigner ces clips par leur nom plutôt que par une valeur d'`enum`,
  sans changer sa logique de projection.
- **Adapter la chaîne `HMI`** : `playerFrameRegion` et `refreshPlayerSprite` consomment le nouveau
  modèle. `flatPlayerFrameIndex` reste la source de vérité de la disposition dans l'atlas procédural.
- **Test de référence** capturant la suite d'images produite pour une séquence d'entrées donnée,
  établi **avant** la migration et vérifié après.

## Fichiers impactés
- `Source/Core/Ecs/Systems/AnimationSystem.{h,cpp}`.
- `Source/HMI/Graphics/TextureAtlas.{h,cpp}`, `TextureAtlasRegions.cpp`, `ProceduralAtlas.{h,cpp}`.
- `Source/HMI/Game/GameSession.{h,cpp}`.
- `Source/Test/Unit/Core/Ecs/test_animation_system.cpp`,
  `Source/Test/Unit/HMI/Graphics/test_texture_atlas.cpp`,
  `test_procedural_atlas.cpp` (adaptés).

## Tests (obligatoires)
- **Non-régression stricte** : pour une séquence d'entrées de référence (repos, course, saut,
  retombée), la suite d'indices d'images et les régions d'atlas produites sont **identiques** à
  celles d'avant la migration.
- Les tests existants d'`AnimationSystem`, de `TextureAtlas` et de `ProceduralAtlas` passent, adaptés
  au nouveau modèle mais **sans changer leurs attentes de comportement**.

## Points d'attention
- **Ne pas en profiter pour ajuster les durées.** Toute retouche de ressenti doit être un choix
  séparé, visible dans son propre lot — sinon une régression de game feel se cacherait dans une
  refonte technique.
- L'atlas procédural reste le **repli** et la référence (`LOT-39`) : il doit continuer de produire
  exactement les mêmes images, et `flatPlayerFrameIndex` reste partagé entre la génération et la
  lecture.
- Vérifier que `refreshPlayerSprite`, appelé à chaque image de rendu, ne fait pas de recherche par
  chaîne de caractères : la résolution du clip doit être faite en amont.

## Définition de fait (DoD)
- Les clips du personnage sont exprimés en données ; l'animation à l'écran est **strictement
  identique** à avant, assertée par un test de référence ; les tests existants passent ;
  `/W4 /WX` propre.

## Exigences
`EX-REN-005` (animations par données) ; réutilise `EX-REN-012` (animations du personnage),
`EX-REN-042` (repli procédural), `EX-NFR-002` (déterminisme), `EX-NFR-020` (tests unitaires).
