# Crédits graphiques

La plupart des images de `Source/Elements/Assets/` (`atlas.png`, les skins du jeu `test`, les
planches de test des dossiers `Objects/`/`Backgrounds/`/`Player/`) sont **générées par script**
(schématiques, sans dépendance externe) : voir `Skins/README.md`, `Objects/README.md`,
`Player/README.md`.

Le jeu de skins `kenney` (`skins.json`), un fond (`Backgrounds/kenney_grass.png`), quatre motifs
de plan (`scripts/motifs/kenney_*.png`) et deux objets (`Objects/kenney_*.png`) font exception :
issus de packs **Kenney** (www.kenney.nl), publiés sous licence **CC0 1.0 Universal** (domaine
public) : <http://creativecommons.org/publicdomain/zero/1.0/>, retouchés pour respecter le contrat
de dimensions du projet quand il en impose un (`Skins/README.md`, `Objects/README.md` : grille de
16×16) — les motifs et le fond, à dimensions libres, ne sont recadrés que pour retirer la marge
transparente, sans être forcés à un carré.

> **`LOT-69`** — le dossier `Decors/` a disparu avec le système de décors-sprites. Les quatre
> images Kenney qu'il contenait sont **conservées** sous `scripts/motifs/`, où elles alimentent
> `scripts/generate_demo_plans.py` : ce ne sont plus des assets chargés par le jeu, mais les
> **sources** des plans picturaux livrés. Les supprimer aurait rendu ce générateur injouable, donc
> non reproductible.

Le CC0 n'exige aucune attribution, mais ce dépôt s'engage à créditer les auteurs malgré tout, même
principe que `Source/Elements/Audio/CREDITS.md` (`LOT-60`).

**Élargissement du second temps du `LOT-65`** — huit motifs (`mushroom`, `crystal`, `stalactite`,
`vine`, `grass_tuft`, `lantern`, `pillar`, `gear` ; décors à l'époque, code de dessin de
`generate_demo_plans.py` depuis le `LOT-69`), trois fonds (`test_forest`, `test_sunset`,
`test_industrial`), quatre objets (`crate_green`, `stone_block`, `metal_plate`, `ice_block`) et la
refonte du personnage sont **entièrement générés par script**, sans source externe : rien à créditer
au-delà du dépôt lui-même. La génération procédurale a été préférée à l'import d'assets libres
supplémentaires pour deux raisons — elle reste reproductible (`python scripts/generate_test_*.py`
régénère l'intégralité à l'identique) et elle n'ajoute aucune dépendance de licence à suivre.

| Fichier livré | Retouche | Sprite d'origine | Pack Kenney | Auteur | Licence |
|---|---|---|---|---|---|
| `Skins/key.png` (image 0/2) | recadrée, réduite à 16×16 | `keyYellow.png` | Platformer Pack Remastered | Kenney | CC0 1.0 |
| `Skins/locked_door.png` (image 0/2) | recadrée, réduite à 16×16 | `lockYellow.png` | Platformer Pack Remastered | Kenney | CC0 1.0 |
| `Skins/kenney_spikes.png` | recadrée, réduite à 16×16 | `spikes.png` | Platformer Pack Remastered | Kenney | CC0 1.0 |
| `Skins/kenney_switch.png` (image 0/2) | recadrée, réduite à 16×16 | `switchYellow.png` | Platformer Pack Remastered | Kenney | CC0 1.0 |
| `Skins/kenney_switch.png` (image 1/2) | recadrée, réduite à 16×16 | `switchYellow_pressed.png` | Platformer Pack Remastered | Kenney | CC0 1.0 |
| `Skins/kenney_door.png` (image 0/2) | recadrée, réduite à 16×16 | `doorClosed_mid.png` | Platformer Pack Remastered | Kenney | CC0 1.0 |
| `Skins/kenney_door.png` (image 1/2) | recadrée, réduite à 16×16 | `doorOpen_mid.png` | Platformer Pack Remastered | Kenney | CC0 1.0 |
| `Backgrounds/kenney_grass.png` | aucune (dimensions libres) | `colored_grass.png` | Platformer Pack Remastered | Kenney | CC0 1.0 |
| `scripts/motifs/kenney_fence.png` | détourée, réduite à largeur 32 (aspect conservé) | `fence.png` | Platformer Pack Remastered | Kenney | CC0 1.0 |
| `scripts/motifs/kenney_torch.png` | détourée, réduite à hauteur 32 (aspect conservé) | `torch1.png` | Platformer Pack Remastered | Kenney | CC0 1.0 |
| `scripts/motifs/kenney_chain.png` | détourée, réduite à hauteur 32 (aspect conservé) | `chain.png` | Platformer Pack Remastered | Kenney | CC0 1.0 |
| `scripts/motifs/kenney_ladder.png` | détourée, réduite à 16×16 | `ladderMid.png` | Platformer Pack Remastered | Kenney | CC0 1.0 |
| `Objects/kenney_coin.png` | détourée, réduite à 16×16 | `coinGold.png` | Platformer Pack Remastered | Kenney | CC0 1.0 |
| `Objects/kenney_gem.png` | détourée, réduite à 16×16 | `gemBlue.png` | Platformer Pack Remastered | Kenney | CC0 1.0 |

Retouche appliquée à chaque sprite de `Skins/` : détourage à la zone non transparente, mise à
l'échelle (`LANCZOS`) vers un carré de 16×16 pixels (ou deux cases de 16×16 côte à côte pour les
assets à deux états — `key.png`/`locked_door.png` (`LOT-63`), `kenney_switch.png`/`kenney_door.png`
(`LOT-65`) —, la seconde image de `key.png`/`locked_door.png` étant une case entièrement
transparente ajoutée par le `LOT-63` pour représenter l'état « ramassée »/« ouverte », absente du
pack d'origine ; `kenney_switch.png`/`kenney_door.png` utilisent chacun deux sprites distincts déjà
présents dans le pack, un par état), sans autre retouche de couleur ou de forme.

Retouche appliquée à chaque décor/objet Kenney de `Decors/`/`Objects/` : détourage à la zone non
transparente, puis mise à l'échelle (`LANCZOS`) — au carré 16×16 imposé pour `Objects/`
(`EX-REN-007`), ou en conservant l'aspect d'origine pour `Decors/` (dimensions libres), calée sur
la largeur ou la hauteur selon l'orientation du sprite. `kenney_grass.png` n'est pas retouché
(dimensions déjà libres, aucune marge transparente à retirer).

Sources des packs :
- Platformer Pack Remastered — <https://kenney.nl/assets/platformer-pack-remastered>
- Platformer Pack: Industrial — <https://kenney.nl/assets/platformer-pack-industrial>
