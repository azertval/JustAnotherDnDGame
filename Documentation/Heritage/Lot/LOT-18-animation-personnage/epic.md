# LOT-18 — Animation du personnage (repos, course, saut) {#lot-18}

> Statut : **terminé**. Second des deux lots dédiés au visuel du personnage (`EX-REN-012`) :
> [LOT-17](../LOT-17-sprite-personnage/epic.md) a livré une silhouette **statique**, celui-ci lui
> donne une **séquence d'images** selon l'état du personnage (repos, course, saut).

## Objectif
Le personnage affiche aujourd'hui une pose unique et fixe, quel que soit son état (immobile,
en train de courir, en l'air). Ce lot ajoute une **animation par séquence d'images**, pilotée par
l'état de simulation déjà disponible (`core::Player::grounded`, `core::Velocity`) — sans manette
ni minuterie propre au joueur : le clip actif est **dérivé** de l'état physique, pas déclenché
manuellement.

## Périmètre

### Inclus
- Trois **clips** : `Idle` (repos, 2 images), `Run` (course, 4 images), `Jump` (saut, 1 image —
  une pose aérienne fixe, pas un cycle : l'arc de saut n'appelle pas de bouclage).
- Un composant `core::Animation` (donnée pure) et un `core::AnimationSystem` (logique pure,
  `EX-ARCH-011`) qui détermine le clip actif et avance l'image courante au pas fixe.
- Une grille de frames dans `TextureAtlas` (7 images 16×16, sur le modèle carré corrigé en
  LOT-17) et le branchement dans `GameScreen` : le sprite du personnage change de région à
  chaque frame de rendu selon l'état d'animation, plus seulement une fois au spawn.

### Exclus (hors périmètre de ce lot)
- **Retournement horizontal** du sprite selon `Player::facing` (le personnage regarde toujours
  « de face ») : aucun support de *flip* dans `Sprite`/`SpriteRenderer` aujourd'hui ; l'ajouter
  exigerait un champ supplémentaire et une modification du calcul des UV, hors de portée d'un lot
  centré sur le cycle d'images. Le personnage reste symétrique gauche/droite (cohérent avec la
  silhouette de LOT-17).
- **Clips additionnels** (atterrissage, dash, etc.) — seuls les trois clips de `EX-REN-012` sont
  couverts.
- Toute interpolation entre images (*tweening*) : le pixel art change d'image par à-coups, comme
  attendu pour ce style (cf. échantillonnage *nearest*, @ref guide-rendu).

## Décisions de cadrage
- **Le clip actif est calculé, pas piloté par un état explicite du joueur.** `AnimationSystem` lit
  `Player::grounded` et `Velocity::value.x` (déjà mis à jour par `CharacterPhysicsSystem` avant que
  l'animation ne s'exécute) : au sol et immobile → `Idle` ; au sol et en mouvement horizontal →
  `Run` ; en l'air → `Jump`. Aucun nouvel état n'est ajouté à `core::Player` : l'animation est une
  **projection** de l'état physique existant, jamais une source de vérité supplémentaire.
- **La logique de clip/frame vit dans `Core`, pas dans `HMI`.** Comme `CharacterPhysicsSystem`,
  `AnimationSystem` est une logique pure, déterministe au pas fixe, testable sans GPU
  (`EX-NFR-010`, `EX-NFR-002`). Seule la table pixels → région d'atlas (savoir à quoi ressemble
  chaque image) vit côté `HMI` — `Core` ignore tout des pixels.
- **La région d'une image reste carrée (16×16)**, exactement comme la silhouette statique de
  LOT-17 : c'est `Transform::scale` (`core::playerSize()`) qui donne au personnage sa proportion
  finale, pas la région (voir la correction de LOT-17 — ne pas réintroduire ce bug avec les
  nouvelles images).
- **Changement de clip = réinitialisation de l'image courante** (image 0, chronomètre à zéro) :
  plus simple qu'une transition entre clips, et suffisant — les transitions (ex. `Run` → `Jump`)
  sont déjà rendues nettes par la coupure d'image du pixel art, pas besoin de fondu.
- **Le saut n'est pas un cycle** : une seule image, tenue tant que le personnage est en l'air.
  Un vrai cycle (montée/sommet/descente) demanderait de distinguer ces phases dans `Player`, hors
  de portée ici — cohérent avec `EX-REN-012` qui ne demande qu'une pose de « saut ».

## Exigences couvertes
- `EX-REN-012` (animation par séquence d'images : repos, course, saut).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-composant-systeme-animation.md) | Composant et système d'animation | `Core/Ecs` | ✅ |
| [TACHE-02](tache-02-frames-atlas-integration.md) | Images dans l'atlas et intégration au rendu | `HMI/Graphics`, `HMI/Interface` | ✅ |
| [TACHE-03](tache-03-documentation-verification.md) | Documentation et vérification | `Documentation` | ✅ |

## Critères d'acceptation du lot
1. Le personnage immobile au sol alterne entre deux poses de repos à intervalle régulier.
2. Le personnage se déplaçant au sol affiche un cycle de course à quatre images.
3. Le personnage en l'air (saut ou chute) affiche une pose distincte, fixe tant qu'il n'est pas
   au sol.
4. Le changement de clip est immédiat et net à la transition d'état (ex. décollage en cours de
   course).
5. Aucune déformation du personnage à l'écran (chaque image reste dans la boîte de collision
   0,4×0,8 — non-régression du bug corrigé en LOT-17).
6. Logique de clip/image **couverte par des tests d'intégration** (`ctest` vert), déterministe,
   sans GPU. Build `/W4 /WX` sans avertissement, Doxygen et lint des exigences verts.

## Dépendances
- Étend `TextureAtlas` et la silhouette statique (LOT-17), consomme `core::Player::grounded`
  (LOT-10/11/12, coyote time/wall jump/budget) et `core::Velocity` (LOT-03).

## Navigation des tâches
- @subpage lot-18-tache-01-composant-systeme-animation
- @subpage lot-18-tache-02-frames-atlas-integration
- @subpage lot-18-tache-03-documentation-verification
