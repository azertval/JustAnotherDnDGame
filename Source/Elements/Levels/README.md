# Elements/Levels/

Cartes du jeu, un fichier **JSON** par carte (`EX-LVL-001`, `EX-LVL-003`).

> **Dossier vide au `LOT-01`.** Les 26 tableaux de démonstration du jeu de plateforme dont ce dépôt
> est dérivé ont été retirés avec leur gameplay. Le contenu du RPG arrive avec le `LOT-27` ; les
> premières cartes d'essai apparaîtront dès le `LOT-06` (déplacement top-down).

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
