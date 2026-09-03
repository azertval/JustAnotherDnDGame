# LOT-42 — Skin des tuiles : jeux de skins et raccords automatiques {#lot-42}

> Statut : **fait**. Prérequis : [LOT-40](@ref lot-40), [LOT-41](@ref lot-41). Répond à la
> demande d'origine (« définir les textures utilisées par un bloc »).

## Objectif
Permettre au level designer d'associer une texture réelle à chaque type de tuile — le cœur de la
demande initiale. Le lot va délibérément plus loin que « un PNG par type » sur deux points, parce
que les deux sont des décisions de **format** qu'on ne peut pas prendre après coup sans refonte :

- **Jeux de skins nommés** (`EX-EDIT-024`). Une association strictement globale à tout le jeu
  condamnerait tous les niveaux à la même apparence, alors que le jeu est structuré en progression
  par salles. Un jeu de skins est un ensemble nommé (« forêt », « grotte ») ; un niveau pourra en
  désigner un (LOT-44).
- **Raccords automatiques** (`EX-EDIT-025`). Une image unique par type répète la même case partout :
  la grille reste visible et le dessus d'une plateforme est identique à son intérieur — c'est le
  rendu physique actuel, en couleur. Le rendu choisit donc l'image dans une **planche** selon les
  tuiles solides voisines.

C'est aussi le lot qui introduit le panneau Qt **« Textures »**, conçu dès maintenant pour être
réutilisé (pas remplacé) par LOT-43/44/45/50.

## Périmètre

### Inclus
- **`skins.json`** : fichier de configuration **hors `Core`**, jamais lu ni écrit par
  `Level`/`LevelDraft`. Structure : un `"version"` de format, puis des **jeux de skins nommés**,
  chacun associant un `TileType` à `{ "asset": <fichier>, "mode": "single" | "bitmask16" }`. Copié à
  côté de l'exécutable comme `Levels`/`Localization` (`Source/Elements/Assets/skins.json` = version
  livrée par défaut). L'éditeur **écrit** au chemin déployé (`executableDirectory()/Assets/skins.json`),
  exactement comme l'enregistrement d'un niveau (`Source/HMI/Editor/LevelFileOperations.cpp`) —
  aucun nouveau mécanisme d'écriture à inventer.
- **Sélection par fichiers indépendants** : `Assets/Skins/*.png`, listés par balayage de dossier —
  aucune saisie de chemin.
- **Raccords automatiques (`bitmask16`)** : table **pure** masque des voisins solides → index de
  case dans une planche 4×4, voisinage lu via `core::isSolid`/`TileMap::isSolid` (aucune nouvelle
  surface `Core`). Déterministe et testée sans GPU. Le contrat de planche (4×4 cases de
  `TILE_SIZE`) est validé par le contrat d'asset de LOT-40 et documenté dans
  `Source/Elements/Assets/README.md`.
- **Mode `single`** : une image unique, pour tous les types dont le voisinage n'a pas de sens
  (dangers, interrupteurs, portes, blocs, pentes et arrondis).
- **Masquage alpha automatique des pentes/arrondis** : pour les douze types à silhouette
  inclinée/courbe, le skin chargé est **détouré** à la bonne forme au chargement, via les fonctions
  pures déjà existantes (`core::slopeSurfaceHeight`/`ceilingSlopeHeight`, `core::isCeilingSlope`) —
  l'artiste peint un carré plein, le moteur applique le masque une fois (16×16 pixels, coût
  négligeable, résultat mis en cache comme toute texture du *TextureCache*).
- **Panneau Qt « Textures »** (nouveau dock, même patron d'enregistrement que `PalettePanel`/
  `LevelsPanel`/`ToolPanel`/`LinksPanel` dans `MainWindow`) : sélection du jeu de skins courant, puis
  une section « Skins » listant chaque `TileType` avec son fichier et son mode. Conçu en
  sections/onglets dès ce lot pour que LOT-44 (Fond), LOT-45 (Objets), LOT-47 (Animations) et
  LOT-50 (Décors) y ajoutent la leur, sans nouveau panneau.
- **Palette de l'éditeur fidèle** (`EX-EDIT-027`) : `PalettePanel` affiche la texture réellement
  assignée en mode Texture, la couleur plate en mode Physique. Peindre sans voir ce que l'on pose
  serait une régression d'usage introduite par ce lot même.
- Résolution en mode Texture : type skinné → texture assignée (case choisie par les raccords si
  `bitmask16`) ; type non skinné → repli en damier magenta (LOT-40) + `GRAPHICS_LOG_WARNING`.
- Toute UI de ce panneau passe par le catalogue de traduction (`_loc.text`, aucun libellé en dur).
- Dossier `Assets/Skins/` + commande `POST_BUILD` dans `Source/HMI/CMakeLists.txt`.

### Exclus (hors périmètre de ce lot)
- **Désignation du jeu de skins par un niveau** : le format des jeux est posé ici, mais le champ
  `skinSet` sur `Level` relève de LOT-44 (premier lot qui touche `Core` et le format JSON). Ce lot
  utilise le jeu par défaut.
- Fond de niveau, objets interactifs, décors, animations (LOT-44/45/46/47/49).
- Vignettes et aperçu visuel dans le panneau (liste texte pour ce lot ; consolidé en LOT-43).
- Rechargement à chaud (LOT-43).
- Variantes aléatoires par position (autre stratégie de dé-répétition) : `single` et `bitmask16`
  couvrent le besoin ; le champ `mode` accueillera une troisième valeur si le besoin apparaît, sans
  changer le format.

## Décisions de cadrage
- **Frontière `Core`/`HMI` non négociable** : `skins.json` est une configuration de présentation,
  jamais une donnée de simulation — ne transite jamais par `LevelDraft` ni son undo/redo. Seule la
  **désignation** du jeu par un niveau (LOT-44) est une donnée de niveau, et ce n'est qu'une chaîne.
- **`mode` par type, pas par jeu** : un même jeu contient des types à raccords (solides) et des types
  à image unique (dangers, mécanismes). Un mode global obligerait à des exceptions dans le code.
- **Raccords sur 4 voisins (16 cases), pas 8 (47 cases)** : couvre bords et coins concaves, et
  demande à l'auteur une planche de seize cases plutôt que quarante-sept — proportionné à un jeu
  dont les assets sont dessinés à la main. Le champ `mode` permettra d'ajouter `bitmask47` sans
  refonte si le rendu s'avère insuffisant.
- **Format versionné dès l'origine** : `skins.json` est un fichier neuf ; ajouter `"version"`
  maintenant coûte une ligne, l'ajouter après trois lots coûte une migration.
- **Mise à l'échelle des blocs réduits gratuite** : `BlockHalf`/`BlockQuarter`
  (`core::tileVisualScale`) fonctionnent sans code supplémentaire — `SpriteRenderer` applique déjà
  l'échelle du `Transform` quelle que soit la source de la texture.
- **Un seul panneau « Textures »**, pas un panneau par couche : décision transverse au programme,
  prise ici puisque c'est le premier panneau créé.

## Exigences couvertes
- Nouvelles : `EX-EDIT-024` (jeux de skins nommés), `EX-EDIT-025` (raccords automatiques),
  `EX-EDIT-027` (palette fidèle au rendu).
- Amendée : `EX-EDIT-042` (association type de tuile → texture, avec mode de rendu).
- Réutilisées : `EX-REN-043` (rendu multi-textures), `EX-REN-046` (bascule Physique/Texture),
  `EX-REN-007` (contrat d'asset), `EX-NFR-040` (repli), `EX-REN-033` (traduction), `EX-NFR-004`
  (vérification sans GPU).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-skins-json.md) | `skins.json` versionné, jeux de skins, lecture/écriture, résolution `TileType → asset + mode` | `Source/HMI/Graphics` | ✅ |
| [TACHE-02](tache-02-raccords-automatiques.md) | Table de raccords automatiques (voisinage solide → case de planche), pure et testée | `Source/HMI/Graphics` | ✅ |
| [TACHE-03](tache-03-masquage-pentes.md) | Masquage alpha automatique des pentes/arrondis au chargement d'un skin | `Source/HMI/Graphics` | ✅ |
| [TACHE-04](tache-04-panneau-textures.md) | Panneau Qt « Textures » (jeu courant + section Skins) + dossier `Assets/Skins/` + `POST_BUILD` | `Source/HMI/Editor`, `Source/HMI/Interface`, `Source/HMI/CMakeLists.txt` | ✅ |
| [TACHE-05](tache-05-palette-fidele.md) | Palette de l'éditeur fidèle au mode de rendu courant | `Source/HMI/Editor` | ✅ |

## Critères d'acceptation du lot
1. Assigner un skin à un type de tuile depuis le panneau « Textures » le rend visible dans le niveau
   et dans l'éditeur, en mode Texture uniquement.
2. Un type en mode `bitmask16` affiche des **bords et des coins distincts de son intérieur** selon
   son voisinage solide ; la table de correspondance est testée exhaustivement sans GPU.
3. Un type sans skin assigné affiche le damier magenta + un avertissement de log nommant le type.
4. Les tuiles à silhouette inclinée/courbe affichent leur skin **découpé à la bonne forme**, pas un
   carré plein.
5. La palette de l'éditeur montre la texture réelle en mode Texture et la couleur plate en mode
   Physique.
6. `skins.json` porte un numéro de version et ne transite jamais par `Core`/`LevelDraft`.
7. Toute nouvelle UI passe par le catalogue de traduction.
8. Résolution `skins.json` et table de raccords testées sans GPU ; build `/W4 /WX`, Doxygen, lint
   verts.

## Dépendances
Bâtit sur [LOT-40](@ref lot-40) (calques, *TextureCache*, contrat d'asset, *QuadRecorder*) et
[LOT-41](@ref lot-41) (bascule). Réutilise `hmi::AssetPaths` (LOT-39) et la géométrie de pentes de
`Core`. Ne modifie pas `Level`/`LevelDraft`. Prépare [LOT-43](@ref lot-43), [LOT-44](@ref lot-44),
[LOT-45](@ref lot-45), [LOT-50](@ref lot-50) (panneau « Textures » réutilisé) et
[LOT-54](@ref lot-54) (mode planche de l'atelier).

## Navigation des tâches
- @subpage lot-42-tache-01-skins-json
- @subpage lot-42-tache-02-raccords-automatiques
- @subpage lot-42-tache-03-masquage-pentes
- @subpage lot-42-tache-04-panneau-textures
- @subpage lot-42-tache-05-palette-fidele
