# .design-mockups/

Sources des **maquettes des écrans du jeu** (`LOT-68`, partie B, `EX-IHM-070`). Ce dossier ne
participe ni à la compilation ni au déploiement : c'est de la matière de conception, versionnée pour
que les décisions d'habillage restent traçables au même titre que les exigences.

## Ce qui est livré

`ecrans-jeu-parchemin.html` — page autonome présentant les neuf planches. Généré, jamais édité à la
main :

```bash
python .design-mockups/build_page.py
```

## Structure

| Fichier | Rôle |
|---|---|
| `_page_head.html` | En-tête de la page **et** habillage des planches (portée identité). C'est ce bloc CSS qui devient la section identité de `Source/Elements/Themes/theme.qss`. |
| `_page_tail.html` | Section « Où on en est » : décisions prises et points restés ouverts. |
| `_body_*.frag` | Une planche par fichier — sept écrans, les trois directions comparées, le système de design. |
| `_caret.frag` | Le curseur de focus, en SVG à pixels entiers. Substitué à `@CARET@` par le générateur. |
| `build_page.py` | Assemble la page : légendes, pastilles, mise à l'échelle des planches. |
| `_assemble.sh`, `*.dc.html` | Variante « canevas de design » (un artboard par écran). |

## Pourquoi une page et non un canevas éditable

Le canevas de design retouchable visuellement était le format retenu, mais son assemblage exige
**Node ou Bun**, absents du poste. Les `*.dc.html` sont conservés : si l'un des deux est installé un
jour, le canevas se monte à partir d'eux sans rien redessiner.

## Conventions tenues par les planches

- Les libellés sont ceux de `Source/Elements/Localization/fr.lang`, **mot pour mot** — aucun texte
  inventé, sans quoi la maquette promettrait ce que le jeu ne dit pas.
- La **palette** des planches et celle de `hmi::identityTokens()` sont vérifiées identiques à
  chaque Pull Request par `scripts/check_design_tokens.py` : une maquette qui ne décrit plus le
  jeu ne sert plus à décider quoi que ce soit.
- Les planches sont dessinées à 1280 × 720, la taille de la fenêtre de jeu, et remises à l'échelle
  par la page : les proportions sont donc exactes, pas approchées.
- Les grandeurs y sont celles du facteur **×2** (`hmi::pixelArtScale` pour une fenêtre 720p).

## État

Direction **A — Ambre nuit** retenue. Les directions B (cyan cathodique) et C (néon arcade) restent
sur la planche 02, en retrait : garder trace de ce qui a été écarté, et de ce que ça coûtait, évite
de refaire le débat dans six mois.
