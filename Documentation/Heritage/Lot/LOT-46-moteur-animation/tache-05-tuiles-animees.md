# TACHE-05 — Skins de tuiles animés {#lot-46-tache-05-tuiles-animees}

**Lot :** [LOT-46](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** fait

## Contexte
Une fois le moteur généralisé, animer une tuile ne demande presque rien : un skin (LOT-42) désigne
un asset, cet asset peut porter une description d'animation (TACHE-03), et la progression est déjà
appliquée à toute entité animée (TACHE-02).

Cette tâche fait le raccord et vérifie que le cas se comporte correctement à l'échelle : un niveau
peut contenir des centaines de tuiles d'eau ou de lave partageant le même asset.

## Travail à réaliser
- **Câblage** : lorsqu'un skin désigne un asset animé, les entités de tuiles correspondantes
  reçoivent un `core::Animation` à la construction de la scène (`core::buildLevelScene`), et leur
  région de sprite suit l'image courante.
- **Animation partagée** : toutes les tuiles d'un même type animé doivent être **en phase** — une
  nappe d'eau dont chaque case ondule à son propre rythme serait un artefact, pas un effet. La
  progression est donc calculée une fois par asset animé, pas une fois par case.
- **Rendu** : aucune modification du pipeline — seule la région de texture change au fil des images,
  exactement comme pour le personnage.
- **Interaction avec les raccords automatiques** : un type en mode `bitmask16` **et** animé
  combinerait deux façons de choisir la case dans l'image. Trancher explicitement : soit le mode
  `bitmask16` exclut l'animation, soit la spritesheet contient une planche par image. La première
  option est la plus simple et suffit au besoin (l'eau et la lave n'ont pas de raccords) ; la
  documenter comme une limite assumée.
- **Interaction avec le masquage de silhouette** (TACHE-03 de LOT-42) : une pente animée devrait
  être détourée à chaque image. Vérifier le coût, ou exclure la combinaison si elle n'a pas d'usage.

## Fichiers impactés
- `Source/Core/Levels/LevelScene.{h,cpp}` (composant d'animation sur les tuiles concernées).
- `Source/HMI/Graphics/TileVisuals.{h,cpp}`, `AnimationCatalog.{h,cpp}`.
- `Source/HMI/Game/GameSession.{h,cpp}` (mise à jour des régions).
- `Source/Test/Unit/HMI/Graphics/test_animated_tiles.cpp` (nouveau).

## Tests (obligatoires)
- Un skin animé produit des régions différentes au fil des pas fixes ; un skin non animé produit une
  région constante.
- **Mise en phase** : deux tuiles du même type animé affichent la même image au même pas.
- La combinaison exclue (raccords + animation, ou silhouette + animation) est détectée et signalée
  clairement, pas silencieusement ignorée.
- Sans GPU.

## Points d'attention
- **Le coût par image** : mettre à jour la région de centaines d'entités à chaque pas serait
  inutilement coûteux si la progression est partagée. Envisager de résoudre la région au moment de
  la composition plutôt que de l'écrire dans chaque `core::Sprite`.
- La mise en phase implique que l'animation d'une tuile ne dépend **pas** de son instanciation :
  charger un niveau au milieu d'une partie ne doit pas décaler l'animation.
- Ne pas oublier le mode Physique : une tuile animée ne s'anime pas en couleurs plates, et c'est
  volontaire.

## Définition de fait (DoD)
- Un skin désignant un asset animé anime toutes ses tuiles, en phase, sans coût par case ; les
  combinaisons non supportées sont signalées ; tests sans GPU verts ; `/W4 /WX` propre.

## Exigences
`EX-REN-005` (animations par données, tuiles comprises) ; réutilise `EX-EDIT-042` (skins),
`EX-EDIT-025` (raccords automatiques, combinaison à trancher), `EX-REN-046` (bascule),
`EX-NFR-005` (budget de primitives).
