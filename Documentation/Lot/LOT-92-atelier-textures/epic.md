# LOT-92 — Atelier des textures : le style de la scène par maquette, une planche par lieu {#lot-92}

> Statut : **en cours, ouvert le 16 septembre 2026.** T0 à T3 faits ; T4 attend la génération de la planche du Colisée.
> Prérequis : [LOT-50](@ref lot-50) (la planche du Colisée et `extract_coliseum_atlas.py`),
> [LOT-91](@ref lot-91) (la méthode de l'atelier des PNJ), [LOT-39](@ref lot-39) (les clés
> d'assets et le cahier), [LOT-37](@ref lot-37) (l'atlas, dont le bloc B lit les lieux).
> Alimente : [LOT-09](@ref lot-09), `LOT-93`, `LOT-95`, `LOT-42`.
>
> Tant que le lot n'est pas livré, ce dossier fait échouer le contrôle 6 de `lint_lots.py` (un
> dossier vaut livraison) : la section de la feuille de route le quitte dans la PR de livraison.

## Objectif

Les figurines ont une méthode (`LOT-91`), pas les textures. Ce lot donne à la scène un **style
écrit**, fixé par une maquette approuvée par l'auteur, puis une méthode pour commander une planche
de textures **par lieu**, depuis le descriptif du lieu dans l'atlas. Première planche : le Colisée
final du `LOT-09`, qui remplace celle du `LOT-50`.

## Les décisions

| Sujet | Décision |
|---|---|
| **Point de vue** | **Isométrique, losange au rapport 0,62** : celui d'`core::IsoProjection` et du composeur du `LOT-86`, que le `WorldViewportItem` du `LOT-09` reprend. La maquette fixe le trait, la palette, la lumière et la taille de tuile, **pas** la géométrie. *Décision de l'auteur, 16 septembre 2026.* |
| **Échelle** | Tuile de **68 × 42 pixels d'art** et personnage de **45 px** : au facteur `ARENA_FIGURE_SCALE` (1,25), la tuile tombe à 85 × 52,5 px, la tuile de 86 px de la planche du `LOT-50`. Figurine et sol sont donc au **même pas de pixel** — ce que la planche du `LOT-50` n'était pas. Proposition à confirmer par la maquette (voir *Risque*). |
| **Murs** | Un mur monte de `ARENA_WALL_RISE` (0,85) largeurs de losange au-dessus de sa case : 58 px d'art. |
| **Lieux de la Capitale** | Ils entrent dans l'atlas. Le *Tanares Sourcebook* décrit la Capitale et ses **douze quartiers** dans un encart de la région Central Empire (PDF p. 50, imprimées 98-99), hors « Places of Interest » : `atlas.py` ne l'a pas lu. Ce lot l'étend. *Décision de l'auteur, 16 septembre 2026.* |
| **Générateur** | Celui du `LOT-91` : génération d'images d'OpenAI, envois **à la main**, retours dans un dossier de travail hors dépôt (`TEXTURE_ATELIER`, par défaut `D:\JustAnotherDnDGame-textures`). Claude rédige, découpe, mesure et intègre ; il ne dessine pas. |
| **Identités** | La scène est en pixel art ; l'interface reste à la charte v2 (`LOT-87`). La spécification le dit (T5). |

### Risque : l'angle 0,62 en pixel art

Le pixel art isométrique trace ses arêtes en marches régulières de 2 pour 1 ; un rapport de 0,62
donne des marches de 21 pour 34, irrégulières, qui se voient sur un sol uni. La maquette dit si
cela se lit ; si non, deux sorties, à trancher avec elle sous les yeux : un losange 68 × 34
(2 : 1, `ARENA_DIAMOND_RATIO` à 0,5 et le cadrage à revoir) ou des sols texturés qui masquent
l'arête (pavés, sable).

## Les tâches

| Tâche | Contenu | Acceptation |
|---|---|---|
| **T0 — La Capitale dans l'atlas** | `scripts/sourcebook/atlas.py` lit l'encart « The Capital City » : la Capitale et ses douze quartiers (Sloghood, Uptown, Artisansquare, Scholarnest, Dweomer, **Martpart**, **Arenarea**, Oldtown, Neckoffoods, **Bloomburgs**, Downtown, Palacedomain) deviennent des lieux de `central-empire`, au schéma `location.schema.json` inchangé. `LIEUX_ATTENDUS` et `test_atlas.cpp` suivent. | `sourcebook atlas` régénère sans diff hors ces treize fiches ; `test_atlas` vert. |
| **T1 — La maquette de style** | `atelier/prompts/maquette.txt` et `atelier/scripts/maquette.py` (l'envoi : prompt + référence d'échelle composée des figurines d'Anariel et de Jade sur la grille de losanges). Tours jusqu'à approbation ; la maquette retenue va dans `atelier/ancres/maquette.png`. | **L'auteur approuve** ; le risque de l'angle est tranché. |
| **T2 — Le bloc A** | `atelier/prompts/style.txt`, écrit depuis la maquette approuvée : pas de pixel, trait, palette du sol et de la pierre, lumière, tuile. | Relu par l'auteur. |
| **T3 — La disposition et la découpe** | `atelier/dispositions/<id>.json` (cellules nommées : sols, pièces hautes, grandes pièces ; emprise en cases, hauteur) ; `scripts/extract_texture_sheet.py` en déduit la grille, le gabarit, le bloc C, la découpe et la clé `scene/<lieu>/<nom>` de chaque texture ; `check_assets_brief.py` valide les dispositions. | `extract_texture_sheet.py --check` reproduit le dossier ; `check_assets_brief.py` vert. |
| **T4 — La planche du Colisée** | Bloc B depuis la fiche du Colisée (l'Illu Die Arena de Martpart), disposition « Colisée » : sable, gradins, loges, couloirs, vestiaires, portes, torches, bannières. Remplace la planche du `LOT-50` au rendu de l'arène. | Découpée par le script ; l'arène se dessine avec. |
| **T5 — Martpart et la spécification** | La planche de Martpart se **commande** (envoi prêt) depuis sa seule fiche d'atlas, bloc A intouché ; `Documentation/Specification/` dit les deux identités, scène et interface. | Envoi préparé sans rédaction à la main ; spécification relue. |

## Journal

- **16 septembre 2026** — ouverture. Point de vue isométrique 0,62 gardé ; la Capitale entre dans
  l'atlas. Rédaction du prompt de maquette (T1).
- **16 septembre 2026, maquette tour 1** (prompt v1). *Tenu* : la grille — les dalles suivent
  exactement les losanges 136 × 84 de la référence, et le rapport 0,62 ne se voit pas en escalier
  sur un sol pavé ; l'ambiance (soir, lanternes, auvents rayés, bannières). *Écarts mesurés* :
  pas de pixel perdu (2 % de voisins identiques par paire, 252 548 couleurs, halos doux) ;
  façades et étals de face, pas le long des arêtes du losange ; porte de ~195 px pour une
  figurine de 90 (2,2 fois) ; figurines redessinées, pas recopiées. Prompt v2 (v1 gardé en
  `maquette_v1.txt`) : blocs 2 × 2 et 64 couleurs, faces le long des diagonales, tailles
  d'objets chiffrées, joints de dalle admis.
- **16 septembre 2026, maquette tour 2** (prompt v2). *Gagné* : un pixel art franc, trait et
  aplats lisibles, palette resserrée à l'œil ; les étals du fond suivent les diagonales. *Perdu* :
  la grille — dalles de ~62 × 35 px (période mesurée), non calées sur la référence ; figurines
  déplacées et agrandies (~105 px). *Toujours faux* : la façade gauche de face, la porte (~225 px).
  Ni l'un ni l'autre tour n'est sur un pas de 2 px (aucune phase dominante de la différence
  horizontale) : le générateur rééchantillonne, la mise au pas se fera à la réception, comme
  `normalise.py` au `LOT-91`. *Constat* : le générateur tient le style **ou** la métrique, pas
  les deux dans une scène.
- **16 septembre 2026, maquette approuvée : le tour 1.** *Décision de l'auteur*, pour son
  ambiance, plus riche et plus chaude que le tour 2. Copiée en `atelier/ancres/maquette.png`.
  Le risque de l'angle 0,62 est levé pour les sols texturés. Les écarts du tour 1 ne se corrigent
  pas dans la maquette : le pas de pixel se remet à la réception (quantification, comme au
  `LOT-91`) ; les faces le long des diagonales et la taille des objets sont des règles du bloc A,
  vérifiées planche par planche, où la cellule de la disposition impose le cadre. Bloc A v1
  écrit (`atelier/prompts/style.txt`), palettes de matière relevées sur la maquette (T2).
- **16 septembre 2026, T0 fait.** `atlas.py` lit l'encart de la Capitale (PDF index 49, déclaré
  dans `CAPITALE` : la typographie ne le distingue pas d'une ouverture de région) : treize lieux
  de `central-empire`, `central-empire-the-capital-city` et ses douze quartiers
  `central-empire-the-capital-city-<quartier>`. Quartiers repérés par leur fragment gras
  « N- Nom. », numérotation contrôlée ; numéro de page imprimé et filigrane de commande écartés.
  Régénération sans autre diff que ces treize fiches et `regions/central-empire.json`
  (`LIEUX_ATTENDUS` 12 → 25) ; 94 → 107 lieux. Nouveau test
  `AtlasTest.LaCapitaleEtSesDouzeQuartiersSontRejouesDepuisLeLivre` (phrases de Martpart et
  d'Arenarea recopiées du PDF) ; `check_rpg_data.py` vert, 1 263 tests unitaires verts.
- **16 septembre 2026, filigrane retiré de l'atlas** (accord de l'auteur). Treize fiches du
  `LOT-37` portaient le numéro de page et le filigrane « Valentin Eloy (Order #…) », en fin ou
  en milieu de description. `sans_filigrane` les retire **à l'assemblage de la description** et
  non à la lecture : retirée plus tôt, la ligne ne sépare plus deux intertitres, et la légende
  « Fisherman's Wharf » du plan de la République fusionnait avec « Locations in Fisherman's
  Wharf ». Le numéro de page n'est retiré que collé au filigrane : un filtre sur toute ligne de
  chiffres effaçait les cellules de la table d6 des rencontres de morts-vivants. Diff audité mot
  à mot : les treize filigranes, rien d'autre. *Reste* : `republic-of-freelands-fisherman-s-wharf`
  est cette légende, pas un lieu ; sa description est désormais vide (le schéma l'admet).
- **16 septembre 2026, T3 fait.** *Décisions* :
  - **La disposition vient avant l'image.** `extract_coliseum_atlas.py` relevait des coordonnées
    sur une planche déjà faite ; ici un JSON déclare chaque cellule par sa classe (`floor`,
    `tall`, `wide`), son emprise en cases (`footprint`, `[1, 2]` pour une pièce allongée sur
    l'arête haut-gauche) et sa hauteur (`rise`, en pixels d'art). La grille (rangées remplies dans
    l'ordre, marge de 8 px), le gabarit envoyé au générateur, le bloc C, la découpe et le
    manifeste s'en déduisent : aucune coordonnée écrite à la main.
  - **Géométrie d'`IsoProjection`** : une emprise a × b fait (a + b) · 34 × (a + b) · 21 px
    d'art ; l'ancre d'une texture est le sommet haut de son emprise, le coin (0, 0) de sa case.
  - **Le cahier des scènes est la disposition.** Le cahier du `LOT-87` est réservé à l'interface
    (clés `ui/…`, jetons, zones de maquette) ; une texture de scène n'y a ni jeton ni maquette
    d'écran. `check_assets_brief.py` fait donc valider les dispositions par
    `extract_texture_sheet.valider_tout` (sans Pillow, comme la CI) : clés `scene/…` au format du
    `LOT-39` et uniques, lieu présent dans l'atlas, grille qui tient dans la planche.
  - **Réception** : planche ramenée à 1536 × 1024, alpha binarisé à 127 (une planche sans
    transparence est refusée), 2 × 2 → 1 par moyenne des pixels opaques, **une palette de
    64 couleurs commune** à la planche, sols découpés au losange exact et refusés sous 97 % de
    couverture, débordement hors canevas signalé. Installés sous `installRoot` avec
    `planche.png` et `manifest.json` ; `--check` refait la découpe en mémoire et compare pixels et
    manifeste **sans rien écrire** (celui du `LOT-50` réécrivait ses PNG).
  - *Disposition du Colisée* (T3, alors sur une planche 1536 × 1024 au pas 2) : 36 cellules — 15 sols, 17 pièces hautes (murs, arches, gradins,
    escaliers, torches et bannières en deux orientations, pilier, brasero, banc, râtelier),
    4 grandes pièces (porte des combattants, loge) ; bloc B depuis la fiche de Martpart, sujet :
    l'Illu Die Arena. Tests : `scripts/tests/test_extract_texture_sheet.py` (13, dont la découpe
    d'une planche synthétique et son `--check`).
- **16 septembre 2026, T2 fait** : le bloc A (`style.txt` v1) est relu par l'auteur.
- **16 septembre 2026, planches plus grandes et au pas 4** (demande de l'auteur). Le 1536 × 1024
  de T3 venait d'une limite du générateur qui n'a plus cours : la documentation d'OpenAI
  (`gpt-image-2` et suivants) admet toute taille aux côtés multiples de 16, d'au plus 3840 px, au
  rapport d'au plus 3 : 1, entre 655 360 et 8 294 400 pixels, et donne le rendu pour
  expérimental au-delà de 2560 × 1440. *Décisions* : la disposition déclare `sheet.size` et
  `sheet.scale` (pixels d'écran par pixel d'art), contrôlés contre ces contraintes ; les cellules
  débordent sur **autant de planches que nécessaire** ; la palette de 64 couleurs reste
  **commune au lieu**. Le Colisée passe à **2560 × 1440 au pas 4** — la plus grande taille
  fiable, quatre fois plus de pixels par pixel d'art qu'au pas 2 : 2 planches (24 et 12
  cellules). La texture installée ne grandit pas (une tuile reste 68 × 42 px d'art, l'échelle des
  figurines) ; c'est la réduction qui gagne en netteté. Le bloc A porte désormais `{PAS}`,
  `{LOSANGE_L}` et `{LOSANGE_H}`, remplis depuis la disposition, sans autre changement ; le bloc C
  précise que la maquette est au pas 2 et la planche à son propre pas.
