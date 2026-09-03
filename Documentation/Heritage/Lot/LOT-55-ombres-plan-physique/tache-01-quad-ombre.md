# TACHE-01 — Quad d'ombre par tuile solide {#lot-55-tache-01-quad-ombre}

**Lot :** [LOT-55](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** fait

## Contexte
Le calque *Shadow* de *RenderLayer* a été réservé en LOT-40 et n'a jamais été utilisé. Cette tâche
est la seule à l'activer, et elle reste volontairement simple : un quad sombre décalé sous chaque
tuile solide, dessiné avant la tuile elle-même.

L'objectif est de **lecture**, pas d'esthétique : donner du relief au plan physique pour qu'il se
détache de ce qui ne l'est pas. C'est le complément du calque de premier plan (LOT-49) — l'un dit
« ceci passe devant vous, donc ne vous porte pas », l'autre « ceci est en relief, donc vous porte ».

## Travail à réaliser
- **Émission** : pour chaque tuile **solide** au sens de `core::isSolid`/`TileMap::isSolid` (aucune
  autre lecture de `Core`), un quad sombre semi-transparent, décalé d'un offset constant, sur le
  calque *Shadow*, donc **avant** les tuiles.
- **Constantes nommées** : offset, opacité et couleur. Pas de configuration par le designer pour ce
  lot.
- **Mode Texture uniquement** : aucune ombre en mode Physique, qui reste la lecture nue des
  collisions et n'en a pas besoin (la couleur plate suffit).
- **Culling** : les quads d'ombre passent par le culling comme le reste (LOT-40, TACHE-05) — ils
  doublent le nombre de primitives du plan physique, c'est le principal effet de ce lot sur le
  budget (`EX-NFR-005`).
- **Silhouettes et blocs réduits : hors de cette tâche.** Les pentes et arrondis ne sont pas solides
  au sens de `core::isSolid` (résolus par des passes de suivi dédiées) et n'auraient donc aucune
  ombre ; les blocs réduits n'occupent qu'une fraction de leur case. Les deux sont traités en
  [TACHE-03](tache-03-ombres-silhouettes.md), qui donne à l'ombre la forme réelle de la tuile. Cette
  tâche se limite au quad rectangulaire pleine case des types `Solid` et `Block`.

## Fichiers impactés
- `Source/HMI/Graphics/ShadowRenderer.{h,cpp}` (nouveau) : `hmi::composeShadows`, unique pour
  TACHE-01 et TACHE-03 (voir Réalisation ci-dessous).
- `Source/HMI/Graphics/SpriteRenderer.{h,cpp}` (appel depuis `SpriteRenderer::render`, paramètre
  `doorCollision`), `Source/HMI/Graphics/DraftRenderer.cpp` (appel gardé par `LayerVisibility`),
  `Source/HMI/Game/GameSession.cpp` (transmet `MechanismController::collisionMap()`).
- `Source/Test/Unit/HMI/Graphics/test_shadow_render.cpp` (nouveau).

## Réalisation
`hmi::composeShadows` parcourt les mêmes entités que `hmi::composeWorldSprites`
(`core::Transform` + `hmi::TileSkinTag`) et ne retient que celles qui projettent une ombre
(`core::isSolid(type) || hmi::hasSilhouette(type)`, plus le cas particulier de la porte ci-dessous).
La région échantillonnée est directement `hmi::regionForTile(type)` — **le même** atlas procédural
que le mode Physique et que le détourage de silhouette (`LOT-42`) : cette région est déjà opaque
exactement là où la matière est présente (case pleine, ou silhouette de pente/arrondi) et
transparente ailleurs, donc teinter le quad en noir semi-transparent donne l'ombre à sa forme
réelle sans réimplémenter la géométrie ni ajouter de cache dédié. Un bloc réduit hérite de sa
taille réelle via `core::Transform::scale` (`core::tileVisualScale`), déjà porté par l'entité
comme pour son propre sprite. Une seule fonction couvre donc à la fois TACHE-01 (quad plein) et
TACHE-03 (silhouettes, blocs réduits) : la distinction entre les deux n'existe plus en pratique une
fois la région réutilisée telle quelle.

Pour la porte (source de solidité, cf. « Points d'attention ») : `TileSkinTag::type` reste
`TileType::Door` quel que soit l'état du mécanisme (figé au chargement, jamais réécrit) — le choix
retenu est donc de suivre l'état courant, via un paramètre optionnel `doorCollision` (la grille
`core::MechanismController::collisionMap()`) plutôt que le type statique. `hmi::GameSession` le
fournit toujours ; `hmi::DraftRenderer` (aucune simulation de mécanisme) passe `nullptr`, et une
porte n'y projette donc jamais d'ombre — état normal de l'éditeur, pas un défaut.

## Tests (obligatoires)
- Une tuile `Solid` ou `Block` produit un quad sur le calque *Shadow*, décalé de l'offset attendu ;
  une tuile non solide n'en produit aucun — asserté via le *QuadRecorder*.
- **Aucun quad d'ombre en mode Physique.**
- Les ombres sont émises **avant** les tuiles dans l'ordre des primitives.
- Sans GPU.

## Points d'attention
- **Aucun effet sur le gameplay** (`EX-ARCH-012`) : les ombres sont purement décoratives et ne
  doivent apparaître dans aucune grille de collision ni boîte de danger.
- Une porte **fermée** est solide, une porte **ouverte** ne l'est plus : si l'ombre est calculée
  depuis la grille de collision (`MechanismController::collisionMap`) plutôt que depuis le niveau,
  elle apparaîtra et disparaîtra avec l'état de la porte. Décider lequel des deux est voulu —
  suivre l'état est probablement le bon choix ici, contrairement aux raccords automatiques (LOT-42,
  TACHE-02) où c'est l'inverse. Le noter, car les deux choix opposés dans le même programme méritent
  une justification explicite.
- Un niveau **sans** fond ni décor d'arrière-plan n'a aucune surface pour recevoir l'ombre : ce n'est
  pas un cas d'erreur, aucun traitement particulier n'est requis.

## Définition de fait (DoD)
- Les tuiles pleines projettent une ombre sur ce qui se trouve derrière elles, en mode Texture
  uniquement, avant leur propre dessin ; la source de solidité (grille du niveau ou de collision)
  est tranchée et documentée ; aucun effet sur le gameplay ; assertions sans GPU vertes ;
  `/W4 /WX` propre.

## Exigences
`EX-REN-045` (ombres du plan physique) ; réutilise `EX-REN-043` (calques), `EX-REN-044` (fond),
`EX-REN-046` (bascule), `EX-DEC-002` (couches de décor), `EX-GP-001` (tuiles solides),
`EX-ARCH-012` (rendu sans effet sur la simulation), `EX-NFR-005` (culling et budget).
