# Rendu & cible technique {#spec-rendu-technique}

> Statut : **livré** (`0.1.0`). Pipeline Direct3D 11 (quads/atlas/caméra 2D), rendu de texte,
> boucle à pas fixe, budget de rendu mesuré (`LOT-62`) et bruitages (`LOT-60`, Sec. 5) tous en
> place. Dépend de [`vision.md`](vision.md).

## 1. Cible technique
- \anchor EX-REN-001 **EX-REN-001** — Le jeu doit fonctionner sous **Windows 10/11 (x64)**.
- \anchor EX-REN-002 **EX-REN-002** — Le rendu doit s'appuyer sur **Direct3D 11** (bon compromis simplicité/capacités pour de la 2D ; Direct3D 12 écarté car surdimensionné au MVP). Depuis le `LOT-69`, l'API n'est plus appelée directement mais **au travers de QRhi** (`EX-REN-050`), qui retient Direct3D 11 par défaut sous Windows : la cible technique est inchangée, seule la couche d'accès l'est.
- \anchor EX-REN-003 **EX-REN-003** — La fenêtre doit être créée via l'API Win32, redimensionnable, avec titre et icône.

## 2. Rendu 2D
- \anchor EX-REN-010 **EX-REN-010** — Le rendu doit afficher une grille de tuiles à partir d'un **atlas de textures** (spritesheet).
- \anchor EX-REN-011 **EX-REN-011** — Le rendu doit afficher des **sprites** pour le personnage et les mécanismes, avec transparence.
- \anchor EX-REN-012 **EX-REN-012** — Le rendu doit supporter des **animations** par séquence d'images (personnage : repos, course, saut). Généralisé à toute entité — tuiles et mécanismes compris — par `EX-REN-005`.
- \anchor EX-REN-013 **EX-REN-013** — Une **caméra 2D** doit cadrer le niveau en jeu : elle reste
  bornée aux limites de ce qu'elle cadre, et pour un contenu plus grand que la fenêtre, elle
  **zoome pour l'englober entièrement** plutôt que de suivre le personnage — aucune zone ne doit
  rester invisible. Pour un niveau qui **tient dans une seule salle** (`RoomGrid`, `EX-REN-015`),
  ce contenu est le **niveau entier** (précisé en LOT-16 ; la formulation initiale « suivre le
  personnage » ne correspondait déjà plus à l'implémentation, une caméra fixe cadrant le tableau
  depuis LOT-08) ; au-delà d'une salle, `EX-REN-015` prend le relais et ce contenu devient la
  **salle courante**.
- \anchor EX-REN-015 **EX-REN-015** — En **mode par salle** (`EX-REN-016`), pour un niveau plus
  grand qu'une **salle** (taille par défaut ou réglée par le niveau, `LOT-64`), la caméra ne cadre
  plus le niveau entier mais la **salle** contenant le personnage, au zoom pixel art natif
  (`EX-REN-013`, appliqué au rectangle de la salle plutôt qu'au niveau) : elle bascule **nettement**
  sur la salle voisine dès que le personnage en franchit la frontière, sans jamais suivre le
  personnage en continu ni rapetisser le rendu quelle que soit la taille totale du niveau — la
  décision de conception d'origine (`LOT-32`), que le mode **suivi** (`EX-REN-016`) complète sans la
  remettre en cause, pour les tableaux qui veulent au contraire un cadrage continu. Le niveau reste
  une **grille de tuiles unique** (aucun format, aucune nouvelle tuile) : une salle a « plusieurs
  entrées/sorties » simplement parce qu'un couloir reste ouvert sur plusieurs de ses bords vers des
  salles voisines — propriété géométrique, pas un mécanisme.
- \anchor EX-REN-016 **EX-REN-016** — Le cadrage de la caméra doit offrir **trois modes**, choisis
  par le niveau (`EX-LVL-006`) et non déduits de ses dimensions : **niveau entier** (`EX-REN-013`),
  **par salle** (`EX-REN-015`) et **suivi du personnage**. Ce dernier — absent du moteur jusqu'ici —
  accompagne le personnage avec une **zone morte** (pas de tremblement permanent), une
  **anticipation** dans le sens du déplacement s'inversant **progressivement**, un lissage cadencé
  sur le **pas fixe** (`EX-REN-021`) et non sur la fréquence de rendu, et un **bornage** aux limites
  du niveau — un axe plus étroit que le cadrage étant **centré** plutôt que borné. Le centre retenu
  reste aligné sur la grille de pixels et le zoom **entier** (`EX-ARCH-022`), sous peine de rendre
  flou tout le pixel art. Aucun effet sur la simulation (`EX-ARCH-012`). Concrétisé en `LOT-64`.
- \anchor EX-REN-017 **EX-REN-017** — En **mode suivi** (`EX-REN-016`), la taille de la zone de
  caméra doit être **réglable par niveau**, avec les mêmes champs que la taille de salle du mode
  *par salle* (`EX-REN-015`), plutôt que de retenir en dur la taille de salle par défaut : un
  niveau sans taille déclarée conserve la taille par défaut actuelle. Concrétisé en `LOT-64`.
- \anchor EX-REN-014 **EX-REN-014** — Le rendu doit gérer un ordre de dessin par **couches**,
  défini par un **ordonnancement unique et explicite**, dont aucun calque concurrent ne peut
  s'écarter : **fond**, **plans derrière**, **ombres**, **tuiles physiques**, **objets**,
  **personnage**, **plans devant**, **interface**, **aides d'édition**. Le calque de **premier
  plan** est dessiné **au-dessus du personnage** : c'est le moyen de lecture immédiate qui distingue
  le décor traversable du décor physique (`EX-DEC-042`). Précisé en `LOT-40`. Les deux calques de
  décor ont été renommés **plans** en `LOT-69`, avec le système qu'ils portent — laisser un calque
  au nom d'un système retiré serait exactement la dette que cette exigence cherche à éviter.
- \anchor EX-REN-018 **EX-REN-018** — En vue de dessus, l'ordre de dessin des acteurs et
  du décor traversé doit venir de leur **profondeur**, et non de leur calque : une entité passe
  devant ce qui est plus haut qu'elle à l'écran, derrière ce qui est plus bas. La profondeur se lit
  au **pied** du sprite — le bord bas, point de contact avec le sol — et non à son coin haut, sans
  quoi deux sprites de hauteurs différentes posés sur la même case s'ordonneraient différemment.
  Ces calques forment une **bande de profondeur** commune, à l'intérieur de laquelle le tri par
  profondeur passe **avant** le regroupement par texture : un personnage et un arbre n'ayant jamais
  la même texture, aucun ordre de calque ni de texture ne peut rendre justes à la fois « derrière
  l'arbre du bas » et « devant l'arbre du haut ». Le surcoût — des passes de dessin
  supplémentaires — est **assumé**, et mesuré par le budget de rendu (`EX-NFR-005`). Le tri doit
  rester **stable** et quantifié au pixel : à profondeur égale, deux sprites gardent un ordre
  constant d'une image à l'autre, sans quoi ils scintilleraient. Hors de cette bande, l'ordre des
  calques reste souverain (`EX-REN-014`). Concrétisé en `LOT-07`.
- \anchor EX-REN-041 **EX-REN-041** — Le rendu doit pouvoir **charger ses textures depuis des
  fichiers image** (PNG au minimum), décodés en pixels RGBA puis créés en texture GPU, en plus
  de la génération procédurale historique. Le filtrage reste *nearest* (pixel art, `EX-ARCH-022`).
  Concrétisé en `LOT-39`.
- \anchor EX-REN-042 **EX-REN-042** — Les **assets graphiques** (atlas de tuiles) doivent être
  **externalisés en fichiers éditables hors code** (remplacer le fichier suffit à changer l'apparence),
  copiés à côté de l'exécutable comme les niveaux et la localisation, avec **repli procédural** si un
  asset est absent/illisible (`EX-NFR-040`). Concrétisé en `LOT-39`.
- \anchor EX-REN-043 **EX-REN-043** — Le rendu doit pouvoir dessiner, en une seule frame, des
  entités provenant de **plusieurs textures distinctes** (au-delà de l'atlas unique historique), sans
  changer le contrat public de `SpriteBatch`/`SpriteRenderer`, et selon l'**ordonnancement de calques
  explicite et unique** de `EX-REN-014`. Concrétisé en `LOT-40`.
- \anchor EX-REN-044 **EX-REN-044** — Un niveau doit pouvoir afficher une **image de fond**
  optionnelle, en dessous de toutes les tuiles, en mode Texture uniquement ; l'absence de fond
  configuré est un état normal (pas de repli visible), un fond référencé mais introuvable déclenche
  le repli en damier (`EX-NFR-040`). Concrétisé en `LOT-44`.
- \anchor EX-REN-045 **EX-REN-045** — Les tuiles **solides** doivent pouvoir projeter une **ombre**
  portée, purement visuelle, sur le fond du niveau en mode Texture, pour distinguer visuellement le
  physique du décor sans aucun effet sur le gameplay (`EX-ARCH-012`). Concrétisé en `LOT-55`.
- \anchor EX-REN-046 **EX-REN-046** — Le jeu doit permettre de basculer, par une commande **fixe et
  non remappable** (`F8`), entre le rendu **Physique** (couleur plate par type de tuile, accès direct
  à la lecture des collisions) et le rendu **Texture** (habillage complet — fond, décor, skin, objets
  interactifs) — disponible aussi bien en édition qu'en jeu réel. Le rendu **Texture** est le défaut
  dans **toutes** les configurations de build, et le dernier choix du joueur est **persisté** entre
  deux sessions : deux binaires du même code ne doivent jamais afficher un rendu différent par
  défaut, sous peine de rendre ambiguë toute capture d'écran ou vérification visuelle.
  Concrétisé en `LOT-41`.
- \anchor EX-REN-005 **EX-REN-005** — Les **animations** doivent être décrites par des **données**
  (clip nommé, suite d'images, durée par image, bouclé ou joué une fois) et non codées en dur, et
  s'appliquer à **toute** entité affichée : personnage, mécanismes, et tuiles animées (eau, lave,
  torche). La progression d'une animation se fait au **pas fixe** (`EX-REN-021`) afin de rester
  déterministe. Un asset sans description d'animation est affiché comme une **image fixe**.
  Concrétisé en `LOT-46`.
- \anchor EX-REN-006 **EX-REN-006** — L'apparence d'un **mécanisme** (porte, interrupteur, plaque
  de pression, danger commuté, danger temporisé, danger mobile) doit refléter son **état logique**
  par le choix d'un clip d'animation, y compris les **transitions** jouées une fois
  (ouverture/fermeture), et non par une modulation de teinte ou d'opacité. Le rendu reste en
  **lecture seule** sur la simulation (`EX-ARCH-012`). Concrétisé en `LOT-47`.
- \anchor EX-REN-007 **EX-REN-007** — Tout asset graphique chargé doit être **validé** à l'entrée
  du rendu (format décodable, dimensions conformes au contrat du type d'asset). Un asset invalide
  n'interrompt jamais le rendu : il est remplacé par le repli visible et journalisé avec le **nom
  du fichier** et la **dimension attendue**, pour que l'auteur sache quoi corriger (`EX-NFR-040`).
  Concrétisé en `LOT-40`.
- \anchor EX-REN-008 **EX-REN-008** — Le jeu doit pouvoir afficher des **effets visuels** de courte
  durée (traînée de déplacement rapide, poussière d'atterrissage, éclatement à la mort, secousse
  d'écran) simulés au **pas fixe** et donc **déterministes** (`EX-NFR-002`), sans aucun effet sur le
  gameplay (`EX-ARCH-012`) et dans un budget borné. Concrétisé en `LOT-53`.
- \anchor EX-REN-009 **EX-REN-009** — Le **personnage** doit pouvoir être habillé depuis une
  **spritesheet externe** (comme tout autre asset, `EX-REN-042`), avec des clips couvrant les états
  de gameplay réellement livrés (repos, course, saut, chute, atterrissage, glissade murale, dash) et
  son orientation. La taille d'une image de la spritesheet est **indépendante** de la boîte de
  collision du personnage, qui reste la source de vérité du gameplay. Concrétisé en `LOT-48`.

## 3. Boucle & temps
- \anchor EX-REN-020 **EX-REN-020** — Le jeu doit tourner à **60 images/seconde** cible.
- \anchor EX-REN-021 **EX-REN-021** — La logique doit être mise à jour à **pas de temps fixe** (simulation déterministe), le rendu pouvant être découplé.
- \anchor EX-REN-022 **EX-REN-022** — Le rendu doit synchroniser la présentation (V-Sync activable) pour éviter le *tearing*. Depuis le `LOT-69` TACHE-02, la présentation appartient au **compositeur de Qt** (`EX-REN-050`) : elle est donc toujours synchronisée, et le réglage exposé dans les Options est conservé mais sans effet — écart assumé, à trancher si le besoin d'une présentation immédiate réapparaît.
- \anchor EX-REN-004 **EX-REN-004** — La présentation doit utiliser le **modèle flip** de DXGI
  (`DXGI_SWAP_EFFECT_FLIP_DISCARD`, au moins deux back buffers) plutôt que l'ancien modèle *blt*
  (`DISCARD`) : sous Windows 10/11, le flip model présente le back buffer **sans copie
  supplémentaire** (compositeur DWM), ce qui réduit la latence entrée → image et régularise la
  cadence, V-Sync activée comprise (`EX-REN-022`). Concrétisé en `LOT-33`.

## 4. Interface (HMI)
- \anchor EX-REN-030 **EX-REN-030** — Le jeu doit afficher un **menu principal** (Jouer, Quitter).
- \anchor EX-REN-031 **EX-REN-031** — Le jeu affiche un écran de **pause** (Échap, ou bouton B
  manette, en partie réelle ; suspend réellement la simulation, sans consommer de pas de temps
  fixe) et un écran de **fin de niveau**/**fin de séquence** à la réussite d'un tableau
  (Continuer/Rejouer, ou retour au menu après le dernier). Détaillé côté interface par
  `EX-IHM-004`. Concrétisé en `LOT-59`.
- \anchor EX-REN-032 **EX-REN-032** — Le jeu doit afficher du **texte** (titres, indications) via une police bitmap ou vectorielle. Le texte de l'**interface hors-jeu** (menus, options, éditeur) est rendu par Qt depuis `LOT-38`, qui a retiré la police bitmap historique. Le texte **dans la scène rendue** — ancré au jeu, hors de portée des widgets Qt — reste à rétablir : concrétisé en `LOT-52`.
- \anchor EX-REN-033 **EX-REN-033** — Tout **texte affiché** doit passer par un **catalogue de traduction** : le code référence des **clés** stables, résolues vers une chaîne selon la **langue active**, chargée depuis un **fichier par langue** (français par défaut). Aucun libellé d'interface n'est codé en dur, afin de rendre l'ajout d'une langue trivial (un fichier de plus, sans modification du code). Une clé ou un fichier de langue manquant est traité comme une **erreur récupérable** (repli déterministe), cf. `EX-NFR-040`.

## 5. Audio
- \anchor EX-REN-040 **EX-REN-040** — Le jeu joue des **bruitages** : saut, atterrissage, dash,
  interrupteur, plaque de pression, mort, victoire de tableau, fin de séquence, et les sons
  d'interface (déplacement dans un menu, validation). Livré en `LOT-60`.
- \anchor EX-REN-047 **EX-REN-047** — La lecture audio vit **entièrement dans `HMI`** : `Core`
  expose des **transitions d'état** (`core::Player::justJumped`, `core::MechanismController::
  isDoorOpen`/`isContinuous`, `core::LevelOutcome`), `HMI` en déduit les sons à jouer
  (`hmi::GameEvents`, `hmi::SoundTriggers`) — jamais l'inverse. La simulation reste pure,
  déterministe et testable **sans périphérique audio** (`EX-NFR-010`), et le son n'a **aucun
  effet** sur elle (`EX-ARCH-012`). La détection de transitions est une fonction pure, réutilisable
  par de futurs effets visuels (`EX-REN-008`) sans être dupliquée. La bibliothèque retenue est
  **Qt Multimedia** (`hmi::AudioEngine`, `QSoundEffect`), par cohérence avec le reste de
  l'interface, déjà intégralement Qt depuis `LOT-38` ; elle est provisionnée sur les trois
  environnements selon `EX-BUILD-010`. Livré en `LOT-60`.
- \anchor EX-REN-048 **EX-REN-048** — Le **volume** est réglable depuis les options, prend effet
  immédiatement et est **persisté** (`QSettings`, même portée que la langue et le mode de rendu).
  L'absence de périphérique audio, de catalogue de sons ou d'un fichier référencé est une **erreur
  récupérable** (`EX-NFR-040`) : le jeu reste pleinement jouable en silence, avec un avertissement
  journalisé une seule fois par asset. Livré en `LOT-60`.

- \anchor EX-REN-049 **EX-REN-049** — Les **plans picturaux** (`EX-DEC-040`) doivent se composer
  dans l'**ordonnancement unique** de `EX-REN-014`, **sans y ajouter de valeur par plan** : leur
  nombre est libre, les énumérer figerait dans le rendu ce que le format déclare variable. Les plans
  de fond occupent le calque de décor, ceux de devant le calque de premier plan, et leur **rang dans
  la liste du niveau** fournit le tri fin **à l'intérieur** du calque. Concrétisé en `LOT-69`.
- \anchor EX-REN-050 **EX-REN-050** — Le rendu doit être présenté dans un **widget composé avec
  l'interface** (`QRhiWidget`, qui dessine dans une texture d'appui) et non dans une **fenêtre
  native** embarquée. Motif : un widget frère d'une fenêtre native ne se dessine jamais de façon
  fiable par-dessus elle, ce qui a déjà coûté deux défauts réels en `LOT-59` (écran de pause
  invisible, puis focus volé par `Qt::Tool`) et imposait un contournement par fenêtre de haut niveau
  à géométrie synchronisée. Les aides d'édition du mode création (`EX-EDIT-046`) en dépendent
  directement. Le rendu reste **Direct3D 11** (`EX-REN-002`), QRhi retenant ce backend par défaut
  sous Windows, et le filtrage reste *nearest* au zoom entier (`EX-ARCH-022`) — un portage qui
  rendrait le pixel art flou serait un échec. Concrétisé en `LOT-69`.

## Traçabilité
Tout ce qui touche fenêtre, rendu, entrées et interface relève de `Source/HMI` ; la logique de simulation reste dans `Source/Core`. Contraintes de performance : [`exigences-non-fonctionnelles.md`](exigences-non-fonctionnelles.md).
