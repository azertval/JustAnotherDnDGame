# TACHE-03 — Format `nom-asset.anim.json` : lecture et mise en cache {#lot-46-tache-03-format-anim-json}

**Lot :** [LOT-46](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** fait

## Contexte
Les clips sont désormais des données (TACHE-01), mais rien ne les décrit encore : il faut un format
de fichier, et décider **où** il vit.

Une animation est une propriété **de l'image**, pas du niveau qui l'utilise ni de l'association
type → fichier : un même asset animé assigné à dix cases n'a qu'une description. Le fichier
accompagne donc l'asset.

## Travail à réaliser
- **Format `nom-asset.anim.json`**, à côté du PNG, décrivant la spritesheet et ses clips :
  ```json
  {
    "version": 1,
    "frameWidth": 16, "frameHeight": 16,
    "clips": {
      "closed":  { "frames": [0], "loop": true },
      "opening": { "frames": [1, 2, 3, 4], "frameDuration": 0.06, "loop": false, "next": "open" },
      "open":    { "frames": [5], "loop": true }
    }
  }
  ```
  Disposition de spritesheet **horizontale** : les images se suivent de gauche à droite, l'index est
  une position dans cette suite.
- **Lecture** (nlohmann/json) sans exception, sur le même patron d'erreurs que `LevelLoader` :
  résultat porteur d'un code exploitable, message nommant le fichier.
- **Absence de fichier = image fixe**, sans erreur **ni avertissement**. C'est le cas par défaut :
  la grande majorité des assets ne sont pas animés, et signaler l'absence produirait un bruit de log
  permanent. À distinguer soigneusement d'un asset **référencé mais introuvable**, qui reste une
  anomalie (`EX-NFR-040`).
- **Mise en cache** : la description est chargée une fois par asset et invalidée en même temps que
  la texture (`TextureCache::invalidate`, LOT-40) — sinon le rechargement à chaud d'un asset animé
  laisserait une description obsolète.
- **Traduction index d'image → région de texture** : à partir de la taille d'image déclarée et de la
  largeur de la spritesheet. Fonction **pure**, testable.
- **Validation** : cohérence entre les dimensions du PNG et la taille d'image déclarée, index hors
  bornes, clip suivant inexistant, version inconnue — vérifiée à la lecture, via le contrat d'asset
  de LOT-40.

## Fichiers impactés
- `Source/HMI/Graphics/AnimationCatalog.{h,cpp}` (nouveau).
- `Source/HMI/Graphics/TextureCache.{h,cpp}` (invalidation conjointe).
- `Source/Test/Unit/HMI/Graphics/test_animation_catalog.cpp` (nouveau).

## Tests (obligatoires)
- Round-trip de lecture : clips multiples, durée par défaut, clip joué une fois avec suivant.
- Cas d'erreur : JSON invalide, index hors bornes, `next` inexistant, taille d'image incohérente
  avec le PNG, version inconnue — chacun produit un résultat exploitable **sans exception**.
- Absence de fichier → image fixe, **sans** message de log.
- Calcul index → région : première image, dernière image, spritesheet à une seule ligne.
- Tout est **pur** : ni GPU, ni Qt.

## Points d'attention
- **Ne pas mettre la description dans `skins.json`** : elle serait dupliquée pour chaque type
  utilisant le même asset, et perdue si l'asset est assigné par surcharge (LOT-45) plutôt que par
  skin.
- Le format porte sa propre `version`, pour la même raison que celui des niveaux.
- La durée est en **secondes** dans le fichier, mais la progression est en **pas fixes** : la
  conversion doit être faite une fois, à la lecture, et documentée — sinon une modification du pas
  fixe changerait silencieusement toutes les vitesses d'animation.

## Définition de fait (DoD)
- Une description d'animation est lue, validée et mise en cache ; un asset sans description est une
  image fixe silencieuse ; les erreurs sont exploitables et sans exception ; tests purs verts ;
  `/W4 /WX` propre.

## Exigences
`EX-REN-005` (animations décrites par des données) ; réutilise `EX-REN-007` (contrat d'asset),
`EX-REN-041` (décodage image), `EX-NFR-040` (erreur récupérable), `EX-NFR-010` (testable sans GPU).
