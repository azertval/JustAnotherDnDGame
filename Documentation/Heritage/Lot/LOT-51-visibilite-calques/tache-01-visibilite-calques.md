# TACHE-01 — Jeu de visibilités par calque {#lot-51-tache-01-visibilite-calques}

**Lot :** [LOT-51](epic.md) · **Emplacement :** `Source/HMI/Game`, `Source/HMI/Graphics` · **Statut :** fait

## Contexte
À ce stade du programme, une scène empile fond, décors d'arrière-plan, ombres, tuiles habillées,
objets surchargés, personnage et décors de premier plan. Quand quelque chose n'apparaît pas comme
prévu, la seule façon de savoir **d'où** vient le problème est de pouvoir isoler chaque calque.

`F8` (LOT-41) ne répond pas à ce besoin : il **compose** le rendu final (surcharge > skin > damier).
Ce mode **décompose**. Les deux réutilisent le même résolveur, avec des règles d'affichage
différentes.

## Travail à réaliser
- **Jeu de visibilités** : un indicateur activable/désactivable par calque pertinent — Fond, Décor
  d'arrière-plan, Ombres, Skin des tuiles, Objets interactifs, Personnage, Décor de premier plan.
  Tous visibles par défaut.
- **Filtrage à la composition** : un calque masqué n'émet aucune primitive. Le filtrage se fait au
  moment de la composition (LOT-40, TACHE-04), pas par un post-traitement.
- **État « Physique seul »** : le rendu en couleurs plates, proposé comme une visibilité parmi les
  autres plutôt qu'un renderer séparé — c'est la même bascule que `F8` vue sous l'angle des calques.
- **Éditeur uniquement** : le jeu de visibilités vit dans `GameViewport` en mode édition et n'a
  **aucun** effet sur `GameSession` en mode jeu ou essai.
- **Aucune persistance** entre deux sessions (hors périmètre du lot) : tout revient visible au
  lancement.

## Fichiers impactés
- `Source/HMI/Graphics/LayerVisibility.h` (nouveau) — classe entièrement `inline`/`constexpr`
  (comme `hmi::RenderLayer` lui-même) : aucune logique non triviale ne justifie un `.cpp` séparé.
- `Source/HMI/Game/GameViewport.{h,cpp}`, `Source/HMI/Graphics/DraftRenderer.{h,cpp}`,
  `Source/HMI/Graphics/ComposedScene.{h,cpp}` (filtrage à la composition).
- `Source/Test/Unit/HMI/Graphics/test_layer_visibility.cpp` (nouveau).

## Tests (obligatoires)
- Un calque masqué n'émet aucune primitive ; les autres sont inchangés — asserté via le
  *QuadRecorder*.
- **Combinaisons** : deux calques visibles, un seul, aucun.
- Aucun effet sur la composition d'une scène de jeu (`GameSession`).
- Sans GPU.

## Points d'attention
- **Ne pas rendre la composition dépendante de l'ordre des indicateurs** : masquer un calque ne doit
  pas modifier l'ordre relatif des autres.
- Le filtrage doit s'appliquer **avant** le culling ou après, mais de façon décidée : masquer un
  calque ne doit pas changer les résultats de test du culling.
- Prévoir dès maintenant l'ajout d'un calque : le jeu de visibilités doit être indexé par la valeur
  de *RenderLayer*, pas par une liste écrite à la main.

## Définition de fait (DoD)
- Chaque calque peut être masqué et réaffiché indépendamment, en édition uniquement, sans effet sur
  le jeu ; le filtrage est asserté sans GPU ; `/W4 /WX` propre.

## Exigences
`EX-EDIT-044` (visibilité par calque) ; réutilise `EX-REN-014` (ordonnancement des calques),
`EX-REN-043` (multi-calques), `EX-REN-046` (distinct de la bascule), `EX-NFR-004` (vérification sans
GPU).
