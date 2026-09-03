# Éditeur de niveaux {#spec-editeur}

> Statut : **édition de tuiles de base et robustesse/confort d'édition validés et livrés**
> (LOT-14 : peinture, mécanismes, entrée/sortie, redimensionnement, undo/redo,
> enregistrement/validation, essai immédiat ; LOT-15 : nommage, garde-fous, caméra, outils de
> zone, découvrabilité ; LOT-16 : niveaux de grande taille, section 7 ; LOT-27 : palette organisée
> par catégories repliables, section 8).
> Dépend de [`niveaux.md`](niveaux.md).

## Objectif
Permettre la **création et la modification de niveaux sans écrire de code**, afin que des membres de l'équipe **non-développeurs** (game design, level design) contribuent directement au contenu du jeu.

## 1. Exigences fonctionnelles
- \anchor EX-EDIT-001 **EX-EDIT-001** — L'éditeur doit permettre de créer et modifier un niveau **sans compétence en programmation** ni ligne de commande.
- \anchor EX-EDIT-002 **EX-EDIT-002** — L'édition doit être **WYSIWYG** : une grille visuelle où l'on peint les tuiles (vide, solide, danger…) à la souris, depuis une **palette** de types.
- \anchor EX-EDIT-003 **EX-EDIT-003** — L'éditeur doit permettre de placer et **relier visuellement les mécanismes** (interrupteur ↔ porte ; clé ↔ porte verrouillée et blocs poussables **dès que `Core` les implémentera côté gameplay** — absents du moteur à ce jour, donc hors périmètre de l'éditeur jusque-là, cf. `epic.md` du lot LOT-14).
- \anchor EX-EDIT-004 **EX-EDIT-004** — L'éditeur doit permettre de définir l'**entrée** et la **sortie** du niveau.
- \anchor EX-EDIT-005 **EX-EDIT-005** — L'éditeur doit permettre de **redimensionner** la grille et de gérer **annuler/refaire** (undo/redo).
- \anchor EX-EDIT-006 **EX-EDIT-006** — L'éditeur doit **enregistrer et charger** au format JSON défini par `EX-LVL-003` (celui réellement implémenté par `LevelLoader`, pas un format hybride ASCII — l'édition texte brut n'est pas visée, cf. `EX-EDIT-001`), en produisant des fichiers **valides**.
- \anchor EX-EDIT-007 **EX-EDIT-007** — L'éditeur doit **valider** le niveau avant enregistrement (présence entrée/sortie, dimensions cohérentes, liaisons de mécanismes valides — `EX-LVL-004`) et signaler les erreurs de façon compréhensible par un non-codeur.
- \anchor EX-EDIT-008 **EX-EDIT-008** — L'éditeur doit permettre de **tester le niveau** immédiatement (le lancer dans le jeu depuis l'éditeur), pour un cycle création → essai rapide.
- \anchor EX-EDIT-009 **EX-EDIT-009** — L'éditeur doit permettre de **nommer** un niveau à sa création et de le **renommer**, et **avertir avant d'écraser** un fichier existant différent du niveau en cours d'édition lors de l'enregistrement.

## 2. Réutilisation & cohérence
- \anchor EX-EDIT-010 **EX-EDIT-010** — L'éditeur doit **réutiliser le modèle de niveau et la validation de `Core`** — aucune duplication de la logique de niveau entre le jeu et l'éditeur (source unique de vérité).
- \anchor EX-EDIT-011 **EX-EDIT-011** — Un niveau enregistré par l'éditeur doit être **directement jouable** par le jeu sans conversion, et réciproquement (round-trip fiable). Ce qu'un éditeur ne sait pas encore modifier, il doit le **transporter** : les couches visibles, les entités et les propriétés libres d'une carte (`EX-LVL-016` à `EX-LVL-018`) traversent un cycle ouvrir/enregistrer sans perte, même avant que l'interface ne les expose — un éditeur qui efface en silence ce qu'il n'affiche pas est pire qu'un éditeur incomplet.

## 3. Distribution & collaboration
- \anchor EX-EDIT-020 **EX-EDIT-020** — L'éditeur doit être fourni comme un **outil exécutable** que les non-codeurs lancent sans étape de build.
- \anchor EX-EDIT-021 **EX-EDIT-021** — Les niveaux sont des **fichiers** rangés dans `Source/Elements` et versionnés ; l'éditeur enregistre directement à cet emplacement.
- \anchor EX-EDIT-022 **EX-EDIT-022** — Le partage des niveaux passe par **Git via une interface graphique** (type GitHub Desktop) : les niveaux sont versionnés dans le dépôt au même titre que le reste du projet. Le level designer publie et récupère les niveaux en quelques clics, **sans ligne de commande**. Un court guide d'utilisation (installation + flux publier/mettre à jour) doit être fourni dans `Documentation/` à destination des non-codeurs.

## 4. Approche d'implémentation (décidée)
**Option retenue : éditeur intégré.** L'édition est un **mode de l'application** de jeu, réutilisant le rendu Direct3D 11 et le modèle de niveau de `Core`.

- \anchor EX-EDIT-030 **EX-EDIT-030** — L'éditeur est intégré à l'application (mode éditeur), et non un outil séparé.
- \anchor EX-EDIT-031 **EX-EDIT-031** — Le mode éditeur réutilise le **rendu D3D11** de `HMI` et le **modèle/validation de niveau** de `Core` (pas de duplication).

Justification : un seul codebase, un rendu identique au jeu, un cycle **création → essai** immédiat et un round-trip garanti avec le format de niveau. *(Repli documenté si le temps manque : l'éditeur libre Tiled avec une couche d'import vers notre format — non retenu par défaut.)*

## 4bis. Décors & pixel art (post-MVP, intégré à l'éditeur)
- \anchor EX-EDIT-040 **EX-EDIT-040** — L'éditeur doit permettre de **placer et transformer des décors** (position, échelle, superposition par couches) — cf. [`decors.md`](decors.md).
- \anchor EX-EDIT-041 **EX-EDIT-041** — L'éditeur doit intégrer la **conversion photo → pixel art** (chargement d'une photo, pixellisation, réduction de palette, paramètres ajustables) et enregistrer l'asset résultant dans `Source/Elements` — cf. `EX-DEC-030/031/032`.

Ces capacités sont livrées **après** l'édition de tuiles de base, mais l'architecture les accommode dès le départ (cf. [`architecture.md`](architecture.md)).

## 5. Non-objectifs (éditeur, MVP)
- Édition collaborative en temps réel (plusieurs personnes sur le même niveau simultanément).
- Édition des assets graphiques/sonores (l'éditeur agence des tuiles existantes, il ne dessine pas
  les sprites) — **exception ciblée** : `LOT-54` introduit un éditeur de texture pixel art minimal
  (peindre/modifier les fichiers d'assets eux-mêmes), sans remettre en cause ce non-objectif pour le
  reste de l'éditeur (agencement de tuiles existantes, pas de génération procédurale de sprites).
- Sélection multiple non contiguë et historique annuler/refaire par delta (l'historique par
  snapshots complets, retenu en LOT-14, reste adapté à la taille des niveaux du projet).
- Palette pilotée par un fichier de configuration externe (la liste de types gérés par `Core`
  reste petite et change rarement ; une couche de données externes serait une abstraction non
  justifiée à ce stade).

## 6. Robustesse et confort d'édition (LOT-15)
Une fois l'édition de tuiles de base livrée (LOT-14), l'usage réel fait ressortir des besoins
complémentaires pour rapprocher l'éditeur d'un outil de production : éviter la perte de travail,
éditer confortablement des niveaux plus grands, et rendre les commandes découvrables sans dépendre
uniquement de la documentation externe.

- \anchor EX-EDIT-012 **EX-EDIT-012** — L'éditeur doit **demander confirmation** avant toute action
  destructrice : un redimensionnement qui supprimerait l'entrée, la sortie ou une liaison de
  mécanisme, et la fermeture de l'éditeur alors que des modifications ne sont **pas enregistrées**.
- \anchor EX-EDIT-013 **EX-EDIT-013** — L'éditeur doit permettre de **déplacer (pan)** et de
  **zoomer** la vue indépendamment du cadrage automatique, pour éditer confortablement des niveaux
  de toute taille.
- \anchor EX-EDIT-014 **EX-EDIT-014** — Au-delà de la peinture case par case, l'éditeur doit fournir
  un **outil de remplissage rectangulaire** et un **outil de sélection** avec **copier/coller** d'une
  zone de tuiles.
- \anchor EX-EDIT-015 **EX-EDIT-015** — L'éditeur doit exposer ses commandes de façon
  **découvrable** à l'écran : une barre d'outils pour changer d'outil, un aperçu des raccourcis
  clavier, et des libellés sur les entrées de la palette. *La barre d'outils à icônes est concrétisée
  en `LOT-56` (`EX-IHM-055`) et l'aperçu des raccourcis en `LOT-57` — lots hérités, archivés sous
  `Documentation/Heritage/Lot/` : le
  changement d'outil passait jusqu'ici par des boutons radio empilés, et aucun aperçu des raccourcis
  n'était atteignable depuis l'application.*
- \anchor EX-EDIT-016 **EX-EDIT-016** — Lorsque plusieurs liaisons interrupteur ↔ porte sont
  visibles simultanément, chacune doit être **visuellement distinguable** des autres (et non une
  teinte unique partagée par toutes les liaisons).

## 7. Niveaux de grande taille (LOT-16)
Le redimensionnement (`EX-EDIT-005`) se faisait jusqu'ici uniquement case par case (flèches),
praticable pour de petits ajustements mais pas pour viser directement une grande taille. Aucune
limite technique n'existe dans `Core` (`TileMap`/`LevelDraft` acceptent toute dimension positive) —
seule l'ergonomie manquait.

- \anchor EX-EDIT-017 **EX-EDIT-017** — L'éditeur doit permettre de **saisir directement** une
  largeur et une hauteur cibles (plutôt que d'incrémenter case par case), sous un **plafond
  généreux** (très au-delà des tailles livrées à ce jour) qui reste configurable au niveau du code,
  pas une limite arbitraire de `Core`.

## 8. Palette organisée par catégories (LOT-27)
La palette (`EX-EDIT-002`) affichait ses types de tuiles en **liste plate** : 19 lignes, chacune
son propre libellé, où les huit variantes de pente/arrondi et les trois tailles de bloc poussable
noyaient les huit types simples au milieu d'orientations presque identiques visuellement. Aucun
souci de compréhension du type lui-même (chaque icône reste le rendu réel de la tuile), mais un
défilement long avant de repérer le bon type.

- \anchor EX-EDIT-018 **EX-EDIT-018** — La palette doit regrouper les types de tuiles en
  **catégories repliables** plutôt qu'en liste plate : quatre catégories (Tuile, Interactif,
  Piège, Jalon) plus une entrée autonome (Vide), chacune dépliable indépendamment sans jamais
  recouvrir le canevas d'édition. Une famille regroupant **plusieurs formes ou tailles d'un même
  type d'usage** (Pente, Arrondi, Bloc poussable) doit être elle-même un **sous-groupe repliable**
  imbriqué dans sa catégorie — trois niveaux d'accordéon au maximum jusqu'à une variante précise,
  plutôt qu'une seconde liste plate cachée sous la première. Si tout déplier en même temps dépasse
  la hauteur de fenêtre disponible, la palette doit rester entièrement accessible par
  **défilement** (molette, barre de défilement) plutôt que de rendre ses dernières entrées
  inaccessibles à la souris ; déplier ou replier un en-tête ne doit jamais le faire disparaître de
  la fenêtre visible.

## 9. Dangers avancés (`LOT-31`)
La catégorie « Piège » (`EX-EDIT-018`), jusqu'ici une entrée autonome à côté de « Vide » (une seule
tuile, `Danger`), devient une catégorie repliable à part entière une fois les quatre nouvelles
variantes ajoutées (`EX-GP-050` à `EX-GP-053`) : une feuille directe « Classique », un sous-groupe
« Directionnel » (quatre bords) et trois feuilles directes (Mobile, Commuté, Clignotant) — même
patron d'accordéon que les catégories existantes.

- \anchor EX-EDIT-019 **EX-EDIT-019** — Un danger commuté doit se lier à un interrupteur/une plaque
  de pression par le **même geste** qu'une porte (clic déclencheur, clic cible, `EX-EDIT-003`) ;
  plusieurs liaisons simultanées (porte ou danger commuté) restent distinguables les unes des
  autres (`EX-EDIT-016`), un déclencheur qui active à la fois une porte et un danger commuté
  partageant une teinte cohérente sur ses trois cases plutôt que deux échelles indépendantes.

L'axe et la portée d'un danger mobile, ainsi que la période/le déphasage d'un danger temporisé,
**ne sont pas** éditables par un widget dédié (cf. exclusion actée dans `Documentation/Lot/
LOT-31-blocs-danger-avances/epic.md`) — un danger mobile peint depuis la palette garde ses valeurs
de conception par défaut tant qu'elles ne sont pas ajustées en modifiant directement le fichier de
niveau.

## 10. Niveaux à salles (`LOT-32`)
Un niveau plus grand qu'une **salle** (`EX-REN-015`) se joue avec une caméra qui cadre la salle
courante, pas le niveau entier — un level designer doit pouvoir repérer où se trouvent les
frontières de salles pendant l'édition, pour aligner ses couloirs inter-salles sans deviner.

- \anchor EX-EDIT-023 **EX-EDIT-023** — L'éditeur doit afficher, en superposition de la grille de
  tuiles, un **quadrillage des frontières de salles** (bascule `F10`, même commande que le
  quadrillage de repère case par case, `EX-EDIT-015`), distinguable visuellement de ce dernier. Ce
  repère n'affecte **pas** le cadrage caméra de l'éditeur (pan/zoom manuel sur le niveau entier,
  `EX-EDIT-013`, inchangé) : seule la caméra du **jeu** cadre par salle (`EX-REN-015`).

## 11. Habillage par textures et décors (`LOT-40` → `LOT-55`)
Au-delà de la couleur plate par type de tuile (rendu « Physique », inchangé), le level designer doit
pouvoir habiller le niveau avec de vraies textures — sans jamais perdre la lecture du physique
(`EX-NFR-040`, `EX-ARCH-012` : purement visuel, aucun effet sur la simulation).

- \anchor EX-EDIT-042 **EX-EDIT-042** — L'éditeur doit permettre d'associer une **texture** à chaque
  type de tuile, choisie parmi des fichiers image existants (pas de saisie de chemin) — répond au
  besoin d'origine d'habiller les blocs. Chaque association déclare un **mode de rendu** : image
  unique, ou **planche à raccords automatiques** (`EX-EDIT-025`). Concrétisé en `LOT-42`.
- \anchor EX-EDIT-024 **EX-EDIT-024** — Les associations type de tuile → texture doivent être
  regroupées en **jeux de skins nommés** (ex. « forêt », « grotte »), et un niveau doit pouvoir
  désigner le jeu qu'il utilise : une seule association globale pour tout le jeu empêcherait toute
  variété d'ambiance au fil de la progression. En l'absence de désignation, le niveau utilise le jeu
  par défaut. Concrétisé en `LOT-42` (jeux de skins) et `LOT-44` (désignation par niveau).
- \anchor EX-EDIT-025 **EX-EDIT-025** — Pour les types de tuiles **solides**, le rendu doit pouvoir
  choisir automatiquement l'image affichée en fonction des **tuiles solides voisines** (raccords de
  bords et de coins) au sein d'une **planche** fournie par l'auteur, plutôt que de répéter la même
  image partout. La règle de choix est **déterministe** et testable indépendamment du GPU.
  Concrétisé en `LOT-42`.
- \anchor EX-EDIT-026 **EX-EDIT-026** — L'éditeur doit permettre de **gérer les fichiers d'assets**
  sans quitter l'application : importer une image externe dans le dossier d'assets, renommer,
  dupliquer, supprimer — avec un **avertissement** lorsque l'asset visé est référencé par un niveau
  ou par un jeu de skins — et **recharger à chaud** les assets modifiés sur disque sans redémarrer.
  Concrétisé en `LOT-43`.
- \anchor EX-EDIT-027 **EX-EDIT-027** — La **palette de l'éditeur** doit afficher l'apparence
  réellement rendue : en mode Texture, la texture assignée au type ; en mode Physique, la couleur
  plate. Peindre sans voir ce que l'on pose est une régression d'usage. Concrétisé en `LOT-42`.
- \anchor EX-EDIT-028 **EX-EDIT-028** — L'éditeur doit permettre de **choisir le mode de cadrage**
  du niveau (`EX-LVL-006`) et de le **prévisualiser** dans le canevas — cadre du niveau, grille de
  salles, ou rectangle visible et zone morte pour le suivi — sans avoir à lancer l'essai. Le
  changement de mode est une opération d'édition **annulable** (`EX-EDIT-005`) et le mode courant est
  visible en permanence dans la barre d'état (`EX-IHM-062`). Sans cela, le cadrage resterait
  accessible aux seuls éditeurs de JSON, ce que l'existence même de l'éditeur (`EX-VIS-006`) exclut.
  Concrétisé en `LOT-64`.
- \anchor EX-EDIT-029 **EX-EDIT-029** — L'éditeur doit permettre de **dessiner et retirer** les
  zones de caméra du mode *par salle* (`EX-LVL-007`) directement sur le canevas, par un outil dédié
  au même titre que les autres outils de dessin (rectangle, sélection), avec un tableau
  récapitulatif dans la section « Cadrage ». Chaque ajout ou retrait est une opération d'édition
  **annulable** (`EX-EDIT-005`). Concrétisé en `LOT-64`.
- \anchor EX-EDIT-032 **EX-EDIT-032** — L'éditeur doit permettre de **manipuler directement au
  canevas** la trajectoire des éléments mobiles : déplacer, insérer et retirer les points de la
  route d'une plateforme mobile (`EX-GP-054`), redéfinir l'axe et la portée d'un danger mobile
  (`EX-GP-051`), au moyen de **poignées glissables** de taille écran constante. Chaque geste complet
  est **une seule** opération annulable (`EX-EDIT-005`), jamais une par position intermédiaire.
  Sans cela, ces trajectoires resteraient accessibles aux seuls éditeurs de JSON, ce que l'existence
  même de l'éditeur (`EX-VIS-006`) exclut. Concrétisé en `LOT-67`.
- \anchor EX-EDIT-033 **EX-EDIT-033** — L'éditeur doit exposer, dans un panneau dédié, les
  **paramètres de temporisation** des éléments mobiles (vitesse, déphasage et mode de parcours d'une
  plateforme ; axe et portée d'un danger mobile ; période, déphasage et durée active d'un danger
  temporisé) ainsi que les **règles du tableau** (budgets de `EX-GP-024` et capacités de
  `EX-GP-055`), sous une forme éditable et **annulable** (`EX-EDIT-005`). Les deux notions de règles
  doivent être présentées **distinctement**, un budget consommable ne se confondant pas avec une
  capacité rechargée. Concrétisé en `LOT-67`.
- \anchor EX-EDIT-043 **EX-EDIT-043** — L'éditeur doit permettre d'assigner une **texture propre à
  une case précise** (« objet interactif », ex. une porte particulière), par un geste de clic dédié,
  prioritaire sur l'association globale (`EX-EDIT-042`) pour cette case. Concrétisé en `LOT-45`.
- \anchor EX-EDIT-044 **EX-EDIT-044** — L'éditeur doit permettre de contrôler la **visibilité de
  chaque calque** indépendamment (fond, décors, skin des tuiles, objets interactifs, personnage,
  premier plan), à des fins d'inspection : cela couvre l'affichage **isolé** d'un calque comme toute
  **combinaison** de calques — distinct de la bascule Physique/Texture en jeu (`EX-REN-046`).
  Concrétisé en `LOT-51`.
- \anchor EX-EDIT-045 **EX-EDIT-045** — L'éditeur doit intégrer un **outil de dessin pixel art**
  minimal (peindre/effacer, palette, zoom, annuler/refaire) pour créer/modifier directement les
  fichiers d'assets de texture, sans dépendance externe, avec un **aperçu du rendu dans le niveau**
  pendant l'édition. Concrétisé en `LOT-54`.
- \anchor EX-EDIT-046 **EX-EDIT-046** — L'éditeur doit offrir un **mode création** — un espace de
  travail à part entière, au même titre que l'édition de niveau et l'atelier pixel art — où le
  niveau **entier** devient une surface peignable, **plan par plan** (`EX-DEC-045`), au pixel près
  et à l'échelle **1:1** (un pixel du plan = un pixel du jeu). Le plan actif est peint avec les
  outils de l'atelier (`EX-EDIT-045`), qui sont **réutilisés et non redéveloppés** ; les tuiles
  physiques et les autres plans sont composés en **référence atténuée** (« pelure d'oignon »), et
  tout plan peut être **isolé** ou masqué le temps du travail. Le repère est **géométrique** : il
  n'a ni raccords automatiques, ni skins, ni animations — l'aperçu fidèle reste le rendu du niveau
  et l'essai. L'historique du dessin est **strictement distinct** de celui de l'édition de niveau :
  annuler un coup de pinceau ne doit jamais annuler une pose de tuile. Le zoom doit descendre
  **sous** le 1:1 pour embrasser un niveau entier, en conservant des pixels **carrés**
  (`EX-ARCH-022`). Concrétisé en `LOT-69`.
- \anchor EX-EDIT-047 **EX-EDIT-047** — L'éditeur doit exposer, dans un **panneau dédié**, la liste
  ordonnée des plans du niveau et leurs réglages : ajout, suppression, **réordonnancement**,
  densité, facteurs de parallaxe, opacité, profondeur et visibilité. Ajouter un plan crée son
  fichier PNG **transparent aux dimensions exactes** attendues ; changer sa densité rééchantillonne
  l'image existante plutôt que de la perdre. Le poids mémoire du niveau (`EX-NFR-043`) est visible
  **pendant** qu'on le dépense. Concrétisé en `LOT-69`.

## Traçabilité
L'éditeur s'appuie sur `Core` (modèle et validation de niveau, `niveaux.md`) et sur le rendu de
`HMI` (`rendu-technique.md`). L'édition de tuiles de base a fait l'objet du lot **LOT-14** (terminé) ;
la robustesse et le confort d'édition (section 6) du lot **LOT-15** (terminé) ; la saisie directe de
grandes tailles (section 7) du lot **LOT-16** (terminé) ; la palette organisée par catégories
(section 8) du lot **LOT-27** (terminé) ; les dangers avancés (section 9) du lot **LOT-31**
(terminé) ; le repère visuel de salles (section 10) du lot **LOT-32** (terminé) ; l'habillage des
tuiles par textures et les décors (section 11) des lots **LOT-40** à **LOT-55** (non commencés).
L'édition de décors (section 4bis, `EX-EDIT-040`) est concrétisée en **LOT-50** ; la conversion
photo → pixel art (`EX-EDIT-041`) reste hors du programme.
