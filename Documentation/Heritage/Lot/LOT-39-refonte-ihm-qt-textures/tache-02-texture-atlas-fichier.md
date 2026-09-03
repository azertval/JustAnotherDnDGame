# TACHE-02 — `TextureAtlas` sur atlas fichier (interface conservée, repli procédural) {#lot-39-tache-02-texture-atlas-fichier}

**Lot :** [LOT-39](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** fait

## Contexte
`hmi::TextureAtlas` est **explicitement conçu pour être remplacé par un chargement de fichier** : son
interface publique (`textureView()`, `tile(col,row)`, `playerFrameRegion(...)`, `width()`/`height()`)
est stable et consommée par `SpriteRenderer` et l'éditeur. Cette tâche fait basculer son
**implémentation** vers un **atlas image chargé depuis un fichier** (via le loader TACHE-01), **sans
changer l'interface** ni les appelants, avec **repli procédural** si l'asset manque.

## Travail à réaliser
- **Atlas fichier** : charger une image d'atlas (`Assets/atlas.png` ou équivalent) + une **description
  des régions** (mapping `tile(col,row)`/`playerFrameRegion` → rectangles en pixels). La description
  peut être une convention de grille (comme aujourd'hui : `TILE_SIZE`, `TILES_PER_SIDE`) ou un petit
  fichier de métadonnées — arbitrer (commencer simple : même grille que l'actuel).
- **Interface inchangée** : `TextureAtlas` expose les mêmes méthodes ; `SpriteRenderer` et l'éditeur
  **ne sont pas modifiés**. Les `core::AtlasRegion` renvoyés restent en pixels (conversion UV par
  l'appelant, inchangée).
- **Repli procédural** : si l'atlas fichier est absent/illisible, retomber sur la génération
  procédurale actuelle (conserver le code existant comme repli), avec un message clair. Permet une
  **migration incrémentale**.
- **Migration tuile par tuile** possible : documenter comment remplacer progressivement les couleurs
  procédurales par de vraies tuiles.

## Fichiers impactés
- `Source/HMI/Graphics/TextureAtlas.{h,cpp}` (implémentation fichier + repli ; interface conservée).
- `Source/Elements/Assets/` (atlas image de base) + copie POST_BUILD (patron `Levels`/`Localization`).

## Tests (obligatoires)
- **Mapping des régions testable** (sans GPU) : `tile(col,row)`/`playerFrameRegion` renvoient les
  rectangles attendus pour la description d'atlas donnée (mêmes valeurs qu'aujourd'hui pour la grille
  par défaut → non-régression).
- **Repli** : en l'absence d'asset, l'atlas procédural est utilisé (comportement défini, pas de crash).
- **Vérification manuelle** : les tuiles s'affichent avec la texture fichier (net, *nearest*) en jeu
  et dans l'éditeur ; le personnage animé reste correct.

## Points d'attention
- **Stabilité de l'interface** : c'est la garantie qui évite de toucher `SpriteRenderer`/éditeur — la
  vérifier (aucune signature publique modifiée).
- **Parité des régions** : conserver la convention de grille existante pour ne rien casser, puis faire
  évoluer l'atlas.
- **Copie d'assets** : brancher la copie POST_BUILD sur la cible `JustAnotherDnDGame`
  (`Assets/` → à côté de l'exe), et le déploiement release.

## Définition de fait (DoD)
- `TextureAtlas` chargé depuis un fichier, interface et appelants **inchangés**, repli procédural
  fonctionnel ; mapping **testé** ; `/W4 /WX` propre ; rendu vérifié visuellement.

## Exigences
`EX-REN-041` (textures fichiers), `EX-REN-042` (assets externalisés, repli) ; réutilise `EX-ARCH-022`
(nearest), `EX-NFR-040` (repli robuste).

## Réalisé
`tile`/`playerFrameRegion` rendues `static` (aucun état d'instance) et déplacées dans un fichier
séparé, `Source/HMI/Graphics/TextureAtlasRegions.cpp` — dépourvu de dépendance Qt/Direct3D 11, il
est compilé et testé directement dans `UnitTests` (`Source/Test/Unit/HMI/Graphics/test_texture_atlas.cpp`),
contrairement à `TextureAtlas.cpp` (chargement fichier + repli, dépendant de Qt/D3D11). La
génération procédurale historique est extraite telle quelle (même sortie, non-régression) dans
`Source/HMI/Graphics/ProceduralAtlas.{h,cpp}` (pur, testé par `test_procedural_atlas.cpp`), servant
à la fois de repli et de générateur pour l'atlas de base
(`JustAnotherDnDGame.exe --export-atlas=...`, cf. TACHE-03). Grille de l'atlas fichier livré identique à
la grille procédurale (parité vérifiée par export + rechargement, dimensions 80×112).
