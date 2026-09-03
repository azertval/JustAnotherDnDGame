# Rendu 2D : de l'ECS à l'écran {#guide-rendu}

Cette page explique comment une entité ECS (@ref guide-ecs) — une simple combinaison de données —
finit par apparaître comme une image à l'écran, en partant des notions de base du rendu temps réel
pour qui n'en a jamais écrit. Tout le rendu vit dans `Source/HMI/Graphics`, sur une surface fournie
par le viewport Qt (`Source/HMI/Game`) ; c'est la seule partie du moteur qui dépend du GPU, via
**QRhi** (voir plus bas — `Core` en reste totalement indépendant, @ref guide-boucle et
`EX-ARCH-040`).

## Vocabulaire de base : GPU, swap chain, back buffer

Un jeu ne dessine pas directement sur l'écran : il dessine dans une zone mémoire dédiée sur la
carte graphique (le **GPU**, *Graphics Processing Unit*, un processeur spécialisé dans le calcul
massivement parallèle nécessaire pour colorier des millions de pixels par seconde), puis cette
image est transmise à l'écran. Dessiner directement dans l'image **actuellement affichée**
provoquerait un artefact visible (*tearing* : une moitié d'image montre l'ancien contenu, l'autre
le nouveau, si l'écran est en train de la rafraîchir pendant qu'on la modifie). La solution
standard, le **double buffering**, utilise **deux** images en mémoire :

- le **front buffer** : l'image actuellement montrée à l'écran, intouchable ;
- le **back buffer** : une image « en coulisse », sur laquelle le jeu dessine librement la frame
  suivante.

Une fois le back buffer entièrement dessiné, une opération de **présentation** échange les deux
rôles (le back buffer devient le front buffer et inversement) — idéalement au moment précis où
l'écran finit de rafraîchir l'image précédente, ce qui s'appelle la **synchronisation verticale**
(V-Sync) : elle évite le *tearing* en alignant l'échange sur le rythme de rafraîchissement de
l'écran, au prix d'attendre ce moment si le jeu est plus rapide que l'écran. L'ensemble
« back buffer(s) + mécanisme d'échange » s'appelle une **swap chain**.

## QRhi : une couche d'accès au GPU, pas un changement de cible

Depuis le `LOT-69` TACHE-02, le projet ne parle plus à Direct3D 11 directement : il passe par
**QRhi**, la couche d'abstraction de rendu de Qt. La cible technique ne change pas — QRhi retient
Direct3D 11 par défaut sous Windows (`EX-REN-002`, amendée) — mais le device, la *swap chain* et la
présentation appartiennent désormais à Qt. La classe qui les portait a été **supprimée** plutôt que
portée : elle n'avait plus rien à porter.

Ce que le projet conserve en propre :

- `hmi::SpriteBatch` : le pipeline 2D (tampons de sommets et d'indices, tampon uniforme,
  échantillonneur, états de mélange) et l'émission des lots de dessin ;
- `hmi::TextureLoader` : la création des textures GPU à partir de pixels décodés ;
- les **shaders**, écrits une fois en GLSL et compilés en `.qsb` par l'outil `qsb` — un `.qsb`
  contient plusieurs traductions (SPIR-V, HLSL, MSL), ce qui permet à QRhi de choisir son backend à
  l'exécution sans que le projet livre un shader par API.

Deux contraintes de QRhi façonnent le code, et méritent d'être connues avant de le lire :

1. **Un téléversement ne se déclare pas pendant une passe de rendu.** Les données (sommets,
   pixels de texture) transitent par un `QRhiResourceUpdateBatch`, soumis **avant** l'ouverture de
   la passe. `hmi::SpriteBatch` enregistre donc toute l'image côté CPU, puis téléverse une fois et
   dessine — au lieu de réécrire son tampon entre deux appels de dessin comme le permettait
   Direct3D 11.
2. **L'espace de clip du shader est celui d'OpenGL**, quelle que soit la cible : la matrice de
   projection est multipliée par `QRhi::clipSpaceCorrMatrix()`, qui la ramène à la convention du
   backend retenu.

## La surface de dessin : le viewport Qt (`hmi::GameViewport`)

Depuis le `LOT-69` TACHE-02, le viewport est un **`QRhiWidget`** : un widget ordinaire, qui rend
dans une texture d'appui composée avec le reste de l'interface. Il n'y a plus de fenêtre native
embarquée, donc plus de `HWND` à transmettre, et surtout plus la limitation qui l'accompagnait — un
widget Qt ne se dessinait jamais de façon fiable par-dessus une fenêtre native, ce qui avait coûté
deux défauts réels au `LOT-59` (écran de pause invisible, puis vol de focus). Les recouvrements du
jeu sont redevenus de simples widgets enfants.

Le viewport a une seconde responsabilité : il possède la **boucle de rendu** (chaque image
redemande la suivante par `QWidget::update()`, et `QRhiWidget::render` l'exécute) et traduit les
événements clavier/souris **Qt** en `hmi::InputState` (@ref
guide-entrees) — c'est pour cela que la capture d'entrée vit au même endroit que le rendu, plutôt que
dans un module totalement séparé.

## Unités monde et pixels : \ref hmi::Camera2D "hmi::Camera2D"

`Core` ne connaît que des **unités monde** (une tuile = 1 unité, @ref guide-maths) — jamais de
pixels. Le rendu doit donc **convertir** une position monde en position d'écran avant de dessiner
quoi que ce soit ; c'est le rôle de `hmi::Camera2D`. Deux paramètres gouvernent cette conversion :

- `PIXELS_PER_UNIT = 16` : l'échelle de base, fixée par convention du projet (`EX-ARCH-021`) — une
  entité de 1×1 unité (une tuile) occupe 16×16 pixels à l'écran avant tout zoom ;
- le **zoom** (`setZoom`) : un multiplicateur additionnel de cette échelle. Un zoom **entier** est
  recommandé (`EX-ARCH-022`) : avec un style *pixel art*, un zoom non entier (1,5× par exemple)
  étirerait certains pixels source sur 1 pixel écran et d'autres sur 2, brisant la netteté des
  contours voulue par ce style visuel.

La caméra a aussi un **centre** (`setCenter`, en unités monde) : le point qui apparaît au milieu de
l'écran. L'éditeur (mode édition du viewport) ne fait jamais suivre ce centre en continu au
personnage : il cadre toujours le niveau entier, avec pan/zoom manuel (`EX-EDIT-013`). En jeu, le
centre est recalculé par **cadrage**, selon le mode choisi par le niveau (`EX-REN-016`, trois
modes détaillés ci-dessous) : le milieu du niveau entier, celui de la **salle courante**, ou un
suivi continu du personnage — le seul des trois qui fait effectivement bouger le centre à chaque
pas. `projectionMatrix()` combine centre, échelle
et dimensions de la fenêtre (le *viewport*) en une **matrice de projection orthographique** : une
transformation mathématique standard en rendu 2D/3D qui convertit une position monde en position
« clip » — l'espace normalisé que le GPU attend en sortie du *vertex shader* (voir plus bas). C'est
cette matrice, et non une conversion manuelle pixel par pixel, que le pipeline de dessin applique à
chaque sommet ; `worldToScreen`/`screenToWorld` exposent la même conversion côté CPU, pour des
besoins hors dessin (par exemple convertir une position de souris en position monde).

### Cadrer un contenu plus grand que la fenêtre : `fitZoom` et `hmi::RoomGrid`

`Camera2D::fitZoom` (`LOT-16`) calcule le zoom qui fait tenir un rectangle donné (en unités monde)
dans une surface disponible (en pixels), sans jamais laisser de zone hors champ : zoom **entier**
tant que le rectangle tient déjà à l'échelle ×1 (netteté pixel art, `EX-ARCH-022`), fractionnaire
seulement si nécessaire pour l'englober malgré tout. Fonction pure, partagée par l'éditeur (niveau
entier) et le jeu.

En jeu (`GameSession`), pour un niveau plus grand qu'une **salle** (`hmi::RoomGrid`, `LOT-32`, taille
fixe en tuiles), ce rectangle n'est **plus le niveau entier** mais celui de la **salle** contenant
le personnage — façon *Celeste* : la caméra reste au zoom pixel art natif quelle que soit la taille
totale du niveau, et **bascule nettement** (un seul appel `setCenter`, pas d'interpolation) quand
`RoomGrid::roomIndexAt` désigne une salle différente de la précédente. Un niveau qui tient dans une
seule salle retombe exactement sur le cadrage « niveau entier » de LOT-16, sans branche spéciale :
`RoomGrid` produit alors une unique salle couvrant le niveau. L'éditeur, lui, garde son cadrage
« niveau entier » avec pan/zoom manuel (`EX-EDIT-013`) — seul un quadrillage superposé (`F10`)
indique les frontières de salles, sans changer sa caméra (@ref guide-editeur).

### Le cadrage choisi par le niveau : trois modes (`LOT-64`)

Avant `LOT-64`, le choix entre « niveau entier » et « par salle » ci-dessus était une règle **unique
et en dur**, déduite des dimensions du niveau (tient dans une salle ou non) — invisible depuis
l'éditeur, et sans échappatoire pour un niveau qui aurait voulu l'un ou l'autre indépendamment de sa
taille. `core::CameraFramingConfig` (`Source/Core/Levels/CameraFraming.h`) en fait une **donnée du
niveau** (`EX-LVL-006`), au même titre que ses tuiles : trois modes, `WholeLevel`, `PerRoom` et
`Follow` (`EX-REN-016`), choisis par le level designer dans la section « Cadrage » de l'éditeur (@ref
guide-editeur), pas déduits.

**Règle de repli** (`core::resolveCameraFraming`) : un niveau qui ne déclare aucun champ
`cameraFraming` — tous les niveaux antérieurs à ce lot — se comporte **exactement** comme avant :
`WholeLevel` s'il tient dans une salle de taille par défaut, `PerRoom` sinon. C'est le seul endroit
qui incarne cette règle ; ni `hmi::GameSession` ni l'éditeur ne la recalculent — ils lisent toujours
un cadrage déjà **résolu** (`core::Level::cameraFraming()`), jamais un champ « peut-être absent ».
La taille de salle du mode *par salle* est elle-même réglable par niveau
(`core::CameraFramingConfig::roomWidthTiles` et `roomHeightTiles`) ; `hmi::RoomGrid::ROOM_WIDTH_TILES`/`ROOM_HEIGHT_TILES`
(`LOT-32`) n'en restent que la valeur par défaut, `RoomGrid` recevant désormais la taille en
paramètre de construction plutôt que de la connaître en dur.

**Le mode `Follow`** (`hmi::FollowCamera.h`) est le seul des trois qui manquait réellement au
moteur : les deux autres ne faisaient qu'exposer une règle qui existait déjà. Il accompagne le
personnage avec quatre mécanismes combinés, chacun répondant à un défaut connu de cette famille de
caméra :

- une **zone morte** (`FOLLOW_DEAD_ZONE_HALF_WIDTH_UNITS`/`HEIGHT_UNITS`) : le personnage se déplace
  librement dans un petit rectangle centré sur le point suivi (l'**ancre**) sans faire bouger la
  caméra ; l'ancre elle-même ne se déplace que lorsque le personnage sort de ce rectangle, tout
  juste assez pour l'y ramener au bord — c'est ce qui supprime le tremblement permanent d'une
  caméra qui collerait exactement à la position du personnage ;
- une **anticipation** (`FOLLOW_ANTICIPATION_DISTANCE_UNITS`) : le centre visé est décalé devant le
  personnage, dans le sens de son déplacement (`core::Player::facing`), pour qu'on voie où l'on va.
  Ce décalage s'**inverse progressivement** au changement de sens (son propre lissage exponentiel,
  `FOLLOW_ANTICIPATION_TIME_CONSTANT_SECONDS`), jamais d'un coup — une inversion instantanée donne
  le mal de mer ;
- un **lissage** exponentiel (`FOLLOW_SMOOTHING_TIME_CONSTANT_SECONDS`) vers ce centre visé, à temps
  de réponse **constant** (indépendant du pas) : `centre += (cible − centre) × (1 − e^(−dt/τ))` ;
- un **bornage** aux limites du niveau : la caméra ne montre jamais hors de la grille. Le cas
  particulier — un niveau plus étroit que le cadrage sur un axe — **centre** la caméra sur cet axe
  plutôt que de la border, sinon elle resterait collée à un bord en permanence.

`hmi::advanceFollowCamera` est une **fonction pure** (aucune horloge système, aucune dépendance
GPU), testée exhaustivement sans GPU (`Source/Test/Unit/HMI/Graphics/test_follow_camera.cpp`) —
même statut que `hmi::RoomGrid`/`hmi::Parallax`. Elle avance l'état d'un pas et renvoie un nouveau
centre déjà borné ; deux pièges, propres à cette famille de caméra, méritent d'être nommés :

1. **Le lissage doit être cadencé sur le pas fixe, jamais sur la fréquence de rendu**
   (`EX-REN-021`) : `hmi::GameSession::updateFollowCamera` avance l'état une seule fois par pas de
   simulation, avec `fixedDelta` — jamais avec un delta de frame. Une caméra lissée par image se
   comporterait différemment à 60 et à 144 Hz, un défaut de déterminisme visuel classique de cette
   architecture à pas fixe découplé du rendu (@ref guide-boucle).
2. **Le centre de caméra doit lui-même être interpolé au rendu**, exactement comme une entité
   portant `hmi::PreviousPosition` (section précédente) : `GameSession::applyCameraFraming` calcule
   `lerp(centre du pas précédent, centre du pas courant, alpha)` avant d'appeler `setCenter`. Sans
   cette étape, le personnage — rendu lisse par interpolation — semblerait **trembler** par rapport
   à un décor calé sur un centre de caméra qui ne bouge que par sauts discrets, une fois par pas
   fixe. C'est le prolongement direct du mécanisme déjà en place pour les entités mobiles : la
   caméra de suivi est, elle aussi, une position qui change au pas fixe et doit donc, elle aussi,
   être interpolée pour l'affichage.

Le centre finalement retenu est **aligné sur la grille de pixels** à l'échelle de rendu courante
(`hmi::roundToScreenPixel`, la même fonction que la parallaxe des plans ci-dessous), **après**
l'interpolation ci-dessus : un centre fractionnaire échantillonnerait chaque texture entre deux
texels et ruinerait la netteté du pixel art que tout le projet protège depuis le `LOT-05`. Le zoom
reste **entier** dans les trois modes (`EX-ARCH-022`), calculé par la même `Camera2D::fitZoom` que
les deux autres modes, appliquée à une surface de référence — la taille de vue du mode suivi
(`EX-REN-017`, voir ci-dessous), faute de rectangle de contenu naturel à ajuster.

### Mélanger plusieurs tailles de caméra : zones dessinées à la main et taille de suivi réglable (`EX-LVL-007`, `EX-REN-017`)

La grille automatique du mode *par salle* ci-dessus impose une **seule** taille de salle pour tout
le niveau. `core::CameraFramingConfig::zones` (une liste de `core::CameraZone`, rectangles en
tuiles) lève cette limite : un niveau peut porter des zones de caméra de tailles **différentes**,
dessinées par le level designer directement sur le canevas de l'éditeur (@ref guide-editeur) plutôt
que calculées. Liste vide par défaut : sans zone dessinée, le comportement de grille automatique
décrit plus haut reste **inchangé**, `zones` n'étant qu'une donnée optionnelle en plus, jamais une
modification du calcul par défaut.

`hmi::activeCameraZoneIndex` (`Source/HMI/Graphics/CameraZones.h`, fonction pure, sans GPU) résout
la zone active : la **première** zone de la liste dont le rectangle contient la position du
personnage (bornes hautes/gauches incluses, basses/droites exclues) l'emporte — même convention
de priorité par ordre que la superposition des plans picturaux (@ref guide-editeur). Aucune zone ne
contient le personnage (trou entre deux zones dessinées) → repli sur le **niveau entier**, jamais
un état indéfini. `hmi::GameSession` résout la zone active au chargement et à chaque bascule,
exactement comme elle résout déjà la salle active de la grille automatique
(`RoomGrid::roomIndexAt`) ; les deux mécanismes ne coexistent jamais pour un même niveau, le choix
entre l'un et l'autre se faisant simplement sur `zones.empty()`.

Le mode `Follow` réutilise les **mêmes champs** que la taille de salle du mode *par salle*
(`roomWidthTiles`/`roomHeightTiles`) pour sa propre taille de vue, plutôt que de retenir en dur la
taille de salle par défaut : un niveau qui veut un suivi plus large ou plus étroit que cette
valeur par défaut le déclare, sans champ dédié supplémentaire — même donnée, deux usages
(`core::validateCameraFramingConfig` l'accepte pour les deux modes).

## Le pipeline de dessin de sprites : \ref hmi::SpriteBatch "hmi::SpriteBatch"

### Pourquoi « batcher » plutôt que dessiner un sprite à la fois

Chaque appel de dessin adressé au GPU (un *draw call*) a un coût fixe non négligeable, indépendant
du nombre de pixels dessinés — piloté par la communication CPU → GPU, pas par le travail du GPU
lui-même. Un niveau de plusieurs centaines de tuiles dessinées par des appels **individuels**
saturerait ce coût fixe avant même de saturer le GPU. Le **batching** (« dessin par lots ») regroupe
un grand nombre de sprites partageant la **même texture** en un minimum d'appels de dessin :
`SpriteBatch` accumule des quads (voir plus bas) dans un tampon CPU et ne les envoie au GPU qu'en un
seul appel, au moment de `flush()`/`end()` — l'usage est `begin(projection, texture)`, puis un ou
plusieurs `draw(quad)`, puis `end()`.

### \ref hmi::SpriteQuad "SpriteQuad" : un rectangle texturé

Un **quad** est simplement un rectangle (deux triangles, en pratique — un GPU ne sait dessiner que
des triangles). `hmi::SpriteQuad` en décrit un par sa position/taille en **unités monde** (`x, y,
width, height`), la portion de texture à échantillonner en **coordonnées UV normalisées** (`u0, v0,
u1, v1`, chacune dans `[0, 1]` — la convention universelle du rendu temps réel pour désigner un
point dans une texture indépendamment de sa résolution en pixels), et une teinte RVBA (`r, g, b,
a`) multipliée avec la texture au dessin — une teinte blanche opaque (1,1,1,1) laisse la texture
inchangée, une teinte plus sombre ou colorée module son apparence sans créer de variante de texture
séparée.

### Sommets, shaders, et échantillonnage *nearest*

En interne, chaque quad devient 4 **sommets** (`Vertex` : position, UV, couleur), envoyés au GPU
avec deux petits programmes qui s'exécutent **sur le GPU** lui-même :

- le **vertex shader** transforme chaque position de sommet (unités monde) vers l'espace clip, via
  la matrice de projection de la caméra ;
- le **pixel shader** (aussi appelé *fragment shader*) calcule la couleur finale de chaque pixel
  couvert par les triangles, en échantillonnant la texture à la coordonnée UV interpolée et en la
  multipliant par la couleur du sommet.

L'échantillonnage utilise le mode ***nearest*** (au lieu du filtrage *bilinéaire*, plus courant
ailleurs) : il choisit le pixel de texture le **plus proche** de la coordonnée demandée, sans
mélanger ses voisins. C'est délibéré pour un rendu **pixel art** : le filtrage bilinéaire
adoucirait/flouterait les contours nets des sprites, un effet indésirable dans ce style visuel.

Le pipeline gère aussi la **transparence** (`_blendState`) : sans un état de *blending* configuré,
le canal alpha d'un quad (utile pour les zones transparentes de l'atlas, voir plus bas) serait
ignoré et chaque sprite dessinerait un rectangle plein.

### \ref hmi::LineQuad "LineQuad" : un segment orienté (liens de mécanismes, `LOT-37`)

`SpriteQuad` décrit toujours un rectangle **aligné aux axes** (`x, y, width, height`) : impossible
d'en tirer un trait en diagonale, nécessaire pour relier deux cases quelconques de la grille (flèche
déclencheur → cible, @ref guide-editeur). `hmi::LineQuad` couvre ce cas sans nouveau pipeline ni
nouveau shader — même tampon, même `draw`/`flush`, juste une seconde façon de calculer les 4
sommets : au lieu d'un rectangle, deux **extrémités** (`ax, ay, bx, by`, unités monde) et une
**épaisseur** perpendiculaire au segment (`thickness`). `SpriteBatch::draw(const LineQuad&)`
calcule la direction normalisée du segment, en déduit une normale (perpendiculaire, longueur
`thickness / 2`), et pousse directement les 4 sommets décalés de part et d'autre des deux
extrémités — le même tampon d'indices (deux triangles par quad) s'applique sans changement, quelle
que soit l'orientation. Un segment dégénéré (les deux extrémités confondues) ne pousse aucun sommet.
Les liens de mécanismes réutilisent la région opaque de l'atlas comme UV (même technique que la
grille de repère, `DraftRenderer::drawGrid`) : la couleur vient uniquement de la teinte RVBA, pas
d'une texture dédiée.

## \ref hmi::TextureAtlas "hmi::TextureAtlas" : un spritesheet, chargé depuis un fichier

Un **atlas de texture** (ou *spritesheet*) regroupe **plusieurs** images dans une **seule** grande
texture, à des positions connues. C'est ce qui permet le batching décrit plus haut :
`SpriteBatch::begin` ne prend **qu'une seule** texture par lot, donc dessiner des sprites différents
dans le même appel exige qu'ils proviennent tous du même atlas — d'où l'intérêt de regrouper toutes
les tuiles d'un jeu dans un seul atlas plutôt qu'une texture par tuile.

`hmi::TextureAtlas` charge son contenu depuis `Assets/atlas.png` (à côté de l'exécutable,
`EX-REN-041`/`EX-REN-042`, `LOT-39`) : une grille de tuiles de 16 pixels de côté (`TILE_SIZE`).
`tile(colonne, ligne)` renvoie la **région** (rectangle en pixels) d'une tuile de cette grille —
c'est cette région, convertie en UV normalisées, qu'un `Sprite` (@ref guide-ecs, composant
`core::Sprite`) référence via son champ `region` (en pixels, agnostique de la résolution réelle de
l'atlas — c'est le rendu qui la normalise). `tile`/`playerFrameRegion` sont de la pure arithmétique
de grille (`static`, aucun état d'instance) : leur résultat ne dépend jamais de l'origine —fichier
ou procédurale— de l'atlas, seulement des constantes de la classe.

### Le pipeline de textures depuis fichiers, et son repli procédural

Avant `LOT-39`, ce projet n'avait pas d'atelier graphique fournissant des images dessinées à la
main : l'atlas était **généré en code**. Cette génération (couleurs distinctes par tuile, dont une
avec des zones transparentes pour exercer le canal alpha, plus les images d'animation du
personnage) n'a pas disparu : `hmi::buildProceduralAtlasImage` (`HMI/Graphics/ProceduralAtlas.h`,
logique **pure**, sans Direct3D ni Qt) reste l'unique source de vérité de ce contenu de référence,
et sert maintenant de **repli** — si `Assets/atlas.png` est absent ou illisible, `TextureAtlas`
retombe dessus sans plantage (`EX-NFR-040`), avec un message de log clair. C'est ce qui permet de
développer sans art final, et de ne jamais bloquer le rendu sur un asset manquant.

Le chargement fichier lui-même (`HMI/Graphics/TextureLoader.h`) se déroule en deux étapes,
symétriques du repli procédural + upload GPU :

1. **Décodage** (`decodeImageFile`) : `QImage::load` puis `convertToFormat(Format_RGBA8888)` — Qt
   est déjà une dépendance depuis `LOT-34`, donc aucune bibliothèque supplémentaire. `RGBA8888` est
   choisi **non prémultiplié** : le blend de `SpriteBatch` utilise
   `QRhiGraphicsPipeline::SrcAlpha`/`OneMinusSrcAlpha` (alpha simple), pas `One` — un format
   prémultiplié donnerait des couleurs assombries aux bords transparents.
2. **Upload GPU** (`createTexture`) : exactement le même chemin `QRhi::newTexture`
   (`QRhiTexture::RGBA8`, un seul niveau de mip) puis `QRhiResourceUpdateBatch::uploadTexture` que
   la génération procédurale — les deux chemins partagent cette fonction, il n'existe qu'un seul
   endroit qui crée une texture d'atlas sur le GPU.

La résolution du chemin d'asset (`hmi::AssetPaths`, `HMI/Graphics/AssetPaths.h`) est, elle, une
classe **pure** (aucune dépendance fenêtre/GPU/Qt) : elle résout un nom de fichier logique vers un
chemin dans un dossier donné, et renvoie `std::nullopt` si le fichier est absent — jamais
d'exception. `TextureAtlas` la construit avec `hmi::executableDirectory() / "Assets"`, le même
patron que `Levels`/`Localization` (@ref guide-editeur). Cette séparation (résolution de chemin
pure / décodage+upload dépendant de Qt+GPU) est ce qui permet de tester le mapping de régions et la
résolution d'assets **sans GPU** (`EX-NFR-010`, `Source/Test/Unit/HMI/Graphics/`), alors que le
décodage et la création de texture restent vérifiés visuellement.

### `Source/Elements/Assets/` : convention et régénération

L'atlas de base (`Source/Elements/Assets/atlas.png`) suit la même grille que la génération
procédurale (voir `Source/Elements/Assets/README.md` pour le détail des dimensions) : **remplacer
le fichier suffit** à changer l'apparence du jeu, sans toucher au code, tant que la grille est
respectée. Il est copié à côté de l'exécutable au build (patron CMake `POST_BUILD` de
`Levels`/`Localization`, `Source/HMI/CMakeLists.txt`).

Pour régénérer cet atlas de base à partir de la génération procédurale de référence (après une
évolution de `buildProceduralAtlasImage`, ou pour repartir d'une base propre) :
`JustAnotherDnDGame.exe --export-atlas=<chemin>.png` — option de développement traitée tout au début de
`main()`, **avant** l'ouverture de toute fenêtre : elle écrit le fichier PNG et quitte
immédiatement (code `0` en cas de succès). Il n'y a pas, à ce stade, de rechargement à chaud dans
l'éditeur — remplacer l'asset puis relancer l'application.

### Les images du personnage : pourquoi elles vivent dans le même atlas

Depuis LOT-17 (silhouette statique) puis LOT-18 (animation), `TextureAtlas` ne génère pas que la
grille de tuiles : une grille supplémentaire est ajoutée **sous** la grille de tuiles, où vivent les
**images de la silhouette humanoïde** (`playerFrameRegion(clip, frameIndex)`) — tête, cheveux,
torse/manches, mains, jambes, chaussures, chacun une couleur distincte, le reste transparent.
Chaque image reste **carrée** (16×16, exactement comme une tuile) : `SpriteRenderer::render`
multiplie ses dimensions en pixels par `Transform::scale` (cf. plus bas), et c'est cette échelle —
`core::playerSize()`, déjà non uniforme (0,4×0,8 unité monde) — qui donne à elle seule au
personnage sa proportion finale deux fois plus haute que large. Une région déjà non carrée
**doublerait** cet effet (bug réellement rencontré en LOT-17, corrigé aussitôt) : chaque image est
donc dessinée **pré-compressée** de moitié en hauteur dans son canevas carré, pour retrouver ses
proportions naturelles une fois étirée par l'échelle du `Transform`.

Cette contrainte (région carrée + pré-compression) reste **exacte pour ce chemin précis** : l'atlas
procédural, utilisé tel quel en `RenderMode::Physique` et comme repli de `RenderMode::Texture` en
l'absence de spritesheet externe (`core::Sprite::region`/`Transform::scale`, **inchangés** depuis
avant `LOT-48`). Elle ne s'applique plus à la spritesheet externe du personnage — voir la section
suivante.

### `LOT-48` : spritesheet externe et découplage image/hitbox

Le personnage était le seul sprite du jeu resté hors du programme d'habillage (`LOT-40` → `LOT-47`)
: en `RenderMode::Texture`, faute de `hmi::TileSkinTag` (réservé aux tuiles), il retombait sur le
damier magenta. `hmi::PlayerSpriteTag` (`HMI/Graphics/PlayerSpriteTag.h`) referme cet écart par un
mécanisme **dédié**, parallèle à `hmi::resolveTileAppearance` plutôt que branché dessus (le
personnage n'est pas une tuile) : `hmi::composeWorldSprites` reconnaît l'entité qui porte ce
composant et l'affiche selon son quad et sa texture **résolus**, en `RenderMode::Texture`
uniquement — `RenderMode::Physique` continue de lire `core::Sprite::region`/`Transform::scale`
exactement comme avant.

`GameSession::refreshPlayerSprite()` résout, chaque image :

1. La **spritesheet** (`Assets/Player/player.png`, `hmi::AssetFamily::CharacterSheet`), chargée par
   le `TextureCache` avec sa description `player.anim.json` (même format que `LOT-46`) ; absente ou
   invalide, repli sur l'atlas procédural — `hmi::PlayerSpriteTag::usesCharacterSheet` indique
   laquelle des deux textures lier.
2. Le **clip à afficher** : `core::AnimationSystem` résout un nom parmi sept (`idle`, `run`, `jump`,
   `fall`, `land`, `wallslide`, `dash`, voir plus bas) ; ni la spritesheet ni l'atlas procédural
   (qui n'en connaît que trois) n'ont à tous les fournir — `hmi::resolveDeclaredPlayerClip` fait
   retomber un clip absent sur le plus proche déclaré (`fall → jump`, `land → idle`,
   `wallslide → jump`, `dash → run`), chaîne de repli **unique**, appliquée identiquement à la
   spritesheet et à l'atlas procédural (traité comme une spritesheet qui n'en déclare que trois,
   `hmi::proceduralPlayerClipNames`).
3. Le **quad** : `hmi::computePlayerSpriteQuad` (fonction pure, testée sans GPU) ancre le
   **centre-bas** de l'image sur le centre-bas de `core::playerSize()` — la seule source de vérité
   de la hitbox, jamais lue en écriture par ce calcul. Une image plus grande ou plus large que la
   hitbox (cape, cheveux, effet de dash) déborde donc symétriquement, sans jamais déplacer la
   collision (`EX-ARCH-012`). L'ancrage horizontal centré rend le retournement (point 4) gratuit en
   position.
4. L'**orientation** : `core::Player::facing` détermine `hmi::PlayerSpriteTag::flipHorizontal` ;
   `hmi::composeWorldSprites` échange alors `u0`/`u1` du quad composé, sans toucher `quadOffset`.

Ce choix — étendre l'atlas existant plutôt que placer le personnage dans une texture séparée —
découle directement de la contrainte de batching énoncée plus haut : le personnage est dessiné à
**chaque** frame parmi des centaines d'autres sprites, d'où l'obligation de partager la texture de
`TextureAtlas`. `SpriteBatch::begin` (et donc
`SpriteRenderer::render`, qui ne fait qu'**un seul** `begin`/`end` pour **toutes** les entités du
monde) ne lie qu'**une seule** texture par lot. Une région de personnage dans une texture séparée
aurait exigé de restructurer `SpriteRenderer` pour trier les entités par texture et faire plusieurs
passes — hors de proportion pour ce lot. En la plaçant dans la texture de `TextureAtlas`, aucune
ligne de `SpriteRenderer` n'a besoin de changer : la normalisation UV s'appuie déjà, génériquement,
sur `atlas.width()`/`height()` (devenus des membres stockés plutôt qu'une formule figée sur un atlas
carré, pour accueillir cette grille supplémentaire).

Chaque image est dessinée par **blocs rectangulaires** (comparaisons d'intervalles sur les
coordonnées de pixel) — direct à lire et à ajuster pour une forme humanoïde à cette résolution. Une pose (largeur des bras,
écartement des jambes) est un simple paramètre de la fonction de dessin : les 7 images de la grille
(2 `Idle`, 4 `Run`, 1 `Jump`) sont produites par la même logique, avec des paramètres différents —
pas 7 fonctions dupliquées.

### L'animation : des clips en données, une progression générale (`LOT-46`)

`EX-REN-005`/`EX-REN-012` demandent une animation par séquence d'images, décrite par des
**données** plutôt que codée en dur, et applicable à **toute** entité — pas seulement au
personnage. Un clip (`core::AnimationClip`) est une donnée pure : un nom, une suite d'indices
d'images, une durée par image, bouclé ou joué une fois (`core::ClipEndMode`) avec un clip suivant.
Plusieurs clips forment un `core::ClipSet`, adressable par nom et résolu en index à l'ajout — la
progression au pas fixe ne compare donc jamais de chaîne. Le composant `core::Animation` référence
ce jeu de clips (`clips`, partagé via `shared_ptr` — plusieurs entités animées par le même jeu,
comme toutes les tuiles d'eau d'un niveau, n'en dupliquent pas le contenu), le clip courant déjà
résolu (`clipIndex`), l'image courante (`frameIndex`) et le temps écoulé (`elapsed`).

`core::AnimationSystem::update` (@ref guide-ecs) fait deux choses, dans une seule traversée des
entités portant `core::Animation` — **entièrement côté `Core`**, sans dépendance aux pixels ni à
`HMI` (`EX-ARCH-011`) :

1. **projection**, spécifique au personnage : pour une entité `Player` + `Velocity` + `Animation`,
   lit l'état physique déjà calculé par `CharacterPhysicsSystem` **pour le même pas** (l'ordre
   d'appel dans `GameSession::update` reste significatif) pour choisir le clip cible par son
   **nom**, parmi sept depuis `LOT-48` : « idle », « run », « jump », « fall », « land »,
   « wallslide », « dash », par ordre de priorité **explicite** (`core::AnimationSystem.cpp`,
   fonction `targetClipName`) — dash (`dashTimer` actif) domine tout, puis un atterrissage en cours
   ou qui débute (transition détectée par comparaison du clip **résolu au pas précédent** avec les
   trois clips aériens, comme les transitions de mécanismes `LOT-47` TACHE-02), puis glissade
   murale, puis chute/saut (signe de la vitesse verticale), puis course/repos ; un changement de
   clip réinitialise net l'image et le chronomètre, et consomme le pas sans faire progresser
   l'image. Aucun champ n'a été ajouté à `core::Player` pour cette extension : l'animation reste une
   **conséquence** de l'état physique existant (`EX-ARCH-012`) ;
2. **progression**, générale (`core::advanceAnimation`, réutilisable hors ECS) : avance l'image
   courante selon la durée du clip résolu, boucle ou bascule sur le clip suivant en fin de clip
   joué une fois — c'est ce mécanisme, déjà générique depuis `LOT-46`, qui enchaîne « land » sur
   « idle » une fois sa brève transition jouée. Une entité sans projection spécifique (une tuile
   animée, `LOT-46` TACHE-05) progresse par ce seul mécanisme.

Le personnage reste le seul consommateur de la projection ; son jeu de clips (`core::playerClipSet()`)
n'a pas de pose procédurale dédiée pour les quatre clips `LOT-48` : `HMI` les fait retomber sur le
plus proche déclaré (voir la section précédente), `Core` n'a pas à le savoir. Côté `HMI`,
`GameSession::render` appelle toujours `refreshPlayerSprite()` à chaque image : elle lit
`core::Animation` du personnage, résout le nom du clip courant (`core::ClipSet::clipAt(clipIndex)
.name`) et en tire à la fois la région **procédurale** (`core::Sprite::region`, comportement
inchangé) et l'apparence **habillée** (`hmi::PlayerSpriteTag`, `LOT-48`, section précédente).
`hmi::PlayerClipKind` reste une énumération **côté présentation seulement**
(`ProceduralAtlas.h`), distincte de `core::AnimationClip` générique : `Core` ignore jusqu'à
l'existence de ces trois poses procédurales en particulier (`EX-ARCH-012`).

Une description `nom-asset.anim.json` (`hmi::AnimationCatalog`), lue à côté de l'asset et mise en
cache par `hmi::TextureCache` (invalidée conjointement avec la texture), permet d'animer un skin de
tuile (eau, lave, torche) sans code supplémentaire : une horloge d'animation est alors partagée par
**asset**, pas par tuile (`GameSession::updateTileAnimations`, avancée au pas fixe), pour que toutes
les tuiles d'un même type restent en phase sans coût par case ; la région courante est résolue à la
**composition** (`hmi::sceneTextures`/`resolveTileAppearance`), jamais écrite dans `core::Sprite`.
Un asset sans fichier d'animation reste une image fixe, sans erreur ni avertissement.

## \ref hmi::SpriteRenderer "hmi::SpriteRenderer" : le pont ECS → écran

C'est ici que les fils se rejoignent. Depuis le `LOT-40`, le rendu se fait en **deux temps
distincts**, et cette séparation est le point le plus important de la page :

1. la **composition** (`hmi::composeWorldSprites` → `hmi::ComposedScene`) parcourt le `World`
   (@ref guide-ecs) par une **vue** sur les entités possédant à la fois `core::Transform` et
   `core::Sprite`, résout l'apparence, construit un `SpriteQuad` en unités monde et l'empile dans
   une liste ordonnée. C'est de la logique **pure** : aucun appel Direct3D ;
2. la **soumission** (`hmi::submitComposedScene`) parcourt cette liste et l'envoie au
   `SpriteBatch`, une passe `begin`/`end` par groupe contigu de même texture.

Pourquoi couper en deux ? Parce que la première moitié devient **testable sans GPU**
(`EX-NFR-004`) : `hmi::QuadRecorder` capture la liste composée et permet d'**asserter** l'ordre des
calques, le regroupement par texture ou l'effet du culling, là où il fallait auparavant regarder
l'écran et juger à l'œil. Un critère d'acceptation du type « le rendu n'a pas changé » cesse d'être
une impression pour devenir un test.

Un détail important : les sprites sont **triés par couche** (`Sprite::layer`, un entier — plus
grand = dessiné **au-dessus**) avant d'être soumis au `SpriteBatch`. Sans ce tri, l'ordre de dessin
suivrait l'ordre arbitraire d'itération de la vue ECS (@ref guide-ecs — le sparse set ne garantit
aucun ordre stable vis-à-vis du sens du jeu), et un élément de décor pourrait apparaître par-dessus
le personnage un pas sur deux.

Cet entier a longtemps eu **deux valeurs magiques** écrites en dur là où les entités sont créées
(`0` pour les tuiles, `100` pour le personnage) : suffisant, mais ce n'était pas un ordonnancement
— rien ne documentait ce que valaient `0` et `100`, ni où s'insérerait un fond ou un décor de
premier plan. Le `LOT-40` les a remplacées par `hmi::RenderLayer`, un jeu de calques **nommé** et
unique (`EX-REN-014`) :

    Background · Decor · Shadow · Tile · Object · Player · Foreground · UI · EditorOverlay

L'ordre de déclaration **est** l'ordre de dessin. Une entité porte son calque via le composant de
présentation `hmi::RenderLayerTag` ; en son absence elle est dessinée sur `Tile`, le cas de très
loin le plus fréquent. `core::Sprite::layer` conserve son rôle de tri **fin à l'intérieur** d'un
calque, et `Core` continue d'ignorer complètement l'existence des calques (`EX-NFR-011`) : c'est
une notion de présentation.

Le tri de la scène composée est donc : **calque**, puis **texture** (regroupement, dans l'ordre de
première apparition), puis `Sprite::layer`. Jamais l'inverse — regrouper par texture ne doit sous
aucun prétexte faire passer une primitive devant une primitive d'un calque inférieur. Le tri est
**stable**, ce qui garantit qu'à clé égale l'ordre de composition est préservé.

### Ne dessiner que ce qui se voit : le culling (`LOT-40`)

La composition écarte toute primitive dont la boîte englobante n'intersecte pas le cadrage de la
caméra (`hmi::Camera2D::visibleBounds`), élargi d'une **marge d'une case** pour qu'une entité à
cheval sur la frontière ne disparaisse pas prématurément. Le test porte sur la boîte englobante
**réelle** et non sur la position d'ancrage : un fond étiré sur tout le niveau reste soumis même si
son coin est hors champ. Le culling est purement visuel — une entité écartée continue d'être
simulée normalement (`EX-ARCH-012`). Les compteurs de l'image (composées, écartées, soumises,
passes) sont exposés par `hmi::ComposedScene::statistics` et journalisés quand ils changent.

### Deux modes de rendu : Physique et Texture (`LOT-41`)

`hmi::RenderMode` a deux valeurs. **Physique** est le rendu historique : une couleur plate par type
de tuile, qui donne la lecture directe de la géométrie de collision. **Texture** est l'habillage,
construit lot après lot à partir du `LOT-42` ; tant qu'aucun skin n'existe, il affiche
légitimement le damier magenta partout.

La touche **`F8`** bascule entre les deux, en édition, en essai et en jeu réel. Elle est traitée en
dur dans `hmi::GameViewport::keyPressEvent`, **hors** des tables de remappage — même parti pris que
`F10` pour la grille de repère : une bascule d'affichage n'est pas une action de gameplay. Le choix
est persisté (`QSettings`, `EX-IHM-011`) et le défaut est **Texture dans toutes les configurations
de build**, pour que deux binaires du même code ne puissent jamais afficher un rendu différent.

Le mode agit à un **point de résolution unique**, `hmi::resolveTileAppearance`, appelé à la
**composition** : basculer ne reconstruit donc jamais la scène ECS, ne coûte aucun pas de
simulation, et n'a aucun effet rémanent. C'est là que le `LOT-42` insérera la priorité
« surcharge par case > skin de tuile > damier ».

`SpriteRenderer` **lit** l'ECS mais ne le modifie **jamais** (`EX-ARCH-012`) — le rendu est un
simple observateur de l'état de simulation, jamais une source de vérité. Ce n'est délibérément
**pas** un `core::ISystem` exécuté par `World::update` : le rendu est **découplé** de la simulation
au pas fixe (`EX-REN-021`), cohérent avec la séparation décrite en @ref guide-boucle — la simulation
avance par pas fixes, discrets ; le rendu, lui, redessine l'état courant une fois par **frame**
réelle, qu'un pas fixe ait eu lieu ou non entre deux frames.

### Isoler un calque pour l'audit : `hmi::LayerVisibility` (`LOT-51`)

`F8` **compose** : il choisit une seule apparence par tuile (surcharge > skin > damier) pour
reproduire fidèlement ce que le joueur voit. L'éditeur a aussi besoin de l'inverse — **décomposer**,
pour répondre à « qu'est-ce qui est réellement configuré sur *ce* calque ? ». C'est le rôle de
`hmi::LayerVisibility` (section « Calques » du panneau Textures, @ref guide-editeur), un jeu de
booléens **indexé par la valeur de `hmi::RenderLayer`** plutôt que par une liste de champs écrite à
la main — un calque futur ne demande donc de grandir que `RENDER_LAYER_COUNT`, jamais de réécrire la
classe.

Deux mécanismes distincts, selon le calque :
- **Fond, Plans, Ombres, Personnage, Plans de premier plan** : un bit à `false` masque
  grossièrement — `hmi::composeWorldSprites`/`DraftRenderer::render` sautent l'entité ou l'appel de
  composition entier avant toute résolution d'apparence, aucune primitive n'est émise.
- **Skin des tuiles et Objets interactifs** : ces deux calques UI pilotent en réalité les **deux
  axes de résolution** d'une même entité « tuile » (toujours dessinée sur `RenderLayer::Tile`, l'ordre
  de dessin ne change jamais) — `RenderLayer::Tile` pour l'axe skin, `RenderLayer::Object` pour l'axe
  surcharge. Tant que les deux valent `true` (le défaut), `hmi::resolveTileAppearance` se comporte
  exactement comme au `LOT-45` : surcharge > skin > damier, sans repli différent. Dès qu'un seul des
  deux est masqué, la résolution **isole** — plus de repli sur le damier pour l'axe inactif : une
  case sans surcharge n'affiche rien quand seul l'axe surcharge est actif, un type sans skin
  n'affiche rien quand seul l'axe skin est actif. C'est le même résolveur, avec un indicateur
  « composer » ou « isoler », **jamais** un second résolveur parallèle qui risquerait de diverger.

Édition uniquement : `hmi::GameSession` ne fournit jamais de `hmi::LayerVisibility` à
`composeWorldSprites` (valeur par défaut, tout visible), donc le jeu réel et l'essai restent
strictement inchangés par ce mode. Aucune persistance entre deux sessions, contrairement à `F8`.

### Interpoler le mouvement : `hmi::PreviousPosition` et le facteur d'interpolation

Ce découplage crée un artefact visuel dès qu'un écran dépasse 60 Hz : entre deux pas de simulation,
la position d'une entité mobile ne change pas, si bien qu'elle reste **figée** plusieurs frames de
rendu puis « saute » d'un coup au pas suivant — un *judder* en marches d'escalier. La parade,
**prévue dès le départ** dans l'architecture (`EX-ARCH-031`) et concrétisée en `LOT-33`, est
l'**interpolation** : dessiner l'entité entre sa position du **pas précédent** et celle du **pas
courant**, selon la fraction de pas déjà écoulée.

Concrètement, un composant de présentation `hmi::PreviousPosition` (rangé dans le `core::World` mais
écrit et lu par `HMI` seul — `Core` l'ignore, sa frontière reste intacte) conserve la position de
l'entité au pas précédent. `hmi::GameSession` la recopie depuis le `core::Transform` au **début** de
chaque pas fixe (`snapshotPreviousPositions`), avant que le pas ne modifie la position ; seules les
entités réellement mobiles (personnage, dangers mobiles, blocs poussables) reçoivent ce composant.
Au rendu, `SpriteRenderer::render` reçoit le **facteur d'interpolation** `[0, 1[` du cadenceur
(`core::FixedTimestep::interpolationAlpha`, passé en paramètre par `hmi::GameSession::render`) et dessine chaque
entité portant le composant à `lerp(position précédente, position courante, alpha)` ; les tuiles
fixes, sans le composant, sont dessinées à leur position courante, inchangées. La caméra, en mode
*niveau entier* ou *par salle*, n'est **pas** interpolée : elle bascule par coupure nette entre
salles (`LOT-32`) ou reste fixe, sans suivi continu. En mode *suivi* (`LOT-64`, détaillé plus haut),
c'est l'inverse : c'est justement l'**absence** d'interpolation du centre de caméra qui produirait
un artefact, puisque ce mode fait bouger le centre à **chaque** pas fixe — voir « Le cadrage choisi
par le niveau » ci-dessus pour ce cas particulier. L'interpolation ne touche que l'**affichage** —
la logique de jeu (collisions, fin de niveau)
continue de lire les positions **simulées** exactes, le déterminisme est préservé (`EX-NFR-002`).

### Plans picturaux et parallaxe (`LOT-69`)

Le décor d'un niveau n'est pas un **assemblage d'images posées**, c'est une **surface peinte**. Un
**plan** (`core::Plane`) est un PNG couvrant le niveau **entier**, rangé à côté du niveau dans
`Levels/Plans/` et référencé par nom. Un niveau porte une **liste ordonnée** de plans, de nombre
libre — l'ordre décide de la superposition (`EX-DEC-040`).

> **Ce que cela remplace.** Le `LOT-49`/`LOT-50` habillait un niveau en **posant des sprites** :
> un décor était un PNG placé à une position, avec échelle et rotation, sur l'une de trois couches
> figées. Trois limites l'ont fait retirer — `hmi::decorRenderLayer` projetait déjà deux des trois
> couches sur le **même** calque de rendu (les trois n'exprimaient donc que deux intentions), la
> parallaxe était **codée en dur** par couche, et poser sept PNG ne permet toujours pas de
> *peindre* une fresque. Les exigences retirées sont conservées, texte intact, en fin de
> [`decors.md`](@ref spec-decors). Le prix explicite du remplacement : il n'existe plus de motif
> décoratif **ponctuel réutilisable** — un tonneau présent dans dix niveaux doit être peint dans
> chaque plan.

Un plan n'a **ni position ni rotation** : il couvre le niveau par construction. Ce qui le distingue
d'un autre, c'est sa **densité**, sa **parallaxe**, son **opacité** et sa **profondeur**.

#### Densité : un compromis de mémoire, jamais de cadrage

`pixelsPerUnit` vaut 16 (natif, la valeur de `hmi::Camera2D::PIXELS_PER_UNIT`), 8 ou 4. C'est le
seul réglage qui décide de la viabilité mémoire d'un niveau (`EX-DEC-041`) : un plan à 4 px/unité
coûte **seize fois moins** qu'un plan natif.

La densité **ne change pas la géométrie**. `hmi::composePlanes` émet un quad couvrant exactement
`[0,0]`–`[largeur, hauteur]` en unités monde, UV pleines, quelle que soit la densité : seule la
**texture** est plus petite, donc plus grossière à l'écran. Un plan lointain n'a pas besoin de la
définition native, et personne ne le verra.

Le PNG doit mesurer **exactement** `largeur × densité` par `hauteur × densité`. Une seule fonction
en décide — `hmi::planePixelSize` — parce que quatre endroits en dépendent (création du fichier,
changement de densité, contrôle du contenu livré, budget mémoire) et que quatre calculs séparés
finiraient par diverger. `Core` ne vérifie **pas** l'existence du fichier (`EX-NFR-011`) et
`hmi::resolvePlaneTextures` replie sur le damier magenta (`EX-NFR-040`) : un plan manquant reste
un niveau valide, simplement visible comme manquant.

#### Profondeur : deux valeurs, pas trois

`core::PlaneDepth` vaut `Behind` (derrière les tuiles physiques) ou `Front` (devant le personnage).
`hmi::planeRenderLayer` les projette sur `RenderLayer::Plane` et `RenderLayer::Foreground` — deux
calques **déjà réservés**, aucune valeur d'énumération ajoutée, `RENDER_LAYER_COUNT` inchangé alors
même que le nombre de plans est libre. C'est le **rang dans la liste** qui ordonne les plans à
l'intérieur d'un calque, jamais une valeur de calque par plan : `EX-REN-014` impose un
ordonnancement unique, et en créer un concurrent serait le défaire.

Une subtilité que seul un test peut figer : `hmi::ComposedScene::sort()` trie par calque, puis par
**rang de première apparition de texture**, puis par `sortOrder`. Chaque plan portant sa propre
image, ce sont donc les rangs de texture qui les départagent — et la propriété « les plans
ressortent dans l'ordre du niveau » n'est vraie que parce qu'ils sont composés **en premier et dans
l'ordre**. Invisible à la lecture, d'où le test.

#### Parallaxe : portée par le plan, bornée, et seulement là où la caméra défile

Chaque plan porte un facteur de défilement **par axe** (`EX-DEC-043`), et le niveau décide **si** la
parallaxe s'applique (champ `parallax`, `true` par défaut). Le décalage est **purement visuel**
(`EX-ARCH-012`) : la simulation ne le voit jamais.

Trois étapes, dans cet ordre — et l'ordre compte :

1. **Décalage relatif au centre de la salle** (`hmi::parallaxRenderPosition`), jamais absolu en
   espace niveau. C'est précisément cette formule qui empêche le saut visible à la bascule de salle
   (`EX-REN-015`) : à chaque salle, le décalage repart de zéro au centre.
2. **Bornage** (`hmi::clampPlaneOffset`), sans lequel le bord du plan se découvrirait en fin de
   course et laisserait une bande vide.
3. **Arrondi au pixel écran** (`hmi::roundToScreenPixel`), sans lequel le pixel art tremblerait.

Inverser 2 et 3 laisserait un décalage borné fractionnaire : l'arrondi doit venir en **dernier**.

**La parallaxe est réactivée en mode suivi**, ce qui **inverse** la décision du `LOT-64`. Celui-ci
l'avait coupée parce qu'un *décor* est un objet collé au contenu et paraissait « suivre » la
caméra ; un *plan* est un fond, l'argument ne tient plus. Le mode suivi est d'ailleurs le seul où
la caméra défile en continu — donc le seul où la parallaxe se voit vraiment.

Corollaire à connaître : en cadrage **niveau entier**, la caméra ne défile pas du tout. Le moteur y
neutralise la parallaxe (`hmi::planeParallaxActive`) et l'éditeur **grise** la case en l'expliquant
— un facteur y produirait un désalignement constant, pas du mouvement.

**Convention à trois profondeurs (`LOT-70`).** Sur les deux seuls tableaux livrés où la parallaxe
est active — `demo-mouvement` (suivi) et `demo-final` (par salle) — la pile de plans suit une
convention à trois profondeurs, facteurs strictement croissants : un plan **lointain** (densité 4,
facteur le plus lent, presque immobile), le plan **fond** hérité du `LOT-69` (densité 8, facteur
intermédiaire), et le plan **devant** (densité native, facteur supérieur à 1 — il dépasse la
caméra). C'est cette progression, pas un seul plan supplémentaire, qui rend la profondeur lisible ;
`scripts/generate_demo_plans.py` la peint pour ces deux tableaux uniquement, les vingt autres restant
le report fidèle du `LOT-69` puisque leur cadrage `WholeLevel` neutralise tout décalage.

#### Coût : constant en taille de niveau, et c'est tout le problème

Un plan coûte **un quad et une passe** par image, quelles que soient les dimensions du niveau —
contrairement aux tuiles. Un plafond exprimé en primitives ne le verrait donc **jamais** grossir,
alors qu'un niveau 200 × 100 à densité native coûterait 20 Mo de texture **par plan**. D'où le
second axe du budget (`EX-NFR-043`, `SceneStatistics::textureBytes`) — voir « Budget de rendu
mesuré » plus bas.

Le coût dominant n'est d'ailleurs pas la mémoire mais le **batch** : chaque plan est une texture
distincte, donc une passe de plus par image, **jamais** écartée par le culling (un plan couvre le
niveau, il est toujours à l'écran).

#### Peindre un plan : le mode création

Peindre se fait dans le **troisième espace de travail** de l'éditeur, pas dans le viewport : le
canevas est `hmi::PixelCanvas`, celui de l'atelier pixel art du `LOT-54`, qui sait déjà peindre,
sélectionner, annuler, coller et contraindre à une palette. Détails dans
[`guide-editeur.md`](@ref guide-editeur) et [`guide-atelier-pixel-art.md`](@ref guide-atelier-pixel-art).

Deux propriétés méritent d'être connues ici :

- **Le CPU reste l'autorité sur les pixels ; le GPU n'est que l'affichage.** Une `DecodedImage` par
  plan est la source de vérité, et seuls les rectangles salis sont téléversés. Peindre dans une
  texture hors écran aurait exigé une **relecture GPU par geste**, et rendu `hmi::PixelOperations`
  intestable sans carte graphique (`EX-NFR-004`).
- **La référence est un repère géométrique, pas un aperçu.** Sous l'image éditée, une pelure
  d'oignon des tuiles (`hmi::buildTileOnionSkin`, qui réutilise la palette du mode Physique) et les
  plans voisins aplatis ; au-dessus, ceux qui passent devant. Ni raccords automatiques, ni skins, ni
  animations : elle dit *où* sont les choses, pas *à quoi elles ressembleront*. L'aperçu fidèle
  reste l'essai.

#### Les plans du contenu livré

Les vingt-deux tableaux de la séquence démo portent chacun un fond à densité 8, et ceux qui avaient
un décor de premier plan un second plan à densité native. Ces images sont **générées**, jamais
dessinées à la main :

```
python scripts/generate_demo_plans.py
```

Le script peint un fond dérivé du thème de chaque niveau puis y **reporte** ses anciens décors à
leurs positions d'origine, avec la même géométrie que l'ancien rendu. Choix de reproductibilité
(`LOT-66`), avec une limite assumée : c'est un report fidèle de l'ancien habillage, pas un décor
peint qui exploiterait vraiment la profondeur — ce dernier est un acte de *level design*.

## Le texte dans la scène : `hmi::BitmapFont` et `hmi::TextRenderer` (`LOT-52`)

Le texte de l'interface **hors-jeu** (menus, libellés, options) ne se dessine toujours **pas** avec
ce pipeline : c'est une préoccupation entièrement différente, portée par les **widgets Qt** de
l'IHM (@ref guide-ihm-qt), dans une couche indépendante de la caméra du monde. Mais depuis `LOT-52`,
le pipeline de rendu sait de nouveau afficher du texte **dans la scène de jeu** — l'ancienne police
bitmap « maison », retirée avec l'IHM « maison » au `LOT-38`, est réintroduite du bon côté de la
frontière (`SpriteBatch`, pas Qt) et rebranchée sur les fondations de `LOT-40` (calque `UI`,
`TextureCache`, contrat d'asset) plutôt que sur son ancien chemin.

**`hmi::BitmapFont`** essaie de charger `Assets/Fonts/font.png` accompagné de ses métriques
(`Assets/Fonts/font.json` : la région et l'avance de chaque glyphe, format JSON versionné comme
`hmi::AnimationCatalog`), validées par le contrat d'asset (`AssetFamily::Font`) puis par leur
cohérence avec les dimensions décodées du PNG. Aucun asset n'est livré pour l'instant
(`Source/Elements/Assets/Fonts/README.md`) : la police retombe donc, comme `hmi::TextureAtlas` sans
`atlas.png`, sur un repli **procédural** déterministe (`hmi::buildProceduralFont`, glyphes 5×7
pixels blancs sur fond transparent, ASCII imprimable et accents français `é è à ç ù ê î ô û`) — le
jeu reste lisible sans aucun asset de police (`EX-NFR-040`). Comme `TextureAtlas`, `BitmapFont`
possède sa **propre** texture GPU (`QRhiTexture`, pas de passage par `TextureCache` : elle n'est chargée
qu'une fois au démarrage, sans rechargement à chaud). Un point de code non couvert est substitué
par un glyphe de remplacement (`?` par défaut), jamais un trou silencieux. La mesure d'une chaîne
(`hmi::measureText`) est **pure** : elle ne dépend que des métriques, pas du GPU, ce qui permet de
cadrer un texte sans le dessiner ; elle parcourt des **points de code** UTF-8, pas des octets — un
caractère accentué du catalogue de traduction (`EX-REN-033`) en occupe plusieurs.

**`hmi::composeText`** (`HMI/Graphics/TextRenderer.h`) compose une chaîne en `SpriteQuad`, un par
glyphe, sur `RenderLayer::UI` — le calque réservé sans être utilisé depuis `LOT-40`. C'est le
**premier** cas du projet où une passe de rendu a sa propre projection : `hmi::screenProjectionMatrix`
construit une projection écran → clip à partir des seules dimensions du viewport, indépendante de
`Camera2D`, pour que le texte ne tourne ni ne change de taille avec le zoom de la caméra du monde.
Un ancrage (`hmi::TextAnchor`, gauche/centre/droite × haut/milieu/bas) évite d'avoir à mesurer le
texte soi-même pour le centrer ; les positions sont arrondies au pixel écran entier, la police
restant en filtrage *nearest* comme le reste du rendu (`EX-ARCH-022`).

Le texte, en espace écran, n'a **pas** de position monde : il ne doit jamais être soumis au culling
par cadrage caméra (`LOT-40` TACHE-05). `hmi::GameSession::renderHud` compose donc le HUD dans une
`hmi::ComposedScene` **dédiée**, distincte de celle de `hmi::SpriteRenderer` et sur laquelle
`setVisibleBounds` n'est jamais appelé — plutôt que d'étendre `hmi::submitComposedScene` à deux
projections, une seconde passe `begin`/`end` complète (même `SpriteBatch`, projection écran) suit
la passe monde de la même frame.

**`hmi::gameHudLines`** (`Source/HMI/Game/GameHud.h`) choisit, en fonction **pure**, les lignes à
afficher : les budgets de sauts et de dashs (`EX-GP-024`, `LOT-12`) — jusqu'ici invisibles, faute de
tout rendu de texte, malgré leur existence dans `core::Player` depuis ce lot —, seulement si le
budget du niveau est **fini** (`-1` = illimité, cas de la grande majorité des tableaux : aucune
ligne superflue), puis le nom du tableau. Affiché en jeu et en essai (parce que `renderHud` est
appelé depuis le point d'entrée unique `hmi::GameSession::render`, jamais depuis `hmi::
DraftRenderer`, seul chemin de l'éditeur en édition pure), avec une ombre portée (décalage d'un
pixel) pour rester lisible sur fond clair comme sur fond sombre.

## Ombres du plan physique (`LOT-55`)

Dernier calque du programme d'habillage à s'activer : `RenderLayer::Shadow`, réservé sans être
utilisé depuis `LOT-40`, entre `Decor` et `Tile` dans l'empilement — sous les tuiles, au-dessus du
fond et des décors d'arrière-plan. L'objectif est de **lecture**, pas d'esthétique : aider le
niveau designer, et en `RenderMode::Texture` le joueur, à distinguer d'un coup d'œil ce qui est
**physique** (solide, collidable) de ce qui est **décor** — le complément exact du calque de
premier plan (`LOT-49`) : l'un dit « ceci passe devant vous, donc ne vous porte pas », l'autre
« ceci est en relief, donc vous porte ».

`hmi::composeShadows` (`HMI/Graphics/ShadowRenderer.h`) parcourt les mêmes entités que
`hmi::composeWorldSprites` (`core::Transform` + `hmi::TileSkinTag`), et n'en retient que celles qui
projettent une ombre : pleines (`core::isSolid`) ou à silhouette inclinée/courbe (`hmi::
hasSilhouette`, `LOT-42`). La région échantillonnée est directement `hmi::regionForTile(type)` — le
**même** atlas procédural que `RenderMode::Physique` et que le détourage de silhouette des skins :
cette région est déjà opaque exactement là où la matière est présente et transparente ailleurs
(`hmi::isInsideSilhouette`), donc teinter le quad en noir semi-transparent, décalé d'un pixel, donne
l'ombre à sa forme réelle — pente, arrondi, ou bloc réduit (`core::tileVisualScale`, porté par
`core::Transform::scale` comme pour le sprite de la tuile) — sans réimplémenter la géométrie ni
ajouter le moindre nouveau prédicat de solidité dans `Core` : une ombre est la projection d'une
**forme**, pas d'un degré de solidité, et cette forme est déjà exposée côté `Core` sous une forme
plus riche qu'un booléen. Un bloc poussable en mouvement voit son ombre suivre automatiquement, par
la même interpolation (`hmi::PreviousPosition`) que son propre sprite — jamais recalculée à part.

Une porte fait exception à la règle « ombre = type statique » : `hmi::TileSkinTag::type` reste
`TileType::Door` quel que soit l'état du mécanisme (figé au chargement), alors que sa solidité
**réelle** dépend de l'interrupteur qui la commande. `hmi::composeShadows` accepte donc une grille
de collision optionnelle (`core::MechanismController::collisionMap()`, fournie par `hmi::
GameSession`) pour trancher l'ombre d'une porte sur son état **courant** plutôt que sur son type
figé — une porte fermée projette une ombre, une porte ouverte n'en projette plus.
`hmi::DraftRenderer` (aucune simulation de mécanisme dans l'éditeur) ne fournit pas cette grille :
une porte n'y projette jamais d'ombre, état normal plutôt qu'un défaut.

Actif **uniquement** en `RenderMode::Texture` (`RenderMode::Physique` reste la lecture nue des
collisions, déjà sans ambiguïté par la couleur plate) et sans le moindre effet sur le gameplay
(`EX-ARCH-012`) : les ombres passent par le même culling que le reste (`hmi::ComposedScene::
addSprite`), et l'axe `Shadow` de `hmi::LayerVisibility` (`LOT-51`) les masque grossièrement dans
l'éditeur, comme `Background`/`Decor`/`Foreground`.

## Particules et secousse d'écran (`LOT-53`)

Une fois le personnage et le décor texturés (`LOT-48`, `LOT-49`), l'absence d'effets devient la
principale différence entre ce rendu et celui d'un jeu fini : un dash ne se distingue d'une course
que par la vitesse, un atterrissage après une longue chute est identique à un pas. `LOT-53` ajoute
un retour visuel bref à quatre transitions déjà exposées par `core::Player` : dash, atterrissage,
mort — sans jamais toucher au gameplay (`EX-ARCH-012`).

### L'émetteur, dans `Core`, déterministe (`core::ParticleSystem`)

Une simulation de particules est l'endroit classique où l'on est tenté d'utiliser l'horloge système
et un générateur aléatoire non maîtrisé — c'est plus simple à écrire, et « ce n'est que du visuel ».
Ce serait ici une régression : le projet tient le déterminisme au pas fixe depuis `LOT-01`
(`EX-NFR-002`). `core::ParticleSystem` simule donc les particules (`core::Particle` : position,
vitesse, durée de vie) comme des entités `core::World` ordinaires, au pas fixe, et tire tout son
aléa (vitesse, angle de dispersion, durée de vie) d'un `core::DeterministicRandom` **reseedé pour
chaque particule** à partir d'un triplet reproductible — graine de base, numéro de pas, identifiant
de l'entité (`core::deriveSeed`) — jamais l'horloge. Deux exécutions de la même séquence d'entrées
produisent ainsi exactement les mêmes particules.

Le nombre de particules vivantes est borné (`core::MAX_PARTICLES`, `EX-NFR-005`) : au-delà, la plus
**ancienne** est recyclée — mais jamais en s'appuyant sur l'ordre d'itération du sparse set de
l'ECS (instable après un retrait, `core::ComponentPool`) : `ParticleSystem` tient sa propre file
d'émission (FIFO) comme seule source de vérité pour l'intégration et le recyclage.

### Les déclencheurs, câblés dans `hmi::GameSession`

`core::ParticleSystem::emitDashTrail`/`emitLanding`/`emitDeath` sont des **émissions**, pas des
**détections** : la détection des transitions du personnage réutilise `hmi::detectPlayerEvents`
(`LOT-60`) déjà calculée par `hmi::GameSession`, sans la dupliquer une troisième fois dans le
projet (après `LOT-47` et `LOT-60`). La traînée de dash est une exception : elle s'émet à **chaque**
pas où `core::Player::dashTimer > 0` (émission continue, pas un événement ponctuel), tandis que la
poussière à l'atterrissage voit son intensité (nombre de particules) croître avec la vitesse
d'impact, nulle en dessous d'un seuil nommé (`core::LANDING_MIN_IMPACT_SPEED`) — un petit saut ne
soulève pas de poussière.

### Le rendu (`hmi::ParticleRenderer`) et la secousse d'écran (`hmi::Camera2D`)

`hmi::composeParticles` (`HMI/Graphics/ParticleRenderer.h`), appelé par `hmi::SpriteRenderer::render`
comme `hmi::composeShadows`, dessine un quad par particule vivante — un simple carré teinté
(région opaque unie de l'atlas, `hmi::TextureAtlas::tile(0, 0)`, pas d'asset dédié), dont l'opacité
suit `life / maxLife` (fondu en fin de vie). Le calque dépend de l'effet : la traînée de dash passe
sur `RenderLayer::Object` (**derrière** le personnage), la poussière et l'éclat de mort sur
`RenderLayer::Foreground` (**devant**) — actif uniquement en `RenderMode::Texture`, comme les
ombres.

La secousse d'écran (atterrissage **lourd**, mort) est le seul effet du lot qui n'est pas une
particule : `hmi::ScreenShakeState` décroît linéairement vers zéro sur une durée brève et
volontairement conservatrice (`hmi::SCREEN_SHAKE_DURATION`), et son décalage courant
(`hmi::screenShakeOffset`, arrondi au pixel écran entier, `EX-ARCH-022`) n'est appliqué qu'à
`Camera2D::projectionMatrix` via `setShakeOffsetPixels` — **jamais** à `Camera2D::_center`. C'est
cette séparation qui garantit, par construction, que la secousse ne peut ni provoquer de bascule de
salle (`updateCurrentRoom`, pilotée par la position du personnage, jamais par la caméra) ni fausser
le culling (`visibleBounds`, dérivé du seul `_center`).

## Assembler la frame complète

Dans le viewport (`hmi::GameViewport::renderFrame`), l'ordre d'une frame de rendu est :
`graphics.clear(...)` (vider le back buffer) → la scène courante dessine — `hmi::GameSession::render`
en jeu, `hmi::DraftRenderer` en édition, typiquement un ou plusieurs passages
`SpriteBatch::begin`/`draw`/`end` avec la projection de `Camera2D` → `graphics.present()` (échanger
les buffers). C'est la même boucle que celle décrite en @ref guide-boucle, dont le rendu n'est qu'une
étape — toujours exécutée **une fois par frame réelle**, après que tous les pas de simulation fixes
de cette frame ont eu lieu.

## Le programme d'habillage, livré (`LOT-40` → `LOT-55`)

Le pipeline d'origine était volontairement minimal : **une** texture liée par lot de dessin, deux
valeurs de couche, aucun culling, et une seule façon de représenter l'état d'un objet — la teinte.
Cela suffisait au rendu en couleurs plates ; cela ne suffit plus dès qu'on veut de vraies textures.
Un programme de seize lots (voir [les lots](@ref lots)) a levé ces limites une à une ; `LOT-55`
(ombres du plan physique, décrites plus haut) en est le dernier :

- **`LOT-40`** — registre de textures par nom logique, calques nommés, regroupement des quads par
  `(calque, texture)`, validation des dimensions d'asset, culling, capture des primitives (décrits
  plus haut dans cette page).
- **`LOT-41`** — la bascule `F8` entre rendu **Physique** et rendu **Texture** (décrite plus haut
  dans cette page).
- **`LOT-42`** — le skin des tuiles (`hmi::TileSkinTag`, `hmi::resolveTileAppearance`) et les
  raccords automatiques entre tuiles voisines : le lot à partir duquel le mode Texture cesse
  d'afficher le damier magenta pour un type habillé.
- **`LOT-43`** — la bibliothèque d'assets à vignettes de l'éditeur (gestion de fichiers,
  rechargement à chaud des textures), hors du pipeline de rendu lui-même (@ref guide-editeur).
- **`LOT-44`** — le fond de niveau (`hmi::composeBackground`, `RenderLayer::Background`),
  recadré en mode *cover* sur les bornes du niveau, en `RenderMode::Texture` uniquement.
- **`LOT-45`** — la texture assignée **par instance** à une case (`EX-EDIT-043`), prioritaire sur
  le skin de son type dans `hmi::resolveTileAppearance`.
- **`LOT-46`** — les animations décrites par des **données** et non par un `enum` figé, applicables
  à toute entité et plus seulement au personnage (décrit plus haut dans cette page).
- **`LOT-47`** — l'apparence des mécanismes (porte, interrupteur, dangers commutés/temporisés)
  pilotée par leur état logique plutôt que par une simple modulation d'opacité de diagnostic.
- **`LOT-48`** — le personnage habillé depuis une spritesheet externe, découplée de sa hitbox
  (décrit plus haut dans cette page).
- **`LOT-49`** — des décors libres hors grille sur trois couches, dont une **au-dessus** du
  personnage, et leur parallaxe relative à la salle courante (décrits plus haut dans cette page).
- **`LOT-50`** — manipulation complète des décors dans l'éditeur — sélectionner, déplacer,
  redimensionner, pivoter, changer de couche, réordonner (décrit plus haut dans cette page).
- **`LOT-51`** — le mode d'inspection « définition des textures » — visibilité et isolement par
  calque, distinct de `F8` (décrit plus haut dans cette page).
- **`LOT-52`** — le retour du texte dans la scène rendue — police bitmap avec repli procédural,
  composition sur le calque `UI` et sa propre projection écran, affichage tête haute des budgets de
  sauts/dashs et du tableau courant (décrits plus haut dans cette page).
- **`LOT-54`** — l'atelier de pixel art intégré à l'éditeur (dessin, palette, aperçu de raccords),
  qui produit les assets consommés par ce pipeline sans le modifier lui-même
  (@ref guide-atelier-pixel-art).
- **`LOT-55`** — les ombres du plan physique (`hmi::composeShadows`, `RenderLayer::Shadow`,
  décrites plus haut dans cette page).

`LOT-53` (effets et particules, décrit plus haut dans cette page) n'appartient pas à ce programme
d'habillage : indépendant de `LOT-55` (aucun des deux ne dépend de l'autre), c'est un effort
distinct qui **bâtit sur** le personnage et le décor texturés plutôt que d'en faire partie.

## Budget de rendu mesuré (`LOT-62`)

`EX-NFR-005` demande que le nombre de primitives émises par image reste **borné et observable** ;
`EX-NFR-001` demande **60 images par seconde**. Les deux exigences existaient depuis les premiers
lots du rendu, sans jamais avoir de moyen de vérification. `LOT-62` leur en donne un — sans
optimiser quoi que ce soit : ce lot **mesure**.

### Le test de non-régression du volume (`Source/Test/Unit/HMI/Graphics/test_render_budget.cpp`)

Pour chaque niveau livré (les quinze fichiers de `Source/Elements/Levels/sequence-demo.json`), le
test reconstruit exactement la scène que `hmi::GameSession` composerait — tuiles, plans,
personnage à l'entrée, caméra cadrée sur la salle de l'entrée — et compare les compteurs de
`hmi::ComposedScene::statistics()` à un **plafond nommé**, dans les deux modes de rendu (le mode
Texture, structurellement plus lourd, est celui qui dérive). Le culling est asserté séparément sur
`demo-salles` (au moins la moitié des primitives écartées) : une borne haute sur le total ne dit pas
si le culling fonctionne, une borne basse sur ce qu'il écarte, si.

#### Le second axe : la mémoire de texture des plans (`LOT-69`)

Un plafond de **primitives** ne voit pas grossir un plan pictural : celui-ci n'en ajoute qu'une, mais
occupe une texture à l'échelle du niveau. Sur le plus grand tableau du dépôt (50 × 26), un plan natif
pèse 1,27 Mio ; sur un niveau 200 × 100, il pèserait **20 Mio** — et seize plans, plus de 300 Mio.

Le test plafonne donc aussi `hmi::planesTextureMemoryBytes` par niveau livré (`EX-NFR-043`), à
16 Mio. Ce plafond-là n'est **pas** décliné par niveau comme celui des primitives : le volume de
primitives dépend du contenu posé par l'auteur, la mémoire de plans ne dépend que de la taille du
niveau et des densités déclarées — deux réglages, pas du contenu.

16 Mio laisse passer **douze** plans natifs sur le plus grand tableau, là où le format en autorise
seize (`core::MAX_PLANES_PER_LEVEL`) : le plafond refuse donc **avant** la limite de format, ce qui
est le seul moyen qu'il refuse quoi que ce soit un jour. Un test le vérifie dans ce sens-là — un
garde-fou qu'on n'a jamais vu refuser ne prouve rien.

Un troisième test fige la propriété qui distingue les plans des tuiles : leur coût en primitives est
**invariant en taille de niveau** (doubler chaque axe quadruple la mémoire, mais ne change ni le
nombre de primitives, ni le nombre de passes).

**Faire évoluer un plafond légitimement** : un lot de contenu qui ajoute un calque, agrandit un
niveau livré, ou change sa salle d'entrée peut légitimement faire grimper les compteurs mesurés.
Dans ce cas, relever les nouvelles valeurs (`ctest` affiche `considered`/`submitted` réels dans le
message d'échec, ventilés par calque via `hmi::QuadRecorder::describe`) et choisir un plafond
**large** au-dessus — de l'ordre de 1,5 à 2 fois la valeur mesurée, pour continuer à attraper un
facteur deux accidentel sans transformer chaque lot de contenu en mise à jour de constante.
**Ajuster un plafond pour faire passer un test sans avoir compris pourquoi il a été dépassé est
exactement ce qu'il ne faut pas faire** : un dépassement est un résultat, il se consigne (voir
tableau de référence ci-dessous) avant de se corriger, jamais pendant.

### Mesures de référence, à la date du `LOT-62` (2026-08-12)

Composées avec `test_render_budget.cpp`, caméra cadrée sur la salle d'entrée, sans skins chargés
(tout retombe sur l'atlas procédural en Physique ou le damier en Texture — sans effet sur le
*volume*, seule chose mesurée ici) :

| Niveau                     | Physique (composées / soumises) | Texture (composées / soumises) |
|-----------------------------|:-------------------------------:|:-------------------------------:|
| `demo-deplacement.json`     | 48 / 48                         | 90 / 90                         |
| `demo-saut.json`             | 44 / 44                         | 85 / 85                         |
| `demo-double-saut.json`      | 39 / 39                         | 71 / 71                         |
| `demo-wall-jump.json`        | 25 / 25                         | 47 / 47                         |
| `demo-dash.json`              | 43 / 43                         | 82 / 82                         |
| `demo-interrupteur.json`      | 33 / 33                         | 61 / 61                         |
| `demo-plaque-pression.json`   | 34 / 34                         | 63 / 63                         |
| `demo-bloc.json`               | 40 / 40                         | 73 / 73                         |
| `demo-budget.json`             | 31 / 31                         | 59 / 59                         |
| `demo-pente.json`              | 31 / 31                         | 59 / 59                         |
| `demo-arrondi.json`            | 31 / 31                         | 59 / 59                         |
| `demo-bloc-reduit.json`        | 34 / 34                         | 65 / 65                         |
| `demo-dangers-avances.json`    | 52 / 52                         | 96 / 96                         |
| `demo-final.json`              | 68 / 46                         | 128 / 89                        |
| `demo-salles.json`             | 241 / 86                        | 479 / 170                       |

`demo-final` et `demo-salles` sont les deux seuls niveaux livrés dépassant une salle
(`hmi::RoomGrid::ROOM_WIDTH_TILES` × `ROOM_HEIGHT_TILES`) : c'est là que le culling écarte une
fraction significative des primitives composées (respectivement 32 % et 64 % en mode Texture).
Chaque autre niveau livré tient dans sa salle d'entrée sans reste : aucune primitive n'y est jamais
écartée.

### Compteur de diagnostic en jeu (`F9`)

`hmi::DiagnosticsHud` (`Source/HMI/Game/DiagnosticsHud.{h,cpp}`) compose quatre lignes — cadence de
rendu (moyenne glissante sur `DIAGNOSTICS_FPS_WINDOW_SECONDS`, jamais une cadence instantanée,
illisible d'une image à l'autre), primitives composées/soumises, passes de dessin, et pas de
simulation consommés à la dernière image (une boucle qui rattrape s'y voit immédiatement) — sur le
patron de `hmi::gameHudLines` (`LOT-52`) : composition **pure**, testée sans rendu
(`Source/Test/Unit/HMI/Game/test_diagnostics_hud.cpp`), dessinée par le même `hmi::TextRenderer` que
le HUD de jeu, coin haut-**droit** pour ne jamais recouvrir les budgets de sauts/dashs (coin
haut-gauche). Activé par **`F9`**, touche dédiée non remappable comme `F8` (bascule de rendu),
désactivée par défaut et sans coût quand elle l'est (rien n'est mesuré tant qu'elle n'est pas
activée).

`EX-NFR-001` (60 images par seconde) reste **hors de portée d'un contrôle automatique** : la
cadence dépend de la machine, une machine virtuelle partagée ne la mesure pas de façon reproductible
(cf. `epic.md`, décisions de cadrage). `F9` sur `demo-salles` — le niveau livré le plus lourd,
480 primitives composées en mode Texture avant culling — est le moyen de l'observer soi-même sur sa
propre machine de développement ; c'est tout ce que ce lot automatise pour elle.

## Voir aussi
- `hmi::SpriteBatch`, `hmi::GameViewport`, `hmi::Camera2D`.
- `hmi::SpriteBatch`, `hmi::SpriteQuad`, `hmi::LineQuad`, `hmi::TextureAtlas`, `hmi::SpriteRenderer`,
  `hmi::DraftRenderer`.
- `hmi::RenderLayer`, `hmi::RenderLayerTag`, `hmi::ComposedScene`, `hmi::QuadRecorder`,
  `hmi::TextureCache`, `hmi::validateAsset`, `hmi::buildMissingTextureImage` — fondations du rendu
  texturé (`LOT-40`, `EX-REN-043`/`EX-REN-007`/`EX-NFR-004`/`EX-NFR-005`).
- `hmi::LayerVisibility`, `hmi::resolveTileAppearance` — visibilité et isolement par calque, mode
  d'inspection éditeur distinct de `F8` (`LOT-51`, `EX-EDIT-044`).
- `hmi::RenderMode`, `hmi::resolveTileAppearance` — bascule Physique/Texture (`LOT-41`,
  `EX-REN-046`).
- `hmi::AssetPaths`, `hmi::TextureLoader` (`decodeImageFile`, `createTexture`,
  `loadTextureFromFile`), `hmi::buildProceduralAtlasImage` — pipeline de textures depuis fichiers et
  repli procédural (`LOT-39`, `EX-REN-041`/`EX-REN-042`).
- `hmi::LinkGeometry`, `hmi::LinkGesture`, `hmi::LinkPanel` — liens de mécanismes (`LOT-37`, voir
  @ref guide-editeur).
- `core::ParticleSystem`, `core::Particle`, `core::DeterministicRandom`, `hmi::ParticleRenderer`,
  `hmi::ScreenShakeState` — particules et secousse d'écran (`LOT-53`, `EX-REN-008`).
- `core::Plane`, `core::PlaneDepth`, `hmi::planeRenderLayer`, `hmi::resolvePlaneTextures`,
  `hmi::composePlanes`, `hmi::planeParallaxActive`, `hmi::parallaxRenderPosition`,
  `hmi::clampPlaneOffset`, `hmi::roundToScreenPixel`, `hmi::planesTextureMemoryBytes` — plans
  picturaux, parallaxe et budget de mémoire (`LOT-69`,
  `EX-DEC-040`/`EX-DEC-043`/`EX-REN-049`/`EX-NFR-043`).
- `hmi::buildTileOnionSkin`, `hmi::flattenPlanes`, `hmi::resamplePlane`, `hmi::PlaneVisibility`,
  `hmi::planePixelSize`, `hmi::uniquePlaneFileName` — repères et cycle de vie du mode création
  (`LOT-69`, `EX-EDIT-046`/`EX-EDIT-047`).
- `hmi::BitmapFont`, `hmi::ProceduralFont`, `hmi::buildProceduralFont`, `hmi::measureText`,
  `hmi::composeText`, `hmi::screenProjectionMatrix`, `hmi::gameHudLines`,
  `hmi::GameSession::renderHud` — texte dans la scène et affichage tête haute (`LOT-52`,
  `EX-IHM-003`/`EX-REN-032`).
- `hmi::composeShadows`, `hmi::SHADOW_OFFSET_X`/`SHADOW_OFFSET_Y`, `hmi::SHADOW_OPACITY` — ombres du
  plan physique (`LOT-55`, `EX-REN-045`).
- `core::Transform`, `core::Sprite`, `core::AtlasRegion`, `core::Color` — les composants lus par le rendu.
- @ref guide-ecs — le `World` et les vues que `SpriteRenderer` parcourt.
- @ref guide-boucle — où le rendu s'insère dans la boucle de jeu.
- @ref guide-maths — les unités monde converties en pixels par la caméra.
