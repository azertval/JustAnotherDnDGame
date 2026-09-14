# Le Colisée — pièces de la scène isométrique (LOT-50)

Ce dossier est **engendré** : `scripts/extract_coliseum_atlas.py` découpe la planche de production
`production_source_atlas.png` (1536 × 1024, la seule source) et écrit tout le reste. On ne retouche
pas une pièce à la main ; on corrige la planche ou le script, puis on relance :

```
python scripts/extract_coliseum_atlas.py            # decoupe et ecrit
python scripts/extract_coliseum_atlas.py --check    # verifie que le dossier suit la planche
```

| Dossier | Contenu |
|---|---|
| `terrain/` | dix losanges de sol (sable, sable sanglant, pierre, pierre usée, marbre, herbe sèche, terre, terre caillouteuse, eau, pont) |
| `coliseum/` | seize dalles du tileset du Colisée |
| `characters/<héros>/` | bandes d'animation `idle`, `walk`, `attack`, `hit`, `death` : cinq images de 48 × 64, ancre au pied centre bas |
| `enemies/<gladiateur>/` | bande `idle` de huit images, même canevas |
| `structures/`, `details/` | enceinte et décor nommés : murs, colonnes, portes, arche, bannières, torches, statues, gradins, cages, loges… |
| `props/`, `rocks/`, `vegetation/`, `effects/` | pièces numérotées, non encore consommées |
| `manifest.json` | chaque fichier, sa taille, sa boîte sur la planche, l'empreinte de la planche |

Consommateur : `Source/Ui/Controls/ArenaScene.ui.qml` et `ArenaTile.ui.qml`. Les pièces sont
embarquées en ressource par `Source/Ui/CMakeLists.txt` (motif `Coliseum/*/*.png`) ; la planche
elle-même ne l'est pas.

Convention de projection : un losange de largeur L a une hauteur de 0,62 L (l'angle des tuiles de la
planche, pas le 2:1 classique) ; une figurine se pose au centre du losange, à l'échelle L / 86.
