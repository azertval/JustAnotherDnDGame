# TACHE-03 — Ombres des silhouettes et des blocs réduits {#lot-55-tache-03-ombres-silhouettes}

**Lot :** [LOT-55](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** fait

## Contexte
La TACHE-01 émet un quad d'ombre rectangulaire pour chaque tuile **solide** au sens de
`core::isSolid`. Cela laisse deux familles de tuiles sans ombre correcte :

- **les douze silhouettes** (pentes, arrondis convexes et concaves, sol et plafond) : elles ne sont
  jamais solides — `core::isSolid` ne retient que `Solid`, `Block`, `BlockHalf`, `BlockQuarter`,
  les silhouettes étant résolues par des passes de suivi dédiées. Elles n'auraient donc **aucune**
  ombre, à côté d'un bloc plein qui en a une ;
- **les blocs réduits** `BlockHalf` et `BlockQuarter` : solides, mais n'occupant qu'une fraction de
  leur case. Un quad d'ombre pleine case serait visiblement trop grand.

Le résultat serait incohérent, et l'incohérence porterait précisément sur ce que le lot cherche à
rendre lisible : ce qui est physique.

## Un prédicat de solidité partielle n'est pas la solution

L'intuition naturelle est d'ajouter à `Core` un `isSolidHalf` ou équivalent. C'est à écarter, pour
deux raisons.

**Ce n'est pas la bonne question.** Une ombre est la projection d'une **forme**, pas d'un degré de
solidité. Savoir qu'une pente est « à moitié solide » ne dit rien de l'endroit où elle l'est —
c'est la silhouette qu'il faut, et un booléen ne la porte pas.

**Tout existe déjà, et du bon côté de la frontière.** La forme des douze silhouettes est calculée
par des fonctions **pures de `Core`** — `core::slopeSurfaceHeight`, `core::ceilingSlopeHeight`,
`core::isCeilingSlope` — que *hmi::slopeShapePixel* (`ProceduralAtlas.cpp`) utilise déjà pour
produire le masque « dedans / dehors » de chaque type, et que le `LOT-42` (TACHE-03) factorise en un
`SlopeMask` pur et mis en cache pour détourer les skins. La taille des blocs réduits est donnée par
`core::tileVisualScale`, décrite dans le code comme la source de vérité partagée entre le visuel et
la collision.

Ajouter un prédicat de solidité **pour un besoin purement visuel** ferait entrer une préoccupation
de présentation dans la simulation (`EX-ARCH-012`), pour une information que `Core` expose déjà sous
une forme plus riche.

## Travail à réaliser
- **Ombre de silhouette** : pour les douze types à silhouette, l'ombre n'est plus un quad uni mais
  la **même silhouette**, remplie de la couleur d'ombre et décalée du même offset que les autres.
  Réutiliser le `SlopeMask` de `LOT-42` (TACHE-03) : produire, par type, une variante « ombre »
  du masque, mise en cache comme toute texture.
- **Ombre des blocs réduits** : quad d'ombre à l'échelle de `core::tileVisualScale`, aligné sur la
  boîte réelle du bloc — pas sur sa case.
- **Blocs poussables en mouvement** : leur ombre suit leur position courante, comme leur sprite
  (`GameSession::refreshBlockVisuals`).
- **Dangers directionnels** : ils ne sont pas solides et n'ont pas à projeter d'ombre ; le vérifier
  explicitement plutôt que de le supposer.

## Fichiers impactés
- `Source/HMI/Graphics/ShadowRenderer.{h,cpp}` — voir « Réalisation » de
  [TACHE-01](tache-01-quad-ombre.md).
- `Source/Test/Unit/HMI/Graphics/test_shadow_render.cpp`.

## Réalisation
Contrairement à l'anticipation ci-dessus, aucune variante d'ombre du `SlopeMask` n'a été mise en
cache dans `TextureCache` : `hmi::regionForTile(type)` pointe déjà, dans l'atlas procédural, vers
une région détourée à la silhouette exacte (`hmi::isInsideSilhouette`, la même fonction que
`hmi::SlopeMask`) — la réutiliser telle quelle pour l'ombre (teintée en noir semi-transparent) est
strictement équivalente à en dériver une variante, sans code ni cache supplémentaires. Voir
« Réalisation » de [TACHE-01](tache-01-quad-ombre.md) pour l'implémentation unique qui couvre les
deux tâches.

## Tests (obligatoires)
- Pour chacun des douze types à silhouette : l'ombre émise a la **même forme** que la silhouette de
  la tuile, décalée de l'offset — comparaison au masque de référence, sans GPU.
- `BlockHalf` et `BlockQuarter` : l'ombre a la taille donnée par `tileVisualScale`, pas la case
  entière.
- Un bloc poussable déplacé voit son ombre suivre.
- Aucune ombre pour les dangers, l'entrée, la sortie et les tuiles vides.

## Points d'attention
- **Une seule source de forme.** Le masque d'ombre doit dériver du même `SlopeMask` que le détourage
  des skins ; deux implémentations divergeraient, et l'ombre cesserait de correspondre à la tuile
  qu'elle accompagne — exactement le défaut que cette tâche corrige.
- Le coût : douze masques de 16×16 pixels, calculés une fois et mis en cache. Négligeable, mais à
  ne pas recalculer par case.
- Cette tâche fait dépendre `LOT-55` de `LOT-42`, ce qui n'était pas le cas de la TACHE-01 seule.
  C'est acceptable puisque `LOT-42` est très en amont dans la séquence.
- Les ombres restent **purement visuelles** : aucun masque d'ombre ne doit être lu par la physique
  ni approcher la grille de collision.

## Définition de fait (DoD)
- Les pentes et arrondis projettent une ombre à leur forme réelle ; les blocs réduits une ombre à
  leur taille réelle ; aucune ombre pour ce qui n'est pas physique ; aucun nouveau prédicat de
  solidité dans `Core` ; formes assertées sans GPU ; `/W4 /WX` propre.

## Exigences
`EX-REN-045` (ombres du plan physique) ; réutilise `EX-GP-003`/`EX-GP-004`/`EX-GP-006`/`EX-GP-007`
(silhouettes de pentes et arrondis), `EX-GP-005` (blocs à taille réduite), `EX-EDIT-042` (masquage
de silhouette du `LOT-42`), `EX-ARCH-012` (rendu sans effet sur la simulation), `EX-NFR-004`
(vérification sans GPU).
