# TACHE-01 — *TextureCache* : registre par nom logique, invalidation, contrat d'asset {#lot-40-tache-01-texture-cache}

**Lot :** [LOT-40](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** fait

## Contexte
`hmi::TextureAtlas` charge **une seule** texture fixe (`Assets/atlas.png`). Les lots suivants (skin
des tuiles, fond de niveau, objets interactifs, décors, personnage) ont besoin de charger un
**nombre variable** de textures indépendantes, chacune identifiée par son nom de fichier logique
(ex. `"forest.png"` dans `Assets/Backgrounds/`). Cette tâche introduit le registre qui les charge à
la demande et les met en cache, en réutilisant telles quelles les primitives de LOT-39.

## Travail à réaliser
- ***TextureCache*** (`Source/HMI/Graphics/TextureCache.{h,cpp}`) :
  - Construit avec un `ID3D11Device*` et un `hmi::AssetPaths` (dossier de base).
  - `get(const std::string& fileName) -> const LoadedTexture*` (ou `std::optional<...>` selon
    convention) : résout via `AssetPaths::resolve`, charge via `hmi::loadTextureFromFile` au premier
    accès, met en cache par nom, renvoie le même pointeur/handle aux accès suivants ; renvoie
    `nullptr`/`nullopt` sans exception si l'asset est absent/illisible (`EX-NFR-040`) — **ne génère
    pas** lui-même le repli en damier magenta (TACHE-03 s'en charge, en amont de l'appelant).
  - `invalidate(const std::string& fileName)` et `invalidateAll()` : retirent une entrée (ou toutes)
    du cache, de sorte que le prochain `get` relise le fichier depuis le disque. **Font partie de
    l'API dès ce lot**, même si aucun appelant ne les utilise avant LOT-43 : le rechargement à chaud
    (LOT-43) et l'aperçu live de l'atelier (LOT-54) en dépendent, et les greffer après coup
    obligerait à modifier une API déjà consommée par quatre lots.
  - RAII strict : toutes les textures possédées sont libérées à la destruction du cache
    (`Microsoft::WRL::ComPtr`, comme `TextureAtlas`/*TextureLoader*).
- **Contrat d'asset et validation** (`EX-REN-007`) : une description par **famille** d'asset (skin
  de tuile, planche à raccords, fond, objet, spritesheet de personnage) déclarant les dimensions
  attendues — multiple de la taille de tuile, nombre de cases, carré ou non. Le cache **vérifie** les
  dimensions décodées et **refuse** un asset non conforme, en journalisant via
  `GRAPHICS_LOG_WARNING` le **nom du fichier**, la **dimension trouvée** et la **dimension
  attendue**. Sans cette validation, un PNG de travers ne produit aucune erreur mais des artefacts
  de filtrage silencieux (`POINT`, sans mipmap). La validation est une fonction **pure**, séparée du
  décodage et de l'upload, donc testable sans GPU.
- Pas d'éviction (cf. décision de cadrage de l'epic) : le cache grossit avec les assets rencontrés,
  jamais libéré avant sa propre destruction. `invalidate` sert au rechargement, **pas** à une
  politique de libération mémoire.

## Fichiers impactés
- `Source/HMI/Graphics/TextureCache.{h,cpp}` (nouveau).
- `Source/HMI/Graphics/AssetContract.{h,cpp}` (nouveau) — familles d'assets et validation pure des
  dimensions.
- `Source/Test/Unit/HMI/Graphics/test_asset_contract.cpp` (nouveau).

## Tests (obligatoires)
- Validation des dimensions par famille d'asset : conforme accepté, non conforme refusé, message
  contenant le nom de fichier et l'attendu — **pure, sans GPU**.
- Logique de résolution/mise en cache (nom → même handle réutilisé, asset absent → repli sans
  exception, `invalidate` force une relecture) : la partie sans GPU est testable via un `AssetPaths`
  pointant un dossier temporaire ; le chargement GPU effectif relève de la vérification visuelle
  (comme *TextureLoader*, LOT-39).

## Points d'attention
- Ne pas dupliquer la logique de `TextureAtlas::loadFromFile` : *TextureCache* et `TextureAtlas`
  partagent `hmi::loadTextureFromFile`/`hmi::AssetPaths`, aucun des deux ne réimplémente le
  décodage/upload.
- Chemins toujours résolus via `hmi::AssetPaths`, jamais de chemin en dur.
- `invalidate` pendant une image en cours de composition : un appelant peut détenir un handle déjà
  retiré du cache. Décider explicitement du contrat (invalidation différée en fin d'image, ou
  durée de vie garantie par `ComPtr` chez l'appelant) plutôt que de le laisser implicite.

## Définition de fait (DoD)
- *TextureCache* charge, met en cache et invalide plusieurs textures indépendantes ; asset manquant
  ou non conforme géré sans exception, avec un message exploitable ; validation testée sans GPU ;
  `/W4 /WX` propre.

## Exigences
`EX-REN-043` (rendu multi-textures), `EX-REN-007` (contrat d'asset) ; réutilise `EX-NFR-040`
(repli), `EX-NFR-010` (testable sans GPU).
