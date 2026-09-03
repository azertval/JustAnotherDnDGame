# TACHE-03 — Atlas de textures procédural {#lot-05-tache-03-atlas-procedural}

**Lot :** [LOT-05](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** fait

## Contexte
Aucun asset graphique n'existe encore. Pour débloquer le rendu sans dépendre d'un fichier
ni d'un chargeur d'image, l'atlas est **généré en mémoire** : une petite texture
contenant quelques tuiles 16×16 distinctes (couleurs / motifs). Un vrai atlas PNG sera
chargé plus tard, avec les graphismes.

## Travail à réaliser
- Générer en code une texture d'atlas (ex. 64×64 ou 128×128) découpée en **tuiles 16×16**
  identifiables (couleurs pleines, damier, une tuile avec zone **transparente** pour
  tester l'alpha).
- Créer la ressource `ID3D11Texture2D` + `ID3D11ShaderResourceView` à partir des pixels
  générés (RAII).
- Exposer une **table de régions** nommées/indexées (rectangle en pixels par tuile) que le
  `Sprite` (TACHE-01) référence.
- Isoler la génération pour qu'elle soit **remplaçable** par un chargement de fichier plus
  tard (interface d'atlas stable).

## Fichiers impactés
- `Source/HMI/Graphics/TextureAtlas.h`, `TextureAtlas.cpp` (nouveau).
- `Source/HMI/CMakeLists.txt`.

## Vérifications (obligatoires)
- La texture générée est correctement créée (dimensions, format RGBA).
- Les régions retournées correspondent aux tuiles attendues (coordonnées en pixels).
- Une tuile avec alpha rend une zone transparente à l'écran (via la démo).

## Points d'attention
- Génération **déterministe** (mêmes pixels à chaque exécution).
- Frontière claire : le composant `Sprite` référence une **région** (données), la
  résolution région → texture se fait dans `HMI`.
- RAII sur la texture et la vue de ressource.

## Définition de fait (DoD)
- Atlas procédural utilisable par le pipeline, régions exposées, remplaçable à terme ;
  build `/W4 /WX`, documenté.

## Exigences
`EX-REN-010`, `EX-REN-011`, `EX-ARCH-021`.
