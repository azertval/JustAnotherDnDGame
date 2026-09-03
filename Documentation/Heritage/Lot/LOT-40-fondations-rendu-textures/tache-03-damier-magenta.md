# TACHE-03 — Texture de repli en damier magenta {#lot-40-tache-03-damier-magenta}

**Lot :** [LOT-40](epic.md) · **Emplacement :** `Source/HMI/Graphics`, `Source/Test` · **Statut :** fait

## Contexte
À partir de LOT-42, une tuile/un objet/un fond peut référencer une texture absente ou non encore
assignée. Pour rester **honnête** (visible, pas silencieux) sans jamais bloquer le rendu
(`EX-NFR-040`), ce cas doit afficher un repli **reconnaissable entre tous** — un damier magenta —
plutôt qu'un carré vide ou une texture par défaut ambiguë. Cette tâche construit ce repli une seule
fois, en amont, pour que LOT-42, LOT-44, LOT-45 et LOT-49 n'aient qu'à l'appeler.

## Travail à réaliser
- ***buildMissingTextureImage()*** (namespace `hmi`, sœur de `hmi::buildProceduralAtlasImage`, même fichier
  `ProceduralAtlas.{h,cpp}` ou un nouveau fichier dédié) : génère en mémoire un damier magenta/noir
  16×16 (ou une taille configurable), déterministe, opaque (pas de canal alpha nécessaire — ce repli
  doit être **visible**, pas transparent).
- **Résolution partagée** : une fonction utilitaire (ex. *resolveOrPlaceholder(cache, assetName) ->
  LoadedTexture*, namespace `hmi`) qui tente d'obtenir la texture via *TextureCache*, et à défaut journalise via
  `GRAPHICS_LOG_WARNING` (message incluant le nom d'asset attendu) puis renvoie la texture de repli
  — **un seul** point d'appel pour ce comportement, réutilisé par tous les lots suivants plutôt que
  chacun réimplémentant son propre repli.

## Fichiers impactés
- `Source/HMI/Graphics/ProceduralAtlas.{h,cpp}` (ou nouveau fichier dédié au repli).
- `Source/Test/Unit/HMI/Graphics/test_missing_texture.cpp` (nouveau).

## Tests (obligatoires)
- Dimensions/déterminisme du damier généré (même patron que `test_procedural_atlas.cpp`), sans GPU.
- Résolution partagée : asset présent → texture réelle ; asset absent → repli + message de log
  (vérifiable sans GPU si le chemin de résolution est découplé de l'upload D3D11, comme
  `AssetPaths::resolve`).

## Points d'attention
- Le damier doit être visuellement **sans ambiguïté possible** avec un asset réel (magenta n'apparaît
  dans aucune palette de couleurs du jeu à ce jour).
- Le message de log doit nommer l'asset attendu, pour que le designer sache **quoi** créer.

## Définition de fait (DoD)
- Repli en damier magenta généré et branché sur un point de résolution unique, réutilisable ;
  avertissement de log systématique ; tests hors GPU verts ; `/W4 /WX` propre.

## Exigences
`EX-REN-043` (rendu multi-textures) ; réutilise `EX-NFR-040` (repli sans plantage).
