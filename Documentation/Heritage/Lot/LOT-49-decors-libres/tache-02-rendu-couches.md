# TACHE-02 — Rendu des trois couches et premier plan {#lot-49-tache-02-rendu-couches}

**Lot :** [LOT-49](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** fait

## Contexte
Les calques `Decor` et `Foreground` ont été réservés dans *%RenderLayer* en LOT-40 sans être
utilisés. Cette tâche les active — et avec eux, la propriété qui fait tout l'intérêt du lot : le
calque `Foreground` est **au-dessus** de `Player`.

C'est la réponse concrète à « que le joueur comprenne ce qui est physique » : ce qui passe devant le
personnage ne le porte pas et ne le bloque pas. Cette convention de lecture est immédiate et ne
demande aucun apprentissage.

## Travail à réaliser
- **Construction de scène** : chaque *%core::Decor* du niveau donne une entité portant
  `core::Transform` (position, échelle, rotation) et `core::Sprite`, créée par
  `core::buildLevelScene` comme les tuiles. L'ordre dans le vecteur alimente `Sprite::layer` (tri
  fin **à l'intérieur** d'une couche).
- **Projection couche → calque** : *%core::DecorLayer* vers les valeurs correspondantes de
  *%RenderLayer*, côté `HMI`. Fonction pure, triviale mais explicite — c'est le point où la frontière
  `Core`/`HMI` est tenue.
- **Mode Texture uniquement** : aucun décor en mode Physique, qui doit rester la lecture nue des
  collisions.
- **Repli** : un décor dont l'asset est introuvable affiche le damier magenta à sa taille, avec
  avertissement (`EX-NFR-040`) — contrairement au fond, un décor **désigné** est toujours censé
  exister.
- **Rotation** : `core::Transform::rotation` existe mais n'a jamais été utilisée par le rendu, qui
  n'émet que des quads alignés aux axes (`SpriteQuad`). Trancher : soit ignorer la rotation pour ce
  lot (et le documenter), soit émettre un quad orienté sur le modèle de `LineQuad` (LOT-37). Ne pas
  laisser le champ présent dans le modèle sans effet silencieux au rendu.

## Fichiers impactés
- `Source/Core/Levels/LevelScene.{h,cpp}` (entités de décor).
- `Source/HMI/Graphics/DecorVisuals.{h,cpp}` (nouveau) — projection et résolution d'asset.
- `Source/HMI/Graphics/SpriteRenderer.{h,cpp}`, `DraftRenderer.{h,cpp}`.
- `Source/Test/Unit/HMI/Graphics/test_decor_visuals.cpp` (nouveau).

## Tests (obligatoires)
- **Ordre des calques** : un décor de premier plan est soumis **après** le personnage, un décor
  d'arrière-plan **avant** les tuiles — asserté via le *QuadRecorder*.
- Ordre intra-couche : deux décors de la même couche respectent l'ordre du vecteur.
- Aucun quad de décor en mode Physique.
- Asset introuvable → damier + avertissement.
- Sans GPU.

## Points d'attention
- **Aucune collision.** Vérifier explicitement qu'aucun décor n'entre dans la grille de collision ni
  dans les boîtes de danger : c'est la distinction même que le lot établit.
- Le personnage doit rester **visible** : un décor de premier plan opaque couvrant toute la salle
  rendrait le jeu injouable. C'est une responsabilité du level designer, mais l'aperçu par calque
  (LOT-51) doit permettre de le diagnostiquer.
- `core::buildLevelScene` reçoit déjà une fonction de résolution d'apparence **injectée** depuis
  `HMI` : conserver ce patron pour les décors, sans faire connaître les assets à `Core`.

## Définition de fait (DoD)
- Les trois couches sont rendues dans le bon ordre, le premier plan au-dessus du personnage ; aucun
  décor en mode Physique ; l'asset introuvable dégrade proprement ; le sort de la rotation est
  tranché et documenté ; ordre asserté sans GPU ; `/W4 /WX` propre.

## Révision (`LOT-50`)
Tranché ici pour l'ignorer (poignée de rotation, `LOT-50` TACHE-02, encore à venir). Une fois cette
poignée livrée, le choix s'est révélé un geste sans effet visible — revu en TACHE-03 de `LOT-50`
(voir @ref lot-50-tache-03-selection-poignees) : la rotation atteint désormais le quad composé.

## Exigences
`EX-DEC-002` (couches), `EX-DEC-003` (pixel art net) ; réutilise `EX-REN-014` (ordonnancement des
calques), `EX-REN-043` (multi-textures), `EX-REN-046` (bascule), `EX-ARCH-012` (rendu sans effet sur
la simulation), `EX-NFR-040` (repli), `EX-NFR-004` (vérification sans GPU).
