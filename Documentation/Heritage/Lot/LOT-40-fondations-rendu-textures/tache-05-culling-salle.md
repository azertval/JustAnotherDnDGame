# TACHE-05 — Culling par salle visible {#lot-40-tache-05-culling-salle}

**Lot :** [LOT-40](epic.md) · **Emplacement :** `Source/HMI/Graphics`, `Source/Test` · **Statut :** fait

## Contexte
`hmi::SpriteRenderer::render` parcourt **toutes** les entités du monde et soumet un quad pour
chacune, quelle que soit sa position — y compris pour les tuiles situées à l'autre bout du niveau,
hors du cadrage. C'était sans conséquence tant qu'un niveau valait une poignée de centaines de
tuiles sur un seul calque.

Ce n'est plus vrai : le programme d'habillage ajoute un fond, des décors sur trois couches, une
ombre par tuile solide, des objets animés, un personnage et des effets, sur des niveaux qui peuvent
compter plusieurs salles depuis `LOT-16`/`LOT-32`. Le plafond de `MAXIMUM_QUADS = 2048` par lot de
`SpriteBatch` n'est pas une limite dure (l'auto-flush protège), mais le volume soumis, lui, croît
sans borne.

La caméra cadre déjà **une salle à la fois** (`EX-REN-015`) : la donnée nécessaire au culling
existe, il suffit de l'utiliser.

## Travail à réaliser
- **Rejet avant soumission** : lors de la composition (TACHE-04), écarter toute primitive dont la
  boîte englobante n'intersecte pas le rectangle cadré par la caméra, obtenu via
  `hmi::RoomGrid::roomBounds` (LOT-32) ou directement les bornes de la caméra.
- **Marge de sécurité** : conserver une marge d'au moins une case autour du cadrage, afin qu'une
  entité partiellement visible (personnage à cheval sur la frontière, décor débordant, effet)
  ne disparaisse pas prématurément. La marge est une constante nommée, pas un nombre magique.
- **Observabilité** (`EX-NFR-005`) : exposer le nombre de primitives composées et soumises pour la
  dernière image, consultable en journalisation de diagnostic.
- Le culling s'applique **uniformément** à tous les calques, y compris ceux introduits par les lots
  suivants — c'est une propriété de la composition, pas une règle par calque.

## Fichiers impactés
- `Source/HMI/Graphics/SpriteRenderer.{h,cpp}`, `Source/HMI/Graphics/DraftRenderer.{h,cpp}`.
- `Source/Test/Unit/HMI/Graphics/test_render_culling.cpp` (nouveau).

## Tests (obligatoires)
- Une entité hors cadrage n'apparaît pas dans les primitives capturées ; la même entité dans le
  cadrage y apparaît (via le *QuadRecorder*, sans GPU).
- Une entité **à cheval** sur la frontière du cadrage est conservée (vérification de la marge).
- Dénombrement : sur un niveau multi-salles, le nombre de primitives composées est proportionnel au
  contenu de la salle visible, pas à la taille du niveau.

## Points d'attention
- **Le fond de niveau (LOT-44) est un cas particulier** : il est étiré sur les bornes du niveau
  entier et ne doit évidemment pas être écarté parce que son centre est hors cadrage. Le test
  d'intersection doit porter sur la boîte englobante réelle, pas sur la position d'ancrage.
- Le culling est purement visuel : il ne doit avoir **aucun** effet sur la simulation
  (`EX-ARCH-012`) ni sur l'interpolation de rendu (`hmi::PreviousPosition`) — une entité écartée
  reste simulée normalement.
- Attention au parallax (LOT-49) : une couche de décor défilant à une vitesse différente n'occupe
  pas le même rectangle monde que le niveau. Le rectangle de test doit être calculé **après**
  application du facteur de défilement.

## Définition de fait (DoD)
- Seules les primitives visibles sont soumises, marge comprise ; nombre de primitives observable ;
  aucun changement visuel constaté ; tests hors GPU verts ; `/W4 /WX` propre.

## Exigences
`EX-NFR-005` (culling et budget de primitives) ; réutilise `EX-REN-015` (cadrage par salle),
`EX-NFR-004` (vérification sans GPU), `EX-ARCH-012` (rendu sans effet sur la simulation).
