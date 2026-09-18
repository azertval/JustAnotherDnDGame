# Elements/Levels/

Cartes du jeu, un fichier **JSON** par carte (`EX-LVL-001`, `EX-LVL-003`).

> **Vidé au `LOT-01`, rouvert au `LOT-50`, peuplé au `LOT-09`.** Les 26 tableaux de démonstration
> du jeu de plateforme dont ce dépôt est dérivé ont été retirés avec leur gameplay. La première
> carte du RPG est `coliseum.json`, **le Colisée en version finale** : 40 × 34 cases, le sable du
> `LOT-50` (20 × 14) au centre comme **zone de combat déclarée** (`combatZone`, `EX-LVL-018`),
> l'enceinte et ses quatre portes, deux couloirs sous les gradins, deux vestiaires, les tribunes du
> nord et du sud, la loge impériale, le grand escalier, le hall et la porte. Elle est posée par un
> script d'atelier (`Documentation/Lot/LOT-09-colisee-premiere-carte/atelier/carte_colisee.py`) puis
> retouchable dans l'éditeur ; `arena-of-the-future.json`, la piste nue du `LOT-50`, est partie avec
> elle. Le contenu du *vertical slice* arrive avec le `LOT-27`.

> **`capital/`, au `LOT-96`** : les quartiers de la Capitale qui ont leur carte — `martpart.json`
> (le quartier du marché, 48 × 40). Même méthode que le Colisée : un script d'atelier les pose
> (`Documentation/Lot/LOT-96-quartiers-capitale/atelier/carte_quartiers.py`, `--check` pour savoir
> si une carte a été retouchée), l'éditeur les retouche. Une carte d'un sous-dossier a pour
> identifiant son **chemin relatif** : un portail vise `capital/martpart`, et le graphe du monde
> comme le navigateur de l'éditeur lisent les sous-dossiers.

- Une carte est un objet JSON : `name`, `width`, `height`, et une liste **`tiles`** d'objets
  `{ "x", "y", "type", … }`. Les cases **vides** ne sont pas listées (absence = vide). Une tuile
  peut porter un champ `"texture"` (nom d'asset, `EX-EDIT-043`) : texture assignée **par instance**,
  prioritaire sur le skin de son type, indépendante du type lui-même.
- Champs racine optionnels d'habillage (aucun n'affecte la géométrie/collision) :
  `"background"` (nom d'asset de `Assets/Backgrounds/`, `EX-REN-044`), `"skinSet"` (nom d'un jeu de
  `skins.json`, `EX-EDIT-024` — absent = jeu par défaut), `"planes"` (liste **ordonnée** d'objets
  `{ "file", "pixelsPerUnit"?, "parallaxX"?, "parallaxY"?, "opacity"?, "depth"? }`, `EX-DEC-040` à
  `EX-DEC-043` — voir « Plans picturaux » ci-dessous), `"parallax"` (booléen, `true` par
  défaut), et `"cameraFraming"` (`{ "mode", "roomWidthTiles"?, "roomHeightTiles"?,
  "zones"? }`, `EX-LVL-006` — `mode` vaut `"wholeLevel"`/`"perRoom"`/`"follow"`, absent =
  règle de repli par dimensions, cf. `core::resolveCameraFraming`).
- Types de tuiles : `empty`, `solid`, `danger`, `entry`, `exit`, `switch`, `pressurePlate`, `door`,
  `block`, `key`, `lockedDoor`. Le vocabulaire propre au RPG (terrains, obstacles, escaliers,
  ponts) est introduit par le `LOT-08`.
- Liaisons de mécanismes : un `switch`/`pressurePlate` porte un `"id"`, la `door` qu'il ouvre porte
  `"opensWith"` avec ce même identifiant. Même schéma pour `key` ↔ `lockedDoor`, à ceci près
  qu'une clé ramassée ouvre sa porte **définitivement**.
- Le format est **versionné** (`"version"`, cf. `core::LEVEL_FORMAT_VERSION`) : un fichier sans le
  champ est lu comme la version initiale, et le chargeur refuse proprement une version qu'il ne
  connaît pas. La montée en multi-couches (sol / décor / collision) est portée par le `LOT-04`.
