# LOT-39 — Refonte IHM (Qt) : textures depuis fichiers (loader + assets) {#lot-39}

> Statut : **fait**. Prérequis : [LOT-34](@ref lot-34) (socle) ; s'intègre à l'éditeur des
> LOT-35 → LOT-37.

## Objectif
Lever le dernier point de douleur du demandeur (« lorsque je vais implémenter des textures, cela va
être compliqué »). Aujourd'hui **toutes** les textures sont **générées procéduralement en mémoire**
(`TextureAtlas`, `BitmapFont`, `FlagIcons`, `SaveIcon`) — **aucun loader d'image** n'existe. Ce lot
introduit le **chargement de textures depuis des fichiers image** et un **remplacement progressif**
du `TextureAtlas` procédural par des assets fichiers, de sorte que les tuiles (et à terme le
personnage/les décors) s'affichent avec de vraies textures, éditables hors code.

## Périmètre

### Inclus
- **Loader d'images → texture D3D11** : décodage d'un fichier image (PNG au minimum) en pixels RGBA,
  puis `CreateTexture2D` + `CreateShaderResourceView` (mêmes appels que `TextureAtlas` aujourd'hui).
  - Décodeur : **`QImage`** (déjà disponible via Qt, pas de dépendance supplémentaire) ou `stb_image`
    (`External/`, sans Qt) — à trancher au cadrage. `QImage` est privilégié tant que le chargement se
    fait côté éditeur/outillage.
- **Chargement des tuiles depuis un atlas fichier** : un atlas image + description des régions
  remplace la grille de couleurs procédurale ; l'**interface publique de `TextureAtlas`**
  (`textureView()`, `tile(col,row)`, régions) — déjà « conçue pour être remplacée » — est
  **conservée**, seule son implémentation change. Repli sur le procédural si un asset manque
  (robustesse).
- **Gestion d'assets minimale** : convention de dossier (`Assets/` copié à côté de l'exécutable,
  comme `Levels`/`Localization`), chargement au démarrage, rechargement à chaud optionnel dans
  l'éditeur.
- **Panneau/aperçu d'assets** (optionnel selon cadrage) : visualiser les textures chargées dans
  l'éditeur.
- **Filtrage pixel art préservé** : échantillonnage *nearest* (déjà en place dans `SpriteBatch`),
  aucune interpolation qui flouterait le pixel art.
- Documentation (guide rendu — pipeline de textures fichiers) et tests de la logique nouvelle
  (résolution de chemin d'asset, mapping des régions) découplée du GPU.

### Exclus (hors périmètre de ce lot)
- **Pipeline photo → pixel art, décors riches** (`EX-DEC-*`) — toujours hors périmètre du projet.
- **Compression de texture (DDS/BCn), atlas packing automatique, mipmaps** — le décodage simple
  (PNG → RGBA) suffit à l'échelle actuelle ; l'optimisation est un lot futur éventuel.
- **Remplacement des polices/icônes procédurales** (`BitmapFont`, `FlagIcons`, `SaveIcon`) — hors UI
  Qt (qui a ses propres polices/icônes) ; ce lot cible les **textures de jeu** (tuiles), pas l'UI.
- **Animation du personnage par fichiers** — peut réutiliser le loader plus tard ; ce lot se
  concentre sur les tuiles pour dérisquer le pipeline.

## Décisions de cadrage
- **Conserver l'interface de `TextureAtlas`, ne changer que l'implémentation** : le rendu
  (`SpriteRenderer`, éditeur) consomme déjà `textureView()` + régions ; garder ce contrat évite de
  toucher aux appelants et permet un basculement procédural ↔ fichier transparent (repli inclus).
- **Décodeur `QImage` par défaut** : Qt est déjà une dépendance depuis le [LOT-34](@ref lot-34), donc
  aucune lib supplémentaire ; `stb_image` reste l'alternative si un chargement **sans Qt** (côté jeu
  pur) devient nécessaire.
- **Assets copiés à côté de l'exécutable** comme `Levels`/`Localization` (patron CMake POST_BUILD
  existant) : cohérent avec l'organisation actuelle, éditable hors code.
- **Repli procédural si asset manquant** : ne jamais bloquer le rendu sur un fichier absent —
  robustesse (`EX-NFR-040`), et migration incrémentale tuile par tuile possible.

## Exigences couvertes
- Nouvelles : `EX-REN-041` (chargement de textures depuis fichiers image), `EX-REN-042` (assets de
  jeu externalisés, éditables hors code, avec repli procédural).
- Réutilisées : `EX-ARCH-022` (échantillonnage pixel art *nearest*), `EX-NFR-040` (robustesse —
  asset manquant), `EX-NFR-010` (logique de résolution d'assets testable sans GPU).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-loader-image-assets.md) | Loader image → texture D3D11 (`QImage`/`stb_image`) + résolution d'assets testable | `Source/HMI/Graphics`, `Source/Editor` | ✅ |
| [TACHE-02](tache-02-texture-atlas-fichier.md) | `TextureAtlas` sur atlas fichier (interface conservée, repli procédural) | `Source/HMI/Graphics` | ✅ |
| [TACHE-03](tache-03-convention-assets-doc.md) | Convention d'assets + (option) aperçu ; documentation & vérification | `Source/Elements`, `Source/Editor`, `Documentation` | ✅ |

## Critères d'acceptation du lot
1. Une **texture chargée depuis un fichier PNG** s'affiche sur les tuiles du niveau (éditeur et jeu),
   au filtrage pixel art *nearest* (net, non flou).
2. L'interface publique de `TextureAtlas` est **inchangée** ; `SpriteRenderer` et l'éditeur n'ont pas
   été modifiés pour consommer les textures fichiers.
3. Un asset **manquant** ne bloque pas le rendu : repli procédural, message clair.
4. Les assets sont copiés à côté de l'exécutable et **éditables hors code** (remplacer le fichier
   suffit) ; le rechargement est documenté.
5. La logique de résolution d'assets / mapping de régions est **couverte par des tests** sans GPU.
6. Build `/W4 /WX`, Doxygen et lint verts ; rendu des textures **vérifié visuellement**.

## Dépendances
- Bâtit sur [LOT-34](@ref lot-34) et l'éditeur (LOT-35 → LOT-37). Réutilise/étend
  `hmi::TextureAtlas`/`SpriteBatch`/`SpriteRenderer` (`LOT-05`) et le patron CMake POST_BUILD de copie
  d'assets (`Levels`/`Localization`). `QImage` (Qt, `LOT-34`) ou `stb_image` (nouvelle dépendance
  `External/`). Ne modifie pas `Core`.

## Navigation des tâches
- @subpage lot-39-tache-01-loader-image-assets
- @subpage lot-39-tache-02-texture-atlas-fichier
- @subpage lot-39-tache-03-convention-assets-doc
