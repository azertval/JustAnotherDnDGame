# Changelog

Toutes les évolutions notables du projet sont consignées ici.
Format inspiré de [Keep a Changelog](https://keepachangelog.com/fr/1.1.0/) ;
le projet suit le [versionnage sémantique](https://semver.org/lang/fr/).

## [Non publié]

- **Le châssis des écrans du RPG** (`LOT-68`). Huit écrans manquaient au jeu, et aucun n'existait
  même en ébauche : fiche de personnage, inventaire et équipement, journal de quêtes, carte du
  monde, dialogue, marchand, tableau de la Guilde, ATH de combat. Ce lot ne les **remplit** pas —
  c'est le travail des `LOT-38`, `LOT-42`, `LOT-45` et `LOT-24` — il livre ce qu'ils ont en commun
  et qu'aucun ne doit réinventer.
  - **L'ossature est une table, et c'est tout le lot.** Le critère de la feuille de route disait :
    *ajouter un neuvième écran ne demande de toucher à aucun des huit*. Il ne se tient pas avec huit
    fichiers d'interface, fussent-ils bien écrits — le premier pied de page à corriger le serait
    huit fois. `hmi::rpgScreens()` décrit donc chaque écran en **données pures** (blocs, genres,
    libellés, `EX-IHM-090`), et le châssis Qt ne connaît **aucun** écran par son nom. Même règle
    pour la feuille de style, qui habille par **rôle** et jamais par nom d'objet.
  - **Les champs annoncés sont relevés sur les modèles déjà livrés** — `core::CharacterSheet`,
    `core::Ability`, `core::Equipment`. Une ossature qui annonce des champs que le modèle ne porte
    pas promet ce que le jeu ne pourra pas tenir. Les valeurs, elles, sont des **tirets** : ce lot
    livre le cadre, et une valeur d'exemple se lirait comme un état du jeu (`EX-IHM-072`).
  - **La règle de superposition appartient à l'écran, pas à l'appelant** (`EX-IHM-091`). La carte du
    monde et l'ATH de combat se consultent **en marchant** — on ouvre une carte pour savoir où l'on
    va sans s'arrêter ; les six autres suspendent la simulation. Décidée au point d'appel, cette
    règle se contredirait d'un appel à l'autre sans que rien ne le signale.
  - **« Nouvelle partie » ouvre le châssis, et c'est un échafaudage assumé.** Cette entrée n'a
    aucune carte à charger — `demo-deplacement.json` n'existe pas, le `LOT-67` l'avait écrit — et
    huit écrans qu'aucun chemin n'atteint ne se relisent ni ne se valident. La ligne à rendre à son
    usage le jour où le `LOT-27` livrera une carte est **une seule**, et elle le dit. En attendant,
    l'écran de jeu et celui de pause ne sont plus atteignables depuis le menu ; la table de
    transitions déclare et teste déjà l'ouverture d'un écran du RPG depuis l'un et l'autre.
  - **Trois défauts d'agencement, trouvés en ouvrant l'application** et invisibles dans le code : le
    pied d'actions passait sous la ligne de flottaison (une seconde zone défilante borne désormais
    le contenu seul, le pied reste posé au bas du cadre) ; le bandeau de titre débordait de la
    fenêtre à la taille des titres d'écran, sortant la colonne de droite du cadre sans qu'aucune
    erreur ne soit levée ; et les rappels de touches imposaient leur largeur — une aide ne décide
    pas de la largeur d'une fenêtre.

- **Menus et vocabulaire d'un RPG** (`LOT-67`). Le jeu décrivait un autre jeu : « Choisir un
  niveau » au menu, « Recommencer le niveau » en pause, et un avertissement de sortie qui parlait
  de « la progression du **tableau** en cours ». Ce lot retire la **notion de niveau discret** —
  des écrans, du code, du vocabulaire et des exigences — et remplace le décor du menu principal par
  la **carte du monde de Tanares**.
  - **Le menu principal perd deux entrées, parce qu'elles ne menaient plus nulle part.**
    « Continuer » reposait sur une progression au tableau, « Choisir un niveau » sur une séquence :
    les deux sont retirées. Les griser aurait coûté plus de confiance qu'elles n'apportaient
    d'information (`EX-IHM-072`). « Continuer » revient avec la sauvegarde du `LOT-17`, les entrées
    RPG de la pause avec les écrans du `LOT-68`.
  - **Il perd aussi son titre** : le fond *est* la carte du monde, et un bandeau posé dessus
    répétait en lettres ce que l'image dit déjà. Les autres écrans gardent le leur — sans image à
    eux, on ne saurait pas où l'on est.
  - **La carte du monde trace la frontière que le `LOT-76` avait ouverte.** Ce lot-là concluait que
    l'habillage se **trace** (`EX-IHM-075`) ; celui-ci livre une image, et c'est la même frontière
    prise de l'autre côté (`EX-IHM-076`) : un ornement se trace parce qu'il doit se redimensionner
    et suivre les jetons, une carte peinte ne le peut pas. Mêmes garde-fous — région déclarée,
    manifeste recoupé en CI avec les fichiers et le code, repli si l'image manque.
  - **Du JPEG, seul du dépôt, et c'est délibéré** : le PNG de cette carte pèse 4,4 Mo, son JPEG
    0,7, pour une différence que personne ne voit sous un voile. Le poids du dépôt est un sujet du
    corpus depuis le début (`EX-CNT-023`). Le filigrane d'achat est **recadré**, jamais effacé :
    l'effacer demanderait de repeindre ce qu'il recouvre.
  - **~1 300 lignes retirées** : deux écrans (sélection de niveau, fin de niveau), deux modèles
    (`Progression`, `LevelSequence`), un bilan de partie (`LevelRunStats`), deux maquettes. C'étaient
    les mises en œuvre des exigences retirées ; les garder aurait laissé du code que plus aucune
    exigence ne justifie.
  - **Seize exigences traitées : douze retirées, trois refondues, une conservée.** `EX-GP-040`,
    `EX-IHM-003` et `EX-IHM-004` avaient un objet **au-delà** du niveau discret — le jeu a toujours
    des états, un ATH et un écran de pause — et les retirer aurait laissé leur mise en œuvre
    orpheline. Les douze autres sont **retirées, pas supprimées** : leurs ancres restent, avec le
    texte d'origine et le motif, comme le `LOT-H-69` l'avait fait pour les décors-sprites — une
    vingtaine de lots hérités s'y réfèrent.
  - **« niveau » devient « carte », partout**, y compris côté éditeur où la famille de clés
    `level.*` devient `map.*` : l'éditeur n'édite pas des niveaux, il édite les cartes du monde.
    Les deux catalogues restent synchrones, 376 clés de chaque côté. L'événement `LevelCompleted`
    devient `ExitReached`, et `SequenceCompleted` disparaît avec son bruitage.
  - **Ce que le lot ne rend pas jouable, et le dit** : « Nouvelle partie » ouvre
    `demo-deplacement.json`, qui **n'existe pas** — le `LOT-01` a purgé les niveaux du jeu de
    plateforme et aucun lot n'en a livré depuis. Le constat est antérieur à ce lot (la « Nouvelle
    partie » d'avant chargeait une séquence tout aussi absente) mais il devient visible.

- **Habillage d'interface extrait des livres** (`LOT-76`). Les écrans du jeu portent enfin ce qui
  fait reconnaître une page de Tanares en une seconde : la **pierre sertie** à l'angle des panneaux
  et le **bandeau de titre à ailes**. Tous deux **tracés**, donc nets à tout facteur
  d'agrandissement et pilotés par les jetons de la charte.
  - **Le découpage d'images a été construit, puis abandonné.** Vingt et une planches PNG à 300 ppp,
    leur manifeste et leur lint d'intégrité existaient ; c'est l'écran qui a tranché. Un cabochon de
    108 pixels sur un panneau haut de 340 mangeait le tiers de sa hauteur, et le bandeau demandait
    l'impossible à un découpage en tranches — une plaque qui s'allonge avec le titre, des ailes qui
    n'en font rien. À ce point-là, on ne redimensionnait plus une image, on la redessinait mal.
  - **Ce que le corpus donne reste entier : la mesure, pas la matière.** Le grenat des cabochons
    (`#701010`) est la dominante quantifiée des pixels rouges d'un cabochon, mesurée **séparément
    sur deux angles opposés** de la planche — même méthode que la palette du `LOT-66`, même
    vérification croisée. Le grenat profond des plaques (`#400000`) vient du bandeau du livre.
  - **`error` n'est plus le seul rôle inventé.** Le `LOT-66` le signalait comme non attesté, « une
    feuille de personnage n'ayant pas d'état d'erreur à montrer ». C'était vrai d'un état d'erreur
    et faux du rouge : il est dans les gemmes. Deux jetons neufs, `gem` et `gemShadow`, relevés.
  - **L'invariant du bandeau : l'envergure des ailes suit la hauteur, jamais la largeur.** Un titre
    long allonge la plaque et rien d'autre — ce qu'une image étirée ne sait pas faire. Quand la
    largeur manque, ce sont les ailes qui cèdent, puis disparaissent ; jamais la plaque, qui porte
    le titre.
  - **La variante accentuée garde ses angles nus.** Son filet passe à la couleur d'accent pour
    signaler un écran superposé ; une pierre par-dessus rendrait ce signal illisible.
  - **Les six titres d'écran deviennent des bandeaux.** Le texte tient **entre** les ailes par les
    marges de contenu, et non par un décalage au moment de peindre — sinon l'élision décide sur la
    mauvaise largeur, et le mot coupé n'apparaît que sur le titre le plus long. Sa couleur passe à
    l'or pâle : l'or des filets tenait sur du parchemin, il disparaît sur le grenat.
  - Exigence ajoutée : `EX-IHM-075` — l'habillage ornemental se **trace**, il ne se livre pas en
    image. `EX-IHM-070` imposait de relever les **couleurs** ; rien n'était écrit des **formes**.

- **Charte visuelle : sortir de l'identité pixel art** (`LOT-66`). L'interface héritée du jeu de
  plateforme laisse place à l'identité du **parchemin de Tanares** — parchemin, encre sépia, filets
  et cabochons dorés, titrage à empattements.
  - **La palette est relevée, pas choisie.** Chaque teinte vient de l'histogramme quantifié des
    pages rendues de `Character_Sheets_Tanares.pdf`. C'est la règle du corpus transposée à la
    couleur : une couleur inventée ressemble à la source sans en venir, et rien ne le dit jamais.
    Un seul rôle n'est pas attesté — `error`, une feuille de personnage n'ayant pas d'état d'erreur
    à montrer — et il est **signalé comme tel dans le code** plutôt que glissé dans la liste.
  - **Les rôles de cadre changent de nom, et c'est le cœur du lot.** `outline`, `bevelLight`,
    `bevelDark` nommaient un **biseau** : une lumière venue d'en haut à gauche. Le parchemin n'a pas
    de relief à simuler. Garder ces noms en peignant un encadrement plat aurait produit du code
    juste dont les noms décrivent autre chose. Ils deviennent `frameEdge`, `frameOrnament`,
    `frameShadow`.
  - **Ce qui fait un encadrement, c'est la réserve.** Un trait d'encre, une **réserve de
    parchemin**, un filet doré : sans la réserve du milieu, les deux traits se touchent et
    l'ensemble devient une bordure épaisse de deux tons — sans qu'aucune erreur ne soit levée,
    toutes les bandes étant toujours là. Un test relève donc le rôle **visible** à mi-hauteur et
    exige d'y trouver du parchemin.
  - **Le facteur d'agrandissement reste entier, pour une autre raison.** Le filtrage au plus proche
    voisin ne le justifie plus ; les longueurs de la feuille de style, elles, sont des entiers de
    pixels, et à 1,5× le trait et le filet s'arrondissent à la même épaisseur — la réserve
    disparaît. Une échelle fractionnaire ne serait pas *floue*, elle serait **fausse**.
  - **641 lignes de widgets pixel art supprimées** de `Source/HMI/Interface/`. Quatre des cinq
    modules disparaissent ; `PixelArtScale` est **renommé** `IdentityScale` parce qu'`EX-IHM-081`,
    que le lot ne touche pas, est écrite en fonction de ce facteur — le supprimer laisserait une
    exigence sans mise en œuvre.
  - **Le focus n'est jamais perdu de vue.** `EX-IHM-071` et `EX-IHM-072` sont tenues sans
    changement. Le curseur en escalier devient un **fleuron** anticrénelé, tracé **une seule fois**
    et appelé des deux côtés : deux tracés séparés dériveraient l'un de l'autre, et le joueur
    croirait à deux états différents.
  - **Onze exigences refondues**, à commencer par la racine `EX-ARCH-022` — dont dix tenaient
    d'elle leur justification.
  - `ctest` reste à **1009** cas, tous verts.

- **Équipement : armes, armures et matériel** (`LOT-34`). Les tables des *Basic Rules* vers
  `Source/Elements/Rpg/` — **37 armes**, **13 armures** et **125 objets** (matériel, outils,
  montures et véhicules).
  - **C'est le lot où le §4 se paie.** « Un tableau ne s'extrait pas en flux de texte » est une
    règle du projet depuis le `LOT-30` ; nulle part sa conséquence n'est aussi silencieuse qu'ici :
    une valeur de prix décalée d'une ligne ne casse rien, ne lève aucune alerte, et déséquilibre
    l'économie sans que personne ne comprenne pourquoi.
  - **Le groupe d'une arme n'est pas dans sa rangée.** Ce sont les intertitres du livre — « Armes
    courantes de corps à corps », « Armes de guerre à distance » — et eux seuls qui disent qu'une
    arme est courante ou de guerre. Une extraction qui ne lirait que les rangées produirait
    trente-sept armes sans catégorie, et `category` est requis au schéma sans que rien ne dise
    qu'il est **juste**. Le défaut s'est manifesté à la première exécution : la bande d'ordonnées
    commençait après le premier intertitre, et dix armes sortaient sans catégorie — c'est le
    contrôle de cardinal, 27 au lieu de 37, qui l'a dit.
  - **Les trois formes de la colonne CA disent trois règles.** `11 + Mod.Dex` sans plafond,
    `14 + Mod.Dex (max +2)` plafonné, `18` sans Dextérité du tout. Les réduire à leur premier
    nombre appliquerait la Dextérité au harnois — ce qui rend le personnage **plus** résistant,
    jamais moins, ne provoque aucune erreur et passe pour de l'équilibrage. Le test choisit une
    Dextérité de +4 précisément pour que l'écart se voie.
  - **Trois cas que le livre écrit et qu'un schéma refusait.** Le **filet** n'inflige aucun dégât —
    il entrave — et `damage` est devenu facultatif plutôt que de lui inventer des dés ; la
    **fronde** n'a pas de poids, et le tiret du livre vaut *absent*, jamais zéro ; le **bouclier**
    n'est pas une armure, il ajoute au lieu de remplacer, et le traiter comme telle donnerait une
    CA de 2 à un personnage en bouclier seul.
  - **La table du matériel est composée en deux sous-tables côte à côte** : une rangée y porte six
    cellules, donc deux objets. Les lire d'un bloc donnerait un objet pesant
    « 500 g Billes de fronde (20) ».
  - **Deux unités converties une seule fois.** Le livre mêle kilogrammes et grammes dans la même
    table, et compte en pièces d'or, d'argent et de cuivre ; les catalogues ne connaissent que les
    grammes et les pièces de cuivre.
  - `ctest` passe de 1001 à **1008** cas, tous verts.

- **Entités de carte et interaction** (`LOT-10`). Les cartes se peuplent d'entités qui ne sont
  **pas des tuiles** — coffres, panneaux, et demain PNJ et portails — et le joueur peut interagir
  avec elles.
  - **Le piège du lot est un coffre ouvert deux fois.** Le critère est facile à énoncer et facile à
    rater : *y compris après un aller-retour de carte*. C'est cette moitié de phrase qui décide de
    la conception — quand le joueur revient, l'entité du coffre est **recréée depuis le fichier de
    niveau**, qui ne sait rien de ce qui s'est passé. Un booléen porté par l'entité disparaîtrait
    avec elle, et le coffre redonnerait son butin à chaque passage : un défaut qui ne casse rien,
    ne lève aucune alerte, et se confond avec de la générosité de conception. L'état vit donc dans
    `core::WorldFlags`, à côté des entités. **Le test détruit le monde et le reconstruit** pour le
    vérifier.
  - **La clé de drapeau est fabriquée, jamais écrite à la main** : `<carte>/<type>@<colonne>,<ligne>`.
    Deux coffres d'une carte se distinguent par leur case, et le nom de carte empêche que vider un
    coffre au village en vide un autre au donjon. Corollaire assumé : déplacer un coffre dans
    l'éditeur le remet à neuf pour une partie en cours — l'inverse demanderait un identifiant
    stable que le `LOT-11` devrait générer et maintenir unique.
  - **Un coffre vidé n'est plus une cible du tout**, et pas seulement une cible qui ne fait rien :
    continuer à l'afficher promettrait au joueur quelque chose qui n'arrivera pas.
  - **La case visée suit la direction dominante, jamais une diagonale.** Un personnage qui regarde
    à 30° vise la case de droite : viser en diagonale rendrait la cible imprévisible à la manette
    analogique, alors que le joueur doit savoir ce qu'il désigne **avant** d'appuyer. Une
    orientation nulle ne vise rien.
  - **L'interaction ne traverse pas un mur**, et **à plusieurs candidats le choix est
    déterministe** — le plus proche du centre de la case visée, puis le plus petit indice. Sans
    départage, deux objets sur la même case donneraient tantôt l'un tantôt l'autre selon l'ordre de
    parcours de l'ECS, qui n'est pas stable.
  - **Un type d'entité inconnu produit tout de même une entité**, sans composant interactif : la
    refuser ferait disparaître un objet de la carte sans que son auteur comprenne pourquoi
    (`EX-NFR-040`). Les familles connues sont une table, non un `if` par cas — le `LOT-15` et le
    `LOT-09` en ajouteront sans retoucher la fonction.
  - `ctest` passe de 992 à **1001** cas, tous verts.

- **La fiche de personnage** (`LOT-13`). Toute créature — héros, PNJ, ennemi — a désormais une
  fiche complète : caractéristiques, points de vie, classe d'armure, niveau, bonus de maîtrise,
  jets de sauvegarde, compétences, vitesse. Et elle **monte de niveau**.
  - **« Aucune valeur de règle dans le C++ » se vérifie sur le diff, pas sur l'intention.** C'est le
    critère le plus facile à croire tenu : une valeur de règle a l'air d'une constante
    d'implémentation, et rien ne les distingue une fois écrites. Ce lot en a trouvé **trois**.
  - **La table d'expérience est une donnée** : vingt seuils et vingt bonus de maîtrise, extraits de
    la table des *Basic Rules* p. 11 et lus **par coordonnée** — ses trois colonnes n'ont ni filet
    ni séparateur. Elle tiendrait en trois lignes de C++, et c'est ce qui la rend dangereuse :
    équilibrer la progression demanderait alors une recompilation à chaque essai. Deux contrôles
    arrêtent la génération — les vingt niveaux présents **et dans l'ordre**, les seuils
    **strictement croissants** : deux seuils inversés rendent une montée infranchissable, ou
    franchissable deux fois.
  - **Deux constantes extraites avec la phrase qui les atteste.** La classe d'armure sans armure et
    le plafond d'une caractéristique — 10 et 20 — sont cherchés dans leur phrase du livre, et la
    phrase est écrite dans la donnée produite. C'est ce qui distingue une constante extraite d'une
    constante tapée de mémoire : la seconde a l'air de la première.
  - **Le `20` que le `LOT-36` avait codé en dur est parti.** `abilityScoreWith()` bornait une
    augmentation d'espèce à une constante ; le plafond est désormais un paramètre lu dans la
    donnée, et le test du `LOT-36` a été repris pour le lire **au même endroit que le moteur** —
    sinon il vérifierait sa propre copie de la règle.
  - **La fiche est un objet autonome, jamais un singleton joueur.** Le test construit **quatre**
    fiches, en blesse une, en fait monter une autre de deux niveaux, et vérifie que les deux
    dernières n'ont pas bougé : si `CharacterSheet` devenait un singleton, ce cas tomberait le
    premier. Le bonus de maîtrise n'y est d'ailleurs pas stocké — il se lit dans la table au niveau
    courant, sans quoi une montée de niveau laisserait un personnage avec le bonus de l'ancien.
  - **Le composant ECS ne porte pas la fiche, il la désigne.** Une fiche n'appartient pas à une
    entité : un personnage garde la sienne quand il change de carte et que son entité est détruite
    puis recréée. `INDICE_ABSENT` distingue une entité **sans** fiche d'une entité liée à la
    première du registre — les confondre ferait attaquer un tonneau avec les caractéristiques du
    héros.
  - **Trois décisions de règle, écrites là où on les lit.** Les points de vie sont
    **déterministes** (le livre laisse le choix ; des PV tirés au dé rendraient une partie
    irrejouable) ; monter de niveau **n'est pas un soin**, les PV courants montent du gain et non
    jusqu'au maximum ; et **perdre de l'expérience n'est pas une règle de ce jeu**, un gain négatif
    est ignoré plutôt que d'aboutir à une descente de niveau silencieuse.
  - **Le lot n'a pas écrit de `ClassDefinition`** : le `LOT-36` l'avait déjà livrée sous le nom de
    `PlayableClass`. Un second type pour la même chose aurait créé deux vérités sur ce qu'est une
    classe.
  - `ctest` passe de 983 à **992** cas, tous verts.

- **Espèces, historiques et classes provisoires** (`LOT-36`). De quoi construire un personnage
  jouable au plus tôt : **22 espèces**, **13 historiques** et les **4 classes simplifiées** qui
  serviront de socle au premier modèle de combat — 39 fichiers tirés de **trois documents et deux
  langues**, et c'est ce mélange qui fait la difficulté du lot.
  - **Le *Manuel des Joueurs* est un scan, et sa graisse ment.** La méthode du `LOT-33` — la
    graisse porte la structure — n'y tient pas : « Vitesse. » ne porte aucune graisse, « Âge. » en
    porte sur deux fragments non contigus, et les titres sont mutilés (`TaiJJe`,
    `Vision dans Je noir`, `tliaumaturgie`). Deux parades : les mécaniques se lisent par **leur
    phrase** et non par leur titre, et les noms de traits viennent du **lexique**, qui les porte
    proprement — la parade que le `LOT-43` employait déjà pour les dons.
  - **Le recoupement a servi dès la première exécution.** Les augmentations de caractéristique
    figurent deux fois dans le *Manuel* : dans le bloc de la race et dans la table de la page 12.
    L'OCR a **entièrement effacé** la ligne d'augmentation du demi-elfe — son bloc commence au
    milieu d'une phrase — et c'est la table qui la restitue. Une valeur présente des deux côtés et
    différente **arrête** la génération ; une valeur présente d'un seul côté est une ligne
    escamotée, rapportée et non fatale.
  - **Quatre corruptions d'OCR déclarées une par une** : `!'ore` pour « l'orc » (le `l` ressort en
    point d'exclamation, le `c` en `e`), `commwi` pour « commun », `(+l)` pour `(+1)` sur toute la
    table. Sans les deux premières, le demi-orc ne parle que le commun et le tieffelin pas du tout.
    Une substitution non déclarée serait indiscernable d'une règle du jeu.
  - **La gouttière du *Manuel* bouge d'une page à l'autre** — `[288, 309]` p. 41, `[272, 296]`
    p. 42, rien du tout p. 44. Un blanc figé y couperait tantôt dans une colonne, tantôt dans
    l'autre. La coupe est désormais **mesurée** page par page.
  - **Le *Player's Guide* dessine ses titres deux fois**, à la coordonnée exacte : un titre
    contourné, dont le remplissage et le trait forment deux passes. Invisible à l'écran, et cela
    double tout ce qui se compte — les douze espèces du chapitre 1 s'y relèvent vingt-quatre fois.
  - **Ce que le schéma ne peut pas dire n'est ni jeté ni inventé.** Les espèces de Tanares laissent
    une augmentation **au choix du joueur** : la partie fixe entre dans la table, le mécanisme est
    déclaré (`EX-CNT-030`) et listé au chargement (`EX-CNT-031`). Le **soulborn**, lui, hérite sa
    taille et sa vitesse des parents du personnage et n'en a donc aucune ; le schéma les exige, et
    l'espèce est **écartée en le disant** plutôt que dotée de valeurs inventées.
  - **Le corpus dit douze espèces de Tanares, pas treize**, et son chapitre des historiques en
    annonce six pour en porter sept — le sommaire fait foi, il indexe ce que le livre contient.
    Tanares ne porte d'ailleurs **aucune mécanique** pour les huit espèces classiques : le fond
    narratif vient de lui, la mécanique des livres français.
  - **Les quatre classes sont provisoires et le déclarent**, avec un critère de retrait écrit
    d'avance et à un seul endroit. Un test balaie tout `Source/Elements/Rpg/` et vérifie qu'aucune
    donnée définitive ne les référence : le jour du retrait, supprimer ces fichiers ne cassera rien.
  - **La progression du niveau 1 au niveau 5 vient de la donnée.** La formule générale donnerait le
    même résultat, et c'est le piège : l'écrire en C++ ferait cesser de lire la table, et la
    première classe dont la progression sort de l'ordinaire passerait inaperçue.
  - **Un seuil de corps se pose sous la valeur mesurée, jamais dessus.** Le corps rendu par un PDF
    est un flottant : 16 s'y lit 15,999998. Un seuil à l'égalité laissait passer le titre de
    chapitre et ratait les quatre races — sans erreur, sans message, avec un catalogue à une entrée.
  - Trois modules partagés sortent de ce que le `LOT-33` avait écrit pour lui seul — la grille à
    deux colonnes, la relecture des catalogues livrés, la déduplication des lignes surimprimées ;
    le bestiaire est reposé dessus et produit une sortie **identique à l'octet près**.
  - `ctest` passe de 974 à **983** cas, tous verts.

- **Le bestiaire de base : les 94 bêtes du SRD** (`LOT-33`). `Animaux.pdf` vers
  `Source/Elements/Rpg/creatures/` — 94 fichiers, 136 traits, 135 actions dont 115 portent des
  dégâts typés. Le premier catalogue rempli du projet, et le premier que le moteur charge.
  - **L'extraction se fait sur la typographie, pas sur des expressions régulières.** Un bloc de
    statistiques n'a ni balise ni ponctuation qui sépare le nom d'un trait de sa description :
    seule la graisse le fait — « **Vue aiguisée**. L'aigle a un avantage… ». Découper au premier
    point donne « Attaque au corps à corps avec une arme : +4 au toucher, allonge 1,50 m » comme
    nom d'action. `Extracteur.lignes()` rend désormais police, corps et graisse, et sept
    discriminants **mesurés** — pas devinés — découpent le document : le corps du titre, la police
    du paragraphe d'ambiance, celle des encadrés « Variante », l'interligne. Le document ne porte
    aucun interligne entre 11,3 et 15,2 pt ; le seuil tombe dans ce vide.
  - **Le mode texte perd des espaces, et la faute est indétectable en aval.** Les titres de traits
    en sortent collés — `Vueaiguisée`, `Sens dela toile`, `Tactiquedegroupe`. Aucun contrôle ne la
    rattrape et aucune relecture de la donnée produite ne la signale, puisque la donnée produite
    *est* la faute. Le fragment de police, lui, porte le texte tel que le document l'écrit.
  - **Le sommaire est le point d'attestation.** 94 entrées page 2, 94 titres dans le corps, et les
    deux listes doivent coïncider nom pour nom : c'est le seul contrôle qui détecte un bloc sauté,
    panne qui ne laisse aucune autre trace — un catalogue de 93 créatures se charge, se valide et
    se joue exactement comme un de 94. La gouttière des deux colonnes, elle, est re-vérifiée page
    par page : si elle bouge, la coupe **échoue** au lieu de mélanger deux créatures.
  - **Ce que le schéma ne peut pas dire n'est ni jeté, ni élargi en silence.** « Résistance aux
    dégâts contondants provenant d'attaques **non magiques** » : mettre `bludgeoning` dans
    `damageResistances` rendrait le diablotin résistant à une masse d'armes ordinaire. La clause
    qualifiée reste dans un trait, et la créature **déclare** le mécanisme
    `resistance-conditionnelle` (`EX-CNT-030`). Même traitement pour « comprend le commun mais ne
    peut pas le parler ». Et « l'**aérien** », que l'aigle géant comprend, n'est ni au catalogue
    des seize langues ni à la table des *Basic Rules* p. 38 : le rapprocher du primordial serait un
    élargissement muet, la génération le **signale** et ne l'écrit pas.
  - **Cinq défauts du lexique mis au jour par les 94 noms.** Les variantes séparées par `/`
    (« Bec de hache / Autrache ») — défaut qui se propage aux catalogues livrés, où
    « Tromperie / Supercherie » faisait désigner au diablotin une compétence introuvable ; « Zombi
    Objets magiques D&D 5 », un titre de section happé ; « Tigre à dents de **sabe** », une
    coquille ; l'entrée « Tigre » dont le côté **anglais** est resté en français, qui sortait un
    identifiant `tigre` au milieu de quatre-vingt-treize identifiants anglais ; et la langue des
    elfes nommée « elfe » quand le livre écrit « **elfique** ». Ce dernier était déjà tranché par
    le `LOT-43` : sa table d'alias est **réutilisée**, pas recopiée. Le quatrième ne se détecte pas
    mécaniquement — « Quasit = Quasit » est identique des deux côtés et juste — et c'est une
    relecture des 94 identifiants qui l'a trouvé.
  - **Le test lit le livre, pas la génération.** Douze profils sont rejoués contre des valeurs
    recopiées à la main du PDF, chacun avec sa page imprimée. Un test qui comparerait la sortie de
    la génération à elle-même passerait quelle que soit la faute d'extraction.
  - **Un champ `description` au schéma de créature** : 21 blocs se terminent par un paragraphe
    d'ambiance qui ne porte aucune règle et qui est pourtant ce que le bestiaire affichera. Le
    premier remplissage d'une famille est le moment où le contrat rencontre la réalité.
  - `core::CreatureSize` rejoint les énumérations fermées, confrontée au **schéma** et non au
    lexique : celui-ci n'en porte que cinq, « Moyenne » ayant échappé à l'extraction du glossaire.
  - `ctest` passe de 968 à **974** cas, tous verts.

- **Le cœur chiffré : dés, caractéristiques, jet de d20** (`LOT-12`). Du calcul pur dans `Core`,
  sans fenêtre ni GPU : la notation `NdF±M`, les six caractéristiques, le d20 avec avantage et
  désavantage, et les six degrés de difficulté en **donnée**.
  - **Le défaut que les tests ont trouvé et que la relecture n'aurait pas vu.** `nextInt` rejette
    la queue de l'intervalle pour éviter le biais modulo ; la formulation naturelle — rejeter
    au-delà du dernier multiple complet — donne un seuil de 2³² quand l'étendue divise 2³², qui ne
    tient pas dans un `uint32` et retombe à **0** : boucle infinie. Le défaut ne se voit que sur
    les **puissances de deux** — un d6 et un d20 passent, un d8 gèle. C'est le test de
    rejouabilité, écrit sur 3d8, qui l'a attrapé. La version retenue rejette la queue basse, vide
    dans ce cas.
  - **Deux arrondis qui ne se voient pas.** `(score - 10) / 2` tronque vers zéro en C++ : un score
    de 7 donnerait `-1` au lieu de `-2`, et le personnage serait moins pénalisé qu'il ne doit
    l'être sur chacun de ses jets, pendant toute la partie. Et avantage plus désavantage
    **s'annulent** (`EX-DND-002`), y compris à deux contre un — la règle annule, elle ne compte
    pas. Les deux ont leur cas de test explicite.
  - **Un 20 naturel n'est pas un total de 20** : `isNaturalTwenty()` regarde le dé retenu. Les
    confondre rendrait critique un jet sur deux à haut niveau, et passerait pour de l'équilibrage.
  - **La restitution est une exigence, pas un journal** (`EX-DND-003`) : les **deux** dés sont
    conservés en cas d'avantage, et chaque modificateur porte son origine —
    `d20 (avantage : 7, 14) = 14 + 3 (Dexterite) + 2 (maitrise) = 19 >= 15 : reussite`.
  - **Les degrés de difficulté sont une donnée** (`EX-DND-021`), extraits de la table « Tâche / DD »
    des *Basic Rules* : six paliers de 5 à 30. Le test lui-même lit le seuil dans le fichier plutôt
    que d'écrire `15`.
  - **Une case vaut 1,5 m**, figé dans une constante nommée. La conversion mètres ↔ cases existe
    forcément quelque part ; le seul choix ouvert était *à un endroit, ou à trente*.
  - `ctest` passe de 954 à **968** cas, tous verts.

- **Les options de personnage : dons, multiclassage, compétences, langues** (`LOT-43`). Quatre
  catalogues oubliés du premier découpage, que la fiche de personnage suppose sans jamais dire d'où
  ils viennent : **18 compétences**, **16 langues**, **42 dons** et la règle du multiclassage — 77
  fichiers de données, quatre schémas, et le mécanisme C++ qui cumule les emplacements de sorts.
  - **Le multiclassage vient du *Manuel des Joueurs***, seule source complète : les *Basic Rules*
    n'en portent ni les prérequis ni les maîtrises et renvoient au chapitre 6. Les douze prérequis
    de caractéristique et les douze lignes de maîtrises en sortent proprement.
  - **Sa table d'emplacements est recoupée**, et il le fallait : l'OCR y efface les cellules valant
    `1`, onze lignes sur vingt amputées. La donnée est prise sur la progression du magicien des
    *Basic Rules* — identique et en texte natif — puis confrontée cellule à cellule au *Manuel* :
    **26 cellules rétablies**, annoncées à chaque génération. Une divergence qui ne serait pas un
    `1` manquant **arrête** la génération.
  - **Les deux sources françaises ne traduisent pas les mêmes dons pareil** : « Adepte des
    éléments » contre « Adepte élémentaire », « Ritualiste » contre « Magie rituelle » — douze dons
    sur quarante-deux. Le lexique fait autorité, la graphie du livre est conservée en `variantes`.
    Les noms viennent d'ailleurs du lexique et non du livre, dont les titres sont des scans
    mutilés : `DouÉ`, `E(PLOR.) __ TEUR DE DONJONS_`.
  - **« Sorcier » n'est pas *sorcerer***. Le *Manuel* appelle ainsi la classe que le lexique nomme
    « occultiste » (*warlock*), alors que l'**ensorceleur** est deux lignes plus haut dans la même
    table. Un rapprochement par ressemblance aurait interverti leurs prérequis en silence ; l'alias
    est déclaré, avec la raison.
  - **Le meilleur test du multiclassage est celui du livre** : *« ce rôdeur 4/magicien 3 […] quatre
    emplacements de niveau 1, trois de niveau 2 et deux de niveau 3 »*. Le test le reproduit et lit
    la table **livrée** — c'est le livre qui vérifie l'implémentation. Deux pièges sont couverts :
    l'arrondi se fait **par classe** (paladin 3/rôdeur 3 donne 2, pas 3) et la **magie de pacte est
    exclue** de la somme (`EX-RPG-052`).
  - **Les 42 dons sont livrés `narratif` et `provisoire`**, critère de retrait écrit d'avance
    (`EX-CNT-032`) : aucun mécanisme de don n'existe encore, et un don qui se présenterait comme
    jouable coûterait plus cher à diagnostiquer qu'un don déclaré non joué.
  - **Toute langue citée par une créature ou une espèce doit exister au catalogue** — nouveau
    contrôle en CI, vérifié par injection d'une créature parlant le « draconien ».
  - `ctest` passe de 948 à **954** cas, tous verts.

- **L'OCR ne corrompt pas seulement les nombres, il en supprime** (constat de préparation du
  `LOT-43`). Sur la table du multiclassage du `Manuel-Des-Joueurs`, l'extraction **efface toute
  cellule valant `1`** : onze lignes sur vingt amputées, et un magicien de niveau 20 y perd ses
  emplacements de niveau 8 et 9. Une valeur fausse finit par se voir ; une valeur absente ressemble
  à une case vide légitime, et cette table en contient de vraies — aucune relecture ne pouvait
  l'attraper. Le défaut a été révélé par **recoupement** avec la table du magicien, identique et en
  texte natif propre dans les *Basic Rules*. La §4 de la feuille de route porte désormais ce
  quatrième niveau de bruit, et la règle qui en découle : *une table numérique tirée d'un scan se
  recoupe contre une seconde source ou contre un invariant.*

- **Les schémas de données RPG** (`LOT-32`). Le contrat **avant** les données : onze schémas JSON,
  un validateur en CI et trois énumérations C++, livrés alors qu'aucune donnée n'existe encore.
  C'est l'ordre qui compte — un contrat écrit après coup se contente de décrire ce qui a déjà été
  produit, défauts compris.
  - **Onze schémas** sous `Source/Elements/Rpg/schema/` : les dix familles annoncées — créature,
    objet, arme, armure, sort, espèce, classe, historique, état, type de dégâts — plus
    `common.schema.json`, qui porte ce que toutes réutilisent. Le champ `source` y est obligatoire
    (`EX-CNT-001`, `EX-CNT-002`), et `additionalProperties: false` partout : sans lui,
    `weigthGrams` au lieu de `weightGrams` passe sans un mot et l'arme pèse zéro.
  - **Un triangle, trois artefacts, deux contrôles.** Le moteur (`core::DamageType`), le contrat
    (`common.schema.json`) et la table de traduction (`rpg.glossary.csv`) nomment les mêmes choses.
    `test_rpg_enums.cpp` compare le C++ au schéma **livré** ; `check_rpg_data.py` compare le schéma
    au lexique. La troisième arête n'est **pas** contrôlée, par transitivité : un troisième contrôle
    serait bruyant, et le jour où deux des trois échouent ensemble on ne saurait plus lequel dit
    vrai. Vérifié par injection — ajouter `"sonic"` au seul schéma fait échouer les deux, chacun
    avec son message.
  - **Les trois énumérations fermées** : `core::DamageType` (13), `core::Condition` (15),
    `core::MagicSchool` (8), avec `switch` exhaustif sans `default`, sur le patron de
    `core::tileTypeName`. Leurs cardinaux sont écrits dans un test : le test de coïncidence
    resterait vert si les deux côtés perdaient la même valeur, celui-ci non.
  - **Le validateur s'auto-teste**, faute de données à valider : trois fixtures valides à accepter,
    **dix invalides à refuser**, chacune nommée d'après son défaut — provenance absente, type de
    dégâts en français, `ld8` au lieu de `1d8`, caractéristique manquante, champ mal orthographié,
    donnée provisoire sans critère de retrait, JSON tronqué… Chacune produit **exactement une**
    violation, située au fichier et à la ligne (`EX-CNT-010`).
  - **Deux unités internes uniques** : prix en pièces de **cuivre**, poids en **grammes**, entiers.
    Le corpus mélange « 500 g » et « 2 kg », « 2 pa » et « 25 po » ; convertir à l'entrée évite les
    arrondis là où l'encombrement se calcule par somme.
  - **Le formalisme des dés est contraint par expression régulière** — c'est la parade au risque
    résiduel que le `LOT-30` avait laissé ouvert : un `1d8` devenu `ld8` à l'OCR est invisible à la
    relecture et fatal à l'exécution.
  - `ctest` passe de 943 à **948** cas, tous verts.

- **La chaîne d'extraction du corpus, et le lexique bilingue** (`LOT-30`). Premier lot de la
  filière contenu : l'outillage qui tirera des huit PDF de `Documentation/SourceBook/` les 176
  créatures, l'équipement, les sorts, les espèces et les dix régions du jeu — puis sa première
  sortie, qui l'éprouve.
  - **`scripts/sourcebook/`, sur PyMuPDF** : manifeste des huit documents (empreinte SHA-256,
    pagination, provenance), texte, tableaux **par coordonnée**, images **par rendu clippé**, cache
    indexé par empreinte, et une ligne de commande — `info`, `verifier`, `texte`, `tableau`,
    `image`, `regions`, `stats`, `glossaire`.
  - **La double page est confirmée** : la page PDF 50 des deux livres Tanares porte les pages
    imprimées 100 et 101. Les six autres décalages ont été relevés un par un, et deux ne valent pas
    zéro. Une empreinte qui ne correspond plus **arrête** l'extraction (`EX-CNT-020`) : poursuivre
    produirait des données décalées sans qu'aucun message ne le dise.
  - **Le tableau par coordonnée tient sur un scan.** La table de progression du barbare du
    `Manuel-Des-Joueurs`, en OCR, donne bien « Attaque supplémentaire » au **niveau 5**, là où un
    rendu en flux décale toute la colonne d'un cran (`EX-CNT-021`). C'était l'affirmation la plus
    risquée de l'analyse du corpus.
  - **Le lexique : 2 084 entrées**, et non « environ 1 200 » comme l'annonçait la feuille de route
    — une estimation à l'œil, fausse de 74 %. La page est corrigée.
  - **Le glossaire porte de vrais homonymes**, ce qui a changé la clé d'unicité : `light` vaut
    « légère » comme propriété d'arme et « Lumière » comme sort, `bane` « Fléau / Imprécation »
    comme sort et « Baine » comme divinité. Dédupliquer sur l'anglais seul en écrasait un des deux
    en silence — et faisait traduire un sort par un adjectif d'arme. La clé est le couple
    **(anglais, catégorie)**.
  - **Le complément des *Basic Rules* est attesté, pas saisi.** Les huit écoles de magie, la
    propriété `special` et la quinzième condition (`exhaustion`) manquaient ou n'étaient pas
    catégorisées. Chaque ajout déclare la page où il est attesté, et la construction échoue si le
    terme ne s'y trouve pas — un complément tapé de mémoire est une donnée inventée qui a
    l'apparence d'une donnée extraite. Au passage, le lexique fige un faux ami : l'école
    `conjuration` se dit **« invocation »**.
  - **`scripts/check_glossary.py`, en CI, s'auto-teste avant de se prononcer.** Aucune clé de règle
    n'existe encore : le contrôle serait vert par vacuité, et personne ne saurait s'il fonctionne —
    la panne exacte du `LOT-78`. Six catalogues fictifs le mettent à l'épreuve à chaque appel. La
    comparaison des traductions ignore la casse mais **pas les accents** : une table d'autorité
    française qui accepte « etourdi » pour « étourdi » n'impose plus rien.
  - **Le corpus n'était pas exclu du dépôt**, contrairement à ce que la feuille de route affirmait
    depuis son écriture : la règle `.gitignore` vivait comme modification locale non commitée sur un
    seul poste, et un `git add -A` sur un clone neuf embarquait les 280 Mo. Elle est commitée, avec
    le cache d'extraction et les worktrees d'agent (1,6 Go), et `check_glossary.py` vérifie
    désormais l'exclusion (`EX-CNT-023`).

- **La feuille de route dit enfin par quoi commencer.** Elle portait un « ordre d'exécution
  recommandé » en cinq lignes de *quand* flous — « démarrables maintenant », « avec `LOT-09` »,
  « avant `LOT-13` » — dont aucune ne désignait un premier lot. La question « et maintenant ? » se
  répondait donc par une relecture de 2 500 lignes, et deux relectures ne donnaient pas forcément
  la même réponse.
  - **Une règle, à la place d'un avis** : *à chaque pas, parmi les lots dont tous les prérequis
    sont faits, celui qui en débloque le plus* — à égalité, le plus petit numéro. « Débloque » se
    compte : le `LOT-30` débloque quarante-neuf des soixante-neuf lots restants, le `LOT-49` aucun.
    « Outillage et contrats ; le plus tôt est le mieux » cesse ainsi d'être un avis éditorial pour
    devenir ce que le graphe dit, chiffre à l'appui. Un premier essai avait pris le **plus petit
    numéro** comme critère : déterministe, mais bête — il plaçait deux lots de moteur devant la
    chaîne qui doit leur fournir leurs catalogues.
  - **Un tableau d'avancement en tête de page**, calculé et non écrit : les soixante-neuf lots
    restants dans l'ordre, avec ce que chacun débloque et son statut — `prochain`, `prêt` (tous
    ses prérequis livrés), `en attente`. `scripts/lint_lots.py` gagne une **règle 13** qui le
    recalcule et le refuse s'il diverge d'une ligne. Le tableau ne figure qu'à un endroit : le
    recopier en section 6 aurait recréé les deux documents divergents que cette page combat.
  - **Le regroupement d'intention est conservé** en section 6, sous son propre titre. Il ne donne
    pas l'ordre — le tableau d'en-tête le donne — mais la *raison* de chaque placement, et c'est
    la seule chose qu'un calcul ne produira jamais : un graphe dit qu'un lot en attend un autre,
    il ne dit pas pourquoi on a voulu ce lien.
  - **Le lint ne voyait pas la moitié du graphe.** Les vingt et un lots absorbés (`LOT-09` à
    `LOT-29`) écrivent leurs prérequis dans un bloc de citation, `> Prérequis : …`, quand les lots
    de la filière les écrivent en italique ; le lint ne lisait que la seconde forme. Ni le contrôle
    d'acyclicité ni celui des prérequis existants ne les avait donc jamais examinés.
  - **Ce que le calcul a révélé** : le `LOT-09`, plus petit numéro restant, n'attend que des lots
    livrés à la lecture de sa seule ligne « Prérequis » — mais le `LOT-37` déclare l'alimenter, le
    graphe de cartes attendant l'atlas des régions sans quoi il relierait des nœuds inventés. Il
    tombe au rang 33. Un prérequis compte quel que soit le côté où il est déclaré, et c'est aussi
    ce qui repousse le `LOT-27` au rang 20 : il ne déclare rien, cinq lots de contenu déclarent
    l'alimenter.
  - **Le lien `LOT-38` / `LOT-39` était déclaré à l'envers**, et le calcul l'a rendu visible : la
    plomberie des clés d'assets se disait prérequis de la maquette de fiche, au motif qu'elle lui
    fournirait ses panneaux de parchemin — mais ces panneaux étaient partis au `LOT-76` lors du
    même audit, et la ligne « Prérequis » n'avait pas suivi. L'ordre réel est `LOT-38` puis
    `LOT-39` : on dessine la maquette, *puis* on nomme les clés de ce qu'elle affiche. La section 6
    l'écrivait déjà en toutes lettres ; c'est la ligne « Prérequis », celle que le graphe lit, qui
    disait le contraire.
  - **Conséquence, et elle n'est pas anodine** : le chemin critique jusqu'au *vertical slice* passe
    de cinq à **huit lots**, et la fiche de personnage y entre — `LOT-30` → `LOT-32` → `LOT-43` →
    `LOT-36` → `LOT-13` → `LOT-38` → `LOT-39` → `LOT-27`. Aucune version de cette page ne disait
    que la fiche et sa maquette étaient sur le chemin critique. Le chemin est long parce qu'il est
    réel : le `LOT-39` produit le marqueur généré sans lequel le slice n'a rien à afficher.
  - **`lint_lots.py --regenerer`** réécrit les deux tableaux calculés. Les règles 10 et 13 savaient
    refuser un tableau qui a dérivé ; sans cette option, corriger une ligne « Prérequis » obligeait
    à recopier jusqu'à cinquante lignes à la main — le geste même qui réintroduit l'erreur qu'on
    vient de corriger.
  - **Pas de date pour la `0.1.0`, et la raison est écrite.** La cadence observée — onze lots en
    deux jours — placerait la version dans deux semaines si on l'extrapolait ; les lots livrés sont
    des lots de socle, ceux qui restent portent des catalogues de plusieurs centaines d'entrées.
    L'extrapolation est fausse, et la page le dit plutôt que de laisser le lecteur la faire.

- **Une seule routine de lecture JSON, et des tests paramétrés** (`LOT-79`). Le dépôt comptait
  **six** réimplémentations de `loadFromFile` — `SkinCatalog`, `AnimationCatalog`, `SoundCatalog`,
  `PixelPalette`, `LevelLoader`, `LevelSequenceLoader` — répétant la même séquence (`accept()` puis
  `parse()`, racine objet, version absente valant 1, version supérieure refusée), dont quatre
  redéfinissaient les **mêmes cinq catégories d'échec** sous quatre noms. La filière contenu
  s'apprêtait à en ajouter quinze.
  - `core::JsonDocument` porte l'enveloppe commune une fois pour toutes, et ne lève jamais
    (`EX-NFR-040`). Les six lecteurs y passent ; chacun garde son énumération publique et traduit
    depuis la catégorie partagée par un `switch` **exhaustif et sans `default`**, si bien
    qu'ajouter une catégorie d'un côté fait échouer la compilation.
  - **Un échec dit désormais où.** `nlohmann` ne rapporte qu'un décalage en octets, que les six
    lecteurs jetaient : le message était « JSON malformé », devant un catalogue de mille lignes.
    Il s'annonce maintenant `sounds.json:12:5 : …` (`EX-CNT-010`). C'est la seule différence qui se
    voit à l'usage, et c'est celle qui compte.
  - **Premiers tests paramétrés du dépôt.** `Source/Test/` ne comptait aucun `TEST_P` ni aucun
    parcours de dossier de fixtures. `Source/Test/Fixtures/Json/` en porte six — valide, tronqué,
    virgule en trop, racine tableau, version future, version non entière — et un test vérifie
    qu'aucune fixture du dossier n'est **orpheline** : un fichier qu'aucun test ne lit ne protège
    de rien. C'est la capacité qui compte plus que ces six cas : un bestiaire de 176 créatures se
    teste en balayant un dossier, pas en écrivant 176 `TEST`.
  - `nlohmann_json` devient une dépendance **PUBLIC** de `Core` : `JsonDocument.h` expose l'arbre
    parsé, donc la bibliothèque fait partie de l'interface et non de l'implémentation. L'éviter
    aurait demandé une façade typée par-dessus `nlohmann`, soit un second modèle d'arbre à
    maintenir pour ne rien gagner d'observable.
  - `ctest` passe de 927 à **943** cas, tous verts.

- **Les numéros de lots ne sont plus ambigus** (`LOT-78`). Ce dépôt est dérivé de `ProjectGaming`,
  livré après **74 lots** ; les deux numérotations repartent de `LOT-01`, et la feuille de route
  atteignant `LOT-84`, la plage héritée était **entièrement recouverte**. `LOT-54` désignait à la
  fois un atelier pixel art livré et le lot Magicien à faire.
  - **201 renvois** préfixés `LOT-H-NN` dans dix fichiers de spécification ; les 49 restants
    désignent de vrais lots de ce programme et gardent leur écriture nue. La convention est écrite
    en tête de `specifications.md`.
  - La classification s'est faite par couple **(fichier, numéro)**, pas fichier par fichier :
    `rendu-technique.md` cite le `LOT-07` courant (tri par Y) et le `LOT-H-08` hérité (caméra
    cadrant un tableau) à onze lignes d'écart. Deux signaux ont suffi, vérifiés contre les titres
    des epics archivés : le **temps du verbe** — aucun lot RPG au-delà du `LOT-08` n'est commencé,
    donc « Concrétisé en » ne peut désigner que l'hérité — et le **vocabulaire** (« tableau »,
    « dash », « parallaxe » contre « région », « fiche », « d20 »).
  - `scripts/lint_lots.py` gagne une **règle 12** : tout `LOT-NN` d'une spécification doit désigner
    un lot existant de ce programme. Sans elle, le travail se déferait au premier renvoi ajouté de
    mémoire, et il faudrait tout reprendre puisque rien ne dirait ce qui a déjà été classé.
  - **Le garde-fou a d'abord échoué en silence** : la règle 12 contenait un caractère de contrôle
    invisible dans son expression régulière, qui l'empêchait de jamais correspondre. Elle n'a été
    tenue pour acquise qu'après avoir vu le lint refuser un `LOT-99` délibérément injecté.

- **La moitié RPG de la spécification** (`LOT-77`). Cinq familles d'exigences étaient **fantômes** —
  `EX-CNT`, `EX-DND`, `EX-RPG`, `EX-CBT`, `EX-INV` — citées par une vingtaine de lots sans qu'aucun
  document ne les porte. Cinq documents les portent désormais, pour **69 exigences** :
  `regles-dnd.md` (le jet d20, la maîtrise, le temps et le repos, les conditions), `rpg.md` (la
  fiche comme agrégat dérivé, classes et ressources, progression, sorts), `combat.md` (tour, espace,
  attaque, agonie), `inventaire.md` (équipement, encombrement, monnaie) et `contenu.md` (provenance,
  contrats, extraction, ce qu'une donnée promet).
  - **La CI était rouge à dessein** depuis la réparation du lint (`FAMILY_REF_RE` captait enfin les
    références de famille entière, comme `EX-CNT-*`). Ce lot est ce qui la remet au vert, et c'était la
    seule façon légitime de le faire — pas une entrée ajoutée à la liste des exceptions.
    `lint_exigences.py` compte **339 exigences déclarées et 339 référencées**.
  - **Trois généricités décident de la faisabilité du programme**, et sont écrites comme telles :
    une ressource de classe est *une quantité, une cadence, ce qu'elle alimente* (`EX-RPG-021`) —
    rage, ki et second souffle sont la même structure, faute de quoi chacun des quinze lots de
    classes modifierait le code du repos ; ajouter une classe ne touche **aucun** fichier C++
    existant hors sa mécanique propre (`EX-RPG-023`) ; **plusieurs systèmes d'emplacements de sorts
    coexistent** (`EX-RPG-052`), parce que la magie de pacte en fournit un second récupéré au repos
    court, et que coder « les emplacements » au singulier obligerait à tout reprendre.
  - **Le catalogue sera complet avant le moteur**, et cela devait être écrit : une donnée déclare
    les mécanismes qu'elle exige (`EX-CNT-030`) et le moteur **refuse en le disant** ce qu'il ne
    sait pas honorer (`EX-CNT-031`). Une classe dont la ressource propre n'existe pas se signale au
    chargement plutôt que de se jouer en silence comme une classe ordinaire amputée.
  - La rubrique **« Exigences couvertes »** est posée sur les 25 lots qui les implémentent. Elle
    manquait partout, faute de familles à citer.

- **Audit de la feuille de route** (`0.1.0`). `roadmap-0.1.0.md` a été confrontée au dépôt et à
  elle-même : trois affirmations sur le dépôt étaient fausses, six comptes internes incohérents, et
  trois travaux annoncés n'avaient aucun porteur.
  - **Le *vertical slice* n'était plus un jalon précoce** : le `LOT-69` déclarait le Colisée en
    prérequis, ce qui plaçait le socle de classe et les seize classes **devant** le `LOT-27`, à
    rebours de l'argument de la page elle-même. Le `LOT-69` est réduit à la suppression de l'atelier
    pixel art ; l'édition dans la scène devient la cible du `LOT-11`, qui n'est pas commencé.
  - **Quatre fusions** (numéros retirés, jamais réattribués) : le lexique rejoint la chaîne
    d'extraction (`LOT-31` → `LOT-30`), le repos rejoint l'horloge (`LOT-71` → `LOT-70`), l'agonie
    rejoint les conditions (`LOT-73` → `LOT-72`), et le `LOT-48` — « volume long, sans jalon » — est
    dissous dans chaque lot de catalogue : la page écrivait qu'« un lot sans date de fin est un lot
    qu'on ne finit pas », puis en gardait un.
  - **Cinq scissions**, sur une règle unique — le code d'un côté, la donnée de l'autre :
    `LOT-37`/`LOT-80`, `LOT-40`/`LOT-81`, `LOT-41`/`LOT-82`, `LOT-45`/`LOT-83`, `LOT-47`/`LOT-84`.
  - **La collision de numéros est bien plus large qu'estimé** : 208 renvois `LOT-NN` ambigus dans
    **douze** fichiers de spécification, et non « six specs ». D'où le `LOT-78`.
  - `scripts/lint_lots.py` **refuse en CI** ce que l'audit a dû trouver à la main : cycle de
    prérequis, lien déclaré d'un seul côté, lot absent du tableau d'ordre, compte annoncé faux,
    exigence revendiquée par deux lots, tableau récapitulatif périmé, arête de diagramme que rien ne
    déclare.

- **Vocabulaire de terrain du RPG** (`LOT-08`). Neuf types de tuile là où le `LOT-01` avait laissé
  le strict minimum hérité : `Grass`, `Dirt`, `Sand`, `Water`, `DeepWater` (sols), `Wall`, `Cliff`
  (obstacles), `Bridge`, `Stairs` (passages). Il **ajoute** sans rien remplacer — le vocabulaire de
  puzzle du socle sert tel quel au RPG.
  - **La chaîne complète pour chacun**, dans le même lot : nom de format, catégorie de palette,
    libellés `fr` **et** `en`, couleur de repli dans l'atlas procédural, franchissabilité, test
    d'aller-retour. C'est la leçon la plus chère de l'héritage, où ajouter un type touchait
    « exactement la même chaîne de huit fichiers » et où l'un d'eux se faisait toujours oublier.
  - Trois garde-fous nouveaux la tiennent : chaque type a une couleur de repli **non noire et
    distincte** (le jeu affiche une carte sans aucun fichier d'image), chaque libellé de palette
    est **traduit dans les deux catalogues**, et la borne de l'énumération reste dérivée du dernier
    type.
  - **L'eau profonde arrête, l'eau peu profonde non** : c'est la seule distinction qui rende une
    rive jouable. `core::isSolid` est le seul endroit d'où la règle de nage la sortira le jour venu
    — plutôt que des tests « sauf si c'est de l'eau » parsemés dans le code.
  - Aucune **silhouette** n'est déclarée : le mécanisme conservé par le `LOT-01` découpe la matière
    qui n'occupe pas toute la case, ce qui décrivait des pentes et des arrondis. Le terrain d'une
    vue de dessus est carré par nature ; lui inventer des découpes serait du travail contre le
    genre.

- **Tri par profondeur : le monde en vue de dessus devient crédible** (`LOT-07`). Le personnage
  passe **devant** ce qui est au-dessus de lui à l'écran, **derrière** ce qui est en dessous.
  - **Ce que l'epic n'avait pas vu** : alimenter le `sortOrder` existant ne suffisait pas.
    `ComposedScene::sort()` triait par (calque, **texture**, `sortOrder`) — la texture *avant* le
    tri fin — et un personnage n'a jamais la texture d'un arbre : le regroupement écrasait l'ordre
    de profondeur. Pire, `Object` et `Player` étant deux calques distincts, le personnage passait
    **toujours** devant un objet, où qu'il soit.
  - Correctif : `Object` et `Player` forment une **bande de profondeur** commune, à l'intérieur de
    laquelle la profondeur tranche avant la texture (`EX-REN-018`). Le surcoût — des passes de
    dessin supplémentaires — est assumé : aucun ordre de calque ne peut rendre justes à la fois
    « derrière l'arbre du bas » et « devant l'arbre du haut ».
  - La profondeur se lit au **pied** du sprite, pas à son coin haut, et se quantifie au pixel
    (16 sous-divisions par unité) : deux sprites que l'écran ne peut pas départager ne doivent pas
    permuter au gré des arrondis flottants. Le tri restant stable, rien ne scintille.
  - Les tuiles d'une couche de **décor** (`LOT-04`) rejoignent la bande ; le sol reste sous tout le
    monde. `core::buildLevelScene` annonce désormais le **rôle** de la couche d'origine de chaque
    tuile — ce qu'elle *est*, pas son rang dans une liste que l'auteur peut réordonner.
  - **Caméra isotrope** : zone morte carrée (1,5 sur les deux axes, contre 1,5 × 1,0) et
    anticipation **vectorielle**, qui suit la marche sur les deux axes. Anticiper seulement à
    gauche et à droite était un reste du jeu de plateforme.

- **Le jeu est de nouveau jouable : déplacement top-down en 8 directions** (`LOT-06`). Referme la
  parenthèse ouverte par le `LOT-01`, où la physique de plateforme avait été retirée sans
  remplaçant. `core::TopDownMovementSystem` enchaîne intention → vitesse → balayage continu →
  position, **sans gravité** : aucun axe n'est privilégié, `x` et `y` sont traités exactement de la
  même façon.
  - **La diagonale n'est pas plus rapide** (`EX-EXP-001`) : l'intention est normalisée avant d'être
    mise à l'échelle. Sans cela, aller en biais donnerait `√2 ≈ 1,41` fois la vitesse cardinale —
    le défaut le plus courant du genre, et le plus visible en jeu. Vérifié par un test, pas par une
    relecture.
  - Le balayage continu déjà en place (`core::sweepAabb`) fait le reste : aucune traversée de mur à
    vitesse absurde, glissement le long d'un obstacle pris en biais, et vitesse de l'axe bloqué
    remise à zéro — pousser contre un mur n'accumule aucun élan (`EX-EXP-002`, `EX-EXP-003`).
  - `core::Actor` remplace `core::Player` : **deux champs au lieu de trente**. Contact au sol,
    coyote time, jump buffering, dash, wall jump et combos décrivaient un personnage de plateforme
    et étaient **inertes** depuis le `LOT-01`. Restent l'orientation — un **vecteur**, parce qu'on
    regarde dans huit directions et que le sprite (`LOT-08`), l'interaction (`LOT-10`) et l'attaque
    (`LOT-21`) en dépendront — et la masse, seuil des plaques de pression.
  - Conséquences assumées de cette disparition : l'animation choisit son clip d'après la **norme**
    de la vitesse (marcher vers le haut est une marche), le HUD perd ses compteurs de sauts et de
    dashs, et la détection d'événements perd ses transitions de personnage (saut, atterrissage,
    glissade murale) — sans producteur, faute d'un état qui puisse les justifier.

- **Modes de jeu : l'ordre des passes sort de la session** (`LOT-05`). `hmi::GameSession` mêlait
  deux rôles — **orchestrateur** du pas fixe (monde ECS, caméra, événements, HUD, `FixedTimestep`,
  interpolation) et **mode de jeu** (l'ordre des passes lui-même). Tant qu'il n'y avait qu'un genre,
  la confusion ne coûtait rien ; le RPG a besoin d'au moins trois ordres — exploration, dialogue
  (`LOT-15`), combat (`LOT-18`) — qui se seraient entassés en `if` dans une fonction déjà longue.
  - `hmi::IGameModePasses` nomme les onze passes d'un pas fixe ; `hmi::IGameMode` les enchaîne.
    `GameSession` **implémente** les passes (héritage privé : elles sont offertes au mode, pas à
    l'appelant) et son `update()` ne fait plus que déléguer — aucun ordre codé en dur, aucun
    `if (mode == …)`, la sélection est polymorphe (`EX-ARCH-002`).
  - `hmi::ExplorationMode` est le premier mode, extrait **à comportement constant** du corps de
    `GameSession::update`. Aucune fonctionnalité ajoutée : mélanger un refactoring et une nouveauté
    ici aurait rendu indécidable lequel des deux avait cassé quoi.
  - L'interface des passes ne parle que de `core::` — pas de Qt, pas de GPU. C'est ce qui rend un
    mode **testable sans fenêtre**, quand `GameSession` exige un atlas, un lot de sprites et une
    police : le mode se vérifie contre des passes qui **enregistrent** la séquence des appels.
    `passOrder()` en fait une documentation exécutable, comparée à la séquence réelle par un test
    — un ordre modifié sans mettre la liste à jour échoue au lieu de mentir aux diagnostics.

- **Format de carte `version: 3` : couches, entités, propriétés libres** (`LOT-04`). Une carte
  n'est plus une grille plate unique mais **N couches typées** — sol, décor — superposées à la
  grille de collision, plus une **liste d'entités** (PNJ, coffres, panneaux, portails,
  déclencheurs). C'est la seule évolution de format structurante du programme, et elle précède
  toute production de carte : une carte dessinée sur le format plat serait à refaire.
  - **La collision reste le tableau racine `tiles`**, celui qui porte déjà l'entrée, la sortie et
    les mécanismes ; `layers` ne décrit que les couches **visibles**, et une couche `collision`
    déclarée est refusée avec un message qui renvoie à la racine. Deux grilles à tenir d'accord se
    désynchronisent, et c'est celle qu'on ne voit pas qui gagne (`EX-LVL-016`).
  - **Migration ascendante** : une carte `version: 2` se charge sans y toucher, sa grille promue en
    couche unique `legacy` — décor **et** collision, comme avant. Réécrite, elle ressort **sans**
    tableau `layers` : l'éditeur ne convertit pas un fichier dans le dos de son auteur.
  - **Propriétés libres et tolérance aux champs inconnus** (`EX-LVL-018`) — le point à ne pas rater
    du lot. Toute clé qu'une couche ou une entité porte sans que le chargeur la connaisse est
    conservée et **réémise**. Sans cela, les besoins du combat découverts en phase D (terrain
    difficile, couverture, hauteur) imposeraient un `version: 4` en plein milieu du programme, avec
    migration de tout le contenu déjà produit.
  - Le **brouillon d'édition** transporte couches, entités et propriétés sans encore savoir les
    modifier (`LOT-11`), redimensionne toutes les couches avec la carte, et écrit la grille éditée
    dans la couche de collision (`EX-EDIT-011`).
  - `LevelWriter::buildJson` prend désormais l'agrégat `core::LevelData` du `LOT-03` : les couches
    et les entités auraient porté sa liste positionnelle à onze paramètres, dont trois `vector`
    voisins interchangeables sans erreur de compilation.
  - `buildLevelScene` boucle sur les couches visibles et **ignore la collision** : un masque n'est
    pas une image. Le rang de la couche ordonne les sprites, le décor par-dessus le sol.

- **Agrégat `LevelData`** (`LOT-03`). Le constructeur de `core::Level` prenait ses composantes en
  **paramètres positionnels** — jusqu'à 19 avant le `LOT-01`, 11 après lui. Deux
  `std::optional<std::string>` voisins (`background`, `skinSet`) s'intervertissaient sans que le
  compilateur bronche, et le RPG s'apprête à rajouter des champs (couches de tuiles, entités,
  connexions de carte, zones de rencontre). La dette, actée dans l'en-tête lui-même depuis le
  `LOT-69` d'origine, est soldée alors que la liste est au plus court.
  - `core::LevelData` s'écrit avec les *designated initializers* de C++20 : chaque site de
    construction est lisible sans commentaire.
  - **`tileMap` n'a volontairement pas de défaut** : `core::TileMap` n'étant pas constructible par
    défaut, l'omettre est une **erreur de compilation**, jamais une grille vide silencieuse. Tous
    les autres champs ont un défaut utile — un site minimal tient en deux lignes.
  - Le constructeur positionnel est retiré d'emblée, sans l'étape `[[deprecated]]` prévue : dix
    sites d'appel seulement, la béquille coûtait plus qu'elle ne rapportait.

- **Fork : `ProjectGaming` devient `JustAnotherDnDGame`** (`LOT-01`). Le dépôt est dérivé du jeu de
  plateforme/puzzle en vue de côté livré en `0.1.3`, dont il reprend le moteur et retire tout le
  gameplay propre à la vue de côté, pour devenir un **RPG en vue de dessus** : exploration temps
  réel façon Zelda, rencontres en combat tactique au tour par tour régi par un système d20 maison.
  L'historique git est intégralement conservé ; le tag `archive/platformer-v0.1.3` marque l'état
  d'avant la purge, et le programme de lots d'origine reste consultable sous
  `Documentation/Heritage/`.
  - **Retiré** : le solveur d'IA autonome (`AiSolver`, 12 k lignes) avec son IHM, ses rejeux et son
    écran « Mode IA » ; la physique du personnage de plateforme (`CharacterPhysicsSystem`, seul
    consommateur de la gravité) ; les contrôleurs de blocs poussables, plateformes mobiles, dangers
    à état et blocs volatils ; la géométrie des pentes et des dangers directionnels ; l'outil
    « Parcours » et le panneau « Propriétés » de l'éditeur ; 25 des 36 types de tuile ; les 26
    niveaux de démonstration et leurs plans. Au total **63 000 lignes** et 376 fichiers.
  - **Conservé** : l'ECS, la boucle à pas fixe, les mathématiques déterministes, le balayage AABB,
    le modèle et le chargeur de niveaux, tout le rendu (atlas procédural, autotuilage, skins,
    caméras, particules, ombres, polices), l'éditeur (undo/redo, bibliothèque d'assets, atelier
    pixel art), l'IHM Qt, l'audio, la localisation et l'infrastructure CI/doc/test.
  - **Sauvé de la purge** : `GridDistanceField` (BFS de plus court chemin sur la grille) quitte
    `AiSolver` pour `Core/World/` — c'est le calcul de portée de déplacement du futur combat
    tactique. `MechanismController` (interrupteur↔porte, plaque, clé) est conservé : c'est du
    vocabulaire de puzzle, pas de plateforme. Le détourage par silhouette devient
    `HMI/Graphics/TileSilhouette`, vidé de sa géométrie mais fonctionnel, en attendant les tuiles
    du `LOT-08`.
  - **Conséquence assumée** : le personnage ne se déplace plus. Le jeu se lance, charge et affiche
    une carte, mais n'est **pas jouable** jusqu'au `LOT-06`, qui livre le déplacement top-down 8
    directions par-dessus le balayage AABB existant.
  - Renommage des cibles, du binaire et des macros (`PROJECTGAMING_` → `JADG_`) ; version remise à
    `0.1.0`.

- **Gameplay — blocs interactifs volatils** (`LOT-74`, `EX-GP-027` à `EX-GP-029`). Trois types de
  tuile pour de la matière qui ne survit pas au passage du personnage, là où le moteur n'avait
  jusqu'ici **aucun** bloc destructible, fragile ou disparaissant : une fois posé, un bloc était là
  pour toujours.
  - **Bloc descendant** (`EX-GP-027`, `sinkingBlock`) : armé par un contact quelconque — dessus,
    côté ou dessous —, il descend ensuite à vitesse constante en **portant** le personnage et les
    blocs poussables posés dessus, s'arrête définitivement contre la matière pleine, et quitte le
    niveau s'il franchit le bord bas. Position **continue**, comme une plateforme mobile.
  - **Bloc fragile** (`EX-GP-028`, `fragileBlock`) : détruit par un **ground pound**, et par lui
    seul. Le geste, livré au `LOT-72`, n'avait jusqu'ici **aucune cible** — l'exclusion était
    consignée deux fois dans le dossier de ce lot, faute d'un type de tuile à casser. La destruction
    est résolue **avant** la physique du pas, pour que le pound traverse la dalle qu'il brise au lieu
    de s'y arrêter d'un pas.
  - **Bloc éphémère** (`EX-GP-029`, `vanishingBlock`) : solide tant qu'on est dessus quelle que soit
    la durée, effacé un court délai après qu'on l'a **quitté** — un front de départ, jamais un simple
    contact. La disparition est **définitive** jusqu'au rechargement du tableau.
  - **Aucune passe de physique nouvelle.** Le bloc descendant émet des `core::PlatformSample`, le
    type que la physique consomme déjà pour les plateformes mobiles : portage, collision continue et
    interpolation d'affichage lui viennent de là, sans code de collision dédié. Les deux autres
    réutilisent la copie mutable du `TileMap` déjà tenue pour les portes.
  - **Trois tableaux de démonstration** (`demo-bloc-descendant`, `demo-bloc-fragile`,
    `demo-bloc-ephemere`), portant la séquence livrée de 22 à **25** tableaux.
  - **Dette soldée** : la borne « dernier énumérateur de `TileType` » était recopiée à la main dans
    **quatre** fichiers — dont deux qui documentaient leur propre fragilité, et un quatrième que
    seule la compilation a révélé. Elle n'existe plus qu'à un seul endroit
    (`core::TILE_TYPE_COUNT`).
  - **Écart de fidélité corrigé** : l'orchestration de référence des tests système n'appliquait pas
    les **capacités de tableau** (`EX-GP-055` : sauts aériens, charges de dash), alors que le jeu et
    l'environnement d'entraînement les appliquent tous deux. Sans conséquence tant qu'aucun tableau
    livré ne déclarait ces champs ; mis au jour par `demo-bloc-fragile`, qui déclare
    `dashCharges: 0`.
  - ⚠️ **Rupture** : l'observation de l'IA passe de **33 à 36 canaux**. La *forme* du tenseur
    d'entrée change, donc les modèles entraînés avant ce lot doivent être **réentraînés**. La
    *signification* des canaux existants est en revanche préservée : les trois nouveaux types sont
    ajoutés en **fin** d'énumération, jamais insérés au milieu.

- **Blocs volatils — correctifs de revue et habillage par défaut** (`LOT-74`). Cinq points relevés
  à la relecture du lot, plus les trois skins qui lui manquaient.
  - **Le ground pound traversait le sol.** La portée verticale de destruction d'un bloc fragile
    valait une case **entière**, avec une comparaison inclusive : des pieds posés sur une case
    pleine de la rangée *R* atteignaient donc un bloc fragile placé en *R+1*, de l'autre côté du
    sol, et un ground pound sur une pile de deux dalles les brisait **toutes les deux** d'un coup.
    La portée passe à **0,75 case** — elle couvre toujours la demi-case parcourue en un pas fixe à
    la vitesse de chute imposée, avec 0,25 case de marge des deux côtés. Trois tests de régression,
    dont un test *positif* qui verrouille la borne basse.
  - **L'agent voyait les blocs descendants à leur case de départ.** L'observation lit la grille de
    collision composée, où la tuile `sinkingBlock` restait figée à sa position de fichier : un bloc
    descendu de deux cases était vu là où il n'était plus, et la case qu'il occupait réellement
    paraissait libre. Nouveau `core::SinkingBlockController::collisionMap`, sur le même patron que
    celui des blocs volatils et des blocs poussables, qui reporte chaque bloc encore présent sur sa
    case **courante**. Sans effet sur la collision ni sur le champ de distances — `SinkingBlock`
    n'est pas `core::isSolid` : seule change ce que la grille **montre**.
  - **Deux copies de `TileMap` par pas en trop** dans la boucle d'entraînement
    (`aisolver::HeadlessLevelEnvironment::step`), la grille « mécanismes + blocs volatils » étant
    composée deux fois. Calculée une fois puis réutilisée, comme le fait déjà `hmi::GameSession`.
  - **Deux commentaires de documentation faux** dans `core::SinkingBlockController` : le paramètre
    `baseCollision` annoncé comme incluant les blocs poussables (aucun des trois appelants ne peut
    le fournir, `core::BlockController` s'exécutant *après* cette passe — un bloc descendant
    traverse donc un bloc poussable, comme il traverse un autre bloc descendant), et un
    `stopLimitAt` décrit comme retournant `std::nullopt` alors qu'il signale l'absence de limite par
    un paramètre de sortie.
  - **Habillage par défaut des trois nouveaux types.** `sinkingBlock`, `fragileBlock` et
    `vanishingBlock` n'étaient assignés dans aucun jeu de skins et retombaient sur le damier
    magenta. Trois dalles 16×16 générées par `scripts/generate_test_skins.py` — chevrons vers le bas
    en kaki, dalle rose fendue, dalle givrée à bordure pointillée et **partiellement transparente**
    — assignées dans le jeu `test` de `Assets/skins.json`. Silhouette commune et teintes reprises de
    l'atlas procédural de repli, pour que la lecture d'un tableau soit la même avec ou sans skins.

## [0.1.3] - 2026-09-01

> Neuvième jalon. Le solveur IA de la version précédente **apprenait mal** : chaque épisode était
> coupé au bout de ~215 pas sur les 3 000 disponibles, et la politique se figeait si complètement
> que 10 000 épisodes archivés produisaient une trajectoire *bit à bit identique* à partir du
> cinquantième. Trois causes, toutes dans l'**environnement** et aucune dans les algorithmes : une
> progression mesurée à vol d'oiseau vers une sortie dont la solution s'éloigne, un budget de pas
> plafonné sous ce que demande le tracé de référence, et une division `0 / 0` qui contaminait tout
> le vecteur d'observation sur les sept niveaux interdisant le dash. Corrigées, mesurées :
> `demo-wall-jump` passe de `0 %` à `98,5 %` de réussite.
>
> Le personnage gagne un **nuancier de mouvement** : dash chargé, poussée renforcée d'un bloc,
> ground pound, et un combo dash + saut avec jump-cancel, wall-jump en sortie et momentum hérité.
>
> Et l'**interface est reprise à la racine**, là où trois symptômes la rendaient pénible à
> l'usage. Une fenêtre qui réclamait plus grand que l'écran : aucune règle ne disait *qui décide
> de la taille de la fenêtre*, et la réponse était, de fait, le plus dense des écrans — dont la
> taille minimale était de surcroît multipliée par un facteur dérivé de la hauteur de cette même
> fenêtre, une boucle sans point fixe. Un gel de plusieurs secondes à chaque changement d'écran :
> le facteur d'agrandissement, qui ne concerne que les écrans du jeu, repolissait les 862 widgets
> de l'application. Un mode IA peu lisible, dont neuf réglages étaient lus puis **jetés** avant
> d'atteindre le moteur, pendant que la configuration du run affirmait le contraire. Les deux
> premiers défauts sont désormais tenus par des **invariants** portés par le chemin d'ajout commun
> des écrans et par deux feuilles de style disjointes — le défaut de taille s'était déjà produit
> deux fois, et avait été corrigé deux fois écran par écran.

- **IHM — un écran ne dicte plus sa taille à la fenêtre, et changer d'écran ne gèle plus
  l'interface** (`LOT-73`, `EX-IHM-080` à `EX-IHM-083`). Trois symptômes rapportés à l'usage —
  fenêtre réclamant plus grand que l'écran, gel de plusieurs secondes en navigation, Mode IA peu
  lisible et incomplet — remontaient à deux défauts structurels et un écart de périmètre.

  - **Un seul écran dense fixait le plancher de la fenêtre entière.**
    `QStackedWidget::minimumSizeHint` vaut le maximum sur *toutes* ses pages, masquées comprises ;
    ce plancher était de surcroît multiplié par le facteur d'agrandissement des écrans du jeu
    (jusqu'à 3×), lui-même dérivé de la hauteur de la fenêtre — une boucle sans point fixe, que rien
    ne bornait à l'écran (`availableGeometry` n'apparaissait nulle part dans le dépôt). Sur un
    affichage de `1920x1009` utiles, le menu principal réclamait `1030x1497` et le Mode IA
    `2280x1560` : Windows refusait la géométrie et l'interface débordait en rognant son contenu.
    Le défaut s'était déjà produit **deux fois**, corrigé deux fois écran par écran. `ScreenPageHost`
    porte désormais la garantie sur le **chemin d'ajout commun** — toute page défile plutôt que de
    rogner, et contribue zéro au minimum de la fenêtre. Le facteur est borné par la zone d'affichage
    disponible, la géométrie restaurée y est ramenée, et un dépassement est journalisé au démarrage.
  - **Une préoccupation d'écran repolissait les 862 widgets de l'application.** Le facteur
    d'agrandissement ne concerne que les écrans du jeu, mais vivait dans la feuille de style
    applicative : en changer coûtait **cinq secondes en configuration Debug**. Le regroupement
    introduit auparavant en atténuait la fréquence sans en réduire l'ampleur, et le faisait atterrir
    *après* le placement de la fenêtre — d'où le recalage visible. Le thème est scindé en deux
    feuilles disjointes (`theme-identity.qss`, `theme-editor.qss`), chacune posée à sa portée :
    l'identité sur la pile d'écrans, le châssis sur l'application. Plus rien à différer, donc plus
    de regroupement : le facteur s'applique dans le redimensionnement même.
  - **Neuf réglages du Mode IA étaient lus puis jetés.** `configFromForm()` remplissait le taux
    d'apprentissage du critique, le taux de croisement, les épisodes par lot, l'entropie,
    l'exploration minimale, l'écrêtage du gradient, les images par décision, le budget de pas et le
    seuil de blocage ; `TrainingRequest` n'avait aucun champ pour les recevoir. Les régler ne
    changeait rien, et le `config.json` du run affirmait le contraire — « Reprendre les réglages de
    ce run » rechargeait donc des valeurs fausses. La traduction requête → surcharges est extraite
    en fonction pure (`hmi::overridesFor`) et vérifiée champ par champ. L'écran ouvre par ailleurs
    sur les défauts du moteur : trois champs en divergeaient, si bien que « Réinitialiser aux
    défauts » les *changeait*.
  - **Le Mode IA devient hybride : enveloppe d'identité, contenu d'outil.** Seul écran de la portée
    identité à n'être pas un écran de joueur, il gardait la police bitmap et le facteur ×3 sur
    vingt-six lignes de formulaire et une table à huit colonnes. Il conserve fond, titre, cadre et
    bouton de retour ; son contenu passe à la densité d'un outil, aux couleurs du jeu.
  - **Le graphique se lit.** Trois graduations avec leurs valeurs, les bornes de l'intervalle de
    générations, et une quatrième courbe de **moyenne mobile** — que l'enregistreur calculait déjà
    et écrivait au CSV, mais ne transmettait pas à l'écran. Il ne se fait plus écraser par la table
    voisine (politique de taille déclarée), et les en-têtes des deux tables sont enfin dimensionnés.
  - **Garde-fou de parité des traductions** : les deux catalogues doivent déclarer exactement les
    mêmes clés, dans les deux sens. Une clé manquante d'un côté ne cassait rien de visible — le repli
    affiche l'autre langue — et ne se voyait qu'en changeant de langue, écran par écran.

  Non traité, consigné dans `LOT-73` TACHE-06 : la latence du **jeu** en configuration Debug (trace
  émise à chaque frame dans le chemin de rendu, puits de journalisation synchrones, absence de
  préréglage `RelWithDebInfo`) — famille de défauts distincte du gel d'interface corrigé ici.

- **Mode IA — l'apprentissage par gradient était bloqué par son environnement, pas par son
  algorithme.** Audit complet du solveur et de son écran. Cause racine, mesurée : sur
  `demo-final.json`, **chaque épisode était coupé au bout de ~215 pas sur les 3 000 disponibles**,
  et la politique se figeait — 10 000 épisodes archivés produisaient une trajectoire *bit à bit
  identique* à partir du cinquantième.

  - **Progression mesurée sur l'objectif immédiat, plus à vol d'oiseau vers la sortie.**
    `HeadlessLevelEnvironment::updateProgress` comparait la distance euclidienne à la sortie à son
    meilleur record. Sur un niveau dont la solution s'éloigne de la sortie — `demo-final.json`
    place sa sortie à trois unités de l'entrée, derrière trois portes dont la clé est à l'opposé —
    ce record était atteint dans les premières secondes et ne pouvait plus être battu : l'épisode
    était déclaré bloqué 200 pas plus tard, quoi que fasse l'agent. La détection lit désormais le
    **même champ de distances que la récompense** (`EX-IA-023`), et son record est réamorcé à
    chaque changement d'objectif.
  - **Budget de pas et seuil de blocage dérivés du niveau** (`AiSolver/Env/StepBudget.h`) : la
    chaîne d'objectifs est mesurée case par case, mécanismes compris, et convertie en budget. Le
    plafond fixe de `3 000` pas était inférieur aux ~`4 000` que demande le tracé de référence de
    `demo-final.json` — une politique parfaite y aurait expiré. Deux tests d'intégration
    (`test_budget_pas.cpp`) confrontent l'estimation aux tracés des niveaux livrés et **fixent** les
    constantes : le budget couvre chaque tracé avec la moitié de marge, et aucun tracé résolvant
    n'est jamais classé bloqué.
  - **Un budget de mouvements nul rendait l'observation `NaN`, et sept niveaux livrés en déclarent
    un.** `dashesRemaining / dashBudget()` valait `0 / 0` dès que le tableau interdit le dash — et
    un seul `NaN` dans le vecteur d'observation contamine toute la propagation avant : `tanh(NaN)`,
    puis un `softmax` entièrement `NaN`, dont le tirage d'action retombait invariablement sur la
    dernière action de l'espace. L'agent rejouait alors le **même** épisode à chaque fois, quelle
    que soit sa graine et quel que soit son entraînement. Mesure : **2 trajectoires distinctes sur
    1 500 épisodes** sur chacun de ces sept niveaux, contre plusieurs centaines partout ailleurs —
    et les deux qu'ils « réussissaient » ne prouvaient rien, cette action constante suffisant à les
    terminer. Effet de la correction seule, sur `demo-wall-jump.json` : de `0 %` à **`98,5 %`** de
    réussite. `decodeStochastic` refuse désormais bruyamment une distribution non finie, au lieu de
    la subir en silence.
  - **L'observation décrit enfin ce que la récompense mesure.** La fenêtre de tuiles lisait la carte
    *statique* du fichier : un bloc poussé y restait à sa case d'origine et une plateforme mobile
    n'était visible qu'à son point de départ. Elle lit maintenant la grille de collision courante,
    gagne un canal « plateforme mobile », et un `ObjectiveEncoder` expose le **gradient local du
    champ d'objectif** — sans quoi l'agent était payé pour se rapprocher d'un but dont rien ne lui
    disait la direction, et deux passages opposés dans le même couloir restaient indiscernables.
    Rayon de fenêtre porté de `2` à `3`. Conséquence assumée : les modèles entraînés avant ce
    changement ne se rechargent plus (refus net de `nn::loadWeights`, jamais silencieux).
  - **Le gradient ne s'effondre plus.** La perte de policy gradient gagne un **terme d'entropie**,
    les retours sont **centrés-réduits**, la norme du gradient est **écrêtée**, et une part
    d'uniforme est mélangée à l'échantillonnage (plancher d'exploration) — le terme d'entropie seul
    ne suffisait pas, sa dérivée s'annulant précisément quand la distribution est déjà saturée. Une
    action décidée est maintenue quatre images : tirer parmi 48 actions à 60 Hz produisait une
    marche aléatoire dont le déplacement espéré est nul.
  - **Défauts réaccordés, chacun sur une mesure** : optimiseur `adam` au lieu de `sgd`, taux
    `0,003`, couche cachée `64` au lieu de `16`, `gamma` `0,995`, taux propre au critique (`0,5` :
    sa sortie doit couvrir l'amplitude des retours, pas celle de logits). Croisement évolutionniste
    **uniforme** et non plus moyenne systématique des deux parents — moyenner deux réseaux ne
    combine pas ce qu'ils ont trouvé.
  - **Garde-fous numériques** : tri topologique de `autodiff::backward` rendu itératif et perte
    assemblée en arbre (une chaîne de profondeur égale au nombre de pas débordait la pile, à la
    rétropropagation comme à la destruction du graphe) ; `logOp` planché (une probabilité de
    `softmax` s'annule en flottant dès qu'une politique se spécialise, et `log(0)` contaminait tous
    les poids) ; terme de progression neutralisé quand une case est inatteignable, au lieu d'un
    écart de plusieurs centaines de points sur un seul pas.
  - Tout cela est réglable : `--max-steps`, `--stuck-threshold`, `--batch-episodes`, `--entropy`,
    `--exploration-floor`, `--grad-clip`, `--action-repeat`, `--crossover-rate`,
    `--critic-learning-rate`, et les champs correspondants dans l'écran.

  **Mesure** — REINFORCE, réglages par défaut, `1 500` épisodes par niveau, graine fixée. Taux de
  réussite de la politique **déterministe** (`evaluate --decoding argmax`, 30 répétitions) :

  | Résultat | Niveaux |
  |---|---|
  | **Terminé de façon fiable** (≥ 60 % sur les 100 derniers épisodes) | `demo-deplacement` 100 %, `demo-wall-jump` 100 %, `demo-interrupteur` 100 %, `demo-cle` 100 %, `demo-bloc-quart` 100 %, `demo-saut` 94 %, `demo-pente` 76 %, `demo-concave` 69 %, `demo-pente-gauche` 65 %, `demo-bloc` 64 % |
  | **Terminé parfois** | `demo-dangers-directionnels` 43 %, `demo-plaque-pression` 2 %, `demo-dangers-avances` 1 % |
  | **Jamais terminé** | `demo-double-saut`, `demo-dash`, `demo-mouvement`, `demo-bloc-reduit`, `demo-plafond`, `demo-plateforme`, `demo-budget`, `demo-synthese`, `demo-final` |

  **Dix niveaux sur vingt-deux, là où l'apprentissage par gradient n'en terminait aucun de façon
  reproductible** — les réussites précédentes sur `demo-pente` et `demo-plafond` n'étaient pas un
  apprentissage mais l'action constante décrite plus haut, et `demo-plafond` retombe d'ailleurs à
  `0 %` maintenant qu'il apprend réellement. `demo-final.json` reste hors de portée : quatre
  mécanismes en séquence et un itinéraire de près de `4 000` pas, pour une politique sans mémoire
  qui ne voit que sept cases autour d'elle. Ce qui est levé, c'est ce qui l'empêchait d'**essayer** ;
  ce qui reste est un problème d'architecture d'agent, pas d'environnement.

- **Mode IA — l'écran imposait sa taille à la fenêtre, qui débordait sous la barre des tâches.**
  Sa colonne de réglages était posée à nu dans l'onglet : sa hauteur minimale — vingt-six lignes de
  formulaire, multipliées par le facteur d'agrandissement des écrans du jeu, jusqu'à `3×`
  (`hmi::pixelArtScale`) — remontait jusqu'à la fenêtre, qui réclamait alors plus de `2 400` pixels
  de haut. Windows refusait la géométrie, la fenêtre débordait sous la barre des tâches et son
  contenu était rogné, sans moyen de faire défiler. Les deux onglets denses vivent désormais dans
  une zone défilante : **un écran s'adapte à la fenêtre, il ne lui dicte jamais sa taille.** Avec
  ses 94 lignes de formulaire, cet écran est le seul du jeu assez dense pour poser le problème —
  les autres en comptent au plus douze.

- **Redimensionner la fenêtre figeait l'application.** Le facteur d'agrandissement des écrans est
  dérivé de la hauteur de la fenêtre ; à chaque changement, la feuille de style de l'application
  était rejouée — ce qui **repolit les 862 widgets** de l'application et leur recalcule métriques,
  tailles et dispositions. Mesuré en Debug : **5 secondes par rejeu**. Or un glisser de bordure qui
  longe un seuil le franchit des dizaines de fois, et chaque franchissement empilait un rejeu
  complet : la fenêtre restait figée bien après que la souris ait été relâchée. Les rejeux sont
  désormais **regroupés** — seul le dernier facteur, celui qu'on voit à l'arrivée, est joué. Mesure
  sur vingt franchissements de seuil : **106,6 s de CPU avant, 10,3 s après**. L'aspect final est
  identique ; seul le chemin pour y arriver a changé. (En Release, le même rejeu est imperceptible :
  l'application y consomme 0 % au repos contre ~40 % en Debug pendant les vingt premières
  secondes.)

- **Crédits — trois sections sur six s'affichaient dans la mauvaise police.** Le thème ciblait les
  intitulés et les lignes de crédit par **nom d'objet**, et la liste n'avait jamais été étendue aux
  sections ajoutées après coup — polices, bibliothèques, licence. Leurs intitulés s'affichaient donc
  dans la fonte du corps de page au milieu d'intitulés en accentué, et leurs lignes de crédit en
  taille normale au milieu de lignes en légende. La sélection se fait désormais par **rôle**
  (`creditsRole`, propriété posée sur le widget dans `CreditsScreen.ui`) : une section ajoutée
  demain porte son rôle là où on l'écrit, au lieu d'attendre qu'on pense à l'inscrire ailleurs.

- **Mode IA — défauts de l'écran.** Fermer la fenêtre pendant une **évaluation** détruisait un
  `QThread` en cours d'exécution (le destructeur n'arrêtait que l'entraînement) ; un worker fuyait
  à chaque run (`deleteLater()` posté sur une boucle d'évènements déjà terminée) ; fermer la fenêtre
  pendant un entraînement figeait la fermeture sans rien dire, la confirmation n'étant câblée que
  sur la sortie de l'écran. L'algorithme d'évaluation était lu dans la liste des runs et non dans le
  modèle choisi — un modèle parcouru au disque était donc évalué avec la mauvaise topologie, ou
  silencieusement supposé évolutionniste. Le dossier de runs choisi pilotait l'écriture sans piloter
  la lecture. Le temps restant affichait « 0 min 00 s » pendant tout le run (division entière avant
  multiplication). Charger un fichier de configuration absent remettait **tout** le formulaire aux
  défauts, en silence. Le tableau de suivi n'avait aucun plafond de lignes. Six messages d'erreur et
  le libellé du graphique étaient en français en dur, quelle que soit la langue. `ScopedLogLevel`
  écrivait un journaliseur global non atomique depuis un fil de travail, et deux portées qui se
  chevauchaient laissaient le journal muet pour le reste de la session : la comptabilité vit
  désormais dans `Logger`, protégée et réentrante.

- **LOT-ANNEXE-22** — Mode IA, IHM complète d'entraînement. L'écran expose désormais **tous** les
  hyperparamètres lus par le moteur, et `aisolver-cli train` gagne les seize drapeaux qui lui
  manquaient (`--hidden-size`, `--tournament-size`, `--mutation-strength`, `--max-generations`,
  `--required-successes`, les huit `--dqn-*`) — plus `--max-steps`, `--seed` et `--decoding` sur
  `evaluate`. Correction notable : le champ « Épisodes / générations max » **ne pilotait rien** pour
  l'algorithme évolutionniste (`StoppingConfig` restait aux défauts, quel que soit l'affichage), et
  la barre de progression était de ce fait indéterminée. Trois indicateurs présents dans l'écran
  mais jamais alimentés — générations stables, epsilon courant, temps restant — sont branchés, et le
  tableau de suivi affiche les huit colonnes du `stats.csv` au lieu de quatre. L'évaluation passe
  sur son propre thread (fenêtre réactive, progression, annulation), accepte un niveau et un modèle
  libres (évaluation croisée), exporte son rapport CSV, et « Exporter comme rejeu » refait réellement
  jouer le modèle au lieu de copier un fichier. Quitter l'écran pendant un entraînement demande
  enfin confirmation.

- **LOT-72** — Mouvement avancé : **dash chargé** (bouton de dash et direction opposée maintenus,
  `EX-GP-056`), **poussée renforcée** d'un bloc par un dash boosté (`EX-GP-057`), **ground pound**
  (`EX-GP-058`, sans charge de dash disponible), et un **combo dash + saut** — jump-cancel d'un dash
  boosté, wall-jump en sortie, momentum hérité d'une poussée renforcée, bonus cumulatif plafonné
  (`EX-GP-061`). Le dash suivait déjà les pentes et le wall slide était déjà livré (LOT-10,
  `EX-GP-016`) : `EX-GP-060` documente et teste ces deux points plutôt que d'y ajouter du code.
  Poussée renforcée et jump-cancel sont restreints au dash **boosté** pour garantir qu'aucun dash
  normal ne change de comportement.

## [0.1.2] - 2026-08-25

> Huitième jalon, et le plus large : le jeu apprend à se jouer tout seul. Un **solveur IA** écrit
> de zéro — tenseurs, différentiation automatique, réseaux de neurones, optimiseurs, quatre
> algorithmes d'apprentissage — s'entraîne hors ligne sur un niveau et en rapporte une **séquence
> d'actions déterministe**, que le moteur rejoue à l'identique. Aucun framework tiers, aucune
> dépendance Python, aucune inférence en temps réel dans le jeu : `Core` et `HMI` ne gagnent pas
> une seule dépendance. Un écran **Mode IA** lance un entraînement, montre les statistiques
> arriver, et rejoue en scène le meilleur individu obtenu.
>
> Côté jeu, le rendu passe par **QRhi** plutôt que d'appeler Direct3D 11 directement, les
> **décors-sprites cèdent la place à des plans picturaux** peints dans l'éditeur avec parallaxe
> réglable, et le tableau final est **retracé** : un gaufre unique de 24×24, cinq profondeurs de
> parallaxe, deux dangers retirés parce qu'ils le rendaient invincible — démontré, pas supposé.
>
> Cette version passe enfin par un **audit de release** complet. Il a supprimé des recalculs (la
> suite de tests unitaires passe de 550 s à 90 s, à résultats identiques au bit près), couvert
> quatre modules qui n'avaient aucun test, rendu falsifiable le test le plus coûteux du parc — ce
> qui a révélé une mesure périmée dans la documentation —, aligné neuf documents sur le code
> qu'ils décrivent, mis l'écran Mode IA en conformité avec l'identité pixel art, et fait dire aux
> commentaires la règle appliquée plutôt que l'historique du défaut corrigé.

### Nouveau tableau final, à cinq profondeurs de parallaxe (LOT-71)

- `demo-final` est **retracé** : un gaufre unique de 24×24, bien plus dense que l'ancien final
  multi-salles — deux clés, un interrupteur à bascule, un bloc sur plaque, un ascenseur à
  plateformes synchronisées, trois puits à wall jump et une cheminée. Il gagne deux plans de
  parallaxe (**ciel** et **proche**) en plus des trois du LOT-70 ; `demo-synthese` passe en
  cadrage par salle.
- **Deux dangers retirés**, sans lesquels le tableau était **invincible** — démontré, pas
  supposé. Le danger statique (17, 9) ne laissait à la rangée 9 que deux trouées, l'une mortelle
  et l'autre débouchant sur un danger permanent, avec une fenêtre d'esquive d'une seule image. Le
  danger mobile (7, 22) balayait exactement le couloir du bas : impossible de l'attendre (il vient
  à vous), de le doubler en marchant (0,05 case par pas contre 0,033, dans le même sens) ni de le
  franchir en ruée (0,25 par pas contre une case de large).
- Le **parcours scripté** du garde-fou système est réécrit en 25 phases documentées. Il est
  désormais porté par `ScriptedLevelSequence.h`, dont la copie était restée sur l'ancien tracé
  50×26 — le test de récompense la rejouait, et le personnage n'y quittait pas son point
  d'apparition.
- Plafond de pas du garde-fou porté de 3 000 à 9 000 : ce seul tableau en demande près de 4 000,
  dont de longues attentes devant les dangers temporisés. C'est une borne de terminaison, pas une
  mesure de difficulté.
- Les quatre niveaux touchés sont **réindentés** à deux espaces, comme `LevelWriter` le fait
  désormais : ils étaient ressortis de l'éditeur en un bloc compact d'une seule ligne.

### Solveur IA — une IA maison qui termine les niveaux (LOT-ANNEXE-01 à 21)

Le jeu gagne un **solveur autonome**, écrit **de zéro** : aucun framework d'apprentissage
automatique, aucune dépendance Python, aucune inférence en temps réel dans le jeu. L'agent
s'entraîne hors ligne sur un niveau, et ce qu'il en rapporte est une **séquence d'actions
déterministe** rejouée à l'identique par le moteur. `Core` et `HMI` ne gagnent aucune dépendance :
tout vit dans une bibliothèque `Source/AiSolver/` et un exécutable `aisolver-cli`.

**Fondations numériques.** Un `Tensor` N-dimensionnel à formes et pas explicites, à tampon partagé
(les vues ne copient rien), et un générateur aléatoire à graine explicite — tout l'aléatoire du
programme en découle, ce qui rend chaque entraînement reproductible. Par-dessus, un **moteur de
différentiation automatique** en mode inverse : un graphe d'opérations construit à la volée, puis
parcouru en sens inverse pour obtenir les gradients. Puis une bibliothèque de **réseaux de
neurones** (couches denses, initialisation Xavier/He, fonctions d'activation, sérialisation des
poids) et deux **optimiseurs**, SGD à moment et Adam.

**Rendre le jeu jouable par une machine.** `HeadlessLevelEnvironment` fait tourner un niveau
**sans fenêtre ni GPU**, en répliquant pas à pas l'ordre de résolution de la partie réelle —
plateformes mobiles, blocs poussés, mécanismes, physique du personnage, dangers. Un test système
permanent compare les deux trajectoires : l'agent ne peut pas apprendre une physique que le joueur
n'aurait pas.

L'état du jeu est traduit en tenseur par trois encodeurs complémentaires : une **fenêtre de tuiles**
centrée sur le personnage encodée par catégorie, un **vecteur d'état** du personnage (vitesse,
appuis, budgets restants), et l'**état des mécanismes** visibles. En sortie, un **espace d'action
discret** : le produit des directions, du saut, du maintien de saut, du dash et de l'interaction.
Le décodage se fait au maximum ou par tirage à température.

**Le signal d'apprentissage.** La récompense mesure la progression en **distance de plus court
chemin sur la grille**, pas à vol d'oiseau : c'est la seule mesure qui récompense un détour imposé
par un mur. Elle vise l'**objectif immédiat** — tant qu'une porte est verrouillée, sa clé compte
comme un but à part entière, sans que rien n'ait à connaître l'ordre de résolution attendu. Chaque
épisode se clôt sur une issue explicite : gagné, mort, bloqué, ou budget épuisé.

**Quatre algorithmes, et de quoi les comparer.** Un **algorithme évolutionniste** d'abord (sélection
par tournoi, croisement, mutation gaussienne) : le chemin le plus court vers un agent qui finit
réellement un niveau, et la ligne de base de tout le reste. Puis trois algorithmes par gradient —
**REINFORCE**, **acteur-critique** (qui réduit la variance en soustrayant une valeur d'état
apprise), et **DQN** avec tampon de rejeu, réseau cible et exploration ε-décroissante. Un harnais de
**benchmark** les met en regard à graines fixées, et une campagne d'**évaluation hors-niveau**
mesure ce qu'un modèle entraîné sur un tableau donne sur les autres.

**Journalisation et rejeu.** Chaque entraînement écrit un CSV de statistiques par génération ou par
épisode (meilleure récompense, moyenne, écart-type, taux de réussite), et un **format de rejeu**
versionné qui porte la séquence d'entrées, l'empreinte du niveau d'origine et les métadonnées
d'entraînement. Un rejeu dont le niveau a changé depuis l'export est **refusé au chargement**, avec
un message explicite, jamais joué à moitié.

**Deux façons de s'en servir.** En ligne de commande, `aisolver-cli` expose `train`, `evaluate` et
`export-replay`. Dans le jeu, un écran **Mode IA** lance un entraînement dans un fil séparé, montre
les statistiques arriver, et permet de rejouer en scène le meilleur individu obtenu.

**Garde-fou d'intégration continue.** Un script vérifie que chaque rejeu publié reste synchronisé
avec le niveau qu'il référence — une réimplémentation Python pure de l'empreinte utilisée par le
C++, donc sans binaire à construire. Sans lui, un rééquilibrage de niveau pourrait périmer un rejeu
versionné pendant des semaines sans qu'aucune Pull Request ne le signale.

### Qualité (audit de release)

- **Cœur tensoriel : l'adressage sorti des boucles chaudes.** `Tensor::at()` reconstruisait une
  `initializer_list`, déréférençait un `shared_ptr` puis relisait **deux `std::vector` du tas**
  (formes et pas) à **chaque multiplication-addition** — or `matmul` en fait trois par itération
  interne, et il est traversé une fois par couche à l'avant et deux fois à l'arrière, pour les
  **quatre** algorithmes d'entraînement. Les pas sont désormais précalculés et le parcours se fait
  sur pointeurs bruts ; `TensorOps` gagne symétriquement un **chemin contigu**
  (`Tensor::isContiguous()`), où l'élément de rang *n* est simplement `data()[n]`.

  L'ordre de boucle `i, j, p` est **conservé** : c'est lui qui fixe l'ordre des sommations
  flottantes, donc le résultat **au bit près**. Aucune valeur attendue n'a été retouchée dans les
  tests de reproductibilité — c'est la vérification qui distingue un recalcul supprimé d'un
  changement de calcul. Mesuré en Debug :

  | Mesure | Avant | Après |
  |---|---|---|
  | `DqnTrainerTest.ReproductibiliteIntegrale` | 6,3 s | **0,7 s** |
  | Comparaison de convergence REINFORCE/acteur-critique | 42,8 s | **3,7 s** |
  | Suite `UnitTests` complète (1 455 cas) | 549,9 s | **89,6 s** |

- **Six modules de production n'avaient aucun test** ; quatre le sont désormais.
  `ObjectiveDistanceFieldCache` — le cache ci-dessus, utilisé par les cinq boucles d'entraînement —
  n'était vérifié par rien, et les tests de reproductibilité ne l'auraient pas attrapé : deux
  exécutions également périmées restent identiques l'une à l'autre. S'y ajoutent
  `PolicyGradientLoss` (la formule **partagée** par REINFORCE et l'acteur-critique, dont les deux
  consommateurs étaient éprouvés mais pas le cœur commun), `ArgmaxRollout` (le chemin qui produit
  le rejeu regardé par le joueur), `DeterministicRandom` (avec des **vecteurs de référence figés**,
  sans lesquels une refonte changerait toutes les suites de particules sans qu'aucun test ne
  bronche) et `QtKeyMap`. Les quatre branches de robustesse CSV annoncées par la version
  précédente n'en avaient elles non plus aucune : elles en ont quatre.
- **Le test le plus coûteux du parc n'affirmait rien.** La campagne de transfert inter-niveaux
  entraînait un agent jusqu'à convergence pour vérifier qu'un taux de réussite est compris entre 0
  et 1 — une propriété de la division, pas du transfert. Il affirme désormais la conclusion du lot
  (transfert **nul**) et borne le nombre de pas. Ce faisant, il a révélé que
  `resultats-transfert.md` était périmé sur l'une des deux paires : le document est corrigé.
- **La documentation décrivait encore un rendu Direct3D 11 direct**, alors que le `LOT-69` l'avait
  fait passer par **QRhi** — plus aucun `#include <d3d11.h>` dans `Source/`. Le guide de rendu se
  contredisait à vingt-six lignes d'intervalle, et le README de `HMI/Graphics` annonçait « pas de
  couche multi-backend », ce que QRhi est précisément. Neuf documents corrigés.
- **`Source/AiSolver/` n'avait aucun README**, à aucun niveau, alors que `Core/` et `Elements/` en
  portent un par sous-module : dix sont écrits, plus ceux de `HMI/{Ai,Diagnostics,Localization}`.
- **L'écran Mode IA passe au crible du `LOT-68`.** Arrivé après la refonte d'identité, il en avait
  les couleurs et la police mais aucun **cadre** (`EX-IHM-070`), et son focus n'était signalé que
  par la teinte — ce qu'`EX-IHM-071` interdit expressément, la navigation à la manette reposant
  entièrement sur le parcours de focus. D'où `hmi::PixelFocusCaret`, réutilisable, qui pose une
  marque explicite à côté des contrôles Qt ordinaires que `PixelMenuButton` ne peut pas couvrir.
  L'écran gagne aussi sa planche de maquette, la seule qui manquait.
- **Un garde-fou de plus** : `scripts/check_design_tokens.py` compare la palette d'identité des
  maquettes à celle du code. Elles coïncidaient, mais à la main — rien n'empêchait une retouche de
  teinte d'un côté de laisser l'autre en arrière, et une maquette qui ne décrit plus le jeu ne
  sert plus à décider quoi que ce soit.
- **Les commentaires disent la règle, plus l'historique du défaut.** Vingt-sept d'entre eux
  racontaient un bug corrigé (« défaut constaté à l'essai », « comme avant ce lot ») au lieu
  d'énoncer l'invariant que le code tient. Deux étaient factuellement **faux**. Deux algorithmes
  n'étaient expliqués nulle part : la mise à jour d'Adam — correction de biais comprise, et
  pourquoi elle est indispensable — et les bornes d'initialisation Xavier/He.
- **Noms et arborescence.** `ScreenId::NiveauTermine` — seul énumérateur français d'un enum
  anglais — devient `LevelComplete` ; `HMI/main.cpp` devient `Main.cpp`, et les 282 fichiers de
  `Source/` hors tests sont enfin tous en PascalCase. Les 73 constantes en `kPascalCase`/`K_*`
  passent en `UPPER_SNAKE_CASE`, la règle écrite — dont deux qui étaient **la même constante sous
  deux styles**. Les abréviations interdites par les conventions disparaissent (`col`, `pos`,
  `algo` ; le drapeau utilisateur `--algo`, lui, ne bouge pas). `Training/Advanced/` devient
  `Training/Dqn/` — il ne contenait que du DQN — et les deux comparateurs de convergence, qui ne
  dépendent d'aucun algorithme, rejoignent `Eval/`.

- **Champ de distance mis en cache.** Le champ de plus court chemin qui alimente la récompense était
  reconstruit par un parcours en largeur complet **à chaque pas de simulation**, dans les cinq
  boucles d'entraînement. Or il ne dépend que de l'état ouvert/fermé des portes : il n'est
  désormais reconstruit qu'à ce moment-là. Les résultats numériques sont identiques — c'est un
  recalcul supprimé, pas un changement de comportement. Mesuré en Debug :
  `DqnTrainerTest.ReproductibiliteIntegrale` passe de 15,1 s à 5,9 s, et la comparaison de
  convergence REINFORCE/acteur-critique de 62,0 s à 41,9 s.
- **Échantillons de plateformes mis en cache** dans `core::PlatformController`, recalculés par
  `update()` au lieu d'être reconstruits à chaque lecture : ils sont lus plusieurs fois par pas, par
  la physique du personnage, la poussée des blocs et l'interpolation d'affichage.
- **Copie de grille supprimée** dans l'environnement sans fenêtre, qui dupliquait toute la grille de
  collision des mécanismes à chaque pas là où la partie réelle en prenait déjà une référence.
- **Température neutre court-circuitée** au décodage stochastique : à température 1 la pondération
  est l'identité, et c'est le seul réglage utilisé à l'entraînement.
- **Robustesse de lecture** : la comparaison de convergence ne suppose plus que le CSV qu'elle lit
  est bien formé — colonne absente, ligne tronquée ou champ non numérique ne provoquent plus ni
  accès hors bornes ni exception traversant la frontière du module.
- **Format des fichiers de niveau** : l'écrivain de niveaux produisait un JSON compact sur une seule
  ligne, seul écrivain JSON du projet dans ce cas. Il indente désormais à deux espaces comme tous
  les autres — un niveau est un fichier versionné, dont la relecture en revue suppose un diff ligne
  à ligne.
- **En-têtes SPDX** ajoutés sur les 32 fichiers de code qui en manquaient encore.
- **Commentaires** : une trentaine de commentaires racontaient l'historique d'une correction (« bug
  réel trouvé en jeu », « corrigé au LOT-xx », « avant cet amendement ») plutôt que la règle que le
  code applique. Réécrits pour énoncer la contrainte, pas l'anecdote.
- **Documentation de code** : blocs Doxygen qui contredisaient la signature ou le comportement
  décrit (constructeur documenté comme multi-source, champs présentés comme des méthodes, version de
  format périmée, `@copydoc` propageant une description fausse), et avertissement ajouté sur le
  partage de tampon à la copie d'un `Tensor`.

### Parallaxe à trois profondeurs des tableaux qui défilent (LOT-70)

- `demo-mouvement` (suivi continu) et `demo-final` (caméra par salle) — les deux seuls tableaux
  livrés où `hmi::planeParallaxActive` est vrai — gagnent un troisième plan, **lointain**, à
  densité 4 et à facteur de parallaxe plus lent que le plan « fond » existant (0.2 contre 0.6 pour
  `demo-mouvement`, 0.15 contre 0.5 pour `demo-final`). Les vingt autres tableaux, dont la
  parallaxe est neutralisée en cadrage `WholeLevel`, sont inchangés.
- `scripts/generate_demo_plans.py` gagne une fonction de peinture dédiée au plan lointain (crête
  déchiquetée unique, ciel plus estompé, semis clairsemé), et un motif procédural `star()` pour le
  thème nocturne. Reproductible comme le reste du générateur (`--check`).
- Un garde-fou système (`test_plans_livres.cpp`) vérifie que tout niveau à parallaxe active déclare
  au moins trois plans aux facteurs strictement croissants, avec son cas négatif.
- Répond au manque explicitement consigné par le `LOT-69` TACHE-10 : la migration n'avait livré
  qu'un report fidèle de l'ancien habillage, pas une fresque exploitant réellement la profondeur.

### Licence

- Le projet passe de « Tous droits réservés » à la **GNU General Public License v3.0 ou ultérieure**
  (`GPL-3.0-or-later`). `LICENSE` contient désormais le texte officiel, verbatim.
- Précision utile, car l'intention initiale était « open source, pas d'usage commercial » : ces deux
  exigences sont **incompatibles**. La définition de l'Open Source interdit de restreindre les
  domaines d'usage (clause 6), donc aucune licence open source ne peut interdire le commercial. La
  GPL apporte la garantie voisine, et plus solide en pratique : toute redistribution d'une version
  modifiée — commerciale ou non — **doit en publier le source** sous la même licence.
- Nouveau **`THIRD-PARTY-NOTICES.md`** : relevé des dépendances de code (Qt LGPLv3, GoogleTest
  BSD-3-Clause, nlohmann/json MIT) et index des licences de ressources (CC0, SIL OFL), qui étaient
  déjà documentées à côté des fichiers concernés mais nulle part rassemblées. Il détaille aussi les
  trois obligations concrètes que le lien dynamique à Qt impose — et pourquoi passer Qt en lien
  statique serait une décision de licence, pas d'optimisation.
- **En-têtes SPDX** (`SPDX-License-Identifier: GPL-3.0-or-later`) sur tous les fichiers de code du
  dépôt. Ils marquent le **code, et lui seul** : images, sons, polices et bibliothèques tierces
  gardent leur propre licence. La précision est écrite dans le README et les notices, parce que
  l'inverse serait facile à croire et faux.
- L'écran **Crédits du jeu** gagne trois sections : **polices** (SIL OFL), **bibliothèques**
  (Qt 6, LGPLv3, lien dynamique) et **licence** du jeu. La LGPLv3 et la SIL OFL exigent cette
  mention ; surtout, un joueur qui n'ouvrira jamais le dépôt doit pouvoir la lire. Seuls les
  intitulés de section sont traduits — un nom de licence ne se paraphrase pas.

### Plans picturaux (LOT-69)

- Le niveau porte désormais une **liste ordonnée de plans** (`EX-DEC-040`) : chaque plan est une
  image couvrant tout le niveau, dans son propre fichier, avec sa **densité** (16, 8 ou 4 px par
  unité), ses **facteurs de parallaxe** par axe, son opacité et sa profondeur. L'ordre de la liste
  est l'ordre de superposition.
- Le format de niveau gagne `planes` et `parallax` (`EX-LVL-009`). Un niveau sans plan est
  strictement inchangé, et aucun champ à sa valeur par défaut n'est écrit — un plan solidaire du
  niveau à densité native ne produit que son nom de fichier.
- Le coût est **borné par le format** et non laissé à l'usage (`EX-DEC-044`) : nombre de plans
  plafonné, et refus d'un plan dont la texture dépasserait la limite du matériel. C'est la
  *combinaison* taille × densité qui est vérifiée, pas la densité seule.
- Tous les mutateurs de plan du brouillon d'édition sont **annulables et refaisables**, et un
  mutateur refusé n'empile aucun pas d'historique.
- Le système de **décors-sprites** (`LOT-49`/`LOT-50`) est **retiré** : modèle, format, rendu,
  outil de canevas, panneau dédié, gestes et assets de placement. Il n'existe plus de motif
  décoratif ponctuel réutilisable — un même élément présent dans dix niveaux se peint (ou se colle)
  dans chaque plan. Les exigences correspondantes sont marquées retirées dans le référentiel,
  leurs ancres conservées.
- Un niveau portant encore le champ `decors` **se charge sans erreur** : le champ est ignoré et
  journalisé comme obsolète, jamais rejeté (`EX-LVL-005`). Un simple charger-puis-enregistrer suffit
  à migrer le fichier, la réécriture ne contenant plus le champ.

### Rendu (LOT-69)

- **Les plans picturaux se composent et se dessinent** (`EX-REN-049`) : un quad par plan couvrant
  le niveau entier, dans l'ordre déclaré, avec son opacité. La densité ne change **pas** la
  géométrie — un plan à 8 px/unité couvre la même surface qu'un plan natif, en plus grossier :
  c'est un compromis de mémoire, pas de cadrage.
- Le calque `Decor` devient **`Plane`**, et les entrées de menu correspondantes « Plans derrière » /
  « Plans devant ». Laisser un calque porter le nom d'un système retiré aurait été précisément la
  dette que `EX-REN-014` cherche à éviter.
- **La parallaxe est portée par le plan** (`EX-DEC-043`), réglable **par axe** et par niveau, à
  la place des trois constantes figées de l'ancien système. Le décalage reste calculé **relativement
  au centre de la salle** : c'est précisément ce qui empêche le saut à la bascule de salle
  (`EX-REN-015`), et un décalage absolu — envisagé au cadrage — rouvrirait ce défaut.
- Le décalage est **borné** : un plan couvre toujours le cadrage, y compris caméra collée au bord
  du niveau. L'alternative « que l'artiste peigne des marges » a été écartée — la marge nécessaire
  dépend du facteur *et* de la taille de salle, et l'erreur ne se verrait qu'en jeu, au bord d'un
  niveau.
- **La parallaxe est réactivée en mode de cadrage *suivi***, ce qui **inverse une décision du
  `LOT-64`. Le motif est net : celui-ci l'avait coupée parce qu'un décor est un objet collé au
  contenu et paraissait « suivre » la caméra ; un plan est un fond, l'argument tombe. En cadrage
  *niveau entier*, elle reste neutralisée — la caméra ne bougeant pas, le décalage ne serait qu'un
  désalignement constant du plan par rapport aux tuiles.
- **Visibilité par plan** dans l'éditeur (`EX-DEC-045`) : masquage individuel et **isolement** d'un
  plan, non persistés — une aide d'édition, pas une propriété du niveau.
- Une propriété invisible à la lecture est désormais **figée par un test** : les plans ne
  ressortent dans l'ordre du niveau que parce qu'ils sont composés en premier et dans l'ordre, le
  tri de la scène intercalant le rang de première apparition de texture entre le calque et le tri
  fin. Sans ce test, un simple réordonnancement d'appels l'aurait cassée en silence.

- **Le rendu passe sur QRhi** (`EX-REN-050`). La cible technique ne change pas — QRhi retient
  **Direct3D 11** par défaut sous Windows (`EX-REN-002`, amendée) — mais l'API n'est plus appelée
  directement : `hmi::GraphicsDevice` (device, swap chain, présentation) est **supprimé**, Qt en
  étant désormais propriétaire, et les shaders sont compilés en `.qsb` puis embarqués.
- **Le viewport est un widget ordinaire** (`QRhiWidget`), plus une fenêtre native embarquée. C'est
  le bénéfice concret : l'écran de pause et l'écran de fin de niveau **redeviennent de simples
  enfants**, là où ils avaient dû devenir des fenêtres de haut niveau à géométrie synchronisée en
  coordonnées écran pour contourner deux défauts réels du `LOT-59` (écran de pause invisible, puis
  `Qt::Tool` empêchant `activateWindow()`). Le focus s'obtient à nouveau directement, sans report
  d'un tour de boucle d'événements.
- **La netteté du pixel art est désormais testée, pas seulement constatée** : un rendu **hors
  écran** d'un motif témoin est relu pixel par pixel et comparé à l'exact — un filtrage devenu
  linéaire, qui passerait sans bruit toute la CI, y échoue.
- **V-Sync** (`EX-REN-022`) : la présentation appartient au compositeur de Qt, elle est donc
  toujours synchronisée. Le réglage des Options est conservé mais n'a plus d'effet — écart assumé
  et documenté dans le référentiel.
- Un défaut de la première version du portage mérite d'être noté, parce qu'il ne se voyait que sur
  du contenu **multicolore** : l'image de téléversement d'une texture ne possédait pas ses pixels,
  et le lot de mises à jour lisait de la mémoire libérée. Le test de rendu hors écran l'a attrapé
  au premier essai.

### Atelier pixel art (LOT-69)

- **Le canevas peut montrer un repère** (`EX-EDIT-046`) : une image **sous** le contenu édité, une
  autre **par-dessus**, et une grille de **tuiles** distincte de la grille de pixels. Les trois sont
  optionnelles et additives — l'atelier qui les ignore se comporte exactement comme avant — et
  n'entrent **jamais** dans le tampon édité, ni dans l'historique, ni dans le copier.
- **Zoom rationnel** : l'échelle descend désormais **sous le 1:1** (dénominateurs 1, 2, 4, 8) pour
  embrasser un plan à l'échelle du niveau entier. Les pixels restent carrés, et la grille de pixels
  s'efface sous le 1:1 où elle couvrirait plus de surface que ce qu'elle sépare. `zoom` reste le
  numérateur, dénominateur 1 par défaut : les tests hérités du `LOT-54` passent **sans être
  modifiés**, ce qui est le seul critère qui prouve que l'extension est bien compatible.
- `PlaneReference` (nouveau, **sans Qt ni GPU**) : pelure d'oignon des tuiles réutilisant la palette
  du mode Physique — inventer un second jeu de couleurs ferait diverger deux vues du même niveau —,
  aplatissement des plans voisins par alpha-over, et rééchantillonnage par ratio **entier** (16 → 8
  garde un pixel sur deux ; jamais d'interpolation, qui fabriquerait des couleurs que l'artiste n'a
  pas posées).
- Ce que la référence **n'est pas** : un aperçu. Ni raccords automatiques, ni skins, ni animation —
  c'est un repère géométrique, et l'essai reste le seul aperçu fidèle.

### Outillage (LOT-69)

- **Qt passe de 6.8.1 à 6.11.2.** Qt 6.11 est la première version à fournir **Qt Canvas Painter**
  (peinture 2D accélérée sur cible QRhi), sur lequel s'appuient les tâches suivantes du `LOT-69`.
  La CI installe désormais aussi `qtshadertools` (l'outil `qsb`, compilateur de shaders exigé par le
  rendu QRhi) et `qtcanvaspainter`.
- Quitter Qt 6.8 **LTS** est assumé : au-delà de 6.8.3, les correctifs de cette branche ne sont
  publiés que sous licence commerciale — le support long terme ne bénéficiait donc pas à ce projet.
- La CI provisionne Qt avec un **`aqtinstall` pris depuis git, à un commit épinglé**
  (`env.AQT_SOURCE`, entrée `aqtsource` de `install-qt-action`). Motif : Qt a changé la disposition
  de son dépôt à partir de 6.11 et `aqtinstall 3.3.0`, dernière version publiée sur PyPI, ne sait
  plus l'atteindre ; le correctif amont est mergé mais pas encore publié. Détour **temporaire et
  documenté**, à retirer dès la parution d'`aqtinstall 3.3.1`.
- `EX-BUILD-010` est amendée : l'exigence de provisionnement reproductible porte explicitement sur
  l'**outil de provisionnement lui-même**, qui doit être épinglé à une révision précise et jamais à
  une branche mobile.

### Mode création (LOT-69)

- **Troisième espace de travail** dans l'éditeur (`EX-EDIT-046`), à côté de l'édition de niveau et
  de l'atelier pixel art. Y entrer masque ce qui n'y sert pas et fait apparaître le canevas : c'est
  cette structure du `LOT-68`, et non un énième bouton, qui rend le mode praticable.
- Conséquence sur la table qui décide où vit un panneau : elle rendait **un** espace par panneau —
  « chaque panneau appartient à exactement un espace » — et cela ne tient plus, canevas, historique
  et palette servant aux **deux** espaces de peinture. Elle rend désormais un **masque**, et la
  garde de complétude passe de « exactement un » à « masque non vide ». Dupliquer les docks aurait
  donné deux canevas et deux historiques à tenir synchronisés.
- **Panneau « Plans »** (`EX-EDIT-047`), à la place de l'ancien panneau « Décors » : liste ordonnée
  avec densité, profondeur, parallaxe par axe, opacité, œil de visibilité et isolement, boutons de
  réordonnancement, et un bouton « Peindre » qui bascule dans le mode création avec le plan chargé.
- **Cycle de vie des fichiers** : « Ajouter » crée un PNG entièrement transparent aux dimensions
  exactes qu'impose la densité, nommé d'après le niveau et rendu unique par un suffixe numérique
  croissant — jamais un identifiant aléatoire, pour qu'un dossier de plans reste lisible à l'œil.
  Changer la densité rééchantillonne l'image et la réécrit.
- **Retirer un plan ne supprime jamais son fichier.** Le brouillon sait annuler une entrée JSON, pas
  la disparition d'une image : un PNG orphelin est moins grave qu'un dessin perdu. Écrit dans la
  documentation utilisateur plutôt que laissé à découvrir.
- Dans le mode création, **`Ctrl+S` enregistre le PNG** ; l'enregistrement de niveau écrit le JSON.
  Deux notions de « modifié » distinctes depuis le `LOT-54`, donc deux garde-fous de perte de
  travail. Un coup de pinceau dans un plan n'apparaît **jamais** dans l'historique d'édition du
  niveau, ni réciproquement.
- La barre d'état annonce la résolution du plan, sa densité et son **poids mémoire** —
  le chiffre même que le budget du dépôt plafonne.
- L'espace de travail est désormais persisté **par son nom** et non par un indice : le `LOT-68`
  écrivait `0`/`1`, et insérer « Plans » entre les deux aurait fait rouvrir en mode création
  l'éditeur laissé dans l'atelier. Le mode création, lui, n'est jamais restauré au démarrage — il
  suppose un niveau ouvert et un plan sélectionné.

### Budget de rendu (LOT-69)

- Le budget mesuré du `LOT-62` gagne un **second axe** : la **mémoire de texture** des plans
  (`EX-NFR-043`), à côté des compteurs de primitives et dans la même structure de statistiques.
  Motif : un plan n'ajoute qu'**une** primitive mais occupe une texture à l'échelle du niveau — un
  plafond exprimé en primitives ne le verrait jamais grossir, alors qu'un niveau 200 × 100 à densité
  native coûterait 20 Mo **par plan**.
- Ce plafond-là n'est **pas** décliné par niveau comme celui des primitives : le volume de
  primitives dépend du contenu posé par l'auteur, la mémoire de plans ne dépend que de la taille du
  niveau et des densités déclarées.
- Il est calibré pour **refuser avant la limite de format** : 16 Mio laissent passer douze plans
  natifs sur le plus grand tableau du dépôt, là où le format en autorise seize. Un test le vérifie
  dans ce sens-là — un garde-fou qu'on n'a jamais vu refuser ne prouve rien.
- Un test fige la propriété qui distingue les plans des tuiles : leur coût en primitives et en
  passes est **invariant en taille de niveau**, alors que leur mémoire suit la surface.

### Contenu (LOT-69)

- **Les vingt-deux tableaux livrés passent aux plans picturaux.** Le cadrage du lot annonçait
  `demo-final` comme seul niveau porteur de décors : c'était faux, tous en portaient (soixante-deux
  entrées au total). Chacun reçoit un **fond** à densité 8, et ceux qui avaient un décor de premier
  plan un second plan à densité native.
- Les décors sont **reportés à leurs positions d'origine**, avec la même géométrie que l'ancien
  rendu. L'intention visuelle de chaque tableau est préservée ; ce n'est pas pour autant une fresque
  peinte qui exploiterait vraiment la profondeur — cela reste un acte de *level design*, hors de ce
  lot d'outillage.
- Les images sont **générées, jamais dessinées à la main** (`scripts/generate_demo_plans.py`) :
  reproductibilité, dans la ligne du `LOT-66`. `Source/Elements/Assets/Decors/` et son générateur
  disparaissent ; les quatre motifs Kenney qu'il contenait sont **conservés** comme *sources* du
  générateur (`scripts/motifs/`) — les supprimer aurait rendu le script injouable, donc non
  reproductible.
- **Des facteurs de parallaxe ne sont déclarés que sur `demo-final` et `demo-mouvement`**, les deux
  seuls niveaux dont la caméra défile. Ailleurs, le cadrage *niveau entier* la neutralise :
  déclarer un facteur y aurait été une promesse que rien ne tient.
- Le garde-fou de couverture des mécaniques exigeait déjà, dans sa documentation, un « décor de
  premier plan » — sans que le code ne le vérifie. Il vérifie désormais réellement la présence d'un
  **plan devant le personnage** et d'une **parallaxe réglée**.
- Deux tests système nouveaux : **aucun niveau du dépôt ne porte encore `decors`**, et **tout plan
  référencé existe aux dimensions attendues**. Ni `Core` (qui ne vérifie pas l'existence d'un
  fichier) ni `HMI` (qui replie sur un damier) ne signaleraient l'un ou l'autre.

### Corrections (LOT-69)

- **La manette n'était plus détectée depuis les écrans d'interface.** Le sondage XInput est espacé
  tant qu'aucune manette n'est présente — interroger un slot vide coûte cher et provoque des
  micro-saccades — mais cet espacement se comptait en **nombre d'appels**. Or le sondage est
  déclenché tantôt par la boucle de rendu (une fois par image), tantôt par un temporisateur
  d'interface (150 ms au menu, 500 ms dans les Options) : le même compteur donnait donc un délai de
  détection allant de deux secondes à **une minute** selon l'appelant. L'espacement est désormais
  décidé en **temps réel**, ce qui borne la détection quelle que soit la cadence.
- **Ouvrir un niveau habillé dans l'éditeur perdait tous ses plans.** Le brouillon d'édition
  (`LevelDraft::fromLevel`) ne recopiait ni la liste des plans ni le drapeau de parallaxe : le
  niveau s'ouvrait, s'éditait et s'enregistrait normalement — mais **sans son habillage**, effacé
  en silence. Aucun message, aucun échec : un champ simplement absent ne fait rien échouer, ce qui
  est exactement la classe de défaut qu'aucun test de comportement n'attrape. Deux tests le figent
  désormais, dont un aller-retour niveau → brouillon → niveau.
- **Les boutons des boîtes de dialogue étaient en anglais** dans une interface française : « Yes »
  et « No » viennent de Qt, pas du catalogue du projet, et Qt les rend en anglais tant qu'aucun
  `QTranslator` n'est installé. Le catalogue `qtbase_<langue>.qm` est désormais chargé et déployé
  à côté de l'exécutable — les deux seuls dont l'IHM propose la langue, pas la trentaine que
  `windeployqt` copierait.
- Dans le mode création, la barre d'état annonçait le **niveau** et son zoom de caméra pendant
  qu'on peignait une image, tant que le focus clavier n'était pas entré dans le canevas. L'espace
  n'ayant pas d'autre sujet, elle parle désormais du plan sans attendre le focus — et dit « Plan »
  plutôt qu'« Asset », un plan n'étant réutilisable par aucun autre niveau.
- Les réglages du plan sélectionné affichaient les **bornes basses** de leurs champs
  (« Parallaxe 0,01 · Opacité 0,00 ») quand aucun plan n'était sélectionné, ce qui se lisait comme
  une configuration choisie. Ils montrent les valeurs par défaut d'un plan. Le facteur de parallaxe
  accepte par ailleurs **0** : un plan strictement immobile est un réglage légitime.
- **Lancer une partie depuis le menu affichait le niveau en mode édition.** Le portage QRhi a rendu
  la création des ressources graphiques paresseuse : elle a lieu à la **première image**, donc
  *après* le démarrage de la partie, et elle commence par libérer ce qui tenait une texture — dont
  la session de jeu qui venait d'être créée. Le viewport retombait alors silencieusement sur le
  brouillon d'édition. La session est désormais **remontée** une fois les ressources disponibles.
## [0.1.1] - 2026-08-20

> Septième jalon : les trajectoires de plateformes et de dangers mobiles se dessinent enfin **à la
> souris** plutôt qu'au JSON (`LOT-67`), et l'éditeur comme les écrans du jeu passent en **pixel
> art** (`LOT-68`) — cadres à bordure franche, police bitmap embarquée, barre d'outils et menus
> dégraissés.
>
> Une route de plateforme mobile suit désormais une suite de points de passage, en aller-retour ou
> en **circuit fermé**, manipulable directement au canevas par l'outil « Parcours » ; un panneau
> « Propriétés » rend enfin accessibles depuis l'éditeur des réglages qui n'existaient jusqu'ici que
> dans le modèle (vitesse, déphasage, mode, axe/portée d'un danger mobile, période d'un danger
> temporisé, capacités de mobilité par tableau). Trois vieux défauts de l'éditeur sont corrigés au
> passage : une plateforme fraîchement posée pouvait rester sans parcours possible avant un
> enregistrement/rechargement, trois types de tuiles (clé, porte verrouillée, plateforme mobile)
> n'avaient pas de texture dans le jeu de skins par défaut, et la palette/l'arbre de textures
> s'ouvraient parfois vides au lancement.
>
> **1185 tests** (1119 au jalon précédent).

### Interface (LOT-68)

- Les six écrans du jeu passent en **pixel art** : police bitmap embarquée (Pixelify Sans, Press
  Start 2P), cadres à bordure franche et coins entaillés, décor au menu principal, et une échelle
  d'agrandissement **entière** dérivée de la taille de la fenêtre — l'interface n'est plus figée à
  une taille visiblement petite dès qu'on dépasse la définition d'un portable.
- L'élément focalisé porte désormais un **curseur** et non une simple nuance de couleur : la
  navigation à la manette n'a pas de pointeur pour dire où elle en est.
- L'écran de fin de tableau affiche un **bilan** : temps, morts et sauts. Le temps est compté en pas
  de simulation, donc comparable d'une machine à l'autre.
- L'écran Options perd ses sélecteurs de **résolution** et de **limite d'images/s**, grisés et jamais
  branchés, et gagne un réglage qui agit : l'affichage du **compteur de diagnostic**, jusqu'ici
  atteignable par la seule touche `F9`.
- La barre d'outils de l'éditeur ne porte plus que les outils et quatre commandes à usage continu :
  elle en portait onze, dont neuf figuraient déjà au menu.
- La barre de menus est réorganisée **par nature d'action** (Fichier, Édition, Niveau, Affichage,
  Atelier, Aide), et les réglages nombreux passent en sous-menus — le menu Affichage alignait
  vingt-trois entrées à plat.
- L'éditeur se présente en **espaces de travail exclusifs** : édition de niveau ou atelier pixel art.
  Chacun n'affiche que ses panneaux et sa barre d'outils, et retient sa propre disposition.


### Ajouté
- **Route multi-points pour les plateformes mobiles** (`LOT-67`, `EX-GP-054`) : une plateforme suit
  désormais une suite de points de passage, parcourue en aller-retour ou en **circuit fermé**, à
  vitesse constante — là où elle ne pouvait relier que deux points.
- **Outil « Parcours » dans l'éditeur** (`EX-EDIT-032`) : les trajectoires se manipulent directement
  au canevas, par des poignées glissables — déplacer, insérer ou retirer un point d'une route,
  redéfinir l'axe et la portée d'un danger mobile. Chaque geste complet ne coûte qu'une annulation.
- **Panneau « Propriétés »** (`EX-EDIT-033`) : vitesse, déphasage et mode d'une plateforme ; axe et
  portée d'un danger mobile ; période, déphasage et durée active d'un danger temporisé ; règles du
  tableau. Ces réglages existaient dans le modèle depuis le `LOT-63` mais **aucun n'était atteignable
  depuis l'éditeur** : il fallait éditer le JSON à la main.
- **Capacités de mobilité par tableau** (`EX-GP-055`) : un niveau peut redéfinir le nombre de sauts
  aériens et de **charges de dash**, rechargés à chaque atterrissage — à distinguer des budgets de
  `EX-GP-024`, consommables une fois pour toutes sur le tableau. Le dash porte désormais un compteur
  de charges et non plus un booléen ; sa valeur par défaut reproduit le comportement historique.

### Modifié
- Le format de niveau porte `waypoints` et `mode` pour les plateformes, ainsi que `airJumps` et
  `dashCharges` à la racine (`EX-LVL-008`). Le couple `endX`/`endY` reste **lu** : un fichier
  antérieur se charge et se joue à l'identique. `demo-plateforme.json` conserve volontairement une
  plateforme à l'ancien format, pour que la compatibilité soit prouvée par le contenu livré.

### Corrigé
- **Dérive de position des plateformes en session longue** : la distance parcourue était cumulée en
  simple précision, ce qui perdait le bit de poids faible au-delà d'environ 16,7 millions de pas
  (~77 h de jeu) et décalait visiblement la plateforme. Le calcul passe en double précision, et un
  test fige le comportement à vingt millions de pas.
- **Budgets de sauts et de dashs non annulables** : `LevelDraft::setJumpBudget` et `setDashBudget`
  n'empilaient pas de pas d'annulation, contrairement à toutes les autres propriétés de niveau
  (fond, jeu de skins, cadrage). `Ctrl+Z` ignorait donc silencieusement ces changements.
- **Plateforme/danger mobile fraîchement posé sans parcours possible** (`EX-GP-026`/`EX-GP-051`,
  `EX-EDIT-032`) : `LevelDraft::paintTile` ne créait sa configuration de route qu'au premier
  enregistrement/rechargement du niveau (seul `LevelLoader` la créait par défaut). Tant que ce
  round-trip n'avait pas eu lieu, l'outil « Parcours » ne trouvait la tuile dans aucun des vecteurs
  qu'il parcourt et ne pouvait donc pas la désigner : impossible de démarrer son parcours sans
  d'abord sauvegarder puis recharger. La pose crée désormais la configuration par défaut
  immédiatement, comme `LevelLoader`.
- **Trois types de tuiles sans texture par défaut** : clé, porte verrouillée et plateforme mobile
  n'avaient d'entrée que dans le jeu de skins `kenney`, pas dans `test` (le jeu par défaut,
  `skins.json`) — damier de repli systématique tant que le niveau ne redéfinissait pas son jeu de
  skins. Les trois assets existaient déjà (`key.png`, `locked_door.png`, `platform.png`) ; il ne
  manquait que les entrées.
- **Palette et arbre de textures vides au lancement de l'éditeur** : le câblage de `MainWindow`
  (à la construction) lisait le catalogue de skins avant que `GameViewport::ensureResources` ne
  l'ait chargé (différé à la première exposition du canevas Direct3D), donc dans un état encore
  vide. La palette de blocs et l'arbre de la section « Textures » s'ouvraient sans aucune vignette,
  jusqu'à la première bascule de mode de rendu ou de jeu de skins qui les rafraîchissait par
  ailleurs. `GameViewport` émet désormais `resourcesReady` une fois le catalogue réellement chargé,
  et `MainWindow` s'y reconstruit.

## [0.1.0] - 2026-08-17

> Sixième jalon, et premier qui **annonce un jeu** plutôt qu'un moteur : les jalons `0.0.x`
> construisaient les briques, celui-ci les assemble en une expérience qui se termine, qui se
> retient et qui se fait entendre.
>
> On peut désormais **mettre le jeu en pause**, voir **la fin d'un tableau** marquée clairement, et
> **retrouver sa progression** au tableau exact où on l'a quittée plutôt que de repartir du premier
> niveau à chaque lancement (`LOT-59`). Sauter, atterrir, dasher, activer un interrupteur, mourir et
> gagner un tableau **font du bruit** — volume réglable et persisté, jeu pleinement jouable en
> silence (`LOT-60`). Des **effets de particules** accompagnent les mouvements du personnage
> (`LOT-53`). Trois mécanismes que les spécifications annonçaient depuis le début arrivent enfin :
> l'action « Interagir », les clés et portes verrouillées, les plateformes mobiles (`LOT-63`). Le
> level designer choisit désormais le **cadrage de caméra** d'un niveau plutôt que de le subir
> (`LOT-64`). Un défaut rencontré par un joueur sur la version publiée laisse maintenant une
> **trace exploitable** (`LOT-61`). Le budget de rendu et l'empreinte mémoire, jusque-là de simples
> promesses, sont désormais **mesurés à chaque build** (`LOT-62`).
>
> Les vingt-deux tableaux de démonstration ont été **entièrement redessinés** pour que chacun
> exploite réellement sa mécanique plutôt que la contourner, banque d'assets et personnage refaits
> en conséquence (`LOT-65`).
>
> Ce jalon règle enfin les trois derniers écarts qui séparaient le dépôt d'une publication propre :
> Qt épinglé et vérifié sur les trois environnements, numéro de version qui n'existe plus qu'à un
> seul endroit, et référentiel de spécifications qui dit la vérité sur ce qui est livré (`LOT-66`).
>
> **1119 tests** (943 au jalon précédent).

### Ajouté
- **LOT-66 TACHE-04 — Bascule `0.1.0` et vérification finale.** `project(VERSION)` passe à `0.1.0`,
  seul endroit où le numéro est écrit (`TACHE-02`). `README.md` mis à jour pour décrire le jeu
  livré (pause, progression, son, effets, mécanismes ajoutés) plutôt que le moteur du jalon
  précédent. Notes de release vérifiées (`scripts/extract_release_notes.py v0.1.0`).
- **LOT-66 TACHE-03 — Statuts des spécifications et exigences orphelines.** Huit fichiers sur dix
  restaient marqués « brouillon », certains depuis vingt lots, alors qu'ils décrivent un système
  livré. Chacun porte désormais un statut réel, daté (`0.1.0`), sur le modèle déjà appliqué à
  `decors.md`/`editeur-niveaux.md`. `EX-ARCH-001`/`060`/`070` et `EX-NFR-032` sont documentés comme
  des **invariants transverses** (respectés par tout lot sans être cités) ; `EX-DEC-031` comme
  **post-MVP** ; `EX-VIS-002` à `EX-VIS-007` reçoivent chacun un renvoi vers l'exigence détaillée
  qui le concrétise (et réciproquement). Les deux points ⚠️ restants (réglage fin du ressenti,
  `gameplay.md`) sont explicitement reportés au-delà de `0.1.0`, pas laissés en suspens.
  `scripts/lint_exigences.py` détecte désormais aussi les exigences **déclarées mais jamais
  référencées** (au lieu de seulement les références orphelines), avec une liste explicite et
  documentée d'exemptions pour les invariants et le post-MVP — même logique que la vérification déjà
  automatique ailleurs dans ce lot plutôt qu'une revue manuelle.
- **LOT-66 TACHE-02 — Numéro de version généré, plus jamais recopié.** Le `Doxyfile` portait sa
  propre copie du `PROJECT_NUMBER`, vérifiée (et non générée) par `scripts/build_docs.py` : un
  oubli devenait un échec de CI au lieu d'être rendu impossible. Le `Doxyfile` versionné ne porte
  plus de numéro (`PROJECT_NUMBER` vide) ; le script l'injecte depuis `project(VERSION)` en le
  passant à Doxygen sur l'entrée standard (`doxygen -`), sans écrire de fichier temporaire.
  Bumper `project(VERSION)` suffit désormais — l'étape manuelle a disparu de `CONTRIBUTING.md`.
- **LOT-66 TACHE-01 — Qt épinglé et vérifié sur les trois environnements** (`EX-BUILD-010`). La CI
  et la release installaient Qt `6.8.1` explicitement ; le poste local retenait silencieusement la
  version la **plus récente** trouvée sur le disque, sans que rien ne signale un écart. Le CMake
  déclare désormais `QT_VERSION_MINIMUM` (source unique), l'utilise comme version minimale de
  `find_package(Qt6 ...)`, avertit (sans bloquer) si la version trouvée diffère de celle de la CI,
  et journalise la version retenue (configuration) ainsi que celle contre laquelle le binaire a été
  compilé (`QT_VERSION_STR`, journal de session LOT-61 — utile en rapport de défaut). La cohérence
  entre `QT_VERSION_MINIMUM` (CMake) et `env.QT_VERSION` (`ci.yml`, `release.yml`) est vérifiée
  automatiquement par `scripts/check_qt_version_pin.py` (job `lint-exigences`), plutôt que par
  relecture.
- **LOT-65 (second temps) — De la couverture à la profondeur.** Le garde-fou de couverture livré en
  `TACHE-01` était vert, et le contenu qu'il validait ne tenait pourtant aucune des promesses de
  `EX-LVL-012` : une revue des vingt-deux tableaux a établi que **onze d'entre eux ne demandent rien
  au joueur** (dix se franchissent en maintenant « droite », `demo-plateforme` sans aucune entrée),
  que chaque mécanique n'existe qu'en **un seul exemplaire**, et que **treize tuiles de mécanique
  sont physiquement hors d'atteinte** — dont les quatre plafonds inclinés de `demo-plafond` et
  l'interrupteur de `demo-dangers-avances`, ce qui rendait son danger commuté impossible à commuter.
  La `TACHE-01` avait pourtant écrit la limite (« couvert ≠ franchi ») sans en tirer les
  conséquences.
  - **Quatre garde-fous qui mesurent l'usage, pas la présence** (`TACHE-05`) : aucun tableau
    franchissable en maintenant « droite » (hors exclusion nommée) ; un type de tuile posé au moins
    trois fois dans la séquence ; `jumpBudget` et `dashBudget` comptés **séparément** (le « ou »
    précédent laissait passer une séquence entière sans budget de dash) ; zones de caméra et taille
    de salle par niveau, invisibles d'un contrôle portant sur le seul mode de cadrage. Le rejeu
    scripté relève désormais la **trajectoire réelle** et refuse une mécanique hors de portée.
    Doctrine correspondante écrite dans `niveaux.md`, Sec. 3.
  - **Deux corrections moteur assumées** (`TACHE-06`), assouplissement délibéré et borné du cadrage
    « ce lot n'ajoute rien au moteur » — sans elles, ni le tutoriel de la clé ni l'énigme du tableau
    final ne tiennent : une porte qui se **referme sur le personnage** est désormais **mortelle**
    (`EX-GP-021`, via `core::Player::squished` déjà en place pour les plateformes) au lieu de le
    laisser encastré dans un mur sans échec possible — ce que `demo-plaque-pression` provoquait à
    **chaque partie** ; et un **bloc poussable peut enfoncer une plaque de pression** (`EX-GP-025`,
    seuil `MIN_TRIGGER_MASS` déjà présent), ce qui débloque l'idiome de puzzle le plus classique du
    genre, jusqu'ici hors d'atteinte parce que le contrôleur ne recevait que la boîte du joueur. Un
    bloc réduit reste trop léger : la distinction est visible dans le tableau.
  - **Batterie de croisements de mécaniques** (`Source/Test/Integration/test_croisements_mecaniques.cpp`) :
    douze combinaisons qui n'étaient testées nulle part — dash contre un bloc plein puis réduit,
    dash sous un plafond incliné, dash sur une plaque, bloc sur plaque, bloc trop léger, porte
    écrasante, bloc poussé sur un danger, plateforme traversant une porte fermée, plateforme
    emportant un bloc, deux déclencheurs sur une même porte.
  - **Vingt-deux tableaux redessinés**, chacun conçu pour que sa mécanique soit la **seule** issue.
    Les interrupteurs passent dans des alcôves du plafond qu'il faut atteindre en sautant ; les
    plafonds inclinés et les dangers directionnels **bordent** le couloir au lieu de flotter à deux
    hauteurs de saut au-dessus ; les blocs deviennent indispensables parce que le budget de sauts ne
    suffit plus à les contourner ; `demo-plaque-pression` repose sur un bloc posé sur la plaque au
    lieu d'un saut qui prend la porte de vitesse. Deux tableaux disparaissent : `demo-arrondi`
    (fusionné dans `demo-pente`, dont il reprenait le tracé à une tuile près) et `demo-salles`
    (272 tuiles, zéro mécanique, 40 % de sa surface scellée sous le sol — et joué **après** le
    final), absorbé par le nouveau final multi-salles. Deux tableaux naissent : `demo-mouvement`
    (synthèse de l'acte de mouvement) et `demo-synthese`.
  - **Un vrai tableau final** : quatre salles, une énigme composée par salle, la clé gardée par des
    dangers temporisés déphasés, et les deux variantes de cadrage du `LOT-64` qu'aucun tableau
    n'employait — zones de caméra dessinées à la main (`EX-LVL-007`) et taille de salle propre au
    niveau (`EX-REN-017`).
  - **Invite « Interagir »** (`hmi::gameHudLines`) : rappel contextuel au contact d'une clé non
    ramassée. Le ramassage exige une entrée qu'aucun autre tableau ne demande et que le jeu ne peut
    pas expliquer ; sans invite, un joueur qui l'ignore reste bloqué devant la porte verrouillée
    sans aucun retour.
  - **Banque d'assets élargie**, entièrement générée par script : huit décors, trois fonds (forêt,
    crépuscule, industriel), quatre objets. Le **personnage** est refait — il était illisible devant
    une tuile de teinte voisine : cerne calculé depuis la silhouette, visage, ombrage, bottes,
    écrasement/étirement, et six phases de course distinctes là où le cycle précédent en comptait
    quatre dont **deux identiques**.
  - **Doublons de test retirés** : `test_physique_personnage.cpp` ne rejoue plus les tableaux
    livrés. Il portait une solution scriptée par tableau, doublon plus faible de
    `ParcoursCompletSysteme` qui les rejoue tous avec deux garde-fous de plus ; la refonte a cassé
    la copie la plus faible sans rien apprendre. Les tests **négatifs** restent : ils n'encodent
    aucune solution.

### Registre des défauts (consignés, non corrigés)
- **Une pente franchissable à la marche ne l'est pas au dash.** Sur la silhouette exacte des
  tableaux de pente livrés (pente à 45° suivie d'un palier plein), un personnage qui **dashe** se
  fige au sommet : il bute contre la colonne pleine du palier avec les pieds encore 0,15 case trop
  bas, et n'en repart jamais. La même géométrie se franchit sans difficulté à la marche. Le suivi de
  surface (`core::resolveSlopeFollow`) n'avait jamais été éprouvé qu'à 0,05 case par pas ; un dash
  en parcourt 0,25. Caractérisé par
  `CroisementsMecaniques.DashSurUnePenteResteBloqueAuSommetDefautConsigne`, qui vérifie d'abord que
  la géométrie se franchit à la marche pour prouver qu'il mesure bien le dash. Conséquence sur le
  contenu : aucun tableau ne rend un dash **obligatoire** dans une montée de pente vers un palier.

### Ajouté (premier temps)
- **LOT-65 — Refonte des niveaux de démonstration** (concrétise `EX-LVL-015` ; actualise
  `EX-LVL-012`, dont le « 3 niveaux » du MVP ne décrivait plus rien depuis longtemps). Dernier lot
  de contenu du programme `0.1.0` : refaire les tableaux livrés pour qu'ils exploitent et testent
  toutes les mécaniques du moteur, plutôt que les quinze bancs d'essai nus hérités du `LOT-25`.
  - **Garde-fou de couverture** (`Source/Test/Systeme/test_couverture_mecaniques.cpp`,
    `EX-LVL-015`) : un contrôle échoue, en nommant précisément ce qui manque, si un type de tuile,
    un mode de cadrage ou une variante significative (danger temporisé déphasé, danger mobile
    vertical, budget de mouvements borné, texture par instance, décor de premier plan) n'apparaît
    dans aucun tableau de la séquence livrée — dérivé des énumérations du code (même technique que
    `core::parseTileType`), jamais d'une liste recopiée à la main.
  - **Banque d'assets renforcée avant la refonte du contenu** : deux fonds supplémentaires (nuit,
    souterrain), cinq décors et deux objets d'instance en plus, une première spritesheet de
    personnage (`Player/player.png`, absente jusqu'ici — le jeu retombait systématiquement sur la
    silhouette procédurale), tous générés par script (schématiques, sans dépendance externe, même
    esprit que l'existant). Le jeu de skins `kenney` (real art CC0, `LOT-63`) étoffé de trois types
    supplémentaires (`danger`, `switch`, `door`) et d'un premier fond réel, retouchés depuis les
    mêmes packs Kenney déjà crédités.
  - **Dix-sept tableaux redessinés** (nouvelle géométrie, jamais recopiée depuis l'historique git
    — repartir d'une page blanche pour chacun, pas seulement pour la séquence) et habillés : fond,
    décors, jeu de skins `kenney` pour trois d'entre eux, et cadrage de caméra **choisi
    explicitement** plutôt que subi — `demo-final.json` (le parcours continu le plus long de la
    séquence) passe ainsi en cadrage *suivi* plutôt que de retomber sur *par salle* du seul fait de
    ses dimensions. Chaque tableau vérifié individuellement par le test système, en itérant sur la
    géométrie jusqu'au franchissement plutôt qu'en le supposant.
  - **Cinq tableaux supplémentaires**, groupés par famille plutôt qu'un par type, couvrant les
    quatorze types de tuile qu'aucun tableau n'employait encore : pentes/arrondis/concaves montant
    vers la **gauche** (`demo-pente-gauche.json`), les quatre variantes de **plafond** incliné
    (`demo-plafond.json`), arrondis **concaves** de sol et de plafond (`demo-concave.json`), bloc
    poussable à taille **quart** (`demo-bloc-quart.json`), dangers directionnels bas/gauche/droite
    (`demo-dangers-directionnels.json`). Séquence portée de dix-sept à **vingt-deux** tableaux.
  - **Registre des défauts** : un défaut de moteur découvert en construisant cette séquence, isolé
    par bissection, **consigné et non corrigé** ici (décision de cadrage du lot) — la seule
    présence d'une configuration de `movingPlatform` dans un niveau, même immobile et loin du
    personnage, casse la résolution de collision pendant le suivi d'une pente ailleurs dans ce même
    niveau. `demo-final.json` n'associe donc pas la plateforme mobile aux autres mécaniques
    combinées ; `demo-plateforme.json` continue de la couvrir isolément. Aucun autre défaut
    découvert en rejouant (scripté, `ctest` à 100 %, 1122 tests) cette séquence. Le parcours
    **manuel** complet (binaire Release, manette, son) — le moment où ce lot attend le plus souvent
    d'en trouver, d'après l'expérience des lots précédents — reste à faire.
- **LOT-64 — Cadrage de caméra choisi par le level designer** (déclare `EX-LVL-006`, `EX-REN-016`,
  `EX-EDIT-028`, `EX-LVL-007`, `EX-REN-017`, `EX-EDIT-029` ; reformule `EX-REN-015`). Le cadrage
  devient une **donnée du niveau**, plus une règle en dur déduite de ses dimensions.
  - **Trois modes** (`core::CameraFramingMode`) : *niveau entier*, *par salle* (comportement
    historique, désormais explicite), *suivi du personnage* — le mode qui manquait au moteur.
  - **Caméra de suivi** (`hmi::FollowCamera`, `Source/HMI/Graphics`) : zone morte, anticipation
    s'inversant progressivement, lissage cadencé sur le pas fixe, bornage aux limites du niveau
    (centrage sur l'axe trop étroit), centre aligné au pixel — fonction pure, testée sans GPU.
  - **Taille de salle réglable par niveau** (`hmi::RoomGrid` reçoit désormais la taille en
    paramètre ; les anciennes constantes n'en restent que la valeur par défaut).
  - **Repli compatible** (`core::resolveCameraFraming`) : un niveau sans champ `cameraFraming`
    déclaré se joue **exactement** comme avant ce lot — les quinze tableaux livrés et le test
    système restent inchangés. Version de format `1` → `2` (`EX-LVL-005`).
  - **Choix et prévisualisation dans l'éditeur** (section « Cadrage » du panneau Textures) : les
    trois modes se voient dans le canevas (cadre du niveau, grille de salles à taille variable,
    rectangle de suivi avec zone morte matérialisée), le mode courant reste visible dans la barre
    d'état, le changement de mode est annulable.
  - **Corrigé pendant l'essai manuel** : la parallaxe des décors (`EX-DEC-006`, `LOT-49`), jusqu'ici
    invisible faute de caméra défilant en continu, apparaissait pour la première fois en mode
    *suivi* — un décor Fond/Premier plan semblait « suivre » la caméra au lieu de rester solidaire
    du niveau. Neutralisée spécifiquement pour ce mode (`hmi::composeWorldSprites`, paramètre
    `applyDecorParallax`) : les trois couches restent strictement fixes dans le niveau, comme dans
    les deux autres modes de cadrage.
  - **Zones de caméra dessinées à la main** (`core::CameraZone`, `EX-LVL-007`) : en mode *par
    salle*, le level designer dessine ses propres rectangles de caméra directement sur le canevas
    (nouvel outil « Zone de caméra »), ce qui permet de **mélanger plusieurs tailles de caméra dans
    un même niveau** — la grille automatique à taille unique n'est plus qu'un cas particulier
    (liste de zones vide). Priorité à la première zone couvrant le personnage en cas de
    chevauchement, repli sur le niveau entier hors de toute zone ; ajout/retrait annulables,
    tableau récapitulatif dans la section « Cadrage ».
  - **Taille de la caméra de suivi réglable par niveau** (`EX-REN-017`) : le mode *suivi* réutilise
    les mêmes champs que la taille de salle du mode *par salle*, au lieu de la constante par défaut
    codée en dur.
- **LOT-63 — Mécanismes manquants du référentiel** (lève `EX-GP-023`, marqué « ⚠️ optionnel MVP »
  depuis la rédaction des spécifications ; déclare `EX-CTRL-022` et `EX-GP-026`). Réduit l'écart
  entre ce que le référentiel promettait et ce que le jeu contenait.
  - **Action logique « Interagir »** (`core::PlayerInput::interactPressed`, touches par défaut
    **E**/**X**, remappables indépendamment au clavier et à la manette) : complète l'activation
    par contact des mécanismes sans la remplacer ; premier usage, le ramassage d'une clé.
  - **Clé et porte verrouillée** (`TileType::Key`/`LockedDoor`) : liaison résolue par la **même**
    infrastructure `core::Mechanism` qu'interrupteur/plaque↔porte (aucune notion de liaison
    dupliquée) ; ramassage par contact **et** « Interagir » ; ouverture **définitive**
    (contrairement à la porte d'un interrupteur).
  - **Plateforme mobile** (`TileType::MovingPlatform`, `core::PlatformController`) : aller-retour
    déterministe entre deux points (fonction du numéro de pas fixe, jamais du temps réel ni d'une
    accumulation), portant le personnage et les blocs poussables sans traversée, glissement ni
    tremblement à l'affichage ; écrasement contre un plafond mortel (décision de cadrage).
  - **Intégration éditeur complète** des trois mécanismes (palette, liaison, paramétrage, rendu de
    brouillon avec parcours matérialisé) et **habillage réel sous licence libre** (CC0, Kenney,
    jeu de skins `kenney`) — crédits dans `Source/Elements/Assets/CREDITS.md` et l'écran Crédits.
  - **Trois niveaux de démonstration** (`demo-cle.json`, `demo-plateforme.json`, action
    « Interagir » exercée) insérés dans la séquence livrée.
  - **Corrigé en cours de lot** : l'agrandissement de la grille procédurale (`TextureAtlas` 5×5 →
    6×6, pour loger `MovingPlatform`) laissait `Source/Elements/Assets/atlas.png` **désynchronisé**
    du code (fichier versionné, jamais recalculé automatiquement, `LOT-39`) — personnage mal recadré
    en mouvement, `Key`/`LockedDoor` invisibles, mécanismes du lot en noir en mode Physique.
    Régénéré (`--export-atlas`) ; couvert par un nouveau test d'intégration
    (`test_plateforme_composition.cpp`) qui exerce le repérage d'entité-tuile et le rafraîchissement
    visuel d'une plateforme mobile exactement comme `hmi::GameSession`, jusqu'ici jamais testés.
- **LOT-62 — Budget de rendu mesuré** (honore `EX-NFR-005` et `EX-NFR-001`, jamais vérifiées
  jusqu'ici). Transforme deux exigences déclaratives en garanties assertées, sans rien optimiser.
  - **Test de non-régression du volume de primitives**
    (`Source/Test/Unit/HMI/Graphics/test_render_budget.cpp`) : chaque niveau livré reste sous un
    plafond nommé, composées et soumises, en mode Physique et en mode Texture ; le culling écarte
    une fraction assertée sur `demo-salles` ; un test négatif démontre qu'une double émission
    dépasse le plafond. Déterministe, sans GPU (`EX-NFR-004`).
  - **Compteur de diagnostic en jeu** (`hmi::DiagnosticsHud`, touche **`F9`** non remappable) :
    cadence de rendu (moyenne glissante), primitives composées/soumises, passes de dessin, pas de
    simulation consommés — désactivé par défaut, sans effet sur la simulation.
  - **Mesures de référence datées** consignées dans le [guide du rendu](@ref guide-rendu).
- **LOT-60 — Audio : socle et bruitages** (lève `EX-REN-040`, marqué ⚠️ depuis sa rédaction ;
  déclare `EX-REN-047`, `EX-REN-048`). Le jeu produit enfin du son.
  - **`hmi::AudioEngine`** (Qt Multimedia, `QSoundEffect`) : détection du périphérique de sortie,
    repli **muet** sans plantage (`EX-NFR-040`), volume borné, tourniquet de trois instances par
    événement (un déclenchement en rafale se recouvre sans s'interrompre lui-même).
    `Qt6::Multimedia` provisionné comme composant additionnel de Qt (même garde que
    `Widgets`/`Gui`), sur les six points `install-qt-action` de `ci.yml`/`release.yml`.
  - **`hmi::SoundCatalog`** : nom d'événement → fichier (`Source/Elements/Audio/sounds.json`),
    même patron que `hmi::SkinCatalog` (fichier absent → catalogue vide, entrée malformée → échec
    entier, jamais deviné). Douze bruitages **réels**, libres de droit (CC0, Kenney), remplacent
    l'idée initiale de sons procéduraux — crédits dans `Source/Elements/Audio/CREDITS.md`.
  - **`hmi::GameEvents`/`hmi::SoundTriggers`** : détection pure des transitions de jeu (saut,
    atterrissage, dash, mécanismes, mort, victoire) au pas fixe, table événement → son exhaustive,
    réutilisable telle quelle par un futur système de particules (`LOT-53`). Deux accesseurs
    ajoutés à `Core` pour exposer ce qu'il est seul à savoir : `core::Player::justJumped` (front
    de saut, aucune combinaison des champs existants n'était fiable) et
    `core::MechanismController::isContinuous` (distinction interrupteur/plaque).
  - **Volume réglable et persisté** (`QSettings`, même portée que la langue/le mode de rendu),
    effet immédiat, retour sonore au relâchement du curseur.
  - **Écran Crédits** dans le menu principal (développement, bruitages).
- **LOT-59 — Boucle de jeu complète : pause, fin de niveau, progression** (`EX-IHM-004`,
  `EX-IHM-005`, `EX-LVL-013`, `EX-LVL-014` ; lève `EX-REN-031` et `EX-GP-040`, tous deux marqués ⚠️
  depuis leur rédaction). Premier lot de **contenu** du programme `0.1.0` : le moteur était
  complet, le *jeu* ne l'était pas.
  - **Machine à états d'écran** (`hmi::ScreenFlow`, logique pure) : deux écrans supplémentaires,
    `Pause` et `NiveauTermine`, recouvrant la scène en jeu sans jamais devenir une page du
    `QStackedWidget` (patron de superposition à un `QWidget::createWindowContainer`) ; un
    septième, `LevelSelect`, atteint depuis le menu.
  - **Écran de pause** : Échap/bouton manette B ouvre la pause au lieu de quitter directement.
    Simulation réellement suspendue (aucun pas de temps fixe consommé, pas un `dt` multiplié par
    zéro) ; réarmement de l'horloge de référence à la reprise pour éviter un rattrapage massif de
    pas ; `hmi::InputState::beginFrame` continue d'être appelé en pause pour qu'un bouton manette
    tenu ne fasse pas osciller entrée/sortie.
  - **Écran de fin de niveau / fin de séquence** : une réussite fige la scène et ouvre un écran
    (Continuer/Rejouer, ou retour au menu après le dernier tableau) au lieu d'enchaîner
    instantanément sur le suivant.
  - **Séquence de niveaux en donnée de contenu** (`core::LevelSequenceLoader`, même patron que
    `core::LevelLoader`) : `sequence-demo.json` remplace le littéral C++ de `MainWindow`.
    `scripts/check_demo_sequence.py` compare désormais ce fichier au test système.
  - **Progression persistée** (`hmi::Progression`, `Settings/progression.json`, écriture
    atomique) : tableau atteint et tableaux terminés stockés par **nom**, jamais par indice, pour
    résister à un réordonnancement de la séquence.
  - **Sélection de niveau côté joueur** : « Jouer » devient trois entrées (Continuer/Nouvelle
    partie/Choisir un niveau). Règle de déverrouillage pure et testée
    (`hmi::isLevelUnlocked`) : les tableaux terminés plus le premier non terminé sont jouables,
    revalidée avant tout lancement (défense en profondeur). Les niveaux personnels du dossier
    (créés dans l'éditeur) sont jouables hors séquence sans jamais toucher la progression.
- **LOT-58 — Vérification Release, sanitizer et analyse statique** (`EX-NFR-023`, `EX-NFR-024`,
  rattache `EX-NFR-003`) : la CI tient enfin les promesses que le dépôt écrivait déjà. Cinq
  vérifications, déclarées ou configurées depuis le début du projet et exécutées **nulle part**,
  rejoignent les contrôles requis pour merger.
  - **Job `build-test-release`** : build et `ctest` en configuration **Release** sur chaque PR,
    plus les presets `vs-release`/`ninja-release`. Jusqu'ici, le premier build Release d'un cycle
    avait lieu dans `release.yml`, **après** le tag — un cas réel (`90f85254`) a déjà coûté une
    casse Release-only découverte trop tard.
  - **Job `sanitize`** : `UnitTests`, `IntegrationTests` et `SystemTests` sous AddressSanitizer.
    `EX-NFR-003` était orpheline depuis le `LOT-01` ; un vrai `heap-use-after-free` a été trouvé et
    corrigé dans `LoggerTest.ClearSinksArreteLaDiffusion` (le test inspectait un sink après que
    `Logger::clearSinks()` l'ait détruit).
  - **Job `clang-tidy`** : analyse ciblée sur le diff de chaque PR. `bugprone-*` ramenée à zéro
    (cinq sous-checks documentés comme non pertinents pour ce projet — idiomes Qt/D3D11/`Result`)
    et rendue **bloquante** ; les autres familles restent consignées, non bloquantes.
  - **Job `format`** : `clang-format --dry-run --Werror`, version LLVM épinglée. Reformatage
    initial isolé (192 fichiers, aucun changement de comportement, `943/943` tests verts après).
  - **Couverture étendue** : agrège désormais `UnitTests` + `IntegrationTests` + `SystemTests`
    (jusqu'ici, seul `UnitTests` était mesuré) avec un seuil qui fait échouer la CI en cas de
    chute. Mesurée à **93.66 %** (build local Ninja Debug, 2026-08-10), seuil posé à 85 %
    (marge ~8.5 points) — à confirmer sur la mesure réelle en CI (preset `vs`, voir
    `tache-05-couverture.md`).
  - Outils épinglés : LLVM `18.1.8` (clang-tidy, clang-format), OpenCppCoverage `0.9.9.0`.
- **Cadrage du programme `0.1.0`** (`LOT-58` → `LOT-66`) : documentation seule, aucun code. Issu
  d'un audit du dépôt comparant l'état livré à ce que les spécifications promettent déjà. Deux
  familles d'écarts, traitées ensemble parce qu'elles se protègent l'une l'autre.
  - **Complétude produit** — le moteur est complet, le *jeu* ne l'est pas. `LOT-59` (boucle de jeu :
    écrans de pause et de fin de niveau, séquence sortie du littéral C++ de `MainWindow`,
    progression persistée, sélection de niveau côté joueur) lève `EX-REN-031` et `EX-GP-040`, tous
    deux marqués ⚠️ depuis leur rédaction. `LOT-60` (audio) lève `EX-REN-040` : le jeu est
    aujourd'hui **totalement silencieux**, alors que `vision.md` place les bruitages dans le MVP —
    la bibliothèque retenue est **Qt Multimedia**, par cohérence avec une interface déjà
    intégralement Qt depuis le `LOT-38`. `LOT-63` livre les mécanismes que le référentiel annonce
    sans les avoir : clé et porte verrouillée (`EX-GP-023`, ⚠️ optionnel MVP), action « Interagir »
    (seule ligne du tableau des contrôles sans identifiant d'exigence), plateforme mobile. Le
    `LOT-53` (effets et particules), cadré de longue date et resté non commencé, rejoint le
    programme.
  - **Durcissement d'ingénierie** — trois vérifications étaient **déclarées** et exécutées **nulle
    part**. `LOT-58` les met en œuvre : la CI ne construisait ni ne testait **jamais** en
    configuration Release (le premier build Release d'un cycle avait lieu *après* la pose du tag —
    une casse Release-only a déjà été livrée, cf. `90f85254`) ; AddressSanitizer n'avait pas tourné
    depuis le `LOT-01`, faisant de `EX-NFR-003` une exigence **orpheline** ; `clang-tidy` et
    `clang-format` n'étaient câblés nulle part malgré `conventions.md`. S'y ajoutent la couverture,
    produite sans seuil et mesurée sur les seuls tests unitaires. `LOT-61` donne une trace
    exploitable à une version publiée — `main()` n'installe aucun sink en Release, donc un défaut
    signalé par un joueur n'est accompagné d'aucun élément. `LOT-62` transforme `EX-NFR-005`
    (« borné et observable ») et `EX-NFR-001` (60 images/seconde) en garanties assertées plutôt que
    déclaratives.
  - **Contrôle du cadrage et contenu à niveau** — deux manques relevés en cours d'audit. `LOT-64` :
    la caméra est aujourd'hui **entièrement automatique** (niveau entier sous 24 × 14 tuiles,
    salle par salle au-delà, deux **constantes de compilation**) et le moteur n'a **aucune caméra de
    suivi** ; le cadrage devient une donnée du niveau, choisie et prévisualisée dans l'éditeur, avec
    trois modes. `LOT-65` : les quinze tableaux datent du `LOT-25` et n'exploitent ni les mécaniques
    ni l'habillage ajoutés depuis — un garde-fou **dérivé des énumérations du code** échouera tant
    qu'une mécanique livrée n'apparaîtra dans aucun niveau franchi par le test système.
  - `LOT-66` clôt le programme : Qt épinglé en local comme en CI (`EX-BUILD-010` — le CMake retient
    aujourd'hui la version *la plus récente installée*), numéro de version **généré** au lieu d'être
    recopié dans le `Doxyfile`, statuts des spécifications sortis de « brouillon », exigences
    orphelines requalifiées, puis bascule `0.1.0`.
  - Nouvelles exigences déclarées ici : `EX-IHM-004`, `EX-IHM-005`, `EX-LVL-006`, `EX-LVL-013`,
    `EX-LVL-014`, `EX-LVL-015`, `EX-NFR-042`, `EX-REN-016`, `EX-REN-047`, `EX-REN-048`,
    `EX-CTRL-022`, `EX-EDIT-028`, `EX-GP-026` — deux exigences de qualité CI supplémentaires sont
    déclarées et exécutées par le `LOT-58` ci-dessus. `vision.md` lève la sauvegarde de
    progression de son hors-périmètre MVP.
  - Les lots sont numérotés **dans leur ordre d'exécution** — aucun n'était livré au moment du
    cadrage, ce que la règle de stabilité des numéros (`Documentation/Lot/lots.md`) autorise. Seul
    le `LOT-53`, déjà publié sous ce numéro, conserve le sien et s'intercale entre le `LOT-60` et le
    `LOT-61`. Principe d'ordonnancement : le **durcissement précède** le contenu qu'il doit
    protéger, et la refonte des niveaux vient en dernier puisqu'elle consomme tout le reste.

## [0.0.5] - 2026-08-10

> Cinquième jalon : le moteur est **habillé**. Le programme `LOT-40` → `LOT-55`, ouvert juste après
> le jalon précédent, est livré en entier : rendu texturé multicouche avec culling, skins de tuiles
> et **raccords automatiques**, texture par instance, fonds de niveau, **décors libres** hors grille
> avec parallaxe, moteur d'**animation piloté par données**, personnage habillé depuis une
> spritesheet externe, retour du **texte dans la scène** et affichage tête haute, et enfin les
> **ombres du plan physique** (LOT-55) — le tout derrière une bascule `F8` qui restitue à tout
> moment la lecture nue des collisions.
>
> L'**éditeur** change d'échelle en parallèle : bibliothèque d'assets à vignettes avec rechargement
> à chaud (LOT-43), manipulation complète des décors (LOT-50), mode d'inspection par calque
> (LOT-51), puis trois lots qui s'attaquent à l'interface elle-même — un **système de design**
> assumé, thème clair/sombre compris (LOT-56), une **redistribution de l'information** où ce qui
> informe devient permanent et ce qui commande devient unique (LOT-57), et un **atelier pixel art
> intégré** (LOT-54) qui ferme la boucle entre dessiner un asset et le voir dans le niveau.
>
> Cette version retire aussi le dernier morceau d'IHM « maison » (LOT-38 Étape B) : il ne reste
> qu'une application Qt. **943 tests** (541 au jalon précédent).
>
> Voir le détail ci-dessous (LOT-38 Étape B, LOT-39 → LOT-57).

### Ajouté
- **LOT-55 — Ombres du plan physique** (`EX-REN-045`, `EX-ARCH-012`) : **dernier lot du programme
  d'habillage** `LOT-40` → `LOT-55`. Le calque `RenderLayer::Shadow`, réservé sans être utilisé
  depuis le `LOT-40`, s'active enfin — entre `Decor` et `Tile`, donc sous les tuiles et au-dessus du
  fond et des décors d'arrière-plan. L'objectif est de **lecture**, pas d'esthétique : distinguer
  d'un coup d'œil ce qui est **physique** (solide, donc porteur) de ce qui n'est que décor — le
  complément exact du calque de premier plan (`LOT-49`), qui dit l'inverse.
  - `hmi::composeShadows` (`HMI/Graphics/ShadowRenderer.h`) parcourt les mêmes entités que
    `hmi::composeWorldSprites` et n'en retient que celles qui projettent une ombre : pleines
    (`core::isSolid`) ou à silhouette inclinée/courbe (`hmi::hasSilhouette`, `LOT-42`). La région
    échantillonnée est directement `hmi::regionForTile` — le **même** atlas procédural que le mode
    Physique, déjà opaque exactement là où la matière est présente. Teinter ce quad en noir
    semi-transparent, décalé d'un pixel, donne donc l'ombre à sa **forme réelle** (pente, arrondi,
    bloc réduit via `core::tileVisualScale`) sans réimplémenter la moindre géométrie et **sans aucun
    nouveau prédicat de solidité dans `Core`** : une ombre est la projection d'une forme, pas d'un
    degré de solidité, et `Core` expose déjà cette forme sous une version plus riche qu'un booléen.
  - Une **porte** fait exception à la règle « ombre = type statique » : son type reste `Door` quel
    que soit l'état du mécanisme, alors que sa solidité réelle dépend de l'interrupteur.
    `composeShadows` accepte donc une grille de collision optionnelle
    (`MechanismController::collisionMap`, fournie par `GameSession`) et tranche sur l'état
    **courant** — fermée, elle projette ; ouverte, elle ne projette plus. `hmi::DraftRenderer`, qui
    ne simule aucun mécanisme, ne la fournit pas : dans l'éditeur une porte n'a jamais d'ombre, état
    normal et non défaut.
  - Actif en `RenderMode::Texture` **uniquement**, aucun effet sur le gameplay (`EX-ARCH-012`), même
    culling que le reste, masquable par l'axe `Shadow` de `hmi::LayerVisibility` (`LOT-51`). Un bloc
    poussable en mouvement voit son ombre suivre par la même interpolation
    (`hmi::PreviousPosition`) que son propre sprite. Un niveau sans fond ni décor n'a simplement
    aucune surface pour recevoir l'ombre — pas d'erreur, pas de cas particulier.
  - Documenté dans `Documentation/Guide/guide-rendu.md`, qui remplace au passage sa section
    d'orientation « ce qui vient ensuite » (obsolète depuis plusieurs lots) par l'état livré complet
    du programme d'habillage. **943 tests verts**, build `/W4 /WX` propre.

- **LOT-54 — Atelier pixel art intégré** (`EX-EDIT-045`) : créer et modifier les fichiers d'assets
  de texture (skins, planches, fonds, objets, décors) **sans quitter l'application**, en voyant
  immédiatement le résultat dans le niveau. Depuis le `LOT-43`, on savait importer et recharger un
  asset, jamais le **modifier** : corriger un pixel imposait un aller-retour vers un éditeur externe,
  particulièrement coûteux sur une planche à raccords dont la justesse ne se juge qu'une fois
  assemblée. Ce lot s'exécute délibérément **derrière** `LOT-56` et `LOT-57` — le canevas n'a donc à
  inventer ni son habillage, ni ses commandes, ni son affichage d'état, et le budget ainsi libéré
  finance des fonctions d'édition plutôt que de la plomberie d'interface.
  - **TACHE-01 — Écriture d'image** : `hmi::encodeImageFile`, symétrique exact de `decodeImageFile`
    (`LOT-40`), écriture PNG **atomique** (fichier temporaire puis remplacement) depuis un tampon
    RGBA non prémultiplié — le rechargement à chaud surveille le même dossier et pourrait lire un
    fichier à demi écrit. `--export-atlas` passe désormais par ce même chemin. `UnitTests` reste
    constructible sans Qt (le seul test touchant Qt est ajouté conditionnellement).
  - **TACHE-02 — Opérations et historique** : `hmi::PixelOperations` (pinceau, gomme, ligne de
    Bresenham sans trou au glisser rapide, remplissage par zone contiguë **itératif** — jamais
    récursif —, pipette), fonctions pures sur `hmi::DecodedImage`, sans Qt ni GPU.
    `hmi::PixelHistory` : pile d'annulation **locale au canevas**, totalement indépendante de
    `core::LevelDraft` (annuler un coup de pinceau n'annule jamais une pose de tuile), à opérations
    **nommées** (clés présentes dans les deux catalogues de traduction), mémorisant des **régions**
    plutôt que des instantanés complets, profondeur plafonnée, retour à un point antérieur en un
    appel.
  - **TACHE-03 — Canevas** : `hmi::PixelCanvasGeometry` (conversions pures vue ↔ image, zoom
    toujours **entier** pour que les pixels restent carrés), réutilisant `hmi::thumbnailPixelSize`
    (`LOT-56`) pour la netteté à toute échelle d'affichage plutôt que de la redéfinir.
    `hmi::PixelCanvas` (`QWidget`) : plus proche voisin, grille de pixels au-delà d'un seuil de zoom,
    damier de transparence. Fond et damier tirés des jetons de portée **invariante** — un fond qui
    changerait de clarté avec le thème fausserait la perception des couleurs posées, rédhibitoire
    pour l'outil dont c'est le sujet. Un geste complet produit **une** entrée d'historique.
  - **TACHE-04 — Actions, barre d'outils, barre d'état** : quatre outils en actions Qt formant un
    groupe exclusif **distinct** de celui des outils de niveau (`EditorActionGroup::PixelTools`),
    icônes dessinées par code. Annuler/Refaire restent une action **unique** à cible contextuelle :
    `PixelCanvas` implémente `hmi::EditContextTarget` et `MainWindow` réassigne la cible au widget
    qui reçoit le focus clavier — le dispatch livré par `LOT-57` n'est pas modifié.
    `hmi::EditorStatus` et `hmi::PanelFocus` sont **étendus** (`PixelEditStatusInfo`, `PanelId::
    PixelCanvas`/`PixelHistory`), jamais doublés. Nouveau `PixelHistoryPanel`.
  - **TACHE-05 — Ouvrir, créer, enregistrer** : `hmi::validAssetSizes` dérive les tailles proposées
    du contrat d'asset (`EX-REN-007`) plutôt que de les redécrire — une création non conforme devient
    impossible au lieu d'être refusée après coup ; les familles à dimensions libres (fond, décor,
    police) ouvrent une saisie libre. Garde-fou d'écrasement nommant les références concernées.
  - **TACHE-06 — Outils de région** : sélection rectangulaire, déplacement, symétries horizontale et
    verticale, rotations par quart de tour, copier/coller par un presse-papiers autonome — toutes
    fonctions pures, exposées comme des actions.
  - **TACHE-07 — Palettes** : `hmi::PixelPalette` (couleurs nommées persistées dans
    `Assets/palettes.json`, même patron que `SkinCatalog` — fichier absent traité comme une palette
    vide, entrée malformée ignorée plutôt que d'abandonner toute la palette),
    `hmi::extractPalette` (ordre déterministe, occurrences, jamais l'alpha nul) et
    `hmi::nearestPaletteColor` (départage stable, alpha d'origine préservé). Panneau d'édition et
    réglage « contraindre à la palette », persisté. Un sélecteur de couleur libre a été ajouté dans
    la foulée : pipette et pastilles étaient les deux seuls moyens de changer de couleur, aucun ne
    permettant d'en choisir une absente des deux.
  - **TACHE-08 — Aperçu live et planche à raccords** : `MainWindow::updateLivePreview` écrit l'image
    en cours et invalide **ciblément** son entrée de `TextureCache` (`GameViewport::invalidateAsset`,
    prévu depuis `LOT-43`) à chaque geste complet — jamais par pixel. `TileAutotile` est étendu
    (`autotileConfigurationLabelKey`, `autotileAssemblyMasks`) pour que la **même** table canonique
    décrive aussi les seize configurations en langage naturel et fournisse l'assemblage 3×3, jamais
    une seconde table. `hmi::PixelAutotilePreview` (pur) détecte une planche 4×4 et compose l'aperçu
    d'assemblage depuis le tampon en mémoire ; le canevas y ajoute repères de cases et infobulle
    nommant la configuration survolée.
  - **Non livrés, actés dans l'épic** : le point d'entrée depuis le panneau Textures n'est pas câblé
    (TACHE-05), et l'aperçu **d'animation** n'est pas livré faute de temps pour intégrer
    `AnimationCatalog` au canevas (TACHE-08) — l'aperçu de **raccords**, lui, l'est.
  - Documenté dans `Documentation/Guide/guide-atelier-pixel-art.md`. Build `/W4 /WX` propre.

- **LOT-57 — Architecture de l'information de l'éditeur** (`EX-IHM-060` à `EX-IHM-062`) :
  redistribution de l'éditeur — ce qui informe devient permanent, ce qui commande devient unique, ce
  qui ne sert qu'à un outil s'efface quand cet outil n'est pas actif.
  - **TACHE-01 — Barre d'état structurée** : remplace la ligne unique `status.edit_help` (figée à
    l'entrée en mode éditeur, définitivement effacée par le premier message transitoire) par cinq
    zones **permanentes** (`hmi::EditorStatus`, fonction pure sur le patron de `hmi::gameHudLines`,
    `LOT-52`) — niveau ouvert, modifications non enregistrées, outil actif, case survolée, zoom —
    ajoutées à la barre d'état via `addPermanentWidget` (jamais recouvertes par un message). Aide
    contextuelle à l'outil actif, restaurée automatiquement à l'expiration d'un message transitoire
    (`MainWindow::refreshStatusHelp`/`showTransientStatusMessage`, minuteur unique). Case survolée et
    zoom nouvellement exposés par `GameViewport` (`hoveredCell()`/`zoom()`, signaux `hoveredCellChanged`/
    `zoomChanged`, émis seulement sur changement réel). **6 nouveaux tests**, sans Qt/GPU ; build
    `/W4 /WX` propre, suite verte.
  - **TACHE-02 — Regroupement des panneaux, suivi de l'outil actif** : les panneaux Niveaux, Liens et
    Textures sont désormais regroupés en onglets par défaut (`tabifyDockWidget`, disposition v4,
    invalide les dispositions antérieures), chacun restant individuellement déplaçable, détachable et
    refermable. L'onglet pertinent est mis en avant à chaque changement d'outil (`hmi::panelForTool`,
    table pure sur le patron d'`ActionCatalog`) tant que l'utilisateur n'a rien imposé lui-même (choix
    manuel d'onglet, déplacement de panneau) — jamais un masquage, réglable et persisté depuis le menu
    Affichage. **4 nouveaux tests**, sans Qt ; build `/W4 /WX` propre, suite verte.
  - **TACHE-03 — Recentrage du panneau Textures** : l'onglet Calques (mode d'inspection « définition
    des textures », `LOT-51`) quitte le panneau Textures pour le menu Affichage — une entrée par
    calque dans l'ordre de dessin, plus « tout afficher », sans changement de comportement
    (`EX-EDIT-044` inchangée, jamais lue par `GameSession`). L'avertissement permanent qu'imposait sa
    présence dans le panneau devient inutile dans son nouvel emplacement. Le panneau Textures ne
    porte plus que la définition d'apparence (Skins, Fond, Objets, Animations, Décors) ; les deux
    sélecteurs de jeu de skins (session d'édition vs. niveau) portent désormais chacun une infobulle
    distincte. Aucun nouveau test (changements Qt purs) ; build `/W4 /WX` propre, suite verte.
  - **TACHE-04 — Déduplication des commandes et raccourcis** : `hmi::EditorKeyBindings` définissait
    dix actions d'éditeur remappables dont neuf n'étaient jamais lues (raccourcis interceptés en
    dur, non remappables ; Copier/Coller au clavier bypassaient même `EditorKeyBindings`). Toutes
    passent désormais par `hmi::EditorActions` (`hmi::keyBindingIconCatalog`, table pure liant
    action remappable et commande du catalogue) : `EditorActions::applyShortcuts` synchronise le
    raccourci **effectif** de chaque `QAction` depuis les touches remappées, y compris après un
    remappage à chaud. Nouvel onglet « Éditeur » de la page Options (`EditorKeybindingsWidget`,
    même patron que le remappage clavier de jeu). Renommer (F2) renomme désormais le niveau ouvert
    (`GameViewport::renameOpenLevel`, réutilise `LevelFileOperations`/`LevelNameValidation` comme
    `LevelBrowserPanel`) ; l'aide (F1) ouvre un aperçu des raccourcis lisant les touches effectives,
    jamais un texte figé. La bascule Physique/Texture (case dupliquée retirée en TACHE-03) rejoint
    le menu Affichage comme entrée unique de l'action déjà existante (`EX-IHM-062`). Annuler/Refaire/
    Copier/Coller dispatchent désormais via `hmi::EditContextTarget`, interface que `GameViewport`
    implémente — le seuil de dispatch qu'un futur atelier pixel art (`LOT-54`) réutilisera pour sa
    propre cible sans le réécrire. Le doublon de sélecteur de couche de décor (panneau Outils vs.
    onglet Décors) est conservé : les deux ciblent des états distincts (couche du prochain décor
    posé vs. couche du décor sélectionné existant), pas un doublon strict. **3 nouveaux tests** (dont
    un garde-fou cassant si une action est ajoutée sans être branchée) ; build `/W4 /WX` propre, suite
    verte.
  - **Amendement post-essai manuel** : premier essai réel de l'éditeur reconstruit, deux retours
    tranchés dans la foulée. Le panneau « Outils » (`ToolPanel`), qui ne portait déjà plus que le
    strict nécessaire de l'outil Décor depuis `LOT-56`, devient le panneau **Décors**
    (`DecorsPanel`) et regroupe désormais aussi l'inspecteur des décors posés déplacé de l'onglet
    « Décors » du panneau Textures — les deux sélecteurs de couche (placement du prochain décor,
    couche du décor sélectionné) coexistent donc maintenant dans le même panneau, renommés
    explicitement pour lever toute ambiguïté. Le panneau Textures sort du regroupement en onglets de
    TACHE-02 et redevient indépendant, comme Palette/Décors. La barre d'outils du haut, elle, reste
    à sa place (barre globale de la fenêtre principale, hors du panneau Décors). `LAYOUT_VERSION`
    4 → 5.

- **LOT-56 — Système de design de l'IHM Qt** (`EX-IHM-050` à `EX-IHM-055`) : l'éditeur prend enfin
  la main sur sa propre apparence — jusqu'ici le style **natif** de la plate-forme, qui ignorait une
  large part de l'unique feuille de style existante (`theme.qss`, restreinte au menu principal et à
  la page Options par `objectName`, faute de mieux).
  - `hmi::DesignTokens` (`HMI/Interface/DesignTokens.h`) — jetons de design purs (couleurs par rôle,
    espacement, typographie, tailles), en deux portées de structure identique : l'**identité** du
    jeu (menu, Options, jeu — invariante) et le **châssis d'édition** (variable). `hmi::
    ApplicationTheme` choisit le style Qt Fusion avant tout widget, construit la `QPalette`
    complète (actif/inactif/désactivé) et produit `theme.qss` par substitution de marqueurs depuis
    les jetons (`hmi::substituteStyleSheetTemplate`, fonction pure) — plus aucune couleur littérale,
    et une étanchéité entre les deux portées garantie par test. Focus clavier visible partout
    (navigation à la manette, `EX-IHM-040`).
  - Police **Inter** embarquée (`Assets/Fonts/Inter-{Regular,Bold}.ttf`, licence SIL OFL 1.1) avec
    repli sur une famille générique si absente (`hmi::resolveFontFamily`, jamais un second nom codé
    en dur) ; typographie à échelle unique, tailles et marges des `.ui` retirées au profit des
    jetons.
  - Barre d'outils à icônes (`hmi::EditorActions`) remplaçant les boutons radio empilés du panneau
    Outils (`EX-EDIT-015`) : six outils et sept commandes (enregistrer, essayer, annuler, refaire,
    grille, recadrer, mode de rendu) exposés comme des actions Qt **uniques**, simultanément dans le
    menu, la barre d'outils et leur raccourci — plus de double définition. Icônes dessinées par code
    (`hmi::iconGeometry`/`hmi::themeIcon`, géométrie pure + rendu `QPainter`), recolorées depuis les
    jetons.
  - Vignettes de la palette, des grilles d'assets et des lignes du panneau Textures rendues à la
    résolution **réelle** de l'écran (`hmi::thumbnailPixelSize`, fonction pure) : nettes à 100 %,
    125 % et 150 % d'échelle d'affichage, régénérées lors d'un changement d'écran.
  - Thème **clair/sombre de l'éditeur** (`hmi::editorLightTokens`, `hmi::
    resolveEffectiveEditorTheme`), suivant par défaut le réglage du système d'exploitation, réglable
    depuis le menu Affichage (Système/Clair/Sombre) et persisté ; appliqué à chaud (palette, feuille
    de style, icônes) sans redémarrage. Contraste texte/fond vérifié pour les deux thèmes (seuils
    WCAG). L'identité (menu principal, Options, jeu) reste rigoureusement inchangée quel que soit le
    thème actif, y compris après une bascule à chaud — garanti par test.
  - **24 nouveaux tests**, tous sans Qt/GPU (la couche Qt — `ApplicationTheme`, `ThemeIcons`,
    `EditorActions` — reste, comme `BitmapFont`/`GameViewport`, hors `UnitTests` ; seule leur
    logique pure — jetons, gabarit de feuille de style, catalogue d'actions, géométrie d'icônes,
    résolution de thème — y est compilée) ; build `/W4 /WX` propre, suite verte.

- **LOT-52 — Texte, police bitmap et affichage tête haute** (`EX-IHM-003`, re-concrétise
  `EX-REN-032` retirée au `LOT-38`) : le jeu peut de nouveau afficher du texte **dans la scène
  rendue** — les budgets de sauts/dashs (`EX-GP-024`, `LOT-12`) et le nom du tableau, jusqu'ici
  invisibles faute de tout rendu de texte, apparaissent désormais en jeu et en essai.
  - `hmi::ProceduralFont`/`hmi::BitmapFont` — police bitmap chargée depuis `Assets/Fonts/
    font.png` + ses métriques (`font.json`, format JSON versionné, même patron que
    `hmi::AnimationCatalog`), validée par le contrat d'asset (`AssetFamily::Font`, `EX-REN-007`)
    et par sa cohérence avec les dimensions du PNG. Repli **procédural** déterministe si l'atlas
    ou ses métriques sont absents/invalides (glyphes 5×7 pixels, ASCII imprimable + accents
    français `é è à ç ù ê î ô û`, sur le modèle de `hmi::buildProceduralAtlasImage`, `LOT-39`) :
    le jeu reste lisible sans aucun asset de police (`EX-NFR-040`). Aucun asset n'est livré pour
    l'instant (`Source/Elements/Assets/Fonts/README.md`) : le repli procédural est donc le
    rendu actif tant qu'un artiste n'a pas déposé `font.png`/`font.json`. Un caractère non
    couvert est substitué par un glyphe de remplacement, jamais un trou silencieux. Mesure de
    texte pure (`hmi::measureText`), parcourant des **points de code** UTF-8 (pas des octets) —
    piège classique explicitement évité, comme documenté dans l'ancien `hmi::BitmapFont` retiré.
  - `hmi::TextRenderer` (`HMI/Graphics/TextRenderer.h`) — compose une chaîne en `SpriteQuad` sur
    le calque `RenderLayer::UI` (réservé sans être utilisé depuis `LOT-40`), avec ancrage
    paramétrable (gauche/centre/droite, haut/milieu/bas) et positions arrondies au pixel écran
    entier (netteté, `EX-ARCH-022`). **Projection écran dédiée** (`hmi::screenProjectionMatrix`,
    dépendant uniquement des dimensions du viewport) : premier cas du projet où une passe de
    rendu a sa propre projection, indépendante de `Camera2D` — le HUD ne tourne ni ne change de
    taille avec le zoom. Composé dans une `hmi::ComposedScene` **dédiée**, distincte de celle de
    `hmi::SpriteRenderer` et jamais soumise à un cadrage de culling caméra : le texte, en espace
    écran, n'a pas de position monde (`LOT-40` TACHE-05 ne s'y applique pas).
  - `hmi::GameHud`/`hmi::GameSession::renderHud` — fonction **pure** (`hmi::gameHudLines`)
    choisissant les lignes à afficher (compteurs de sauts/dashs, seulement si le budget du
    niveau est **fini** — `-1` = illimité, cas de la grande majorité des tableaux, aucune ligne
    superflue ; nom du tableau) à partir de `core::Player`/`core::Level::name`, testée sans GPU.
    Affichage en jeu et en essai (hérité du point d'entrée unique `hmi::GameSession::render`,
    jamais appelé en édition pure — `hmi::DraftRenderer` reste le seul chemin de l'éditeur) avec
    une ombre portée (décalage d'un pixel) pour rester lisible sur fond clair comme sur fond
    sombre. Nouvelles clés `hud.jumps_remaining`/`hud.dashes_remaining` dans les deux catalogues
    de traduction (`EX-REN-033`).
  - **32 nouveaux tests**, tous sans GPU (`hmi::ProceduralFont`/`hmi::TextRenderer`/`hmi::
    GameHud` sont des fichiers séparés de leurs classes propriétaires de ressources Direct3D —
    `hmi::BitmapFont` n'est pas compilé dans `UnitTests`, comme `hmi::TextureAtlas`) ; build
    `/W4 /WX` propre, suite verte.

- **LOT-51 — Mode d'inspection « définition des textures » par calque** (`EX-EDIT-044`) : nouvel
  onglet « Calques » du panneau Textures — une case à cocher par calque de rendu, dans l'ordre de
  dessin, plus « Physique seul » et « Tout afficher ». Réservé à l'éditeur, **sans aucun effet** ni
  sur `hmi::GameSession` ni sur la bascule `F8` (`LOT-41`), distinction rappelée par une info-bulle.
  `hmi::LayerVisibility` (nouveau) : jeu de visibilités indexé par `RenderLayer`, tout visible par
  défaut, sans persistance entre deux sessions. `hmi::resolveTileAppearance` étend le résolveur
  unique surcharge > skin > damier avec un mode « isoler » : les bits Tuile/Objet pilotent
  désormais les deux axes de résolution d'une tuile plutôt qu'un calque physique distinct, en
  isolant sans repli sur le damier quand un seul axe est actif. `composeWorldSprites`/
  `DraftRenderer::render` filtrent à la **composition** (jamais à la construction de la scène ECS).

- **LOT-50 — Manipulation de décors dans l'éditeur** (`EX-DEC-010`) : outillage d'édition complet
  pour un décor déjà posé (`LOT-49`) — sélectionner, déplacer, redimensionner, pivoter, changer de
  couche et réordonner, en plus du placement/retrait déjà existants.
  - Mutateurs `core::LevelDraft` (`moveDecor`/`resizeDecor`/`rotateDecor`/`setDecorLayer`/
    `bring*Forward`/`send*Backward`/`bring*ToFront`/`send*ToBack`) : position et échelle
    appliquées **atomiquement** pour un redimensionnement, une seule entrée d'historique
    (undo/redo) par geste complet.
  - Géométrie partagée (`hmi::DecorGeometry`) et machine à état pure du geste (`hmi::
    DecorGesture`, même patron que `hmi::LinkGesture`, `LOT-37`) : désignation, distinction
    clic/glisser, poignées (taille écran constante), abandon — jamais de mutation directe du
    brouillon pendant l'aperçu. Remplace `hmi::DecorPlacementGesture` (`LOT-49`), supersedé par la
    détection par rectangle englobant.
  - Section « Décors » du panneau Textures : liste groupée par couche puis par ordre de
    superposition, sélection **croisée** unique avec le canevas, signalement des assets manquants.
  - **Trois défauts corrigés en cours de lot** : `decorWorldBounds` oubliait de convertir les
    pixels de l'asset en unités monde (rectangle englobant, donc poignées et cadre de sélection,
    seize fois trop grands) ; la rotation posée par la poignée dédiée n'avait aucun effet visible
    (`core::Transform::rotation` était ignorée au rendu depuis `LOT-49` — `hmi::SpriteQuad` porte
    désormais une rotation optionnelle, appliquée par `SpriteBatch::draw`, coins tournés autour du
    centre) ; le cadre de sélection et ses poignées restaient alignés aux axes pendant que le
    décor tournait sous eux (`hmi::decorRotatedPoint`, même formule de rotation que le rendu).

- **LOT-49 — Décors libres, rendu multicouche et parallaxe** (`EX-DEC-001`, `EX-DEC-002`,
  `EX-DEC-006`) : `core::Decor` (position/échelle/rotation libres, hors grille, couche,
  statique/manipulable) sérialisé dans le format de niveau versionné (`LOT-44`). Rendu sur les
  calques arrière-plan et premier plan (le premier plan passe **au-dessus** du personnage), repli
  sur le damier magenta pour un asset introuvable ; **parallaxe** relative au centre de la salle
  courante, cohérente avec la caméra à coupure nette entre salles (`LOT-32`). Placement minimal
  dans l'éditeur (poser/supprimer) depuis une nouvelle bibliothèque `Assets/Decors/`.

- **LOT-48 — Personnage habillé depuis une spritesheet externe** : dernier sprite resté hors du
  programme d'habillage — en mode Texture, le personnage retombait jusqu'ici sur le damier
  magenta. Spritesheet externe (`Assets/Player/`) avec repli procédural, taille de l'image
  **découplée** de la hitbox par un point d'ancrage centre-bas (une image plus grande que la
  hitbox ne la déforme donc plus), projection état → clip étendue à la chute, l'atterrissage, la
  glissade murale et le dash (clips prioritaires, résolus sans nouveau champ sur `core::Player`),
  personnage retourné selon son sens de déplacement.

- **LOT-47 — Apparence des mécanismes pilotée par leur état logique** (`EX-REN-006`) : une porte,
  un interrupteur, une plaque de pression et les dangers commuté/temporisé/mobile changent
  désormais d'**apparence** selon leur état en mode Texture, plutôt que la simple modulation
  d'opacité qu'appliquait `GameSession` jusque-là. `hmi::MechanismVisuals` traduit l'état lu dans
  `Core` en clip attendu (infrastructure `LOT-46`) ; un clip manquant retombe proprement sur
  l'image statique. La modulation d'opacité de diagnostic reste réservée au mode **Physique**,
  désormais isolée dans une fonction pure. Nouvelle section « Animations » du panneau Textures
  (asset par défaut par famille de mécanisme, diagnostic des clips manquants, aperçu).

- **LOT-46 — Moteur d'animation générique piloté par données** (précise `EX-REN-012`) : remplace
  l'`enum AnimationClip` figé (`Idle`/`Run`/`Jump`) par des clips-**données**
  (`core::AnimationClip`/`core::ClipSet`) et généralise `AnimationSystem` à **toute** entité
  portant `core::Animation`, plus seulement le personnage. Nouveau format `nom-asset.anim.json`
  (`hmi::AnimationCatalog`) décrivant une spritesheet animée, mis en cache et invalidé
  **conjointement** par `TextureCache`. Anime les skins de tuiles (eau, lave, torche) via une
  horloge **partagée par asset**, résolue à la composition du rendu plutôt qu'écrite par tuile —
  toutes les tuiles d'un même type animé restent ainsi en phase, sans coût par case. Le personnage
  est migré à l'identique (mêmes durées, mêmes images), non-régression attestée par le test de
  référence existant. **Deux défauts corrigés en cours de lot** : le canevas d'édition
  (`hmi::DraftRenderer`, rendu en continu hors essai) composait toujours la première image d'un
  asset animé, sans jamais progresser (horloge partagée factorisée entre pas fixe déterministe du
  jeu et temps réel de l'aperçu d'édition) ; `TextureCache::getAnimation` résolvait le descripteur
  à la racine du dossier `Assets` plutôt que dans `Assets/Skins/`, empêchant silencieusement
  toute tuile de s'animer.

- **LOT-45 — Texture par instance sur les objets interactifs** : assigne une texture à une case
  **précise** d'un niveau, prioritaire sur le skin de son type (`LOT-42`) — `core::
  TileTextureOverride` sur `Level`/`LevelDraft` (JSON, nettoyage, undo/redo), outil `hmi::
  EditorTool::TextureAssign` avec geste pur (`hmi::TextureAssignGesture`) et raccourci `T`,
  résolveur de priorité **unique** (surcharge > skin > damier) dans `hmi::resolveTileAppearance`,
  partagé par le jeu et l'éditeur. Section « Objets » du panneau Textures (choix d'asset, liste
  des surcharges, retrait, surbrillance croisée) et dossier `Assets/Objects/`. Corrige au passage
  une régression silencieuse survenue lors de la réécriture Qt/Direct3D 11 (`LOT-33`-`35`) : le
  pan/zoom manuel de l'éditeur (molette, glisser bouton droit, touche `0`) avait disparu, la
  caméra recalculant un cadrage automatique à chaque image.

- **LOT-44 — Fond de niveau et versionnement du format** : associe un fond à un niveau (calque
  `Background`, ratio préservé, recadrage par le centre, repli en damier si introuvable) et
  introduit le **numéro de version** du format JSON de niveau — première extension de
  `core::Level`/`LevelDraft` du programme d'habillage, pour que les formats suivants (décors,
  surcharges de texture) puissent évoluer sans casser les niveaux déjà écrits. Section « Fond » et
  sélecteur de jeu de skins **du niveau** (distinct du jeu de skins courant d'édition) ajoutés au
  panneau Textures.

- **LOT-43 — Bibliothèque d'assets à vignettes, gestion de fichiers et rechargement à chaud** :
  le panneau Textures affichait les skins par nom de fichier et imposait de passer par
  l'explorateur puis de relancer l'application à chaque retouche d'asset — un coût payé à chaque
  itération des lots d'habillage suivants. Widget de vignettes partagé (`hmi::
  AssetThumbnailView`), import/renommage/duplication/suppression avec **détection des
  références** (`skins.json`), et rechargement à chaud (invalidation `TextureCache` + relecture du
  catalogue) sans reconstruire la scène ni toucher au brouillon en cours d'édition. La
  mémoïsation/invalidation de `TextureCache` est extraite dans `hmi::CacheRegistry`, un registre
  générique testable sans GPU dont `LOT-40` dépendait déjà sans qu'elle soit vérifiée isolément.

- **LOT-42 — Skins de tuiles, raccords automatiques et panneau Textures** : premier lot de
  contenu visuel du programme d'habillage — le mode Texture (`LOT-41`) cesse d'afficher un damier
  partout.
  - `hmi::SkinCatalog` — associe un type de tuile à un asset et un mode de découpage, en **jeux
    nommés** ; format versionné dès sa création (une version supérieure à celle gérée est refusée
    plutôt que lue au mieux), aucune exception ne franchit la lecture (`EX-NFR-040`, patron de
    `core::LevelLoadResult`).
  - `hmi::TileAutotile` — table de **raccords automatiques** par masque de voisinage solide
    (quatre voisins, seize configurations, planche 4×4) : une image unique par type rendait la
    grille visible et le dessus d'une plateforme indiscernable de son intérieur. L'extérieur du
    niveau compte comme solide et le raccord suit la **solidité**, pas le type (un bloc poussable
    jouxtant un mur ne laisse pas de couture) ; les pentes/arrondis, jamais solides, ne
    participent pas au voisinage.
  - Skins appliqués au rendu via `hmi::TileSkinTag` (composant de présentation, même patron que
    `RenderLayerTag`) et le résolveur unique `hmi::resolveTileAppearance` (priorité skin >
    damier) ; le masque de voisinage est calculé **une fois** à la construction, indépendant du
    mode et du jeu de skins, pour que `F8` ou une réassignation se voient à l'image suivante sans
    reconstruire l'ECS.
  - **Détourage** des skins des douze types à silhouette inclinée/courbe (`hmi::
    isInsideSilhouette`, point de vérité unique partagé avec l'atlas procédural) : un skin fourni
    par l'auteur reste une image carrée, le moteur le découpe à la forme exacte de la hitbox
    (`core::slopeSurfaceHeight`), jamais anticrénelée (filtrage *nearest*, `EX-ARCH-022`).
  - Dock **« Textures »**, organisé en onglets dès ce lot bien qu'il n'en compte qu'un — sans
    cette structure, chacun des lots d'habillage suivants (Fond, Objets, Animations, Décors)
    aurait créé son propre panneau. Fichier et mode choisis dans une liste fermée (balayage de
    `Assets/Skins/`, jamais une saisie de chemin).
  - Palette de l'éditeur rendue **fidèle** au mode de rendu courant (`hmi::paletteThumbnail`,
    fonction pure appliquant exactement la même priorité que `hmi::resolveTileAppearance` —
    vérifié par test qu'elles ne divergent jamais) : texture réelle en mode Texture, couleur plate
    en mode Physique, damier pour un type non habillé.
  - **Portabilité du build** corrigée à l'ouverture du lot : sur une configuration neuve, aucun
    exécutable n'était produit (Qt non découvert par `find_package` hors `CMAKE_PREFIX_PATH`, et
    l'architecture du terminal appelant décidait silencieusement de celle du build, x86 depuis une
    « Developer PowerShell »). `scripts/build.ps1` établit désormais lui-même l'environnement x64
    (`vswhere`), et un garde-fou rejette les deux cas à la configuration CMake.

- **LOT-41 — Bascule Physique/Texture (`F8`)** (`EX-REN-046`) : une commande **fixe et non
  remappable** bascule, en édition, en essai **et** en jeu réel, entre le rendu **Physique**
  (couleur plate par type de tuile — la lecture directe des collisions, comportement historique
  strictement inchangé) et le rendu **Texture** (habillage, construit à partir de `LOT-42`).
  - `hmi::RenderMode` et `hmi::resolveTileAppearance` — **point de résolution unique** de
    l'apparence, appelé à la **composition** et non à la construction de la scène : basculer de
    mode ne reconstruit jamais l'ECS, ne coûte aucun pas de simulation et n'a aucun effet rémanent.
    Le mode Texture affiche pour l'instant le damier magenta partout, ce qui est le comportement
    attendu tant qu'aucun skin n'existe (`EX-NFR-040`). La géométrie composée est **identique**
    dans les deux modes : seule la texture échantillonnée change.
  - `F8` est traité en dur dans `hmi::GameViewport::keyPressEvent`, avant toute autre branche pour
    couvrir les trois contextes — même parti pris que `F10` (grille de repère). La touche
    n'apparaît dans **aucune** table de remappage : `hmi::qtKeyToHmiKey` ne traduit pas
    `Qt::Key_F8`, elle ne peut donc structurellement pas être liée à une action (`EX-CTRL-012`).
  - **Défaut `Texture` dans toutes les configurations de build** et **persistance** du dernier
    choix entre deux sessions (`QSettings`, `EX-IHM-011`) : deux binaires du même code ne doivent
    jamais afficher un rendu différent par défaut, et reperdre son mode d'affichage à chaque
    lancement serait une friction quotidienne pendant treize lots. Une préférence absente, vide ou
    corrompue retombe silencieusement sur `Texture`.
  - `hmi::TextureCache` est désormais **câblé en production** (porté par `hmi::GameViewport`,
    propriétaire du damier partagé et point d'entrée des skins à partir de `LOT-42`).

- **LOT-40 — Fondations du rendu texturé : calques nommés, multi-textures, testabilité, culling**
  (`EX-REN-043`, `EX-REN-007`, `EX-NFR-004`, `EX-NFR-005`, précise `EX-REN-014`) : lot **structurel**
  qui ne change aucun pixel affiché, mais lève les quatre verrous qui bloquaient tout le programme
  d'habillage.
  - `hmi::RenderLayer` — ordonnancement de calques **unique et explicite** (fond, décor, ombres,
    tuiles, objets, personnage, premier plan, interface, aides d'édition), réservé en entier dès
    maintenant ; le premier plan est au-dessus du personnage par construction (`EX-DEC-002`). Les
    valeurs de calque magiques (`layer = 100` du joueur, `0` des tuiles) ont disparu au profit du
    composant de présentation `hmi::RenderLayerTag` ; `core::Sprite::layer` conserve son rôle de tri
    **fin à l'intérieur** d'un calque et `Core` continue d'ignorer les calques (`EX-NFR-011`).
  - **Rendu multi-textures** : la composition regroupe les primitives par `(calque, texture)` et le
    rendu émet une passe `begin/end` par groupe contigu, dans l'ordre des calques. Le contrat public
    de `hmi::SpriteBatch` est strictement inchangé ; ses primitives (`SpriteQuad`, `LineQuad`) sont
    extraites dans `HMI/Graphics/Quad.h`, sans dépendance Direct3D.
  - `hmi::ComposedScene` / `hmi::QuadRecorder` — séparation **composition / soumission** : la liste
    ordonnée des primitives d'une image est produite sans device Direct3D, donc **assertable** par un
    test (ordre des calques, contiguïté des groupes de texture, dénombrement, présence d'une
    primitive). Les critères « rendu identique » et « ordre de calque correct » cessent d'être des
    vérifications à l'œil (`EX-NFR-004`).
  - `hmi::TextureCache` — registre de textures chargées à la demande par **nom logique**, bâti sur
    `hmi::TextureLoader`/`hmi::AssetPaths` (LOT-39), avec `invalidate`/`invalidateAll` dès l'origine
    (prérequis du rechargement à chaud, LOT-43) et mémorisation des échecs pour ne pas relire le
    disque à chaque image.
  - `hmi::validateAsset` — **contrat de dimensions par famille d'asset** (atlas, skin de tuile,
    planche à raccords, fond, objet, spritesheet, décor) : validation **pure**, intercalée entre le
    décodage et l'upload, qui refuse un asset non conforme en journalisant le fichier, la dimension
    trouvée et l'attendue (`EX-REN-007`).
  - `hmi::buildMissingTextureImage` — repli **damier magenta** opaque et déterministe, résolu par le
    point d'appel unique `hmi::resolveOrPlaceholder` : un asset manquant se voit, sans jamais
    interrompre le rendu (`EX-NFR-040`).
  - **Culling par cadrage caméra** (`EX-NFR-005`) : seules les primitives intersectant le cadrage de
    la caméra (`hmi::Camera2D::visibleBounds`), élargi d'une marge d'une case, sont soumises ; le
    volume composé, écarté et soumis est observable en journalisation de diagnostic. Le culling est
    purement visuel — une entité écartée reste simulée (`EX-ARCH-012`).

- **LOT-39 — Textures depuis fichiers + repli procédural** (`EX-REN-041`, `EX-REN-042`) : l'atlas de
  tuiles (`hmi::TextureAtlas`) charge désormais `Assets/atlas.png` (à côté de l'exécutable, copié
  comme `Levels`/`Localization`) via un nouveau loader d'image générique (`hmi::TextureLoader`,
  décodage `QImage` → upload Direct3D 11) et une résolution de chemins pure et testable
  (`hmi::AssetPaths`) ; en l'absence d'asset, repli automatique sur la génération procédurale
  historique (`hmi::buildProceduralAtlasImage`, extraite telle quelle, sans régression), sans jamais
  bloquer le rendu (`EX-NFR-040`). Interface publique de `TextureAtlas` inchangée ; `tile`/
  `playerFrameRegion` rendues `static` (pure arithmétique de grille) et testées sans GPU. Un outil de
  développement (`JustAnotherDnDGame.exe --export-atlas=<chemin>`) régénère l'atlas de base depuis la
  génération procédurale de référence.

- **Feuille de route LOT-40 → LOT-55 — Programme d'habillage : textures, animations, décors** :
  cadrage documenté (`Documentation/Lot/LOT-40-*` à `LOT-55-*`) pour la suite du travail sur les
  textures, amorcé après LOT-39 puis **entièrement revu** à la suite d'un audit critique du premier
  cadrage. Aucune implémentation de code à ce stade — uniquement le cadrage
  (`epic.md`/`tache-*.md`) et les exigences correspondantes.

  Séquence : fondations du rendu — registre de textures, calques nommés, culling, testabilité
  (LOT-40) ; bascule Physique/Texture (`F8`, LOT-41) ; skin des tuiles avec jeux de skins et
  raccords automatiques (LOT-42) ; bibliothèque d'assets, import et rechargement à chaud (LOT-43) ;
  fond de niveau et versionnement du format (LOT-44) ; texture par objet interactif (LOT-45) ;
  moteur d'animation piloté par données (LOT-46) ; états visuels des mécanismes (LOT-47) ;
  personnage habillé depuis un fichier (LOT-48) ; décors libres et parallaxe (LOT-49) ; édition des
  décors (LOT-50) ; visibilité par calque (LOT-51) ; texte et affichage tête haute (LOT-52) ; effets
  et particules (LOT-53) ; atelier pixel art intégré (LOT-54) ; ombres du plan physique (LOT-55).

  Écarts par rapport au premier cadrage, issus de l'audit : ajout des **animations** (aucun lot n'en
  prévoyait, alors que portes, plaques et pièges n'ont aujourd'hui qu'une modulation d'opacité pour
  signaler leur état) ; ajout du **personnage**, absent du programme ; ajout des **décors libres et
  du calque de premier plan**, spécifiés de longue date (`decors.md`) mais rattachés à aucun lot ;
  ajout du **rendu de texte** (`EX-REN-032`, jamais implémentée — les budgets de sauts/dashs de
  LOT-12 n'étaient visibles nulle part) ; **raccords automatiques** et **jeux de skins** décidés
  avant la fabrication de `skins.json` plutôt qu'après ; **versionnement** du format de niveau au
  premier changement de schéma ; **culling** et **testabilité du rendu sans GPU** intégrés aux
  fondations ; **rechargement à chaud** avancé avant les couches visuelles ; défaut de rendu unifié
  et persisté au lieu de dépendre de la configuration de build.

  Nouveaux ids : `EX-REN-005` à `009`, `EX-EDIT-024` à `027`, `EX-LVL-005`, `EX-NFR-004`/`005`,
  `EX-DEC-006`, `EX-IHM-003` (en plus de `EX-REN-043` à `046` et `EX-EDIT-042` à `045` du premier
  cadrage). Exigences amendées : `EX-REN-012`, `EX-REN-014`, `EX-REN-032`, `EX-REN-046`,
  `EX-EDIT-042`, `EX-EDIT-044`. Les lots `LOT-40` à `LOT-48`, tous non commencés, ont été
  **renumérotés une seule fois** pour que le numéro suive l'ordre d'implémentation ; l'exception est
  actée dans `Documentation/Lot/lots.md`.

- **LOT-37 — Liens de mécanismes par traits/flèches** (`EX-IHM-030`, `EX-IHM-031`) : la liaison
  déclencheur → cible (interrupteur/plaque → porte, danger commuté) était jusqu'ici signalée par une
  simple teinte de case, illisible au-delà de quelques liens. Chaque liaison est désormais dessinée
  comme une **flèche explicite** dans le viewport (`hmi::DraftRenderer::drawLinks`, nouvelle
  primitive de segment orienté `hmi::LineQuad` sur `hmi::SpriteBatch`), alignée au zoom/pan courant.
  Un **outil « Lien » dédié** (`hmi::EditorTool::Link`, panneau Outils) remplace le geste au clic :
  cliquer un déclencheur passe en attente de cible, cliquer une cible crée la liaison (rejouer la
  paire la supprime) ; `Échap` annule une attente en cours. Plusieurs liens partageant un
  déclencheur s'écartent en **éventail** depuis une base commune plutôt que de se superposer. Un
  **panneau « Liens »** (dock Qt) liste toutes les liaisons du niveau, met en surbrillance celle
  sélectionnée et permet de la supprimer. Géométrie et machine à état du geste
  (`hmi::LinkGeometry`, `hmi::LinkGesture`) sont pures et testées sans GPU/Qt (`EX-NFR-010`) ; le
  modèle de liaison (`core::LevelDraft::linkMechanism`/`unlinkMechanism`) est inchangé.

- **LOT-34 → LOT-38 — Refonte de l'IHM vers Qt** (`EX-IHM-*`, `EX-BUILD-010`) : toute l'interface
  hors-jeu (menu, options, remappage clavier/manette, éditeur de niveaux) est désormais une
  application **Qt 6** (`JustAnotherDnDGame`), le **rendu in-game restant Direct3D 11** embarqué dans un
  viewport (`hmi::GameViewport`, `QWindow` + `HWND`). Fenêtres dockables (`QDockWidget` : Palette,
  Outils, Niveaux) à disposition persistée (`QSettings`, `EX-IHM-011`), palette en arbre
  (`QTreeView`), navigateur de niveaux (recherche, créer/renommer/dupliquer/supprimer). Mises en
  page éditables hors code (`.ui`/`.qrc`/`.qss` dans `Source/Elements/UI` et `Themes`). Déploiement
  autonome via `windeployqt` (aucune bibliothèque à installer côté utilisateur).
  **Internationalisation** (`EX-REN-033`) : toute l'IHM Qt (menu, options, éditeur, dialogues,
  palette, remappage) est traduite via `hmi::Localization` (catalogues `fr`/`en`, clés en anglais) ;
  l'onglet **Général** des Options offre le **choix de langue** (retraduction à chaud, persistée dans
  `QSettings`) et un bouton **« Enregistrer les journaux »** (`hmi::saveSessionLog`, build dev).

### Modifié
- **Préparation de la release — dette de documentation et de vérification** : audit complet mené
  avant de poser le tag, et correction de ce qu'il a mis au jour.
  - **Une seule source de vérité pour le numéro de version.** `core::Engine::version()` renvoyait
    `"0.1.0"` en dur et `project(… VERSION …)` valait `0.1.0` — aucun des deux n'avait jamais
    correspondu à un tag publié, seul le `Doxyfile` étant bumpé à chaque jalon. Le `project()`
    racine devient l'unique endroit où le numéro est écrit : il alimente `core::Engine::version()`
    par définition de compilation (`JADG_VERSION`), et `scripts/build_docs.py` — déjà
    exécuté par la CI — **échoue** désormais si le `PROJECT_NUMBER` du `Doxyfile` s'en écarte.
  - **Le garde-fou du Cahier de test en était un à moitié.** `generate_cahier_test.py` ne
    reconnaissait que la forme `TEST(` : les cas de test attachés à une *fixture* (`TEST_F`)
    disparaissaient du cahier **sans le moindre message**, dont l'intégralité de
    `test_image_encode.cpp`. Et `--check` ne comparait le fichier qu'au résultat du script, jamais
    au code : un test jamais documenté n'apparaissait d'aucun côté de la comparaison, donc passait.
    Les trois formes sont maintenant reconnues, et un **contrôle de complétude** échoue en listant
    tout test dépourvu de bloc `\castest{}`.
  - **145 tests sur 943 (15 %) n'étaient dans aucun cahier** — des fichiers entiers issus des
    `LOT-46` à `LOT-50`. Tous documentés : le Cahier de test passe de **790 à 943 cas**. Au passage,
    deux blocs de documentation détachés de leur test dans `test_physique_personnage.cpp` (trois
    blocs empilés devant un seul `TEST`) ont été remis en face du test qu'ils décrivent.
  - **Guide du développeur** : `LOT-54`, `LOT-56` et `LOT-57` n'y avaient aucune couverture. Deux
    pages ajoutées — `guide-atelier-pixel-art.md` et `guide-design-ihm.md` — et `guide-ihm-qt.md`,
    qui s'arrêtait au `LOT-36`, recadré et relié aux deux nouvelles ; références à `hmi::ToolPanel`
    (supprimé au `LOT-57`) corrigées dans `guide-editeur.md`.
  - **README et manuel** : la liste des fonctionnalités décrivait l'état d'avant le programme
    d'habillage ; `HMI/` y était encore « fenêtre Win32 » ; le tableau d'intégration continue
    annonçait un runtime « statique » (faux depuis le `LOT-38` — les DLL Qt imposent `/MD`) et
    ignorait le déclencheur `vX.Y.Z`. Le manuel ne connaissait que la préversion roulante et
    documente désormais les **versions publiées**.
  - **Notes de release lisibles** : `release.yml` publiait avec `--generate-notes`, qui produit une
    liste brute de messages de commit — utile à un développeur qui connaît déjà le projet, opaque
    pour le non-développeur à qui la release versionnée est justement destinée. Le workflow lit
    désormais la section correspondante du CHANGELOG (`scripts/extract_release_notes.py`,
    `--notes-file`), et **échoue avant de publier** si elle est absente : une release ouverte avec
    des notes vides ne se corrige pas proprement. La marche à suivre pour publier une version est
    consignée dans `CONTRIBUTING.md`.
- **LOT-38 (Étape B) — Retrait du legacy & réorganisation** : suppression de l'IHM « maison »
  (écrans `IScreen`/`ScreenManager`, widgets d'éditeur, police bitmap, fenêtre Win32) et de
  l'exécutable historique ; `Source/HMI` devient l'unique cible (`JustAnotherDnDGame`), code réparti par
  domaine (`Platform`/`Input`/`Graphics`/`Game`/`Localization`/`Interface`/`Editor`). Documentation
  (guides écrans/éditeur/rendu/entrées) et journalisation mises à jour en conséquence.

## [0.0.4] - 2026-07-27

> Quatrième jalon : **fluidité du moteur** au-dessus de 60 Hz (LOT-33) — les entrées sont désormais
> consommées par **pas de simulation** plutôt que par frame de rendu (plus de perte à haut
> framerate), la présentation passe en **flip-model** (latence entrée → image réduite, cadence plus
> régulière) et le rendu **interpole** la position des entités mobiles entre deux pas fixes, sans
> jamais toucher au déterminisme de la simulation. **541 tests** (540 au jalon précédent).
>
> Voir le [CHANGELOG](CHANGELOG.md) pour le détail (LOT-33).

### Modifié
- **LOT-33 — Fluidité du moteur** (`EX-REN-004`, `EX-ARCH-031`, `EX-CTRL-020`, `EX-CTRL-021`) :
  ensemble de corrections de choix techniques boucle/rendu/entrées qui dégradaient le ressenti
  au-dessus de 60 Hz. **Entrées nerveuses** : les fronts (pressée/relâchée) sont désormais consommés
  par **pas de simulation** et non par frame de rendu — un appui capturé sur une frame réelle sans
  pas (rendu > 60 Hz, ≈ 2 sur 3 à 144 Hz) n'est plus perdu (`hmi::Window::beginInputFrame`, nouvelle,
  appelée après chaque pas ; `pumpMessages` n'avance plus les fronts). Les touches ne restent plus
  « collées » à un `Alt+Tab` (`InputState::releaseAll` sur `WM_KILLFOCUS`). Le sondage `XInputGetState`
  d'un slot **sans manette** — coûteux — est throttlé (une frame sur ~120), supprimant les
  micro-saccades chez un joueur clavier. **Présentation flip-model** (`DXGI_SWAP_EFFECT_FLIP_DISCARD`,
  deux back buffers) à la place du modèle *blt* legacy : latence entrée → image réduite et cadence
  plus régulière (la cible de rendu est reliée au back buffer à chaque `clear()`, le flip model la
  dé-liant à `Present`). **Interpolation de rendu** (`EX-ARCH-031`, prévue dès `LOT-01` mais jamais
  exploitée) : nouveau composant de présentation `hmi::PreviousPosition` (dans le `core::World`,
  écrit/lu par `HMI` seul, `Core` intact), rempli au début de chaque pas ; `hmi::SpriteRenderer`
  dessine les entités mobiles (personnage, dangers mobiles, blocs) à `lerp(précédente, courante,
  alpha)` via `core::FixedTimestep::interpolationAlpha`, supprimant le *judder* à haut framerate. La
  simulation reste strictement déterministe (l'interpolation ne touche que l'affichage) ; la caméra
  n'est pas interpolée (coupure nette par salle, `LOT-32`).

## [0.0.3] - 2026-07-25

> Troisième jalon : le prototype devient un **vrai platformer**. Personnage animé (LOT-17/18) et
> **physique newtonienne** (LOT-19) ; **manette** pleinement supportée et **remappage** complet,
> clavier **et** manette (LOT-20, LOT-29, LOT-30) ; bibliothèque de tuiles de plateforme — pentes,
> arrondis (convexes puis concaves), blocs poussables et blocs à taille fractionnaire, sol **et**
> plafond (LOT-21 à LOT-26, LOT-28) ; **dangers avancés** — directionnels, mobiles, commutés,
> temporisés (LOT-31) ; niveaux à **salles** façon *Celeste* (LOT-32) ; palette de l'éditeur
> réorganisée par catégories (LOT-27) et refactoring complet des niveaux de démonstration (LOT-25).
> **540 tests** (292 au jalon précédent).
>
> Voir le [CHANGELOG](CHANGELOG.md) pour le détail par lot (LOT-17 → LOT-32).

### Ajouté
- **LOT-32 — Niveaux à salles** (`EX-REN-015`, `EX-EDIT-023`) : un niveau plus grand qu'une
  **salle** (nouveau `hmi::RoomGrid`, taille fixe en tuiles, `Source/HMI/Graphics`) se joue avec
  une caméra qui cadre la salle **courante** du personnage, au zoom pixel art natif, et bascule
  **nettement** sur la salle voisine dès qu'il en franchit la frontière — façon *Celeste*, sans
  jamais rapetisser le rendu quelle que soit la taille totale du niveau. Le niveau reste une grille
  de tuiles **unique** (aucun format, aucune nouvelle tuile) : une salle a « plusieurs entrées/
  sorties » simplement parce qu'un couloir reste ouvert sur plusieurs de ses bords vers des salles
  voisines. Un niveau qui tient dans une seule salle se comporte à l'identique du cadrage « niveau
  entier » de LOT-16 (non-régression). Dans l'éditeur, un quadrillage de salles (même bascule
  `F10` que le repère fin existant) aide à aligner les couloirs inter-salles, sans changer son
  cadrage (toujours « niveau entier », pan/zoom manuel). `EX-REN-013` reformulée en conséquence
  (scopée aux niveaux tenant dans une seule salle). Nouveau niveau de démonstration
  `demo-salles.json` (2×2 salles) intégré à la séquence jouée.
- **LOT-31 — Blocs de danger avancés** (`EX-GP-050` à `EX-GP-053`) : quatre nouvelles variantes de
  la tuile `Danger` — directionnel (`DangerUp`/`Down`/`Left`/`Right`, mortel uniquement sur une
  bande étroite du bord désigné plutôt que la case entière), mobile (`DangerMover`, aller-retour
  linéaire déterministe autour de sa position de départ), commuté (`DangerSwitched`, mortel
  uniquement quand l'interrupteur/la plaque de pression qui lui est lié est actif — inverse d'une
  porte) et temporisé (`DangerBlink`, alterne mortel/inoffensif selon une période fixe et un
  déphasage par tuile). Nouveau `core::DangerController` (mobile/temporisé) ; `core::
  MechanismController` étendu (commuté, même détection front/continu qu'une porte) ;
  `core::evaluateOutcome` gagne un paramètre `extraDangerBoxes` pour les dangers à état, assemblés
  par `hmi::GameScreen`. Palette de l'éditeur : catégorie « Piège » restructurée (Classique +
  sous-groupe Directionnel + Mobile/Commuté/Clignotant) ; liaison éditeur au même geste qu'une
  porte. Nouveau niveau de démonstration `demo-dangers-avances.json` intégré à la séquence jouée.
- **LOT-30 — Remappage manette** (`EX-CTRL-002`, `EX-CTRL-012`) : troisième sous-menu dans Options
  (« Touches de la manette »), même patron que LOT-29, listant les six actions de jeu avec le
  bouton XInput actuellement lié (`Up`/`Down`/`Left`/`Right`, `A`/`B`/`X`/`Y`, épaules gauche/
  droite) ; capture, échange sur conflit et réinitialisation identiques au remappage clavier.
  Nécessite une manette connectée pour entrer en capture, sinon affiche une invite dédiée.
  `InputState` gagne une piste brute par `GamepadButton`, indépendante de la fusion clavier/manette
  existante sur `Key` (celle-ci reste réservée à la navigation de menu, jamais remappable) ;
  `PlayerInputMapper` vérifie désormais, par action, la touche clavier **ou** le bouton manette
  liés — chaque source remappable indépendamment de l'autre. Le filet de sécurité de LOT-29
  (retomber sur la touche par défaut du clavier pour ne pas casser la manette) devient inutile et
  est retiré : la manette a maintenant sa propre couche de configuration. Persistance dans la même
  section `Settings/keybindings.json` (nouvelle section `"manette"`).
- **LOT-29 — Remappage des touches** (`EX-CTRL-012`) : deux nouveaux sous-menus dans Options
  (« Touches de jeu », « Touches de l'éditeur »), chacun listant ses actions avec la touche
  actuellement liée ; sélectionner puis confirmer une action capture la touche suivante pressée
  (échange automatique en cas de conflit, jamais de doublon), une entrée « Réinitialiser » restaure
  les valeurs par défaut. Six actions de jeu (Gauche/Droite/Sauter/Dash/Viser haut/Viser bas) et un
  sous-ensemble significatif de neuf actions d'éditeur (Sauvegarder/Annuler/Refaire/Copier/Coller/
  Test rapide/Grille/Aide/Renommer) — pas un remappage exhaustif (navigation de menu, `Ctrl+R`,
  redimensionnement par flèches, `"0"`, `Tab`, Maj+clic restent câblés en dur). **Persistance**
  (première du projet) dans `Settings/keybindings.json`, à côté de l'exécutable. Le panneau d'aide
  de l'éditeur (`F1`) reflète désormais les touches réellement liées. Un filet de sécurité
  (`GameKeyBindings::defaultKey` revérifiée en plus du binding courant) garantissait que la manette
  continue de fonctionner après un remap clavier (`EX-CTRL-002`) tant qu'un vrai remappage manette
  restait hors périmètre — devenu inutile et retiré en `LOT-30`, qui apporte ce remappage manette.
- **LOT-28 — Arrondis concaves** (`EX-GP-007`) : quatre nouvelles tuiles, `ConcaveUpRight`/
  `ConcaveUpLeft` (sol) et `ConcaveDownRight`/`ConcaveDownLeft` (plafond) — une seconde famille de
  quart de cercle, **concave** plutôt que **convexe** (`RoundedUpRight`/`RoundedUpLeft` et leurs
  variantes de plafond, `EX-GP-004`/`EX-GP-006`) : centre du cercle du côté **plein** plutôt que du
  côté creux, courbure inversée (tangente horizontale côté creux, verticale côté plein), utile pour
  un raccord en creux entre deux surfaces perpendiculaires. Réutilise l'infrastructure de suivi de
  surface/silhouette posée par `LOT-22`/`LOT-23`/`LOT-26` — la formule de hauteur
  (`core::slopeSurfaceHeight`, deux nouveaux `case` ; `core::ceilingSlopeHeight`, mapping miroir
  étendu) change comme prévu. Sol **et** plafond dans un seul lot (le plafond ne coûtant qu'un
  mapping de deux lignes, infrastructure déjà générique depuis `LOT-26`). Nouveau sous-groupe de
  palette **Concave**, frère de **Arrondi** sous la catégorie **Tuile** (`LOT-27`) ; rendu avec la
  silhouette réelle en gris, logé dans quatre des cases déjà réservées de l'atlas procédural
  (`TextureAtlas::TILES_PER_SIDE` inchangé à `5`).
- **Corrigé en cours de lot (LOT-28)** : `ConcaveDownLeft` visait initialement la case d'atlas
  `(4,4)`, en réalité réservée au damier de transparence (affichait le damier au lieu de sa
  silhouette) — réassignée à `(2,4)`, authentiquement libre.
- **Corrigé en cours de lot (LOT-28), défaut antérieur à ce lot** : `core::resolveSlopeFollow`/
  `core::resolveCeilingSlopeFollow` sélectionnaient la case à consulter uniquement par le **centre**
  de la boîte du personnage — pour la dernière fraction (environ la moitié de la largeur de la
  boîte) de la largeur de toute case pente/arrondi/concave, ce centre atterrit déjà dans la case
  **voisine** ; si celle-ci est non solide (typiquement deux arrondis/concaves posés côte à côte —
  un arc, une voûte), aucun filet ne rattrape le personnage, qui tombe ou saute au travers sans être
  bloqué. Reproduit sur `SlopeDownRight` (`LOT-26`) avec le même symptôme exact : le défaut était
  **latent depuis `LOT-22`**, jamais exposé faute d'avoir déjà chaîné deux tuiles non solides
  adjacentes (une case solide voisine masquait silencieusement le même défaut via la collision
  classique sur grille). Corrigé en élargissant la sélection aux colonnes réellement couvertes par
  la largeur de la boîte, la colonne centrale gardant exactement son calcul d'origine (aucune
  régression pour tout appelant dont la boîte ne dépasse pas d'une seule colonne pertinente).
- **Corrigé en cours de lot (LOT-28), second défaut plus profond** : le correctif ci-dessus ne
  suffisait pas quand le personnage **marche pendant qu'il saute** — la case qui aurait dû bloquer
  pouvait redevenir invisible sur PLUSIEURS pas consécutifs (pas seulement le précédent) avant que
  le seuil vertical de blocage n'y soit atteint, laissant un saut traverser un plafond incliné/
  courbe malgré le premier correctif. Corrigé en faisant mémoriser à `core::CharacterPhysicsSystem`
  l'étendue horizontale couverte par la boîte depuis le **début de la montée courante**
  (`Player::ascentSweepMinX`/`ascentSweepMaxX`), pas seulement le pas précédent. Vérifié généralisé
  aux pentes linéaires de plafond (`LOT-26`), pas spécifique aux arrondis concaves de ce lot. Zéro
  régression sur la suite complète après les deux correctifs.
- **Corrigé en cours de lot (LOT-28), troisième défaut, distinct des deux précédents** : un saut
  bloqué tout près du bord **fin** (silhouette quasi vide) d'un arrondi concave de plafond se
  retrouvait, un pas après le blocage, téléporté au-dessus du plafond — `core::resolveSlopeFollow`
  interprétait à tort le chevauchement résiduel (bord bas du personnage encore dans la case de
  plafond après un blocage par en dessous) comme un atterrissage sur la face du haut de la tuile.
  Corrigé en exigeant que le bord bas ait déjà été au-dessus de la case avant le pas pour qu'un tel
  calage soit accepté (garde-fou restreint aux tuiles de plafond, sans effet sur le calage normal
  d'un atterrissage sur sol plat). Nouvelle infrastructure de journalisation réutilisable
  (`Source/Core/Physics/PhysicsLog.h`, macros `PHYSICS_LOG_*`, réservées aux événements rares) pour
  faciliter le diagnostic d'anomalies similaires à l'avenir. Zéro régression sur la suite complète
  après les trois correctifs.
- **Corrigé en cours de lot (LOT-28)** : `slopeShapePixel` (`TextureAtlas.cpp`) échantillonnait au
  **centre** de chaque pixel, qui n'atteint jamais exactement les bords `0`/`1` de la case (dernier
  pixel plafonné à `≈0,969`) — sans conséquence pour les pentes/arrondis existants, mais visible
  comme une encoche près du bord **plein** (tangente raide) d'un arrondi concave, à l'endroit
  précis où la silhouette doit au contraire être la plus pleine. Corrigé en échantillonnant au bord
  des pixels de coin plutôt qu'à leur centre. Vérifié en jeu : deux arrondis concaves adjacents
  (arche/pic) se traversent désormais en marchant sans chute, silhouette rendue comme un pic net.
- **LOT-27 — Palette de l'éditeur organisée par catégories** (`EX-EDIT-018`) : la palette de
  tuiles, jusqu'ici une liste plate de 19 types, devient un **accordéon à trois niveaux**. Premier
  niveau : deux entrées autonomes toujours visibles (Vide, Piège — ex-Danger, renommé à l'affichage
  seulement) et trois catégories repliables (Tuile, Interactif, Jalon). Deuxième niveau : une
  catégorie dépliée expose ses tuiles directes (ex. Porte, Plaque, Interrupteur) et, pour Tuile/
  Interactif, des **sous-groupes** repliables (Pente, Arrondi, Bloc poussable — familles à
  plusieurs formes/tailles). Troisième niveau : un sous-groupe déplié expose ses variantes
  (orientations, tailles). La hauteur du panneau étant désormais variable, `TilePalette::bottom()`
  remplace le compte fixe (`EditorLayout::PALETTE_TYPE_COUNT`/`TOOLBAR_TOP`, supprimés) ;
  `ToolBar::relayout(top)` repositionne la barre d'outils juste sous la palette à chaque frame,
  quel que soit son état de dépliage. Un mockup HTML/CSS interactif a été itéré avec le demandeur
  (position du dépliage, choix des quatre catégories, troisième niveau d'accordéon) **avant**
  l'implémentation. **Défilement** ajouté après une première revue (« n'est pas complet ») : tout
  déplier en même temps (25 lignes au maximum) pouvait dépasser la hauteur de fenêtre disponible,
  rendant les dernières entrées définitivement inaccessibles à la souris — `TilePalette` expose
  désormais une **fenêtre visible** (`scroll`/`setViewportHeight`), la molette au-dessus du panneau
  latéral la fait défiler (plutôt que de zoomer la caméra), et une barre de défilement (piste +
  curseur, même principe que `LevelPicker`, `LOT-15`) apparaît si le contenu déplié déborde.
  Replier/déplier un en-tête ne le fait **jamais** disparaître de la fenêtre
  (`TilePalette::followRow`, suit automatiquement l'en-tête tout juste basculé — sans quoi le
  défilement aurait simplement déplacé le problème plutôt que de le résoudre). **11 nouveaux
  tests** (10 `TilePalette`, 1 `ToolBar`), aucune régression (406/406 tests verts).

### Corrigé
- **Blocs poussables traversant les pentes/arrondis** (`EX-GP-003`/`EX-GP-004`/`EX-GP-006`/
  `EX-GP-022`) : `BlockController` ne connaît que la solidité statique (`core::isSolid`) pour
  décider si une case est libre — or une pente/arrondi (sol ou plafond) n'est **jamais** solide
  (sa collision passe par un suivi de surface propre au personnage, que `BlockController` ignore
  entièrement). Un bloc suspendu au-dessus d'une pente la traversait donc en tombant, et pouvait
  être poussé dedans. `isFree` traite désormais une case de pente/arrondi comme un **obstacle
  simple** (comme une case solide), cohérent avec le modèle case-par-case des blocs. **2 nouveaux
  tests**, aucune régression (395/395 tests verts).
- **Sélecteur de niveau de l'éditeur illisible au-delà d'une poignée de fichiers** (`EX-EDIT-001`) :
  la liste (« Nouveau niveau » + fichiers `.json` du dossier) s'affichait entièrement, sans
  défilement — au-delà de la hauteur de fenêtre (13 niveaux démo depuis `LOT-25`), les entrées
  en trop restaient invisibles et donc inaccessibles à la souris. `LevelPicker` ne dessine
  désormais que la fenêtre visible (le défilement suit la sélection au clavier, ou la molette
  sans changer la sélection), avec une barre de défilement (piste + curseur) indiquant la
  position dans la liste. 5 nouveaux tests.
- **Pentes/arrondis en jeu dans des couleurs disparates** (`EX-GP-003`/`EX-GP-004`) : la forme
  suit désormais la hitbox réelle (triangle/courbe, correction précédente) mais chaque variante
  gardait sa propre teinte (vert sarcelle, vieux rose, bleu violet, magenta) — incohérent avec un
  simple matériau de plateforme. Rempli en gris (même couleur que `Solid`), comme les blocs
  réduits (`BlockHalf`/`BlockQuarter`, déjà gris) ; la forme (triangle/courbe) reste inchangée.
- **Blocs réduits affichés à taille pleine dans l'éditeur** (`EX-GP-005`) : le canevas de l'éditeur
  (grille du niveau en cours d'édition) dessinait `BlockHalf`/`BlockQuarter` en carré plein
  (1×1 case), sans rapport avec leur boîte de collision réelle (`×0.5`/`×0.25`, centrée) — l'éditeur
  devait rester un aperçu **fidèle** de ce que le jeu affiche, pas une approximation. Réutilise
  désormais la même formule que `core::BlockController::boxAt` (nouvelle fonction partagée
  `core::tileVisualScale`, `Core/Levels/TileType.h`) pour centrer et mettre à l'échelle le sprite,
  côté éditeur **et** côté jeu (`GameScreen::refreshBlockVisuals`, inchangé) — aucune divergence
  possible entre les deux par construction. Les pentes/arrondis, déjà fidèles à leur forme réelle
  (correction précédente), sont inchangés.
- **Pentes et arrondis affichés comme des carrés pleins** (`EX-GP-003`/`EX-GP-004`) : l'atlas
  procédural peignait ces quatre tuiles d'une simple couleur plate, sans rapport avec leur
  hitbox réelle (surface inclinée ou courbe, `core::slopeSurfaceHeight`) — visuellement un mur
  plein là où le personnage pouvait en fait marcher en diagonale au-dessus du vide. `TextureAtlas`
  peint désormais ces quatre cases avec un masque de forme (opaque sous la surface suivie par la
  physique, transparent au-dessus), calculé avec la **même** fonction que la physique : l'affichage
  correspond par construction à la hitbox, sans pouvoir diverger. `hmi::slopeTileGridPosition`
  (`HMI/Graphics/TileVisuals.h`) devient la source de vérité unique des coordonnées d'atlas de ces
  quatre tuiles, partagée entre le rendu (couleur) et la génération du masque.
- **Cahier de test illisible (page unique, 321 cas à plat)** : la page agrégeait tous les cas de
  test sur un seul niveau (mécanisme Doxygen `\xrefitem`, sans aucune section). Remplacée par
  `Documentation/CahierTest.md`, généré par le nouveau `scripts/generate_cahier_test.py` à partir
  des mêmes blocs `\castest{...}` (source de vérité inchangée), structuré selon l'arborescence de
  `Source/Test/` (Tests unitaires par module Core/HMI, Tests d'intégration et Système par fichier)
  — navigable via l'arbre latéral comme toute autre page. Vérifié en CI (`--check`).
- **Accumulateur à pas de temps fixe non utilisé dans `main`** (`EX-NFR-002`) : depuis
  l'intégration du menu principal (LOT-06), la boucle appelait `ScreenManager::update` une seule
  fois par frame réelle avec un delta constant, sans jamais mesurer le temps réel écoulé ni
  appeler `core::FixedTimestep::advance` — la simulation ne restait déterministe que par
  coïncidence, tant que le V-Sync cadençait l'affichage à 60 Hz. La mesure du temps réel
  (`std::chrono::steady_clock`) et la boucle d'accumulateur (rattrapage de plusieurs pas sur une
  frame lente, plafonné par `maximumStepsPerCall`) sont restaurées, conformément à
  `Documentation/Guide/guide-boucle.md`.
- **Sélection de niveau à la souris** (`hmi::LevelPicker`) : le sélecteur de niveau de l'éditeur
  (« Choisir un niveau ») ne répondait qu'au clavier (`↑`/`↓`/`Entrée`) — le survol et le clic
  gauche sélectionnent et confirment désormais un choix, comme le menu principal
  (`hmi::MenuModel`, même mise en page à chasse fixe).
- **Séparateur `*` accepté pour la taille de grille** (`hmi::parseLevelSize`, `Ctrl+R`) : en plus
  de `x`/`X`, un niveau peut être redimensionné en tapant `largeur*hauteur` (ex. `60*40`).

### Ajouté
- **LOT-26 — Pentes/arrondis de plafond** (`EX-GP-006`) : quatre nouvelles tuiles,
  `SlopeDownRight`/`SlopeDownLeft`/`RoundedDownRight`/`RoundedDownLeft` — miroir vertical exact des
  pentes/arrondis de sol existants (`EX-GP-003`/`EX-GP-004`, matière pleine en haut de la case
  plutôt qu'en bas). Comme leurs équivalents de sol, elles ne sont **jamais solides**
  (`core::isSolid`) pour la grille classique : leur collision est résolue par une passe de suivi
  dédiée, miroir de `resolveSlopeFollow` (`core::resolveCeilingSlopeFollow`, déclenchée en
  **montant** plutôt qu'en tombant) — le personnage ne peut **jamais franchir** leur silhouette en
  sautant (bonk précis contre le profil incliné/courbe réel, pas une case pleine uniforme), mais ne
  « marche » jamais dessus non plus (pas de déplacement latéral calé sous un plafond). Leur **face
  du haut**, toujours plate au sommet de la case, supporte normalement un personnage qui tombe
  dessus **par au-dessus** (`core::slopeSurfaceHeight` étendu, hauteur constante `0`, réutilise
  `resolveSlopeFollow` tel quel — sans quoi il serait tombé au travers). Réutilise
  `core::slopeSurfaceHeight`/`core::resolveSlopeFollow` de la variante de sol miroir (aucune formule
  dupliquée, comparaison inversée). Grille de tuiles de l'atlas procédural agrandie
  (`TextureAtlas::TILES_PER_SIDE` `4→5`) pour loger les quatre nouvelles silhouettes, sans décaler
  les couleurs des tuiles existantes (jeu de couleurs recalé explicitement). Placeables depuis la
  palette de l'éditeur (19 types désormais). **12 nouveaux tests** (géométrie miroir, classification
  `isSolid`/`isFollowableSurface`, aller-retour JSON, physique de blocage par en dessous et de
  support par au-dessus), aucune régression (393/393 tests verts).
- **LOT-25 — Refactoring complet des niveaux démo** (`EX-GP-003`/`EX-GP-004`/`EX-GP-005`/
  `EX-GP-015`/`EX-GP-016`/`EX-GP-017`/`EX-GP-020`/`EX-GP-022`/`EX-GP-024`/`EX-GP-025`) : les
  anciens niveaux (`demo.json`…`demo5.json`, accumulés au fil des lots sans repasse d'ensemble —
  `demo5.json`, en particulier, était chargé en jeu mais absent du test système) sont supprimés et
  remplacés par **13 niveaux**, un par mécanique (ou petit groupe cohérent), plus un niveau final
  qui les combine (dash, pente, bloc poussable, interrupteur/porte, double saut). `Source/HMI/
  main.cpp` et `Source/Test/Systeme/test_parcours_complet.cpp` chargent désormais exactement la
  même liste, dans le même ordre — un nouveau script CI, `scripts/check_demo_sequence.py`, échoue
  si les deux divergent de nouveau. La plaque de pression (`EX-GP-025`) a nécessité une conception
  particulière : une géométrie plaque/porte/sortie empilées verticalement s'est révélée
  infranchissable par construction (la porte se referme dès que la boîte du personnage — plus
  petite qu'une case — quitte la plaque, avant d'avoir fini de la traverser) ; le niveau livré
  ouvre à la place une porte au-dessus d'un mur d'une case, franchie par un saut pendant la fenêtre
  où elle est ouverte. **2 nouveaux tests** (franchissement et nécessité du saut), aucune
  régression (376/376 tests verts).
- **LOT-24 — Blocs à taille fractionnaire** (`EX-GP-005`) : deux nouvelles tuiles, `BlockHalf`/
  `BlockQuarter` (`×0.5`/`×0.25` d'une case), gérées par `core::BlockController` au même titre que
  `Block` (poussée/chute identiques, toujours case par case) mais avec une boîte de collision
  **réelle** plus petite et **centrée** dans leur case. La grille classique (`sweepAabb`) ne peut
  pas représenter une occupation partielle de case : nouvelle routine `core::sweepAabbVsAabb`
  (`Core/Physics/AabbVsAabb.h`, balayage continu boîte-contre-boîte, même méthode de clamp direct
  que `SweptCollision.cpp`), composée par `hmi::GameScreen::update` **après** le balayage sur
  grille — la restriction la plus stricte des deux l'emporte toujours. Rendu à l'échelle et centré
  dans l'éditeur, avec exactement la même formule que la collision (aucune divergence possible
  entre le sprite affiché et la boîte testée). **14 nouveaux tests** (unitaires et intégration),
  aucune régression (364/364 tests verts).
- **LOT-23 — Collision arrondie** (`EX-GP-004`) : deux nouvelles tuiles, `RoundedUpRight`/
  `RoundedUpLeft`, variante en **quart de cercle** des pentes de LOT-22 (`h(x) = 1 - sqrt(1 - (1 -
  x)²)` et sa symétrique). Réutilise intégralement l'infrastructure de suivi de surface posée par
  LOT-22 (`core::resolveSlopeFollow`, la correction du balayage horizontal) : un nouveau `case`
  dans `core::slopeSurfaceHeight`/`core::isFollowableSurface` suffit, aucune autre passe n'a été
  modifiée. Palette de l'éditeur (`PALETTE_TYPE_COUNT` 11→13) et couleurs plates distinctes des
  pentes. **7 nouveaux tests** (unitaires et intégration), aucune régression (350/350 tests verts).
- **LOT-22 — Pentes réelles** (`EX-GP-003`) : deux nouvelles tuiles, `SlopeUpRight`/`SlopeUpLeft`
  (pente à 45°, montée pleine sur toute la largeur d'une case), disponibles dans l'éditeur. Jamais
  solides pour le balayage classique (`core::isSolid`) — une nouvelle passe de résolution
  (`core::resolveSlopeFollow`, `Core/Physics/SlopeGeometry.h/.cpp`) cale la position verticale du
  personnage sur la surface de la pente à chaque pas fixe, sauf en cas de saut volontaire (vitesse
  verticale négative). Corrige au passage un piège découvert par les tests d'intégration : le
  balayage horizontal (`sweepX`) traitait comme un mur tout bloc plein partageant une ligne que le
  personnage chevauche déjà en suivant une pente (raccord pente → palier, le cas le plus courant) —
  généralise le principe de la « peau » (`kSkin`) déjà en place pour le sol plat
  (`rowIsSlopeGround`, `Core/Physics/SweptCollision.cpp`). **12 nouveaux tests** (unitaires et
  d'intégration), aucune régression sur la physique existante (343 tests, tous verts).
- **LOT-21 — Bloc poussable** (`EX-GP-022`), nouvelle tuile disponible dans l'éditeur.
  `core::BlockController` (nouveau, `Core/Gameplay`) résout chaque pas fixe, avant la physique du
  personnage : **poussée** horizontale d'une case si la case suivante est libre (ni mur, ni autre
  bloc), et **chute** discrète (une case toutes les `FALL_INTERVAL_STEPS` pas) si le bloc n'est
  plus soutenu par le dessous. Un bloc occupe toujours exactement une case entière, comme les
  autres mécanismes de ce moteur — jamais de position intermédiaire. Un bloc posé sur une plaque
  de pression ne l'active pas encore (évolution à venir, voir `Documentation/Guide/guide-niveaux.md`).
  **10 nouveaux tests unitaires** (331 au total, 321 au jalon précédent).
- **Journalisation étendue pour le diagnostic** : trois nouvelles catégories de log
  (`Core/Gameplay/GameplayLog.h`, `Core/Levels/LevelsLog.h`, `HMI/Editor/EditorLog.h`), sur le
  modèle déjà établi (voir `Documentation/Guide/guide-journalisation.md`). Journalise désormais les
  bascules d'état des mécanismes (interrupteur/plaque de pression), le chargement d'un niveau et
  chaque raison d'échec de validation (`Core/Levels/LevelLoader`), les actions de l'éditeur
  (liaison/déliaison de mécanisme, annuler/refaire, redimensionnement), le nombre de niveaux trouvés
  par le sélecteur, et la connexion/déconnexion de la manette — toujours en dehors des chemins
  exécutés à chaque frame ou à chaque pas fixe (uniquement sur changement d'état réel).
- **LOT-20 — Manette et menu d'options** (`EX-CTRL-002`). Le jeu, le menu et l'éditeur sont
  désormais jouables/navigables à la **manette** (XInput) : D-pad/stick gauche pilotent les mêmes
  directions que les flèches/`Q`/`D`, **A** valide (menu) et saute (jeu), **B**/**Start**
  reviennent/échappent, l'épaule droite dashe. La manette est **fusionnée** dans `InputState` au
  niveau des mêmes touches déjà lues par tout le code existant (deux sources indépendantes
  combinées en lecture, jamais en écriture, pour ne jamais effacer une touche clavier réellement
  maintenue) : aucun consommateur (`MenuModel`, `PlayerInputMapper`, raccourcis de l'éditeur…) n'a
  été modifié. Nouveau **menu d'options** (4ᵉ entrée du menu principal) : bascule **V-Sync**
  (jusqu'ici fixée en dur), bouton de langue, état de connexion de la manette, retour au menu.
  **12 nouveaux tests** (321 au total, 309 au jalon précédent) ; vérifié visuellement dans
  l'application (menu, options, bascule V-Sync/langue).
- **LOT-19 — Physique newtonienne et plaque de pression** (`EX-GP-019`, `EX-GP-025`). Deux
  évolutions liées par le **poids** du personnage : la chute suit désormais un modèle
  **newtonien** — masse (`core::Player::mass`) et traînée proportionnelle à la vitesse, dont
  l'équilibre fait **émerger** une vitesse terminale progressive plutôt qu'un plafond arbitraire
  (`std::min`) ; calibrée pour retomber sur l'ancienne vitesse terminale (25 unités/s) à masse par
  défaut, seule la **courbe** change, pas le résultat final. La montée du saut reste inchangée
  (ressenti LOT-11 non affecté). Nouveau mécanisme de puzzle, la **plaque de pression**
  (`TileType::PressurePlate`) : ouvre la porte liée tant qu'un poids suffisant y repose,
  la referme dès qu'il en part — activation **continue**, à la différence de l'interrupteur à
  bascule (conservé tel quel). Réutilise l'infrastructure de liaison existante (identifiant/
  `opensWith`, éditeur Maj+clic généralisé aux deux types de déclencheur). Niveau de démonstration
  `demo5.json` ajouté à la séquence de jeu. **8 nouveaux tests** (305 au total, 297 au jalon
  précédent) ; vérifié visuellement dans l'application (courbe de chute, plaque de pression,
  éditeur).
- **LOT-18 — Animation du personnage** (`EX-REN-012`). Le personnage anime désormais trois
  **clips** dérivés de son état physique (`core::Player::grounded`, `core::Velocity`) : `Idle`
  (repos, 2 images, au sol et immobile), `Run` (course, 4 images, au sol en mouvement), `Jump`
  (saut, 1 pose fixe, en l'air) — un nouveau composant `core::Animation` et
  `core::AnimationSystem` (logique pure, `Core`, testée sans GPU) déterminent le clip et l'image
  courante chaque pas fixe, **après** `CharacterPhysicsSystem` (qui vient de calculer `grounded`
  pour ce même pas). Côté rendu, `TextureAtlas` expose une grille de 7 images (16×16, toujours
  **carrées** — non-régression du bug d'échelle de LOT-17) et `GameScreen` met à jour la région du
  sprite **à chaque frame** (plus seulement au spawn). Second des deux lots dédiés au personnage
  (`EX-REN-012`, après la silhouette statique de LOT-17). **5 nouveaux tests** d'intégration
  (297 au total, 292 au jalon précédent) ; vérifié visuellement dans l'application (repos, course,
  chute/saut).
- **LOT-17 — Sprite du personnage (statique)** (`EX-REN-011`). Le personnage n'est plus une tuile
  de couleur unie (`_atlas.tile(1, 1)`, cyan) : il affiche désormais une **silhouette humanoïde**
  (tête, cheveux, torse/manches, mains, jambes, chaussures) générée en code, comme le reste de
  l'atlas — aucun fichier image, aucune dépendance externe. La région reste **carrée** (16×16,
  comme une tuile) : c'est l'échelle du `Transform` (`core::playerSize()`, déjà non uniforme,
  0,4×0,8) qui donne au personnage sa proportion finale deux fois plus haute que large — une région
  déjà non carrée aurait doublé cet effet et fait déborder la silhouette de la boîte de collision.
  Elle vit dans la **même** texture que les tuiles (`TextureAtlas`, agrandie d'une bande sous la
  grille), plutôt que dans une classe séparée façon `SaveIcon`/`FlagIcons` : `SpriteRenderer` ne
  dessine qu'une seule texture
  par passe, étendre l'atlas existant évite toute restructuration du rendu. Premier de **deux
  lots** : ce lot livre une **pose statique unique** ; l'animation par séquence d'images
  (repos/course/saut, `EX-REN-012`) est explicitement un lot séparé à venir. Vérifié visuellement
  dans l'application ; `ctest` inchangé (292/292, aucune logique `Core` nouvelle — génération de
  texture non testable hors GPU).

## [0.0.2] - 2026-07-22

> Deuxième jalon : **éditeur de niveaux intégré**, du prototype (LOT-14) à l'outil de production
> (LOT-15 : nommage, garde-fous contre la perte de travail, caméra manuelle, outils de zone,
> panneau latéral, découvrabilité) puis aux grands niveaux (LOT-16 : saisie directe de taille,
> caméra qui cadre un niveau plus grand que la fenêtre). Un level designer peint, lie des
> mécanismes, redimensionne, annule/refait, enregistre et teste un niveau sans écrire de code —
> guide non-codeur inclus pour partager les niveaux via Git sans ligne de commande. **292 tests**
> (215 au jalon précédent), CI verte.
>
> Voir le [CHANGELOG](CHANGELOG.md) pour le détail par lot (LOT-14 → LOT-16).

### Ajouté
- **LOT-16 — Niveaux de grande taille** (`EX-EDIT-017`, `EX-REN-013` corrigée). L'éditeur permet
  désormais de **saisir directement** une taille cible (**Ctrl+R**, format `largeurxhauteur`, ex.
  `60x40`) plutôt que d'incrémenter case par case aux flèches (toujours disponibles pour l'ajustement
  fin) — un **plafond généreux** (100 cases par axe, garde-fou d'usage côté `HMI`, sans limite dans
  `Core`) s'applique aux deux voies, qui passent toutes deux par le même point de redimensionnement
  (`EditorScreen::requestResize`, même confirmation destructrice qu'avant si la nouvelle taille
  perdrait l'entrée/la sortie/une liaison). Le champ de saisie du nom (LOT-15) est généralisé pour
  porter aussi cet usage — un seul mécanisme de saisie de texte, pas un second construit en
  parallèle. **Un niveau plus grand que la fenêtre reste entièrement visible** : la caméra, qui ne
  descendait jamais sous le zoom ×1 (une partie de la grille restait hors champ, dans l'éditeur
  comme en jeu), zoome désormais **en dessous de ×1** quand c'est nécessaire pour cadrer le niveau
  entier — correction unique (`Camera2D::fitZoom`, testée) partagée par l'éditeur et `GameScreen`,
  sans dupliquer le calcul ; le zoom reste entier (netteté pixel art) pour tout niveau tenant déjà
  dans la fenêtre, aucune régression sur les niveaux livrés à ce jour. `EX-REN-013` reformulée pour
  refléter la stratégie réellement implémentée (caméra qui cadre le niveau, ne suit pas le
  personnage — la formulation d'origine ne correspondait déjà plus au code depuis LOT-08). Couvert
  par tests **unitaires** (`LevelSizeValidation`, `Camera2D::fitZoom`) ; vérifié manuellement dans
  l'application (grand niveau entièrement visible à l'ouverture et en essai immédiat, niveaux
  existants inchangés).
- **LOT-15 — Éditeur de niveaux : robustesse et confort d'édition** (`EX-EDIT-009`, `EX-EDIT-012` à
  `EX-EDIT-016`). L'éditeur intégré (LOT-14) se rapproche d'un outil de production : créer un
  niveau **demande un nom** (plus de collision silencieuse sur « Nouveau niveau.json ») et **F2**
  le renomme en cours d'édition ; enregistrer sous un nom qui **écraserait** un autre fichier, un
  **redimensionnement** qui supprimerait l'entrée/la sortie/une liaison, ou **Échap** avec des
  modifications **non enregistrées**, sont désormais **confirmés** avant d'agir. La caméra se
  **déplace** (glisser bouton droit) et **zoome** (molette) indépendamment du cadrage automatique
  (« 0 » y revient), bornée entre ce cadrage (rien à voir au-delà du niveau) et un maximum de
  **4 cases visibles** sur le plus petit axe (précision suffisante). Une **grille de repère**
  (lignes fines sur chaque bord de case) bascule au clavier (`F10`) pour simplifier le repérage
  d'une case avant d'y peindre. Deux nouveaux outils au-delà du pinceau : **Rectangle**
  (remplissage d'une zone glissée) et **Sélection** (`Ctrl+C`/`Ctrl+V`, copier/coller une zone de
  tuiles) — `Tab` fait défiler Pinceau/Rectangle/Sélection, la liaison de mécanismes (`Maj`+clic)
  restant disponible quel que soit l'outil actif. Découvrabilité : la palette et la barre d'outils
  rejoignent un **panneau latéral** vertical fixe (au lieu de bandes empilées pouvant se
  superposer entre elles ou avec la grille), avec un libellé à côté de chaque entrée ; un aperçu
  des raccourcis (`F1`) ; et des liaisons interrupteur↔porte teintées **différemment par
  interrupteur** (au lieu d'une seule teinte cyan partagée).
  Côté dette technique : l'essai immédiat (`P`) transmet désormais le niveau **directement en
  mémoire** à une session de jeu interne (plus de fichier temporaire partagé), et les messages
  d'erreur de validation s'appuient sur un **code d'erreur catégorisé** (`LevelValidationError`)
  plutôt que sur une recherche de sous-chaîne dans le message technique. `LevelDraft::paintRegion`
  (remplissage/collage) réutilise la sémantique cellule-par-cellule de `paintTile`, sans dupliquer
  de règle de niveau (`EX-EDIT-010`), en un seul snapshot undo par opération. Couvert par tests
  **unitaires** (`LevelDraft::paintRegion`/`wouldResizeDropContent`, `TextInputField`,
  `LevelNameValidation`, `ToolBar`, `InputState` — molette/texte tapé, `LevelLoader` — codes
  d'erreur) ; vérifié manuellement dans l'application pour la caméra et le glisser-déposer des
  outils de zone (logique non testable hors GPU, comme le reste d'`EditorScreen`/`GameScreen`).
- **LOT-14 — Éditeur de niveaux intégré** (`EX-EDIT-001` à `EX-EDIT-011`, `EX-EDIT-020` à
  `EX-EDIT-022`, `EX-EDIT-030`/`031`). Le menu **« Mode Édition »** ouvre désormais un véritable
  éditeur, intégré à l'application (même exécutable, même rendu Direct3D 11 que le jeu) : un
  sélecteur propose **« Nouveau niveau »** ou l'un des fichiers déjà enregistrés ; la grille se
  peint à la **souris** depuis une **palette** de tuiles (vide, solide, danger, entrée, sortie,
  interrupteur, porte) ; **Maj**+clic lie un interrupteur à une porte (répéter la paire la délie) ;
  les **flèches** redimensionnent la grille ; **Ctrl+Z**/**Ctrl+Y** annulent/refont n'importe
  quelle mutation (pile d'historique portée par `core::LevelDraft`, LOT-14) ; **Ctrl+S** valide
  puis enregistre le niveau (message clair, non technique, en cas d'échec — aucun fichier écrit) ;
  la touche **P** lance un **essai immédiat** du niveau en cours d'édition dans une session de jeu
  **intégrée** (Échap y met fin et restitue l'éditeur intact, brouillon et historique compris).
  Toute la logique de modèle/sérialisation/validation (`core::LevelWriter`, `core::LevelDraft`)
  vit dans `Core`, pure et testée sans GPU, et réutilise **sans duplication** la validation de
  `LevelLoader` (le brouillon se convertit en niveau en repassant par le même chemin que le
  chargement d'un fichier). Nouveau guide non-codeur (`Documentation/Manuel/partager-un-niveau.md`)
  expliquant, sans ligne de commande, comment créer un niveau et le partager via GitHub Desktop.
  Couvert par tests **unitaires** (`LevelWriter`, `LevelDraft` — mutateurs, undo/redo,
  `LevelPicker`, `TilePalette`) et un test **système** (parcours complet d'édition : peindre, lier
  un mécanisme, annuler, redimensionner, enregistrer, recharger, vérifier que le niveau produit est
  directement jouable). Vérifié manuellement dans l'application (peinture, palette, liaisons,
  redimensionnement, undo/redo, enregistrement sur disque, essai immédiat).

### Corrigé
- **`F10` (éditeur, LOT-15) ne déclenchait rien.** Win32 délivre cette touche (comme les
  combinaisons `Alt`+quelque-chose) via `WM_SYSKEYDOWN`/`WM_SYSKEYUP`, jamais
  `WM_KEYDOWN`/`WM_KEYUP` — une convention historique d'activation du menu, indépendante de
  l'absence de menu dans cette fenêtre. `Window::handleMessage` capture désormais aussi ces deux
  messages (comme les autres touches), en laissant `DefWindowProcW` traiter le comportement
  système par défaut (`Alt+F4`, `Alt+Tab`) — sauf pour `F10` lui-même, absorbé pour éviter
  l'activation visuelle, inutile ici, du (non-)système de menu au relâchement. Découvert lors d'un
  essai interactif par l'utilisateur.

## [0.0.1] - 2026-07-18

> Premier jalon : base stable du **moteur physique 2D** (personnage humanoïde, gravité asymétrique,
> saut/double-saut/wall-jump/dash, niveaux JSON avec mécanismes puzzle et budget de mouvements),
> **215 tests** verts, documentation consolidée (Guide du développeur + Cahier de test).

### Ajouté
- **LOT-13 — Consolidation de la documentation** : mise à jour des pages Doxygen et du `README.md`
  (section « Fonctionnalités du moteur », cibles de test unitaire/intégration/**système**). Nouvelle
  rubrique **Guide du développeur** (`Documentation/Guide/`) : accueil + 6 pages par domaine
  (physique, boucle de simulation, ECS, mathématiques, niveaux, entrées) expliquant chaque notion du
  moteur avec liens **code ↔ mathématiques** (balayage AABB, gravité, normalisation…). Nouvelle
  rubrique **Cahier de test** générée depuis les sources via une balise Doxygen avancée `\castest`
  (alias + `\xrefitem` agrégeant une page unique) : les **215 cas** (unitaires, intégration, système)
  sont annotés (titre, **catégorie**, sous-catégorie, **criticité**, **étapes** détaillées, **résultat
  attendu**), la documentation vivant **à côté du test** pour la maintenabilité. Aucun changement de
  comportement moteur.
- **LOT-12 — Niveau puzzle : mécanismes interrupteur/porte + budget de mouvements** (`EX-GP-020`, `EX-GP-021`, `EX-GP-024`). Les liaisons interrupteur↔porte (chargées depuis le LOT-07 mais inertes) deviennent **fonctionnelles** : un `MechanismController` (Core, pur) **bascule** l'état d'une porte au **contact** de son interrupteur (front) ; une porte **fermée** est **solide** (bloque), **ouverte** franchissable. La physique consomme une **grille de collision** reflétant l'état des portes (la carte du niveau reste la source de vérité). Nouveau **budget de mouvements par tableau** : un niveau peut limiter le nombre de **sauts** et/ou de **dashs** (`jumpBudget`/`dashBudget` dans le JSON, `-1` = illimité) ; à budget épuisé, l'action est **refusée**, et le budget est **réinitialisé** au (re)chargement. Le `GameScreen` exécute les mécanismes, initialise le budget au spawn, recharge le niveau à l'échec (mécanismes + budget remis) et **teinte** les portes selon leur état. Niveau puzzle `demo4.json` (interrupteur ouvrant la porte vers la sortie, budget serré) ajouté à la séquence. Couvert par tests **unitaires** (mécanismes, budget parsing), **intégration** (budget refusé au-delà, porte fermée bloque, `demo4` franchissable via l'interrupteur) et **système** (les 4 niveaux enchaînés).
- **LOT-11 — Ressenti avancé : personnage humanoïde, gravité asymétrique, finitions** (`EX-GP-018`). Le personnage n'est plus un carré 1×1 mais une **silhouette humanoïde** (0,4 × 0,8, collision **et** rendu), centrée dans la tuile d'entrée au spawn (`Core/Physics/PlayerSpawn.h`, source unique taille/placement). La **gravité** devient **asymétrique** : chute plus rapide que la montée (`fallGravityMultiplier`), **flottement à l'apex** (gravité réduite quand la vitesse verticale est faible) et **fast-fall** (maintenir « bas » accélère la chute, via `moveY`). La retombée reste à gravité **constante à multiplicateur près** (`EX-GP-011`). Toute la logique est **pure et déterministe au pas fixe**. Les preuves de franchissabilité (intégration) et le parcours **système** utilisent désormais la **vraie taille** du personnage. Couvert par tests **unitaires** (réglages, taille/spawn), **intégration** (chute > montée, apex hang, fast-fall) et **système** (les 3 niveaux franchis par l'humanoïde).
- **LOT-10 — Mécaniques aériennes avancées : double saut, wall jump, dash** (au-delà du MVP : nouvelles exigences `EX-GP-015`/`016`/`017`, `EX-CTRL-013` ; `EX-GP-013` assouplie). Le saut devient **contextuel** (sol/coyote, mur, ou saut aérien). **Double saut** (`EX-GP-015`) : N sauts aériens paramétrables, rechargés au contact du sol. **Wall jump + wall slide** (`EX-GP-016`) : contre un mur en l'air, le personnage glisse (chute plafonnée) et un saut l'éjecte **en diagonale opposée** au mur (avec verrou horizontal transitoire). **Dash** (`EX-GP-017`) : ruée à vitesse élevée dans l'une des **8 directions** (touches directionnelles, à défaut l'orientation), sur une courte durée, **gravité suspendue**, **une ruée par phase aérienne** rechargée au sol ; touche **Maj** (`EX-CTRL-013`). Toute la logique est **pure et déterministe au pas fixe** dans `Core`, câblée via `toPlayerInput`/`GameScreen`. Un troisième niveau `demo3.json` (couloir bas + fosse, **dash requis**) rejoint la séquence. Couvert par tests **unitaires** (mapping), **d'intégration** (double saut paramétrable, wall slide/jump, dash 8 directions, franchissabilité avec/sans mécanique) et **système** (parcours `demo → demo2 → demo3`).
- **LOT-09 — Saut, game feel et enchaînement de niveaux** : le personnage **saute** (`Espace`/`W`, `EX-GP-011`/`EX-GP-013`) — impulsion au sol, retombée sous **gravité constante** — avec un *game feel* de platformer : **hauteur de saut variable** (relâcher tôt = petit saut), **coyote time** (~80 ms, sauter juste après un bord) et **jump buffering** (~120 ms, saut pré-appuyé honoré à la pose). Les minuteries sont portées par le composant `Player` et décomptées au **pas fixe déterministe** (`EX-NFR-002`) ; l'intention de saut distingue *pressé*/*maintenu* (`EX-CTRL-011`, `toPlayerInput`). Le jeu enchaîne désormais une **séquence ordonnée de niveaux** (`LevelSequence`, `EX-LVL-010`) : à la réussite d'un niveau, le **suivant** se charge automatiquement ; après le **dernier**, retour au titre (`EX-LVL-011`). Deux niveaux livrés — `demo.json` (déplacement/chute) puis `demo2.json` (**marche ascendante, saut requis**). Couvert par tests **unitaires** (mapping saut, `LevelSequence`), **d'intégration** (saut au sol, pas de double saut, hauteur variable, coyote, buffering, franchissabilité avec/sans saut) et un premier test **système** (`Source/Test/Systeme/`) rejouant un **parcours complet** de la séquence sans la couche GPU.
- **LOT-08 — Gameplay personnage (jouable)** : le niveau statique devient **jouable**. Un **personnage** apparaît à l'entrée et est piloté au clavier — **actions logiques** dissociées des touches (`←`/`Q` gauche, `→`/`D` droite, `EX-CTRL-010`, `toPlayerInput`). La physique (`CharacterPhysicsSystem`) applique, **au pas fixe déterministe** (`EX-NFR-002`), le **déplacement horizontal** à vitesse constante (`EX-GP-010`), la **gravité continue** (`EX-GP-012`), et résout les **collisions** par **balayage continu** (`sweepAabb`) contre les tuiles solides — aucune traversée à vitesse élevée, glissement le long des surfaces (`EX-GP-014`). Atteindre la **sortie** revient au menu (`EX-GP-030`) ; toucher un **danger** ou tomber sous le niveau **redémarre** le personnage à l'entrée (`EX-GP-031`, `EX-GP-032`, `evaluateOutcome`). Jeu **par tableaux** : la caméra reste **fixe** et cadre le tableau (adaptation de `EX-REN-013`). Pas de **saut** ni de **mécanismes** (lots ultérieurs). Toute la simulation est **pure dans `Core`** (composants `Player`/`Collider`, `PhysicsConfig`, balayage `Aabb`) et **testée** (balayage, physique, règles, mapping) ; `HMI` ne fait que mapper les touches et afficher. Logique couverte par tests unitaires et d'intégration ; intégration `GameScreen` vérifiée visuellement.
- **Complétude des tests (audit)** : combler des trous de couverture — test unitaire de `approximatelyEqual` (tolérance relative-absolue, grandes magnitudes), test que le **catalogue français livré** (`fr.lang`) se charge et résout ses clés, et un test d'**intégration** « fichier de niveau livré (`demo.json`) → monde ECS » (une entité par tuile non vide). Rééquilibre unit/intégration.
- **LOT-07 — Niveaux (terminé)** : **« Charger niveau »** ouvre désormais un **vrai niveau** chargé depuis `Source/Elements/Levels/demo.json` (format JSON) au lieu de la scène codée en dur. `GameScreen` (TACHE-06) charge le fichier via `LevelLoader`, projette chaque tuile non vide en **entité ECS** (couleur d'atlas par type : solide, danger, entrée, sortie, interrupteur, porte) rendue par le `SpriteRenderer` (`EX-REN-010`, `EX-ARCH-012`), et ajuste la caméra pour cadrer le niveau. Échec de chargement **récupérable** (état neutre à l'écran, pas de plantage, `EX-NFR-040`) ; **Échap** revient au menu. Vérifié de bout en bout (grille de tuiles affichée conforme au fichier).
- **LOT-07 (TACHE-05)** : niveau de démonstration `Source/Elements/Levels/demo.json` (12×8, bords solides, entrée/sortie, danger, paire interrupteur/porte) au format JSON ; copié à côté de l'exécutable par CMake (comme les `.lang`). Un test unitaire vérifie que le niveau **livré se charge et se valide** via `LevelLoader`.
- **LOT-07 (TACHE-04)** : validation du niveau (`EX-LVL-004`) — le chargement rejette proprement, avec message exploitable, un niveau **incohérent** : tuile hors bornes, **positions en double**, entrée/sortie **absente** ou en **plusieurs exemplaires** (unicité), liaison de mécanisme non résolue. Couvert par tests unitaires.
- **LOT-07 (TACHE-03)** : chargement de niveau dans `Core/Levels` — `LevelLoader` lit le format **JSON** (objet `{name, width, height, tiles}`, `tiles` = liste d'objets `{x, y, type, …}`) via nlohmann/json et construit un `Level` ; liaisons interrupteur↔porte résolues par identifiant (`switch.id` ↔ `door.opensWith`). Renvoie un **résultat récupérable** (`LevelLoadResult`, jamais d'exception vers l'appelant, `EX-NFR-040`) : JSON malformé, champ manquant, type inconnu, tuile hors bornes, entrée/sortie absente, liaison non résolue. nlohmann/json reste **confiné au `.cpp`**. Couvert par tests unitaires (`EX-LVL-001`, `EX-LVL-003`).
- **LOT-07 (TACHE-02)** : modèle de niveau dans `Core/Levels` — `TileType` (`Empty`/`Solid`/`Danger`/`Entry`/`Exit`/`Switch`/`Door`), `TileMap` (grille dense typée, accès borné, `isSolid`), `GridPosition` et `Level` (nom + grille + entrée/sortie + mécanismes). Donnée pure, testable sans GPU (`EX-GP-001`, `EX-LVL-002`) ; couvert par tests unitaires.
- **LOT-07 (TACHE-01)** : dépendance **nlohmann/json** (v3.11.3, épinglée via FetchContent, *header-only*) ajoutée pour le parsing des fichiers de niveaux ; liée **en privé** à `Core` (`EX-NFR-031`).
- **Niveau de log configurable au lancement** : le niveau minimum du journaliseur (par défaut `Trace`) se règle via la variable d'environnement `JADG_LOG_LEVEL` ou l'argument `--log-level=<trace|info|warning|error>` (ce dernier prioritaire) ; une valeur non reconnue est ignorée et signalée. Un analyseur pur `core::parseLogLevel` (couvert par tests) fait la conversion. Permet de réduire le bruit en release ou d'augmenter le détail pour déboguer.
- **Bouton d'enregistrement des logs de la session** (**outil de développement, masqué en Release**) : un `MemoryLogSink` capture désormais tous les messages émis, et un **bouton** (icône « télécharger » générée en code) en bas à droite du menu, à gauche du drapeau, écrit la session dans `logs/session_AAAAMMJJ_HHMMSS.log` à côté de l'exécutable. Le bouton n'est présent que dans les builds de développement, via `core::kDeveloperBuild` (`if constexpr`, éliminé en Release). Logique du bouton (`SaveLogButton`) et sérialisation (`serializeSessionLog`) **pures et testées** ; l'action d'enregistrement est injectée dans `MenuScreen` (découplage de la journalisation/fichiers). Écriture **récupérable** (échec signalé, pas de plantage).
- **Journalisation des événements de cycle de vie sur tout le projet** (via l'infrastructure de log du LOT-02). Chaque module a sa **catégorie** de log (en-têtes dédiés, modèle `HMI/HmiLog.h`) : `Core`, `Ecs`, `Graphics`, `Platform`, `HMI`. Sont tracés : création et dimensions de la fenêtre (`Platform`), création des ressources de rendu — device/swap chain, `SpriteBatch`, atlas, police, drapeaux — et redimensionnement (`Graphics`), cadenceur à pas fixe et enregistrement des systèmes ECS (`Core`/`Ecs`), initialisation, **transitions d'écran** (Menu ↔ Jeu ↔ Éditeur), demande de fermeture et changement de langue (`HMI`). Les traces sont **pilotées par événement** — **aucune journalisation dans les chemins par frame** (rendu, mise à jour ECS) — et respectent les niveaux (`INFO` pour les jalons, `TRACE` pour le détail) ; les logs désactivés sont gratuits (garde `isEnabled` avant formatage). Objectif : rendre le déroulé de l'application observable pour le débogage plutôt qu'un fonctionnement silencieux.
- **LOT-06 (TACHE-08)** : sélecteur de langue au menu — un **bouton drapeau** en bas à droite affiche le drapeau de la **langue courante** et bascule entre **français** et **anglais** au clic (`EX-REN-033`). Ajoute le catalogue anglais (`en.lang`), des **icônes de drapeaux générées en code** (`FlagIcons` : France, Royaume-Uni) et une logique `LanguageSelector` **pure, testable sans GPU** (ancrage bas-droit, détection du clic, bascule) ; le `MenuScreen` recharge le catalogue au clic et redessine les libellés dans la nouvelle langue. Bascule **récupérable** (langue conservée si le fichier cible manque). Couvert par tests unitaires ; vérifié visuellement (menu fr ↔ en, drapeaux France / Royaume-Uni).
- **LOT-06 — Menu principal (terminé)** : au lancement, l'exécutable ouvre désormais un **menu principal** au lieu d'une scène codée en dur. `main` (TACHE-07) initialise les ressources partagées (device, `SpriteBatch`, atlas, `BitmapFont`, `Localization` chargée sur le français) et une boucle **pilotée par le `ScreenManager`** démarrant sur le `MenuScreen` : entrées échantillonnées **une fois par frame** (`EX-CTRL-021`), une mise à jour d'écran, puis effacement/rendu/présentation. « Charger niveau » ouvre la scène de démonstration, « Mode Edition » l'écran « à venir », « Quitter » ferme proprement ; **Échap** revient au menu (il ne ferme plus l'application). La scène de démonstration a quitté `main` pour le `GameScreen`. Vérifié de bout en bout (menu → jeu → menu → éditeur → fermeture).
- **LOT-06 (TACHE-06)** : écrans cibles dans `HMI/Interface` — `GameScreen` encapsule la **scène de démonstration** du LOT-05 (grille de tuiles + sprite mobile), avec son `core::World`+`MovementSystem` à pas fixe (`EX-ARCH-030`) et le rendu en lecture seule de l'ECS (`EX-ARCH-012`) ; `EditorScreen` affiche un placeholder « à venir » issu du catalogue de traduction (`EX-REN-032`). Depuis les deux écrans, **Échap** revient au menu. Chaque écran possède ses ressources (RAII), libérées à la transition.
- **LOT-06 (TACHE-05)** : écran de menu principal dans `HMI/Interface` — trois options (**Charger niveau**, **Mode Edition**, **Quitter**) navigables au **clavier** (flèches + Entrée, bouclage) **et à la souris** (survol + clic), l'option sélectionnée mise en évidence (`EX-REN-030`, `EX-CTRL-011`, `EX-CTRL-001`). Les libellés proviennent du **catalogue de traduction** par leur clé (aucun texte en dur). La logique de sélection/transition est isolée dans un `MenuModel` **pur, testable sans GPU** (`EX-NFR-010`), tandis que `MenuScreen` (`IScreen`) se limite au dessin via la police bitmap. Couvert par tests unitaires (navigation, bouclage, survol, clic, actions).
- **LOT-06 (TACHE-04)** : structure d'écrans dans `HMI/Interface` — interface `IScreen` (`update` renvoyant une intention de transition **rester / basculer / quitter**, `render`), `RenderContext` (ressources de rendu partagées) et `ScreenManager` qui détient l'écran courant, applique les transitions et fabrique l'écran suivant via une **fabrique injectée**. Découplé des écrans concrets (aucun écran n'en connaît un autre) et testable sans fenêtre ni GPU (`EX-NFR-010`) ; la demande « quitter » se propage à la boucle (`EX-REN-030`). Couvert par tests unitaires (exécution, basculement, fermeture).
- **LOT-06 (TACHE-03)** : catalogue de traduction (i18n) dans `HMI/Localization` — `Localization` résout tout texte d'interface par **clé** selon la **langue active**, chargée depuis un **fichier par langue** (`Source/Elements/Localization/<langue>.lang`, format `clé = valeur` UTF-8, commentaires `#`). **Repli déterministe** (langue active → langue par défaut → la clé elle-même) : une clé ou un fichier manquant est une erreur **récupérable**, jamais un plantage (`EX-REN-033`, `EX-NFR-040`). Logique pure, testable sans fenêtre ni GPU (`EX-NFR-010`) ; catalogue français fourni (`fr.lang`) et copié à côté de l'exécutable par CMake. Couvert par tests unitaires (analyse, repli, changement de langue, fichier absent).
- **LOT-06 (TACHE-02)** : rendu de texte dans `HMI/Graphics` — `BitmapFont`, police bitmap **monospace générée en code** (glyphes 5×7 intégrés couvrant ASCII imprimable, plus les lettres accentuées françaises composées d'une lettre de base et d'un diacritique). `drawText` dessine une chaîne **UTF-8** en **espace écran** (pixels) via le `SpriteBatch` : un quad par glyphe, teinte appliquée (glyphes blancs colorés par multiplication), échantillonnage *nearest* (`EX-REN-032`, `EX-ARCH-022`) ; les caractères non couverts sont ignorés sans plantage. Fournit aussi `textWidth`/`lineHeight` (centrage) et `screenProjection` (projection pixels → clip de l'interface). Ressources Direct3D en RAII. Vérifié visuellement (libellés du menu, jeu de caractères, accents, teintes).
- **LOT-06 (TACHE-01)** : entrées clavier/souris dans `HMI/Input` — `InputState` échantillonne l'état par frame et expose les fronts **pressée / maintenue / relâchée** (`EX-CTRL-011`) pour le clavier et les boutons souris, plus la position souris. Indépendant de toute fenêtre (aucun `<Windows.h>`), donc testable en isolation (`EX-NFR-010`) ; `hmi::Window` capture les messages Win32 (`WM_KEY*`, `WM_MOUSE*`) et échantillonne l'`InputState` une fois par frame en tête de `pumpMessages` (`EX-CTRL-021`), et expose `input()` + `requestClose()`. Couvert par tests unitaires (fronts déterministes).

### Modifié
- **Pas de console en Release** : l'exécutable est désormais bâti en **sous-système Windows** en Release (aucune fenêtre console — expérience utilisateur plus propre), tout en conservant la **console en Debug** (logs visibles). Le point d'entrée reste le `main()` standard (`/ENTRY:mainCRTStartup`). En conséquence, **aucun sink de log n'est ajouté en Release** (ni console ni capture mémoire) : pas de destination de log, pas de croissance mémoire.
- **LOT-06 (TACHE-01)** : la touche **Échap ne ferme plus** la fenêtre — elle devient une entrée normale (destinée au retour menu) ; la fermeture programmée passe désormais par `Window::requestClose()` (action « Quitter »), la croix continuant de fermer.

### Supprimé
- **Code mort** (audit projet) : retrait de fonctions publiques définies mais **jamais appelées ni testées** — `Camera2D::center()`/`zoom()`, `Logger::minimumLevel()`, `Localization::defaultLanguage()`. Le build `/W4 /WX` couvre déjà locals/paramètres/fonctions-fichier inutilisés ; l'API testée mais consommée au prochain lot (ex. `TileMap::isSolid`, `Level::entry/exit`, `MovementSystem`) et les macros de log uniformes (sans code généré) sont **conservées**.

### Corrigé
- La cible de tests `UnitTests` compile désormais aussi en **Release** : `test_assert.cpp` marquait `[[maybe_unused]]` manquant sur une variable non lue lorsque `JADG_ASSERT` est neutralisé en Release (`/W4 /WX`).

### Ajouté
- **LOT-05 — Rendu 2D (terminé)** : l'exécutable affiche désormais une **scène issue de l'ECS**. La boucle de `main` branche un `core::World` (avec le `MovementSystem`) cadencé à **pas fixe**, puis un **rendu découplé** en lecture seule. `main` construit une scène de démonstration (grille de tuiles + sprite mobile partiellement transparent) rendue via `SpriteRenderer` / `SpriteBatch` / `TextureAtlas` et la `Camera2D`. Première image réelle du jeu (tuiles, transparence, couches, déplacement déterministe).
- **LOT-05 (TACHE-05)** : système de rendu des sprites `SpriteRenderer` dans `HMI/Graphics` — parcourt `world.view<Transform, Sprite>()`, construit le quad monde de chaque entité, résout la région d'atlas en coordonnées de texture, trie par **couche** (tri stable) et dessine via le `SpriteBatch` en appliquant la projection de la caméra. Lecture seule de l'ECS (`EX-ARCH-012`) ; rendu découplé de la simulation.
- **LOT-05 (TACHE-04)** : caméra 2D `Camera2D` dans `HMI/Graphics` — projette le monde vers l'écran (16 px/unité, origine haut-gauche, Y-bas, zoom), fournit la matrice de projection pour le vertex shader et les conversions monde↔écran. Logique pure (DirectXMath), couverte par tests unitaires (conversions réciproques, projection).
- **LOT-05 (TACHE-03)** : atlas de textures procédural `TextureAtlas` dans `HMI/Graphics` — génère en mémoire une grille 4×4 de tuiles 16×16 (couleurs distinctes + une tuile à zones **transparentes** pour valider l'alpha), crée la `ID3D11Texture2D` et sa vue de ressource (RAII), et expose les régions (`tile(colonne, ligne)`). Génération déterministe, remplaçable plus tard par un chargement de fichier.
- **LOT-05 (TACHE-02)** : pipeline de rendu 2D `SpriteBatch` dans `HMI/Graphics` — dessine des quads texturés en Direct3D 11 (shaders HLSL compilés à l'exécution via `D3DCompile`), avec fusion **alpha** (transparence) et échantillonnage **nearest** (pixel art) ; API `begin`/`draw`/`end` avec *batching* (buffers dynamiques, RAII `ComPtr`). `GraphicsDevice` expose `device()`/`context()`.
- **LOT-05 (TACHE-01)** : composant `Sprite` (données pures) dans `Core/Ecs/Components` — région d'atlas (`AtlasRegion`, en pixels), couche de dessin et teinte (`Color` RVBA, blanc opaque par défaut). Agnostique du backend (aucun type DirectX) ; lu par le rendu de `HMI`. Couvert par tests unitaires.
- **LOT-04 — Documentation Doxygen (terminé)** : le site Doxygen est désormais une documentation **navigable** — page d'accueil (`index.md`), rubriques **Spécifications**, **Lots** et **Manuel utilisateur** (page « Télécharger et lancer »), en plus de la référence de code. Navigation hiérarchique (`@page`/`@subpage`) ; l'ordre des spécifications est porté par l'index (pas par des préfixes de fichiers). Garde-fou qualité `WARN_AS_ERROR` sur la génération. **Traçabilité des exigences** : chaque `EX-…` est une ancre Doxygen (`@ref`), avec un lint CI (`scripts/lint_exigences.py`) garantissant l'unicité des identifiants et l'absence de référence orpheline (mode `--next` pour le prochain numéro libre).

### Modifié
- **LOT-04 (TACHE-01)** : réorganisation de l'arborescence documentaire — `Specification/` et `Lot/` déplacés sous `Documentation/` ; le guide de conventions rejoint les spécifications (`Documentation/Specification/conventions.md`) ; les fichiers de spécification perdent leur préfixe numérique (`00-vision.md` → `vision.md`, …), l'ordre étant désormais porté par la navigation Doxygen. Références mises à jour dans tout le dépôt.

### Ajouté
- **Tests d'intégration** : première cible `IntegrationTests` (niveau *Integration*) — la démonstration de mouvement de l'ECS est reclassée depuis les tests unitaires (`test_ecs_mouvement.cpp`), et un test inter-lots vérifie que le cadenceur à pas fixe `FixedTimestep` (LOT-01) pilote correctement la simulation ECS (LOT-03) sans dérive et de façon déterministe (`test_boucle_simulation.cpp`).
- **LOT-03 (TACHE-06)** : premier composant réel et système de démonstration, bouclant l'ECS de bout en bout — composants données pures `Transform` (position, échelle, rotation, en unités monde) et `Velocity` (`Core/Ecs/Components`), et `MovementSystem` (`Core/Ecs/Systems`) qui applique `position += velocity * fixedDelta` aux entités possédant les deux composants. Déterministe, couvert par tests unitaires. **LOT-03 (fondation ECS & math `Core`) terminé.**
- **LOT-03 (TACHE-05)** : façade `World` et systèmes dans `Core/Ecs` — `World` possède les entités (`EntityManager`), les pools de composants (une par type, créée à la demande, via l'interface `IComponentPool`) et les systèmes ; API composants (`addComponent` / `getComponent` / `hasComponent` / `removeComponent`), vues (`view<...>()`) et cycle de vie (`destroyEntity` retire l'entité de **toutes** les pools). Interface `ISystem` (`update(World&, float)`) ; `World::update` exécute les systèmes **dans l'ordre d'enregistrement** au pas de temps fixe. Couvert par tests unitaires.
- **LOT-03 (TACHE-04)** : vue multi-composants dans `Core/Ecs` — `View<Components...>` itère exactement les entités possédant **tous** les composants demandés, pilotée par la plus petite pool (filtrage sur les autres). Deux formes d'usage : `for (auto [entity, ...] : view)` et `view.each([](Entity, A&, B&){ ... })`, avec accès en référence aux composants. Couvert par tests unitaires.
- **LOT-03 (TACHE-03)** : stockage de composants dans `Core/Ecs` — `ComponentPool<T>`, sparse set typé (tableau dense contigu + tableau creux indexé par entité) avec `add` / `get` / `has` / `remove` / `removeIfPresent` ; suppression par swap-and-pop préservant la densité ; `get`/`remove` sur entité absente traités par assertion de précondition. Couvert par tests unitaires.
- **LOT-03 (TACHE-02)** : entités de l'ECS dans `Core/Ecs` — `Entity` (handle générationnel `index`+`generation`, `INVALID_ENTITY`) et `EntityManager` (`create` / `destroy` / `isAlive`, recyclage des index par liste libre avec incrément de génération pour invalider les handles périmés). Couverts par tests unitaires.
- **LOT-03 (TACHE-01)** : types mathématiques de `Core` dans `Core/Math`, sans dépendance DirectX — `Vector2` (opérateurs, produit scalaire, longueur, normalisation, égalité approchée), `Rect` (bords, `contains`, `intersects`, origine haut-gauche / Y-bas) et `MathUtils.h` (`approximatelyEqual`, `kEpsilon`). Couverts par tests unitaires.
- **LOT-02** implémenté : journalisation & diagnostics dans `Core/Diagnostics` — niveaux de log, `Logger` (filtrage + sinks), `ConsoleLogSink` / `MemoryLogSink`, macros `JADG_LOG_*` (horodatage + fichier/ligne) et assertions `JADG_ASSERT` (handler surchargeable, actives en Debug). `main` journalise désormais son démarrage et ses erreurs.
- **LOT-01** implémenté : fenêtre Win32 (`hmi::Window`), initialisation Direct3D 11 en RAII (`hmi::GraphicsDevice`, effacement + présentation V-Sync + redimensionnement) et boucle de jeu à pas de temps fixe déterministe (`core::FixedTimestep`, testée). L'exécutable ouvre une fenêtre stable et se ferme proprement (croix / Échap).
- Arborescence du projet (`Specification/`, `Lot/`, `Documentation/`, `Source/`, `External/`).
- Découpage `Source/` : `Core`, `HMI`, `Elements`, `Test` (`Unit`, `Integration`, `Systeme`).
- Build **CMake** (C++20) avec presets Visual Studio et Ninja ; Visual Studio utilisé via son intégration CMake native.
- **GoogleTest** (FetchContent) et un premier test unitaire.
- **CI GitHub Actions** : configure, build et tests sur `windows-latest`.
- Documentation **Doxygen** (`Doxyfile`) et **guide de conventions** de code.
- Outillage qualité : `.clang-format`, `.clang-tidy`, `.editorconfig`, avertissements `/W4 /WX`, option AddressSanitizer.
- En-têtes précompilés (`Source/pch.h`, option `ENABLE_PCH`).
- CI : couverture de code (OpenCppCoverage, artefact Cobertura + HTML) et génération de la documentation Doxygen (artefact HTML).
- Gouvernance : `CONTRIBUTING.md` (Conventional Commits, trunk-based), `CHANGELOG.md`, `LICENSE`.
