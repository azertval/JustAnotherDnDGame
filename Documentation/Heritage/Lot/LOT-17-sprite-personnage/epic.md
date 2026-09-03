# LOT-17 — Sprite du personnage (statique) {#lot-17}

> Statut : **terminé**. Premier de deux lots dédiés au visuel du personnage : celui-ci remplace le
> rectangle de couleur unie par une silhouette humanoïde statique (`EX-REN-011`). L'animation par
> séquence d'images (repos/course/saut, `EX-REN-012`) est **explicitement hors périmètre**, prévue
> comme un lot séparé.

## Objectif
Le personnage est aujourd'hui rendu comme une simple tuile de couleur (`_atlas.tile(1, 1)`, cyan
uni) — aucune forme, aucune lecture visuelle. Ce lot livre une **silhouette humanoïde** simple
(tête, cheveux, torse/manches, mains, jambes, chaussures) générée en code comme le reste de
l'atlas, sans introduire de fichier image ni de dépendance externe (`EX-ARCH-050`).

## Périmètre

### Inclus
- Une région dédiée de l'atlas (16×16 pixels, **carrée** comme une tuile — voir décision de
  cadrage ci-dessous) dessinant une silhouette humanoïde multi-couleurs (peau, cheveux,
  chemise/manches, pantalon, chaussures), avec zones transparentes hors silhouette.
- Le branchement de cette région sur l'entité joueur (`GameScreen::spawnPlayer`), à la place de la
  tuile de couleur unie.

### Exclus (hors périmètre de ce lot)
- **Animation** (repos/course/saut, `EX-REN-012`) — lot séparé à venir ; ce lot livre une **pose
  unique fixe**.
- Sprites des mécanismes (interrupteurs/portes) — déjà couverts par les tuiles existantes,
  inchangés.
- Tout pipeline photo → pixel art ou asset externe — toujours hors périmètre (`EX-DEC-*`).

## Décisions de cadrage
- **La silhouette vit dans `TextureAtlas`, pas dans une classe séparée** (à la différence de
  `SaveIcon`/`FlagIcons`). Raison : `SpriteRenderer::render` dessine **toutes** les entités
  `Transform`+`Sprite` du monde en **une seule passe**, liée à **une seule texture** (`_atlas`,
  cf. `SpriteRenderer.cpp`). Une classe séparée aurait exigé soit une deuxième texture partagée par
  aucune entité de jeu actuelle, soit une restructuration de `SpriteRenderer` pour trier par
  texture — hors de proportion pour ce lot. Étendre l'atlas existant (texture plus haute, une
  nouvelle région) ne touche **ni** `SpriteRenderer` **ni** le contrat `core::AtlasRegion` : la
  normalisation UV se fait déjà génériquement à partir de `atlas.width()`/`height()`.
- **Silhouette par blocs rectangulaires**, pas par distance géométrique (à la différence de
  `FlagIcons`) : plus simple à lire et à faire évoluer pour une forme humanoïde à cette résolution,
  suffisant pour une silhouette reconnaissable à l'échelle du jeu (16 px/unité).
- **La région reste carrée (16×16), le ratio 1:2 vient de `Transform::scale`, pas de la région.**
  Version initiale de ce lot : région déjà non carrée (16×32, reprenant directement le ratio de
  `core::playerSize()`). Bug détecté immédiatement à la vérification visuelle en jeu — la
  silhouette débordait largement de la boîte de collision et traversait les murs — car
  `SpriteRenderer::render` multiplie **déjà** les dimensions de la région par `Transform::scale`
  (lui-même égal à `core::playerSize()`, donc déjà non uniforme 0,4×0,8) : une région 16×32
  appliquait le ratio 1:2 **deux fois** (0,4×1,6 à l'écran au lieu de 0,4×0,8). Corrigé en gardant
  la région carrée (16×16, comme `tile()`) et en dessinant la silhouette **pré-compressée** de
  moitié en hauteur dans ce canevas — l'étirement du `Transform` restitue les proportions
  naturelles à l'écran, exactement comme pour une tuile normale.
- **Un seul lot pour le statique, un second pour l'animation** (décision explicite, périmètre
  demandé) : évite de concevoir la structure de séquence d'images (`EX-REN-012`) avant d'avoir une
  pose de référence validée visuellement.

## Exigences couvertes
- `EX-REN-011` (silhouette humanoïde avec transparence) — partie statique.
- `EX-REN-012` (animation) — **non couverte**, prévue au lot suivant.

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-silhouette-personnage.md) | Silhouette du personnage dans l'atlas | `HMI/Graphics`, `HMI/Interface` | ✅ |
| [TACHE-02](tache-02-documentation-verification.md) | Documentation et vérification | `Documentation` | ✅ |

## Critères d'acceptation du lot
1. Le personnage affiche une silhouette humanoïde reconnaissable (tête, torse, jambes) au lieu
   d'un rectangle de couleur unie, en jeu comme dans l'essai immédiat de l'éditeur.
2. Les zones hors silhouette de la région restent transparentes (pas de halo rectangulaire visible
   autour du personnage).
3. Aucune régression : les tuiles existantes de l'atlas (niveau, mécanismes) restent inchangées à
   l'identique (mêmes régions, mêmes couleurs).
4. Build `/W4 /WX` sans avertissement ; `ctest` vert (292 tests inchangés — aucune nouvelle logique
   testable hors GPU, cf. TACHE-01) ; Doxygen et lint des exigences verts.

## Dépendances
- Étend `TextureAtlas` (LOT-05, génération procédurale) et `core::playerSize()`/`PlayerSpawn`
  (LOT-09) pour le ratio de la région. Consommé par `GameScreen::spawnPlayer` (LOT-07/09).

## Navigation des tâches
- @subpage lot-17-tache-01-silhouette-personnage
- @subpage lot-17-tache-02-documentation-verification
