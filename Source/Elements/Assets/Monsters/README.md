# Les monstres de l'atelier (LOT-93)

Un dossier par créature, `<slug>/`, avec ses bandes d'animation, leurs `.anim.json` et `portrait.png`
(son portrait pixel art, 1 024 × 1 024), et un `manifest.json` à la racine. Les dossiers sont écrits
par `Documentation/Lot/LOT-93-atelier-monstres/atelier/scripts/integre.py`, jamais à la main ; les
références de chaque créature (fiche, bloc de prompt, palette, portrait) restent dans
`atelier/monstres/<slug>/`.

Les bandes sont celles des PNJ (`Npc/`), à la taille du **gabarit** de la créature :

| Bande | Images | Moyen (1 case) | Grand (2 × 2 cases) | Durée / image | Boucle |
|---|---|---|---|---|---|
| `idle.png` | 6 | 48 × 64 | 96 × 96 | 0,15 s | oui |
| `walk.png` | 8 | 48 × 64 | 96 × 96 | 0,10 s | oui |
| `hit.png` | 4 | 48 × 64 | 96 × 96 | 0,08 s | non |
| `death.png` | 6 | 96 × 64 | 96 × 96 | 0,12 s | non, tenue |
| `attack.png` | 8 | 96 × 64, pied à x = 32 | 192 × 96, pied à x = 64 | 0,08 s | non |
| `cast.png` | 8 | 96 × 64, pied à x = 32 | 192 × 96, pied à x = 64 | 0,10 s | non |

Une créature **sans sort n'a pas de `cast.png`**, et son entrée au manifeste ne le déclare pas.

## `manifest.json`

- `animations` : les six animations possibles ;
- `templates` : les gabarits, `medium` et `large` — `frame` (la cellule de base), `wideFrame` (la
  cellule d'`attack` et de `cast`), `deathWide` (si la mort prend la cellule large) ;
- `monsters` : une entrée par créature livrée — `slug`, `template`, `creature` (l'identifiant du
  catalogue `Rpg/creatures/`, ou `null` tant que son bloc n'est pas extrait, et alors `awaiting`, le
  lot qui l'apportera) et `animations` (celles qu'elle livre).

`scripts/check_asset_keys.py` vérifie tout cela en CI ; la galerie des assets
(`--screen=AssetGallery`) montre chaque créature, animation par animation. Le jeu ne les affiche pas
encore : c'est le `LOT-27`.
