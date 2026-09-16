# Les PNJ de l'atelier (LOT-91)

Un dossier par PNJ, `<slug>/`, avec ses six bandes d'animation, leurs `.anim.json` et `portrait.png`
(son portrait pixel art, 1 024 × 1 024), et un `manifest.json` à la racine. Les dossiers sont écrits par
`Documentation/Lot/LOT-91-atelier-pnj/atelier/scripts/integre.py`, jamais à la main ; les références
de chaque PNJ (bloc de prompt, palette, portrait) restent dans `atelier/pnj/<slug>/`.

Les cinq PNJ du pilote de la preuve de concept
(`Documentation/Lot/LOT-91-atelier-pnj/poc.md`) : **Anariel**, la planche de référence (tours 4 et 6),
puis **Lizz**, **Xorius**, **Nakral** et **Jade**, produits par la méthode standard (§4bis). Toutes
les bandes sont la sortie de `compose.py`, pas une retouche à la main.

| Bande | Images | Cellule | Ancre au pied | Durée / image | Boucle |
|---|---|---|---|---|---|
| `idle.png` | 6 | 48 × 64 | centre bas | 0,15 s | oui |
| `walk.png` | 8 | 48 × 64 | centre bas | 0,10 s | oui |
| `hit.png` | 4 | 48 × 64 | centre bas | 0,08 s | non |
| `death.png` | 6 | 96 × 64 | centre bas | 0,12 s | non, tenue |
| `attack.png` | 8 | 96 × 64 | x = 32 | 0,08 s | non |
| `cast.png` | 8 | 96 × 64 | x = 32 | 0,10 s | non |

Le personnage fait 45 pixels d'art de haut, comme les héros du Colisée. Chaque `.anim.json`
déclare sa propre `frameWidth` : c'est elle que le rendu lit (`hmi::ArenaTexture::frameWidth`),
pas une constante — une bande large se dessine deux fois plus large, centrée au même pied.

## `manifest.json`

- `npcs` : les slugs présents ;
- `replaces` : héros du Colisée → slug. Le rendu de l'arène (`hmi::ArenaSceneRenderer`) lit ce
  fichier à côté de `Coliseum/` et fait lire les bandes du héros nommé dans `Npc/<slug>/`
  (`hmi::ArenaAppearanceCatalog::applyNpcManifest`). Le héros garde son nom et sa place au
  roster ; seul le dossier lu change. Aujourd'hui, pour juger les PNJ en jeu (étape J) :
  `kaelith_voss` → `anariel`, `elira` → `jade`, `darin` → `xorius`, `bram` → `nakral` ; Lizz n'a
  pas de héros à remplacer.

Le dossier `Coliseum/` n'est pas touché : il reste engendré par `scripts/extract_coliseum_atlas.py`.
Le rendu n'affiche pour l'instant que `idle.png` debout et `death.png` à terre ; `walk`, `hit`,
`attack` et `cast` sont chargées (clips) mais pas encore jouées par la scène.
