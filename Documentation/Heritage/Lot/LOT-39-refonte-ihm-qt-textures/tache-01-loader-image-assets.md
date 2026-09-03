# TACHE-01 — Loader image → texture D3D11 (`QImage`/`stb_image`) + résolution d'assets testable {#lot-39-tache-01-loader-image-assets}

**Lot :** [LOT-39](epic.md) · **Emplacement :** `Source/HMI/Graphics`, `Source/Editor` · **Statut :** fait

## Contexte
Aucun **loader d'image** n'existe : `hmi::TextureAtlas` crée sa texture D3D11 à partir de pixels
**générés en code** (`CreateTexture2D` + `CreateShaderResourceView`). Cette tâche introduit le
**décodage d'un fichier image en pixels RGBA** puis la création de la texture D3D11 correspondante,
plus la **résolution du chemin d'asset** (où trouver les fichiers) isolée en logique testable.

## Travail à réaliser
- **Décodeur image → RGBA** :
  - Par défaut **`QImage`** (`QImage::load` → `Format_RGBA8888`), déjà disponible via Qt (`LOT-34`),
    aucune dépendance supplémentaire.
  - Alternative documentée **`stb_image`** (ajout `External/`, sans Qt) si un chargement **sans Qt**
    (côté jeu pur) devient nécessaire.
- **Création de la texture D3D11** : à partir des pixels RGBA + dimensions, réutiliser exactement le
  chemin `CreateTexture2D` (`D3D11_TEXTURE2D_DESC` R8G8B8A8_UNORM) + `CreateShaderResourceView` déjà
  employé par `TextureAtlas` ; renvoyer un `ShaderResourceView` (ComPtr, RAII).
  - Fonction utilitaire `hmi::loadTexture(device, path) -> TextureResource` (ou intégrée à un
    `TextureLoader`).
- **Résolution d'assets** (`Source/Editor/AssetPaths.{h,cpp}` ou `HMI`, **pur**) : localiser le dossier
  `Assets/` à côté de l'exécutable (comme `Levels`/`Localization` via `hmi::executableDirectory()`),
  résoudre un nom logique → chemin fichier, signaler un asset manquant.

## Fichiers impactés
- `Source/HMI/Graphics/TextureLoader.{h,cpp}` (nouveau : décodage + création SRV).
- `Source/Editor/AssetPaths.{h,cpp}` (ou `Source/HMI/...`) : résolution de chemins, **pur**.
- `Source/Test/Unit/.../test_asset_paths.cpp` (nouveau).
- Éventuel `External/CMakeLists.txt` si `stb_image` retenu.

## Tests (obligatoires)
- **Résolution d'assets pure** (sans GPU) : nom logique → chemin attendu ; asset manquant détecté ;
  robustesse chemin inexistant.
- **Décodage** : si testable sans GPU (dimensions/format d'une image connue via `QImage`), le couvrir ;
  la **création de la texture D3D11** relève de la vérification visuelle (dépendance GPU).
- **Vérification manuelle** : une image PNG connue est chargée et affichable (validée en TACHE-02).

## Points d'attention
- **Format et prémultiplication alpha** : cohérence avec le blend state de `SpriteBatch` (alpha
  simple) ; convertir en `RGBA8888` non prémultiplié (ou ajuster le blend) — le documenter.
- **Filtrage pixel art** : l'échantillonnage *nearest* est fixé dans `SpriteBatch` (inchangé) ; ne pas
  introduire de filtrage linéaire qui flouterait.
- **Chemins** : passer par `hmi::executableDirectory()`, pas de chemin en dur.

## Définition de fait (DoD)
- Chargement d'un PNG → SRV D3D11 fonctionnel ; résolution d'assets **testée** ; `/W4 /WX` propre.

## Exigences
`EX-REN-041` (chargement de textures depuis fichiers) ; `EX-NFR-010` (résolution testable),
`EX-NFR-040` (asset manquant) ; réutilise `EX-ARCH-022` (nearest).

## Réalisé
`QImage` retenu (pas de `stb_image` : aucun besoin de chargement sans Qt identifié). Fichiers
livrés : `Source/HMI/Graphics/TextureLoader.{h,cpp}` (décodage + `createTexture`, upload D3D11
partagé avec le repli procédural) et `Source/HMI/Graphics/AssetPaths.{h,cpp}` (résolution pure, pas
`Source/Editor` — ce dossier n'existe pas dans l'arborescence actuelle, cf. `Source/HMI/CMakeLists.txt`).
Testé : `Source/Test/Unit/HMI/Graphics/test_asset_paths.cpp`.
