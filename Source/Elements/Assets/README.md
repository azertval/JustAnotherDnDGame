# Elements/Assets/

**Assets graphiques éditables hors code** (`LOT-39`, `EX-REN-042`) : atlas de tuiles au format PNG,
copié à côté de l'exécutable au build (patron `Levels`/`Localization`) et chargé au démarrage par
`hmi::TextureAtlas` (`hmi::AssetPaths`/`hmi::TextureLoader`).

## Contenu

- `atlas.png` — l'atlas de tuiles + images d'animation du personnage. Convention de grille
  (inchangée depuis la génération procédurale historique, `hmi::TextureAtlas::TILE_SIZE`/
  `TILES_PER_SIDE`/`PLAYER_FRAME_SIZE`/`PLAYER_FRAME_COLUMNS`) :
  - une grille de tuiles carrées de 16×16 pixels, `TILES_PER_SIDE` (6) tuiles par côté, dans le
    coin haut-gauche de l'image ;
  - sous cette grille, une ou plusieurs lignes d'images d'animation du personnage (16×16 chacune,
    même largeur de grille), dans l'ordre `Idle` puis `Run` puis `Jump`
    (`hmi::flatPlayerFrameIndex`).
  - `hmi::TextureAtlas::tile(colonne, ligne)`/`playerFrameRegion(clip, index)` calculent la région
    à partir de ces seules constantes — **remplacer le fichier suffit** à changer l'apparence, sans
    toucher au code, tant que la grille (dimensions, position des cases utilisées par
    `hmi::TileVisuals::regionForTile`) reste respectée.

## Éditer / régénérer

- **Remplacer une tuile** : éditer `atlas.png` directement (n'importe quel éditeur d'image), en
  respectant la grille ci-dessus. Relancer l'éditeur pour voir le résultat (pas de rechargement à
  chaud à ce stade).
- **Régénérer `atlas.png` à partir de la génération procédurale historique** (référence, en cas de
  divergence ou pour repartir d'une base propre) :
  `JustAnotherDnDGame.exe --export-atlas=Source/Elements/Assets/atlas.png` (option de développement,
  n'ouvre aucune fenêtre — voir `Documentation/Guide/guide-rendu.md`).
- **Asset absent ou illisible** : `hmi::TextureAtlas` retombe automatiquement sur la génération
  procédurale (`EX-NFR-040`), sans bloquer le rendu — utile pour développer sans art final.

> Anciennement `Textures/` (réservé, jamais peuplé) — ce dossier est le point d'entrée réel des
> assets graphiques depuis le LOT-39, nommé `Assets/` pour suivre le même patron que `Levels/` et
> `Localization/`.

## `Skins/` — skins de tuiles et planches à raccords (`LOT-42`)

`skins.json` associe chaque type de tuile à un fichier de `Skins/` et à un **mode** de découpage.
Le fichier porte un numéro de `version` et regroupe les associations en **jeux nommés** (`jeux`),
pour qu'un même jeu puisse proposer plusieurs ambiances. Il s'édite depuis le panneau « Textures »
de l'éditeur ; l'écrire à la main reste possible.

### Mode `single` — une image, une case

Un PNG de **16×16 pixels exactement**, utilisé tel quel. C'est le mode qui convient à tout type
dont le voisinage n'a pas de sens : dangers, interrupteurs, portes, blocs, pentes et arrondis.

Pour les douze types à **silhouette inclinée ou courbe** (pentes, arrondis convexes et concaves,
au sol comme au plafond), dessiner un **carré plein** : le moteur détoure automatiquement l'image
à la forme exacte de la hitbox, de sorte que l'affichage ne puisse pas mentir sur la géométrie de
collision.

### Mode `bitmask16` — une planche de raccords

Un PNG de **4×4 cases de 16×16 pixels** (64×64 au total). La case affichée est choisie selon les
**quatre voisins orthogonaux solides** de la tuile, ce qui donne des bords et des coins distincts
de l'intérieur — sans quoi un mur de vingt tuiles répéterait vingt fois le même motif et la grille
resterait visible.

Contenu attendu de chaque case (colonne 0 à gauche, ligne 0 en haut) :

| Ligne | Colonne 0 | Colonne 1 | Colonne 2 | Colonne 3 |
|:-----:|-----------|-----------|-----------|-----------|
| **0** | isolée (bordée partout) | extrémité basse de colonne | extrémité gauche de rangée | coin extérieur bas-gauche |
| **1** | extrémité haute de colonne | segment vertical | coin extérieur haut-gauche | bord gauche |
| **2** | extrémité droite de rangée | coin extérieur bas-droite | segment horizontal | bord bas |
| **3** | coin extérieur haut-droite | bord droit | **bord haut** (dessus de plateforme) | **intérieur plein** |

Deux conventions, explicites parce qu'elles se voient à l'écran :

- **L'extérieur du niveau compte comme solide.** Un mur de bordure ne dessine donc pas de contour
  sur sa face invisible, hors du niveau.
- **Le raccord suit la solidité, pas le type.** Un mur et un bloc poussable se raccordent entre
  eux : ils forment visuellement la même matière, et exiger le même type laisserait une couture
  partout où ils se touchent. Les pentes et arrondis, jamais solides, ne participent pas au
  voisinage.

## `Backgrounds/` — fonds de niveau (`LOT-44`)

Un niveau peut désigner une image de `Backgrounds/` comme fond (`core::Level::background`,
`EX-REN-044`), sélectionnée depuis la section « Fond » du panneau « Textures ». Dimensions
**libres** : le rendu étire l'image sur les bornes du niveau en préservant son ratio d'aspect
(recadrage par le centre sur la dimension excédentaire, jamais de déformation).

## `Objects/` — textures d'objets interactifs (`LOT-45`)

Un niveau peut assigner une texture à **une case précise** (`core::TileTextureOverride`,
`EX-EDIT-043`), sélectionnée depuis la section « Objets » du panneau « Textures » — prioritaire sur
le skin de son type. Voir `Objects/README.md` pour le détail du format attendu.

## Animations (`LOT-46`)

Un asset de `Skins/` (mode `single`) accompagné d'un fichier `<asset>.anim.json` de même nom
s'anime : la spritesheet est une bande horizontale (une case de haut, une image par case), décrite
par ses clips (`hmi::AnimationCatalog`). Un asset sans ce fichier reste une image fixe, sans erreur
ni avertissement — voir `Skins/README.md` pour le format et les exemples livrés (`water`, `lava`,
`torch`) et `Documentation/Lot/LOT-46-moteur-animation/` pour le détail du moteur.

## Apparence des mécanismes (`LOT-47`)

Une porte, un interrupteur, une plaque de pression et les dangers commuté/temporisé/mobile
changent d'apparence selon leur **état logique**, en mode Texture : `hmi::MechanismVisuals`
traduit l'état lu dans `Core` en **nom de clip attendu**, que l'asset assigné au type (section
« Animations » du panneau « Textures », ou skin ordinaire de la section « Skins » — même
mécanisme, `hmi::SkinCatalog`) doit fournir dans son `<asset>.anim.json`.

Convention de noms, un asset par famille :

| Famille (`core::TileType`) | Clips attendus |
|---|---|
| `Door` (porte) | `closed`, `opening`, `open`, `closing` — `opening`/`closing` sont des transitions jouées **une fois** (`"loop": false`), qui enchaînent sur l'état cible via `"next"` (`opening.next = "open"`, `closing.next = "closed"`). |
| `Switch` (interrupteur) | `inactive`, `active` |
| `PressurePlate` (plaque de pression) | `released`, `pressed` |
| `DangerSwitched` (danger commuté) | `inactive`, `active` |
| `DangerBlink` (danger temporisé) | `harmless`, `lethal` |
| `DangerMover` (danger mobile) | `idle` (un seul clip : l'état est porté par la position, pas par une bascule) |
| `Key` (clé, `EX-GP-023`, `LOT-63`) | `present`, `collected` |
| `LockedDoor` (porte verrouillée, `EX-GP-023`, `LOT-63`) | `closed`, `open` |

Un asset qui ne fournit pas un clip attendu **n'efface pas** la tuile : le jeu retombe sur le
premier clip disponible (repli lisible) et journalise l'état et le clip manquants **une seule
fois**, jamais à chaque pas. Le panneau « Textures » (section « Animations ») diagnostique ces
clips manquants sans lancer le jeu. Seule la **porte** transitionne visiblement ; les autres
familles basculent directement d'un état à l'autre. Cette apparence reste purement visuelle : la
collision d'une porte bascule au pas fixe où `core::MechanismController` le décide, jamais à la
fin d'une transition (voir `Documentation/Lot/LOT-47-etats-visuels-mecanismes/`).

## `Player/` — spritesheet du personnage (`LOT-48`)

Un fichier `Player/player.png`, accompagné de sa description `Player/player.anim.json` (même
format que `Skins/`, `hmi::AnimationCatalog`), habille le personnage en mode Texture — grille
d'images de la taille d'une case ou d'un multiple (`hmi::AssetFamily::CharacterSheet`, une bande
horizontale, une image par case, comme un skin animé). Fichier absent ou invalide : repli
automatique sur la silhouette procédurale historique (`atlas.png`), sans plantage ni avertissement
bloquant — le jeu reste jouable et lisible sans aucun asset.

**Ancrage image ↔ hitbox** : la taille de l'image est **indépendante** de la boîte de collision
(`core::playerSize()`, 0,4×0,8 unité, seule source de vérité de la hitbox — ce fichier ne la
modifie jamais). Le point d'ancrage est fixe et non configurable : le **centre-bas** de l'image
coïncide avec le centre-bas de la hitbox (`hmi::computePlayerSpriteQuad`). Une image plus grande
que la hitbox (cape, cheveux, effet de dash) déborde donc symétriquement de chaque côté et vers le
haut, jamais vers le bas ni les côtés de façon asymétrique — c'est ce qui permet à l'auteur de
dessiner un personnage aligné sans calcul : centrer le sujet horizontalement, poser ses pieds en
bas de chaque image.

**Un seul sens dessiné** : le personnage regarde par défaut vers la **droite** ; le jeu retourne
l'image horizontalement quand `core::Player::facing` pointe vers la gauche (aucun art à dupliquer).

**Clips attendus**, projetés depuis l'état de simulation (`core::AnimationSystem`) : `idle`, `run`,
`jump`, `fall` (chute, distincte du saut), `land` (atterrissage, transition jouée une fois),
`wallslide` (glissade murale), `dash`. Un clip non fourni par la spritesheet **n'empêche pas** de
l'utiliser : le jeu retombe sur le plus proche déclaré (`fall → jump`, `land → idle`,
`wallslide → jump`, `dash → run`) — une spritesheet ne dessinant que `idle`/`run`/`jump` reste donc
parfaitement valide, exactement comme l'atlas procédural qu'elle remplace.

## À venir (programme `LOT-40` → `LOT-55`)

Ce dossier accueillera d'autres **sous-dossiers par famille d'asset**, chacun avec ses dimensions
attendues **validées au chargement** (`EX-REN-007`).

> **`LOT-69`** — le sous-dossier `Decors/`, ajouté au `LOT-49`, a été **retiré** : l'habillage d'un
> niveau n'est plus un assemblage d'images posées mais un **plan pictural** peint, stocké à côté du
> niveau (`Source/Elements/Levels/Plans/`) et non parmi les assets réutilisables. Un plan est une
> donnée **de niveau**, pas une famille d'asset.

Le **rechargement à chaud**, absent aujourd'hui, arrive au `LOT-43` : éditer un asset se reflétera
sans relancer l'application.
