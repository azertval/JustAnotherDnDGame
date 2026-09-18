# LOT-96 — Martpart et Arenarea se parcourent : le graphe des quartiers {#lot-96}

> Statut : **ouvert le 18 septembre 2026**, sur la branche `lot-96-quartiers-capitale`.
> Prérequis : [LOT-09](@ref lot-09) (l'exploration dans le jeu, le graphe de cartes et les
> portails, prouvés au Colisée), [LOT-94](@ref lot-94) (le plan de la Capitale peint par l'auteur,
> source du tracé, ses douze quartiers placés, et l'écran « Carte » qui le montre),
> [LOT-11](@ref lot-11) (l'éditeur, où les quartiers se retouchent), [LOT-15](@ref lot-15) (le
> dialogue de la sentinelle).
> Alimente : [LOT-16](@ref lot-16), [LOT-17](@ref lot-17), [LOT-27](@ref lot-27).
> Exigences couvertes : `EX-IHM-076`, `EX-NFR-040`.

## Objectif

« Nouvelle partie » pose le personnage à une porte de **Martpart**, le quartier du marché ; on y
marche ; on passe à **Arenarea** par l'avenue et on revient sur ses pas, au point d'arrivée nommé ;
on parle à une sentinelle Ironhand qui garde un quartier fermé. L'écran « Carte » montre le
quartier où l'on est et ceux qu'on a visités, et descend de la ville au quartier, puis à l'îlot.
Ce lot livre une ville qu'on **parcourt**, pas une ville qu'on regarde : l'habillage complet est au
[LOT-27](@ref lot-27).

## Ce que le dépôt contenait à l'ouverture (18 septembre 2026)

- **Le moteur est prêt.** `core::ExplorationSession`, `core::WorldTravel` et sa validation
  (`validateWorldGraph`, `validateWorldMap`), `hmi::WorldModel` en singleton, le composeur et le
  rendu de lieu (`WorldSceneComposer`, `WorldSceneRenderer`) : tout est livré au
  [LOT-09](@ref lot-09) et éprouvé sur cinq cartes de fixture. Ce lot n'y ajoute que des cartes.
- **« Nouvelle partie » ouvre le Colisée**, en dur : `WorldModel::START_MAP = "coliseum"`
  (`Source/HMI/Runtime/WorldModel.h`).
- **`Source/Elements/Levels/` ne porte que `coliseum.json`.** Il n'y a pas de
  `Source/Elements/World/capital.json`.
- **Les douze quartiers sont déjà placés sur le plan** : `world-maps.json`, section
  `cities.central-empire-the-capital-city.places`, en fractions du plan peint par l'auteur
  (`city-central-empire-the-capital-city.jpg`, 1920 × 1080). Chacun a sa fiche d'atlas dans
  `Source/Elements/World/locations/`.
- **L'écran « Carte » s'arrête à la ville** (`CityMap.qml`, `WorldMapModel::city`) : aucun niveau
  quartier ni îlot, aucune position du héros.
- **La planche de Martpart est installée** (`Source/Elements/Assets/Scene/martpart/`,
  [LOT-92](@ref lot-92)) : rues, place, ruelle, façades, portes, fenêtres, murs, lumière, quatre
  objets. **Arenarea n'a pas de planche.**
- **Aucune figurine de sentinelle** : l'atelier du [LOT-91](@ref lot-91) a livré cinq figurines
  (`anariel`, `jade`, `lizz`, `nakral`, `xorius`), aucune n'est un garde Ironhand.
- **Cinq dialogues** dans `Source/Elements/World/dialogues/`, tous du Colisée : ils servent de
  modèle à `sentinelle-ironhand.json`.

## Décisions d'ouverture

- **Le niveau îlot est un rendu de la carte de niveau** (*décision de l'auteur*, 18 septembre
  2026). L'îlot n'a pas d'image à lui : l'écran « Carte » montre la carte du quartier telle que le
  jeu la dessine, vue de haut, par le composeur de lieu — les mêmes planches, le même marqueur, le
  héros à sa case. Rien à peindre ni à commander, et un plan qui ne peut pas diverger du terrain
  qu'on parcourt. Un îlot est un rectangle de cases nommé sur la carte du quartier.
- **Le niveau quartier est un recadrage du plan de ville, provisoire** (*décision de l'auteur*,
  18 septembre 2026). C'est une entorse nommée à la règle du [LOT-94](@ref lot-94) — « une carte
  repeinte n'est pas une carte recadrée » — tolérée jusqu'à ce que l'auteur peigne la vue de
  chaque quartier. Pour qu'elle ne s'installe pas : **aucun fichier recadré n'est commité**. Le
  quartier porte un **cadre** sur le plan de ville, en fractions, comme le `frame` d'une région
  sur le monde, et l'écran agrandit ce cadre ; le jour où une image peinte existe, un champ
  `image` la désigne et le cadre ne sert plus qu'à l'origine du zoom. `check_map_assets.py`
  signale chaque quartier encore sans image.
- **Les cartes se posent par script, puis se retouchent dans l'éditeur** — la méthode du
  [LOT-09](@ref lot-09), reconduite : un script d'atelier versionné dans ce dossier écrit
  `martpart.json` et `arenarea.json` depuis une description de haut niveau (rues, îlots, bâtiments
  nommés, portes), relevée sur le plan de ville ; l'éditeur du [LOT-11](@ref lot-11) reste l'outil
  de retouche.

## Les phases

L'ordre a changé à la première phase (voir le journal) : les cartes d'abord, le graphe qui les
nomme ensuite, pour que chaque commit reste vert.

| # | Phase | Ce qu'elle livre |
|---|---|---|
| 1 | **Martpart** | `Source/Elements/Levels/capital/martpart.json`, tracée par le script d'atelier sur la planche du [LOT-92](@ref lot-92) et sa table d'apparence ; le graphe du monde et le navigateur de l'éditeur lisent les sous-dossiers ; `--map=<carte>[@<arrivée>]` pour ouvrir une carte en développement |
| 2 | **Arenarea, et l'avenue** | `Source/Elements/Levels/capital/arenarea.json` ; les portails de l'avenue, aller et retour, par points d'arrivée nommés ; l'aller-retour Martpart → Arenarea → Martpart en headless, à la bonne case, l'état de la carte quittée conservé |
| 3 | **Le graphe de la Capitale** | `Source/Elements/World/cities/capital.json` : les douze quartiers, leur fiche d'atlas, et pour chacun **soit** sa carte, **soit** sa sentinelle ; la porte de départ, que « Nouvelle partie » lit ; `check_rpg_data.py` lie chaque quartier à sa carte ou à sa sentinelle, et à son point de `world-maps.json` |
| 4 | **Les dix portes gardées** | Une entité `npc` sentinelle Ironhand à la porte de chaque quartier sans carte, sur le bord qui lui fait face ; `sentinelle-ironhand.json`, un dialogue de refus ; une figurine absente se dessine par le marqueur du [LOT-39](@ref lot-39) |
| 5 | **Le plan descend** | Le héros et les quartiers visités sur le plan de ville ; le niveau quartier (cadre agrandi du plan) ; le niveau îlot (rendu de la carte) |
| 6 | **Captures et vérification** | Captures de référence QML de Martpart, d'Arenarea et de chaque niveau ajouté au plan ; le geste au clavier et à la manette |

## Points ouverts

- **La planche d'Arenarea.** La feuille de route autorise le marqueur là où l'atelier n'a rien
  livré. Si l'auteur commande une planche Arenarea à l'atelier du [LOT-92](@ref lot-92) (envois
  manuels), la phase 3 la pose ; sinon, le marqueur tient jusqu'au [LOT-27](@ref lot-27).
- **La figurine de la sentinelle.** Même règle : marqueur du [LOT-39](@ref lot-39) jusqu'à ce que
  l'atelier du [LOT-91](@ref lot-91) produise un garde Ironhand. Reprendre une des cinq figurines
  livrées ferait d'un personnage nommé un garde anonyme — écarté.
- **Le Colisée du menu principal** reste tel que le [LOT-09](@ref lot-09) l'a livré ; le
  brancher depuis Arenarea est au [LOT-27](@ref lot-27).

## Critères d'acceptation

- **À l'écran** : « Nouvelle partie » ouvre Martpart à sa porte ; on passe à Arenarea et on revient
  **au point d'arrivée nommé** ; on parle à la sentinelle d'une porte fermée et l'écran de dialogue
  s'ouvre sur son dialogue ; le plan montre le héros dans le bon quartier.
- **Headless** : Martpart → Arenarea → Martpart ramène à la bonne case, et l'état de la carte
  quittée est conservé dans la session.
- `check_rpg_data.py` lie chaque quartier de `capital.json` à sa carte ou à sa sentinelle, et à son
  quartier de `world-maps.json`.
- L'écran « Carte » descend de la ville à un quartier puis à un îlot ; captures de référence QML de
  Martpart, d'Arenarea et de chaque niveau ajouté.
- Aucune tuile de test ; le marqueur du [LOT-39](@ref lot-39) est le seul substitut ; aucune image
  du corpus, ni aucun recadrage commité (`EX-IHM-076`).

## Journal

- **18 septembre 2026, ouverture.** Les trois décisions d'ouverture ci-dessus. Le lot quitte la
  section 11 de la [feuille de route](@ref roadmap) pour cette page ; les deux tableaux de la
  section 6 sont régénérés (`lint_lots.py --regenerer`) ; le diagramme perd les arêtes entrantes
  du lot, et son nœud est marqué « en cours ». Au passage, le nœud du [LOT-09](@ref lot-09),
  livré le 17, prend sa marque « livré », oubliée à la livraison.
- **18 septembre 2026, phase 1 — Martpart.** Trois constats en traçant, et ce qu'ils ont décidé :
  - *L'ordre des phases change.* Le graphe (`capital.json`) nomme les cartes de chaque quartier ;
    le poser d'abord aurait fait échouer son contrôle jusqu'à ce qu'elles existent. Les cartes
    viennent donc d'abord.
  - *Les portes se posent d'après le plan.* Le script lit les points des quartiers dans
    `world-maps.json` et pose chaque porte sur le bord que coupe la direction du voisin : on sort
    de Martpart par l'ouest-nord-ouest vers Arenarea, comme sur le plan. Le reste — rues, place,
    îlots, ruelles — n'est écrit nulle part dans le livre, et le script le décide.
  - *Le cœur des îlots reste noir.* La planche de Martpart n'a pas de toit. Des murs pleins dans
    le cœur des îlots dessinaient un treillis de clôtures, un pavé de place les faisait lire comme
    des places ouvertes derrière des murets ; sans sol ni pièce, l'îlot se lit comme une masse
    bâtie, comme le dehors du Colisée. Les toits sont un habillage, au [LOT-27](@ref lot-27).
  - *Les cartes d'un sous-dossier ont pour identifiant leur chemin relatif* (`capital/martpart`) :
    `core::loadWorldGraph` et le navigateur de l'éditeur lisaient le seul premier niveau du
    dossier, si bien qu'un portail vers un quartier aurait été déclaré orphelin.

