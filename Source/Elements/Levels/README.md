# Elements/Levels/

Cartes du jeu, un fichier **JSON** par carte (`EX-LVL-001`, `EX-LVL-003`).

> **Peuplé au `LOT-09`.** La première carte est `coliseum.json`, **le Colisée en version finale** : 40 × 34 cases, le sable du
> `LOT-50` (20 × 14) au centre comme **zone de combat déclarée** (`combatZone`, `EX-LVL-018`),
> l'enceinte et ses quatre portes, deux couloirs sous les gradins, deux vestiaires, les tribunes du
> nord et du sud, la loge impériale, le grand escalier, le hall et la porte. Elle est posée par un
> script d'atelier (`Documentation/Lot/LOT-09-colisee-premiere-carte/atelier/carte_colisee.py`) puis
> retouchable dans l'éditeur ; `arena-of-the-future.json`, la piste nue du `LOT-50`, est partie avec
> elle. Le contenu du *vertical slice* arrive avec le `LOT-27`.

> **`capital/`, au `LOT-96`** : les quartiers de la Capitale qui ont leur carte — `martpart.json`
> (le quartier du marché, 48 × 40) et `arenarea.json` (le quartier des arènes, 48 × 40, sur la
> planche de Martpart faute de planche propre), reliés par l'avenue ; « Nouvelle partie » ouvre
> le jeu à la porte de l'Est de Martpart, que nomme `World/cities/capital.json`. Même méthode que le Colisée : un script d'atelier les pose
> (`Documentation/Lot/LOT-96-quartiers-capitale/atelier/carte_quartiers.py`, `--check` pour savoir
> si une carte a été retouchée), l'éditeur les retouche. Une carte d'un sous-dossier a pour
> identifiant son **chemin relatif** : un portail vise `capital/martpart`, et le graphe du monde
> comme le navigateur de l'éditeur lisent les sous-dossiers.

- Une carte est un objet JSON : `name`, `width`, `height`, et une liste **`tiles`** d'objets
  `{ "x", "y", "type", … }` — la grille de **collision**. Les cases **vides** ne sont pas listées
  (absence = vide). Une tuile peut porter un champ `"texture"` (nom de pièce de la planche du lieu,
  `EX-EDIT-043`) : pièce assignée **par case**, prioritaire sur la table d'apparence du lieu,
  indépendante du type lui-même.
- Types de tuiles : `empty`, `solid`, `entry`, et le terrain du RPG (`LOT-08`) : `grass`, `dirt`,
  `sand`, `water`, `deepWater`, `wall`, `cliff`, `bridge`, `stairs`. Une carte porte **exactement
  une** case `entry`, le point d'arrivée par défaut du héros.
- `"layers"` (optionnel, `LOT-04`) : couches visibles `{ "name", "kind", "tiles" }` (`kind` :
  `ground`, `decor`) ; `"entities"` (optionnel) : entités placées `{ "type", "x", "y", … }` —
  PNJ, portails, zones de combat —, leurs autres champs étant des propriétés libres
  (`EX-LVL-017`, `EX-LVL-018`).
- Le format est **versionné** (`"version"`, cf. `core::LEVEL_FORMAT_VERSION`) : un fichier sans le
  champ est lu comme la version initiale, et le chargeur refuse proprement une version qu'il ne
  connaît pas.
