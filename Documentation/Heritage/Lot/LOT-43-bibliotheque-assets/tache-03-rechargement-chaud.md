# TACHE-03 — Rechargement à chaud des assets {#lot-43-tache-03-rechargement-chaud}

**Lot :** [LOT-43](epic.md) · **Emplacement :** `Source/HMI/Graphics`, `Source/HMI/Editor` · **Statut :** fait

## Contexte
Aujourd'hui, `hmi::TextureAtlas` charge son fichier **une fois** au démarrage : modifier
`atlas.png` impose de relancer l'application. Le README des assets le dit explicitement (« pas de
rechargement à chaud à ce stade »), et `EX-ARCH-080` l'excluait du MVP.

Ce n'est plus tenable dès qu'on itère sur des textures : c'est la boucle
« peindre → regarder → corriger » de six lots entiers, et de l'atelier pixel art (LOT-54) dont
l'aperçu live en dépend directement.

L'API nécessaire a été prévue en amont : `TextureCache::invalidate(name)` et `invalidateAll()`
existent depuis LOT-40, sans appelant.

## Travail à réaliser
- **Action « Recharger les textures »** dans l'interface d'édition (panneau « Textures » et/ou menu),
  traduite, qui :
  - invalide le *TextureCache* (`invalidateAll`) ;
  - vide le cache de vignettes (TACHE-01) ;
  - relit `skins.json` (un asset a pu être renommé hors application) ;
  - déclenche un redessin du viewport.
- **Rechargement ciblé** : `invalidate(name)` pour un seul asset, utilisé par l'atelier (LOT-54)
  après enregistrement, afin de ne pas relire toute la bibliothèque à chaque coup de pinceau.
- **Surveillance de dossier optionnelle** (`QFileSystemWatcher`) : si elle est retenue, elle doit
  être **débattue et anti-rebond** — un éditeur d'image externe écrit souvent un fichier en
  plusieurs fois, et recharger sur un fichier partiellement écrit produirait un asset invalide.
  Retenir la surveillance seulement si le comportement est fiable ; sinon, l'action manuelle suffit.
- **Robustesse** : un asset devenu illisible ou invalide entre deux chargements bascule sur le
  repli (`EX-NFR-040`) sans interrompre le rendu ni perdre l'état d'édition.

## Fichiers impactés
- `Source/HMI/Graphics/TextureCache.{h,cpp}` (appelants de l'invalidation).
- `Source/HMI/Editor/TexturePanel.{h,cpp}`, `Source/HMI/Editor/AssetThumbnailView.{h,cpp}`.
- `Source/HMI/Interface/MainWindow.{h,cpp}` (entrée de menu, traduction).
- `Source/Elements/Localization/fr.lang`, `en.lang`.

## Tests (obligatoires)
- Invalidation du cache : après `invalidate(name)`, le prochain accès relit le fichier ; les autres
  entrées sont conservées. `invalidateAll` vide tout. Testé sans GPU sur la partie non graphique du
  cache.
- Un asset devenu invalide après invalidation produit le repli, sans exception.

## Points d'attention
- **Durée de vie des handles** : une texture peut être en cours d'utilisation par la composition de
  l'image courante au moment de l'invalidation. Respecter le contrat fixé en LOT-40 (TACHE-01) —
  invalidation en dehors de la composition, ou durée de vie garantie par `ComPtr` chez l'appelant.
- Le rechargement ne doit **rien** changer à l'état du niveau en cours d'édition ni à l'historique
  d'annulation : c'est une opération de présentation.
- Ne pas recharger pendant un pas de simulation en mode essai.

## Définition de fait (DoD)
- Une action explicite reflète les assets modifiés sur disque sans redémarrer ; le rechargement
  ciblé est disponible pour l'atelier ; un asset devenu invalide dégrade proprement ; l'invalidation
  est testée ; `/W4 /WX` propre.

## Exigences
`EX-EDIT-026` (rechargement à chaud) ; réutilise `EX-ARCH-080` (gestion des ressources, amendée),
`EX-NFR-040` (repli), `EX-REN-033` (traduction), `EX-NFR-041` (RAII).
