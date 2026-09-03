# TACHE-01 — Le clip devient une donnée {#lot-46-tache-01-modele-clip}

**Lot :** [LOT-46](epic.md) · **Emplacement :** `Source/Core/Ecs` · **Statut :** fait

## Contexte
L'animation actuelle est **entièrement figée dans le code** :

- `core::AnimationClip` est un `enum class { Idle, Run, Jump }` — trois valeurs, ajoutées à la main ;
- le nombre d'images de chaque clip est une constante de compilation (`IDLE_FRAME_COUNT`,
  `RUN_FRAME_COUNT`, `JUMP_FRAME_COUNT`) ;
- la durée par image aussi (`IDLE_FRAME_DURATION`, `RUN_FRAME_DURATION`) ;
- tous les clips bouclent, sans exception possible.

Animer une porte reviendrait donc à ajouter des valeurs à un `enum` de `Core` et des constantes de
durée pour chaque objet du jeu — c'est-à-dire à faire de la simulation un catalogue d'apparences, en
contradiction avec `EX-ARCH-012`.

## Travail à réaliser
- **Modèle de clip** (données pures, `Core`) : un nom, une suite d'indices d'images, une durée par
  image, et un mode de fin — **bouclé** ou **joué une fois** avec clip suivant.
- **Jeu de clips** : ensemble nommé de clips, adressable par nom. C'est l'unité que décrira le
  fichier d'animation (TACHE-03).
- **`core::Animation` révisé** : référence le clip courant **par nom** au sein d'un jeu, conserve
  l'index d'image et le temps écoulé. Reste une `struct` de données pures, sans méthode ni type GPU,
  conformément au patron des composants ECS.
- **Suppression des constantes figées** de `Animation.h` (nombres d'images et durées), remplacées
  par les données des clips.
- **Accès sûr** : demander un clip inexistant ne doit ni planter ni lever — repli déterministe sur
  un clip par défaut ou la première image.

## Fichiers impactés
- `Source/Core/Ecs/Components/Animation.h`, `Source/Core/Ecs/AnimationClip.{h,cpp}` (nouveau).
- `Source/Core/CMakeLists.txt`.
- `Source/Test/Unit/Core/Ecs/test_animation_clip.cpp` (nouveau).

## Tests (obligatoires)
- Construction et interrogation d'un jeu de clips : clip existant, clip inexistant → repli.
- Un clip à une seule image, un clip à durées inégales, un clip joué une fois avec clip suivant.
- Données pures : ni GPU, ni fichier, ni Qt.

## Points d'attention
- **Ne pas introduire de machine à états.** Le choix du clip reste une **projection** de l'état de
  simulation, comme l'actuel `targetClip()` ; des conditions de transition déclaratives seraient une
  abstraction non justifiée pour une dizaine de clips, et déplaceraient de la logique de gameplay
  dans des fichiers de données.
- Le nom de clip est une chaîne : prévoir dès maintenant comment éviter les comparaisons de chaînes
  à chaque pas fixe (résolution en index à la construction, par exemple).
- `Core` ne doit connaître ni la spritesheet, ni la taille des images, ni le fichier d'origine : un
  clip décrit des **indices**, la traduction en région de texture est le travail de `HMI`.

## Définition de fait (DoD)
- Un clip est une donnée constructible et interrogeable ; `core::Animation` n'a plus aucune
  constante de durée ni de nombre d'images ; l'accès à un clip inexistant est sûr ; tests purs
  verts ; `/W4 /WX` propre.

## Exigences
`EX-REN-005` (animations décrites par des données) ; amende `EX-REN-012` (animations par séquence
d'images) ; réutilise `EX-ARCH-012` (`Core` ignore le rendu), `EX-NFR-010` (testable sans GPU).
