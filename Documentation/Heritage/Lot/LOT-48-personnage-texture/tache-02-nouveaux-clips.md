# TACHE-02 — Clips couvrant les états de gameplay livrés {#lot-48-tache-02-nouveaux-clips}

**Lot :** [LOT-48](epic.md) · **Emplacement :** `Source/Core/Ecs/Systems`, `Source/HMI/Graphics` · **Statut :** fait

## Contexte
Le gameplay du personnage s'est enrichi sur trois lots sans que le visuel suive :

- `LOT-10` — double saut, glissade murale, saut mural, dash directionnel ;
- `LOT-11` — gravité asymétrique, suspension à l'apex, chute rapide ;
- `LOT-12` — budgets de sauts et de dashs.

`core::AnimationSystem::targetClip()` continue pourtant de projeter tout cela sur **trois** clips :
`!grounded` → saut, `|vitesse.x| > 0,01` → course, sinon repos. Un dash, une glissade murale et une
chute libre affichent donc la même image.

Toute l'information nécessaire est déjà dans `core::Player` et `core::Velocity`, calculée pour la
physique : `grounded`, `wallDirection`, `dashTimer`, `facing`, signe de la vitesse verticale.

## Travail à réaliser
- **Étendre la projection état → clip**, sans ajouter aucun champ à `core::Player` :
  - **dash** — `dashTimer` actif, prioritaire sur tout le reste ;
  - **glissade murale** — `wallDirection` non nul et non au sol ;
  - **chute** — non au sol, vitesse verticale descendante, distincte du saut ;
  - **saut** — non au sol, vitesse verticale ascendante ;
  - **atterrissage** — transition vers `grounded`, clip joué une fois (LOT-46) enchaînant sur repos
    ou course ;
  - **course** et **repos** — inchangés.
- **Ordre de priorité explicite** entre les clips, documenté : plusieurs conditions peuvent être
  vraies simultanément (dash pendant une chute, glissade au moment de toucher le sol).
- **Repli** : un clip non fourni par la spritesheet retombe sur le clip le plus proche déclaré
  (chute → saut, atterrissage → repos), afin qu'une spritesheet partielle reste utilisable.

## Fichiers impactés
- `Source/Core/Ecs/Systems/AnimationSystem.{h,cpp}` (projection étendue).
- `Source/HMI/Graphics/PlayerSprite.{h,cpp}` (repli entre clips).
- `Source/Test/Unit/Core/Ecs/test_animation_system.cpp` (étendu).

## Tests (obligatoires)
- **Chaque état** produit le clip attendu, à partir d'un `core::Player`/`core::Velocity` construit
  pour le cas.
- **Combinaisons ambiguës** : dash en chute, glissade murale à l'instant du contact au sol, saut
  mural — le clip retenu est celui de la priorité documentée.
- **Atterrissage** : joué une fois, puis repos ou course selon la vitesse horizontale.
- Repli entre clips quand la spritesheet est partielle.
- Tests `Core` purs, sans GPU.

## Points d'attention
- **Aucun nouveau champ dans `core::Player`.** L'animation doit rester une **conséquence** de l'état
  physique existant ; ajouter un champ « pour l'animation » ferait entrer la présentation dans la
  simulation (`EX-ARCH-012`) et fausserait le déterminisme des tests de gameplay.
- Le seuil de vitesse distinguant repos et course existe déjà (`0,01`) : ne pas en introduire un
  second pour la chute sans nécessité, et le nommer s'il en faut un.
- L'atterrissage est une **transition**, pas un état : il se détecte par comparaison avec le pas
  précédent, comme les transitions de mécanismes (LOT-47, TACHE-02).

## Définition de fait (DoD)
- Chute, atterrissage, glissade murale et dash sont visuellement distincts du saut ; la priorité
  entre clips est documentée et testée ; aucun champ ajouté à `core::Player` ; les tests de gameplay
  existants passent inchangés ; `/W4 /WX` propre.

## Exigences
`EX-REN-009` (clips couvrant les états de gameplay) ; réutilise `EX-REN-005` (animations par
données), `EX-GP-015`/`EX-GP-016`/`EX-GP-017` (double saut, saut mural, dash), `EX-GP-018` (ressenti
vertical), `EX-ARCH-012` (rendu sans effet sur la simulation).
