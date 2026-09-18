# Elements/Assets/

**Assets graphiques éditables hors code** (`LOT-39`) : images et polices, copiées à côté de
l'exécutable au build (patron `Levels`/`Localization`, sous `Assets/`) et chargées à l'exécution.
Chaque sous-dossier est une **famille** ; la plupart sont **engendrées** par un script ou un atelier
et ne se retouchent pas à la main — le README de la famille dit lequel.

## Contenu

- `Coliseum/` — pièces de la scène isométrique du Colisée (`LOT-50`), découpées de la planche de
  production par `scripts/extract_coliseum_atlas.py` (voir `Coliseum/README.md`).
- `Entities/` — familles des illustrations d'entité et leur contrat de dimensions
  (`families.json`, `core::loadAssetFamilies`) ; une donnée désigne son image par une **clé**, et
  un marqueur en tient lieu tant qu'aucune n'existe (voir `Entities/README.md`).
- `Fonts/` — polices TTF de l'interface et leurs licences (voir `Fonts/README.md`).
- `Maps/` — cartes peintes par l'auteur : le monde, les régions et les villes (`LOT-94`), décrites
  par `manifest.json`.
- `Npc/` — PNJ de l'atelier : bandes d'animation, `.anim.json` et portrait par PNJ (`LOT-91`, voir
  `Npc/README.md`).
- `Scene/<lieu>/` — textures de scène d'un lieu (`coliseum`, `martpart`), découpées de leurs
  planches par `scripts/extract_texture_sheet.py` (`LOT-92`) : `manifest.json` (planches et
  textures) et `appearance.json` (ce que le lieu pose sur une case, `hmi::PlaceAppearance`).
- `UI/` — illustrations de l'interface (`background/menu-scene.png`, le fond du menu), décrites par
  `illustrations.json`.

Le canevas de l'éditeur distingue les types de tuile par l'atlas **procédural** de
`hmi::TextureAtlas` (`EX-NFR-040`) : aucun fichier d'atlas n'est livré ni lu.

Tout asset livré ici doit paraître dans la galerie de débug (`--screen=AssetGallery`,
`EX-CNT-042`).

> Anciennement `Textures/` (réservé, jamais peuplé) — ce dossier est le point d'entrée réel des
> assets graphiques depuis le LOT-39, nommé `Assets/` pour suivre le même patron que `Levels/` et
> `Localization/`.
